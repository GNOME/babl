static const char* noise_reduction_cl_source =
"#define NEIGHBOURS 8                                                          \n"
"#define AXES       (NEIGHBOURS/2)                                             \n"
"                                                                              \n"
"#define POW2(a) ((a)*(a))                                                     \n"
"                                                                              \n"
"#define GEN_METRIC(before, center, after) POW2((center) * (float4)(2.0f) - (before) - (after))\n"
"                                                                              \n"
"#define BAIL_CONDITION(new,original) ((new) < (original))                     \n"
"                                                                              \n"
"#define SYMMETRY(a)  (NEIGHBOURS - (a) - 1)                                   \n"
"                                                                              \n"
"#define O(u,v) (((u)+((v) * (src_stride))))                                   \n"
"                                                                              \n"
"__kernel void noise_reduction_cl (__global       float4 *src_buf,             \n"
"                                  int src_stride,                             \n"
"                                  __global       float4 *dst_buf,             \n"
"                                  int dst_stride)                             \n"
"{                                                                             \n"
"    int gidx = get_global_id(0);                                              \n"
"    int gidy = get_global_id(1);                                              \n"
"                                                                              \n"
"    __global float4 *center_pix = src_buf + (gidy + 1) * src_stride + gidx + 1;\n"
"    int dst_offset = dst_stride * gidy + gidx;                                \n"
"                                                                              \n"
"    int offsets[NEIGHBOURS] = {                                               \n"
"        O(-1, -1), O( 0, -1), O( 1, -1),                                      \n"
"        O(-1,  0),            O( 1,  0),                                      \n"
"        O(-1,  1), O( 0,  1), O( 1,  1)                                       \n"
"    };                                                                        \n"
"                                                                              \n"
"    float4 sum;                                                               \n"
"    int4   count;                                                             \n"
"    float4 cur;                                                               \n"
"    float4 metric_reference[AXES];                                            \n"
"                                                                              \n"
"    for (int axis = 0; axis < AXES; axis++)                                   \n"
"      {                                                                       \n"
"        float4 before_pix = *(center_pix + offsets[axis]);                    \n"
"        float4 after_pix  = *(center_pix + offsets[SYMMETRY(axis)]);          \n"
"        metric_reference[axis] = GEN_METRIC (before_pix, *center_pix, after_pix);\n"
"      }                                                                       \n"
"                                                                              \n"
"    cur = sum = *center_pix;                                                  \n"
"    count = 1;                                                                \n"
"                                                                              \n"
"    for (int direction = 0; direction < NEIGHBOURS; direction++)              \n"
"      {                                                                       \n"
"        float4 pix   = *(center_pix + offsets[direction]);                    \n"
"        float4 value = (pix + cur) * (0.5f);                                  \n"
"        int    axis;                                                          \n"
"        int4   mask = {1, 1, 1, 0};                                           \n"
"                                                                              \n"
"        for (axis = 0; axis < AXES; axis++)                                   \n"
"          {                                                                   \n"
"            float4 before_pix = *(center_pix + offsets[axis]);                \n"
"            float4 after_pix  = *(center_pix + offsets[SYMMETRY(axis)]);      \n"
"                                                                              \n"
"            float4 metric_new = GEN_METRIC (before_pix,                       \n"
"                                            value,                            \n"
"                                            after_pix);                       \n"
"            mask = BAIL_CONDITION (metric_new, metric_reference[axis]) & mask;\n"
"          }                                                                   \n"
"        sum   += mask >0 ? value : (float4)(0.0);                             \n"
"        count += mask >0 ? 1     : 0;                                         \n"
"      }                                                                       \n"
"    dst_buf[dst_offset]   = (sum/convert_float4(count));                      \n"
"    dst_buf[dst_offset].w = cur.w;                                            \n"
"}                                                                             \n"
"__kernel void transfer(__global float4 * in,                                  \n"
"              int               in_width,                                     \n"
"              __global float4 * out)                                          \n"
"{                                                                             \n"
"    int gidx = get_global_id(0);                                              \n"
"    int gidy = get_global_id(1);                                              \n"
"    int width = get_global_size(0);                                           \n"
"    out[gidy * width + gidx] = in[gidy * in_width + gidx];                    \n"
"}                                                                             \n"
;