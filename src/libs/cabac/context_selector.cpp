#include "contexts.h"
#include "context_selector.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace context_selector{

    unsigned int getContextIdOrder1TU(unsigned int n, unsigned int prevSymbol, unsigned int restPos=10){
        unsigned int ctxIdx = 0;

        /* 
    ContextID dependent on bin at position n in bin-string corresponding to previous symbol

    In total we have num_ctx_total = 3*restPos + 1 contexts
        The ctx_id is computed as follows:
    ctx_id:                     meaning:
        0*restPos ... 1*restPos-1:  previously coded bin at position n<restPos not available
        1*restPos ... 2*restPos-1:  previously coded bin at position n<restPos is 0
        2*restPos ... 3*restPos-1:  previously coded bin at position n<restPos is 1
        3*restPos:                  position n>=restPos: rest case with single context
        */
        if(n < restPos) { // we are in the first n<restPos bins
            // actual context modelling
            if(n > prevSymbol){ // previously coded bin at position n not available
                ctxIdx = 0*restPos + n; // N/A case
            } else if(n < prevSymbol){ // previously coded bin 0 at position n available
                ctxIdx = 1*restPos + n; // 0 case
            } else { // n == prvSymbol // previously coded bin 1 at position n available
                ctxIdx = 2*restPos + n; // 1 case
            }
        }
        else {
            // no context modelling: rest case with single context
            ctxIdx = 3 * restPos;
        }

        return ctxIdx;
    }

    void getContextIdsOrder1TU(std::vector<unsigned int>& ctxIds, unsigned int prevSymbol, unsigned int restPos=10, unsigned int numTUmax=24){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        //std::vector<unsigned int> ctxIds(numTUmax, 0);
        for(unsigned int n=0; n<numTUmax; n++){
            ctxIds[n] = getContextIdOrder1TU(n, prevSymbol, restPos);
        }
    }

    unsigned int getContextIdOrder1EG0(unsigned int n, unsigned int prevSymbol, unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */
        unsigned int k = 0;  // TODO
        auto prevValuePlus1 = (unsigned int)(prevSymbol + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));
        auto prevNumPrefixBins = (unsigned int)(prevNumLeadZeros + 1);

        return getContextIdOrder1TU(n, prevNumPrefixBins, restPos);
    }

    void getContextIDsOrder1EG0( std::vector<unsigned int>& ctxIds,unsigned int prevSymbol, unsigned int restPos, unsigned int numPrefixMax=48){
        /*
        Get context IDs for all bins of a EG0-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the prefix code.
        */
        
        unsigned int k = 0;  // TODO
        auto prevValuePlus1 = (unsigned int)(prevSymbol + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));
        auto prevNumPrefixBins = (unsigned int)(prevNumLeadZeros + 1);

        //std::vector<unsigned int> ctxIds(numPrefixMax);
        getContextIdsOrder1TU(ctxIds, prevNumPrefixBins, restPos, numPrefixMax);

    }
};

#endif  // RWTH_PYTHON_IF
