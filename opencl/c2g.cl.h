static const char* c2g_cl_source =
"#define ANGLE_PRIME 95273                                                     \n"
"#define RADIUS_PRIME 29537                                                    \n"
"                                                                              \n"
"void sample_min_max(const __global   float4 *src_buf,                         \n"
"                                     int     src_width,                       \n"
"                                     int     src_height,                      \n"
"                    const __global   float  *radiuses,                        \n"
"                    const __global   float  *lut_cos,                         \n"
"                    const __global   float  *lut_sin,                         \n"
"                                     int     x,                               \n"
"                                     int     y,                               \n"
"                                     int     radius,                          \n"
"                                     int     samples,                         \n"
"                                     float4 *min,                             \n"
"                                     float4 *max,                             \n"
"                                     int     j,                               \n"
"                                     int     iterations)                      \n"
"{                                                                             \n"
"    float4 best_min;                                                          \n"
"    float4 best_max;                                                          \n"
"    float4 center_pix = *(src_buf + src_width * y + x);                       \n"
"    int i;                                                                    \n"
"                                                                              \n"
"    best_min = center_pix;                                                    \n"
"    best_max = center_pix;                                                    \n"
"                                                                              \n"
"    int angle_no  = (src_width * y + x) * (iterations) *                      \n"
"                       samples + j * samples;                                 \n"
"    int radius_no = angle_no;                                                 \n"
"    angle_no  %= ANGLE_PRIME;                                                 \n"
"    radius_no %= RADIUS_PRIME;                                                \n"
"    for(i=0; i<samples; i++)                                                  \n"
"    {                                                                         \n"
"        int angle;                                                            \n"
"        float rmag;                                                           \n"
"        /* if we've sampled outside the valid image                           \n"
"           area, we grab another sample instead, this                         \n"
"           should potentially work better than mirroring                      \n"
"           or extending the image */                                          \n"
"                                                                              \n"
"         angle = angle_no++;                                                  \n"
"         rmag  = radiuses[radius_no++] * radius;                              \n"
"                                                                              \n"
"         if( angle_no  >= ANGLE_PRIME)                                        \n"
"             angle_no   = 0;                                                  \n"
"         if( radius_no >= RADIUS_PRIME)                                       \n"
"             radius_no  = 0;                                                  \n"
"                                                                              \n"
"         int u = x + rmag * lut_cos[angle];                                   \n"
"         int v = y + rmag * lut_sin[angle];                                   \n"
"                                                                              \n"
"         if(u>=src_width || u <0 || v>=src_height || v<0)                     \n"
"         {                                                                    \n"
"             //--i;                                                           \n"
"             continue;                                                        \n"
"         }                                                                    \n"
"         float4 pixel = *(src_buf + (src_width * v + u));                     \n"
"         if(pixel.w<=0.0f)                                                    \n"
"         {                                                                    \n"
"             //--i;                                                           \n"
"             continue;                                                        \n"
"         }                                                                    \n"
"                                                                              \n"
"         best_min = pixel < best_min ? pixel : best_min;                      \n"
"         best_max = pixel > best_max ? pixel : best_max;                      \n"
"    }                                                                         \n"
"                                                                              \n"
"    (*min).xyz = best_min.xyz;                                                \n"
"    (*max).xyz = best_max.xyz;                                                \n"
"}                                                                             \n"
"                                                                              \n"
"void compute_envelopes(const __global  float4 *src_buf,                       \n"
"                                       int     src_width,                     \n"
"                                       int     src_height,                    \n"
"                       const __global  float  *radiuses,                      \n"
"                       const __global  float  *lut_cos,                       \n"
"                       const __global  float  *lut_sin,                       \n"
"                                       int     x,                             \n"
"                                       int     y,                             \n"
"                                       int     radius,                        \n"
"                                       int     samples,                       \n"
"                                       int     iterations,                    \n"
"                                       float4 *min_envelope,                  \n"
"                                       float4 *max_envelope)                  \n"
"{                                                                             \n"
"    float4 range_sum = 0;                                                     \n"
"    float4 relative_brightness_sum = 0;                                       \n"
"    float4 pixel = *(src_buf + src_width * y + x);                            \n"
"                                                                              \n"
"    int i;                                                                    \n"
"    for(i =0; i<iterations; i++)                                              \n"
"    {                                                                         \n"
"        float4 min,max;                                                       \n"
"        float4 range, relative_brightness;                                    \n"
"                                                                              \n"
"        sample_min_max(src_buf, src_width, src_height,                        \n"
"                        radiuses, lut_cos, lut_sin, x, y,                     \n"
"                        radius,samples,&min,&max,i,iterations);               \n"
"        range = max - min;                                                    \n"
"        relative_brightness = range <= 0.0f ?                                 \n"
"                               0.5f : (pixel - min) / range;                  \n"
"        relative_brightness_sum += relative_brightness;                       \n"
"        range_sum += range;                                                   \n"
"    }                                                                         \n"
"                                                                              \n"
"    float4 relative_brightness = relative_brightness_sum / (float4)(iterations);\n"
"    float4 range = range_sum / (float4)(iterations);                          \n"
"                                                                              \n"
"    if(max_envelope)                                                          \n"
"        *max_envelope = pixel + (1.0f - relative_brightness) * range;         \n"
"                                                                              \n"
"    if(min_envelope)                                                          \n"
"        *min_envelope = pixel - relative_brightness * range;                  \n"
"}                                                                             \n"
"                                                                              \n"
"__kernel void c2g(const __global float4 *src_buf,                             \n"
"                                 int     src_width,                           \n"
"                                 int     src_height,                          \n"
"                  const __global float  *radiuses,                            \n"
"                  const __global float  *lut_cos,                             \n"
"                  const __global float  *lut_sin,                             \n"
"                        __global float2 *dst_buf,                             \n"
"                                 int     radius,                              \n"
"                                 int     samples,                             \n"
"                                 int     iterations)                          \n"
"{                                                                             \n"
"    int gidx = get_global_id(0);                                              \n"
"    int gidy = get_global_id(1);                                              \n"
"                                                                              \n"
"    int x = gidx + radius;                                                    \n"
"    int y = gidy + radius;                                                    \n"
"                                                                              \n"
"    int src_offset = (src_width * y + x);                                     \n"
"    int dst_offset = gidx + get_global_size(0) * gidy;                        \n"
"    float4 min,max;                                                           \n"
"                                                                              \n"
"    compute_envelopes(src_buf, src_width, src_height,                         \n"
"                      radiuses, lut_cos, lut_sin, x, y,                       \n"
"                      radius, samples, iterations, &min, &max);               \n"
"                                                                              \n"
"    float4 pixel = *(src_buf + src_offset);                                   \n"
"                                                                              \n"
"    float nominator=0, denominator=0;                                         \n"
"    float4 t1 = (pixel - min) * (pixel - min);                                \n"
"    float4 t2 = (pixel - max) * (pixel - max);                                \n"
"                                                                              \n"
"    nominator   = t1.x + t1.y + t1.z;                                         \n"
"    denominator = t2.x + t2.y + t2.z;                                         \n"
"                                                                              \n"
"    nominator   = sqrt(nominator);                                            \n"
"    denominator = sqrt(denominator);                                          \n"
"    denominator+= nominator + denominator;                                    \n"
"                                                                              \n"
"    dst_buf[dst_offset].x = (denominator > 0.000f)                            \n"
"                             ? (nominator / denominator) : 0.5f;              \n"
"    dst_buf[dst_offset].y =  src_buf[src_offset].w;                           \n"
"}                                                                             \n"
;