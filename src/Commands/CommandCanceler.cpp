#include "CommandCanceler.h"

#include "Commands/Command.h"
#include "Commands/Scheduler.h"

CommandCanceler::CommandCanceler(Command *sys, double delay) :
	Command("CommandCanceler") {
	this->cancel = sys;
	SetTimeout(delay);
}

CommandCanceler::~CommandCanceler() {
}

void CommandCanceler::Initialize() {
}

void CommandCanceler::Execute() {
}

bool CommandCanceler::IsFinished() {
	if (IsTimedOut()) {
		Scheduler::GetInstance()->Remove(cancel);
		return true;
	}
	return false;
}

void CommandCanceler::End() {
	Scheduler::GetInstance()->Remove(cancel);
}

void CommandCanceler::Interrupted() {
	Scheduler::GetInstance()->Remove(cancel);
}
