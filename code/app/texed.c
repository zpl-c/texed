#include "zpl.h"

#include "raylib.h"
#include "raylib_helpers.h"
#include "cwpack/cwpack.h"

#define RAYGUI_IMPLEMENTATION
#define UI_ENHANCED_MOUSE
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_TEXTBOX_EXTENDED_IMPLEMENTATION
#include "gui_textbox_extended.h"

// NOTE(zaklaus): Bump it every release
#define TEXED_VERSION "0.4.6"

#define TD_DEFAULT_IMG_WIDTH 64
#define TD_DEFAULT_IMG_HEIGHT 64
#define TD_UI_PADDING 5.0f
#define TD_UI_PREVIEW_BORDER 4.0f
#define TD_UI_DEFAULT_ZOOM 4.0f
#define TD_IMAGES_MAX_STACK 128
#define TD_OP_MOD_ICON_DIM 20.0f
#define TD_OP_LIST_ITEM_HEIGHT 25.0f

static uint16_t screenWidth = 1280;
static uint16_t screenHeight = 720;
static float zoom = TD_UI_DEFAULT_ZOOM;
static float old_zoom = TD_UI_DEFAULT_ZOOM;
static Texture2D checker_tex;
static uint16_t old_screen_w;
static uint16_t old_screen_h;
static bool is_invalid_ver = false;
static bool is_repaint_locked = false;
static int render_tiles = 0;
static char workdir[1000] = {0};

#include "texed_params.h"

static td_param *selected_gizmo_param = NULL;

typedef struct {
    tcat_kind kind;
    char const *icon;
    Color color;
} tcat_desc;

typedef struct {
    twid_kind kind;
    uint8_t id;
} twid_desc;

#include "texed_ops_ids.h"

typedef struct {
    td_op_kind kind;
    char const *name;
    tcat_kind cat;
    bool is_hidden;
    bool is_locked;
    bool is_fixed;
    
    uint8_t num_params;
    td_param *params;
    
    uint8_t num_gizmos;
    twid_desc *gizmos;
} td_op;

#define OP(n) .kind = n, .name = #n 

typedef struct {
    bool visible;
    char const *title;
    char const *message;
    char const *buttons;
    int result;
} td_msgbox;

typedef struct {
    char *filepath;
    int32_t img_pos;
    Image img[TD_IMAGES_MAX_STACK];
    Texture2D tex;
    td_msgbox msgbox;
    bool is_saved;
    
    td_op *ops; //< zpl_array
    int selected_op;
    int bp_op;
} td_ctx;

static td_ctx ctx = {0};

static char filename[200];

#include "texed.h"
#include "texed_ops_list.c"
#include "texed_helpers.c"
#include "texed_img.c"
#include "texed_ops.c"
#include "texed_prj.c"
#include "texed_widgets.c"

void texed_raylib_logger(int level, char const *text, va_list args);

int main(int argc, char **argv) {
    zpl_opts opts={0};
    zpl_opts_init(&opts, zpl_heap(), argv[0]);
    zpl_opts_add(&opts, "f", "file", "open project file", ZPL_OPTS_STRING);
    zpl_opts_add(&opts, "td-i", "texed-import", "convert an image to ecotex format", ZPL_OPTS_STRING);
    zpl_opts_add(&opts, "td-ec", "texed-export-cc", "export ecotex image to C header file", ZPL_OPTS_STRING);
    zpl_opts_add(&opts, "td-ep", "texed-export-png", "export ecotex image to PNG format", ZPL_OPTS_STRING);
    zpl_opts_positional_add(&opts, "file");
    uint32_t ok = zpl_opts_compile(&opts, argc, argv);
    
    if (!ok) {
        zpl_opts_print_errors(&opts);
        zpl_opts_print_help(&opts);
        return -1;
    }
    
    // NOTE(zaklaus): create dirs if they don't exist yet
    {
        zpl_path_mkdir_recursive("art/gen", 0660);
        strcpy(workdir, GetWorkingDirectory());
        zpl_printf("Texed version: %s\n", TEXED_VERSION);
        zpl_printf("workdir: %s\n", workdir);
        SetTraceLogCallback(texed_raylib_logger);
    }
    
    if (zpl_opts_has_arg(&opts, "texed-import")) {
        zpl_string path = zpl_opts_string(&opts, "texed-import", "");
        if (FileExists(zpl_bprintf("art/%s", path)) && IsFileExtension(path, ".png")) {
            Image orig = LoadImage(zpl_bprintf("art/%s", path));
            texed_new(orig.width, orig.height);
            is_repaint_locked = true;
            texed_add_op(TOP_DRAW_IMAGE);
            td_param *params = ctx.ops[1].params;
            zpl_strcpy(params[0].str, path);
            is_repaint_locked = false;
            texed_compose_image();
            zpl_strcpy(filename, zpl_bprintf("%s", GetFileNameWithoutExt(path)));
            ctx.filepath = filename;
            texed_save();
        } else {
            zpl_printf("%s\n", "provided file does not exist!");
        }
        return 0;
    }
    
    if (zpl_opts_has_arg(&opts, "texed-export-cc")) {
        zpl_string path = zpl_opts_string(&opts, "texed-export-cc", "");
        if (FileExists(zpl_bprintf("art/%s.ecotex", path))) {
            zpl_array_init(ctx.ops, zpl_heap());
            zpl_strcpy(filename, zpl_bprintf("%s", path));
            ctx.filepath = filename;
            texed_load();
            texed_export_cc(path);
        } else {
            zpl_printf("%s\n", "provided file does not exist!");
        }
        return 0;
    }
    
    if (zpl_opts_has_arg(&opts, "texed-export-png")) {
        zpl_string path = zpl_opts_string(&opts, "texed-export-png", "");
        if (FileExists(zpl_bprintf("art/%s.ecotex", path))) {
            zpl_array_init(ctx.ops, zpl_heap());
            zpl_strcpy(filename, zpl_bprintf("%s.ecotex", path));
            ctx.filepath = filename;
            texed_load();
            texed_export_png(path);
        } else {
            zpl_printf("%s\n", "provided file does not exist!");
        }
        return 0;
    }
    
    InitWindow(screenWidth, screenHeight, "zpl.texed: stack-based texture generation tool");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    
    texed_new(TD_DEFAULT_IMG_WIDTH, TD_DEFAULT_IMG_HEIGHT);
    
    if (zpl_opts_has_arg(&opts, "file")) {
        zpl_string path = zpl_opts_string(&opts, "file", "");
        if (FileExists(zpl_bprintf("art/%s.ecotex", path))) {
            zpl_strcpy(filename, zpl_bprintf("%s", path));
            ctx.filepath = filename;
            texed_load();
        } else {
            zpl_printf("provided file does not exist: %s\n", path);
            return 1;
        }
    }
    
    while (1) {
        zpl_aabb2 screen = {
            .min = (zpl_vec2) {.x = 0.0f, .y = 0.0f},
            .max = (zpl_vec2) {.x = GetScreenWidth(), .y = GetScreenHeight()},
        };
        zpl_aabb2 orig_screen = screen;
        
        zpl_aabb2 topbar = zpl_aabb2_cut_top(&screen, 20.0f);
        zpl_aabb2 oplist_pane = zpl_aabb2_cut_right(&screen, screenWidth / 2.0f);
        zpl_aabb2 property_pane = zpl_aabb2_cut_bottom(&screen, screenHeight / 2.0f);
        zpl_aabb2 preview_window = screen;
        
        // NOTE(zaklaus): contract all panes for a clean UI separation
        oplist_pane = zpl_aabb2_contract(&oplist_pane, TD_UI_PADDING);
        preview_window = zpl_aabb2_contract(&preview_window, TD_UI_PADDING);
        property_pane = zpl_aabb2_contract(&property_pane, TD_UI_PADDING);
        
        Rectangle preview_rect = aabb2_ray(preview_window);
        
        if (old_screen_w != GetScreenWidth() || old_screen_h != GetScreenHeight()) {
            old_screen_w = GetScreenWidth();
            old_screen_h = GetScreenHeight();
            Image checkerboard = GenImageChecked(preview_rect.width, preview_rect.height, 16, 16, BLACK, ColorAlpha(GRAY, 0.2f));
            checker_tex = LoadTextureFromImage(checkerboard);
            UnloadImage(checkerboard);
        }
        
        // NOTE(zaklaus): ensure we reset styling to our defaults each frame
        {
            GuiLoadStyleDefault();
            GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x012e33ff);
            GuiSetStyle(BUTTON, BASE, 0x202020ff);
            GuiSetStyle(BUTTON, BASE + GUI_STATE_DISABLED*3, 0x303030ff);
            GuiSetStyle(BUTTON, TEXT + GUI_STATE_FOCUSED*3, 0x303030ff);
            GuiSetStyle(BUTTON, BORDER, 0xffffffff);
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xffffffff);
            GuiSetStyle(LISTVIEW, SCROLLBAR_SIDE, SCROLLBAR_LEFT_SIDE);
        }
        
        BeginDrawing();
        GuiResetFrame();
        ClearBackground(GetColor(0x222034));
        {
            if (ctx.msgbox.visible) GuiLock();
            
            DrawTextureEx(checker_tex, (Vector2){ preview_window.min.x, preview_window.min.y}, 0.0f, 1.0f, WHITE);
            
            Rectangle tex_rect = aabb2_ray(preview_window);
            float tile_x = tex_rect.x + zpl_max(0.0f, tex_rect.width/2.0f - (ctx.tex.width*zoom)/2.0f);
            float tile_y = tex_rect.y + zpl_max(0.0f, tex_rect.height/2.0f - (ctx.tex.height*zoom)/2.0f);
            
            for (int x = -render_tiles; x <= render_tiles; x++) {
                for (int y = -render_tiles; y <= render_tiles; y++) {
                    DrawTextureEx(ctx.tex, (Vector2){tile_x + (ctx.tex.width*zoom) * x, tile_y + (ctx.tex.height*zoom)*y}, 0.0f, zoom, WHITE);
                }
            }
            
            DrawAABB(topbar, BLACK);
            DrawAABB(property_pane, GetColor(0x422060));
            DrawAABB(oplist_pane, GetColor(0x425060));
            
            if (selected_gizmo_param) GuiLock();
            texed_draw_topbar(topbar);
            texed_draw_props_pane(property_pane);
            texed_draw_oplist_pane(oplist_pane);
            texed_draw_gizmos(preview_window);
            if (selected_gizmo_param) GuiUnlock();
            
            if (ctx.msgbox.visible) GuiUnlock();
            
            texed_draw_msgbox(orig_screen);
        }
        EndDrawing();
        
        static bool exit_pending = false;
        if (WindowShouldClose()) {
            if (!ctx.is_saved) {
                texed_msgbox_init("Discard unsaved work?", "You have an unsaved work! Do you want to proceed?", "OK;Cancel");
                exit_pending = true;
            } else {
                break;
            }
        }
        
        if (exit_pending && ctx.msgbox.result != -1) {
            exit_pending = false;
            if (ctx.msgbox.result == 1) {
                break;
            }
            ctx.msgbox.result = -1;
        }
        
        if (is_invalid_ver && ctx.msgbox.result != -1) {
            is_invalid_ver = false;
            ctx.msgbox.result = -1;
            if (zpl_array_count(ctx.ops) < 1) {
                texed_new(TD_DEFAULT_IMG_WIDTH, TD_DEFAULT_IMG_HEIGHT);
            }
        }
    }
    
    UnloadTexture(checker_tex);
    zpl_opts_free(&opts);
    texed_destroy();
    return 0;
}

void texed_new(int32_t w, int32_t h) {
    ctx.img_pos = -1;
    ctx.selected_op = -1;
    ctx.bp_op = -1;
    zpl_memset(ctx.img, 0, sizeof(Image)*TD_IMAGES_MAX_STACK);
    ctx.filepath = NULL;
    ctx.msgbox.result = -1;
    zpl_array_init(ctx.ops, zpl_heap());
    is_repaint_locked = true;
    texed_add_op(TOP_NEW_IMAGE);
    zpl_i64_to_str(w, ctx.ops[0].params[0].str, 10);
    zpl_i64_to_str(h, ctx.ops[0].params[1].str, 10);
    is_repaint_locked = false;
    texed_repaint_preview();
    
    ctx.is_saved = true;
}

void texed_clear(void) {
    zpl_array_clear(ctx.ops);
    for (int i = 0; i <= ctx.img_pos; i+=1)
        UnloadImage(ctx.img[i]);
    ctx.img_pos = -1;
    ctx.selected_op = -1;
}

void texed_destroy(void) {
    texed_clear();
    CloseWindow();
}

void texed_export_cc(char const *path) {
    zpl_printf("Building texture %s ...\n", zpl_bprintf("art/gen/%s.h", GetFileNameWithoutExt(path)));
    ExportImageAsCode(ctx.img[ctx.img_pos], zpl_bprintf("art/gen/%s.h", GetFileNameWithoutExt(path)));
}

void texed_export_png(char const *path) {
    zpl_printf("Exporting texture %s ...\n", zpl_bprintf("art/gen/%s.png", GetFileNameWithoutExt(path)));
    ExportImage(ctx.img[ctx.img_pos], zpl_bprintf("art/gen/%s.png", GetFileNameWithoutExt(path)));
}

void texed_msgbox_init(char const *title, char const *message, char const *buttons) {
    ctx.msgbox.result = -1;
    ctx.msgbox.visible = true;
    ctx.msgbox.title = title;
    ctx.msgbox.message = message;
    ctx.msgbox.buttons = buttons;
}

void texed_raylib_logger(int level, char const *text, va_list args) {
    (void)level;
    zpl_printf_va(text, args);
    zpl_printf("%s", "\n");
}