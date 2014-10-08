#ifndef __OVERRIDABLE_BUTTON_H
#define __OVERRIDABLE_BUTTON_H
#include "WPILib.h"

class OverridableButton : public Button {
private:
	Trigger *override;
	Trigger *button;
	bool lastState;
	bool overrideState;
public:
	/**
	 * @brief Attaches a button to a override switch
	 * @param override Switch that overrides the button
	 * @param button
	 * @param overrideState (true/false)
	 */
	OverridableButton(Trigger *override, Trigger *button, bool overrideState=true);
	virtual ~OverridableButton();
	virtual bool Get();
};

#endif

