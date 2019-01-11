#include "connection.h"
#include "ep.h"

MyH323Connection::MyH323Connection(MyH323EndPoint & _ep, unsigned ref)
  : H323Connection(_ep, ref), ep(_ep),identify(0),speaker(false)
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
  PString order = value.Mid(3,value.GetSize()-2);
  if(value[0] == '-'){
    switch(value[1]){
      case 'c':
        if(RoomID != NULL)
          break; 
        if(ep.GetRoomIDList().Contains(RoomID)){
          this->SendUserInput("该房间名已存在，请选择新房间名或加入该房间.");
          break;
        }
        RoomID = value.Mid(3,value.GetSize()-2);
        ep.AddMember(this);
        this->SendUserInput("成功创建会议房间："+RoomID);
        break;
      case 'j':
        if(RoomID != NULL)
          break;
        i = value.Mid(3,value.GetSize()-2).AsInt64();
        if(i > ep.GetRoomIDList().GetSize()){
          this->SendUserInput("请选择正确的房间号");
          break;
        }
        RoomID = ep.GetRoomIDList().GetKeyAt(i);
        ep.AddMember(this);
        this->SendUserInput("成功加入会议房间："+RoomID);
        break;
      case 'R':
        if(RoomID == NULL)
          this->SendUserInput(ep.GetRoomNameList());
        else
          this->SendUserInput(ep.GetMemberName(RoomID));
        break;
      case 'y':

        break;
      default:
        this->SendUserInput("无法识别信息，请按以下格式发送：" + ep.GetHelpString());
        PTRACE(1,"收到无法识别信息：" << value);
        break;
    }
  }
  else{
    this->SendUserInput("无法识别信息，请按以下格式发送：" + ep.GetHelpString());
    PTRACE(1,"收到无法识别信息：" << value);
  }
  return;
}


void MyH323Connection::SetIdentify(int i)
{
  switch(i){
    case 0:
      identify = Listener;
      break;
    case 1:
      identify = Speaker;
      break;
    case 2:
      identify = ChairMan;
      break;
    default:
      identify = 0;
      break;
  }
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
  if (isEncoding) {
    if (outgoingAudio == NULL)
      outgoingAudio = new MCUAudioChannel(ep, *this);
    codec.AttachChannel(outgoingAudio, FALSE);
  } 
  else{
    if (incomingAudio == NULL)
      incomingAudio = new MCUAudioChannel(ep, *this);
    codec.AttachChannel(incomingAudio, FALSE);
  }
  return TRUE;
}

