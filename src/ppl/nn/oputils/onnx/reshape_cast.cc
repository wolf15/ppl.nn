#include "ppl/nn/oputils/onnx/reshape_cast.h"
#include "ppl/nn/runtime/tensor_impl.h"
using namespace ppl::common;
using namespace ppl::nn::common;

namespace ppl { namespace nn { namespace oputils {

RetCode ReshapeCast(InputOutputInfo* info, const void* arg) {
    if (info->GetInputCount() != 1 || info->GetOutputCount() != 1) {
        return RC_INVALID_VALUE;
    }

    auto param = (const CastParam*)arg;
    if (param->to == DATATYPE_UNKNOWN) {
        return RC_UNSUPPORTED;
    }

    auto input = &info->GetInput<TensorImpl>(0)->GetShape();
    auto output = &info->GetOutput<TensorImpl>(0)->GetShape();

    output->SetDataType(param->to);
    if (input->IsScalar()) {
        output->ReshapeAsScalar();
    } else {
        output->Reshape(input->GetDims(), input->GetDimCount());
    }
    return RC_SUCCESS;
}

}}} // namespace ppl::nn::oputils
