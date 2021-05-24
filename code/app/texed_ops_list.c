
#define PARAM(k,n,v) { .kind = k, .name = n, .str = v }
#define PARAMS(n) .num_params = n, .params = (td_param[])
#define PARAM_DEF_COLOR "000000ff"

static td_op default_ops[] = {
    {
        OP(TOP_NEW_IMAGE),
        .cat = TCAT_STACK,
        .is_fixed = true,
        PARAMS(3) {
            PARAM(TPARAM_COORD, "w", "64"),
            PARAM(TPARAM_COORD, "h", "64"),
            PARAM(TPARAM_COLOR, "color", "ffffffff"),
        }
    },{
        OP(TOP_PUSH_IMAGE),
        .cat = TCAT_STACK,
        PARAMS(3) {
            PARAM(TPARAM_COORD, "w", "64"),
            PARAM(TPARAM_COORD, "h", "64"),
            PARAM(TPARAM_COLOR, "color", "ffffffff"),
        }
    },{
        OP(TOP_POP_IMAGE),
        .cat = TCAT_STACK,
        PARAMS(4) {
            PARAM(TPARAM_PAD, "pos", "0,0"),
            PARAM(TPARAM_COORD, "w", "0"),
            PARAM(TPARAM_COORD, "h", "0"),
            PARAM(TPARAM_COLOR, "tint", "ffffffff"),
        }
    },{
        OP(TOP_CLONE_IMAGE),
        .cat = TCAT_STACK,
        PARAMS(1) {
            PARAM(TPARAM_COLOR, "tint", "ffffffff"),
        }
    },{
        OP(TOP_DROP_IMAGE),
        .cat = TCAT_STACK,
    },{
        OP(TOP_IMAGE_ALPHA_MASK),
        .cat = TCAT_STACK,
    },{
        OP(TOP_IMAGE_ALPHA_MASK_CLEAR),
        .cat = TCAT_STACK,
        PARAMS(2) {
            PARAM(TPARAM_COLOR, "color", "ffffffff"),
            PARAM(TPARAM_FLOAT, "threshold", "1.0"),
        }
    },{
        OP(TOP_DRAW_RECT),
        .cat = TCAT_DRAW,
        PARAMS(3) {
            PARAM(TPARAM_PAD, "p1", "0,0"),
            PARAM(TPARAM_PAD, "p2", "0.1,0.1"),
            PARAM(TPARAM_COLOR, "color", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_DRAW_LINE),
        .cat = TCAT_DRAW,
        PARAMS(3) {
            PARAM(TPARAM_PAD, "p1", "0,0"),
            PARAM(TPARAM_PAD, "p2", "1,1"),
            PARAM(TPARAM_COLOR, "color", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_DRAW_IMAGE),
        .cat = TCAT_DRAW,
        PARAMS(7) {
            PARAM(TPARAM_STRING, "src", "samples/test.png"),
            PARAM(TPARAM_PAD, "pos", "0,0"),
            PARAM(TPARAM_COORD, "w", "0"),
            PARAM(TPARAM_COORD, "h", "0"),
            PARAM(TPARAM_COLOR, "tint", "ffffffff"),
            PARAM(TPARAM_INT, "flip?", "0"),
            PARAM(TPARAM_INT, "rotate?", "0"),
        }
    },{
        OP(TOP_DRAW_IMAGE_INSTANCE),
        .cat = TCAT_DRAW,
        PARAMS(4) {
            PARAM(TPARAM_PAD, "pos", "0,0"),
            PARAM(TPARAM_COORD, "w", "0"),
            PARAM(TPARAM_COORD, "h", "0"),
            PARAM(TPARAM_COLOR, "tint", "ffffffff"),
        }
    },{
        OP(TOP_DRAW_TEXT),
        .cat = TCAT_DRAW,
        PARAMS(4) {
            PARAM(TPARAM_STRING, "text", "hello world"),
            PARAM(TPARAM_PAD, "pos", "0,0"),
            PARAM(TPARAM_COORD, "size", "16"),
            PARAM(TPARAM_COLOR, "color", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_DITHER),
        .cat = TCAT_MOD,
        PARAMS(4) {
            PARAM(TPARAM_INT, "r_bpp", "4"),
            PARAM(TPARAM_INT, "g_bpp", "4"),
            PARAM(TPARAM_INT, "b_bpp", "4"),
            PARAM(TPARAM_INT, "a_bpp", "4"),
        }
    },{
        OP(TOP_RESIZE_IMAGE),
        .cat = TCAT_MOD,
        PARAMS(3) {
            PARAM(TPARAM_COORD, "w", "64"),
            PARAM(TPARAM_COORD, "h", "64"),
            PARAM(TPARAM_COORD, "mode (0=nearest,1=bicubic)", "0"),
        }
    },{
        OP(TOP_COLOR_TWEAKS),
        .cat = TCAT_MOD,
        PARAMS(5) {
            PARAM(TPARAM_SLIDER, "contrast", "0.5"),
            PARAM(TPARAM_SLIDER, "brightness", "0.5"),
            PARAM(TPARAM_COLOR, "tint", "FFFFFFFF"),
            PARAM(TPARAM_INT, "invert?", "0"),
            PARAM(TPARAM_INT, "grayscale?", "0"),
        }
    },{
        OP(TOP_BLUR_IMAGE),
        .cat = TCAT_MOD,
        PARAMS(1) {
            PARAM(TPARAM_COORD, "amount", "4"),
        }
    },{
        OP(TOP_COLOR_REPLACE),
        .cat = TCAT_MOD,
        PARAMS(2) {
            PARAM(TPARAM_COLOR, "original", "FFFFFFFF"),
            PARAM(TPARAM_COLOR, "new", "FF0000FF"),
        }
    },{
        OP(TOP_IMAGE_GRAD_V),
        .cat = TCAT_GEN,
        PARAMS(2) {
            PARAM(TPARAM_COLOR, "top", "ffffffff"),
            PARAM(TPARAM_COLOR, "bottom", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_IMAGE_GRAD_H),
        .cat = TCAT_GEN,
        PARAMS(2) {
            PARAM(TPARAM_COLOR, "left", "ffffffff"),
            PARAM(TPARAM_COLOR, "right", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_IMAGE_GRAD_RAD),
        .cat = TCAT_GEN,
        PARAMS(3) {
            PARAM(TPARAM_SLIDER, "density", "0.5"),
            PARAM(TPARAM_COLOR, "inner", "ffffffff"),
            PARAM(TPARAM_COLOR, "outer", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_IMAGE_CHECKED),
        .cat = TCAT_GEN,
        PARAMS(4) {
            PARAM(TPARAM_COORD, "checks_x", "16"),
            PARAM(TPARAM_COORD, "checks_y", "16"),
            PARAM(TPARAM_COLOR, "color1", "ffffffff"),
            PARAM(TPARAM_COLOR, "color2", PARAM_DEF_COLOR),
        }
    },{
        OP(TOP_IMAGE_NOISE_WHITE),
        .cat = TCAT_GEN,
        PARAMS(4) {
            PARAM(TPARAM_COORD, "seed", "1"),
            PARAM(TPARAM_SLIDER, "factor", "0.5"),
            PARAM(TPARAM_COLOR, "color1", PARAM_DEF_COLOR),
            PARAM(TPARAM_COLOR, "color2", "ffffffff"),
        }
    },{
        OP(TOP_IMAGE_NOISE_PERLIN),
        .cat = TCAT_GEN,
        PARAMS(8) {
            PARAM(TPARAM_COORD, "offset_x", "0"),
            PARAM(TPARAM_COORD, "offset_y", "0"),
            PARAM(TPARAM_SLIDER, "scale", "0.02"),
            PARAM(TPARAM_COLOR, "color1", PARAM_DEF_COLOR),
            PARAM(TPARAM_COLOR, "color2", "ffffffff"),
            PARAM(TPARAM_SLIDER, "lacunarity", "0.2"),
            PARAM(TPARAM_SLIDER, "gain", "0.5"),
            PARAM(TPARAM_COORD, "octaves", "6"),
        }
    },{
        OP(TOP_IMAGE_CELLULAR),
        .cat = TCAT_GEN,
        PARAMS(6) {
            PARAM(TPARAM_COORD, "seed", "1"),
            PARAM(TPARAM_COORD, "tile_size", "16"),
            PARAM(TPARAM_COLOR, "color1", PARAM_DEF_COLOR),
            PARAM(TPARAM_COLOR, "color2", "ffffffff"),
            PARAM(TPARAM_SLIDER, "cell_offset", "0.0"),
            PARAM(TPARAM_SLIDER, "density", "1.0"),
        }
    },{
        OP(TOP_TRACE_SUN),
        .cat = TCAT_RAY,
        PARAMS(2) {
            PARAM(TPARAM_SLIDER, "azimuth", "0.25"),
            PARAM(TPARAM_SLIDER, "elevation", "0.5"),
        }
    }
};

#define DEF_OPS_LEN (int)(sizeof(default_ops) / (sizeof(default_ops[0])))

static tcat_desc default_cats[] = {
    {.kind = TCAT_STACK, .icon = "#197#", .color = RED},
    {.kind = TCAT_GEN, .icon = "#125#", .color = BLUE},
    {.kind = TCAT_DRAW, .icon = "#23#", .color = GREEN},
    {.kind = TCAT_MOD, .icon = "#25#", .color = ORANGE},
    {.kind = TCAT_RAY, .icon = "#119#", .color = PURPLE},
};
