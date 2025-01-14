#include "ppl/nn/engines/x86/kernels/onnx/pow_kernel.h"
#include "ppl/nn/common/logger.h"

#include "ppl/kernel/x86/fp32/arithmetic_max6d.h"

namespace ppl { namespace nn { namespace x86 {

ppl::common::RetCode PowKernel::DoExecute(KernelExecContext* ctx) {
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

    const auto data_type = A->GetShape().GetDataType();

    if (data_type == ppl::common::DATATYPE_FLOAT32) {
        if (MayUseISA(ppl::common::ISA_X86_AVX)) {
            return ppl::kernel::x86::pow_ndarray_max6d_fp32_avx(&A->GetShape(), &B->GetShape(),
                                                                A->GetBufferPtr<float>(), B->GetBufferPtr<float>(),
                                                                C->GetBufferPtr<float>());
        } else if (MayUseISA(ppl::common::ISA_X86_SSE)) {
            return ppl::kernel::x86::pow_ndarray_max6d_fp32_sse(&A->GetShape(), &B->GetShape(),
                                                                A->GetBufferPtr<float>(), B->GetBufferPtr<float>(),
                                                                C->GetBufferPtr<float>());
        } else {
            LOG(ERROR) << "get unsupported isa " << GetISA();
        }
    } else {
        LOG(ERROR) << "unsupported datatype: " << ppl::common::GetDataTypeStr(data_type) << ".";
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}} // namespace ppl::nn::x86
