static inline
void DrawAABB(zpl_aabb2 rect, Color color) {
    DrawRectangleEco(rect.min.x, rect.min.y,
                     rect.max.x-rect.min.x,
                     rect.max.y-rect.min.y,
                     color);
}

static inline
Rectangle aabb2_ray(zpl_aabb2 r) {
    return (Rectangle) {
        .x = r.min.x,
        .y = r.min.y,
        .width = r.max.x-r.min.x,
        .height = r.max.y-r.min.y
    };
}
