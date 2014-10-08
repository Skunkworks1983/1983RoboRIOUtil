#include "Time.h"
#include "Utility.h"

double getCurrentMillis() {
	return GetFPGATime() * 1.0e-3;
}
