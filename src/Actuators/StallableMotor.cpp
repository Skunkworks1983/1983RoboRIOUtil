#include "StallableMotor.h"
#include <math.h>
#include <vector>
#include "../Time.h"
#include "../Logger.h"

std::vector<StallableMotor*> StallableMotor::motors = std::vector<
StallableMotor*>();

StallableMotor::StallableMotor(SpeedController *backend, double stallSpeed,
		double stallTimeThreshold, double stallTimeRefresh) {
	this->backend = backend;
	this->stallTimeThreshold = stallTimeThreshold;
	this->cacheSpeed = 0;
	this->cacheSyncGroup = 0;
	this->stallStart = -1.0;
	this->stallTimeRefresh = stallTimeRefresh;
	this->stalled = false;
	this->stalledCount = 0;
	this->stallSpeed = stallSpeed;
	this->name = "";
	StallableMotor::motors.push_back(this);
}

StallableMotor *StallableMotor::setName(char *name) {
	this->name = name;
	return this;
}

StallableMotor *StallableMotor::setEncoderSource(Encoder *enc) {
	this->stallPot = NULL;
	this->stallEncoder = enc;
	return this;
}

StallableMotor *StallableMotor::setPotSource(AnalogPot *enc) {
	this->stallPot = enc;
	this->stallEncoder = NULL;
	return this;
}

StallableMotor::~StallableMotor() {
	std::vector<StallableMotor*>::iterator it = std::find(
			StallableMotor::motors.begin(), StallableMotor::motors.end(), this);
	if (it != StallableMotor::motors.end()) {
		StallableMotor::motors.erase(it);
	}
}

void StallableMotor::updateControllers() {
	std::vector<StallableMotor*>::iterator it;
	for (it = StallableMotor::motors.begin(); it
			!= StallableMotor::motors.end(); it++) {
		StallableMotor *motr = (*it);
		motr->updateController();
	}
}

bool StallableMotor::isStalled() {
	return stalled;
}

void StallableMotor::updateController() {
	//Update stall state
	this->stalled = false;
	float rate = 0;
	if (stallEncoder != NULL) {
		rate = stallEncoder->GetRate();
	} else if (stallPot != NULL) {
		rate = stallPot->GetRate();
	}

	if (stallSpeed >= 0 && fabs(rate) <= stallSpeed && fabs(cacheSpeed) > 0.0) {
		double time = getCurrentMillis() - stallStart;
		if (stallStart < 0.0) {
			stallStart = getCurrentMillis();
		} else if (time > stallTimeRefresh && stallTimeRefresh > 0) {
			this->stalled = false;
			this->cacheSpeed /= 2.0;
			stallStart = -1;
			Logger::log(Logger::kWarning, "Stallable", "\"%s\" refreshed.",
					this->name);
		} else if (time > stallTimeThreshold) {
			if (!this->stalled) {
				this->stalledCount++;
			}
			if (!this->stalled) {
				Logger::log(Logger::kWarning, "Stallable",
						"\"%s\" is now stalled @ %f.", this->name, rate);
			}
			this->stalled = true;
		}
	} else {
		this->stalled = false;
		stallStart = -1;
		this->stalledCount = 0;
	}

	if (stalled) {
		backend->Set(0, cacheSyncGroup);
	} else {
		backend->Set(cacheSpeed, cacheSyncGroup);
	}
}

void StallableMotor::Set(float speed, UINT8 syncGroup) {
	this->cacheSpeed = speed;
	this->cacheSyncGroup = syncGroup;
}

float StallableMotor::Get() {
	return cacheSpeed;
}

void StallableMotor::Disable() {
	this->cacheSpeed = 0.0;
	this->backend->Disable();
}

void StallableMotor::PIDWrite(float speed) {
	this->cacheSpeed = speed;
}
