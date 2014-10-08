#include "DualLiveSpeed.h"

DualLiveSpeed::DualLiveSpeed(SpeedController *aA, SpeedController *bB,
		bool reverseB) {
	this->a = aA;
	this->b = bB;
	this->reverseB = reverseB;
}

DualLiveSpeed::~DualLiveSpeed() {
	delete a;
	if (b != NULL) {
		delete b;
	}
}
void DualLiveSpeed::ValueChanged(ITable* source, const std::string& key,
		EntryValue value, bool isNew) {
	Set(value.f);
}

float DualLiveSpeed::Get() {
	return (0.5 * a->Get())
			+ (b != NULL ? (reverseB ? -0.5 : 0.5) * b->Get() : 0);
}

void DualLiveSpeed::Set(float f, UINT8 syncGroup) {
	a->Set(f, syncGroup);
	if (b != NULL) {
		b->Set(reverseB ? -f : f, syncGroup);
	}
}

void DualLiveSpeed::UpdateTable() {
	if (m_table != NULL) {
		m_table->PutNumber("Value", Get());
	}
}

void DualLiveSpeed::StartLiveWindowMode() {
	if (m_table != NULL) {
		m_table->AddTableListener("Value", this, true);
	}
}

void DualLiveSpeed::StopLiveWindowMode() {
	Set(0);
	if (m_table != NULL) {
		m_table->RemoveTableListener(this);
	}
}

std::string DualLiveSpeed::GetSmartDashboardType() {
	return "Speed Controller";
}

void DualLiveSpeed::InitTable(ITable *subTable) {
	m_table = subTable;
	UpdateTable();
}

void DualLiveSpeed::PIDWrite(float f) {
	Set(f);
}

void DualLiveSpeed::Disable() {
	Set(0);
}

ITable *DualLiveSpeed::GetTable() {
	return m_table;
}
