// cli/CLIRunner.h
#pragma once

#include <QStringList>

class CLIRunner
{
public:
	int run(int argc, char* argv[]);

private:
	void printHelp();
};
