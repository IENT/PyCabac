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

class cabacSimpleSequenceDecoder : public cabacDecoder{
  public:
    cabacSimpleSequenceDecoder(std::vector<uint8_t> bs) : cabacDecoder(bs){}

    // GABAC/GENIE stuff from here

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsBIbypass(const unsigned int num_bins) {
        return decodeBinsEP(num_bins);
    }

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsBI(const std::vector<unsigned int> & ctx_ids, const unsigned int num_bins) {
        unsigned int bins = 0;
        for (size_t i = num_bins; i > 0; i--) {
            bins = (bins << 1u) | decodeBin(ctx_ids[i]);
        }
        return bins;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsTUbypass(const unsigned int num_max_bins=512) {
        unsigned int i = 0;
        while (i < num_max_bins) {
            if (decodeBinsEP(1) == 0) break;
            i++;
        }
        return i;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsTU(const std::vector<unsigned int> & ctx_ids, const unsigned int num_max_bins=512) {
        unsigned int i = 0;

        while (i < num_max_bins) {
            if (decodeBin(ctx_ids[i]) == 0) break;
            i++;
        }
        return i;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsEG0bypass() {
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
        return bins - 1;
    }

    // ---------------------------------------------------------------------------------------------------------------------

    unsigned decodeBinsEG0(const std::vector<unsigned int> & ctx_ids) {

        // Prefix
        unsigned int i = 0;
        while (decodeBin(ctx_ids[i]) == 0) {
            i++;
        }

        // Suffix
        unsigned int bins = 0;
        if (i != 0) {
            bins = (1u << i) | decodeBinsEP(i);
        } else {
            return 0;
        }
        return bins - 1;
    }
}; // class cabacSimpleSequenceDecoder

#endif  // RWTH_PYTHON_IF