#include "ppl/nn/engines/common/onnx/split_to_sequence_op.h"
#include "ppl/nn/engines/common/onnx/split_to_sequence_kernel.h"
#include "ppl/nn/common/logger.h"
using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace common {

void SplitToSequenceOp::Init(uint64_t axis, uint64_t keepdims, const SplitToSequenceKernel::SplitFunc& f) {
    axis_ = axis;
    keepdims_ = keepdims;
    split_func_ = f;
}

KernelImpl* SplitToSequenceOp::CreateKernelImpl() const {
    auto kernel = new SplitToSequenceKernel(node_);
    if (kernel) {
        kernel->SetExecutionInfo(axis_, keepdims_, split_func_);
    }
    return kernel;
}

static RetCode CopyMatrix(uint64_t rows, uint64_t cols, uint32_t element_size, const BufferDesc* src_buf,
                          uint64_t src_step, Device* device, BufferDesc* dst_buf) {
    auto src = *src_buf;
    auto dst = *dst_buf;
    auto bytes_per_col = cols * element_size;
    auto bytes_per_step = src_step * element_size;
    for (uint64_t i = 0; i < rows; ++i) {
        dst.addr = static_cast<char*>(dst.addr) + i * bytes_per_col;
        src.addr = static_cast<char*>(src.addr) + i * bytes_per_step;
        auto status = device->Copy(&dst, src, bytes_per_col);
        if (status != RC_SUCCESS) {
            LOG(ERROR) << "copy data failed: " << GetRetCodeStr(status);
            return status;
        }
    }

    return RC_SUCCESS;
}

RetCode SplitToSequenceOp::GenericSplitFunc(uint64_t dims_before_axis, uint64_t dims_from_axis,
                                            uint64_t dims_after_axis, uint32_t dims_of_chunk, uint32_t element_size,
                                            Device* device, BufferDesc* src_cursor, BufferDesc* dst) {
    auto elements_per_copy = dims_of_chunk * dims_after_axis;
    auto status =
        CopyMatrix(dims_before_axis, elements_per_copy, element_size, src_cursor, dims_from_axis, device, dst);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "copy data failed: " << GetRetCodeStr(status);
        return status;
    }

    src_cursor->addr = (char*)(src_cursor->addr) + elements_per_copy * element_size;
    return RC_SUCCESS;
}

}}} // namespace ppl::nn::common
