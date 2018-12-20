#include "channel.h"

#if 0
// ***********************************************************************
NullChannel::NullChannel(): isOpen(true)
{
	cout << "Creating NULL channel" << endl;
}
// ***********************************************************************
NullChannel::~NullChannel()
{
	cout << "Deleting NULL channel" << endl;
}
// ***********************************************************************
bool NullChannel::Close()
{
	isOpen = false;
	return true;
}
// ***********************************************************************
bool NullChannel::Write(const void *buf, PINDEX len)
{
	lastWriteCount = len;
	writeDelay.Delay(len/2/8);
	return true;
}
// ***********************************************************************
bool NullChannel::Read(void *buf, PINDEX len)
{
	memset(buf, 0, len);
	lastReadCount = len;
	readDelay.Delay(len/2/8);
	return true;	
}
#endif
// ***********************************************************************
MCUAudioChannel::MCUAudioChannel(MyH323EndPoint & _ep, MyH323Connection & _conn) : ep(_ep) , conn(_conn)
{
	isOpen = true;
}
// ***********************************************************************
MCUAudioChannel::~MCUAudioChannel()
{
	isOpen = false;
}
// ***********************************************************************
bool MCUAudioChannel::Write(const void * buffer, PINDEX len)
{
	PWaitAndSignal mutexW(audioChanMutex);
	if(!IsOpen())
		return false;
	Delay.Delay(len/2/8);
	lastWriteCount = len;
	return conn.OnIncomingAudio(buffer, len);
}
// ***********************************************************************
bool MCUAudioChannel::Close()
{
	PWaitAndSignal mutexA(audioChanMutex);
	isOpen = false;
	return true;
}
// ***********************************************************************
bool MCUAudioChannel::Read(void * buffer, PINDEX len)
{
	PWaitAndSignal mutexR(audioChanMutex);
	if(!IsOpen())
		return false;
	Delay.Delay(len/2/8);
	lastReadCount = len;
	return conn.OnOutgoingAudio(buffer, len);
}