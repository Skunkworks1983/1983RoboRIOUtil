#ifndef __VALUE_CHANGE_TRIGGER_H
#define __VALUE_CHANGE_TRIGGER_H
#include "WPILib.h"

#define FORCE_VALUE_CHANGE			-100.0

typedef double (*GET_DOUBLE)();

class ValueChangeTrigger: public Trigger {
private:
	GET_DOUBLE getValue;
	double changeThreshold;
	double lastValue;
public:
	/**
	 * @brief ¯\_(@_@)_/¯
	 * @param getValue
	 * @param changeThreshold
	 */
	ValueChangeTrigger(GET_DOUBLE getValue, double changeThreshold);
	virtual ~ValueChangeTrigger();
	virtual bool Get();
};

#endif

