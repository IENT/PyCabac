#ifndef __RWTH_BINARIZATION_H__
#define __RWTH_BINARIZATION_H__

#include "contexts.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace binarization{
    
    enum class BinarizationId : uint8_t {
        BI = 0,
        TU = 1,
        EGk = 2,
        NA = 3,
        RICE = 4
    };

};

#endif  // RWTH_PYTHON_IF
#endif  // __RWTH_BINARIZATION_H__