#include "CommandStarter.h"

#include "Commands/Command.h"
#include "Commands/Subsystem.h"
#include "Commands/Scheduler.h"
#include "../../Utils/Time.h"

CommandStarter::CommandStarter(CreateCommand create, bool waitForRequirements,
		double maxWait) :
	Command("CommandStarter") {
	this->create = create;
	this->waitForRequirements = waitForRequirements;
	this->maxWait = maxWait;
}

CommandStarter::~CommandStarter() {
}

void CommandStarter::Initialize() {
	this->orders = create();
	this->startTime = getCurrentMillis();
}

void CommandStarter::Execute() {
	if (orders != NULL) {
		if (waitForRequirements) {
			for (SubsystemSet::iterator itr = orders->GetRequirements().begin(); itr
					!= orders->GetRequirements().end(); itr++) {
				Subsystem *sys = *(itr);
				if (sys != NULL && sys->GetCurrentCommand() != NULL
						&& sys->GetCurrentCommand() != sys->GetDefaultCommand()) {
					return;
				}
			}
		}
		Scheduler::GetInstance()->AddCommand(orders);
		orders = NULL;
	}
}

bool CommandStarter::IsFinished() {
	return (maxWait > 0 && getCurrentMillis() > (startTime + maxWait))
			|| orders == NULL;
}

void CommandStarter::End() {
}

void CommandStarter::Interrupted() {
}
