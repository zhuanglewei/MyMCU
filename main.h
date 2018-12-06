#ifndef _MyMCU_MAIN_H
#define _MyMCU_MAIN_H

#include <ptlib.h>
#include <ptlib/pprocess.h>

class MyMCU: public PProcess
{
	PCLASSINFO(MyMCU,PProcess)
public:
	MyMCU();
	virtual ~MyMCU();
	void printHelp();
	virtual void Main();
};

#endif