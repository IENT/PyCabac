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
  void encodeBinsBI(uint64_t symbol, const unsigned int * ctxIds, const unsigned int numBins) {
    unsigned int bin = 0;  // bin to encode
    unsigned int i = 0;  // counter for context selection
    for (int exponent = numBins - 1; exponent >= 0; exponent--) {  // i must be signed
      // 0x1u is the same as 0x1. (The u stands for unsigned.).
      bin = static_cast<unsigned int>(static_cast<uint64_t>(symbol) >> static_cast<uint8_t>(exponent)) & 0x1u;
      encodeBin(bin, ctxIds[i]);
      i++;
    }
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
  void encodeBinsTU(uint64_t symbol, const unsigned int * ctxIds, const unsigned int numMaxBins=512) {
    //assert(ctxIds.size() <= numMaxBins);

    uint64_t i;
    for (i = 0; i < symbol; i++) {
      encodeBin(1, ctxIds[i]);
    }
    if (symbol < numMaxBins) {  // symbol == numMaxBins is coded as all 1s
      encodeBin(0, ctxIds[i]); // terminating '0'
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsEG0bypass(uint64_t symbol) {
    auto valuePlus1 = (unsigned int)(symbol + 1);
    auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

    /* prefix */
    encodeBinsBIbypass(1, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
    if (numLeadZeros) {
      /* suffix */
      encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------
  // Taken from GABAC/GENIE
  void encodeBinsEG0(uint64_t symbol, const unsigned int * ctxIds) {
    auto valuePlus1 = (unsigned int)(symbol + 1);
    auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

    //assert(ctxIds.size() >= (numLeadZeros + 1));

    /* prefix */
    encodeBinsBI(1, ctxIds, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
    if (numLeadZeros) {
        /* suffix */
      encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEGkbypass(uint64_t symbol, unsigned k) {
    //assert(k > 0); // For k=0, use more efficient GABAC method
    
    if(symbol == 0 && k == 0) {
      encodeBinEP(1);
      return;
    }

    auto numLeadZeros = (unsigned int)(floor(log2(symbol + (1 << k))) - k);
    auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));

    /* prefix */
    encodeBinsBIbypass(1, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')
    /* suffix */
    encodeBinsBIbypass(symbol - m, numLeadZeros + k); // TU encoding of (symbol - m)

  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEGk(uint64_t symbol, unsigned k, const unsigned int * ctxIds) {
    //assert(k > 0); // For k=0, use more efficient GABAC method

    if(symbol == 0 && k == 0) {
      // Opt out
      encodeBin(1, ctxIds[0]);
      return;
    }

    auto numLeadZeros = (unsigned int)(floor(log2(symbol + (1 << k))) - k);
    auto m = (unsigned int)((1 << (numLeadZeros + k)) - (1 << k));

    /* prefix */
    //assert(ctxIds.size() >= (numLeadZeros + 1));
    encodeBinsBI(1, ctxIds, numLeadZeros + 1); // TU encoding (numLeadZeros '0' and terminating '1')

    /* suffix */
    encodeBinsBIbypass(symbol - m, numLeadZeros + k); // TU encoding of (symbol - m)
  }


  // ---------------------------------------------------------------------------------------------------------------------
  // Overloaded functions for latter use in cabacSimpleSequenceEncoder
  // ---------------------------------------------------------------------------------------------------------------------
  void encodeBinsBI(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numBins = binParams[0];
    encodeBinsBI(symbol, ctxIds.data(), numBins);
  }

  void encodeBinsTU(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int numMaxBins = binParams[0];
    encodeBinsTU(symbol, ctxIds.data(), numMaxBins);
  }

  void encodeBinsEG0(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    encodeBinsEG0(symbol, ctxIds.data());
  }

  void encodeBinsEGk(uint64_t symbol, const std::vector<unsigned int>& ctxIds, const std::vector<unsigned int> binParams) 
  {
    const unsigned int k = binParams[1];
    encodeBinsEGk(symbol, k, ctxIds.data());
  }


};  // class cabacSymbolEncoder


#endif // RWTH_PYTHON_IF