#include "ppl/nn/engines/cuda/optimizer/ops/onnx/batch_normalization_op.h"

#include "ppl/nn/common/logger.h"
#include "ppl/nn/engines/cuda/kernels/onnx/batch_normalization_kernel.h"
#include "ppl/nn/oputils/onnx/reshape_batch_normalization.h"

using namespace std;
using namespace ppl::common;
using namespace ppl::nn::common;

namespace ppl { namespace nn { namespace cuda {

RetCode BatchNormalizationOp::Init(const OptKernelOptions& options) {
    auto status = GenericLoadParam<BatchNormalizationParam>(options, &param_);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "load param failed: " << GetRetCodeStr(status);
        return status;
    }

    infer_type_func_ = [this](InputOutputInfo* info, datatype_t type) -> RetCode {
        auto in_shape = &info->GetInput<TensorImpl>(0)->GetShape();
        type = in_shape->GetDataType();
        return InferDefaultType(info, type);
    };

    infer_dims_func_ = [this](InputOutputInfo* info) -> RetCode {
        return oputils::ReshapeBatchNormalization(info, nullptr);
    };

    return RC_SUCCESS;
}

RetCode BatchNormalizationOp::Finalize(const OptKernelOptions& options) {
    auto status = SetCommonParam(options);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "load common param failed: " << GetRetCodeStr(status);
        return status;
    }

    return RC_SUCCESS;
}

KernelImpl* BatchNormalizationOp::CreateKernelImpl() const {
    return CreateKernelImplWithParam<BatchNormalizationKernel>(&param_);
}

}}} // namespace ppl::nn::cuda
