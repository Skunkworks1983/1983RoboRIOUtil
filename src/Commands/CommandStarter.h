#ifndef __CMD_COMMANDSTARTER_H
#define __CMD_COMMANDSTARTER_H

#include "Commands/Command.h"

typedef Command* (*CreateCommand)();

class CommandStarter: public Command {
private:
	CreateCommand create;
	Command *orders;
	bool waitForRequirements;
	double maxWait;
	double startTime;
public:
	/*
	 * @brief Provides a way to start the command in an ordered way through 
	 * the command and the ready state
	 * @param createFunc The command being stopped or started
	 * @param waitForRequirements The state of being ready or not
	 */
	CommandStarter(CreateCommand createFunc, bool waitForRequirements = false, double maxWait = -1);
	~CommandStarter();
	virtual void Initialize();
	virtual void Execute();
	virtual bool IsFinished();
	virtual void End();
	virtual void Interrupted();
};

#endif
