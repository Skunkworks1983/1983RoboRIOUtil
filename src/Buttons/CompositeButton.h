#ifndef __COMPOSITE_BUTTON_H
#define __COMPOSITE_BUTTON_H
#include "WPILib.h"

class CompositeButton: public Button {
public:
	enum BinaryOperation {
		kAND = '&',
		kOR = '|',
		kXOR = '^',
		kNAND = '#',
		kNOT = '!',
		kNOR = '@',
		kNXOR = '-'
	};
private:
	Trigger *portA;
	Trigger *portB;
	BinaryOperation op;
public:
	/**
	 * @brief Creats a button with different operations to set it on/off
	 * @param portA
	 * @param portB
	 * @param BinaryOperation
	 */
	CompositeButton(Trigger *portA, Trigger *portB, BinaryOperation op = kAND);
	virtual ~CompositeButton();
	virtual bool Get();
};

#endif

