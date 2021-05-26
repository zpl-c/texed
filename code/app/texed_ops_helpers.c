#include "raymath.h"

float texed_map_value(float v, float min, float max) {
    float slope = max-min;
    return min + zpl_round(slope * v);
}

float texed_map_value_flt(float v, float min, float max) {
    float slope = max-min;
    return min + slope * v;
}

/* This algorithm is mentioned in the ISO C standard, here extended
   for 32 bits.  */
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

Image texed_generate_cellular(uint32_t seed, int width, int height, int tileSize, Color color1, Color color2, float cell_offset, float density)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
    
    int seedsPerRow = width/tileSize;
    int seedsPerCol = height/tileSize;
    int seedsCount = seedsPerRow*seedsPerCol;
    
    Vector2 *seeds = (Vector2 *)RL_MALLOC(seedsCount*sizeof(Vector2));
    
    for (int i = 0; i < seedsCount; i++)
    {
        if (_rand_r(&seed)%99 > density*100.0f) continue;
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
            
            float minDistance = ZPL_F32_MAX;
            
            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;
                
                for (int j = -1; j < 2; j++)
                {
                    if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;
                    
                    Vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];
                    
                    float dist = (float)hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
                    minDistance = zpl_min(minDistance, dist);
                }
            }
            
            minDistance = zpl_max(cell_offset, minDistance);
            float intensity = zpl_min(((minDistance-cell_offset)/(float)tileSize), 1.0f);
            pixels[y*width + x] = ColorLerp(color1, color2, intensity);
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

Image texed_generate_perlin(int width, int height, int offsetX, int offsetY, float scale, Color color1, Color color2, float lacunarity, float gain, int octaves) {
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
            float p = (stb_perlin_fbm_noise3(nx, ny, 1.0f, lacunarity, gain, octaves) + 1.0f)/2.0f;
            
            pixels[y*width + x] = ColorLerp(color1, color2, zpl_clamp(p, 0.0f, 1.0f));
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

void texed_filter_image(Color *pixels, int32_t w, int32_t h, double *filter, int32_t filter_w, int32_t filter_h, float weight, float bias)
{
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int r=0, g=0, b=0, a=0;
            for (int fy = 0; fy < filter_h; fy++) {
                for (int fx = 0; fx < filter_w; fx++) {
                    int img_x = (x - filter_w / 2 + fx + w) % w;
                    int img_y = (y - filter_h / 2 + fy + h) % h;
                    
                    r += pixels[img_y * w + img_x].r * filter[fy*filter_w + fx];
                    g += pixels[img_y * w + img_x].g * filter[fy*filter_w + fx];
                    b += pixels[img_y * w + img_x].b * filter[fy*filter_w + fx];
                    a += pixels[img_y * w + img_x].a * filter[fy*filter_w + fx];
                }
            }
            
            pixels[y * w + x].r = zpl_min(zpl_max((int32_t)(weight * r + bias), 0), 255);
            pixels[y * w + x].g = zpl_min(zpl_max((int32_t)(weight * g + bias), 0), 255);
            pixels[y * w + x].b = zpl_min(zpl_max((int32_t)(weight * b + bias), 0), 255);
            pixels[y * w + x].a = zpl_min(zpl_max((int32_t)(weight * a + bias), 0), 255);
        }
    }
}

void texed_blur_image(Image *image, int32_t amount)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;
    
    Color *pixels = LoadImageColors(*image);
    
    static double filter[9] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };
    
    while (amount--)
        texed_filter_image(pixels, image->width, image->height, &filter[0], 3, 3, 1.0f/9.0f, 0.0f);
    
    int format = image->format;
    RL_FREE(image->data);
    //RL_FREE(filter);
    
    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    
    ImageFormat(image, format);
}

void texed_draw_line(Image *dst, int x0, int y0, int x1, int y1, Color color) {
    int dx = (int)abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = (int)-abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    while (1) {
        ImageDrawPixel(dst, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2*err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        } 
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void texed_raytrace_lamp(Image *dst, Image *src, int px, int py, float radius, float threshold) {
    float max_dist = Vector2LengthSqr((Vector2){dst->width, dst->height}) * radius;
    Color *pixels = LoadImageColors(*src);
    
    for (int y = 0; y < dst->height; y++) {
        for (int x = 0; x < dst->width; x++) {
            if (Vector2LengthSqr(Vector2Subtract((Vector2){px, py}, (Vector2){x, y})) > max_dist)
                continue;
            int x0 = x;
            int y0 = y;
            int dx = (int)abs(px-x0);
            int sx = x0<px ? 1 : -1;
            int dy = (int)-abs(py-y0);
            int sy = y0<py ? 1 : -1;
            int err = dx+dy;
            while (1) {
                bool hit = ColorToHSV(pixels[y0 * src->width + x0]).z > threshold;
                if (x0 == x && y0 == y && hit) {
                    break;
                } else if (hit) {
                    ImageDrawPixel(dst, x, y, WHITE);
                    break;
                }
                if (x0 == px && y0 == py) break;
                int e2 = 2*err;
                if (e2 >= dy) {
                    err += dy;
                    x0 += sx;
                } else if (e2 <= dx) {
                    err += dx;
                    y0 += sy;
                }
            }
        }
    }
    
    RL_FREE(pixels);
}


void texed_raytrace_sun(Image *dst, Image *src, float azimuth, float pitch, float threshold) {
    float max_dist = Vector2Length((Vector2){dst->width, dst->height}) * pitch;
    Color *pixels = LoadImageColors(*src);
    Vector2 dir = (Vector2){-cos(azimuth), -sin(azimuth)};
    
    for (int y = 0; y < dst->height; y++) {
        for (int x = 0; x < dst->width; x++) {
            int px = x + (int)(dir.x*max_dist);
            int py = y + (int)(dir.y*max_dist);
            int x0 = x;
            int y0 = y;
            int sx = x0<px ? 1 : -1;
            int sy = y0<py ? 1 : -1;
            int dx = (int)abs(px-x0);
            int dy = (int)-abs(py-y0);
            int err = dx+dy;
            while (1) {
                if (x0 < 0 || x0 >= src->width) break;
                if (y0 < 0 || y0 >= src->height) break;
                bool hit = ColorToHSV(pixels[y0 * src->width + x0]).z > threshold;
                if (x0 == x && y0 == y && hit) {
                    break;
                } else if (hit) {
                    ImageDrawPixel(dst, x, y, WHITE);
                    break;
                }
                if (x0 == px && y0 == py) break;
                int e2 = 2*err;
                if (e2 >= dy) {
                    err += dy;
                    x0 += sx;
                } else if (e2 <= dx) {
                    err += dx;
                    y0 += sy;
                }
            }
        }
    }
    
    RL_FREE(pixels);
}