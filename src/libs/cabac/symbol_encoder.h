#pragma once

#include "CommonDef.h"
#include <cstdint>
#include <vector>
#include <cmath>


#if RWTH_PYTHON_IF
#include "bin_encoder.h"


// Here we binarize and encode integer symbols directly
class cabacSymbolEncoder : public cabacEncoder{
public:
  cabacSymbolEncoder() : cabacEncoder(){}

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsBIbypass(uint64_t symbol, const unsigned int numBins) {
    encodeBinsEP(symbol, numBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsBI(uint64_t symbol, CtxFunction &ctxFun, const unsigned int numBins) {
    unsigned int bin = 0;  // bin to encode
    unsigned int i = 0;  // counter for context selection
    for (int exponent = numBins - 1; exponent >= 0; exponent--) {  // i must be signed
      // 0x1u is the same as 0x1. (The u stands for unsigned.).
      bin = static_cast<unsigned int>(static_cast<uint64_t>(symbol) >> static_cast<uint8_t>(exponent)) & 0x1u;
      encodeBin(bin, ctxFun(i));
      i++;
    }
  }
  void encodeBinsBI(uint64_t symbol, const unsigned int * ctxIds, const unsigned int numBins) {
    // Create wrapper function for array ctxIds
    CtxFunction ctxFun = [&ctxIds](unsigned int n) {
      return ctxIds[n];
    };
    // Call encodeBinsBI with wrapper function
    encodeBinsBI(symbol, ctxFun, numBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsTUbypass(uint64_t symbol, const unsigned int numMaxBins=512) {
    for (uint64_t i = 0; i < symbol; i++) {
      encodeBinEP(1);
    }
    if (numMaxBins > symbol) {  // symbol == numMaxBins is coded as all 1s
      encodeBinEP(0); // terminating '0'
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsTU(uint64_t symbol, CtxFunction &ctxFun, const unsigned int numMaxBins=512) {
    // Encode sequence of '1' bins of length 'symbol'
    uint64_t i;
    for (i = 0; i < symbol; i++) {
      encodeBin(1, ctxFun(i));
    }
    // Encode terminating '0' bin
    if (symbol < numMaxBins) {  // symbol == numMaxBins is coded as all '1's
      encodeBin(0, ctxFun(i)); // terminating '0'
    }
  }
  void encodeBinsTU(uint64_t symbol, const unsigned int * ctxIds, const unsigned int numMaxBins=512) {
    CtxFunction ctxFun = [&ctxIds](unsigned int n) {
      return ctxIds[n];
    };
    encodeBinsTU(symbol, ctxFun, numMaxBins);
  }

  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsEGkbypass(uint64_t symbol, unsigned k) {
    if (k == 0) { // Taken from GABAC/GENIE
      auto valuePlus1 = (unsigned int)(symbol + 1);
      auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

      // Prefix
      encodeBinsBIbypass(1, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
      if (numLeadZeros) {
        // Suffix
        encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
      }
    } else {
      auto numLeadZeros = (unsigned int)(floor(log2(symbol + (1 << k))) - k);
      auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));

      // Prefix
      encodeBinsBIbypass(1, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
      // Suffix
      encodeBinsBIbypass(symbol - m, numLeadZeros + k); // TU encoding of (symbol - m)
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsEGk(uint64_t symbol, unsigned k, CtxFunction &ctxFun) {
    if(k == 0){ // Taken from GABAC/GENIE
      auto valuePlus1 = (unsigned int)(symbol + 1);
      auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

      // Prefix
      encodeBinsBI(1, ctxFun, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
      if (numLeadZeros) {
        // Suffix
        encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
      }
    } else {
      auto numLeadZeros = (unsigned int)(floor(log2(symbol + (1 << k))) - k);
      auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));

      // Prefix
      encodeBinsBI(1, ctxFun, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')

      // Suffix
      encodeBinsBIbypass(symbol - m, numLeadZeros + k); // TU encoding of (symbol - m)
    }
  }
  void encodeBinsEGk(uint64_t symbol, unsigned k, const unsigned int * ctxIds) {
    CtxFunction ctxFun = [&ctxIds](unsigned int n) {
      return ctxIds[n];
    };
    encodeBinsEGk(symbol, k, ctxFun);
  }


  // ---------------------------------------------------------------------------------------------------------------------
  // Overloaded functions for latter use in cabacSimpleSequenceEncoder
  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsBIbypass(uint64_t symbol, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numBins = binParams[0];
    encodeBinsBIbypass(symbol, numBins);
  }

  void encodeBinsBI(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numBins = binParams[0];
    encodeBinsBI(symbol, ctxIds.data(), numBins);
  }

  void encodeBinsTUbypass(uint64_t symbol, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numMaxBins = binParams[0];
    encodeBinsTUbypass(symbol, numMaxBins);
  }

  void encodeBinsTU(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numMaxBins = binParams[0];
    encodeBinsTU(symbol, ctxIds.data(), numMaxBins);
  }

  void encodeBinsEGkbypass(uint64_t symbol, const std::vector<unsigned int> binParams) 
  {
    const unsigned int k = binParams[1];
    encodeBinsEGkbypass(symbol, k);
  }

  void encodeBinsEGk(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int k = binParams[1];
    encodeBinsEGk(symbol, k, ctxIds.data());
  }

};  // class cabacSymbolEncoder


#endif // RWTH_PYTHON_IF