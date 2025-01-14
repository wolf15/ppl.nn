#include "ppl/nn/engines/x86/kernels/onnx/and_kernel.h"
#include "ppl/nn/common/logger.h"

#include "ppl/kernel/x86/bool/logical.h"

namespace ppl { namespace nn { namespace x86 {

ppl::common::RetCode AndKernel::DoExecute(KernelExecContext* ctx) {
    auto A = ctx->GetInput<TensorImpl>(0);
    auto B = ctx->GetInput<TensorImpl>(1);
    auto C = ctx->GetOutput<TensorImpl>(0);

    PPLNN_X86_DEBUG_TRACE("Op: %s\n", GetName().c_str());
    PPLNN_X86_DEBUG_TRACE("Input [A]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(A);
    PPLNN_X86_DEBUG_TRACE("Input [B]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(B);
    PPLNN_X86_DEBUG_TRACE("Output [C]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(C);
    PPLNN_X86_DEBUG_TRACE("isa: %u\n", GetISA());

    const bool is_eltwise =
        A->GetShape().GetElementsExcludingPadding() == C->GetShape().GetElementsExcludingPadding() &&
        B->GetShape().GetElementsExcludingPadding() == C->GetShape().GetElementsExcludingPadding();
    const ppl::common::datatype_t data_type = A->GetShape().GetDataType();

    if (data_type == ppl::common::DATATYPE_BOOL) {
        if (is_eltwise) {
            return kernel::x86::and_eltwise_bool(&C->GetShape(), A->GetBufferPtr<uint8_t>(), B->GetBufferPtr<uint8_t>(),
                                                 C->GetBufferPtr<uint8_t>());
        } else {
            return kernel::x86::and_ndarray_bool(&A->GetShape(), &B->GetShape(), &C->GetShape(),
                                                 A->GetBufferPtr<uint8_t>(), B->GetBufferPtr<uint8_t>(),
                                                 C->GetBufferPtr<uint8_t>());
        }
    } else {
        LOG(ERROR) << "unsupported data type: " << ppl::common::GetDataTypeStr(data_type);
        return ppl::common::RC_UNSUPPORTED;
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}} // namespace ppl::nn::x86
