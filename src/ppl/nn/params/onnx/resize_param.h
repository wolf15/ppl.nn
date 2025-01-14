#ifndef _ST_HPC_PPL_NN_PARAMS_ONNX_RESIZE_PARAM_H_
#define _ST_HPC_PPL_NN_PARAMS_ONNX_RESIZE_PARAM_H_

#include <stdint.h>

namespace ppl { namespace nn { namespace common {

struct ResizeParam {
    enum {
        RESIZE_COORD_TRANS_MODE_HALF_PIXEL = 0,
        RESIZE_COORD_TRANS_MODE_PYTORCH_HALF_PIXEL = 1,
        RESIZE_COORD_TRANS_MODE_ALIGN_CORNERS = 2,
        RESIZE_COORD_TRANS_MODE_ASYMMETRIC = 3,
        RESIZE_COORD_TRANS_MODE_TF_HALF_PIXEL_FOR_NN = 4,
        RESIZE_COORD_TRANS_MODE_TF_CROP_AND_RESIZE = 5
    };
    typedef int32_t resize_coord_trans_mode_t;

    enum { RESIZE_MODE_NEAREST = 0, RESIZE_MODE_LINEAR = 1, RESIZE_MODE_CUBIC = 2 };
    typedef int32_t resize_mode_t;

    enum {
        RESIZE_NEAREST_MODE_ROUND_PREFER_FLOOR = 0,
        RESIZE_NEAREST_MODE_ROUND_PREFER_CEIL = 1,
        RESIZE_NEAREST_MODE_FLOOR = 2,
        RESIZE_NEAREST_MODE_CEIL = 3
    };
    typedef int32_t resize_nearest_mode_t;

    resize_coord_trans_mode_t coord_trans_mode;

    float cubic_coeff_a;
    int32_t exclude_outside;
    float extrapolation_value;

    resize_mode_t mode;
    resize_nearest_mode_t nearest_mode;

    bool operator==(const ResizeParam& p) const {
        return this->coord_trans_mode == p.coord_trans_mode && this->cubic_coeff_a == p.cubic_coeff_a &&
            this->exclude_outside == p.exclude_outside && this->extrapolation_value == p.extrapolation_value &&
            this->mode == p.mode && this->nearest_mode == p.nearest_mode;
    }
};

}}} // namespace ppl::nn::common

#endif
