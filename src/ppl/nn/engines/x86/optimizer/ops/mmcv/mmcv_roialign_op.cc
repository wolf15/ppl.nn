#include "ppl/nn/engines/x86/optimizer/ops/mmcv/mmcv_roialign_op.h"
#include "ppl/nn/engines/x86/kernels/mmcv/mmcv_roialign_kernel.h"
#include "ppl/nn/oputils/mmcv/reshape_mmcv_roialign.h"
#include "ppl/nn/common/logger.h"
using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace x86 {

RetCode MMCVROIAlignOp::Init(const OptKernelOptions& options) {
    auto status = GenericLoadParam(options, &param_);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "load param failed: " << GetRetCodeStr(status);
        return status;
    }

    infer_dims_func_ = [this](InputOutputInfo* info) -> RetCode {
        auto ret = oputils::ReshapeMMCVROIAlign(info, param_.get());
        if (ret != RC_SUCCESS) {
            return ret;
        }

        auto input0 = info->GetInput<TensorImpl>(0);
        auto input1 = info->GetInput<TensorImpl>(1);
        if (input0->GetShape().GetDataType() != DATATYPE_FLOAT32 ||
            input1->GetShape().GetDataType() != DATATYPE_FLOAT32) {
            LOG(ERROR) << "input tensor x & rois only support fp32 now.";
            return RC_UNSUPPORTED;
        }
        if (input1->GetShape().GetDataFormat() != DATAFORMAT_NDARRAY) {
            LOG(ERROR) << "only support ndarray now.";
            return RC_UNSUPPORTED;
        }
        return RC_SUCCESS;
    };

    infer_type_func_ = GenericInferType;

    return RC_SUCCESS;
}

RetCode MMCVROIAlignOp::SelectFormat(const InputOutputInfo& info, vector<dataformat_t>* selected_input_formats,
                                     vector<dataformat_t>* selected_output_formats) {
    if (info.GetInput<TensorImpl>(0)->GetShape().GetDataFormat() == DATAFORMAT_N16CX) {
        selected_input_formats->at(0) = DATAFORMAT_N16CX;
        selected_output_formats->at(0) = DATAFORMAT_N16CX;
    }
    return RC_SUCCESS;
}

KernelImpl* MMCVROIAlignOp::CreateKernelImpl() const {
    return CreateKernelImplWithParam<MMCVROIAlignKernel>(param_.get());
}

}}} // namespace ppl::nn::x86
