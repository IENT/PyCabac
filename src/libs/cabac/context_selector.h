#ifndef __RWTH_CONTEXT_SELECTOR_H__
#define __RWTH_CONTEXT_SELECTOR_H__

#include "contexts.h"
#include "binarization.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace contextSelector{
    
    enum class ContextModelId : uint8_t {
        BINSORDER1 = 0,
        SYMBOLORDER1 = 1
    };

    /* Context model on bin-to-symbol level */
    // TU binarization
    unsigned int getContextIdBinsOrder1TU(unsigned int, uint64_t, unsigned int);
    void getContextIdsBinsOrder1TU(std::vector<unsigned int>&, uint64_t, unsigned int);

    // EG0 binarization
    unsigned int getContextIdBinsOrder1EG0(unsigned int, uint64_t, unsigned int);
    void getContextIdsBinsOrder1EG0(std::vector<unsigned int>&, uint64_t, unsigned int);

    // EGk binarization
    unsigned int getContextIdBinsOrder1EGk(unsigned int, uint64_t, unsigned int, unsigned int);
    void getContextIdsBinsOrder1EGk(std::vector<unsigned int>&, uint64_t, unsigned int, unsigned int);

    /* Context model on symbol-to-symbol level */
    // TU binarization
    unsigned int getContextIdSymbolOrder1TU(unsigned int, uint64_t, unsigned int, unsigned int);
    void getContextIdsSymbolOrder1TU(std::vector<unsigned int>&, uint64_t, unsigned int, unsigned int);

    // EG0 binarization
    unsigned int getContextIdSymbolOrder1EG0(unsigned int, uint64_t, unsigned int, unsigned int);
    void getContextIdsSymbolOrder1EG0(std::vector<unsigned int>&, uint64_t, unsigned int, unsigned int);

    // EGk binarization
    unsigned int getContextIdSymbolOrder1EGk(unsigned int, uint64_t, unsigned int, unsigned int, unsigned int);
    void getContextIdsSymbolOrder1EGk(std::vector<unsigned int>&, uint64_t, unsigned int, unsigned int, unsigned int);

    // General stuff
    void getContextIds(std::vector<unsigned int>&, uint64_t, binarization::BinarizationId, contextSelector::ContextModelId, const std::vector<unsigned int>, const std::vector<unsigned int>);
    
};

#endif  // RWTH_PYTHON_IF
#endif  // __RWTH_CONTEXT_SELECTOR_H__