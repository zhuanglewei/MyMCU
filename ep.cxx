#include <ptlib.h>
#include "ep.h"
#include "connection.h"
#include "channel.h"

MyH323EndPoint::MyH323EndPoint(const ProgConf & conf) : progConf(conf)
{}

MyH323EndPoint::~MyH323EndPoint()
{}

// ***********************************************************************

bool MyH323EndPoint::Init()
{
	// Set user name
	if (!progConf.userAliases.IsEmpty()) 
	{
		SetLocalUserName(progConf.userAliases[0]); 
		for (PINDEX i=1; i<progConf.userAliases.GetSize(); i++)
			AddAliasName(progConf.userAliases[i]); 
	} 
	
	capabilities.AddAllCapabilities(0, 0, "SpeexIETFNarrow-8k");
	
	H323Capability *gsmCap = H323Capability::Create("GSM-06.10{sw}");
	if (gsmCap != NULL)
	{
		SetCapability(0, 0, gsmCap);
		gsmCap->SetTxFramesInPacket(4); // For GSM 06.10, 1 frame ~ 20 milliseconds
	} 
	
	SetCapability(0, 0, new H323_G711Capability(H323_G711Capability::muLaw) );
	SetCapability(0, 0, new H323_G711Capability(H323_G711Capability::ALaw) ); 
	
	AddAllUserInputCapabilities(0, 1); 
					
	PTRACE(1, "Capabilities:\n" << setprecision(2) << capabilities); 

	// Start H323 Listener
	PIPSocket::Address addr = INADDR_ANY; 
	H323ListenerTCP *listener = new H323ListenerTCP(*this, addr,
							progConf.port);
	if (listener == NULL || !StartListener(listener))
	{	PError << "Unable to start H323 Listener at port "
				<< progConf.port << endl;
		if (listener != NULL)
			delete listener;
		return false;
	} 

	// Gatekeeper registration
	bool gkResult = false; 
	switch (progConf.gkMode)
	{
	case ProgConf::NoGatekeeper: 
		gkResult = true;
		break;
	case ProgConf::RegisterWithGatekeeper:
		gkResult = UseGatekeeper(progConf.gkAddr, progConf.gkId);
		break;
	default:
		break;
	}

	if (!gkResult)
	{
		PError << "Failed to register with gatekeeper" << endl; 
		return false;
	} 		
	return true;
}

// ***********************************************************************

void MyH323EndPoint::OnConnectionEstablished(H323Connection & connection, 
						const PString & token)
{
	PTRACE(1, "Connection established, token is " << token);
}

// ***********************************************************************

void MyH323EndPoint::OnConnectionCleared(H323Connection &connection, 
						const PString &token)
{
	PTRACE(1, "Connection cleared, token is " << token);
}

// ***********************************************************************

H323Connection::AnswerCallResponse MyH323EndPoint::OnAnswerCall( 
						H323Connection & connection,
						const PString & caller, 
						const H323SignalPDU &, 
						H323SignalPDU &)
{
	PTRACE(1, "Accepting connection from " << caller);
	return H323Connection::AnswerCallNow;
} 

// ***********************************************************************

bool MyH323EndPoint::OnStartLogicalChannel(H323Connection & connection, 
						H323Channel & channel)
{
	PString dir;
	switch (channel.GetDirection())
	{
	case H323Channel::IsTransmitter :
		dir = "sending";
		break;
	case H323Channel::IsReceiver :
		dir = "receiving";
		break;
	default :
		break;
	}
	cout <<  "Started logical channel " << dir << " " << channel.GetCapability() << endl;
	return true;
}

// ***********************************************************************

H323Connection * MyH323EndPoint::CreateConnection(unsigned callReference)
{
	return new MyH323Connection(*this,callReference);
}

// ***********************************************************************

void MyH323EndPoint::AddMember(MyH323Connection * newMember)
{
  PWaitAndSignal mutex(memberMutex);
  PString newToken = newMember->GetCallToken();

  memberList.AppendString(newToken);
  cout << "newmember adding." << endl;
  // add the new member to every other member,
  // and add every other member to the new member.
  PINDEX i;
  for (i = 0; i < memberList.GetSize(); i++) {
    PString token = memberList[i];
    if (token != newToken) {      
      cout << "Adding member " << newToken << " to list of " << token << endl;
      MyH323Connection * conn = (MyH323Connection *)FindConnectionWithLock(token);
      if (conn != NULL) {
        conn->AddMember(newToken);
        newMember->AddMember(token);
        conn->Unlock();
      }
    } 
    else 
        cout << "Member " << newToken << " will not hear their own voice" << endl; 
  }
}

// ***********************************************************************

void MyH323EndPoint::RemoveMember(MyH323Connection * oldConn)
{
  PWaitAndSignal mutex(memberMutex);
  PString oldToken = oldConn->GetCallToken();
  PINDEX i;
  // remove this member from the audio buffer lists
  for (i = 0; i < memberList.GetSize(); i++) {
    PString token = memberList[i];
    if (token != oldToken) {
      MyH323Connection * conn = (MyH323Connection *)FindConnectionWithLock(token);
      if (conn != NULL) {
	conn->RemoveMember(oldToken);
	oldConn->RemoveMember(token);
        conn->Unlock();
      }
    }
  }
  // remove this connection from the member list
  memberList.RemoveAt(memberList.GetStringsIndex(oldToken));
}
// ***********************************************************************
