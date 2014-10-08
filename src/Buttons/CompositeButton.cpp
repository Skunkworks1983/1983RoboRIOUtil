#include "CompositeButton.h"
#include "DriverStation.h"

CompositeButton::CompositeButton(Trigger *portA, Trigger *portB,
		BinaryOperation op) {
	this->portA = portA;
	this->portB = portB;
	this->op = op;
}

CompositeButton::~CompositeButton() {
	// twiddle our thumbs
}

bool CompositeButton::Get() {
	bool a = portA->Get();
	bool b= (portB!=NULL ? portB->Get() : true);
	switch (op) {
	case CompositeButton::kNAND:
		return !(a&&b);
	case CompositeButton::kNOT:
		return !a;
	case CompositeButton::kOR:
		return a||b;
	case CompositeButton::kXOR:
		return a^b;
	case CompositeButton::kNOR:
		return !(a||b);
	case CompositeButton::kNXOR:
		return !(a^b);
	case CompositeButton::kAND:
	default:
		return a && b;
	}
}
