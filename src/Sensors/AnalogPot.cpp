#include "AnalogPot.h"

#include "AnalogChannel.h"
#include "LiveWindow/LiveWindow.h"

#include "../Time.h"

void AnalogPot::InitAnalogPot(uint8_t moduleNumber, uint32_t channel) {
	backend = new AnalogChannel(moduleNumber, channel);

	LiveWindow::GetInstance()->AddSensor("Encoder", backend->GetModuleNumber(),
			backend->GetChannel(), this);

	// Default to just voltage
	this->a = 0;
	this->b = 1;
	this->c = 0;

	pAngle = GetAngle();
	pTime = getCurrentMillis();
	for (int i = 0; i < ANALOG_POT_AVERAGE_LENGTH; i++) {
		this->pRate[i] = 0;
	}

	accel = 0.0;
	pAverageRate = 0.0;
}

AnalogPot::AnalogPot(uint8_t moduleNumber, uint32_t channel) {
	InitAnalogPot(moduleNumber, channel);
}

AnalogPot::AnalogPot(uint32_t channel) {
	InitAnalogPot(GetDefaultAnalogModule(), channel);
}

void AnalogPot::setVoltageToAngle(float a, float b, float c) {
	this->a = a;
	this->b = b;
	this->c = c;
}

void AnalogPot::SetAverageBits(int bits) {
	this->backend->SetAverageBits(bits);
}

float AnalogPot::GetRawAngle() {
	float volts = backend->GetVoltage();
	return (this->a * volts * volts) + (this->b * volts) + this->c;
}

float AnalogPot::GetAngle(float ffwd) {
	float volts = backend->GetAverageVoltage();
	float angle = (this->a * volts * volts) + (this->b * volts) + this->c;
	if (ffwd > 0.0) {
		angle += (pAverageRate + (accel * 0.5 * ffwd)) * ffwd;
	}
	return angle;
}

/**
 * Units per second
 */
float AnalogPot::GetRate() {
	float angle = GetAngle(0.0);
	float time = getCurrentMillis();

	if (time - this->pTime < ANALOG_POT_RATE_SAMPLE_PERIOD) {
		float totalRate = 0;
		float totalCounts = 0;
		for (int i = 0; i < ANALOG_POT_AVERAGE_LENGTH; i++) {
			totalCounts++;
			totalRate += this->pRate[i];
		}
		return totalRate / totalCounts;
	}

	float totalRate = 0;
	float totalCounts = 0;
	for (int i = 1; i < ANALOG_POT_AVERAGE_LENGTH; i++) {
		this->pRate[i - 1] = this->pRate[i];
		totalCounts++;
		totalRate += this->pRate[i - 1];
	}
	{
		this->pRate[ANALOG_POT_AVERAGE_LENGTH - 1] = (1000.0 * (angle - pAngle)
				/ (time - pTime));
		totalRate += this->pRate[ANALOG_POT_AVERAGE_LENGTH - 1];
		totalCounts++;
	}
	accel = 1000.0 * ((totalRate / totalCounts) - pAverageRate)
			/ (time - pTime);
	pAverageRate = totalRate / totalCounts;

	this->pTime = time;
	this->pAngle = angle;
	return totalRate / totalCounts;
}

double AnalogPot::PIDGet() {
	return GetAngle();
}

void AnalogPot::StartLiveWindowMode() {
}

void AnalogPot::StopLiveWindowMode() {
}

std::string AnalogPot::GetSmartDashboardType() {
	return "Encoder";
}

void AnalogPot::InitTable(ITable *subTable) {
	m_table = subTable;
	UpdateTable();
}

ITable * AnalogPot::GetTable() {
	return m_table;
}

void AnalogPot::UpdateTable() {
	if (m_table != NULL) {
		m_table->PutNumber("Speed", GetRate());
		m_table->PutNumber("Distance", GetAngle());
		m_table->PutNumber("Distance per Tick", backend->GetAverageVoltage());
	}
}
