#ifndef _ST_HPC_PPL_NN_ENGINES_CUDA_OPTIMIZER_OPS_ONNX_ADD_OP_H_
#define _ST_HPC_PPL_NN_ENGINES_CUDA_OPTIMIZER_OPS_ONNX_ADD_OP_H_

#include "ppl/nn/engines/cuda/optimizer/opt_kernel.h"

namespace ppl { namespace nn { namespace cuda {

class AddOp final : public CudaOptKernel {
public:
    AddOp(const ir::Node* node) : CudaOptKernel(node) {}
    KernelImpl* CreateKernelImpl() const override;
    ppl::common::RetCode Init(const OptKernelOptions&) override;
    ppl::common::RetCode Finalize(const OptKernelOptions& options) override;

private:
    uint64_t mask = 0;
};

}}} // namespace ppl::nn::cuda

#endif