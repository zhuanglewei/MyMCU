#include "connection.h"
#include "ep.h"

MyH323Connection::MyH323Connection(MyH323EndPoint & _ep, unsigned ref)
  : H323Connection(_ep, ref), ep(_ep) 
{
  incomingAudio = NULL;
  outgoingAudio = NULL;
}

// ***********************************************************************

PBoolean MyH323Connection::OnStartLogicalChannel(H323Channel & channel)
{
  if (!H323Connection::OnStartLogicalChannel(channel))
    return FALSE;

  cout << "Started logical channel: ";

  switch (channel.GetDirection()) {
    case H323Channel::IsTransmitter :
      cout << "sending ";
      break;

    case H323Channel::IsReceiver :
      cout << "receiving ";
      break;

    default :
      break;
  }

  cout << channel.GetCapability() << endl;  

  return TRUE;
}

// ***********************************************************************

void MyH323Connection::OnUserInputString(const PString & value)
{
  cout << "User input received: \"" << value << '"' << endl;
  if(value == "create"){
    RoomID = this->GetRemotePartyName();
    ep.AddMember(this);
    cout << "创建房间" << RoomID << endl;
  }
}

// ***********************************************************************

void MyH323Connection::AddMember(const PString & token)
{
   audioMutex.Wait();
   cout << "Adding audio buffer for " << token 
        << " to connection " << GetCallToken() << endl;

   audioBuffers.SetAt(token, new AudioBuffer);
   audioMutex.Signal();
}

// ***********************************************************************

void MyH323Connection::RemoveMember(const PString & token)
{
  PWaitAndSignal mutex(audioMutex);

  cout << "Removing audio buffer for " << token 
       << " from connection " << GetCallToken() << endl;

  audioBuffers.RemoveAt(token);  
}

// ***********************************************************************

bool MyH323Connection::OnSendSignalSetup( H323SignalPDU & callProceedingPDU )
{
  cout << "Adding connection to room " << endl;
  ep.AddMember(this);
  return H323Connection::OnSendSignalSetup( callProceedingPDU );
}

// ***********************************************************************

H323Connection::AnswerCallResponse
     MyH323Connection::OnAnswerCall(const PString & caller,
                                    const H323SignalPDU & setupPDU,
                                    H323SignalPDU & /*connectPDU*/)
{
//  ep.AddMember(this);

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