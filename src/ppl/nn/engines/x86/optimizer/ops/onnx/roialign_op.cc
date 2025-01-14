#include "ppl/nn/engines/x86/optimizer/ops/onnx/roialign_op.h"
#include "ppl/nn/engines/x86/kernels/onnx/roialign_kernel.h"
#include "ppl/nn/oputils/onnx/reshape_roialign.h"
#include "ppl/nn/common/logger.h"
using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace x86 {

RetCode ROIAlignOp::Init(const OptKernelOptions& options) {
    auto status = GenericLoadParam(options, &param_);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "load param failed: " << GetRetCodeStr(status);
        return status;
    }

    infer_dims_func_ = [this](InputOutputInfo* info) -> RetCode {
        auto ret = oputils::ReshapeROIAlign(info, param_.get());
        if (ret != RC_SUCCESS) {
            return ret;
        }

        if (info->GetInput<TensorImpl>(0)->GetShape().GetDataType() != DATATYPE_FLOAT32 ||
            info->GetInput<TensorImpl>(1)->GetShape().GetDataType() != DATATYPE_FLOAT32) {
            LOG(ERROR) << "input tensor x & rois only support fp32 now.";
            return RC_UNSUPPORTED;
        }
        if (info->GetInput<TensorImpl>(2)->GetShape().GetDataType() != DATATYPE_INT64) {
            LOG(ERROR) << "input tensor batch_indices must be int64.";
            return RC_INVALID_VALUE;
        }
        if (info->GetInput<TensorImpl>(1)->GetShape().GetDataFormat() != DATAFORMAT_NDARRAY ||
            info->GetInput<TensorImpl>(2)->GetShape().GetDataFormat() != DATAFORMAT_NDARRAY) {
            LOG(ERROR) << "only support ndarray now.";
            return RC_UNSUPPORTED;
        }
        return RC_SUCCESS;
    };

    infer_type_func_ = GenericInferType;

    return RC_SUCCESS;
}

RetCode ROIAlignOp::SelectFormat(const InputOutputInfo& info, vector<dataformat_t>* selected_input_formats,
                                 vector<dataformat_t>* selected_output_formats) {
    if (info.GetInput<TensorImpl>(0)->GetShape().GetDataFormat() == DATAFORMAT_N16CX) {
        selected_input_formats->at(0) = DATAFORMAT_N16CX;
        selected_output_formats->at(0) = DATAFORMAT_N16CX;
    }
    return RC_SUCCESS;
}

KernelImpl* ROIAlignOp::CreateKernelImpl() const {
    return CreateKernelImplWithParam<ROIAlignKernel>(param_.get());
}

}}} // namespace ppl::nn::x86
