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
		PError << "注册网守失败" << endl; 
		return false;
	} 		
	return true;
}

// ***********************************************************************

void MyH323EndPoint::OnConnectionEstablished(H323Connection & connection, 
						const PString & token)
{
	PTRACE(1, "连接建立, token标识符是 " << token);
}

// ***********************************************************************

void MyH323EndPoint::OnConnectionCleared(H323Connection &connection, 
						const PString &token)
{
//	RemoveMember(（MyH323Connection）&connection);
	PTRACE(1, "connection连接被清除, token标识符是 " << token);
}

// ***********************************************************************

H323Connection::AnswerCallResponse MyH323EndPoint::OnAnswerCall( 
						H323Connection & connection,
						const PString & caller, 
						const H323SignalPDU &, 
						H323SignalPDU &)
{
	PTRACE(1, "允许来自于 " << caller << " 的呼叫");
	return H323Connection::AnswerCallNow;
} 

// ***********************************************************************

bool MyH323EndPoint::OnStartLogicalChannel(H323Connection & connection, 
						H323Channel & channel)
{
	PTRACE(1,"开启逻辑通道: " << ((channel.GetDirection()==H323Channel::IsTransmitter) ? (" sending "):(" receiving "))
                           <<  channel.GetCapability() );
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
	PString RoomID = newMember->GetRoomID();
	if(RoomID == NULL)
		return;
	if(!memberListDict.Contains(RoomID)){
		PTRACE(1,"创建会议房间 - " << RoomID);
		memberListDict.SetAt(RoomID,new PStringList);
		RoomMemberList.SetAt(RoomID,new PStringList);
		newMember->SetIdentify(ChairMan);
		PTRACE(1,newMember->GetRemotePartyName() + "成为会员房间 " + RoomID + " 的主席.");
	}
	PStringList & memberList = memberListDict[RoomID];
	PStringList & memberName = RoomMemberList[RoomID];
	PTRACE(1,"会议房间：" << RoomID << "添加成员-" << newToken);
	memberList.AppendString(newToken);
	memberName.AppendString(newMember->GetRemotePartyName());
	PINDEX i;
  	for (i = 0; i < memberList.GetSize(); i++) {
    	PString token = memberList[i];
    	if (token != newToken) {      
      		MyH323Connection * conn = (MyH323Connection *)FindConnectionWithLock(token);
      		if (conn != NULL) {
        		conn->AddMember(newToken);
        		newMember->AddMember(token);
        		conn->Unlock();
      		}
    } 
  }
}

// ***********************************************************************

void MyH323EndPoint::RemoveMember(MyH323Connection * oldConn)
{
  	PWaitAndSignal mutex(memberMutex);
  	PString oldToken = oldConn->GetCallToken();
  	PString RoomID = oldConn->GetRoomID();
  	if(RoomID == NULL || !memberListDict.Contains(RoomID))
  		return;
  	PStringList memberList = memberListDict[RoomID];
  	PTRACE(1,"会议房间 " << RoomID << "删除成员-" << oldToken);
  	PINDEX i;
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
  	memberList.RemoveAt(memberList.GetStringsIndex(oldToken));
  	if(memberList.GetSize() == 0){
  		PTRACE(1,"删除会议房间:" << RoomID);
  		memberListDict.RemoveAt(RoomID);
  	}
}

// ***********************************************************************

PString MyH323EndPoint::GetRoomNameList()
{
	PString RoomList;
	if(memberListDict.GetSize() == 0){
		RoomList += "MCU 不存在会议房间.\n";
	}
	else{
		RoomList += "MCU已存在会议房间列表：" ;
		RoomList += "\n序号\t会议房间名字";
		PINDEX i;
		for(i=0; i<memberListDict.GetSize(); i++){
			PString RoomID = memberListDict.GetKeyAt(i);
			char str[3];
			sprintf(str,"%d",i);
			RoomList += "\n";
			RoomList += str;
			RoomList += "\t" + RoomID +"\n";		    	
		}
	}
	return RoomList;
}

void MyH323EndPoint::DeleteAllRoom()
{
	PINDEX i;
	for(i=0; i<memberListDict.GetSize(); i++){
		PString RoomID = memberListDict.GetKeyAt(i);
		PStringList memberList = memberListDict[RoomID];
		PINDEX j;
		for(j=0; j<memberList.GetSize(); j++)
		{
			PString token = memberList[j];
      		MyH323Connection * member = (MyH323Connection *)FindConnectionWithLock(token);
      		if (member != NULL) { 
				RemoveMember(member);
				member->ClearCall();
        		member->Unlock();
        	}

    	}
    	
	}
}

PString MyH323EndPoint::GetHelpString(){
 	PString helpString;
  	helpString += "欢迎使用MCU服务器\n请输入以下命令与MCU交互：\n";
  	helpString += "-Read\t\t\t查看MCU已存在房间\n";
  	helpString += "-c + 房间号\t\t创建房间\t如：-c+001\n";
  	helpString += "-j + 房间号\t\t加入房间\t如：-j+001\n";
  	helpString += "-y + IP地址\\用户名\t邀请加入房间\t如：-y+Name";
  	return helpString;
}

PString MyH323EndPoint::GetMemberName(PString & RoomID){
	PString nameList ;
	nameList += "会议房间 " + RoomID +" 存在成员：\n";
	PStringList & memberList = RoomMemberList[RoomID];
	PINDEX i;
	for(i=0; i<memberList.GetSize(); i++){
		nameList += "\t" +memberList[i];
		if(i%4 == 0)
			nameList += "\n";
	}
	return nameList;
}