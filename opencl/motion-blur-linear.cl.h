static const char* motion_blur_linear_cl_source =
"int CLAMP(int val,int lo,int hi)                                              \n"
"{                                                                             \n"
"    return (val < lo) ? lo : ((hi < val) ? hi : val);                         \n"
"}                                                                             \n"
"                                                                              \n"
"float4 get_pixel_color(const __global float4 *in_buf,                         \n"
"                       int     rect_width,                                    \n"
"                       int     rect_height,                                   \n"
"                       int     rect_x,                                        \n"
"                       int     rect_y,                                        \n"
"                       int     x,                                             \n"
"                       int     y)                                             \n"
"{                                                                             \n"
"    int ix = x - rect_x;                                                      \n"
"    int iy = y - rect_y;                                                      \n"
"                                                                              \n"
"    ix = CLAMP(ix, 0, rect_width-1);                                          \n"
"    iy = CLAMP(iy, 0, rect_height-1);                                         \n"
"                                                                              \n"
"    return in_buf[iy * rect_width + ix];                                      \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void motion_blur_linear(const __global float4 *src_buf,              \n"
"                                 int     src_width,                           \n"
"                                 int     src_height,                          \n"
"                                 int     src_x,                               \n"
"                                 int     src_y,                               \n"
"                                 __global float4 *dst_buf,                    \n"
"                                 int     dst_x,                               \n"
"                                 int     dst_y,                               \n"
"                                 int     num_steps,                           \n"
"                                 float   offset_x,                            \n"
"                                 float   offset_y)                            \n"
"{                                                                             \n"
"    int gidx = get_global_id(0);                                              \n"
"    int gidy = get_global_id(1);                                              \n"
"                                                                              \n"
"    float4 sum = 0.0f;                                                        \n"
"    int px = gidx + dst_x;                                                    \n"
"    int py = gidy + dst_y;                                                    \n"
"                                                                              \n"
"    for(int step = 0; step < num_steps; ++step)                               \n"
"    {                                                                         \n"
"        float t = num_steps == 1 ? 0.0f :                                     \n"
"            step / (float)(num_steps - 1) - 0.5f;                             \n"
"                                                                              \n"
"        float xx = px + t * offset_x;                                         \n"
"        float yy = py + t * offset_y;                                         \n"
"                                                                              \n"
"        int   ix = (int)floor(xx);                                            \n"
"        int   iy = (int)floor(yy);                                            \n"
"                                                                              \n"
"        float dx = xx - floor(xx);                                            \n"
"        float dy = yy - floor(yy);                                            \n"
"                                                                              \n"
"        float4 mixy0,mixy1,pix0,pix1,pix2,pix3;                               \n"
"                                                                              \n"
"        pix0 = get_pixel_color(src_buf, src_width,                            \n"
"            src_height, src_x, src_y, ix,   iy);                              \n"
"        pix1 = get_pixel_color(src_buf, src_width,                            \n"
"            src_height, src_x, src_y, ix+1, iy);                              \n"
"        pix2 = get_pixel_color(src_buf, src_width,                            \n"
"            src_height, src_x, src_y, ix,   iy+1);                            \n"
"        pix3 = get_pixel_color(src_buf, src_width,                            \n"
"            src_height, src_x, src_y, ix+1, iy+1);                            \n"
"                                                                              \n"
"        mixy0 = dy * (pix2 - pix0) + pix0;                                    \n"
"        mixy1 = dy * (pix3 - pix1) + pix1;                                    \n"
"                                                                              \n"
"        sum  += dx * (mixy1 - mixy0) + mixy0;                                 \n"
"    }                                                                         \n"
"                                                                              \n"
"    dst_buf[gidy * get_global_size(0) + gidx] =                               \n"
"        sum / (float4)(num_steps);                                            \n"
"}                                                                             \n"
;
