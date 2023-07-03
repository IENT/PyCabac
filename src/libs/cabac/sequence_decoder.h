#pragma once

#include "CommonDef.h"
#include <cstdint>
#include <vector>

#if RWTH_PYTHON_IF
#include "context_selector.h"
#include "binarization.h"
#include "symbol_decoder.h"


typedef uint64_t (cabacSymbolDecoder::*binReader)(const std::vector<unsigned int>&, std::vector<unsigned int>);
typedef uint64_t (cabacSymbolDecoder::*binBypassReader)(std::vector<unsigned int>);

class cabacSimpleSequenceDecoder : public cabacSymbolDecoder{
  public:
    cabacSimpleSequenceDecoder(std::vector<uint8_t> bs) : cabacSymbolDecoder(bs){}

    // ---------------------------------------------------------------------------------------------------------------------
    // Context model dependent on bins of previous symbol value (bin at same position)
    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsBIbinsOrder1(uint64_t symbolPrev, unsigned int numBins, unsigned int restPos=10) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numBins, 0);
      contextSelector::getContextIdsBinsOrder1BI(ctxIds, symbolPrev, numBins, restPos);

      // Decode symbol
      return decodeBinsBI(ctxIds.data(), numBins);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsTUbinsOrder1(uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxBins=512) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxBins, 0);
      contextSelector::getContextIdsBinsOrder1TU(ctxIds, symbolPrev, restPos);

      // Decode symbol
      return decodeBinsTU(ctxIds.data(), numMaxBins);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEGkbinsOrder1(uint64_t symbolPrev, unsigned int k, unsigned int restPos=10, unsigned int numMaxPrefixBins=24) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
      contextSelector::getContextIdsBinsOrder1EGk(ctxIds, symbolPrev, k, restPos);

      // Decode bins
      return decodeBinsEGk(k, ctxIds.data());
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Context model dependent on previous symbol value (integer)
    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsBIsymbolOrder1(uint64_t symbolPrev, unsigned int numBins, unsigned int restPos=8, unsigned int symbolMax=32) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numBins, 0);
      contextSelector::getContextIdsSymbolOrder1BI(ctxIds, symbolPrev, restPos, symbolMax);

      // Decode symbol
      return decodeBinsBI(ctxIds.data(), numBins);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsTUsymbolOrder1(uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxBins=512) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxBins, 0);
      contextSelector::getContextIdsSymbolOrder1TU(ctxIds, symbolPrev, restPos, symbolMax);

      // Decode symbol
      return decodeBinsTU(ctxIds.data(), numMaxBins);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEGksymbolOrder1(uint64_t symbolPrev, unsigned int k, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins=24) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
      contextSelector::getContextIdsSymbolOrder1EGk(ctxIds, symbolPrev, k, restPos, symbolMax);

      // Decode bins
      return decodeBinsEGk(k, ctxIds.data());
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // This is a general method for decoding a sequence of symbols for given binarization and context model
    // parameter definition see encodeSymbols
    void decodeSymbols(uint64_t * symbols, const unsigned int numSymbols,
      binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId,
      const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
      
      auto order = ctxParams[0];
      std::vector<uint64_t> symbolsPrev(order, 0);
      const unsigned int numMaxBins = binParams[0];
      std::vector<unsigned int> ctxIds(numMaxBins, 0);

      // Get reader
      binReader func = nullptr;
      switch(binId){
        case binarization::BinarizationId::BI:{
          func = &cabacSimpleSequenceDecoder::decodeBinsBI;
        } break;
        case binarization::BinarizationId::TU: {
          func = &cabacSimpleSequenceDecoder::decodeBinsTU;
        } break;
        case binarization::BinarizationId::EGk: {
          func = &cabacSimpleSequenceDecoder::decodeBinsEGk;
        } break;
        default:
          throw std::runtime_error("decodeSymbols: Unknown binarization ID");
      }

      for (unsigned int i = 0; i < numSymbols; i++) {
        // Get context ids for each bin
        if(i > 0) {
          symbolsPrev[0] = symbols[i - 1];
        }
        if(order > 1 && i > 1) {
          symbolsPrev[1] = symbols[i - 2];
        }
        if(order > 2 && i > 2) {
          symbolsPrev[2] = symbols[i - 3];
        }
        contextSelector::getContextIds(ctxIds, symbolsPrev.data(), binId, ctxModelId, binParams, ctxParams);

        // Decode bins
        symbols[i] = (*this.*func)(ctxIds, binParams);
      }
    }

    std::vector<uint64_t> decodeSymbols(const unsigned int numSymbols, 
      binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId,
      const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
      // Allocate memory
      std::vector<uint64_t> symbols(numSymbols, 0);

      // Fill symbols
      decodeSymbols(symbols.data(), numSymbols, binId, ctxModelId, binParams, ctxParams);

      // Return symbols
      return symbols;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // This is a general method for bypass decoding a sequence of symbols for given binarization
    // parameter definition see encodeSymbols
    void decodeSymbolsBypass(uint64_t * symbols, const unsigned int numSymbols,
      binarization::BinarizationId binId, const std::vector<unsigned int> binParams)
    {
      // Get reader
      binBypassReader func = nullptr;
      switch(binId){
        case binarization::BinarizationId::BI:{
          func = &cabacSimpleSequenceDecoder::decodeBinsBIbypass;
        } break;
        case binarization::BinarizationId::TU: {
          func = &cabacSimpleSequenceDecoder::decodeBinsTUbypass;
        } break;
        case binarization::BinarizationId::EGk: {
          func = &cabacSimpleSequenceDecoder::decodeBinsEGkbypass;
        } break;
        default:
          throw std::runtime_error("decodeSymbolsBypass: Unknown binarization ID");
      }

      // Decode bins
      for (unsigned int i = 0; i < numSymbols; i++) {
        symbols[i] = (*this.*func)(binParams);
      }
    }

    std::vector<uint64_t> decodeSymbolsBypass(const unsigned int numSymbols, 
      binarization::BinarizationId binId, const std::vector<unsigned int> binParams)
    {
      // Allocate memory
      std::vector<uint64_t> symbols(numSymbols, 0);

      // Fill symbols
      decodeSymbolsBypass(symbols.data(), numSymbols, binId, binParams);

      // Return symbols
      return symbols;
    }

}; // class cabacSimpleSequenceDecoder

#endif  // RWTH_PYTHON_IF