#ifndef _EP_H
#define _EP_H

#include <h323.h>
#include "version.h"
#include "connection.h"
#include "channel.h"
#include <string.h>

PDICTIONARY(StringListDict, PString, PStringList);

class MyH323Connection;

class MyH323EndPoint: public H323EndPoint
{
	PCLASSINFO(MyH323EndPoint, H323EndPoint)
	const ProgConf & progConf;  
public:
	MyH323EndPoint(const ProgConf & conf);

	~MyH323EndPoint();
	bool Init(void); 
//协议状态回调函数
	virtual void OnConnectionEstablished(H323Connection &connection, const PString &token);
	virtual void OnConnectionCleared(H323Connection &connection,const PString &token);
	virtual H323Connection::AnswerCallResponse OnAnswerCall(H323Connection &connection,const PString &caller,const H323SignalPDU & signalPDU,H323SignalPDU &);
	virtual bool OnStartLogicalChannel(H323Connection & connection,H323Channel & channel);
//创建connection函数
	virtual H323Connection * CreateConnection(unsigned callReference);
//会议房间管理函数：
	StringListDict & GetRoomIDList(){ return memberListDict; }//获取房间列表，房间成员以标识符token区分
	PStringList & GetMemberList(PString RoomID){ return RoomMemberList[RoomID]; }//获取房间成员名字列表
	PString GetRoomNameList();//获取房间列表
	PString GetMemberName(PString & RoomID);//获取房间成员
	void DeleteAllRoom();//删除所有会议房间
//会议成员管理函数：
	void AddMember(MyH323Connection * newMember);//添加会议成员
    void RemoveMember(MyH323Connection * oldConn);//删除会议成员
//获取MCU使用简介
	PString GetHelpString();
private:
	StringListDict memberListDict;
	StringListDict RoomMemberList;
	PMutex memberMutex;
};

#endif