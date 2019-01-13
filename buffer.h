#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <ptlib.h>
#include "version.h"

class AudioBuffer : public PObject
{
	PCLASSINFO(AudioBuffer,PObject);
public:
	AudioBuffer();
	~AudioBuffer();	
	void Write(const BYTE * ptr, PINDEX amount);
	void Read(BYTE * ptr, PINDEX amount);
	void ReadAndMix(BYTE * data, PINDEX amount, PINDEX channels);
private:
	void Mix(BYTE * dst, const BYTE * src, PINDEX count, PINDEX channels);

	BYTE * buffer;
	PINDEX bufferLen;
	PINDEX bufferStart;
	PINDEX bufferSize;
	PMutex audioBufferMutex;
};

PDICTIONARY(AudioBufferDict,PString,AudioBuffer);

#if Video
class VideoBuffer : public PObject
{
	PCLASSINFO(VideoBuffer,PObject);
public:
	VideoBuffer();
	~VideoBuffer();
	
	void WriteAll(BYTE * ptr,PINDEX amount);
	void Write(BYTE * ptr,PINDEX amount,PINDEX posn);
	void Read(BYTE * ptr, PINDEX amount);
	void Clear(PINDEX posn);
	void SetSize(int x, int y);

private:
	BYTE * buffer;
	PINDEX videoBufferSize;
	PMutex VideoBufferMutex;
	int	   bufferFrameSize;
	int    xSize;
	int    ySize;
};

PDICTIONARY(VideoBufferDict,PString,VideoBuffer);

#endif

#endif