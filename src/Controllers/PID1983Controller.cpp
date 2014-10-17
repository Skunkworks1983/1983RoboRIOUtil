/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "PID1983Controller.h"
#include "NetworkCommunication/UsageReporting.h"
#include "Notifier.h"
#include "PIDSource.h"
#include "PIDOutput.h"
#include "Timer.h"
#include <cmath>
#include "HAL/cpp/Synchronized.hpp"
#include <pthread.h>

static const char *kP = "p";
static const char *kI = "i";
static const char *kD = "d";
static const char *kF = "f";
static const char *kSetpoint = "setpoint";
static const char *kEnabled = "enabled";

/**
 * Allocate a PID object with the given constants for P, I, D
 * @param Kp the proportional coefficient
 * @param Ki the integral coefficient
 * @param Kd the derivative coefficient
 * @param source The PIDSource object that is used to get values
 * @param output The PIDOutput object that is set to the output value
 * @param period the loop time for doing calculations. This particularly effects calculations of the
 * integral and differental terms. The default is 50ms.
 */
PID1983Controller::PID1983Controller(float Kp, float Ki, float Kd,
		PIDSource *source, PIDOutput *output, float period) {
	Initialize(Kp, Ki, Kd, 0.0f, source, output, period);
	Timer::GetFPGATimestamp();
}

/**
 * Allocate a PID object with the given constants for P, I, D
 * @param Kp the proportional coefficient
 * @param Ki the integral coefficient
 * @param Kd the derivative coefficient
 * @param source The PIDSource object that is used to get values
 * @param output The PIDOutput object that is set to the output value
 * @param period the loop time for doing calculations. This particularly effects calculations of the
 * integral and differental terms. The default is 50ms.
 */
PID1983Controller::PID1983Controller(float Kp, float Ki, float Kd, float Kf,
		PIDSource *source, PIDOutput *output, float period) :
			 m_mutex(PTHREAD_MUTEX_INITIALIZER){
	Initialize(Kp, Ki, Kd, Kf, source, output, period);
}

void PID1983Controller::Initialize(float Kp, float Ki, float Kd, float Kf,
		PIDSource *source, PIDOutput *output, float period) {
	m_table = NULL;

	m_controlLoop = new Notifier(PID1983Controller::CallCalculate, this);

	m_P = Kp;
	m_I = Ki;
	m_D = Kd;
	m_F = Kf;

	m_maximumOutput = 1.0;
	m_minimumOutput = -1.0;

	m_maximumInput = 0;
	m_minimumInput = 0;

	m_continuous = false;
	m_enabled = false;
	m_setpoint = 0;

	m_prevError = 0;
	m_totalError = 0;
	m_tolerance = .05;

	m_result = 0;

	m_pidInput = source;
	m_pidOutput = output;
	m_period = period;

	m_controlLoop->StartPeriodic(m_period);

	static int32_t instances = 0;
	instances++;
	nUsageReporting::report(nUsageReporting::kResourceType_PIDController,
			instances);

	m_toleranceType = kNoTolerance;

	m_maximumITerm = -1;
	m_lastTick = 0;
}

/**
 * Free the PID object
 */
PID1983Controller::~PID1983Controller() {
	pthread_mutex_destroy(&m_mutex);
	delete m_controlLoop;
}

/**
 * Call the Calculate method as a non-static method. This avoids having to prepend
 * all local variables in that method with the class pointer. This way the "this"
 * pointer will be set up and class variables can be called more easily.
 * This method is static and called by the Notifier class.
 * @param controller the address of the PID controller object to use in the background loop
 */
void PID1983Controller::CallCalculate(void *controller) {
	PID1983Controller *control = (PID1983Controller*) controller;
	control->Calculate();
}

/**
 * Read the input, calculate the output accordingly, and write to the output.
 * This should only be called by the Notifier indirectly through CallCalculate
 * and is created during initialization.
 */
void PID1983Controller::Calculate() {
	bool enabled;
	PIDSource *pidInput;

	pthread_mutex_lock(&m_mutex);
		if (m_pidInput == 0) {
			pthread_mutex_unlock(&m_mutex);
			return;
		}
		if (m_pidOutput == 0) {
			pthread_mutex_unlock(&m_mutex);
			return;
		}
		enabled = m_enabled;
		pidInput = m_pidInput;
	pthread_mutex_unlock(&m_mutex);

	if (enabled) {
		double cTime = Timer::GetFPGATimestamp();
		if (m_lastTick > 0) {
			double tickDifference = cTime - m_lastTick;
			double tickScaling = tickDifference / m_period;

			float input = pidInput->PIDGet();
			float result;
			PIDOutput *pidOutput;

			{
				Synchronized sync(&m_mutex);
				m_error = m_setpoint - input;
				if (m_continuous) {
					if (fabs(m_error) > (m_maximumInput - m_minimumInput) / 2) {
						if (m_error > 0) {
							m_error = m_error - m_maximumInput + m_minimumInput;
						} else {
							m_error = m_error + m_maximumInput - m_minimumInput;
						}
					}
				}

				if (m_I != 0) {
					double potentialIGain = (m_totalError + (m_error
							*tickScaling)) * m_I;
					if (m_maximumITerm < 0) {
						if (potentialIGain < m_maximumOutput) {
							if (potentialIGain > m_minimumOutput)
								m_totalError += (m_error*tickScaling);
							else
								m_totalError = m_minimumOutput / m_I;
						} else {
							m_totalError = m_maximumOutput / m_I;
						}
					} else {
						if (fabs(potentialIGain) > m_maximumITerm) {
							m_totalError = (m_maximumITerm / m_I)
									* (potentialIGain < 0 ? -1.0 : 1.0);
						} else {
							m_totalError += (m_error*tickScaling);
						}
					}
				}

				m_result = m_P * m_error + m_I * m_totalError + m_D * ((m_error
						- m_prevError)/tickScaling) + m_setpoint * m_F;
				m_prevError = m_error;

				if (m_result > m_maximumOutput)
					m_result = m_maximumOutput;
				else if (m_result < m_minimumOutput)
					m_result = m_minimumOutput;

				pidOutput = m_pidOutput;
				result = m_result;
			}
			pidOutput->PIDWrite(result);
		}
		m_lastTick = cTime;
	} else {
		m_lastTick = 0;
	}
}

/**
 * Set the PID Controller gain parameters.
 * Set the proportional, integral, and differential coefficients.
 * @param p Proportional coefficient
 * @param i Integral coefficient
 * @param d Differential coefficient
 */
void PID1983Controller::SetPID(float p, float i, float d) {
	pthread_mutex_lock(&m_mutex);
		m_P = p;
		m_I = i;
		m_D = d;
	pthread_mutex_unlock(&m_mutex);

	if (m_table != NULL) {
		m_table->PutNumber("p", m_P);
		m_table->PutNumber("i", m_I);
		m_table->PutNumber("d", m_D);
	}
}

/**
 * Set the PID Controller gain parameters.
 * Set the proportional, integral, and differential coefficients.
 * @param p Proportional coefficient
 * @param i Integral coefficient
 * @param d Differential coefficient
 * @param f Feed forward coefficient
 */
void PID1983Controller::SetPID(float p, float i, float d, float f) {
	pthread_mutex_lock(&m_mutex);
		m_P = p;
		m_I = i;
		m_D = d;
		m_F = f;
	pthread_mutex_unlock(&m_mutex);

	if (m_table != NULL) {
		m_table->PutNumber("p", m_P);
		m_table->PutNumber("i", m_I);
		m_table->PutNumber("d", m_D);
		m_table->PutNumber("f", m_F);
	}
}

/**
 * Get the Proportional coefficient
 * @return proportional coefficient
 */
float PID1983Controller::GetP() {
	pthread_mutex_lock(&m_mutex);
		return m_P;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Get the Integral coefficient
 * @return integral coefficient
 */
float PID1983Controller::GetI() {
	pthread_mutex_lock(&m_mutex);
		return m_I;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Get the Differential coefficient
 * @return differential coefficient
 */
float PID1983Controller::GetD() {
	pthread_mutex_lock(&m_mutex);
		return m_D;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Get the Feed forward coefficient
 * @return Feed forward coefficient
 */
float PID1983Controller::GetF() {
	pthread_mutex_lock(&m_mutex);
		return m_F;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Return the current PID result
 * This is always centered on zero and constrained the the max and min outs
 * @return the latest calculated output
 */
float PID1983Controller::Get() {
	float result;
	pthread_mutex_lock(&m_mutex);
		result = m_result;
	pthread_mutex_unlock(&m_mutex);
	return result;
}

/**
 *  Set the PID controller to consider the input to be continuous,
 *  Rather then using the max and min in as constraints, it considers them to
 *  be the same point and automatically calculates the shortest route to
 *  the setpoint.
 * @param continuous Set to true turns on continuous, false turns off continuous
 */
void PID1983Controller::SetContinuous(bool continuous) {
	pthread_mutex_lock(&m_mutex);
		m_continuous = continuous;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Sets the maximum and minimum values expected from the input.
 * 
 * @param minimumInput the minimum value expected from the input
 * @param maximumInput the maximum value expected from the output
 */
void PID1983Controller::SetInputRange(float minimumInput, float maximumInput) {
	pthread_mutex_lock(&m_mutex);
		m_minimumInput = minimumInput;
		m_maximumInput = maximumInput;
	pthread_mutex_unlock(&m_mutex);

	SetSetpoint(m_setpoint);
}

/**
 * Sets the minimum and maximum values to write.
 * 
 * @param minimumOutput the minimum value to write to the output
 * @param maximumOutput the maximum value to write to the output
 */
void PID1983Controller::SetOutputRange(float minimumOutput, float maximumOutput) {
	pthread_mutex_lock(&m_mutex);
		m_minimumOutput = minimumOutput;
		m_maximumOutput = maximumOutput;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * Set the setpoint for the PID1983Controller
 * @param setpoint the desired setpoint
 */
void PID1983Controller::SetSetpoint(float setpoint) {
	pthread_mutex_lock(&m_mutex);
		if (m_maximumInput> m_minimumInput) {
			if (setpoint> m_maximumInput) {
				m_setpoint = m_maximumInput;
			} else if (setpoint < m_minimumInput) {
				m_setpoint = m_minimumInput;
			} else {
				m_setpoint = setpoint;
			}
		} else {
			m_setpoint = setpoint;
		}
	pthread_mutex_unlock(&m_mutex);

	if (m_table != NULL) {
		m_table->PutNumber("setpoint", m_setpoint);
	}
}

/**
 * Returns the current setpoint of the PID1983Controller
 * @return the current setpoint
 */
float PID1983Controller::GetSetpoint() {
	float setpoint;
	pthread_mutex_lock(&m_mutex);
		setpoint = m_setpoint;
	pthread_mutex_unlock(&m_mutex);
	return setpoint;
}

/**
 * Retruns the current difference of the input from the setpoint
 * @return the current error
 */
float PID1983Controller::GetError() {
	float error;
	pthread_mutex_lock(&m_mutex);
		error = m_setpoint - m_pidInput->PIDGet();
	pthread_mutex_unlock(&m_mutex);
	return error;
}

/*
 * Set the percentage error which is considered tolerable for use with
 * OnTarget.
 * @param percentage error which is tolerable
 */
void PID1983Controller::SetTolerance(float percent) {
	pthread_mutex_lock(&m_mutex);
		m_toleranceType = kPercentTolerance;
		m_tolerance = percent;
	pthread_mutex_unlock(&m_mutex);
}

/*
 * Set the percentage error which is considered tolerable for use with
 * OnTarget.
 * @param percentage error which is tolerable
 */
void PID1983Controller::SetPercentTolerance(float percent) {
	pthread_mutex_lock(&m_mutex);
		m_toleranceType = kPercentTolerance;
		m_tolerance = percent;
	pthread_mutex_unlock(&m_mutex);
}

/*
 * Set the absolute error which is considered tolerable for use with
 * OnTarget.
 * @param percentage error which is tolerable
 */
void PID1983Controller::SetAbsoluteTolerance(float absTolerance) {
	pthread_mutex_lock(&m_mutex);
		m_toleranceType = kAbsoluteTolerance;
		m_tolerance = absTolerance;
	pthread_mutex_unlock(&m_mutex);
}

/*
 * Return true if the error is within the percentage of the total input range,
 * determined by SetTolerance. This asssumes that the maximum and minimum input
 * were set using SetInput.
 * Currently this just reports on target as the actual value passes through the setpoint.
 * Ideally it should be based on being within the tolerance for some period of time.
 */
bool PID1983Controller::OnTarget() {
	bool temp;
	pthread_mutex_lock(&m_mutex);
		switch (m_toleranceType) {
			case kPercentTolerance:
				temp = fabs(GetError()) < (m_tolerance / 100 * (m_maximumInput - m_minimumInput));
				break;
			case kAbsoluteTolerance:
				temp = fabs(GetError()) < m_tolerance;
				break;
			//TODO: this case needs an error
			case kNoTolerance:
				temp = false;
				break;
			default:
				//TODO what goes here?
				break;
		}
	pthread_mutex_unlock(&m_mutex);
	return temp;
}

/**
 * Begin running the PID1983Controller
 */
void PID1983Controller::Enable() {
	pthread_mutex_lock(&m_mutex);
		m_enabled = true;
	pthread_mutex_unlock(&m_mutex);

	if (m_table != NULL) {
		m_table->PutBoolean("enabled", true);
	}
}

/**
 * Stop running the PID1983Controller, this sets the output to zero before stopping.
 */
void PID1983Controller::Disable() {
	pthread_mutex_lock(&m_mutex);
		m_pidOutput->PIDWrite(0);
		m_enabled = false;
	pthread_mutex_unlock(&m_mutex);

	if (m_table != NULL) {
		m_table->PutBoolean("enabled", false);
	}
}

/**
 * Return true if PID1983Controller is enabled.
 */
bool PID1983Controller::IsEnabled() {
	bool enabled;
	pthread_mutex_lock(&m_mutex);
		enabled = m_enabled;
	pthread_mutex_unlock(&m_mutex);
	return enabled;
}

/**
 * Reset the previous error,, the integral term, and disable the controller.
 */
void PID1983Controller::Reset() {
	Disable();

	pthread_mutex_lock(&m_mutex);
		m_prevError = 0;
		m_totalError = 0;
		m_result = 0;
	pthread_mutex_unlock(&m_mutex);
}

std::string PID1983Controller::GetSmartDashboardType() {
	return "PID1983Controller";
}

void PID1983Controller::InitTable(ITable* table) {
	if (m_table!=NULL)
		m_table->RemoveTableListener(this);
	m_table = table;
	if (m_table!=NULL) {
		m_table->PutNumber(kP, GetP());
		m_table->PutNumber(kI, GetI());
		m_table->PutNumber(kD, GetD());
		m_table->PutNumber(kF, GetF());
		m_table->PutNumber(kSetpoint, GetSetpoint());
		m_table->PutBoolean(kEnabled, IsEnabled());
		m_table->AddTableListener(this, false);
	}
}

ITable* PID1983Controller::GetTable() {
	return m_table;
}

void PID1983Controller::ValueChanged(ITable* source, const std::string& key,
		EntryValue value, bool isNew) {
	if (key==kP || key==kI || key==kD || key==kF) {
		if (m_P != m_table->GetNumber(kP) || m_I != m_table->GetNumber(kI)
				|| m_D != m_table->GetNumber(kD) || m_F
				!= m_table->GetNumber(kF) ) {
			SetPID(m_table->GetNumber(kP, 0.0), m_table->GetNumber(kI, 0.0),
					m_table->GetNumber(kD, 0.0), m_table->GetNumber(kF, 0.0));
		}
	} else if (key==kSetpoint && m_setpoint != value.f) {
		SetSetpoint(value.f);
	} else if (key==kEnabled && m_enabled != value.b) {
		if (value.b) {
			Enable();
		} else {
			Disable();
		}
	}
}

void PID1983Controller::UpdateTable() {

}

void PID1983Controller::StartLiveWindowMode() {
	Disable();
}

void PID1983Controller::StopLiveWindowMode() {

}
