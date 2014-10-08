#ifndef __POTENTIOMETER_H
#define __POTENTIOMETER_H

#include "PIDSource.h"
#include "LiveWindow/LiveWindowSendable.h"
#include "SensorBase.h"

class AnalogChannel;

#define ANALOG_POT_AVERAGE_LENGTH (5)
#define ANALOG_POT_RATE_SAMPLE_PERIOD (50.0)

class AnalogPot : public PIDSource, public LiveWindowSendable, public SensorBase {
private:
	AnalogChannel *backend;
	float a, b, c;
	
	float pTime, pAngle;
	float pRate[ANALOG_POT_AVERAGE_LENGTH];
	
	float pAverageRate;
	float accel;
	
	ITable *m_table;

	void InitAnalogPot(uint8_t moduleNumber, uint32_t channel);
public:
	/*
	 * @brief Is a replacement to the potentiometer in WPILib, adds 
	 * functionality to livewindow and scaling factors
	 * @param moduleNumber The digital sidecar it's on
	 * @param channel The channel number of the pot
	 */
	AnalogPot(uint8_t moduleNumber, uint32_t channel);
	AnalogPot(uint32_t channel);

	/**
	 * Coeffecients for something of the form "angle = av^2 + bv + c"
	 */
	void setVoltageToAngle(float a, float b, float c);

	float GetAngle(float ffwd = 0.0);
	
	float GetRate();
	bool GetStopped();
	
	float GetRawAngle();

	double PIDGet();
	
	void SetAverageBits(int bits);
	
	void UpdateTable();
	void StartLiveWindowMode();
	void StopLiveWindowMode();
	std::string GetSmartDashboardType();
	void InitTable(ITable *subTable);
	ITable * GetTable();
};
#endif
