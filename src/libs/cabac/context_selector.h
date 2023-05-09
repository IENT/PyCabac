#ifndef __RWTH_CONTEXT_SELECTOR_H__
#define __RWTH_CONTEXT_SELECTOR_H__

#include "contexts.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace context_selector{

    unsigned int getContextIdOrder1TU(unsigned int, unsigned int, unsigned int);

    void getContextIdsOrder1TU(std::vector<unsigned int>&, unsigned int, unsigned int, unsigned int);

    unsigned int getContextIdOrder1EG0(unsigned int, unsigned int, unsigned int);

    void getContextIdsOrder1EG0(std::vector<unsigned int>&, unsigned int, unsigned int, unsigned int);
};

#endif  // RWTH_PYTHON_IF
#endif  // __RWTH_CONTEXT_SELECTOR_H__