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
  void encodeSymbols(const uint64_t * symbols, unsigned int numSymbols, 
    binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId, 
    const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams,
    std::vector<unsigned int> prevSymbolOffsets={},
    const bool * mask=nullptr, unsigned int lenMask=0
  )
  {
    /**
     * Encode a sequence of symbols for given binarization and context model.
     * 
     * @param symbols Array of symbols to encode.
     * @param numSymbols Number of symbols to encode.
     * @param binId Binarization ID.
     * @param ctxModelId Context model ID.
     * @param binParams Binarization parameters. The structure is:
     *                  {numMaxBins or numBins, [k, [riceParam, cuttoff, maxLog2TrDynamicRange]]}
     * @param ctxParams Context model parameters. The structure is:  
     *                  {order, restPos, offset, symbolMax, symbolPosMode, idx1, idx2, idx3}
     *                  If symbolPosMode=1, the context IDs get an offset corresponding to the following symbol position intervals:
     *                  [0, idx1), [idx1, idx2), [idx2, idx3), [idx3, oo)
     * @param prevSymbolOffsets Offsets to access previous symbols for context selection. The structure is:
     *                          {offset1, offset2, offset3, ...}
     *                          Defaults to 1, 2, ..., order.
     * @param mask Optional mask of length lenMask to encode only certain symbols.
     * @param lenMask Length of mask, if mask is given, 0 otherwise.
     * 
     */

    // Check parameters
    if (numSymbols == 0) {
      throw std::runtime_error("encodeSymbols: numSymbols must be greater than 0");
    }
    if (binParams.size() < 1) {
      throw std::runtime_error("encodeSymbols: binParams must contain at least one element (numMaxBins or numBins)");
    }
    if (ctxParams.size() < 3) {
      throw std::runtime_error("encodeSymbols: ctxParams must contain at least three elements (order, restPos, offset)");
    }
    auto order = ctxParams[0];
    contextSelector::checkOrder(order, binId, ctxModelId);
    
    // Allocate memory
    std::vector<uint64_t> symbolsPrev(order, 0);
    const unsigned int numMaxBins = binParams[0];
    std::vector<unsigned int> ctxIds(numMaxBins, 0);

    // Check and fill prevSymbolOffsets
    contextSelector::checkFillPrevSymbolOffsets(prevSymbolOffsets, order);

    // Get writer
    binWriter func = getWriter(binId);

    auto symbolMax = 0;
    if(lenMask > 0) {
      contextSelector::checkForSymbolMax(ctxParams);
      auto symbolMax = ctxParams[3];
    }
    for (unsigned int i = 0; i < numSymbols; i++) {
      
      contextSelector::fillPreviousSymbols2(symbolsPrev, symbols, i, order, prevSymbolOffsets, mask, lenMask, symbolMax);
      contextSelector::getContextIds(ctxIds, i, symbolsPrev.data(), binId, ctxModelId, binParams, ctxParams);

      if (lenMask == 0 || (mask[i] == true)) { // Only encode if mask is true or no mask is given
        // Encode symbol
        (*this.*func)(symbols[i], ctxIds, binParams);
      }
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  void encodeSymbolsBypass(const uint64_t * symbols, unsigned int numSymbols, 
    binarization::BinarizationId binId, const std::vector<unsigned int> binParams,
    const bool * mask=nullptr, const unsigned int lenMask=0)
  {
    /**
     * Bypass encode a sequence of symbols for given binarization type.
     * Parameter definition see encodeSymbols
     */
    // Get writer
    binBypassWriter func = getBypassWriter(binId);

    for (unsigned int i = 0; i < numSymbols; i++) {
      if (lenMask == 0 || (mask[i] == true)) { // Only encode if mask is true or no mask is given
        // Encode symbol
        (*this.*func)(symbols[i], binParams);
      }
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  void encodeSymbol(const uint64_t symbol, const unsigned int d, const uint64_t * symbolsPrev, 
    binarization::BinarizationId binId, contextSelector::ContextModelId ctxModelId, 
    const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
  {
    /**
     * Encode a symbol for given binarization type and context model.
     * Parameter definition see encodeSymbols
     */
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