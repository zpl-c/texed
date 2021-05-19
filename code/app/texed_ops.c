static inline
float texed_map_value(float v, float min, float max);

static inline
Image texed_generate_noise(uint32_t seed, int width, int height, float factor, Color color1, Color color2);

static inline
Image texed_generate_cellular(uint32_t seed, int width, int height, int tileSize, Color color1, Color color2, float cell_offset);

static inline
Image texed_generate_perlin(int width, int height, int offsetX, int offsetY, float scale, Color color1, Color color2);

void texed_process_ops(void) {
    for (int i = 0; i <= ctx.img_pos; i+=1)
        UnloadImage(ctx.img[i]);
    ctx.img_pos = -1;
    
    for (int i = 0; i < zpl_array_count(ctx.ops); i += 1) {
        td_op *op = &ctx.ops[i];
        if (op->is_hidden) continue;
        //zpl_printf("processing op: %s ... \n", op->name);
        
        switch (op->kind) {
            case TOP_PUSH_IMAGE:
            case TOP_NEW_IMAGE: {
                texed_img_push(op->params[0].i32, op->params[1].i32, op->params[2].color);
            }break;
            case TOP_POP_IMAGE: {
                texed_img_pop(op->params[0].i32,
                              op->params[1].i32,
                              op->params[2].i32,
                              op->params[3].i32,
                              op->params[4].color,
                              true);
            }break;
            case TOP_DRAW_IMAGE_INSTANCE: {
                texed_img_pop(op->params[0].i32,
                              op->params[1].i32,
                              op->params[2].i32,
                              op->params[3].i32,
                              op->params[4].color,
                              false);
            }break;
            case TOP_DROP_IMAGE: {
                if (ctx.img_pos > 0) {
                    UnloadImage(ctx.img[ctx.img_pos]);
                    ctx.img_pos--;
                }
            }break;
            case TOP_CLONE_IMAGE: {
                Image *oi = &ctx.img[ctx.img_pos];
                texed_img_push(oi->width, oi->height, BLANK);
                Image *di = &ctx.img[ctx.img_pos];
                Rectangle rec = {0, 0, oi->width, oi->height};
                ImageDraw(di, *oi, rec, rec, op->params[0].color);
            }break;
            case TOP_IMAGE_ALPHA_MASK: {
                if (ctx.img_pos == 0) break;
                
                Image *oi = &ctx.img[ctx.img_pos];
                Image *di = &ctx.img[ctx.img_pos-1];
                ImageAlphaMask(di, *oi);
                ctx.img_pos--;
            }break;
            case TOP_IMAGE_ALPHA_MASK_CLEAR: {
                ImageAlphaClear(&ctx.img[ctx.img_pos], op->params[0].color, op->params[1].flt);
            }break;
            case TOP_DRAW_RECT: {
                ImageDrawRectangle(&ctx.img[ctx.img_pos], 
                                   op->params[0].i32,
                                   op->params[1].i32,
                                   op->params[2].i32,
                                   op->params[3].i32,
                                   op->params[4].color);
            }break;
            case TOP_DRAW_LINE: {
                ImageDrawLine(&ctx.img[ctx.img_pos], 
                              op->params[0].i32,
                              op->params[1].i32,
                              op->params[2].i32,
                              op->params[3].i32,
                              op->params[4].color);
            }break;
            case TOP_DITHER: {
                ImageDither(&ctx.img[ctx.img_pos],
                            op->params[0].i32,
                            op->params[1].i32,
                            op->params[2].i32,
                            op->params[3].i32);
            }break;
            case TOP_DRAW_IMAGE: {
                char const *str = zpl_bprintf("art/%s", op->params[0].str);
                if (FileExists(str)) {
                    Image img = LoadImage(str);
                    int x = op->params[1].i32;
                    int y = op->params[2].i32;
                    int w = op->params[3].i32;
                    int h = op->params[4].i32;
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
                    
                    ImageDraw(&ctx.img[ctx.img_pos], img, 
                              (Rectangle){0.0f, 0.0f, img.width, img.height},
                              (Rectangle){x, y, img.width, img.height},
                              op->params[5].color);
                    
                    UnloadImage(img);
                } else {
                    zpl_printf("TOP_LOAD_IMAGE: src %s not found!\n", str);
                }
            }break;
            case TOP_DRAW_TEXT: {
                char const *str = op->params[0].str;
                int x = op->params[1].i32;
                int y = op->params[2].i32;
                int size = op->params[3].i32;
                Color color = op->params[4].color;
                ImageDrawText(&ctx.img[ctx.img_pos], str, x, y, size, color);
            }break;
            case TOP_RESIZE_IMAGE: {
                if (ctx.img[ctx.img_pos].width == 0) break;
                int w = op->params[0].i32;
                int h = op->params[1].i32;
                int mode = op->params[2].i32;
                if (mode) {
                    ImageResize(&ctx.img[ctx.img_pos], w, h);
                } else {
                    ImageResizeNN(&ctx.img[ctx.img_pos], w, h);
                }
            }break;
            case TOP_COLOR_TWEAKS: {
                ImageColorContrast(&ctx.img[ctx.img_pos], texed_map_value(op->params[0].flt, -100.0f, 100.0f));
                ImageColorBrightness(&ctx.img[ctx.img_pos], (int)texed_map_value(op->params[1].flt, -255.0f, 255.0f));
                ImageColorTint(&ctx.img[ctx.img_pos], op->params[2].color);
                
                if (op->params[3].i32) {
                    ImageColorInvert(&ctx.img[ctx.img_pos]);
                }
                if (op->params[4].i32) {
                    ImageColorGrayscale(&ctx.img[ctx.img_pos]);
                }
            }break;
            case TOP_COLOR_REPLACE: {
                ImageColorReplace(&ctx.img[ctx.img_pos], op->params[0].color, op->params[1].color);
            }break;
            case TOP_IMAGE_GRAD_V: {
                Image *dst = &ctx.img[ctx.img_pos];
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageGradientV(w, h, op->params[0].color, op->params[1].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_GRAD_H: {
                Image *dst = &ctx.img[ctx.img_pos];
                int w = dst->width;
                int h = dst->height;
                Image img = GenImageGradientH(w, h, op->params[0].color, op->params[1].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_GRAD_RAD: {
                Image *dst = &ctx.img[ctx.img_pos];
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
                Image *dst = &ctx.img[ctx.img_pos];
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
                Image *dst = &ctx.img[ctx.img_pos];
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
                Image *dst = &ctx.img[ctx.img_pos];
                int w = dst->width;
                int h = dst->height;
                Image img = texed_generate_perlin(w, h, 
                                                  op->params[0].i32,
                                                  op->params[1].i32,
                                                  op->params[2].flt*50.0f,
                                                  op->params[3].color,
                                                  op->params[4].color);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            case TOP_IMAGE_CELLULAR: {
                Image *dst = &ctx.img[ctx.img_pos];
                int w = dst->width;
                int h = dst->height;
                Image img = texed_generate_cellular(op->params[0].u32,
                                                    w, h, 
                                                    op->params[1].i32,
                                                    op->params[2].color,
                                                    op->params[3].color,
                                                    op->params[4].flt*20.0f);
                Rectangle rec = {0, 0, w, h};
                ImageDraw(dst, img, rec, rec, WHITE);
                UnloadImage(img);
            }break;
            default: {
                zpl_printf("unsupported op: %s\n!", op->name);
            }break;
        }
        
        if (op->is_breakpoint) break;
    }
}

void texed_process_params(void) {
    for (int i = 0; i < zpl_array_count(ctx.ops); i += 1) {
        td_op *op = &ctx.ops[i];
        
        for (int j = 0; j < op->num_params; j += 1) {
            td_param *p = &op->params[j];
            
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
                    p->color = GetColor(color);
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

static inline
float texed_map_value(float v, float min, float max) {
    float slope = max-min;
    return min + zpl_round(slope * v);
}

/* This algorithm is mentioned in the ISO C standard, here extended
   for 32 bits.  */
static inline
int _rand_r(unsigned int *seed) {
    unsigned int next = *seed;
    int result;
    
    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next / 65536) % 2048;
    
    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;
    
    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next / 65536) % 1024;
    
    *seed = next;
    
    return result;
}

static inline
Image texed_generate_noise(uint32_t seed, int width, int height, float factor, Color color1, Color color2) {
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
    
    for (int i = 0; i < width*height; i++) {
        if ((_rand_r(&seed)%99) < (int)(factor*100.0f)) pixels[i] = color1;
        else pixels[i] = color2;
    }
    
    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    return image;
}

static inline
Image texed_generate_cellular(uint32_t seed, int width, int height, int tileSize, Color color1, Color color2, float cell_offset)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
    
    int seedsPerRow = width/tileSize;
    int seedsPerCol = height/tileSize;
    int seedsCount = seedsPerRow*seedsPerCol;
    
    Vector2 *seeds = (Vector2 *)RL_MALLOC(seedsCount*sizeof(Vector2));
    
    for (int i = 0; i < seedsCount; i++)
    {
        int y = (i/seedsPerRow)*tileSize + _rand_r(&seed)%(tileSize - 1);
        int x = (i%seedsPerRow)*tileSize + _rand_r(&seed)%(tileSize - 1);
        seeds[i] = (Vector2){ (float)x, (float)y};
    }
    
    for (int y = 0; y < height; y++)
    {
        int tileY = y/tileSize;
        
        for (int x = 0; x < width; x++)
        {
            int tileX = x/tileSize;
            
            float minDistance = (float)strtod("Inf", NULL);
            
            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;
                
                for (int j = -1; j < 2; j++)
                {
                    if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;
                    
                    Vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];
                    
                    float dist = (float)hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
                    minDistance = (float)fmin(minDistance, dist);
                }
            }
            
            minDistance = zpl_max(cell_offset, minDistance);
            
            float intensity = (minDistance-cell_offset)/(float)tileSize;
            if (intensity > 1.0f) intensity = 1.0f;
            
            pixels[y*width + x] = ColorLerp(color1, color2, (float)intensity);
        }
    }
    
    RL_FREE(seeds);
    
    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    return image;
}

float stb_perlin_fbm_noise3(float x, float y, float z, float lacunarity, float gain, int octaves);

Image texed_generate_perlin(int width, int height, int offsetX, int offsetY, float scale, Color color1, Color color2) {
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = (float)(x + offsetX)*scale/(float)width;
            float ny = (float)(y + offsetY)*scale/(float)height;
            
            // Typical values to start playing with:
            //   lacunarity = ~2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
            //   gain       =  0.5   -- relative weighting applied to each successive octave
            //   octaves    =  6     -- number of "octaves" of noise3() to sum
            
            // NOTE: We need to translate the data from [-1..1] to [0..1]
            float p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6) + 1.0f)/2.0f;
            
            pixels[y*width + x] = ColorLerp(color1, color2, p);
        }
    }
    
    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    
    return image;
}
