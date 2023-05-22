#include "contexts.h"
#include "context_selector.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace contextSelector{

    /*
    Context model on bin-to-symbol level
        Each to-be-decoded bin is modelled with a context (up to n<restPos).
        Context is dependent on 
        - position n of to be-decoded bin in bin-string of current symbol and 
        - bin at position n in bin-string corresponding to previous symbol.
        
        Parameter-overview:
        - TU: (all bins (up to n<restPos) are context modelled)
            - getContextIdBinsOrder1TU: restPos (to yield at max 3*restPos+1 contexts)
            - getContextIdsBinsOrder1TU: restPos and lengths of ctxIds vector
        - EGk: (only prefix bins are context modelled)
            - getContextIdBinsOrder1EGk: k and restPos
            - getContextIdsBinsOrder1EGk: k, restPos and lengths of ctxIds vector
        - EG0: (only prefix bins are context modelled)
            - getContextIdBinsOrder1EG0: restPos
            - getContextIdsBinsOrder1EG0: restPos and lengths of ctxIds vector


    Context model on symbol-to-symbol level
        Each to-be-decoded bin is modelled with a context (up to n<restPos).
        Context is dependent on
        - position n of to be-decoded bin in bin-string of current symbol and
        - previous symbol value (clipped to symbolMax).

        Parameter-overview:
        - TU: (all bins (up to n<restPos) are context modelled)
            - getContextIdSymbolOrder1TU: restPos and symbolMax (to yield at max (symbolMax+2)*restPos+1 contexts)
            - getContextIdsSymbolOrder1TU: restPos, symbolMax and lengths of ctxIds vector

    Generally, each symbol is binarized to a bin-string of a certain length.
    
    For TU, each bin is context modelled, which means we need as many contexts as there are bins in the bin-string.
    If symbolMax is the maximum symbol value, then the longest bin-string has length 
    symbolMax.
    In total we need a context vector of that length. 
    Each element of the context vector points at one of 3*restPos+1 contexts. 
    
    For EGk, only the prefix is context modelled, which means we need as many contexts as there are bins in the prefix.
    If symbolMax is the maximum symbol value, then the longest prefix has length 
    floor(log2(symbolMax + 2^k)) - k. (largest value for k=0: log2(symbolMax+1))
    In total we need a context vector of that length.
    Each element of the context vector points at one of 3*restPos+1 contexts.
        
    */

    unsigned int getContextIdBinsOrder1TU(unsigned int n, uint64_t symbolPrev, unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in TU-binarized bin-string corresponding to bin in previous symbol.
        In fact, it should model the probability p(b_t,n | b_t-1,n) with bin b_n at position n in TU-binarized bin-string.
        b_t-1,n is the bin at position n in TU-binarized bin-string corresponding to previous symbol.
        b_t,n is the bin at position n in TU-binarized bin-string corresponding to current symbol.

        In total we have num_ctx_total = 3*restPos + 1 contexts

        The ctx_id is computed as follows:
        ctx_id:                     meaning:
        0*restPos ... 1*restPos-1:  previously coded bin at position n<restPos not available
        1*restPos ... 2*restPos-1:  previously coded bin at position n<restPos is 0
        2*restPos ... 3*restPos-1:  previously coded bin at position n<restPos is 1
        3*restPos:                  position n>=restPos: rest case with single context
        */

        unsigned int ctxId = 0;
        if(n < restPos) { // we are in the first n<restPos bins
            // actual context modelling
            if(n > symbolPrev){ // previously coded bin at position n not available
                ctxId = 0*restPos + n; // N/A case
            } else if(n < symbolPrev){ // previously coded bin 0 at position n available
                ctxId = 1*restPos + n; // 0 case
            } else { // n == prvSymbol // previously coded bin 1 at position n available
                ctxId = 2*restPos + n; // 1 case
            }
        }
        else {
            // no context modelling: rest case with single context
            ctxId = 3 * restPos;
        }

        return ctxId;
    }

    void getContextIdsBinsOrder1TU(std::vector<unsigned int>& ctxIds, uint64_t symbolPrev, unsigned int restPos=10){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        
        // get context ID for rest case as mini speedup
        unsigned int ctxIdRest = getContextIdBinsOrder1TU(restPos, symbolPrev, restPos);
        for(unsigned int n=0; n<ctxIds.size(); n++){
            if(n<restPos){
                ctxIds[n] = getContextIdBinsOrder1TU(n, symbolPrev, restPos);
            } else {
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdBinsOrder1EG0(unsigned int n, uint64_t symbolPrev, unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        auto prevValuePlus1 = (unsigned int)(symbolPrev + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));

        return getContextIdBinsOrder1TU(n, prevNumLeadZeros, restPos);
    }

    void getContextIdsBinsOrder1EG0( std::vector<unsigned int>& ctxIds, uint64_t symbolPrev, unsigned int restPos){
        /*
        Get context IDs for all bins of a EG0-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the prefix code.
        */
        
        auto prevValuePlus1 = (unsigned int)(symbolPrev + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));

        getContextIdsBinsOrder1TU(ctxIds, prevNumLeadZeros, restPos);
    }

    unsigned int getContextIdBinsOrder1EGk(unsigned int n, uint64_t symbolPrev, unsigned int k, unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        // Get number of leading zeros to encode previous symbol with EGk code
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);

        // Return context ID for the prefix TU code
        return getContextIdBinsOrder1TU(n, prevNumLeadZeros, restPos);
    }

    void getContextIdsBinsOrder1EGk(std::vector<unsigned int>& ctxIds, uint64_t symbolPrev, unsigned int k, unsigned int restPos){
        /*
        Get context IDs for all bins of a EGk-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the prefix code.
        */
        
        // Get number of leading zeros to encode previous symbol with EGk code
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);

        // Get Context IDs for the prefix TU code
        getContextIdsBinsOrder1TU(ctxIds, prevNumLeadZeros, restPos);

    }

    unsigned int getContextIdSymbolOrder1TU(unsigned int n, uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32){
        /* 
        ContextID dependent on bin position n of to-be-decoded symbol as well as previous integer symbol value, in fact
        modeling the probability p(b_n | symbolPrev) with bin b_n at position n in TU-binarized bin-string.

        In total we have num_ctx_total = (symbolMax+2)*restPos + 1 contexts

        The ctx_id is computed as follows:
        ctx_id:                       meaning:
        0*restPos ... 1*restPos-1:    previously coded symbol = 0 (and n<restPos)
        1*restPos ... 2*restPos-1:    previously coded symbol = 1 (and n<restPos)
        ...
        (symbolMax+0)*restPos ... (symbolMax+1)*restPos-1:  previously coded symbol = symbolMax (and n<restPos)
        (symbolMax+1)*restPos ... (symbolMax+2)*restPos-1:  previously coded symbol > symbolMax (and n<restPos)
        (symbolMax+2)*restPos:                              position n>=restPos: rest case with single context
        */

        unsigned int ctxId = 0;
        
        if(n < restPos){  // we are in the first n<restPos bins
            // actual context modelling
            if(symbolPrev <= symbolMax){
                ctxId = symbolPrev*restPos + n;
            } else{ // clip symbolPrev to symbolMax but still use bin-position dependent contexts
                ctxId = (symbolMax+1)*restPos + n;
            }
        } else{ // we are in n>=restPos bins
            // no context modelling: rest case with single context for all n>=restPos bins
            ctxId = (symbolMax+2) * restPos;
        }

        return ctxId;
    }

    void getContextIdsSymbolOrder1TU(std::vector<unsigned int>& ctxIds, uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        
        // get context ID for rest case as mini speedup
        unsigned int ctxIdRest = getContextIdSymbolOrder1TU(restPos, symbolPrev, restPos, symbolMax);
        for(unsigned int n=0; n<ctxIds.size(); n++){
            if(n<restPos){
                ctxIds[n] = getContextIdSymbolOrder1TU(n, symbolPrev, restPos, symbolMax);
            } else {
                ctxIds[n] = ctxIdRest;
            }
        }
    }
};

#endif  // RWTH_PYTHON_IF
