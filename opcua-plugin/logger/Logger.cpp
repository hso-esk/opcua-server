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
std::map<std::string, LogLevel>::iterator LoggerIterator;

//Set the initial logging level
LogLevel Logger::displayLevel_ = Debug;

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

	// make sure key value is in lower cases
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);

	// find the appropriate level to the argument
	auto it = logLevels.find(key);

	// if found, return the correct enum value
	if (it != logLevels.end())
		return it->second;
	else
		return Error;
}

LogLevel Logger::getLogLevel() {
	return Logger::displayLevel_;
}


bool Logger::setLoggerDisplayLevel(LogLevel loggingLevel) {
	Logger::displayLevel_ = loggingLevel;
	return true;
}

}

