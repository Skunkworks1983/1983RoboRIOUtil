#ifndef LOGGER_H_
#define LOGGER_H_
// CSTDLIB
#include <stdarg.h>
#include <stdlib.h>

class SendableChooser;

class Logger {
public:
	static int kSevere;
	static int kWarning;
	static int kDiagnostic;
	static int kInfo;
	static int kConfig;
	static int kFine;
	static int kFiner;
	static int kFinest;
private:
	static int minLogLevel;
	static SendableChooser *chooser;
	Logger();
	virtual ~Logger();
	static char* logLevelToName(int level) {
		if (level==kSevere)
			return "SEVERE";
		if (level==kWarning)
			return "WARNING";
		if (level==kDiagnostic)
			return "DIAG";
		if (level==kInfo)
			return "INFO";
		if (level==kConfig)
			return "CONFIG";
		if (level==kFine)
			return "FINE";
		if (level==kFiner)
			return "FINER";
		if (level==kFinest)
			return "FINEST";
		return "UNKNOWN";
	}
public:
	static inline void setLoggingLevel(int level) {
		minLogLevel = level;
	}
	static void log(int level, char *src, char *fmt, ...);
	static SendableChooser *createLogLevelChooser();
};

#endif /* LOGGER_H_ */
