/*
 *      Copyright (C) 2010-2012 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "DecBase.h"
#include "H264RandomAccess.h"

#include <map>

#define AVCODEC_MAX_THREADS 16
#define AVCODEC_USE_DR1 0
#define AVCODEC_DR1_DEBUG 0

typedef struct {
  REFERENCE_TIME rtStart;
  REFERENCE_TIME rtStop;
} TimingCache;

class CDecAvcodec : public CDecBase
{
public:
  CDecAvcodec(void);
  virtual ~CDecAvcodec(void);

  // ILAVDecoder
  STDMETHODIMP InitDecoder(AVCodecID codec, const CMediaType *pmt);
  STDMETHODIMP Decode(const BYTE *buffer, int buflen, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BOOL bSyncPoint, BOOL bDiscontinuity);
  STDMETHODIMP Flush();
  STDMETHODIMP EndOfStream();
  STDMETHODIMP GetPixelFormat(LAVPixelFormat *pPix, int *pBpp);
  STDMETHODIMP_(REFERENCE_TIME) GetFrameDuration();
  STDMETHODIMP_(BOOL) IsInterlaced();
  STDMETHODIMP_(const WCHAR*) GetDecoderName() { return L"avcodec"; }
  STDMETHODIMP HasThreadSafeBuffers() { return m_bDR1 ? S_OK : S_FALSE; }

  // CDecBase
  STDMETHODIMP Init();

protected:
  virtual HRESULT AdditionaDecoderInit() { return S_FALSE; }
  virtual HRESULT PostDecode() { return S_FALSE; }
  virtual HRESULT HandleDXVA2Frame(LAVFrame *pFrame) { return S_FALSE; }
  STDMETHODIMP DestroyDecoder();

private:
  STDMETHODIMP ConvertPixFmt(AVFrame *pFrame, LAVFrame *pOutFrame);

#if AVCODEC_USE_DR1
  static int lav_get_buffer(struct AVCodecContext *c, AVFrame *pic);
  static int lav_reget_buffer(struct AVCodecContext *c, AVFrame *pic);
  static void lav_release_buffer(struct AVCodecContext *c, AVFrame *pic);
  static void lav_frame_destruct(struct LAVFrame *);
#endif

protected:
  AVCodecContext       *m_pAVCtx;
  AVFrame              *m_pFrame;
  AVCodecID            m_nCodecId;
  BOOL                 m_bDXVA;

private:
  AVCodec              *m_pAVCodec;
  AVCodecParserContext *m_pParser;

  BYTE                 *m_pFFBuffer;
  BYTE                 *m_pFFBuffer2;
  int                  m_nFFBufferSize;
  int                  m_nFFBufferSize2;

  SwsContext           *m_pSwsContext;

  CH264RandomAccess    m_h264RandomAccess;

  BOOL                 m_bNoBufferConsumption;
  BOOL                 m_bHasPalette;

  // Timing settings
  BOOL                 m_bFFReordering;
  BOOL                 m_bCalculateStopTime;
  BOOL                 m_bRVDropBFrameTimings;
  BOOL                 m_bInputPadded;

  BOOL                 m_bBFrameDelay;
  TimingCache          m_tcBFrameDelay[2];
  int                  m_nBFramePos;

  TimingCache          m_tcThreadBuffer[AVCODEC_MAX_THREADS];
  int                  m_CurrentThread;

  REFERENCE_TIME       m_rtStartCache;
  BOOL                 m_bWaitingForKeyFrame;
  int                  m_iInterlaced;

  BOOL                 m_bDR1;
  BOOL                 m_bEndOfSequence;

#if AVCODEC_USE_DR1
  CCritSec             m_BufferCritSec;
  std::map<uint8_t*,int> m_Buffers;
#endif
};
