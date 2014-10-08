#include "Logger.h"
// CSTDLIB
#include <stdarg.h>
#include <stdio.h>
// WPILib
#include "SmartDashboard/SendableChooser.h"

SendableChooser *Logger::chooser= NULL;

int Logger::minLogLevel = 8;

int Logger::kSevere=1;
int Logger::kWarning=2;
int Logger::kDiagnostic=3;
int Logger::kInfo=4;
int Logger::kConfig=5;
int Logger::kFine=6;
int Logger::kFiner=7;
int Logger::kFinest=8;

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::log(int level, char *src, char *fmt, ...) {
	if (chooser != NULL) {
		int *obj = (int*) chooser->GetSelected();
		if (obj != NULL) {
			if (*obj != minLogLevel) {
				printf("Changing log level to %s\n",
						Logger::logLevelToName(*obj));
			}
			minLogLevel = *obj;
		}
	}
	if (level <= minLogLevel) {
		va_list args;
		va_start(args, fmt);
		char formatted[250];
		vsprintf(formatted, fmt, args);
		printf("[%s] (%s):\t%s\n", Logger::logLevelToName(level), src,
				formatted);
		va_end(args);
	}
}

SendableChooser *Logger::createLogLevelChooser() {
	if (chooser == NULL) {
		chooser = new SendableChooser();
		chooser->AddDefault(logLevelToName(Logger::kFinest), &Logger::kFinest);
		chooser->AddObject(logLevelToName(Logger::kFiner), &Logger::kFiner);
		chooser->AddObject(logLevelToName(Logger::kFine), &Logger::kFine);
		chooser->AddObject(logLevelToName(Logger::kConfig), &Logger::kConfig);
		chooser->AddObject(logLevelToName(Logger::kInfo), &Logger::kInfo);
		chooser->AddObject(logLevelToName(Logger::kDiagnostic),
				&Logger::kDiagnostic);
		chooser->AddObject(logLevelToName(Logger::kWarning), &Logger::kWarning);
		chooser->AddObject(logLevelToName(Logger::kSevere), &Logger::kSevere);
	}
	return chooser;
}
