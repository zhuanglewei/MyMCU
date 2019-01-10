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
	virtual void OnConnectionEstablished(H323Connection &connection, 
					const PString &token);
	virtual void OnConnectionCleared(H323Connection &connection,
					const PString &token);
	virtual H323Connection::AnswerCallResponse OnAnswerCall(
					H323Connection &connection,
					const PString &caller,
					const H323SignalPDU & signalPDU,
					H323SignalPDU &);
	virtual bool OnStartLogicalChannel(H323Connection & connection,
					H323Channel & channel);

	void AddMember(MyH323Connection * newMember);
    void RemoveMember(MyH323Connection * oldConn);

	virtual H323Connection * CreateConnection(unsigned callReference);
	StringListDict & GetRoomIDList(){ return memberListDict; }
	PString GetHelpString();
	PString GetRoomNameList();
	void DeleteAllRoom();
private:
//	PStringList memberList;
	StringListDict memberListDict;
	PMutex memberMutex;

	

};

#endif