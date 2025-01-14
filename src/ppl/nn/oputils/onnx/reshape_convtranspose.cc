#include "ppl/nn/oputils/onnx/reshape_convtranspose.h"
#include "ppl/nn/runtime/tensor_impl.h"
using namespace ppl::common;
using namespace ppl::nn::common;

namespace ppl { namespace nn { namespace oputils {

RetCode ReshapeConvTranspose(InputOutputInfo* info, const void* arg) {
    auto param = (const ConvTransposeParam*)arg;
    const TensorShape& in_shape0 = info->GetInput<TensorImpl>(0)->GetShape();

    int kernel_h_eff = (param->kernel_shape[0] - 1) * param->dilations[0] + 1;
    int kernel_w_eff = (param->kernel_shape[1] - 1) * param->dilations[1] + 1;
    int src_h = in_shape0.GetDim(2);
    int src_w = in_shape0.GetDim(3);

    int batch = in_shape0.GetDim(0);
    int out_h = param->strides[0] * (src_h - 1) + kernel_h_eff - 2 * param->pads[0];
    int out_w = param->strides[1] * (src_w - 1) + kernel_w_eff - 2 * param->pads[1];

    info->GetOutput<TensorImpl>(0)->GetShape().Reshape(
        {batch, info->GetInput<TensorImpl>(1)->GetShape().GetDim(1), out_h, out_w});
    return RC_SUCCESS;
}

}}} // namespace ppl::nn::oputils
