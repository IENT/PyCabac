#pragma once

#include "CommonDef.h"
#include <cstdint>
#include <vector>

#if RWTH_PYTHON_IF
#include "context_selector.h"
#include "binarization.h"
#include "symbol_encoder.h"


typedef void (cabacSymbolEncoder::*binWriter)(uint64_t, const std::vector<unsigned int>&, std::vector<unsigned int>);
typedef void (cabacSymbolEncoder::*binBypassWriter)(uint64_t, std::vector<unsigned int>);


class cabacSimpleSequenceEncoder : public cabacSymbolEncoder{
public:
  cabacSimpleSequenceEncoder() : cabacSymbolEncoder(){}


  // ---------------------------------------------------------------------------------------------------------------------
  // Context model dependent on bins of previous symbol value (bin at same position)
  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsBIbinsOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int numBins, unsigned int restPos=10){

    // Get context ids
    std::vector<unsigned int> ctxIds(numBins, 0);
    contextSelector::getContextIdsBinsOrder1BI(ctxIds, symbolPrev, numBins, restPos);

    // Encode symbol
    encodeBinsBI(symbol, ctxIds.data(), numBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsTUbinsOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxBins=512){

    // Get context ids
    std::vector<unsigned int> ctxIds(numMaxBins, 0);
    contextSelector::getContextIdsBinsOrder1TU(ctxIds, symbolPrev, restPos);

    // Encode symbol
    encodeBinsTU(symbol, ctxIds.data(), numMaxBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEG0binsOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxPrefixBins=24){

    // Get context ids for each bin
    std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
    contextSelector::getContextIdsBinsOrder1EG0(ctxIds, symbolPrev, restPos);

    // Encode bins
    encodeBinsEG0(symbol, ctxIds.data());
    
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEGkbinsOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int k, unsigned int restPos=10, unsigned int numMaxPrefixBins=24){

    // Get context ids for each bin
    std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
    contextSelector::getContextIdsBinsOrder1EGk(ctxIds, symbolPrev, k, restPos);

    // Encode bins
    encodeBinsEGk(symbol, k, ctxIds.data());
    
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Context model dependent on previous symbol value (integer)
  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsBIsymbolOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int numBins, unsigned int restPos=8, unsigned int symbolMax=32){

    // Get context ids
    std::vector<unsigned int> ctxIds(numBins, 0);
    contextSelector::getContextIdsSymbolOrder1BI(ctxIds, symbolPrev, restPos, symbolMax);

    // Encode symbol
    encodeBinsBI(symbol, ctxIds.data(), numBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsTUsymbolOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxBins=512){

    // Get context ids
    std::vector<unsigned int> ctxIds(numMaxBins, 0);
    contextSelector::getContextIdsSymbolOrder1TU(ctxIds, symbolPrev, restPos, symbolMax);

    // Encode symbol
    encodeBinsTU(symbol, ctxIds.data(), numMaxBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEG0symbolOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins=24){

    // Get context ids for each bin
    std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
    contextSelector::getContextIdsSymbolOrder1EG0(ctxIds, symbolPrev, restPos, symbolMax);

    // Encode bins
    encodeBinsEG0(symbol, ctxIds.data());
    
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEGksymbolOrder1(uint64_t symbol, uint64_t symbolPrev, unsigned int k, unsigned int restPos=8, unsigned int symbolMax=32, unsigned int numMaxPrefixBins=24){

    // Get context ids for each bin
    std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
    contextSelector::getContextIdsSymbolOrder1EGk(ctxIds, symbolPrev, k, restPos, symbolMax);

    // Encode bins
    encodeBinsEGk(symbol, k, ctxIds.data());
    
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // This is a general method for encoding a sequence of symbols for given binarization and context model
  // binParams = {numMaxBins or numBins, k}
  // ctxParams = {order, restPos, offset, symbolMax}
  void encodeSymbols(const uint64_t * symbols, unsigned int numSymbols, 
    binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId, 
    const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
  {
    auto order = ctxParams[0];
    // Check order
    if(order == 0 || order > 3) {
      throw std::runtime_error("encodeSymbols: Order must be 1, 2 or 3");
    }
    
    std::vector<uint64_t> symbolsPrev(order, 0);
    const unsigned int numMaxBins = binParams[0];
    std::vector<unsigned int> ctxIds(numMaxBins, 0);

    // Get writer
    binWriter func = nullptr;
    switch(binId){
      case binarization::BinarizationId::BI: {
        func = &cabacSimpleSequenceEncoder::encodeBinsBI;
      } break;
      case binarization::BinarizationId::TU: {
        func = &cabacSimpleSequenceEncoder::encodeBinsTU;
      } break;
      case binarization::BinarizationId::EG0: {
        func = &cabacSimpleSequenceEncoder::encodeBinsEG0;
      } break;
      case binarization::BinarizationId::EGk: {
        func = &cabacSimpleSequenceEncoder::encodeBinsEGk;
      } break;
      default:
        throw std::runtime_error("encodeSymbols: Unknown binarization ID");
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

      // Encode symbol
      (*this.*func)(symbols[i], ctxIds, binParams);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // This is a general method for bypass-encoding a sequence of symbols for given binarization
  // parameter definition see encodeSymbols
  void encodeSymbolsBypass(const uint64_t * symbols, unsigned int numSymbols, 
    binarization::BinarizationId binId, const std::vector<unsigned int> binParams)
  {
    // Get writer
    binBypassWriter func = nullptr;
    switch(binId){
      case binarization::BinarizationId::BI: {
        func = &cabacSimpleSequenceEncoder::encodeBinsBIbypass;
      } break;
      case binarization::BinarizationId::TU: {
        func = &cabacSimpleSequenceEncoder::encodeBinsTUbypass;
      } break;
      case binarization::BinarizationId::EG0: {
        func = &cabacSimpleSequenceEncoder::encodeBinsEG0bypass;
      } break;
      case binarization::BinarizationId::EGk: {
        func = &cabacSimpleSequenceEncoder::encodeBinsEGkbypass;
      } break;
      default:
        throw std::runtime_error("encodeSymbolsBypass: Unknown binarization ID");
    }

    for (unsigned int i = 0; i < numSymbols; i++) {
      // Encode symbol
      (*this.*func)(symbols[i], binParams);
    }
  }

}; // class cabacSimpleSequenceEncoder

#endif // RWTH_PYTHON_IF