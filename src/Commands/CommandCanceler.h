#ifndef __CMD_COMMANDCANCEL_H
#define __CMD_COMMANDCANCEL_H

#include "Commands/Command.h"

class Subsystem;
class CommandCanceler: public Command {
private:
	Command *cancel;
public:
	/*
	 * @brief This utility stops the command that it is given
	 * @param cancel is the command that is going to be canceled
	 */
	CommandCanceler(Command *cancel, double delay = 0.0);
	~CommandCanceler();
	virtual void Initialize();
	virtual void Execute();
	virtual bool IsFinished();
	virtual void End();
	virtual void Interrupted();
};

#endif
