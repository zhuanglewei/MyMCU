#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <ptlib.h>
#include <h323.h>
#include <ptclib/pwavfile.h>
#include <ptclib/delaychan.h>

// ***********************************************************************
class NullChannel: public PIndirectChannel
{
  PCLASSINFO(NullChannel, PIndirectChannel);
	PAdaptiveDelay writeDelay;
	PAdaptiveDelay readDelay;
	bool isOpen;
public:
	NullChannel();
	~NullChannel();
	virtual bool Close(); 
	virtual bool IsOpen() const { return isOpen; };
	virtual bool Read(void *buf, PINDEX len);
	virtual bool Write(const void *buf, PINDEX len); 
};



#endif