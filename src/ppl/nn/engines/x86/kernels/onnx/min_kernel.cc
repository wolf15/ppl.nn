#include "ppl/nn/engines/x86/kernels/onnx/min_kernel.h"
#include "ppl/nn/common/logger.h"

#include "ppl/kernel/x86/fp32/min.h"

namespace ppl { namespace nn { namespace x86 {

uint64_t MinKernel::CalcTmpBufferSize(const KernelExecContext& ctx) const {
    const uint32_t input_num = ctx.GetInputCount();
    const uint32_t input_ptrs_buffer_size = input_num * sizeof(const float*);

    uint32_t kernel_inner_buffer_size = 0;
    if (MayUseISA(ppl::common::ISA_X86_AVX)) {
        kernel_inner_buffer_size = kernel::x86::min_fp32_avx_get_temp_buffer_bytes(input_num);
    } else if (MayUseISA(ppl::common::ISA_X86_SSE)) {
        kernel_inner_buffer_size = kernel::x86::min_fp32_sse_get_temp_buffer_bytes(input_num);
    } else {
        LOG(ERROR) << "unsupported isa: " << GetISA();
        return 0;
    }

    return input_ptrs_buffer_size + kernel_inner_buffer_size;
}

ppl::common::RetCode MinKernel::DoExecute(KernelExecContext* ctx) {
    BufferDesc tmp_buffer_desc;
    auto tmp_buffer_size = CalcTmpBufferSize(*ctx);
    auto status = GetX86Device()->AllocTmpBuffer(tmp_buffer_size, &tmp_buffer_desc);
    if (status != ppl::common::RC_SUCCESS) {
        LOG(ERROR) << "alloc tmp buffer size[" << tmp_buffer_size << "] for kernel[" << GetName()
                   << "] failed: " << ppl::common::GetRetCodeStr(status);
        return status;
    }
    BufferDescGuard __tmp_buffer_guard(&tmp_buffer_desc, [this](BufferDesc* buffer) -> void {
        GetX86Device()->FreeTmpBuffer(buffer);
    });
    auto tmp_buffer = tmp_buffer_desc.addr;

    auto min = ctx->GetOutput<TensorImpl>(0);

    PPLNN_X86_DEBUG_TRACE("Op: %s\n", GetName().c_str());
    PPLNN_X86_DEBUG_TRACE("Input num: %u\n", ctx->GetInputCount());
    PPLNN_X86_DEBUG_TRACE("Output [min]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(min);
    PPLNN_X86_DEBUG_TRACE("isa: %u\n", GetISA());

    const auto data_type = min->GetShape().GetDataType();
    const auto data_format = min->GetShape().GetDataFormat();
    if (data_type != ppl::common::DATATYPE_FLOAT32 || data_format != ppl::common::DATAFORMAT_NDARRAY) {
        LOG(ERROR) << "only support fp32 ndarray now.";
        return ppl::common::RC_UNSUPPORTED;
    }

    bool is_eltwise = true;
    const uint32_t input_num = ctx->GetInputCount();
    for (uint32_t i = 0; i < input_num; i++) {
        if (ctx->GetInput<TensorImpl>(i)->GetShape().GetElementsIncludingPadding() !=
            min->GetShape().GetElementsIncludingPadding()) {
            is_eltwise = false;
            break;
        }
    }

    const float** input_ptrs = (const float**)tmp_buffer;
    for (uint32_t i = 0; i < input_num; i++) {
        input_ptrs[i] = ctx->GetInput<TensorImpl>(i)->GetBufferPtr<float>();
    }
    void* temp = (uint8_t*)tmp_buffer + input_num * sizeof(const float*);

    if (is_eltwise) {
        if (MayUseISA(ppl::common::ISA_X86_AVX)) {
            kernel::x86::min_eltwise_fp32_avx(&min->GetShape(), input_ptrs, input_num, min->GetBufferPtr<float>());
        } else if (MayUseISA(ppl::common::ISA_X86_SSE)) {
            kernel::x86::min_eltwise_fp32_sse(&min->GetShape(), input_ptrs, input_num, min->GetBufferPtr<float>());
        } else {
            LOG(ERROR) << "get unsupported isa " << GetISA();
            return ppl::common::RC_UNSUPPORTED;
        }
    } else {
        if (MayUseISA(ppl::common::ISA_X86_AVX)) {
            std::vector<const TensorShape*> input_shapes(ctx->GetInputCount());
            for (uint32_t i = 0; i < ctx->GetInputCount(); ++i) {
                input_shapes[i] = &ctx->GetInput<TensorImpl>(i)->GetShape();
            }

            kernel::x86::min_ndarray_fp32_avx(input_shapes.data(), &min->GetShape(), input_ptrs, input_num, temp,
                                              min->GetBufferPtr<float>());
        } else if (MayUseISA(ppl::common::ISA_X86_SSE)) {
            std::vector<const TensorShape*> input_shapes(ctx->GetInputCount());
            for (uint32_t i = 0; i < ctx->GetInputCount(); ++i) {
                input_shapes[i] = &ctx->GetInput<TensorImpl>(i)->GetShape();
            }

            kernel::x86::min_ndarray_fp32_sse(input_shapes.data(), &min->GetShape(), input_ptrs, input_num, temp,
                                              min->GetBufferPtr<float>());
        } else {
            LOG(ERROR) << "get unsupported isa " << GetISA();
            return ppl::common::RC_UNSUPPORTED;
        }
    }

    return ppl::common::RC_SUCCESS;
}

}}} // namespace ppl::nn::x86
