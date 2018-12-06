#include "channel.h"


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
// ***********************************************************************
IncomingAudio::IncomingAudio(MyH323EndPoint & _ep, MyH323Connection & _conn) : ep(_ep) , conn(_conn)
{
	isOpen = true;
}
// ***********************************************************************
IncomingAudio::~IncomingAudio()
{
	isOpen = false;
}
// ***********************************************************************
bool IncomingAudio::Write(const void * buffer, PINDEX len)
{
	PWaitAndSignal mutexW(audioChanMutex);
	if(!IsOpen())
		return false;
	writeDelay.Delay(len/2/8);
	lastWriteCount = len;
	return conn.OnIncomingAudio(buffer, len);
}
// ***********************************************************************
bool IncomingAudio::Close()
{
	PWaitAndSignal mutexA(audioChanMutex);
	isOpen = false;
	return true;
}
// ***********************************************************************
OutgoingAudio::OutgoingAudio(MyH323EndPoint & _ep, MyH323Connection & _conn): ep(_ep), conn(_conn),isOpen(true)
{}
// ***********************************************************************
OutgoingAudio::~OutgoingAudio()
{}
// ***********************************************************************
bool OutgoingAudio::Read(void * buffer, PINDEX len)
{
	PWaitAndSignal mutexR(audioChanMutex);
	if(!IsOpen())
		return false;
	readDelay.Delay(len/2/8);
	lastReadCount = len;
	return conn.OnOutgoingAudio(buffer, len);
}
// ***********************************************************************
bool OutgoingAudio::Close()
{
	PWaitAndSignal mutexA(audioChanMutex);
	isOpen = false;
	return true;
}