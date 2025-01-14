#include "ppl/kernel/x86/common/internal_include.h"
#include "ppl/kernel/x86/common/mmcv_gridsample/mmcv_gridsample_common.h"

#include <vector>

namespace ppl { namespace kernel { namespace x86 {

template <bool align_corners, grid_sampler_padding padding_mode>
ppl::common::RetCode mmcv_gridsample_linear_ndarray_fp32_kernel(
    const ppl::nn::TensorShape *src_shape,
    const ppl::nn::TensorShape *grid_shape,
    const float *src,
    const float *grid,
    float *dst)
{
    const int64_t batch    = src_shape->GetDim(0);
    const int64_t channels = src_shape->GetDim(1);
    const int64_t src_h    = src_shape->GetDim(2);
    const int64_t src_w    = src_shape->GetDim(3);
    const int64_t dst_h    = grid_shape->GetDim(1);
    const int64_t dst_w    = grid_shape->GetDim(2);

    std::vector<float> ixs_buffer(dst_h * dst_w * PPL_OMP_MAX_THREADS());
    std::vector<float> iys_buffer(dst_h * dst_w * PPL_OMP_MAX_THREADS());

    PRAGMA_OMP_PARALLEL_FOR()
    for (int64_t n = 0; n < batch; n++) {
        float *ixs = (float *)ixs_buffer.data() + dst_h * dst_w * PPL_OMP_THREAD_ID();
        float *iys = (float *)iys_buffer.data() + dst_h * dst_w * PPL_OMP_THREAD_ID();

        const float *l_grid = grid + n * dst_h * dst_w * 2;
        for (int64_t h = 0; h < dst_h; h++) {
            for (int64_t w = 0; w < dst_w; w++) {
                float x = l_grid[(h * dst_w + w) * 2 + 0];
                float y = l_grid[(h * dst_w + w) * 2 + 1];

                ixs[h * dst_w + w] = grid_sampler_compute_source_index<float, align_corners, padding_mode>(x, src_w);
                iys[h * dst_w + w] = grid_sampler_compute_source_index<float, align_corners, padding_mode>(y, src_h);
            }
        }

        for (int64_t c = 0; c < channels; c++) {
            const float *l_src = src + (n * channels + c) * src_h * src_w;
            float *l_dst       = dst + (n * channels + c) * dst_h * dst_w;
            for (int64_t h = 0; h < dst_h; h++) {
                for (int64_t w = 0; w < dst_w; w++) {
                    float ix = ixs[h * dst_w + w];
                    float iy = iys[h * dst_w + w];

                    int64_t x0 = std::floor(ix);
                    int64_t y0 = std::floor(iy);
                    int64_t x1 = x0 + 1;
                    int64_t y1 = y0 + 1;

                    float x1_lambda = ix - x0;
                    float y1_lambda = iy - y0;
                    float x0_lambda = x1 - ix;
                    float y0_lambda = y1 - iy;

                    l_dst[h * dst_w + w] = (within_bounds_2d(y0, x0, src_h, src_w) ? l_src[y0 * src_w + x0] * y0_lambda * x0_lambda : 0) +
                                           (within_bounds_2d(y0, x1, src_h, src_w) ? l_src[y0 * src_w + x1] * y0_lambda * x1_lambda : 0) +
                                           (within_bounds_2d(y1, x0, src_h, src_w) ? l_src[y1 * src_w + x0] * y1_lambda * x0_lambda : 0) +
                                           (within_bounds_2d(y1, x1, src_h, src_w) ? l_src[y1 * src_w + x1] * y1_lambda * x1_lambda : 0);
                }
            }
        }
    }

    return ppl::common::RC_SUCCESS;
}

ppl::common::RetCode mmcv_gridsample_linear_ndarray_fp32(
    const ppl::nn::TensorShape *src_shape,
    const ppl::nn::TensorShape *grid_shape,
    const float *src,
    const float *grid,
    const bool align_corners,
    const int64_t padding_mode,
    float *dst)
{
    if (align_corners) {
        if (padding_mode == ZEROS) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<true, ZEROS>(src_shape, grid_shape, src, grid, dst);
        } else if (padding_mode == BORDER) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<true, BORDER>(src_shape, grid_shape, src, grid, dst);
        } else if (padding_mode == REFLECTION) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<true, REFLECTION>(src_shape, grid_shape, src, grid, dst);
        }
    } else {
        if (padding_mode == ZEROS) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<false, ZEROS>(src_shape, grid_shape, src, grid, dst);
        } else if (padding_mode == BORDER) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<false, BORDER>(src_shape, grid_shape, src, grid, dst);
        } else if (padding_mode == REFLECTION) {
            return mmcv_gridsample_linear_ndarray_fp32_kernel<false, REFLECTION>(src_shape, grid_shape, src, grid, dst);
        }
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}}; // namespace ppl::kernel::x86
