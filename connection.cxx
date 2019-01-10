#include "connection.h"
#include "ep.h"

MyH323Connection::MyH323Connection(MyH323EndPoint & _ep, unsigned ref)
  : H323Connection(_ep, ref), ep(_ep),identify(0),Listen_status(0)
{
  incomingAudio = NULL;
  outgoingAudio = NULL;
}

// ***********************************************************************

PBoolean MyH323Connection::OnStartLogicalChannel(H323Channel & channel)
{
  if (!H323Connection::OnStartLogicalChannel(channel))
    return FALSE;
  PTRACE(1,"开启逻辑通道: " << ((channel.GetDirection()==H323Channel::IsTransmitter) ? (" sending "):(" receiving "))
                           <<  channel.GetCapability() );
  return TRUE;
}

// ***********************************************************************

void MyH323Connection::OnUserInputString(const PString & value)
{
  int i;
  if(value[0] == '-'){
    switch(value[1]){
      case 'c':
        if(RoomID != NULL)
          break; 
        RoomID = value.Mid(1,value.GetSize()-2);
        ep.AddMember(this);
        break;
      case 'j':
        if(RoomID != NULL)
          break;
        i = value.Mid(1,value.GetSize()-2).AsInt64();
        RoomID = ep.GetRoomIDList().GetKeyAt(i);
        ep.AddMember(this);
        break;
      case 'R':
        this->SendUserInput(ep.GetRoomNameList());
        break;
      case 'y':

        break;
      default:
        this->SendUserInput("无法识别信息，请按以下格式发送：" + ep.GetHelpString());
        PTRACE(1,"收到无法识别信息：" << value);
        break;
    }
  }
  return;
}

// ***********************************************************************

void MyH323Connection::AddMember(const PString & token)
{
   audioMutex.Wait();
   PTRACE(1,"在连接 " << GetCallToken() << " 添加属于 " << token << " 的音频buffer区.");
   audioBuffers.SetAt(token, new AudioBuffer);
   audioMutex.Signal();
}

// ***********************************************************************

void MyH323Connection::RemoveMember(const PString & token)
{
  PWaitAndSignal mutex(audioMutex);
  PTRACE(1,"在连接 " << GetCallToken() << " 删除属于 " << token << " 的音频buffer区.");
  audioBuffers.RemoveAt(token);  
}

// ***********************************************************************

bool MyH323Connection::OnSendSignalSetup( H323SignalPDU & callProceedingPDU )
{
//  cout << "Adding connection to room " << endl;
//  ep.AddMember(this);

  return H323Connection::OnSendSignalSetup( callProceedingPDU );
}

// ***********************************************************************

H323Connection::AnswerCallResponse
     MyH323Connection::OnAnswerCall(const PString & caller,
                                    const H323SignalPDU & setupPDU,
                                    H323SignalPDU & /*connectPDU*/)
{
  this->SendUserInput(ep.GetHelpString());
  return AnswerCallNow;
}

// ***********************************************************************

bool MyH323Connection::OpenAudioChannel(bool isEncoding, unsigned bufferSize, H323AudioCodec & codec)
{
  PWaitAndSignal mutex(audioMutex);
  if (incomingAudio == NULL){
    incomingAudio = new MCUAudioChannel(ep, *this);
  }

  if (outgoingAudio == NULL) {
    outgoingAudio = new MCUAudioChannel(ep, *this);
  }

  if (isEncoding) {
    codec.AttachChannel(outgoingAudio, FALSE);
  } 
  else{
    codec.AttachChannel(incomingAudio, FALSE);
  }

  return TRUE;
}

