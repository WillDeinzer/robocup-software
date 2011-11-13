#include "Physics/Env.hpp"
#include "Config.hpp"
#include "SimControl.hpp"

#include <QApplication>
#include <QFile>

#include <stdio.h>
#include <signal.h>

using namespace std;

void quit(int signal)
{
	fprintf(stderr, "Exiting due to signal %d\n", signal);
	exit(0);
}

void usage(const char* prog)
{
	fprintf(stderr, "usage: %s [-c <config file>] [--sv]\n", prog);
	fprintf(stderr, "\t--help  Show usage message\n");
	fprintf(stderr, "\t--sv    Use shared vision multicast port\n");
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	Env* env = new Env();

	QString configFile = "simulator.cfg";
	bool sendShared = false;

	//loop arguments and look for config file
	for (int i=1 ; i<argc ; ++i)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
			return 1;
		} else if (strcmp(argv[i], "--sv") == 0)
		{
			sendShared = true;
		} else if (strcmp(argv[i], "-c") == 0)
		{
			++i;
			if (i < argc)
			{
				configFile = argv[i];
			}
			else
			{
				printf ("Expected config file after -c parameter\n");
				return 1;
			}
		} else if ((strcmp(argv[i], "--h") == 0) || (strcmp(argv[i], "--help") == 0))
		{
			usage(argv[0]);
			return 0;
		} else {
			printf("%s is not recognized as a valid flag\n", argv[i]);
			return 1;
		}
	}

	env->sendShared = sendShared;
	
	if (!QFile(configFile).exists())
	{
		fprintf(stderr, "Configuration file %s does not exist\n", (const char *)configFile.toAscii());
		return 1;
	}
	
	Config* config = 0;
	config = new Config(configFile, env);

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = quit;
	sigaction(SIGINT, &act, 0);
	
	SimControl win;
	win.env(env);
	win.show();
	
	int ret = app.exec();

	delete env;

	delete config;

	return ret;
}