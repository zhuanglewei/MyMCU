#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <ptlib.h>

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

#endif