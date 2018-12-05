#include "channel.h"


// ***********************************************************************
NullChannel::NullChannel(): isOpen(true)
{
	cout << "Creating NULL channel" << endl;
}
// ***********************************************************************
NullChannel::~NullChannel()
{
	cout << "Deleting NULL channel" << endl;
}
// ***********************************************************************
bool NullChannel::Close()
{
	isOpen = false;
	return true;
}
// ***********************************************************************
bool NullChannel::Write(const void *buf, PINDEX len)
{
	lastWriteCount = len;
	writeDelay.Delay(len/2/8);
	return true;
}
// ***********************************************************************
bool NullChannel::Read(void *buf, PINDEX len)
{
	memset(buf, 0, len);
	lastReadCount = len;
	readDelay.Delay(len/2/8);
	return true;	
}


