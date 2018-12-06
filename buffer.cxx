#include "buffer.h"

#define PCM_PACKET_LEN          240
// size of a PCM data buffer, in bytes
#define PCM_BUFFER_LEN          (PCM_PACKET_LEN * 2)
// number of PCM buffers to keep
#define PCM_BUFFER_COUNT        2
#define PCM_BUFFER_SIZE         (PCM_BUFFER_LEN * PCM_BUFFER_COUNT)

AudioBuffer::AudioBuffer()
  : bufferSize(PCM_BUFFER_SIZE)
{
  buffer = new BYTE[bufferSize];
  bufferStart = bufferLen = 0;
}

AudioBuffer::~AudioBuffer()
{
  delete[] buffer;
}

void AudioBuffer::Read(BYTE * data, PINDEX amount)
{
  if (amount == 0)
    return;

  PWaitAndSignal mutex(audioBufferMutex);
  
  if (bufferLen == 0) {
    memset(data, 0, amount); // nothing in the buffer. return silence
    return;
  }

  // fill output data block with silence if audiobuffer is
  // almost empty.
  if (amount > bufferLen) 
    memset(data + bufferLen, 0, amount - bufferLen);

  // only copy up to the amount of data remaining
  PINDEX copyLeft = PMIN(amount, bufferLen);

  // if buffer is wrapping, get first part
  if ((bufferStart + copyLeft) > bufferSize) {
    PINDEX toCopy = bufferSize - bufferStart;

    memcpy(data, buffer + bufferStart, toCopy);

    data        += toCopy;
    bufferLen   -= toCopy;
    copyLeft    -= toCopy;
    bufferStart = 0;
  }

  // get the remainder of the buffer
  if (copyLeft > 0) {

    memcpy(data, buffer + bufferStart, copyLeft);

    bufferLen -= copyLeft;
    bufferStart = (bufferStart + copyLeft) % bufferSize;
  }
}

void AudioBuffer::ReadAndMix(BYTE * data, PINDEX amount, PINDEX channels)
{
  if (amount == 0)
    return;

  PWaitAndSignal mutex(audioBufferMutex);
  
  if (bufferLen == 0) {
    // nothing in the buffer to mix.
    return;
  }

  // only mix up to the amount of data remaining
  PINDEX copyLeft = PMIN(amount, bufferLen);

  // if buffer is wrapping, get first part
  if ((bufferStart + copyLeft) > bufferSize) {
    PINDEX toCopy = bufferSize - bufferStart;

    Mix(data, buffer + bufferStart, toCopy, channels);

    data        += toCopy;
    bufferLen   -= toCopy;
    copyLeft    -= toCopy;
    bufferStart = 0;
  }

  // get the remainder of the buffer
  if (copyLeft > 0) {

    Mix(data, buffer + bufferStart, copyLeft, channels);

    bufferLen -= copyLeft;
    bufferStart = (bufferStart + copyLeft) % bufferSize;
  }
}

void AudioBuffer::Mix(BYTE * dst, const BYTE * src, PINDEX count, PINDEX /*channels*/)
{
#if 0
  memcpy(dst, src, count);
#else
  PINDEX i;
  for (i = 0; i < count; i += 2) {

    int srcVal = *(short *)src;
    int dstVal = *(short *)dst;

    int newVal = dstVal;

#if 1     //The loudest person gains the channel.
#define mix_abs(x) ((x) >= 0 ? (x) : -(x))
    if (mix_abs(newVal) > mix_abs(srcVal))
      dstVal = newVal;
    else
      dstVal = srcVal; 
#else   //Just add up all the channels.
    if ((newVal + srcVal) > 0x7fff)
      dstVal = 0x7fff;
    else
      dstVal += srcVal;
#endif
    *(short *)dst = (short)dstVal;

    dst += 2;
    src += 2;
  }
#endif
}

void AudioBuffer::Write(const BYTE * data, PINDEX amount)
{
  if (amount == 0)
    return;

  PWaitAndSignal mutex(audioBufferMutex);
  
  // if there is not enough room for the new data, make room
  PINDEX newLen = bufferLen + amount;
  if (newLen > bufferSize) {
    PINDEX toRemove = newLen - bufferSize;
    bufferStart = (bufferStart + toRemove) % bufferSize;
    bufferLen -= toRemove;
  }

  // copy data to the end of the new data, up to the end of the buffer
  PINDEX copyStart = (bufferStart + bufferLen) % bufferSize;
  if ((copyStart + amount) > bufferSize) {
    PINDEX toCopy = bufferSize - copyStart;
    memcpy(buffer + copyStart, data, toCopy);
    copyStart = 0;
    data      += toCopy;
    amount    -= toCopy;
    bufferLen += toCopy;
  }

  // copy the rest of the data
  if (amount > 0) {
    memcpy(buffer + copyStart, data, amount);
    bufferLen   += amount;
  }
}