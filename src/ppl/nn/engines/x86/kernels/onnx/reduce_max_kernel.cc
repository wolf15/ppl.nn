#include "ppl/nn/engines/x86/kernels/onnx/reduce_max_kernel.h"
#include "ppl/nn/common/logger.h"

#include "ppl/kernel/x86/fp32/reduce.h"
#include "ppl/kernel/x86/int64/reduce.h"

namespace ppl { namespace nn { namespace x86 {

ppl::common::RetCode ReduceMaxKernel::DoExecute(KernelExecContext* ctx) {
    auto data = ctx->GetInput<TensorImpl>(0);
    auto reduced = ctx->GetOutput<TensorImpl>(0);

    const uint32_t dim_count = data->GetShape().GetDimCount();
    auto fixed_axes = param_->axes;
    if (param_->axes.empty()) { // empty axes means reduce all dims
        fixed_axes.resize(dim_count);
        for (size_t i = 0; i < dim_count; i++) {
            fixed_axes[i] = i;
        }
    }

    for (uint32_t i = 0; i < fixed_axes.size(); i++) {
        if (fixed_axes[i] < 0) { // turn negative axes to positive axes
            fixed_axes[i] = fixed_axes[i] + dim_count;
        }
    }

    PPLNN_X86_DEBUG_TRACE("Op: %s\n", GetName().c_str());
    PPLNN_X86_DEBUG_TRACE("Input [data]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(data);
    PPLNN_X86_DEBUG_TRACE("Input [reduced]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(reduced);
    for (uint32_t i = 0; i < fixed_axes.size(); ++i) {
        PPLNN_X86_DEBUG_TRACE("axes[%d]: %d\n", i, fixed_axes[i]);
    }
    PPLNN_X86_DEBUG_TRACE("keepdims: %d\n", param_->keep_dims);
    PPLNN_X86_DEBUG_TRACE("isa: %u\n", GetISA());

    auto data_type = data->GetShape().GetDataType();
    if (data_type == ppl::common::DATATYPE_FLOAT32) {
        if (MayUseISA(ppl::common::ISA_X86_AVX)) {
            return kernel::x86::reduce_max_fp32_avx(&data->GetShape(), &reduced->GetShape(),
                                                    data->GetBufferPtr<float>(), fixed_axes.data(), fixed_axes.size(),
                                                    reduced->GetBufferPtr<float>());
        } else if (MayUseISA(ppl::common::ISA_X86_SSE)) {
            return kernel::x86::reduce_max_fp32_sse(&data->GetShape(), &reduced->GetShape(),
                                                    data->GetBufferPtr<float>(), fixed_axes.data(), fixed_axes.size(),
                                                    reduced->GetBufferPtr<float>());
        } else {
            LOG(ERROR) << "get unsupported isa " << GetISA();
            return ppl::common::RC_UNSUPPORTED;
        }
    } else if (data_type == ppl::common::DATATYPE_INT64) {
        return kernel::x86::reduce_max_int64(&data->GetShape(), &reduced->GetShape(), data->GetBufferPtr<int64_t>(),
                                             fixed_axes.data(), fixed_axes.size(), reduced->GetBufferPtr<int64_t>());
    } else {
        LOG(ERROR) << "unsupported data type: " << ppl::common::GetDataTypeStr(data_type) << ".";
        return ppl::common::RC_UNSUPPORTED;
    }

    return ppl::common::RC_UNSUPPORTED;
}

}}} // namespace ppl::nn::x86
