#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <ptlib.h>
#include <h323.h>
//#include <ptclib/pwavfile.h>
#include <ptclib/delaychan.h>
#include "ep.h"
#include "connection.h"

class MyH323EndPoint;
class MyH323Connection;

// ***********************************************************************
#if 0
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
class MCUAudioChannel : public PChannel
{
	PCLASSINFO(MCUAudioChannel,PChannel);
public:
	MCUAudioChannel(MyH323EndPoint & _ep, MyH323Connection & _conn);
	~MCUAudioChannel();
	virtual bool Read(void * buffer, PINDEX len);
	virtual bool Write(const void * buffer, PINDEX len);
	virtual bool Close();
	virtual bool IsOpen() const { return isOpen; };
private:
	bool isOpen;
	MyH323EndPoint & ep;
	MyH323Connection & conn;
	PMutex audioChanMutex;
	PAdaptiveDelay Delay;	
};

#endif
