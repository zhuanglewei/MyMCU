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

class IncomingAudio : public PChannel
{
	PCLASSINFO(IncomingAudio,PChannel);
public:
	IncomingAudio(MyH323EndPoint & _ep, MyH323Connection & _conn);
	~IncomingAudio();

	virtual bool Write(const void * buffer, PINDEX len);
	virtual bool Close();
	virtual bool IsOpen() const { return isOpen; };
private:
	bool isOpen;
	MyH323EndPoint & ep;
	MyH323Connection & conn;
	PMutex audioChanMutex;
	PAdaptiveDelay writeDelay;	
};

class OutgoingAudio : public PChannel
{
	PCLASSINFO(OutgoingAudio, PChannel);
public:
	OutgoingAudio(MyH323EndPoint & _ep, MyH323Connection & _conn);
	~OutgoingAudio();
	virtual bool Read(void * buffer, PINDEX len);
	virtual bool Close();
	virtual bool IsOpen() { return isOpen; };
private:
	MyH323EndPoint & ep;
	MyH323Connection & conn;
	bool isOpen;
	PMutex audioChanMutex;
	PAdaptiveDelay readDelay;
};


#endif