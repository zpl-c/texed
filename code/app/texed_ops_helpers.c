float texed_map_value(float v, float min, float max) {
    float slope = max-min;
    return min + zpl_round(slope * v);
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
