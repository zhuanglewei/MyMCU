#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <ptlib.h>
#include <h323.h>
#include <ptclib/pwavfile.h>
#include <ptclib/delaychan.h>
#include "ep.h"
#include "connection.h"

class MyH323EndPoint;
class MyH323Connection;

// ***********************************************************************



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
	bool ReadBufferAudio(void * buffer, PINDEX amount);
	bool WriteMemberAudio(const void * buffer, PINDEX len);
	bool WriteBufferAudio(const PString & token, const void * buffer, PINDEX len,MyH323Connection * member);
	PString * thisToken;
	bool isOpen;
	MyH323EndPoint & ep;
	MyH323Connection & conn;
	PMutex audioChanMutex;
	PAdaptiveDelay Delay;	
};

#endif
