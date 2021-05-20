
void texed_img_push(int w, int h, Color color) {
    if (ctx.img_pos == TD_IMAGES_MAX_STACK)
        return;
    
    ctx.img_pos += 1;
    ctx.img[ctx.img_pos] = GenImageColor(w, h, color);
}

void texed_img_pop(int x, int y, int w, int h, Color tint, bool drop) {
    if (ctx.img_pos == 0)
        return;
    
    Image *oi = &ctx.img[ctx.img_pos];
    Image *di = &ctx.img[ctx.img_pos-1];
    
    Rectangle src = {
        0, 0,
        oi->width, oi->height
    };
    
    w = (w == 0) ? di->width : w;
    h = (h == 0) ? di->height : h;
    
    Rectangle dst = {
        x, y,
        w, h,
    };
    
    ImageDraw(di, *oi, src, dst, tint); 
    if (drop) {
        UnloadImage(ctx.img[ctx.img_pos]);
        ctx.img_pos -= 1;
    }
}

void texed_repaint_preview(void) {
    if (is_repaint_locked) return;
    texed_compose_image();
    
    if (!IsWindowReady()) return;
    UnloadTexture(ctx.tex);
    ctx.tex = LoadTextureFromImage(ctx.img[ctx.img_pos]);
}

void texed_compose_image(void) {
    if (is_repaint_locked) return;
    ctx.is_saved = false;
    texed_process_params();
    texed_process_ops();
}
