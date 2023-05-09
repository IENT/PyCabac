#include "contexts.h"
#include "context_selector.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>



#if RWTH_PYTHON_IF
namespace contextSelector{

    /* Not a useful context model for BI-binarized symbols*/
    //unsigned int getContextIdOrder1BI(unsigned int n, unsigned int symbolPrev, unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in BI-binarized bin-string corresponding to previous symbol

        In total we have num_ctx_total = 2*restPos + 1 contexts

        The ctx_id is computed as follows:
        ctx_id:                     meaning:
        0*restPos ... 1*restPos-1:  previously coded bin at position n<restPos is 0
        1*restPos ... 2*restPos-1:  previously coded bin at position n<restPos is 1
        2*restPos:                  position n>=restPos: rest case with single context
        */
    /*
        unsigned int ctxId = 0;
        unsigned int binValuePrev = static_cast<unsigned int>(symbolPrev >> static_cast<unsigned>(n)) & 0x1u;

        if(n < restPos) { // we are in the first n<restPos bins
            ctxId = binValuePrev * restPos + n;
        } else {
            // no context modelling: rest case with single context
            ctxId = 2 * restPos;
        }

        return ctxId;
    }
    */

    //void getContextIdsOrder1BI(std::vector<unsigned int>& ctxIds, unsigned int symbolPrev,  const unsigned int numBins, unsigned int restPos=10){
        /* 
        Get context IDs for all bins of a BI-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        //std::vector<unsigned int> ctxIds(numMaxBins, 0);
    /*
        for(unsigned int n=0; n<numBins; n++){
            ctxIds[n] = getContextIdOrder1BI(n, symbolPrev, restPos);
        }
    }
    */


    unsigned int getContextIdOrder1TU(unsigned int n, unsigned int symbolPrev, unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in TU-binarized bin-string corresponding to previous symbol

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

    void getContextIdsOrder1TU(std::vector<unsigned int>& ctxIds, unsigned int symbolPrev, unsigned int restPos=10, unsigned int numMaxBins=512){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        //std::vector<unsigned int> ctxIds(numMaxBins, 0);
        for(unsigned int n=0; n<numMaxBins; n++){
            ctxIds[n] = getContextIdOrder1TU(n, symbolPrev, restPos);
        }
    }

    unsigned int getContextIdOrder1EG0(unsigned int n, unsigned int symbolPrev, unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */
        unsigned int k = 0;  // TODO
        auto prevValuePlus1 = (unsigned int)(symbolPrev + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));
        //auto prevNumPrefixBins = (unsigned int)(prevNumLeadZeros + 1);

        return getContextIdOrder1TU(n, prevNumLeadZeros, restPos);
    }

    void getContextIdsOrder1EG0( std::vector<unsigned int>& ctxIds, unsigned int symbolPrev, unsigned int restPos, unsigned int numMaxPrefixBins=48){
        /*
        Get context IDs for all bins of a EG0-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the prefix code.
        */
        
        unsigned int k = 0;  // TODO
        auto prevValuePlus1 = (unsigned int)(symbolPrev + 1);
        auto prevNumLeadZeros = (unsigned int)(floor(log2(prevValuePlus1)));
        //auto prevNumPrefixBins = (unsigned int)(prevNumLeadZeros + 1);

        //std::vector<unsigned int> ctxIds(numMaxPrefixBins);
        getContextIdsOrder1TU(ctxIds, prevNumLeadZeros, restPos, numMaxPrefixBins);

    }
};

#endif  // RWTH_PYTHON_IF
