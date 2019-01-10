#include "main.h"
#include "version.h"
#include "ep.h"

PCREATE_PROCESS(MyMCU)

static PSyncPoint terminationSync; 

MyMCU::MyMCU():PProcess("vt","MyMCU",MyMCU_VER_MAJOR,MyMCU_VER_MINOR,
						MyMCU_VER_STATUS,MyMCU_VER_BUILD)
{}

MyMCU::~MyMCU()
{}

void MyMCU::Main()
{
	cout << "MyMCU 运行中..." << endl;

	PConfigArgs args(GetArguments()); 
	
	args.Parse(
#if PTRACING
			"t-trace."
			"o-output:"
#endif 
			"g-gatekeeper:"
			"G-gatekeeper-id:"
			"h-help."
			"n-no-gatekeeper."
			"p-port:"
			"u-user:"
		);

	if (args.HasOption('h')) 
	{
		printHelp();
		return;
	} 

#if PTRACING 
  PTrace::Initialise(args.GetOptionCount('t'),
		args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
		PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif 

	ProgConf progConf; 

	progConf.gkMode = ProgConf::RegisterWithGatekeeper; 

	if (args.HasOption('n') || (!args.HasOption('G') && !args.HasOption('G'))) 
		progConf.gkMode = ProgConf::NoGatekeeper;

	if (args.HasOption('g'))
	{
		progConf.gkAddr = args.GetOptionString('g');
		progConf.gkMode = ProgConf::RegisterWithGatekeeper;
	}

	if (args.HasOption('G'))
	{
		progConf.gkId = args.GetOptionString('G');
		progConf.gkMode = ProgConf::RegisterWithGatekeeper;
	} 

	if (args.HasOption('p')) 
		progConf.port = args.GetOptionString('p').AsUnsigned();

	if (progConf.port == 0) 
		progConf.port = DEFAULT_PORT;

	if (args.HasOption('u')) 
		progConf.userAliases = args.GetOptionString('u').Lines(); 

	// 创建并初始化EndPoint
	MyH323EndPoint endpoint(progConf); 

	if (endpoint.Init()) 
	{
//		cout << "MyMCU running" << endl;
		for (;;)
		{
    		cout << "MyMCU> " << flush;
    		PCaselessString cmd;
  		    cin >> cmd;
		    if (cmd == "X")
		        break;
		    if (cmd == "L")
		    {
		    	cout << endpoint.GetRoomNameList();
		    }
        }
        endpoint.DeleteAllRoom();
	}

	cout << "MyMCU关闭..." << endl;
}

void MyMCU::printHelp() 
{
	PError << "Available options:\n"
#if PTRACING
			"-o <file> --output <file>      将PTrace信息保存至<file>中\n"
			"-t --trace                     启用PTrace功能, 使用更多的t获取更多的调试信息\n"
#endif
			"-g <addr> --gatekeeper <addr>  网守的IP地址或DNS域名\n"
			"-G <id> --gatekeeper-id <id>   网守标识符\n"
			"-h --help                      输出help信息并退出\n"
			"-n --no-gatekeeper             不向网守注册\n"
			"-p <port> --port <port>        TCP监听端口号\n"
			"-u <user> --user <user>        使用用户名 (能有多个用户名)\n";
} 