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
}


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
bool MyH323Connection::WriteAudio(const PString & token, const void * buffer, PINDEX len)
{
  PWaitAndSignal mutex(audioMutex);
  AudioBuffer * audioBuffer = audioBuffers.GetAt(token);
  if (audioBuffer != NULL)
    audioBuffer->Write((BYTE *)buffer, len);
  return TRUE;
}
// ***********************************************************************
bool MyH323Connection::ReadAudio(const PString & /*token*/, void * buffer, PINDEX amount)
{
  PWaitAndSignal mutex(audioMutex);
  // First, set the buffer to empty.
  memset(buffer, 0, amount);
  // get number of channels to mix
  PINDEX numChannels = audioBuffers.GetSize();
  if (numChannels== 0) 
    return TRUE;
  // scan through the audio buffers and mix the signals
  PINDEX i;
  for (i = 0; i < numChannels; i++) {
    PString key = audioBuffers.GetKeyAt(i);
    audioBuffers[key].ReadAndMix((BYTE *)buffer, amount, numChannels);
  }
  return TRUE;
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
  ep.AddMember(this);

  return AnswerCallNow;
}
// ***********************************************************************
bool MyH323Connection::OnIncomingAudio(const void * buffer, PINDEX len)
{
  return ep.WriteAudio(GetCallToken(), buffer, len);
}
// ***********************************************************************
bool MyH323Connection::OnOutgoingAudio(void * buffer, PINDEX len)
{
  return ep.ReadAudio(GetCallToken(), buffer, len);
}
// ***********************************************************************
bool MyH323Connection::OpenAudioChannel(bool isEncoding, unsigned bufferSize, H323AudioCodec & codec)
{
  PWaitAndSignal mutex(audioMutex);
  if (incomingAudio == NULL){
    incomingAudio = new IncomingAudio(ep, *this);
  }

  if (outgoingAudio == NULL) {
    outgoingAudio = new OutgoingAudio(ep, *this);
  }

  if (isEncoding) {
    codec.AttachChannel(outgoingAudio, FALSE);
  } 
  else{
    codec.AttachChannel(incomingAudio, FALSE);
  }

  return TRUE;
}