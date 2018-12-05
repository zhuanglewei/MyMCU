#ifndef _EP_H
#define _EP_H

#include <h323.h>
#include "version.h"

#define _H323Connnection 0

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
	virtual bool OpenAudioChannel(H323Connection &connection,
					bool isEncoding, unsigned bufferSize,
					H323AudioCodec &codec);
	virtual H323Connection::AnswerCallResponse OnAnswerCall(
					H323Connection &connection,
					const PString &caller,
					const H323SignalPDU & signalPDU,
					H323SignalPDU &);
	virtual bool OnStartLogicalChannel(H323Connection & connection,
					H323Channel & channel);
#if _H323Connnection
	virtual H323Connection * CreateConnection(unsigned callReference);
#endif
};

#endif