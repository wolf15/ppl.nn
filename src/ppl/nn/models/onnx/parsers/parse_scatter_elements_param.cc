#include "ppl/nn/models/onnx/parsers/parse_scatter_elements_param.h"
#include "ppl/nn/models/onnx/utils.h"

namespace ppl { namespace nn { namespace onnx {

ppl::common::RetCode ParseScatterElementsParam(const ::onnx::NodeProto& pb_node, void* arg, ir::Node*, ir::GraphTopo*) {
    auto param = static_cast<ppl::nn::common::ScatterElementsParam*>(arg);
    param->axis = utils::GetNodeAttrByKey(pb_node, "axis", 0);
    return ppl::common::RC_SUCCESS;
}

}}} // namespace ppl::nn::onnx