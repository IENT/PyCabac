/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2020, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     BinDecoder.h
 *  \brief    Low level binary symbol writer
 */

#pragma once

// #include "CommonLib/Contexts.h"
#include "bitstream.h"
#include "contexts.h"
#include <cstdint>
#include <vector>
#include <tuple>

#if RWTH_PYTHON_IF
#include "context_selector.h"
#endif

#if RExt__DECODER_DEBUG_BIT_STATISTICS
class CodingStatisticsClassType;
#endif

#if RWTH_PYTHON_IF
class BinDecoderBase
#else
class BinDecoderBase : public Ctx
#endif
{
protected:
  template <class BinProbModel>
  BinDecoderBase ( const BinProbModel* dummy );
public:
  ~BinDecoderBase() {}
public:
  void      init    ( InputBitstream* bitstream );
  void      uninit  ();
  void      start   ();
  void      finish  ();
  void      reset   ( int qp, int initId );
#if RExt__DECODER_DEBUG_BIT_STATISTICS
  void      set     ( const CodingStatisticsClassType& type) { ptype = &type; }
#endif

public:
  virtual unsigned  decodeBin           ( unsigned ctxId    ) = 0;

public:
  unsigned          decodeBinEP         ();
  unsigned          decodeBinsEP        ( unsigned numBins  );
  unsigned          decodeRemAbsEP      ( unsigned goRicePar, unsigned cutoff, int maxLog2TrDynamicRange );
  unsigned          decodeBinTrm        ();
  void              align               ();
  unsigned          getNumBitsRead      () { return m_Bitstream->getNumBitsRead() + m_bitsNeeded; }
private:
  unsigned          decodeAlignedBinsEP ( unsigned numBins  );
protected:
  InputBitstream*   m_Bitstream;
  uint32_t          m_Range;
  uint32_t          m_Value;
  int32_t           m_bitsNeeded;
#if RExt__DECODER_DEBUG_BIT_STATISTICS
  const CodingStatisticsClassType* ptype;
#endif
};

template <class BinProbModel>
class TBinDecoder : public BinDecoderBase
{
public:
  TBinDecoder ();
  ~TBinDecoder() {}
  unsigned decodeBin ( unsigned ctxId );
private:
#if RWTH_PYTHON_IF
  friend class cabacDecoder;
  std::vector<BinProbModel> m_Ctx;
#else
  CtxStore<BinProbModel>& m_Ctx;
#endif
};

typedef TBinDecoder<BinProbModel_Std>   BinDecoder_Std;

#if RWTH_PYTHON_IF
class cabacDecoder : public BinDecoder_Std {
public:
  cabacDecoder(std::vector<uint8_t> bs) {
    m_Bitstream = new InputBitstream(bs);
  }
  ~cabacDecoder() { delete m_Bitstream; }
  void initCtx(const std::vector<std::tuple<double, uint8_t>> initCtx) {
    m_Ctx.resize(initCtx.size());
    for (int i = 0; i < initCtx.size(); ++i) {
      m_Ctx[i].initFromP1AndShiftIdx(std::get<0>(initCtx[i]),
                                     std::get<1>(initCtx[i]));
    }
  }

  void initCtx(unsigned numCtx, double pInit, uint8_t shiftInit){
    m_Ctx.resize(numCtx);
    for (int i = 0; i < numCtx; ++i) {
      m_Ctx[i].initFromP1AndShiftIdx(pInit, shiftInit);
    }
  }
  
}; // class cabacDecoder
#endif  // RWTH_PYTHON_IF



#if RWTH_PYTHON_IF

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
    uint64_t decodeBinsBI(const std::vector<unsigned int> & ctxIds, const unsigned int numBins) {
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
    uint64_t decodeBinsTU(const std::vector<unsigned int> & ctxIds, const unsigned int numMaxBins=512) {
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
        unsigned int i = 0;
        while (decodeBinsBIbypass(1) == 0) {
            i++;
        }
        if (i != 0) {
            bins = (1u << i) | decodeBinsEP(i);
        } else {
            return 0;
        }
        return static_cast<uint64_t>(bins - 1);
    }

    // ---------------------------------------------------------------------------------------------------------------------
    // Taken from GABAC/GENIE
    uint64_t decodeBinsEG0(const std::vector<unsigned int> & ctxIds) {

        // Prefix
        unsigned int i = 0;
        while (decodeBin(ctxIds[i]) == 0) {
            i++;
        }

        // Suffix
        unsigned int bins = 0;
        if (i != 0) {
            bins = (1u << i) | decodeBinsEP(i);
        } else {
            return 0;
        }
        return static_cast<uint64_t>(bins - 1);
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

    uint64_t decodeBinsEGk(unsigned k, const std::vector<unsigned int> & ctxIds) {

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

}; // class cabacSymbolDecoder

class cabacSimpleSequenceDecoder : public cabacSymbolDecoder{
  public:
    cabacSimpleSequenceDecoder(std::vector<uint8_t> bs) : cabacSymbolDecoder(bs){}

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsTUorder1(uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxBins=512) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxBins, 0);
      contextSelector::getContextIdsOrder1TU(ctxIds, symbolPrev, restPos, numMaxBins);

      // Decode symbol
      return decodeBinsTU(ctxIds, numMaxBins);
    }

    std::vector<uint64_t> decodeSymbolsTUorder1(unsigned int numSymbols, unsigned int restPos=10, unsigned int numMaxBins=512) {
      std::vector<uint64_t> symbols(numSymbols, 0);
      uint64_t symbolPrev = 0;
      std::vector<unsigned int> ctxIds(numMaxBins, 0);

      for (unsigned int n = 0; n < numSymbols; n++) {
        // Get context ids for each bin
        if(n > 0) {
          symbolPrev = symbols[n-1];
        }
        contextSelector::getContextIdsOrder1TU(ctxIds, symbolPrev, restPos, numMaxBins);

        // Decode bins
        symbols[n] = decodeBinsTU(ctxIds, numMaxBins);
      }

      return symbols;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEG0order1(uint64_t symbolPrev, unsigned int restPos=10, unsigned int numMaxPrefixBins=24) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
      contextSelector::getContextIdsOrder1EG0(ctxIds, symbolPrev, restPos, numMaxPrefixBins);

      // Decode bins
      return decodeBinsEG0(ctxIds);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    std::vector<uint64_t> decodeSymbolsEG0order1(unsigned int numSymbols, unsigned int restPos=10, unsigned int numMaxPrefixBins=512) {
      std::vector<uint64_t> symbols(numSymbols, 0);
      uint64_t symbolPrev = 0;
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);

      for (unsigned int n = 0; n < numSymbols; n++) {
        // Get context ids for each bin
        if(n > 0) {
          symbolPrev = symbols[n-1];
        }
        contextSelector::getContextIdsOrder1EG0(ctxIds, symbolPrev, restPos, numMaxPrefixBins);

        // Decode bins
        symbols[n] = decodeBinsEG0(ctxIds);
      }

      return symbols;
    }

        // ---------------------------------------------------------------------------------------------------------------------

    uint64_t decodeBinsEGkorder1(uint64_t symbolPrev, unsigned int k, unsigned int restPos=10, unsigned int numMaxPrefixBins=24) {
      // Get context ids
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);
      contextSelector::getContextIdsOrder1EGk(ctxIds, symbolPrev, k, restPos, numMaxPrefixBins);

      // Decode bins
      return decodeBinsEGk(k, ctxIds);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    std::vector<uint64_t> decodeSymbolsEGkorder1(unsigned int numSymbols, unsigned int k, unsigned int restPos=10, unsigned int numMaxPrefixBins=512) {
      std::vector<uint64_t> symbols(numSymbols, 0);
      uint64_t symbolPrev = 0;
      std::vector<unsigned int> ctxIds(numMaxPrefixBins, 0);

      for (unsigned int n = 0; n < numSymbols; n++) {
        // Get context ids for each bin
        if(n > 0) {
          symbolPrev = symbols[n-1];
        }
        contextSelector::getContextIdsOrder1EGk(ctxIds, symbolPrev, k, restPos, numMaxPrefixBins);

        // Decode bins
        symbols[n] = decodeBinsEGk(k, ctxIds);
      }

      return symbols;
    }

}; // class cabacSimpleSequenceDecoder

#endif  // RWTH_PYTHON_IF