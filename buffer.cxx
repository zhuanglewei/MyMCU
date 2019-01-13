#include "buffer.h"

#define PCM_PACKET_LEN          320
#define PCM_BUFFER_LEN          (PCM_PACKET_LEN * 2)
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
    memset(data, 0, amount); //如果buffer不存在数据，返回静音
    return;
  }
  // 读取数据大小大于buffer存在数据，将buffer数据后以0补齐
  if (amount > bufferLen) 
    memset(data + bufferLen, 0, amount - bufferLen);
  // 
  PINDEX copyLeft = PMIN(amount, bufferLen);

  // 如果buffer区正在更新，则获取第一部分
  if ((bufferStart + copyLeft) > bufferSize) {
    PINDEX toCopy = bufferSize - bufferStart;
    memcpy(data, buffer + bufferStart, toCopy);
    data        += toCopy;
    bufferLen   -= toCopy;
    copyLeft    -= toCopy;
    bufferStart = 0;
  }
  // 获取buffer区的其余部分
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
    return;
  }

  // only mix up to the amount of data remaining
  PINDEX copyLeft = PMIN(amount, bufferLen);

  // 如果buffer区被划分为首尾连段
  if ((bufferStart + copyLeft) > bufferSize) {
    PINDEX toCopy = bufferSize - bufferStart;

    Mix(data, buffer + bufferStart, toCopy, channels);

    data        += toCopy;
    bufferLen   -= toCopy;
    copyLeft    -= toCopy;
    bufferStart = 0;
  }

  // 获取剩余的buffer区
  if (copyLeft > 0) {

    Mix(data, buffer + bufferStart, copyLeft, channels);

    bufferLen -= copyLeft;
    bufferStart = (bufferStart + copyLeft) % bufferSize;
  }
}

void AudioBuffer::Write(const BYTE * data, PINDEX amount)
{
  if (amount == 0)
    return;

  PWaitAndSignal mutex(audioBufferMutex);
  
  // 如果没有足够空间存储数据
  PINDEX newLen = bufferLen + amount;
  if (newLen > bufferSize) {
    PINDEX toRemove = newLen - bufferSize;
    bufferStart = (bufferStart + toRemove) % bufferSize;
    bufferLen -= toRemove;
  }
  //将数据复制到新数据的末尾，直到buffer区的末尾
  PINDEX copyStart = (bufferStart + bufferLen) % bufferSize;
  if ((copyStart + amount) > bufferSize) {
    PINDEX toCopy = bufferSize - copyStart;
    memcpy(buffer + copyStart, data, toCopy);
    copyStart = 0;
    data      += toCopy;
    amount    -= toCopy;
    bufferLen += toCopy;
  }
  // 复制剩余数据
  if (amount > 0) {
    memcpy(buffer + copyStart, data, amount);
    bufferLen   += amount;
  }
}

void AudioBuffer::Mix(BYTE * dst, const BYTE * src, PINDEX count, PINDEX /*channel*/)
{
  int const MAX = 32767;
  int const MIN =-32767;
  double f = 1;
  PINDEX i;
  for(i=0; i<count; i+=2){
    int srcVal = *(short *)src;
    int dstVal = *(short *)dst;
    int output = dstVal + srcVal;
    output = (int)(output * f);
    if(output > MAX){
      f = (double)MAX / (double)(output);
      output = MAX;
    }
    if(output < MIN){
      f = (double)MIN / (double)(output);
      output = MIN;
    }
    if(f<1)
      f += ((double)1-f)/(double)32;
    *(short *)dst = (short)output;
    dst += 2;
    src += 2;
  }
}

#if Video

#endif