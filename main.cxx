#include "main.h"
#include "version.h"

PCREATE_PROCESS(MyMCU)

static PSyncPoint terminationSync; 

MyMCU::MyMCU():PProcess("vt","MyMCU",MyMCU_VER_MAJOR,MyMCU_VER_MINOR,
						MyMCU_VER_STATUS,MyMCU_VER_BUILD)
{}

MyMCU::~MyMCU()
{}

void MyMCU::Main()
{
	cout << "MyMCU running." << endl;
	terminationSync.Wait();
}