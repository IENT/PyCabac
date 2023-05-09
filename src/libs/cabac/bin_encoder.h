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

#include "bitstream.h"
#include "contexts.h"
#include "vector"
#include <tuple>
#include <list>
#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
#include <cassert>

#if RWTH_PYTHON_IF
#include "context_selector.h"
#endif

#if !RWTH_PYTHON_IF
class BinStore
{
public:
  BinStore () : m_inUse(false), m_allocated(false)  {}
  ~BinStore()                                       {}

  void  reset   ()
  {
    if( m_inUse )
    {
      for( unsigned n = 0; n < Ctx::NumberOfContexts; n++ )
      {
        m_binBuffer[n].clear();
      }
    }
  }
  void  addBin  ( unsigned bin, unsigned ctxId )
  {
    if( m_inUse )
    {
      std::vector<bool>& binBuffer = m_binBuffer[ctxId];
      if( binBuffer.size() < m_maxNumBins )
      {
        binBuffer.push_back( bin == 1 );
      }
    }
  }

  void                      setUse      ( bool useStore )         { m_inUse = useStore; if(m_inUse){xCheckAlloc();} }
  bool                      inUse       ()                  const { return m_inUse; }
  const std::vector<bool>&  getBinVector( unsigned ctxId )  const { return m_binBuffer[ctxId]; }

private:
  void  xCheckAlloc()
  {
    if( !m_allocated )
    {
      m_binBuffer.resize( Ctx::NumberOfContexts );
      for( unsigned n = 0; n < Ctx::NumberOfContexts; n++ )
      {
        m_binBuffer[n].reserve( m_maxNumBins );
      }
      m_allocated = true;
    }
  }

private:
  static const std::size_t          m_maxNumBins = 100000;
  bool                              m_inUse;
  bool                              m_allocated;
  std::vector< std::vector<bool> >  m_binBuffer;
};
#endif

#if !RWTH_PYTHON_IF
class BinEncIf : public Ctx
{
protected:
  template <class BinProbModel>
  BinEncIf( const BinProbModel* dummy ) : Ctx( dummy ) {}
public:
  virtual ~BinEncIf() {}
public:
  virtual void      init              ( OutputBitstream* bitstream )        = 0;
  virtual void      uninit            ()                                    = 0;
  virtual void      start             ()                                    = 0;
  virtual void      finish            ()                                    = 0;
  virtual void      restart           ()                                    = 0;
  virtual void      reset             ( int qp, int initId )                = 0;
public:
  virtual void      resetBits         ()                                    = 0;
  virtual uint64_t  getEstFracBits    ()                              const = 0;
#if !RWTH_PYTHON_IF
  virtual unsigned  getNumBins        ( unsigned    ctxId )           const = 0;
#endif
public:
  virtual void      encodeBin         ( unsigned bin,   unsigned ctxId    ) = 0;
  virtual void      encodeBinEP       ( unsigned bin                      ) = 0;
  virtual void      encodeBinsEP      ( unsigned bins,  unsigned numBins  ) = 0;
  virtual void      encodeRemAbsEP    ( unsigned bins,
                                        unsigned goRicePar,
                                        unsigned cutoff,
                                        int      maxLog2TrDynamicRange    ) = 0;
  virtual void      encodeBinTrm      ( unsigned bin                      ) = 0;
  virtual void      align             ()                                    = 0;
public:
#if !RWTH_PYTHON_IF
  virtual uint32_t  getNumBins        ()                                    = 0;
#endif
  virtual bool      isEncoding        ()                                    = 0;
  virtual unsigned  getNumWrittenBits ()                                    = 0;
public:
#if !RWTH_PYTHON_IF
  virtual void            setBinStorage     ( bool b )                      = 0;
  virtual const BinStore* getBinStore       ()                        const = 0;
  virtual BinEncIf*       getTestBinEncoder ()                        const = 0;
#endif
};
#endif


#if !RWTH_PYTHON_IF
class BinCounter
{
public:
  BinCounter();
  ~BinCounter() {}
public:
  void      reset   ();
  void      addCtx  ( unsigned ctxId )          { m_NumBinsCtx[ctxId]++; }
  void      addEP   ( unsigned num   )          { m_NumBinsEP+=num; }
  void      addEP   ()                          { m_NumBinsEP++; }
  void      addTrm  ()                          { m_NumBinsTrm++; }
  uint32_t  getAll  ()                  const;
  uint32_t  getCtx  ( unsigned ctxId )  const   { return m_NumBinsCtx[ctxId]; }
  uint32_t  getEP   ()                  const   { return m_NumBinsEP; }
  uint32_t  getTrm  ()                  const   { return m_NumBinsTrm; }
private:
  std::vector<uint32_t> m_CtxBinsCodedBuffer;
  uint32_t*             m_NumBinsCtx;
  uint32_t              m_NumBinsEP;
  uint32_t              m_NumBinsTrm;
};
#endif

#if RWTH_PYTHON_IF
class BinEncoderBase
#else 
class BinEncoderBase : public BinEncIf, public BinCounter
#endif
{
protected:
  template <class BinProbModel>
  BinEncoderBase ( const BinProbModel* dummy );
public:
  ~BinEncoderBase() {}
public:
  void      init    ( OutputBitstream* bitstream );
  void      uninit  ();
  void      start   ();
  void      finish  ();
  void      restart ();
  void      reset   ( int qp, int initId );
public:
  void      resetBits           ();
  uint64_t  getEstFracBits      ()                    const { THROW( "not supported" ); return 0; }
#if !RWTH_PYTHON_IF
  unsigned  getNumBins          ( unsigned ctxId )    const { return BinCounter::getCtx(ctxId); }
#endif
public:
  void      encodeBinEP         ( unsigned bin                      );
  void      encodeBinsEP        ( unsigned bins,  unsigned numBins  );
  void      encodeRemAbsEP      ( unsigned bins,
                                  unsigned goRicePar,
                                  unsigned cutoff,
                                  int      maxLog2TrDynamicRange    );
  void      encodeBinTrm        ( unsigned bin                      );
  void      align               ();
  unsigned  getNumWrittenBits   () { return ( m_Bitstream->getNumberOfWrittenBits() + 8 * m_numBufferedBytes + 23 - m_bitsLeft ); }
public:
#if !RWTH_PYTHON_IF
  uint32_t  getNumBins          ()                          { return BinCounter::getAll(); }
#endif
  bool      isEncoding          ()                          { return true; }
protected:
  void      encodeAlignedBinsEP ( unsigned bins,  unsigned numBins  );
  void      writeOut            ();
protected:
  OutputBitstream*        m_Bitstream;
  uint32_t                m_Low;
  uint32_t                m_Range;
  uint32_t                m_bufferedByte;
  int32_t                 m_numBufferedBytes;
  int32_t                 m_bitsLeft;
#if !RWTH_PYTHON_IF
  BinStore                m_BinStore;
#endif
#if RWTH_ENABLE_TRACING
 protected:
  std::vector<std::list<std::pair<uint16_t, uint8_t>>> m_pAndMpsTrace;
#endif

};



template <class BinProbModel>
class TBinEncoder : public BinEncoderBase
{
public:
  TBinEncoder ();
  ~TBinEncoder() {}
  void  encodeBin   ( unsigned bin, unsigned ctxId );
public:
#if !RWTH_PYTHON_IF
  void            setBinStorage     ( bool b )          { m_BinStore.setUse(b); }
  const BinStore* getBinStore       ()          const   { return &m_BinStore; }
  BinEncIf*       getTestBinEncoder ()          const;
#endif
private:
#if RWTH_PYTHON_IF
  friend class cabacEncoder;
  std::vector<BinProbModel> m_Ctx;
#else
  CtxStore<BinProbModel>& m_Ctx;
#endif
};

typedef TBinEncoder  <BinProbModel_Std>   BinEncoder_Std;

template class TBinEncoder<BinProbModel_Std>;

#if RWTH_PYTHON_IF
class cabacEncoder : public BinEncoder_Std {
public:
  cabacEncoder() { m_Bitstream = new OutputBitstream; }
  ~cabacEncoder() { delete m_Bitstream; }

  void initCtx(const std::vector<std::tuple<double, uint8_t>> initCtx) {
    m_Ctx.resize(initCtx.size());
    for (int i = 0; i < initCtx.size(); ++i) {
      m_Ctx[i].initFromP1AndShiftIdx(std::get<0>(initCtx[i]),
                                     std::get<1>(initCtx[i]));
    }
#if RWTH_ENABLE_TRACING
    m_pAndMpsTrace.resize(initCtx.size());
#endif
  }

  void initCtx(unsigned numCtx, double pInit, uint8_t shiftInit){
    m_Ctx.resize(numCtx);
    for (int i = 0; i < numCtx; ++i) {
      m_Ctx[i].initFromP1AndShiftIdx(pInit, shiftInit);
    }
#if RWTH_ENABLE_TRACING
    m_pAndMpsTrace.resize(numCtx);
#endif
  }

  void writeByteAlignment() { m_Bitstream->writeByteAlignment(); }

  std::vector<uint8_t> getBitstream() {
    uint8_t *byteStream = m_Bitstream->getByteStream();
    std::vector<uint8_t> byteVector;
    for (int i = 0; i < m_Bitstream->getByteStreamLength(); i++) {
      byteVector.push_back(*byteStream);
      byteStream++;
    }
    return byteVector;
  }

#if RWTH_ENABLE_TRACING
  std::vector<std::list<std::pair<uint16_t, uint8_t>>> getTrace() {
    return m_pAndMpsTrace;
  }
#endif
#endif
}; // class cabacEncoder 



#if RWTH_PYTHON_IF

class cabacSimpleSequenceEncoder : public cabacEncoder{
public:
  cabacSimpleSequenceEncoder() : cabacEncoder(){}


  void encodeSymbolsTU(std::vector<unsigned> symbols, unsigned int restPos=10, unsigned int num_max_bins=24){

    unsigned int symbolPrev = 0;

    std::vector<unsigned int> ctxIdxs(num_max_bins, 0);

    for (unsigned int n = 0; n < symbols.size(); n++) {
      if(n > 0){
        symbolPrev = symbols[n - 1];
      } 
      context_selector::getContextIdsOrder1TU(ctxIdxs, symbolPrev, restPos, num_max_bins);
      encodeBinsTU(symbols[n], ctxIdxs, num_max_bins);
    }
  }

  // GABAC/GENIE stuff from here

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsBIbypass(unsigned input, const unsigned int num_bins) {
    encodeBinsEP(input, num_bins);
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsBI(unsigned int input, const std::vector<unsigned int>& ctx_ids, const unsigned int num_bins) {
    unsigned int bin = 0;  // bin to encode
    unsigned int i = 0;  // counter for context selection
    for (int exponent = num_bins - 1; exponent >= 0; exponent--) {  // i must be signed
      // 0x1u is the same as 0x1. (The u stands for unsigned.). i & 0x1u is the same as i % 2?
      bin = static_cast<unsigned int>(input >> static_cast<unsigned>(exponent)) & 0x1u;
      encodeBin(bin, ctx_ids[i]);
      i++;
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsTUbypass(unsigned input, const unsigned int num_max_bins=512) {
    for (unsigned i = 0; i < input; i++) {
      encodeBinEP(1);
    }
    if (num_max_bins > input) {  // input == num_max_bins is coded as all 1s
      encodeBinEP(0);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsTU(unsigned input, const std::vector<unsigned int>& ctx_ids, const unsigned int num_max_bins=512) {
    //assert(ctx_ids.size() <= num_max_bins);

    unsigned int i;
    for (i = 0; i < input; i++) {
      encodeBin(1, ctx_ids[i]);
    }
    if (input < num_max_bins) {  // input == num_max_bins is coded as all 1s
      encodeBin(0, ctx_ids[i++]);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEG0bypass(unsigned input) {
    auto valuePlus1 = (unsigned int)(input + 1);
    auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

    /* prefix */
    encodeBinsBIbypass(1, numLeadZeros + 1);
    if (numLeadZeros) {
      /* suffix */
      encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
    }
  }

  // ---------------------------------------------------------------------------------------------------------------------

  void encodeBinsEG0(unsigned input, const std::vector<unsigned int>& ctx_ids) {
    auto valuePlus1 = (unsigned int)(input + 1);
    auto numLeadZeros = (unsigned int)floor(log2(valuePlus1));

    assert(ctx_ids.size() >= (numLeadZeros + 1));

    /* prefix */
    encodeBinsBI(1, ctx_ids, numLeadZeros + 1);
    if (numLeadZeros) {
        /* suffix */
      encodeBinsBIbypass(valuePlus1 & ((1u << numLeadZeros) - 1), numLeadZeros);
    }
  }
}; // class cabacSimpleSequenceEncoder

#endif // RWTH_PYTHON_IF