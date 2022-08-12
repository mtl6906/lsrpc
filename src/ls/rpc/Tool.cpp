#include "ls/rpc/Tool.h"
#include "unistd.h"
#include "signal.h"
#include "ls/DefaultLogger.h"

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

		Tool::Tool(QueueFactory &qf) :	qm(config.workerNumber, qf.makeQueue(config.workerNumber * config.connectionNumber)),
						wm(config.workerNumber, config.connectionNumber, config.buffersize)
		{
			_signal();
		}

		void Tool::run()
		{
			LOGGER(ls::INFO) << "start..." << ls::endl;
//			qm.run(pm.size());
//			LOGGER(ls::INFO) << "qm start..." << ls::endl;
			wm.run(&pm, &qm);
			LOGGER(ls::INFO) << "wm start..." << ls::endl;
//			pm.run(&qm);
//			LOGGER(ls::INFO) << "pm start..." << ls::endl;
			for(;;)
				sleep(1);
		}

		void Tool::push(Protocol *protocol)
		{
			pm.push(protocol);
		}
	}
}
