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

    binReader getReader(binarization::BinarizationId binId)
    {
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
        case binarization::BinarizationId::NA: {
          func = &cabacSimpleSequenceDecoder::decodeBinsNA;
        } break;
        case binarization::BinarizationId::RICE: {
          throw std::runtime_error("getReader: Binarization RICE not supported with context-adaptive coding");
        } break;
        default:
          throw std::runtime_error("getReader: Unknown binarization ID");
      }
      return func;
    }

    binBypassReader getBypassReader(binarization::BinarizationId binId)
    {
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
        case binarization::BinarizationId::NA: {
          func = &cabacSimpleSequenceDecoder::decodeBinsNAbypass;
        } break;
        case binarization::BinarizationId::RICE: {
          func = &cabacSimpleSequenceDecoder::decodeBinsRicebypass;
        } break;
        default:
          throw std::runtime_error("getBypassReader: Unknown binarization ID");
      }
      return func;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // This is a general method for decoding a sequence of symbols for given binarization and context model
    // parameter definition see encodeSymbols
    void decodeSymbols(uint64_t * symbols, const unsigned int numSymbols,
      binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId,
      const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
      // Allocate memory
      auto order = ctxParams[0];
      std::vector<uint64_t> symbolsPrev(order, 0);
      const unsigned int numMaxBins = binParams[0];
      std::vector<unsigned int> ctxIds(numMaxBins, 0);

      // Get reader
      binReader func = getReader(binId);

      for (unsigned int i = 0; i < numSymbols; i++) {
        // Get context ids for each bin
        for (unsigned int o = 0; o < order; o++) {
          if (i > o) {
            symbolsPrev[o] = symbols[i-o - 1];
          }
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
      binBypassReader func = getBypassReader(binId);

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

    // ---------------------------------------------------------------------------------------------------------------------
    // This is a general method for decoding a symbol for given binarization and context model
    // parameter definition see encodeSymbols
    uint64_t decodeSymbol(const uint64_t * symbolsPrev,
      binarization::BinarizationId binId, const contextSelector::ContextModelId ctxModelId,
      const std::vector<unsigned int> binParams, const std::vector<unsigned int> ctxParams)
    {
      // Get reader
      binReader func = getReader(binId);

      // Get context id for each bin
      const unsigned int numMaxBins = binParams[0];
      std::vector<unsigned int> ctxIds(numMaxBins, 0);
      contextSelector::getContextIds(ctxIds, symbolsPrev, binId, ctxModelId, binParams, ctxParams);

      // Decode bins
      return (*this.*func)(ctxIds, binParams);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // This is a general method for bypass decoding a symbol for given binarization
    // parameter definition see encodeSymbols
    uint64_t decodeSymbolBypass(binarization::BinarizationId binId, const std::vector<unsigned int> binParams)
    {
      // Get reader
      binBypassReader func = getBypassReader(binId);

      // Decode bins
      return (*this.*func)(binParams);
    }

}; // class cabacSimpleSequenceDecoder

#endif  // RWTH_PYTHON_IF