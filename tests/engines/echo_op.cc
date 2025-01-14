#include "tests/engines/echo_op.h"
#include "ppl/nn/runtime/tensor_impl.h"
#include "ppl/nn/common/logger.h"
using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace test {

RetCode TestKernel::Execute(KernelExecContext* ctx) {
    auto& type = GetType();
    LOG(INFO) << "Execute kernel: name[" << GetName() << "], type[" << type.domain << ":" << type.name << "]";

    string input_names_str = "input[";
    for (uint32_t i = 0; i < ctx->GetInputCount(); ++i) {
        auto tensor = ctx->GetInput<TensorImpl>(i);
        input_names_str += string(tensor->GetName()) + ", ";
    }
    input_names_str.resize(input_names_str.size() - 2);
    input_names_str += "]";

    string output_names_str = "output[";
    for (uint32_t i = 0; i < ctx->GetOutputCount(); ++i) {
        auto tensor = ctx->GetOutput<TensorImpl>(i);
        output_names_str += string(tensor->GetName()) + ", ";
    }
    output_names_str.resize(output_names_str.size() - 2);
    output_names_str += "]";

    LOG(INFO) << "    " << input_names_str.c_str();
    LOG(INFO) << "    " << output_names_str.c_str();

    return RC_SUCCESS;
}

}}} // namespace ppl::nn::test
