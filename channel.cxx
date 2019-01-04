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
	return WriteMemberAudio(buffer, len);
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
	return ReadBufferAudio(buffer, len);
}

bool MCUAudioChannel::ReadBufferAudio(void * buffer, PINDEX amount)
{
//  PWaitAndSignal mutex(audioMutex);
  
  memset(buffer, 0, amount);

  PINDEX numChannels = conn.audioBuffers.GetSize();
  if (numChannels== 0) 
    return TRUE;

  PINDEX i;
  for (i = 0; i < numChannels; i++) {
    PString key = conn.audioBuffers.GetKeyAt(i);
    conn.audioBuffers[key].ReadAndMix((BYTE *)buffer, amount, numChannels);
  }
  return TRUE;
}

bool MCUAudioChannel::WriteMemberAudio(const void * buffer, PINDEX len)//获取列表成员
{
	PString thisToken = conn.GetCallToken();

	PINDEX i;
    for (i = 0; i < ep.memberList.GetSize(); i++) {
    	
    	PString token = ep.memberList[i];
    	
    	if (token != thisToken) {
      		MyH323Connection * member = (MyH323Connection *)ep.FindConnectionWithLock(token);
      		if (member != NULL) { 
				WriteBufferAudio(thisToken, buffer, len, member);
        	member->Unlock();
        	}
    	} 
    	else
    		continue;
    		     
    }

    return TRUE;
    

}

bool MCUAudioChannel::WriteBufferAudio(const PString & token, const void * buffer, PINDEX len,MyH323Connection * member)
{

  AudioBuffer * audioBuffer = member->audioBuffers.GetAt(token);
  if (audioBuffer != NULL)
    audioBuffer->Write((BYTE *)buffer, len);
  return TRUE;
}