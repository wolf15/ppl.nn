#include "ppl/nn/engines/x86/kernels/onnx/expand_kernel.h"

#include "ppl/kernel/x86/fp32/expand.h"
#include "ppl/kernel/x86/int64/expand.h"

namespace ppl { namespace nn { namespace x86 {

bool ExpandKernel::CanDoExecute(const KernelExecContext& ctx) const {
    auto input = ctx.GetInput<TensorImpl>(0);
    auto shape = ctx.GetInput<TensorImpl>(1);
    auto output = ctx.GetOutput<TensorImpl>(0);
    if (input->GetShape().GetBytesIncludingPadding() == 0 || shape->GetShape().GetBytesIncludingPadding() == 0 ||
        output->GetShape().GetBytesIncludingPadding() == 0) {
        return false;
    }
    return true;
}

ppl::common::RetCode ExpandKernel::DoExecute(KernelExecContext* ctx) {
    auto input = ctx->GetInput<TensorImpl>(0);
    auto shape = ctx->GetInput<TensorImpl>(1);
    auto output = ctx->GetOutput<TensorImpl>(0);

    PPLNN_X86_DEBUG_TRACE("Op: %s\n", GetName().c_str());
    PPLNN_X86_DEBUG_TRACE("Input [input]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(input);
    PPLNN_X86_DEBUG_TRACE("Input [shape]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(shape);
    PPLNN_X86_DEBUG_TRACE("Output [output]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(output);
    PPLNN_X86_DEBUG_TRACE("isa: %u\n", GetISA());

    const ppl::common::datatype_t data_type = output->GetShape().GetDataType();
    if (data_type == ppl::common::DATATYPE_FLOAT32) {
        return kernel::x86::expand_ndarray_fp32(&input->GetShape(), &output->GetShape(), input->GetBufferPtr<float>(),
                                                output->GetBufferPtr<float>());
    } else if (data_type == ppl::common::DATATYPE_INT64) {
        return kernel::x86::expand_ndarray_int64(&input->GetShape(), &output->GetShape(),
                                                 input->GetBufferPtr<int64_t>(), output->GetBufferPtr<int64_t>());
    } else {
        LOG(ERROR) << "unsupported data type: " << ppl::common::GetDataTypeStr(data_type);
        return ppl::common::RC_UNSUPPORTED;
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}} // namespace ppl::nn::x86
