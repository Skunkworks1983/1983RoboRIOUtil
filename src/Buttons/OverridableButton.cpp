#include "OverridableButton.h"

OverridableButton::OverridableButton(Trigger *override, Trigger *button,
		bool overrideState) {
	this->override = override;
	this->button = button;
	this->overrideState = overrideState;
	this->lastState = button->Get();
}

OverridableButton::~OverridableButton() {
	// twiddle our thumbs
}

bool OverridableButton::Get() {
	if (override->Get() == overrideState) {
		return lastState = button->Get();
	} else {
		return lastState;
	}
}
