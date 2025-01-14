#include "ppl/nn/engines/cuda/kernels/onnx/non_zero_kernel.h"

#include "cudakernel/nn/non_zero.h"

namespace ppl { namespace nn { namespace cuda {

uint64_t NonZeroKernel::CalcTmpBufferSize(const KernelExecContext& ctx) const {
    auto input_shape = ctx.GetOutput<TensorImpl>(0)->GetShape();
    return PPLNonZeroGetTempBufferSize(&input_shape);
}

ppl::common::RetCode NonZeroKernel::DoExecute(KernelExecContext* ctx) {
    BufferDesc tmp_buffer_desc;
    auto tmp_buffer_bytes = CalcTmpBufferSize(*ctx);
    auto status = GetCudaDevice()->AllocTmpBuffer(tmp_buffer_bytes, &tmp_buffer_desc);
    if (status != ppl::common::RC_SUCCESS) {
        LOG(ERROR) << "alloc tmp buffer size[" << tmp_buffer_bytes << "] for kernel[" << GetName()
                   << "] failed: " << ppl::common::GetRetCodeStr(status);
        return status;
    }
    BufferDescGuard __tmp_buffer_guard(&tmp_buffer_desc, [this](BufferDesc* buffer) -> void {
        GetCudaDevice()->FreeTmpBuffer(buffer);
    });
    auto tmp_buffer = tmp_buffer_desc.addr;

    auto input = ctx->GetInput<TensorImpl>(0);
    auto output = ctx->GetOutput<TensorImpl>(0);

    status = PPLCUDANonZeroForwardImp(GetStream(), &input->GetShape(), input->GetBufferPtr(), &output->GetShape(),
                                      output->GetBufferPtr<int64_t>(), (int32_t*)tmp_buffer);

    return status;
}

}}} // namespace ppl::nn::cuda
