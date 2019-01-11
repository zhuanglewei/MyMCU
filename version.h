#ifndef _VERSION_H
#define _VERSION_H

#include <ptlib.h>

#define DEFAULT_PORT 1720
#define Speaker 1
#define Listener 0
#define ChairMan 2

class ProgConf
{
public:
	ProgConf(WORD _port = DEFAULT_PORT):port(_port)
	{};
	enum GKMode{
		NoGatekeeper,
		RegisterWithGatekeeper
	};
	GKMode 		gkMode;
	PString		gkAddr;
	PString		gkId;
	WORD		port;
	PString		call;
	PStringArray userAliases;	
};

#define MyMCU_VER_MAJOR 0
#define MyMCU_VER_MINOR 1
#define MyMCU_VER_STATUS ReleaseCode
#define MyMCU_VER_BUILD 1

#endif