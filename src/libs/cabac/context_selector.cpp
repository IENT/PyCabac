#include "contexts.h"
#include "context_selector.h"
#include <cstdint>
#include <vector>
#include <tuple>
#include <cmath>
#include "binarization.h"



#if RWTH_PYTHON_IF
namespace contextSelector{
    const std::vector<unsigned int> offsetsBinOrder1BI = {1, 2, 4, 8}; // lookup table for "speedup"
    const std::vector<unsigned int> offsetsBinOrder1TU = {1, 3, 9, 27}; // lookup table for "speedup"

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
            - getContextIdSymbolOrder1TU: restPos and symbolMax (to yield at max (symbolMax+1)*restPos+1 contexts)
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

    // --------------------------------------------------------------------------------
    // Bin-to-bin level, order 1
    // --------------------------------------------------------------------------------
    unsigned int getContextIdBinsOrder1BI(const unsigned int n, const uint64_t symbolPrev, const unsigned int numBins, const unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in BI-binarized bin-string corresponding to bin in previous symbol.
        In fact, it should model the probability p(b_t,n | b_t-1,n) with bin b_n at position n in BI-binarized bin-string.
        b_t-1,n is the bin at position n in BI-binarized bin-string corresponding to previous symbol.
        b_t,n is the bin at position n in BI-binarized bin-string corresponding to current symbol.

        In total we have num_ctx_total = 2*R + 1 contexts with R = restPos

        The ctx_id is computed as follows:
        ctx_id:         meaning:
        0*R ... 1*R-1:  previously coded bin at position n<R is 0
        1*R ... 2*R-1:  previously coded bin at position n<R is 1
        2*R:            position n>=R: rest case with single context

        Let the ctx_ids be numbered from 0 to num_ctx_total-1
        For TU, the numbering is the same (from left to right), e.g.
        n:   0 1 2 3 ...       0 1 2 3 ...
        DEC -> TU           -> BI
        0   -> 0            -> 0 0 0 0
        1   -> 1 0          -> 0 0 0 1
        2   -> 1 1 0        -> 0 0 1 0
        3   -> 1 1 1 0      -> 0 0 1 1
        BI exponent:       ... 3 2 1 0
        */

        // Get bin at position n in BI-binarized bin-string corresponding to previous symbol
        unsigned int symbolPrevBin = static_cast<unsigned int>(symbolPrev >> static_cast<uint8_t>(numBins-n-1)) & 0x1u;
        unsigned int ctxId = 0;
        if(n < restPos) { // we are in the first n<restPos bins
            // actual context modelling
            if(symbolPrevBin){
                ctxId = 1*restPos + n; // 1 case
            } else {
                ctxId = 0*restPos + n; // 0 case
            }
        }
        else {
            // no context modelling: rest case with single context
            ctxId = 2 * restPos;
        }

        return ctxId;
    }

    void getContextIdsBinsOrder1BI(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int numBins, const unsigned int restPos=10){
        /* 
        Get context IDs for all bins of a BI-binarized symbol given the previous symbol and the number of rest bins.
        */
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdBinsOrder1BI(restPos, symbolPrev, numBins, restPos);
        for(unsigned int n=0; n<ctxIds.size(); n++) {
            if(n < restPos) { // actual context modelling for bins at position n<restPos 
                ctxIds[n] = getContextIdBinsOrder1BI(n, symbolPrev, numBins, restPos);
            } else { // bins at position n>=restPos are modeled with the same rest context
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdBinsOrder1TU(const unsigned int n, const uint64_t symbolPrev, const unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in TU-binarized bin-string corresponding to bin in previous symbol.
        In fact, it should model the probability p(b_t,n | b_t-1,n) with bin b_n at position n in TU-binarized bin-string.
        b_t-1,n is the bin at position n in TU-binarized bin-string corresponding to previous symbol.
        b_t,n is the bin at position n in TU-binarized bin-string corresponding to current symbol.

        In total we have num_ctx_total = 3*R + 1 contexts with R = restPos

        The ctx_id is computed as follows:
        ctx_id:         meaning:
        0*R ... 1*R-1:  previously coded bin at position n<R not available
        1*R ... 2*R-1:  previously coded bin at position n<R is 0
        2*R ... 3*R-1:  previously coded bin at position n<R is 1
        3*R:            position n>=R: rest case with single context
        */

        unsigned int ctxId = 0;
        if(n < restPos) { // we are in the first n<restPos bins
            // actual context modelling
            if(n > symbolPrev) { // previously coded bin at position n not available
                ctxId = 0*restPos + n; // N/A case
            } else if(n < symbolPrev) { // previously coded bin 0 at position n available
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

    void getContextIdsBinsOrder1TU(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int restPos=10){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdBinsOrder1TU(restPos, symbolPrev, restPos);
        for(unsigned int n=0; n < ctxIds.size(); n++) {
            if(n < restPos) { // actual context modelling for bins at position n<restPos 
                ctxIds[n] = getContextIdBinsOrder1TU(n, symbolPrev, restPos);
            } else { // bins at position n>=restPos are modeled with the same rest context
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdBinsOrder1EG0(const unsigned int n, const uint64_t symbolPrev, const unsigned int restPos=10){
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + 1)));
        return getContextIdBinsOrder1TU(n, prevNumLeadZeros, restPos);
    }

    void getContextIdsBinsOrder1EG0(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int restPos){
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + 1)));
        getContextIdsBinsOrder1TU(ctxIds, prevNumLeadZeros, restPos);
    }

    unsigned int getContextIdBinsOrder1EGk(const unsigned int n, const uint64_t symbolPrev, const unsigned int k, const unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        // Get number of leading zeros to encode previous symbol with EGk code
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);

        // Return context ID for the prefix TU code
        return getContextIdBinsOrder1TU(n, prevNumLeadZeros, restPos);
    }

    void getContextIdsBinsOrder1EGk(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int k, const unsigned int restPos){
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);
        getContextIdsBinsOrder1TU(ctxIds, prevNumLeadZeros, restPos);
    }


    // --------------------------------------------------------------------------------
    // Bin-to-bin level, order N
    // --------------------------------------------------------------------------------

    unsigned int getContextIdBinsOrderNBI(const unsigned int order, const unsigned int n, const std::vector<uint64_t> symbolsPrev, 
        const unsigned int numBins, const unsigned int restPos=10
    ){
        /* 
        ContextID dependent on bin at position n in BI-binarized bin-string corresponding to bin in previous symbols.
        In fact, it should model the probability p(b_t,n | b_t-1,n, b_t-2,n, ...) with bin b_n at position n in BI-binarized bin-string.
        b_t,n is the bin at position n in BI-binarized bin-string corresponding to current symbol.
        b_t-1,n is the bin at position n in BI-binarized bin-string corresponding to previous symbol and so on.
        
        In total we have num_ctx_total = (3^order)*R + 1 contexts with R = restPos

        The ctx_id is computed as follows (example with order=3):

        b[t-3, n]   b[t-2, n]   b[t-1, n]   ctx_id
        --------------------------------------------------------------- 
        0           0           0           ( 0 = 0*2^2 + 0*2 + 0)*R + n
        0           0           1           ( 1 = 0*2^2 + 0*2 + 1)*R + n
        0           1           0           ( 2 = 0*2^2 + 1*2 + 0)*R + n
        0           1           1           ( 3 = 0*2^2 + 1*2 + 1)*R + n
        1           0           0           ( 4 = 1*2^2 + 0*2 + 0)*R + n
        1           0           1           ( 5 = 1*2^2 + 0*2 + 1)*R + n
        1           1           0           ( 6 = 1*2^2 + 1*2 + 0)*R + n
        1           1           1           ( 7 = 1*2^2 + 1*2 + 1)*R + n
        n >= R                              ( 8 = 2^order)*R

        ctx_id = (i_o*2^(o-1) + ... + i_3*2^2 + i_2*2^1 + i_3*2^0)*R + n with i_o in {0, 1}, indicating if b[t-o, n] either 0 or 1

        */

        unsigned int ctxId = 0;
        unsigned int symbolPrevBin = 0;

        if(n < restPos) { // actual context modelling for bins at position n<restPos 
            for(unsigned int o=0; o < order; o++) {
                symbolPrevBin = static_cast<unsigned int>(symbolsPrev[o] >> static_cast<uint8_t>(numBins-n-1)) & 0x1u;
                ctxId += symbolPrevBin*offsetsBinOrder1BI[o];
            }
            ctxId *= restPos; 
            ctxId += n;
        } else { // Rest case
            ctxId = offsetsBinOrder1BI[order]*restPos;
        }

        return ctxId;
    }

    void getContextIdsBinsOrderNBI(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int numBins, const unsigned int restPos=10){
        /* 
        Get context IDs for all bins of a TU-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the TU code.
        */
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdBinsOrderNBI(order, restPos, symbolsPrev, numBins, restPos);
        for(unsigned int n=0; n<ctxIds.size(); n++){
            if(n < restPos) { // actual context modelling for bins at position n<restPos 
                ctxIds[n] = getContextIdBinsOrderNBI(order, n, symbolsPrev, numBins, restPos);
            } else { // bins at position n>=restPos are modeled with the same rest context
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdBinsOrderNTU(const unsigned int order, const unsigned int n, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=10){
        /* 
        ContextID dependent on bin at position n in TU-binarized bin-string corresponding to bin in previous symbols.
        In fact, it should model the probability p(b_t,n | b_t-1,n, b_t-2,n, ...) with bin b_n at position n in TU-binarized bin-string.
        b_t,n is the bin at position n in TU-binarized bin-string corresponding to current symbol.
        b_t-1,n is the bin at position n in TU-binarized bin-string corresponding to previous symbol and so on.
        
        In total we have num_ctx_total = (3^order)*R + 1 contexts with R = restPos

        The ctx_id is computed as follows (example with order=3):

        b[t-3, n]   b[t-2, n]   b[t-1, n]   ctx_id
        --------------------------------------------------------------- 
        NA          NA          NA          ( 0 = 0*3^2 + 0*3 + 0)*R + n
        NA          NA          0           ( 1 = 0*3^2 + 0*3 + 1)*R + n
        NA          NA          1           ( 2 = 0*3^2 + 0*3 + 2)*R + n 
        NA          0           NA          ( 3 = 0*3^2 + 1*3 + 0)*R + n
        NA          0           0           ( 4 = 0*3^2 + 1*3 + 1)*R + n
        NA          0           1           ( 5 = 0*3^2 + 1*3 + 2)*R + n
        NA          1           NA          ( 6 = 0*3^2 + 2*3 + 0)*R + n
        NA          1           0           ( 7 = 0*3^2 + 2*3 + 1)*R + n
        NA          1           1           ( 8 = 0*3^2 + 2*3 + 2)*R + n
        0           NA          NA          ( 9 = 1*3^2 + 0*3 + 0)*R + n
        0           NA          0           (10 = 1*3^2 + 0*3 + 1)*R + n
        0           NA          1           (11 = 1*3^2 + 0*3 + 2)*R + n
        ...         ...         ...         ...
        1           1           NA          (24 = 2*3^2 + 2*3 + 0)*R + n
        1           1           0           (25 = 2*3^2 + 2*3 + 1)*R + n
        1           1           1           (26 = 2*3^2 + 2*3 + 2)*R + n
        n >= R                              (27 = 9*3 + 0)*R = (3^order)*R

        ctx_id = (i_o*3^(o-1) + ... + i_3*3^2 + i_2*3^1 + i_3*3^0)*R + n with i_o in {0, 1, 2}, indicating if b[t-o, n] either NA, 0 or 1

        */

        unsigned int ctxId = 0;
        unsigned int ctxIdCurrent = 0;

        if(n < restPos) { // actual context modelling for bins at position n<restPos 
            for(unsigned int o=0; o<order; o++) {
                ctxIdCurrent = 0;  // either 0, 1 or 2

                if(n > symbolsPrev[o]) {
                    ctxIdCurrent = 0;
                } else if (n < symbolsPrev[o]) {
                    ctxIdCurrent = 1;
                } else { // n == symbolsPrev[n]
                    ctxIdCurrent = 2;
                }

                ctxId += ctxIdCurrent*offsetsBinOrder1TU[o];
            }
            ctxId *= restPos; 
            ctxId += n;
        } else { // rest case
            ctxId = offsetsBinOrder1TU[order]*restPos;
        }

        return ctxId;
    }

    void getContextIdsBinsOrderNTU(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=10){
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdBinsOrderNTU(order, restPos, symbolsPrev, restPos);
        for(unsigned int n=0; n<ctxIds.size(); n++){
            if(n<restPos){ // actual context modelling for bins at position n<restPos 
                ctxIds[n] = getContextIdBinsOrderNTU(order, n, symbolsPrev, restPos);
            } else { // bins at position n>=restPos are modeled with the same rest context
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdBinsOrderNEG0(const unsigned int n, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=10){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++){
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
        }
        return getContextIdBinsOrderNTU(order, n, prevNumsLeadZeros, restPos);
    }

    void getContextIdsBinsOrderNEG0(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++){
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
        }
        getContextIdsBinsOrderNTU(ctxIds, order, prevNumsLeadZeros, restPos);
    }

    unsigned int getContextIdBinsOrderNEGk(const unsigned int n, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int k, const unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        // Get number of leading zeros to encode previous symbol with EGk code
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++) {
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
        }

        // Return context ID for the prefix TU code
        return getContextIdBinsOrderNTU(order, n, prevNumsLeadZeros, restPos);
    }

    void getContextIdsBinsOrderNEGk(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int k, const unsigned int restPos){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++) {
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
        }
        getContextIdsBinsOrderNTU(ctxIds, order, prevNumsLeadZeros, restPos);
    }

    // --------------------------------------------------------------------------------
    // Symbol-to-symbol level, order 1
    // --------------------------------------------------------------------------------
    unsigned int getContextIdSymbolOrder1BI(const unsigned int n, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        /* 
        ContextID dependent on bin position n of to-be-decoded symbol as well as previous integer symbol value, in fact
        modeling the probability p(b_n | symbolPrev) with bin b_n at position n in BI-binarized bin-string.
        Let S = symbolMax and R = restPos

        In total we have num_ctx_total = (S+1)*R + 1 contexts

        The ctx_id is computed as follows:
        ctx_id:                 meaning:
            0*R ...     1*R-1:  previously coded symbol = 0 (and n<R)
            1*R ...     2*R-1:  previously coded symbol = 1 (and n<R)
        ...
        (S+0)*R ... (S+1)*R-1:  previously coded symbol = S (and n<R)
        (S+0)*R ... (S+1)*R-1:  previously coded symbol > S (and n<R)
        (S+1)*R:                position n>=R: rest case with single context
        */

        unsigned int ctxId = 0;
        uint64_t symbolPrevClipped = symbolPrev <= symbolMax ? symbolPrev : symbolMax;
        
        if(n < restPos){  // we are in the first n<restPos bins
            // actual context modelling
            ctxId = symbolPrevClipped*restPos + n;
            
        } else{ // we are in n>=restPos bins
            // no context modelling: rest case with single context for all n>=restPos bins
            ctxId = (symbolMax+1) * restPos;
        }

        return ctxId;
    }

    void getContextIdsSymbolOrder1BI(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        /* 
        Get context IDs for all bins of a BI-binarized symbol given the previous symbol, the number of rest bins and the maximum symbol value.
        */
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdSymbolOrder1BI(restPos, symbolPrev, restPos, symbolMax);
        for(unsigned int n=0; n<ctxIds.size(); n++){
            if(n<restPos){
                ctxIds[n] = getContextIdSymbolOrder1BI(n, symbolPrev, restPos, symbolMax);
            } else {
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdSymbolOrder1TU(const unsigned int n, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        return getContextIdSymbolOrder1BI(n, symbolPrev, restPos, symbolMax); // For TU, the symbol-wise order1 context model is the same as for BI
    }

    void getContextIdsSymbolOrder1TU(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        getContextIdsSymbolOrder1BI(ctxIds, symbolPrev, restPos, symbolMax); // For TU, the symbol-wise order1 context model is the same as for BI
    }

    unsigned int getContextIdSymbolOrder1EG0(const unsigned int n, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + 1)));
        return getContextIdSymbolOrder1TU(n, prevNumLeadZeros, restPos, symbolMax);
    }

    void getContextIdsSymbolOrder1EG0(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + 1)));
        getContextIdsSymbolOrder1TU(ctxIds, prevNumLeadZeros, restPos, symbolMax);
    }

    unsigned int getContextIdSymbolOrder1EGk(const unsigned int n, const uint64_t symbolPrev, const unsigned int k, const unsigned int restPos=8, const unsigned int symbolMax=32){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        // Get number of leading zeros to encode previous symbol with EGk code
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);

        // Return context ID for the prefix TU code
        return getContextIdSymbolOrder1TU(n, prevNumLeadZeros, restPos, symbolMax);
    }

    void getContextIdsSymbolOrder1EGk(std::vector<unsigned int>& ctxIds, const uint64_t symbolPrev, const unsigned int k, const unsigned int restPos=8, const unsigned int symbolMax=32){
        /*
        Get context IDs for all bins of a EGk-binarized symbol given the previous symbol, the number of rest bins and the maximum number of bins in the prefix code.
        */
        
        // Get number of leading zeros to encode previous symbol with EGk code
        auto prevNumLeadZeros = (unsigned int)(floor(log2(symbolPrev + (1 << k))) - k);

        // Get Context IDs for the prefix TU code
        getContextIdsSymbolOrder1TU(ctxIds, prevNumLeadZeros, restPos, symbolMax);
    }

    unsigned int getContextIdSymbolOrderNBI(const unsigned int order, const unsigned int n, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        /* 
        ContextID dependent on bin position n of to-be-decoded symbol as well as previous integer symbol value, in fact
        modeling the probability p(b_n | symbolPrev) with bin b_n at position n in BI-binarized bin-string.
        
        In total we have num_ctx_total = ( (S+1)^3 )*R + 1 contexts with S = symbolMax and R = restPos

        The ctx_id is computed as follows:
        
        symbol[n-3] symbol[n-2] symbol[n-1] ctx_id
        0           0           0           0*R ...     1*R-1
        0           0           1           1*R ...     2*R-1
        0           0           2           2*R ...     3*R-1
        ...         ...         ...         ...
        0           0           S           S*R ... (S+1)*R-1
        0           0           S+1         S*R ... (S+1)*R-1     (same ctx as for S)
        --------------------------------------------------------------------------------------------------
        0           1           0           (1*(S+1) + 0)*R ... (1*(S+1) + 1  )*R-1   
        0           1           S           (1*(S+1) + S)*R ... (1*(S+1) + S+1)*R-1
        --------------------------------------------------------------------------------------------------
        0           2           0           (2*(S+1) + 0)*R ... (2*(S+1) + 1  )*R-1
        0           2           S           (2*(S+1) + S)*R ... (2*(S+1) + S+1)*R-1
        ...         ...         ...         ...
        0           S           0           (S*(S+1) + 0)*R ... (S*(S+1) + 1  )*R-1
        0           S           S           (S*(S+1) + S)*R ... (S*(S+1) + S+1)*R-1
        ...         ...         ...         ...
        --------------------------------------------------------------------------------------------------
        1           0           0           (1*(S+1)^2 + 0*(S+1) + 0)*R ... (1*(S+1)^2 + 0*(S+1) + 1  )*R-1
        ...         ...         ...         ...
        1           S           S           (1*(S+1)^2 + S*(S+1) + S)*R ... (1*(S+1)^2 + S*(S+1) + S+1)*R-1
        ...         ...         ...         ...
        S           0           0           (S*(S+1)^2 + 0*(S+1) + 0)*R ... (S*(S+1)^2 + 0*(S+1) + 1  )*R-1
        ...         ...         ...         ...
        S           S           S           (S*(S+1)^2 + S*(S+1) + S)*R ... (S*(S+1)^2 + S*(S+1) + S+1)*R-1
        --------------------------------------------------------------------------------------------------
        s3          s2          s1          (s3*(S+1)^2 + s2*(S+1) + s1)*R ... (s3*(S+1)^2 + s2*(S+1) + s1+1)*R-1
        --------------------------------------------------------------------------------------------------
        n >= R                              ( (S+1)^3 )*R

        */

        uint64_t symbolMaxPlusOne = symbolMax + 1;
        uint64_t symbolPrevClipped = 0;
        unsigned int offset = restPos;
        unsigned int ctxId = 0;

        if(n < restPos){  // we are in the first n<restPos bins
            // actual context modelling
            for (unsigned int o = 0; o < order; o++) {
                // symbolPrevClipped = symbolsPrev[o] <= symbolMax ? symbolsPrev[o] : symbolMax;
                symbolPrevClipped = std::min<uint64_t>(symbolsPrev[o], symbolMax);
                
                ctxId += symbolPrevClipped * offset;
                offset *= symbolMaxPlusOne;
            }
            ctxId += n; // bin position dependent

        } else{ // we are in n>=restPos bins
            // no context modelling: rest case with single context for all n>=restPos bins
            ctxId = pow(symbolMaxPlusOne, order) * restPos;
        }

        return ctxId;
    }

    void getContextIdsSymbolOrderNBI(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev,const unsigned int restPos=8, const unsigned int symbolMax=32){
        /* 
        Get context IDs for all bins of a BI-binarized symbol given the previous symbol, the number of rest bins and the maximum symbol value.
        */
        
        // get context ID for rest case (n=>restPos) as mini speedup
        unsigned int ctxIdRest = getContextIdSymbolOrderNBI(order, restPos, symbolsPrev, restPos, symbolMax);
        for(unsigned int n=0; n < ctxIds.size(); n++) {
            if(n < restPos) {
                ctxIds[n] = getContextIdSymbolOrderNBI(order, n, symbolsPrev, restPos, symbolMax);
            } else {
                ctxIds[n] = ctxIdRest;
            }
        }
    }

    unsigned int getContextIdSymbolOrderNTU(const unsigned int order, const unsigned int n, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=8, const unsigned int symbolMax=32){
        return getContextIdSymbolOrderNBI(order, n, symbolsPrev, restPos, symbolMax); // For TU, the symbol-wise orderN context model is the same as for BI
    }

    void getContextIdsSymbolOrderNTU(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev,const unsigned int restPos=8, const unsigned int symbolMax=32){
        getContextIdsSymbolOrderNBI(ctxIds, order, symbolsPrev, restPos, symbolMax); // For TU, the symbol-wise orderN context model is the same as for BI
    }

    unsigned int getContextIdSymbolOrderNEG0(const unsigned int n, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos=10){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++){
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
        }
        return getContextIdSymbolOrderNTU(order, n, prevNumsLeadZeros, restPos);
    }

    void getContextIdsSymbolOrderNEG0(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int restPos){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++){
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
        }
        getContextIdsSymbolOrderNTU(ctxIds, order, prevNumsLeadZeros, restPos);
    }

    unsigned int getContextIdSymbolOrderNEGk(const unsigned int n, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int k, const unsigned int restPos=10){
        /*
        EG-Codes are constructed out of prefix and suffix. Here, only the prefix is modelled.
        The prefix is modelled as a TU code with a context for each bin.
        */

        // Get number of leading zeros to encode previous symbol with EGk code
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++) {
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
        }

        // Return context ID for the prefix TU code
        return getContextIdSymbolOrderNTU(order, n, prevNumsLeadZeros, restPos);
    }

    void getContextIdsSymbolOrderNEGk(std::vector<unsigned int>& ctxIds, const unsigned int order, const std::vector<uint64_t> symbolsPrev, const unsigned int k, const unsigned int restPos){
        std::vector<uint64_t> prevNumsLeadZeros(order, 0);
        for(unsigned int o=0; o < order; o++) {
            prevNumsLeadZeros[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
        }
        getContextIdsSymbolOrderNTU(ctxIds, order, prevNumsLeadZeros, restPos);
    }

    // This is a general method for encoding a sequence of symbols for given binarization and context model
    // See encodeSymbols for definition of binParams and ctxParams
    unsigned int getContextId(const unsigned int n, const uint64_t* symbolsPrev,
        const binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId, 
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
        auto order = ctxParams[0];
        auto restPos = ctxParams[1];
        auto ctxOffset = ctxParams[2];

        unsigned int contextId = 0;
        std::vector<uint64_t> symbolsPrevForTU(order, 0);

        // Prepare symbolsPrev to be used for TU context model
        switch (binId) {
            case binarization::BinarizationId::BI:
            case binarization::BinarizationId::TU: {
                symbolsPrevForTU = std::vector<uint64_t>(symbolsPrev, symbolsPrev + order);
            } break;
            case binarization::BinarizationId::EG0: {
                for(unsigned int o=0; o < order; o++){
                    symbolsPrevForTU[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
                }
            } break;
            case binarization::BinarizationId::EGk: {
                auto k = binParams[1];
                for(unsigned int o=0; o < order; o++) {
                    symbolsPrevForTU[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
                }
            } break;
            default:
                throw std::runtime_error("getContextId: Unknown binarization ID");
        }

        // Get context ID
        switch(ctxModelId){
            case contextSelector::ContextModelId::BINSORDERN: {
                
                switch(binId){
                    case binarization::BinarizationId::BI: {
                        auto numBins = binParams[0];
                        contextId = getContextIdBinsOrderNBI(order, n, symbolsPrevForTU, numBins, restPos);
                    } break;
                    default: {
                        contextId = getContextIdBinsOrderNTU(order, n, symbolsPrevForTU, restPos);
                    } break;
                }

            } break;
            case contextSelector::ContextModelId::SYMBOLORDERN: {
                auto symbolMax = ctxParams[3];
                contextId = getContextIdSymbolOrderNTU(order, n, symbolsPrevForTU, restPos, symbolMax);
            } break;
            default:
                throw std::runtime_error("getContextId: Unknown context model ID. If you want to use the *ORDER1 context models, just set order=1 and use the *ORDERN models :).");
        }

        // Add offset to context ID
        contextId += ctxOffset;

        return contextId;
    }

    void getContextIds(std::vector<unsigned int>& ctxIds, const uint64_t * symbolsPrev,
        const binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId, 
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
        auto order = ctxParams[0];
        auto restPos = ctxParams[1];
        auto ctxOffset = ctxParams[2];

        std::vector<uint64_t> symbolsPrevForTU(order, 0); // for BI and TU
        switch(binId) {
            case binarization::BinarizationId::BI:
            case binarization::BinarizationId::TU: {
                symbolsPrevForTU = std::vector<uint64_t>(symbolsPrev, symbolsPrev + order);
            } break;
            case binarization::BinarizationId::EG0: {
                for(unsigned int o=0; o < order; o++){
                    symbolsPrevForTU[o] = (unsigned int)(floor(log2(symbolsPrev[o] + 1))); // number of leading zeros
                }
            } break;
            case binarization::BinarizationId::EGk: {
                auto k = binParams[1];
                for(unsigned int o=0; o < order; o++) {
                    symbolsPrevForTU[o] = (unsigned int)(floor(log2(symbolsPrev[o] + (1 << k))) - k); // number of leading zeros
                }
            } break;
            default:
                throw std::runtime_error("getContextIds: Unknown binarization ID");
        }

        // Get context IDs
        switch(ctxModelId){
            case contextSelector::ContextModelId::BINSORDERN: {

                switch(binId){
                    case binarization::BinarizationId::BI: {
                        auto numBins = binParams[0];
                        getContextIdsBinsOrderNBI(ctxIds, order, symbolsPrevForTU, numBins, restPos);
                    } break;
                    default: {
                        getContextIdsBinsOrderNTU(ctxIds, order, symbolsPrevForTU, restPos);
                    } break;
                }
            } break;
            case contextSelector::ContextModelId::SYMBOLORDERN: {

                auto symbolMax = ctxParams[3];
                getContextIdsSymbolOrderNTU(ctxIds, order, symbolsPrevForTU, restPos, symbolMax);
            } break;
            default:
                throw std::runtime_error("getContextIds: Unknown context model ID. If you want to use the *ORDER1 context models, just set order=1 and use the *ORDERN models :).");
        }

        // Add offset to context IDs
        for(unsigned int n=0; n<ctxIds.size(); n++){
            ctxIds[n] += ctxOffset;
        }
    }

    unsigned int getNumContexts(const binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId, 
        const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams){
        unsigned int numContexts = 0;

        auto order = ctxParams[0];
        auto restPos = ctxParams[1];

        switch(ctxModelId){
            case contextSelector::ContextModelId::BINSORDER1:
            case contextSelector::ContextModelId::BINSORDERN: {
                switch(binId){
                    case binarization::BinarizationId::BI: {
                        numContexts = pow(2, order)*restPos + 1; // 0, 1
                    } break;
                    default: {
                        numContexts = pow(3, order)*restPos + 1; // 0, 1, NA
                    } break;
                }
            } break;
            case contextSelector::ContextModelId::SYMBOLORDER1:
            case contextSelector::ContextModelId::SYMBOLORDERN: {
                auto symbolMax = ctxParams[3];
                numContexts = pow(symbolMax+1, order)*restPos + 1;
            } break;
            default:
                throw std::runtime_error("getNumContexts: Unknown context model ID");
        }

        return numContexts;
    }

}; // namespace contextSelector

#endif  // RWTH_PYTHON_IF
