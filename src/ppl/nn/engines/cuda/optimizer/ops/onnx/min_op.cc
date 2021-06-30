#include "ppl/nn/engines/cuda/optimizer/ops/onnx/min_op.h"

#include "ppl/nn/common/logger.h"
#include "ppl/nn/engines/cuda/kernels/onnx/min_kernel.h"
#include "ppl/nn/oputils/onnx/reshape_max.h"

using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace cuda {

RetCode MinOp::Init(const OptKernelOptions& options) {
    infer_type_func_ = [this](InputOutputInfo* info, datatype_t type) -> RetCode {
        return type != DATATYPE_UNKNOWN ? InferDefaultType(info, type) : InferInheritedType(info);
    };

    infer_dims_func_ = [this](InputOutputInfo* info) -> RetCode {
        return oputils::ReshapeMax(info, nullptr);
    };

    return RC_SUCCESS;
}

RetCode MinOp::Finalize(const OptKernelOptions& options) {
    auto status = SetCommonParam(options);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "load common param failed: " << GetRetCodeStr(status);
        return status;
    }

    return RC_SUCCESS;
}

KernelImpl* MinOp::CreateKernelImpl() const {
    return CreateKernelImplWithoutParam<MinKernel>();
}

}}} // namespace ppl::nn::cuda