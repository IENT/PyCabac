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
        BAC = 0,
        BINPOSITION = 1,
        BINSORDERN = 2,
        SYMBOLORDERN = 3,
        SYMBOLPOSITION = 4,
        BINSYMBOLPOSITION = 5,
        BINSORDERNSYMBOLPOSITION = 6,
        SYMBOLORDERNSYMBOLPOSITION = 7,
    };

    unsigned int getContextIdBinPosition(const unsigned int, const unsigned int);
    void getContextIdsBinPosition(std::vector<unsigned int>&, const unsigned int);

    /* Context model on bin-to-symbol level, order=1 */
    // BI binarization
    unsigned int getContextIdBinsOrder1BI(const unsigned int, const uint64_t, const unsigned int, const unsigned int);
    void getContextIdsBinsOrder1BI(std::vector<unsigned int>&, const uint64_t, const unsigned int, const unsigned int);

    // TU binarization
    unsigned int getContextIdBinsOrder1TU(const unsigned int, const uint64_t, const unsigned int);
    void getContextIdsBinsOrder1TU(std::vector<unsigned int>&, const uint64_t, const unsigned int);

    // EGk binarization
    unsigned int getContextIdBinsOrder1EGk(const unsigned int, const uint64_t, const unsigned int, const unsigned int);
    void getContextIdsBinsOrder1EGk(std::vector<unsigned int>&, const uint64_t, const unsigned int, const unsigned int);


    /* Context model on bin-to-symbol level, order=N */
    // BI binarization
    unsigned int getContextIdBinsOrderNBI(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);
    void getContextIdsBinsOrderNBI(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);
    
    // TU binarization
    unsigned int getContextIdBinsOrderNTU(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int);
    void getContextIdsBinsOrderNTU(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int);
        
    // EGk binarization
    unsigned int getContextIdBinsOrderNEGk(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);
    void getContextIdsBinsOrderNEGk(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);


    /* Context model on symbol-to-symbol level, order=1 */
    // BI binarization
    unsigned int getContextIdSymbolOrder1BI(const unsigned int, const uint64_t, const unsigned int, const unsigned int);
    void getContextIdsSymbolOrder1BI(std::vector<unsigned int>&, const uint64_t, const unsigned int, const unsigned int);

    // TU binarization
    unsigned int getContextIdSymbolOrder1TU(const unsigned int, const uint64_t, const unsigned int, const unsigned int);
    void getContextIdsSymbolOrder1TU(std::vector<unsigned int>&, const uint64_t, const unsigned int, const unsigned int);

    // EGk binarization
    unsigned int getContextIdSymbolOrder1EGk(const unsigned int, const uint64_t, const unsigned int, const unsigned int, const unsigned int);
    void getContextIdsSymbolOrder1EGk(std::vector<unsigned int>&, const uint64_t, const unsigned int, const unsigned int, const unsigned int);

    /* Context model on symbol-to-symbol level, order=1 */
    // BI binarization
    unsigned int getContextIdSymbolOrderNBI(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int, const unsigned int);
    void getContextIdsSymbolOrderNBI(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int, const unsigned int);
    
    // TU binarization
    unsigned int getContextIdSymbolOrderNTU(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);
    void getContextIdsSymbolOrderNTU(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int);
    
    // EGk binarization
    unsigned int getContextIdSymbolOrderNEGk(const unsigned int, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int, const unsigned int);
    void getContextIdsSymbolOrderNEGk(std::vector<unsigned int>&, const unsigned int, const std::vector<uint64_t>,
        const unsigned int, const unsigned int, const unsigned int);

    /* General stuff */
    unsigned int getContextIdOrder1(const unsigned int, const uint64_t, 
        const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);
    void getContextIdsOrder1(std::vector<unsigned int>&, const uint64_t, 
        const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);
    unsigned int getContextId(const unsigned int, const unsigned int, const uint64_t *,
        const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);
    void getContextIds(std::vector<unsigned int>&, const unsigned int, const uint64_t *, 
        const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);

    unsigned int getNumContexts(const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);

    unsigned int getSymbolPositionContextOffset(const unsigned int, 
        const binarization::BinarizationId, const contextSelector::ContextModelId, 
        const std::vector<unsigned int>, const std::vector<unsigned int>);

    contextSelector::ContextModelId getBaseContextModelId(const contextSelector::ContextModelId);

};  // namespace contextSelector

#endif  // RWTH_PYTHON_IF
#endif  // __RWTH_CONTEXT_SELECTOR_H__