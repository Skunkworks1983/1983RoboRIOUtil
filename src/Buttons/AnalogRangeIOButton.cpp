#include "AnalogRangeIOButton.h"
#include "DriverStation.h"

AnalogRangeIOButton::AnalogRangeIOButton(int port, double lowThreshold, double highThreshold) {
	this->port = port;
	this->lowThreshold = lowThreshold;
	this->highThreshold = highThreshold;
}

AnalogRangeIOButton::~AnalogRangeIOButton() {
	// twiddle our thumbs
}

bool AnalogRangeIOButton::Get() {
	double volts = DriverStation::GetInstance()->GetEnhancedIO().GetAnalogIn(port);
	return volts >= lowThreshold && volts <= highThreshold;
}
