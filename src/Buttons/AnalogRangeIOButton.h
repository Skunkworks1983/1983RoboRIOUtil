#ifndef __ANALOGRANGE_IO_BUTTON_H
#define __ANALOGRANGE_IO_BUTTON_H
#include "WPILib.h"

class AnalogRangeIOButton: public Button {
private:
	double lowThreshold, highThreshold;
	int port;
public:
	
	/**
	 * @brief Creates on/off states for buttons that have varying analog states
	 * @param port
	 * @param lowThreshold
	 * @param highThreshold
	 */
	
	AnalogRangeIOButton(int port, double lowThreshold, double highThreshold);
	virtual ~AnalogRangeIOButton();
	virtual bool Get();
};

#endif

