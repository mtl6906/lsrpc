#include "ls/rpc/Tool.h"
#include "unistd.h"
#include "signal.h"

namespace ls
{
	namespace rpc
	{
		void handle_pipe(int sig)
		{
		
		}

		void _signal()
		{
			struct sigaction sa;
			sa.sa_handler = handle_pipe;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
			sigaction(SIGPIPE, &sa, NULL);
		}

		Tool::Tool() : config(new Config()), wm(new WorkerManager(config -> workerNumber, config -> connectionNumber)), cm(new ConnectionManager(config -> connectionNumber, config -> buffersize)), pm(new ProtocolManager())
		{
			_signal();
		}

		void Tool::run()
		{
			pm -> run(cm.get(), wm.get());
			wm -> run(cm.get(), pm.get());
			for(;;)
				sleep(1);
		}

		void Tool::push(Protocol *protocol)
		{
			pm -> push(protocol);
		}
	}
}
