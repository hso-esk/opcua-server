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

#include "Logger.h"

#include <boost/regex.hpp>
#include <sstream>

namespace OpcUaLWM2M {

using namespace OpcUaStackCore;

std::map<std::string, LogLevel> Logger::logLevels = {
		{ "error", Error },
		{ "warning", Warning },
		{ "info", Info },
		{ "debug", Debug },
		{ "trace", Trace }
};

// Getting the default logLevel
// NOT SAFE; NO WAY OF KNOWING IF FileLogger HAS BEEN INSTANCED!
LogLevel Logger::displayLevel_ = FileLogger::logLevel();

Logger::Logger() :
		Logger(Error) {
}

Logger::Logger(LogLevel displayLevel) {
	Logger::displayLevel_ = displayLevel;
}

// Empty destructor
Logger::~Logger(){
	// Nothing to destroy!
}

void Logger::replaceWithParameters(std::string &message,
		std::queue<std::string> params) {
	const boost:: regex expression("<>");
	std::string parameter;

	while (params.size() > 0) {
		parameter = params.front();
		params.pop();
		message = boost::regex_replace(message, expression, parameter, boost::regex_constants::format_first_only);
	}
}

LogLevel Logger::getLogLevel(std::string key) {

	// find the appropriate level to the argument
	auto it = logLevels.find(key);

	// if found, return the correct enum value
	if (it != logLevels.end())
		return it->second;
	else
		return Error;
}


bool Logger::setLoggerDisplayLevel(LogLevel loggingLevel) {
	Logger::displayLevel_ = loggingLevel;
	return true;
}

}

