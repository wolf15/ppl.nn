#include <stdarg.h>

#include "ppl/nn/engines/x86/engine.h"
#include "ppl/nn/engines/x86/engine_context.h"
#include "ppl/nn/engines/x86/optimizer/opt_kernel_creator_manager.h"
#include "ppl/nn/engines/x86/optimizer/opt_graph.h"
#include "ppl/nn/common/logger.h"
#include "ppl/nn/utils/utils.h"
#include "ppl/kernel/x86/common/simd_tools.h"

using namespace std;
using namespace ppl::common;

namespace ppl { namespace nn { namespace x86 {

X86Engine::X86Engine() : EngineImpl("x86"), device_(X86_DEFAULT_ALIGNMENT, ppl::common::GetCpuISA()) {
    ppl::kernel::x86::set_denormals_zero(true);
}

EngineContext* X86Engine::CreateEngineContext(const string&, const EngineContextOptions& options) {
    return new X86EngineContext(GetName(), device_.GetISA(), options);
}

bool X86Engine::CanRunOp(const ir::Node* node) const {
    auto& type = node->GetType();
    return (OptKernelCreatorManager::Instance()->Find(type.domain, type.name) != nullptr);
}

RetCode X86Engine::DoOptimize(ir::Graph* graph, utils::SharedResource* resource, RuntimePartitionInfo* info) {
    OptGraph opt_graph;
    auto status = opt_graph.Init(graph, resource, info);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "init OptGraph failed: " << GetRetCodeStr(status);
        return status;
    }

    status = opt_graph.DoOptimize(&device_);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "OptGraph DoOptimize failed: " << GetRetCodeStr(status);
        return status;
    }

    return RC_SUCCESS;
}

RetCode X86Engine::ProcessGraph(utils::SharedResource* resource, ir::Graph* graph, RuntimePartitionInfo* info) {
    auto status = DoOptimize(graph, resource, info);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "DoOptimize failed: " << GetRetCodeStr(status);
        return status;
    }

    status = utils::LoadConstants(*graph, &device_, &info->constants);
    if (status != RC_SUCCESS) {
        LOG(ERROR) << "LoadConstants failed: " << GetRetCodeStr(status);
        return status;
    }

    return RC_SUCCESS;
}

/* -------------------------------------------------------------------------- */

RetCode X86Engine::DisableAVX512(X86Engine* engine, va_list) {
    auto isa = engine->device_.GetISA();
    isa &= (~ppl::common::ISA_X86_AVX512);
    engine->device_.SetISA(isa);
    return RC_SUCCESS;
}

X86Engine::ConfHandlerFunc X86Engine::conf_handlers_[] = {
    X86Engine::DisableAVX512,
};

RetCode X86Engine::Configure(uint32_t option, ...) {
    if (option >= X86_CONF_MAX) {
        LOG(ERROR) << "invalid option[" << option << "] >= [" << X86_CONF_MAX << "]";
        return RC_INVALID_VALUE;
    }

    va_list args;
    va_start(args, option);
    auto status = conf_handlers_[option](this, args);
    va_end(args);

    return status;
}

}}} // namespace ppl::nn::x86