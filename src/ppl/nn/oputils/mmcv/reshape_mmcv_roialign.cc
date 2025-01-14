#include "ppl/nn/oputils/mmcv/reshape_mmcv_roialign.h"
#include "ppl/nn/runtime/tensor_impl.h"
using namespace ppl::common;
using namespace ppl::nn::common;

namespace ppl { namespace nn { namespace oputils {

RetCode ReshapeMMCVROIAlign(InputOutputInfo* info, const void* arg) {
    if (info->GetInputCount() != 2 || info->GetOutputCount() != 1) {
        return RC_INVALID_VALUE;
    }

    auto param = (const MMCVROIAlignParam*)arg;

    auto input = &info->GetInput<TensorImpl>(0)->GetShape();
    auto rois = &info->GetInput<TensorImpl>(1)->GetShape();
    auto output = &info->GetOutput<TensorImpl>(0)->GetShape();

    if (input->GetDimCount() != 4) {
        return RC_INVALID_VALUE;
    }

    if (rois->GetDimCount() != 2 || rois->GetDim(1) != 5) {
        return RC_INVALID_VALUE;
    }

    const int32_t num_rois = rois->GetDim(0);
    const int32_t channels = input->GetDim(1);

    output->Reshape({num_rois, channels, param->aligned_height, param->aligned_width});
    return RC_SUCCESS;
}

}}} // namespace ppl::nn::oputils
