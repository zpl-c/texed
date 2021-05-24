#include "texed_ops_helpers.c"
#include "texed_ops_mgr.c"

void texed_process_ops(void) {
    for (int i = 0; i <= ctx.img_pos; i+=1)
        UnloadImage(ctx.img[i]);
    ctx.img_pos = -1;
    
    for (int i = 0; i < zpl_array_count(ctx.ops); i += 1) {
        td_op *op = &ctx.ops[i];
        if (op->is_hidden) continue;
        
        int iw = ctx.img[ctx.img_pos].width;
        int ih = ctx.img[ctx.img_pos].height;
        Image *ip = &ctx.img[ctx.img_pos];
        switch (op->kind) {
            case TOP_PUSH_IMAGE:
            case TOP_NEW_IMAGE: {
                texed_img_push(op->params[0].i32, op->params[1].i32, op->params[2].color);
            }break;
            case TOP_POP_IMAGE: {
                if (ctx.img_pos > 0) {
                    iw = ctx.img[ctx.img_pos-1].width;
                    ih = ctx.img[ctx.img_pos-1].height;
                }
                texed_img_pop(op->params[0].vec.x*iw,
                              op->params[0].vec.y*ih,
                              op->params[1].i32,
                              op->params[2].i32,
                              op->params[3].color,
                              true);
            }break;
            case TOP_DRAW_IMAGE_INSTANCE: {
                if (ctx.img_pos > 0) {
                    iw = ctx.img[ctx.img_pos-1].width;
                    ih = ctx.img[ctx.img_pos-1].height;
                }
                texed_img_pop(op->params[0].vec.x*iw,
                              op->params[0].vec.y*ih,
                              op->params[1].i32,
                              op->params[2].i32,
                              op->params[3].color,
                              false);
            }break;
            case TOP_DROP_IMAGE: {
                if (ctx.img_pos > 0) {
                    UnloadImage(ctx.img[ctx.img_pos]);
                    ctx.img_pos--;
                }
            }break;
            case TOP_CLONE_IMAGE: {
                Image *oi = ip;
                Image *di = texed_img_push(oi->width, oi->height, BLANK);
                Rectangle rec = {0, 0, oi->width, oi->height};
                ImageDraw(di, *oi, rec, rec, op->params[0].color);
            }break;
            case TOP_IMAGE_ALPHA_MASK: {
                if (ctx.img_pos == 0) break;
                
                Image *oi = ip;
                Image *di = &ctx.img[ctx.img_pos-1];
                ImageAlphaMask(di, *oi);
                ctx.img_pos--;
            }break;
            case TOP_IMAGE_ALPHA_MASK_CLEAR: {
                ImageAlphaClear(ip, op->params[0].color, op->params[1].flt);
            }break;
            case TOP_DRAW_RECT: {
                int px = (int)(op->params[0].vec.x*iw);
                int py = (int)(op->params[0].vec.y*ih);
                ImageDrawRectangle(ip, 
                                   px,
                                   py,
                                   op->params[1].vec.x*iw - px,
                                   op->params[1].vec.y*ih - py,
                                   op->params[2].color);
            }break;
            case TOP_DRAW_LINE: {
                texed_draw_line(ip, 
                                op->params[0].vec.x*iw,
                                op->params[0].vec.y*ih,
                                op->params[1].vec.x*iw,
                                op->params[1].vec.y*ih,
                                op->params[2].color);
            }break;
            case TOP_DITHER: {
                ImageDither(ip,
                            op->params[0].i32,
                            op->params[1].i32,
                            op->params[2].i32,
                            op->params[3].i32);
            }break;
            case TOP_DRAW_IMAGE: {
                char const *str = zpl_bprintf("art/%s", op->params[0].str);
                if (FileExists(str)) {
                    Image img = LoadImage(str);
                    int x = (int)(op->params[1].vec.x*iw);
                    int y = (int)(op->params[1].vec.y*ih);
                    int w = op->params[2].i32;
                    int h = op->params[3].i32;
                    int flip = op->params[5].i32;
                    int rotate = op->params[6].i32;
                    
                    if (w != 0 || h != 0) {
                        ImageResize(&img, w != 0 ? w : img.width, h != 0 ? h : img.height);
                    }
                    
                    if (flip == 1) {
                        ImageFlipVertical(&img);
                    } else if (flip == 2) {
                        ImageFlipHorizontal(&img);
                    }
                    
                    if (rotate == 1) {
                        ImageRotateCW(&img);
                    } else if (rotate == 2) {
                        ImageRotateCCW(&img);
                    }
                    
                    ImageDraw(ip, img, 
                              (Rectangle){0.0f, 0.0f, img.width, img.height},
                              (Rectangle){x, y, img.width, img.height},
                              op->params[4].color);
                    
                    UnloadImage(img);
                } else {
                    zpl_printf("TOP_LOAD_IMAGE: src %s not found!\n", str);
                }
            }break;
            case TOP_DRAW_TEXT: {
                char const *str = op->params[0].str;
                int x = op->params[1].vec.x*iw;
                int y = op->params[1].vec.y*ih;
                int size = op->params[2].i32;
                Color color = op->params[3].color;
                ImageDrawText(ip, str, x, y, size, color);
            }break;
            case TOP_RESIZE_IMAGE: {
                if (ctx.img[ctx.img_pos].width == 0) break;
                int w = op->params[0].i32;
                int h = op->params[1].i32;
                int mode = op->params[2].i32;
                if (mode) {
                    ImageResize(ip, w, h);
                } else {
                    ImageResizeNN(ip, w, h);
                }
            }break;
            case TOP_COLOR_TWEAKS: {
                ImageColorContrast(ip, texed_map_value(op->params[0].flt, -100.0f, 100.0f));
                ImageColorBrightness(ip, (int)texed_map_value(op->params[1].flt, -255.0f, 255.0f));
                ImageColorTint(ip, op->params[2].color);
                
                if (op->params[3].i32) {
                    ImageColorInvert(ip);
                }
                if (op->params[4].i32) {
                    ImageColorGrayscale(ip);
                }
            }break;
            case TOP_BLUR_IMAGE: {
                texed_blur_image(ip, op->params[0].u32);
            }break;
            case TOP_COLOR_REPLACE: {
                ImageColorReplace(ip, op->params[0].color, op->params[1].color);
            }break;
            case TOP_IMAGE_GRAD_V: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageGradientV(w, h, op->params[0].color, op->params[1].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_GRAD_H: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageGradientH(w, h, op->params[0].color, op->params[1].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_GRAD_RAD: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageGradientRadial(w, h, 
                                                   op->params[0].flt,
                                                   op->params[1].color, 
                                                   op->params[2].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_CHECKED: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageChecked(w, h, 
                                            op->params[0].i32,
                                            op->params[1].i32,
                                            op->params[2].color, 
                                            op->params[3].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_NOISE_WHITE: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = texed_generate_noise(op->params[0].u32, 
                                                 w, h, 
                                                 op->params[1].flt,
                                                 op->params[2].color,
                                                 op->params[3].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_NOISE_PERLIN: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = texed_generate_perlin(w, h, 
                                                  op->params[0].i32,
                                                  op->params[1].i32,
                                                  op->params[2].flt*50.0f,
                                                  op->params[3].color,
                                                  op->params[4].color,
                                                  op->params[5].flt*10.0f,
                                                  op->params[6].flt,
                                                  op->params[7].i32);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_CELLULAR: {
                Image *dst = ip;
                int w = dst->width;
                int h = dst->height;
                Image img = texed_generate_cellular(op->params[0].u32,
                                                    w, h, 
                                                    op->params[1].i32,
                                                    op->params[2].color,
                                                    op->params[3].color,
                                                    op->params[4].flt*20.0f,
                                                    op->params[5].flt);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_TRACE_LAMP: {
                if (ctx.img_pos < 1) break;
                Image *dst = texed_img_push(iw, ih, BLANK);
                texed_raytrace_lamp(dst, &ctx.img[ctx.img_pos-1],
                                    op->params[0].vec.x*iw,
                                    op->params[0].vec.y*ih,
                                    op->params[1].flt,
                                    op->params[2].flt);
                UnloadImage(ctx.img[ctx.img_pos-1]);
                ctx.img[ctx.img_pos-1] = *dst;
                ctx.img_pos--;
            }break;
            case TOP_TRACE_SUN: {
                if (ctx.img_pos < 1) break;
                Image *dst = texed_img_push(iw, ih, BLANK);
                texed_raytrace_sun(dst, &ctx.img[ctx.img_pos-1],
                                   texed_map_value_flt(op->params[0].flt, 0.0f, ZPL_TAU),
                                   texed_map_value_flt(op->params[1].flt, 0.0f, 0.5f),
                                   op->params[2].flt);
                UnloadImage(ctx.img[ctx.img_pos-1]);
                ctx.img[ctx.img_pos-1] = *dst;
                ctx.img_pos--;
            }break;
            default: {
                zpl_printf("unsupported op: %s!\n", op->name);
            }break;
        }
        
        if (ctx.bp_op == i) break;
    }
}

void texed_process_params(void) {
    for (int i = 0; i < zpl_array_count(ctx.ops); i += 1) {
        td_op *op = &ctx.ops[i];
        
        for (int j = 0; j < op->num_params; j += 1) {
            td_param  *p = &op->params[j];
            
            switch (p->kind) {
                case TPARAM_SLIDER:
                case TPARAM_FLOAT: {
                    p->old_flt = p->flt = (float)zpl_str_to_f64(p->str, NULL);
                }break;
                case TPARAM_INT:
                case TPARAM_COORD: {
                    p->i32 = (int32_t)zpl_str_to_i64(p->str, NULL, 10);
                }break;
                case TPARAM_COLOR: {
                    uint32_t color = (uint32_t)zpl_str_to_u64(p->str, NULL, 16);
                    p->old_color = p->color = GetColor(color);
                }break;
                case TPARAM_PAD: {
                    char *str = NULL;
                    p->vec.x = zpl_str_to_f32(p->str, &str);
                    p->vec.y = zpl_str_to_f32(str+1, NULL);
                }break;
                case TPARAM_STRING: {
                    // NOTE(zaklaus): no-op
                }break;
                default: {
                    zpl_printf("%s\n", "unsupported param!");
                }break;
            }
        }
    }
}
