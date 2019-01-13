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

    virtual PBoolean OnStartLogicalChannel(H323Channel &);//开启逻辑通道回调函数
    virtual bool OpenAudioChannel(bool isEncoding, unsigned bufferSize, H323AudioCodec & codec);//开启音频通道
    virtual void OnUserInputString(const PString &);//交互信号处理

    virtual bool OnSendSignalSetup(H323SignalPDU & callProceedingPDU);
    virtual AnswerCallResponse OnAnswerCall(const PString &, const H323SignalPDU &, H323SignalPDU &);    
//AudioBuffer区管理
    void AddMember(const PString & token);//增加buffer区
    void RemoveMember(const PString & token);//删除buffer区
    AudioBufferDict & GetAudioBuffers() { return audioBuffers; }//获取音频buffer表 

    PString  GetRoomID() {  return RoomID; }//获取会议房间号
    void SetIdentify(int i);//设置用户身份标识符
private:
    AudioBufferDict audioBuffers;
	MCUAudioChannel * incomingAudio;
	MCUAudioChannel * outgoingAudio;
	
	PMutex audioMutex;
	MyH323EndPoint & ep;
    PString RoomID;
    int identify;
    bool speaker;
};

#endif