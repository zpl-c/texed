#pragma once

void texed_new(int w, int h);
void texed_clear(void);
void texed_destroy(void);
void texed_load(void);
void texed_save(void);
void texed_export_cc(char const *path);
void texed_export_png(char const *path);
void texed_repaint_preview(void);
void texed_compose_image(void);
void texed_msgbox_init(char const *title, char const *message, char const *buttons);
void texed_process_ops(void);
void texed_process_params(void);

void texed_img_push(int w, int h, Color color);
void texed_img_pop(int x, int y, int w, int h, Color tint, bool drop);

void texed_add_op(int kind);
void texed_clone_op(int idx, td_op *dop);
void texed_rem_op(int idx);
void texed_swp_op(int idx, int idx2);
int texed_find_op(int kind);

void texed_draw_oplist_pane(zpl_aabb2 r);
void texed_draw_props_pane(zpl_aabb2 r);
void texed_draw_topbar(zpl_aabb2 r);
void texed_draw_msgbox(zpl_aabb2 r);
