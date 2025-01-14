#include "ppl/nn/engines/x86/kernels/onnx/tile_kernel.h"
#include "ppl/nn/common/logger.h"

#include "ppl/kernel/x86/fp32/tile.h"
#include "ppl/kernel/x86/int64/tile.h"

namespace ppl { namespace nn { namespace x86 {

ppl::common::RetCode TileKernel::DoExecute(KernelExecContext* ctx) {
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

    const auto data_type = ctx->GetInput<TensorImpl>(0)->GetShape().GetDataType();
    const auto data_format = ctx->GetInput<TensorImpl>(0)->GetShape().GetDataFormat();
    if (data_format == ppl::common::DATAFORMAT_NDARRAY) {
        if (data_type == ppl::common::DATATYPE_FLOAT32) {
            return ppl::kernel::x86::tile_ndarray_fp32(&A->GetShape(), &C->GetShape(), A->GetBufferPtr<const float>(),
                                                       B->GetBufferPtr<const int64_t>(), C->GetBufferPtr<float>());
        } else if (data_type == ppl::common::DATATYPE_INT64) {
            return ppl::kernel::x86::tile_ndarray_int64(&A->GetShape(), &C->GetShape(),
                                                        A->GetBufferPtr<const int64_t>(),
                                                        B->GetBufferPtr<const int64_t>(), C->GetBufferPtr<int64_t>());
        } else {
            LOG(ERROR) << "unsupported data type " << ppl::common::GetDataTypeStr(data_type) << ".";
        }
    } else {
        LOG(ERROR) << "unsupported data format " << ppl::common::GetDataFormatStr(data_format) << ".";
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}} // namespace ppl::nn::x86
