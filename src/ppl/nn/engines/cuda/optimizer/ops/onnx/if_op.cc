#include "ppl/nn/engines/cuda/optimizer/ops/onnx/if_op.h"

#include "ppl/nn/common/logger.h"
#include "ppl/nn/engines/common/onnx/if_kernel.h"

using namespace std;
using namespace ppl::common;
using namespace ppl::nn::common;

namespace ppl { namespace nn { namespace cuda {

RetCode IfOp::Init(const OptKernelOptions& options) {
    infer_dims_func_ = [this](InputOutputInfo* info) -> RetCode {
        for (uint32_t i = 0; i < info->GetOutputCount(); ++i) {
            auto out_shape = &info->GetOutput<TensorImpl>(i)->GetShape();
            if (out_shape->GetDataFormat() == DATAFORMAT_UNKNOWN) {
                out_shape->Reshape({1, 3, 128, 128});
                out_shape->SetDataFormat(DATAFORMAT_NDARRAY);
            }
        }
        return RC_SUCCESS;
    };
    infer_type_func_ = [this](InputOutputInfo* info, datatype_t) -> RetCode {
        for (uint32_t i = 0; i < info->GetOutputCount(); ++i) {
            auto out_shape = &info->GetOutput<TensorImpl>(i)->GetShape();
            out_shape->SetDataType(DATATYPE_UNKNOWN);
        }
        return RC_SUCCESS;
    };

    auto node = GetNode();
    auto graph_data = options.graph->data.get();
    auto attr_ref = graph_data->attrs.find(node->GetId());
    if (attr_ref == graph_data->attrs.end()) {
        LOG(ERROR) << "cannot find attr for if kernel[" << node->GetName() << "]";
        return RC_NOT_FOUND;
    }

    auto if_param = static_cast<ppl::nn::onnx::IfParam*>(attr_ref->second.get());
    return op_.Init(options.resource, if_param);
}

KernelImpl* IfOp::CreateKernelImpl() const {
    return op_.CreateKernelImpl();
}

}}} // namespace ppl::nn::cuda
