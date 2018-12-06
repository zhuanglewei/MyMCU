#ifndef _EP_H
#define _EP_H

#include <h323.h>
#include "version.h"
#include "connection.h"

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
#if 0
	virtual bool OpenAudioChannel(H323Connection &connection,
					bool isEncoding, unsigned bufferSize,
					H323AudioCodec &codec);
#endif
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

	bool WriteAudio(const PString & thisToken, const void * buffer, PINDEX len);
	bool ReadAudio(const PString & thisToken, void * buffer, PINDEX len);

private:
	PMutex memberMutex;

	PStringList memberList;

};

#endif