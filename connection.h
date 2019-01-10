#ifndef _CONNECTION_H_
#define _CONNECTION_H_


#include <ptlib.h>
#include <h323.h>
#include "ep.h"
#include "buffer.h"
#include "channel.h"

class MCUAudioChannel;
class MyH323EndPoint;

class MyH323Connection : public H323Connection
{
    PCLASSINFO(MyH323Connection, H323Connection);

  public:
    MyH323Connection(MyH323EndPoint & _ep, unsigned ref);

    virtual PBoolean OnStartLogicalChannel(H323Channel &);
    virtual void OnUserInputString(const PString &);

    virtual bool OnSendSignalSetup(H323SignalPDU & callProceedingPDU);
    virtual AnswerCallResponse OnAnswerCall(const PString &, const H323SignalPDU &, H323SignalPDU &);
    virtual bool OpenAudioChannel(bool isEncoding, unsigned bufferSize, H323AudioCodec & codec);

    bool OnIncomingAudio(const void * buffer, PINDEX len);
    void AddMember(const PString & token);
    void RemoveMember(const PString & token);


    bool ReadAudio(void * buffer, PINDEX len);
    bool WriteAudio(const PString & token, const void * buffer, PINDEX len);
    
    PString  GetRoomID() {  return RoomID; }
    AudioBufferDict & GetAudioBuffers() { return audioBuffers; } 
private:
    AudioBufferDict audioBuffers;
	MCUAudioChannel * incomingAudio;
	MCUAudioChannel * outgoingAudio;
	
	PMutex audioMutex;
	MyH323EndPoint & ep;

    PString RoomID;
    int identify;
    int Listen_status;
};

#endif