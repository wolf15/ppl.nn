#ifndef _ST_HPC_PPL_NN_ENGINES_X86_KERNELS_ONNX_BATCH_NORMALIZATION_KERNEL_H_
#define _ST_HPC_PPL_NN_ENGINES_X86_KERNELS_ONNX_BATCH_NORMALIZATION_KERNEL_H_

#include "ppl/nn/engines/x86/kernel.h"
#include "ppl/nn/params/onnx/batch_normalization_param.h"

namespace ppl { namespace nn { namespace x86 {

class BatchNormalizationKernel : public X86Kernel {
public:
    BatchNormalizationKernel(const ir::Node* node) : X86Kernel(node) {}

    void SetParam(const ppl::nn::common::BatchNormalizationParam* p) {
        param_ = p;
    }

    void SetFuseReLU(bool fuse_relu) {
        fuse_relu_ = fuse_relu;
    }

private:
    ppl::common::RetCode DoExecute(KernelExecContext*) override;

private:
    const ppl::nn::common::BatchNormalizationParam* param_ = nullptr;
    bool fuse_relu_ = false;
};

}}} // namespace ppl::nn::x86

#endif
