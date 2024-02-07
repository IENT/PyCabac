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

  binWriter getWriter(binarization::BinarizationId binId){
    binWriter func = nullptr;
    switch(binId){
      case binarization::BinarizationId::BI: {
        func = &cabacSimpleSequenceEncoder::encodeBinsBI;
      } break;
      case binarization::BinarizationId::TU: {
        func = &cabacSimpleSequenceEncoder::encodeBinsTU;
      } break;
      case binarization::BinarizationId::EGk: {
        func = &cabacSimpleSequenceEncoder::encodeBinsEGk;
      } break;
      case binarization::BinarizationId::NA: {
        func = &cabacSimpleSequenceEncoder::encodeBinsNA;
      } break;
      case binarization::BinarizationId::RICE: {
        throw std::runtime_error("getWriter: Binarization RICE not supported with context-adaptive coding");
      } break;
      default:
        throw std::runtime_error("getWriter: Unknown binarization ID");
    }
    return func;
  }

  binBypassWriter getBypassWriter(binarization::BinarizationId binId){
    binBypassWriter func = nullptr;
      switch(binId){
        case binarization::BinarizationId::BI: {
          func = &cabacSimpleSequenceEncoder::encodeBinsBIbypass;
        } break;
        case binarization::BinarizationId::TU: {
          func = &cabacSimpleSequenceEncoder::encodeBinsTUbypass;
        } break;
        case binarization::BinarizationId::EGk: {
          func = &cabacSimpleSequenceEncoder::encodeBinsEGkbypass;
        } break;
        case binarization::BinarizationId::NA: {
          func = &cabacSimpleSequenceEncoder::encodeBinsNAbypass;
        } break;
        case binarization::BinarizationId::RICE: {
          func = &cabacSimpleSequenceEncoder::encodeBinsRicebypass;
        } break;
        default:
          throw std::runtime_error("getBypassWriter: Unknown binarization ID");
      }

      return func;
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // This is a general method for encoding a sequence of symbols for given binarization and context model
  // binParams = {numMaxBins or numBins, [k, [riceParam, cuttoff, maxLog2TrDynamicRange]]}
  // ctxParams = {order, restPos, offset, symbolMax, symbolPosMode, idx1, idx2, idx3}
  //    If symbolPosMode=1: The ctxs ids get an offset corresponding to the following symbol position intervals 
  //    [0, idx1), [idx1, idx2), [idx2, idx3), [idx3, oo)
  // prevSymbolOffsets = {offset1, offset2, offset3, ...}  // holds offsets to access previous symbols for context selection. 
  //    Defaults to 1, 2, ..., order
  void encodeSymbols(const uint64_t * symbols, unsigned int numSymbols, 
    binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId, 
    const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams,
    std::vector<unsigned int> prevSymbolOffsets = {})
  {
    auto order = ctxParams[0];
    // Check order
    if(order == 0) {
      throw std::runtime_error("encodeSymbols: Order must be larger than 0"); // TODO: Add support for higher orders
    }
    if(
        (
          ctxModelId == contextSelector::ContextModelId::BINSORDERN || 
          ctxModelId == contextSelector::ContextModelId::BINSORDERNSYMBOLPOSITION
        ) && order > 8
      ) {
      throw std::runtime_error("encodeSymbols: Order must be smaller than 8 for BINSORDERN* context models");
    }
    
    std::vector<uint64_t> symbolsPrev(order, 0);
    const unsigned int numMaxBins = binParams[0];
    std::vector<unsigned int> ctxIds(numMaxBins, 0);

    // Get writer
    binWriter func = getWriter(binId);

    // Check prevSymbolOffsets
    if(prevSymbolOffsets.empty()){
      for (unsigned int i = 0; i < order; i++){
        prevSymbolOffsets.push_back(i+1); // holds offsets 1, 2, 3, ...
      }
    } else { // check if length is equal to order
      if(prevSymbolOffsets.size() != order){
        throw std::runtime_error("encodeSymbols: prevSymbolOffsets must have the same length as order");
      }
      // Check if all values are greater than 0 and unique
      for (unsigned int i = 0; i < order; i++){
        if(prevSymbolOffsets[i] == 0){
          throw std::runtime_error("encodeSymbols: prevSymbolOffsets must have values greater than 0");
        }
        for (unsigned int j = i+1; j < order; j++){
          if(prevSymbolOffsets[i] == prevSymbolOffsets[j]){
            throw std::runtime_error("encodeSymbols: prevSymbolOffsets must have unique values");
          }
        }
      }
    }

    int i_offset = 0;
    for (unsigned int i = 0; i < numSymbols; i++) {
      // Get context ids for each bin
      for (unsigned int o = 0; o < order; o++){
        i_offset = i - prevSymbolOffsets[o];    
        if (i_offset >= 0) {  // Take only previous values
          symbolsPrev[o] = symbols[i_offset];
        }
      }
      contextSelector::getContextIds(ctxIds, i, symbolsPrev.data(), binId, ctxModelId, binParams, ctxParams);

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
    binBypassWriter func = getBypassWriter(binId);

    for (unsigned int i = 0; i < numSymbols; i++) {
      // Encode symbol
      (*this.*func)(symbols[i], binParams);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // This is a general method for encoding a symbol for given binarization and context model
  // parameter definition see encodeSymbols
  void encodeSymbol(const uint64_t symbol, const unsigned int d, const uint64_t * symbolsPrev, 
    binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId, 
    const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
  {   
    // Get writer
    binWriter func = getWriter(binId);

    // Get context id for each bin
    const unsigned int numMaxBins = binParams[0];
    std::vector<unsigned int> ctxIds(numMaxBins, 0);
    contextSelector::getContextIds(ctxIds, d, symbolsPrev, binId, ctxModelId, binParams, ctxParams);

    // Encode symbol
    (*this.*func)(symbol, ctxIds, binParams);
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // This is a general method for bypass-encoding a symbol for given binarization
  // parameter definition see encodeSymbols
  void encodeSymbolBypass(const uint64_t symbol, 
    binarization::BinarizationId binId, const std::vector<unsigned int> binParams)
  {
    // Get writer
    binBypassWriter func = getBypassWriter(binId);

    // Encode symbol
    (*this.*func)(symbol, binParams);
  }
  

}; // class cabacSimpleSequenceEncoder

#endif // RWTH_PYTHON_IF