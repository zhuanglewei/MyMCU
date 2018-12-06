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
	cout << "MyMCU running." << endl;
	
	PConfigArgs args(GetArguments()); 

	args.Parse( 
			"g-gatekeeper:"
			"G-gatekeeper-id:"
			"h-help."
			"n-no-gatekeeper."
#if PTRACING
			"o-output:"
#endif
			"p-port:"
#if PTRACING
			"t-trace."
#endif
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

	if (args.HasOption('n')) 
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

//	progConf.fileName = args.GetOptionString('f'); 

	if (args.HasOption('p')) 
		progConf.port = args.GetOptionString('p').AsUnsigned();

	if (progConf.port == 0) 
		progConf.port = DEFAULT_PORT;

	if (args.HasOption('u')) 
		progConf.userAliases = args.GetOptionString('u').Lines(); 

	// Allocate and initialise H.323 endpoint
	MyH323EndPoint endpoint(progConf); 

	if (endpoint.Init()) 
	{
		cout << "MyMCU running" << endl;
		for (;;)
		{
    		cout << "MyMCU> " << flush;
    		PCaselessString cmd;
  		    cin >> cmd;
		    if (cmd == "X")
		        break;
        }
	}

	cout << "MyMCU shutting down..." << endl;
}

void MyMCU::printHelp() 
{
	PError << "Available options:\n"
			"-c <ip> --call <ip>        the IP of the Endpoint\n"
			"-g <addr> --gatekeeper <addr>  the IP address or DNS name of the gatekeeper\n"
			"-G <id> --gatekeeper-id <id>   gatekeeper identifier\n"
			"-h --help                      print this message and exit\n"
			"-n --no-gatekeeper             do not register with gatekeeper\n"
#if PTRACING
			"-o <file> --output <file>      send trace output to <file>\n"
#endif
			"-p <port> --port <port>        TCP port to listen at\n"
#if PTRACING
			"-t --trace                     enable trace, use multiple times for more detail\n"
#endif
			"-u <user> --user <user>        user name or number (can be used multiple times)\n";
} 