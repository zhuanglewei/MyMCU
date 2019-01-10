#include "channel.h"

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
  if(conn.GetRoomID() == NULL)
    return true;
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

// ***********************************************************************

bool MCUAudioChannel::ReadBufferAudio(void * buffer, PINDEX amount)
{  
  memset(buffer, 0, amount);
  if(conn.GetRoomID() == NULL || ep.GetRoomIDList()[conn.GetRoomID()].GetSize()==1   )
    return true;

  AudioBufferDict & audioBuffers = conn.GetAudioBuffers();

  PINDEX numChannels = audioBuffers.GetSize();
  if (numChannels== 0) 
    return TRUE;
  PINDEX i;
  for (i = 0; i < numChannels; i++) {
    PString key = audioBuffers.GetKeyAt(i);
    audioBuffers[key].ReadAndMix((BYTE *)buffer, amount, numChannels);
  }
  return TRUE;
}

// ***********************************************************************

bool MCUAudioChannel::WriteMemberAudio(const void * buffer, PINDEX len)//向同个会议成员写入音频数据
{
	PString thisToken = conn.GetCallToken();
  PString RoomID = conn.GetRoomID();
  StringListDict  & memberListDict =  ep.GetRoomIDList();
  if(!memberListDict.Contains(RoomID))
    return false;
  PStringList memberList = memberListDict[RoomID];
	PINDEX i;
    for (i = 0; i < memberList.GetSize(); i++) { 	
    	PString token = memberList[i];
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
    return true;
}

// ***********************************************************************

bool MCUAudioChannel::WriteBufferAudio(const PString & token, const void * buffer, PINDEX len,MyH323Connection * member)//向连接中的指定token的buffer写入数据
{

  AudioBuffer * audioBuffer = member->GetAudioBuffers().GetAt(token);
  if (audioBuffer != NULL)
    audioBuffer->Write((BYTE *)buffer, len);
  return TRUE;
}

