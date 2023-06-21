#pragma once

#include "CommonDef.h"
#include <cstdint>
#include <vector>

#if RWTH_PYTHON_IF
#include "bin_decoder.h"

class cabacSymbolDecoder : public cabacDecoder{
  public:
    cabacSymbolDecoder(std::vector<uint8_t> bs) : cabacDecoder(bs){}

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsBIbypass(const unsigned int numBins) {
        return decodeBinsEP(numBins);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsBI(const unsigned int * ctxIds, const unsigned int numBins) {
        unsigned int bins = 0; // bins to decode
        unsigned int i = 0; // counter for context selection
        for (int exponent = numBins; exponent > 0; exponent--) {
            bins = (bins << 1u) | decodeBin(ctxIds[i]);
            i++;
        }
        return static_cast<uint64_t>(bins);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsTUbypass(const unsigned int numMaxBins=512) {
        unsigned int i = 0;
        while (i < numMaxBins) {
            if (decodeBinsEP(1) == 0) break;
            i++;
        }
        return static_cast<uint64_t>(i);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsTU(const unsigned int * ctxIds, const unsigned int numMaxBins=512) {
        unsigned int i = 0;

        while (i < numMaxBins) {
            if (decodeBin(ctxIds[i]) == 0) break;
            i++;
        }
        return static_cast<uint64_t>(i);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsEG0bypass() {
        unsigned int bins = 0;
        unsigned int numBinsSuffix = 0;
        while (decodeBinsBIbypass(1) == 0) {
            numBinsSuffix++;
        }
        if (numBinsSuffix != 0) {
            bins = (1u << numBinsSuffix) | decodeBinsEP(numBinsSuffix);
            return static_cast<uint64_t>(bins - 1);
        } else {
            return 0;
        }
        
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsEG0(const unsigned int * ctxIds) {

        // Prefix
        unsigned int numBinsSuffix = 0;
        while (decodeBin(ctxIds[numBinsSuffix]) == 0) {
            numBinsSuffix++;
        }

        // Suffix
        unsigned int bins = 0;
        if (numBinsSuffix != 0) {
            bins = (1u << numBinsSuffix) | decodeBinsEP(numBinsSuffix);
            return static_cast<uint64_t>(bins - 1);
        } else {
            return 0;
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEGkbypass(unsigned k) {

        unsigned int symbol = 0;

        /* prefix */
        unsigned int numLeadZeros = 0;
        while (decodeBinsBIbypass(1) == 0) {
          numLeadZeros++;
        }

        if(k == 0 && numLeadZeros == 0){
          return 0;
        }
        auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));
        symbol = decodeBinsBIbypass(numLeadZeros + k);
        symbol += m;

        return static_cast<uint64_t>(symbol);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEGk(unsigned k, const unsigned int * ctxIds) {

        unsigned int symbol = 0;

        /* prefix */
        unsigned int numLeadZeros = 0;
        while (decodeBin(ctxIds[numLeadZeros]) == 0) {
            numLeadZeros++;
        }

        if(k == 0 && numLeadZeros == 0){
          return 0;
        }

        auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));
        symbol = decodeBinsBIbypass(numLeadZeros + k);
        symbol += m;

        return static_cast<uint64_t>(symbol);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Overloaded functions for latter use in cabacSimpleSequenceDecoder
    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsBIbypass(const std::vector<unsigned int> binParams)
    {
        const unsigned int numBins = binParams[0];
        return decodeBinsBIbypass(numBins);
    }

    uint64_t decodeBinsBI(const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams)
    {
        const unsigned int numBins = binParams[0];
        return decodeBinsBI(ctxIds.data(), numBins);
    }

    uint64_t decodeBinsTUbypass(const std::vector<unsigned int> binParams)
    {
        const unsigned int numMaxBins = binParams[0];
        return decodeBinsTUbypass(numMaxBins);
    }

    uint64_t decodeBinsTU(const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams)
    {
        const unsigned int numMaxBins = binParams[0];
        return decodeBinsTU(ctxIds.data(), numMaxBins);
    }

    uint64_t decodeBinsEG0bypass(const std::vector<unsigned int> binParams)
    {
        return decodeBinsEG0bypass();
    }

    uint64_t decodeBinsEG0(const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams)
    {
        return decodeBinsEG0(ctxIds.data());
    }

    uint64_t decodeBinsEGkbypass(const std::vector<unsigned int> binParams)
    {
        const unsigned int k = binParams[1];
        return decodeBinsEGkbypass(k);
    }

    uint64_t decodeBinsEGk(const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams)
    {
        const unsigned int k = binParams[1];
        return decodeBinsEGk(k, ctxIds.data());
    }    

}; // class cabacSymbolDecoder

#endif  // RWTH_PYTHON_IF