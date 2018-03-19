/*
 * --- License -------------------------------------------------------------- *
 */

/*
 * Copyright (c) 2015
 *
 * Hochschule Offenburg, University of Applied Sciences
 * Institute for reliable Embedded Systems
 * and Communications Electronic (ivESK)
 *
 * All rights reserved
 */

#ifndef NIKI_Logger_Logger_H_
#define NIKI_Logger_Logger_H_

#include "OpcUaStackCore/Core/FileLogger.h"
#include "OpcUaStackCore/Base/Log.h"

#include <stdarg.h>
#include <queue>
#include <map>
#include <string>

namespace OpcUaLWM2M {

using namespace OpcUaStackCore;

class Logger {

	static LogLevel displayLevel_;
	static std::map<std::string, LogLevel> logLevels;

	static void getParameters(std::queue<std::string>& parameterQueue) {}
	static std::string to_string() {return nullptr;}
	static void replaceWithParameters(std::string &message,
			std::queue<std::string> params);

public:

	Logger();
	Logger(LogLevel displayLevel);
	~Logger();
	static LogLevel getLoggerDisplayLevel();
	static void log() {}
	static bool setLoggerDisplayLevel(LogLevel loggingLevel);
	static LogLevel getLogLevel(std::string key);

// --------------------- TEMPLATE INSTANTIATION ----------------------------

private:

	template<typename P>
	static void getParameters(std::queue<std::string>& parameterQueue,
			P parameter) {
		parameterQueue.push(Logger::to_string(parameter));
	}

	template<typename H, typename ... P>
	static void getParameters(std::queue<std::string>& parameterQueue,
			H paramaterHead, P ... parameterTail) {
		parameterQueue.push(Logger::to_string(paramaterHead));
		getParameters(parameterQueue, parameterTail...);
	}

	// Fix for the to_string method, Kudos to Stack Overflow
	template<typename T>
	static std::string to_string(T value) {
		std::ostringstream os;

		os << value;

		return os.str();
	}

public:

	template<typename T = LogLevel, typename ... P>
	static void log(T level, std::string message, P ... parameter) {

		std::queue<std::string> parameters;

		getParameters(parameters, parameter...);

		if (!parameters.empty())
			replaceWithParameters(message, parameters);

		if (Logger::displayLevel_ >= level)
			Log::logout(level, message);
	}
};

}

#endif /* NIKI_Logger_Logger_H_ */
