#include "ppl/nn/engines/x86/kernels/onnx/range_kernel.h"
#include "ppl/nn/common/logger.h"

namespace ppl { namespace nn { namespace x86 {

ppl::common::RetCode RangeKernel::DoExecute(KernelExecContext* ctx) {
    auto start = ctx->GetInput<TensorImpl>(0);
    auto limit = ctx->GetInput<TensorImpl>(1);
    auto delta = ctx->GetInput<TensorImpl>(2);
    auto output = ctx->GetOutput<TensorImpl>(0);

    PPLNN_X86_DEBUG_TRACE("Op: %s\n", GetName().c_str());
    PPLNN_X86_DEBUG_TRACE("Input [start]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(start);
    PPLNN_X86_DEBUG_TRACE("Input [limit]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(limit);
    PPLNN_X86_DEBUG_TRACE("Input [delta]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(delta);
    PPLNN_X86_DEBUG_TRACE("Output [output]:\n");
    PPL_X86_TENSOR_PRINT_DEBUG_MSG(output);
    PPLNN_X86_DEBUG_TRACE("isa: %u\n", GetISA());

    const auto data_type = output->GetShape().GetDataType();
    if (data_type == ppl::common::DATATYPE_INT64) {
        const int64_t start_val = start->GetBufferPtr<int64_t>()[0];
        const int64_t delta_val = delta->GetBufferPtr<int64_t>()[0];
        int64_t* output_data = output->GetBufferPtr<int64_t>();
        for (uint32_t i = 0; i < output->GetShape().GetElementsExcludingPadding(); i++) {
            output_data[i] = start_val + i * delta_val;
        }
    } else if (data_type == ppl::common::DATATYPE_FLOAT32) {
        const float start_val = start->GetBufferPtr<float>()[0];
        const float delta_val = delta->GetBufferPtr<float>()[0];
        float* output_data = output->GetBufferPtr<float>();
        for (uint32_t i = 0; i < output->GetShape().GetElementsExcludingPadding(); i++) {
            output_data[i] = start_val + i * delta_val;
        }
    } else {
        LOG(ERROR) << "unsupported data type: " << common::GetDataTypeStr(data_type) << ".";
        return ppl::common::RC_UNSUPPORTED;
    }

    return ppl::common::RC_SUCCESS;
}

}}} // namespace ppl::nn::x86
