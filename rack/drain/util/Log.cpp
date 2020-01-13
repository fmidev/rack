/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */
/*
 * Debug.h
 *
 *  Created on: Aug 20, 2010
 *      Author: mpeura
 */

#include <string.h>

#include "Log.h"

namespace drain {

//Log monitor;


Log & getLog(){

	static Log log;
	return log;

}

namespace image {

Log & getImgLog(){

	static Log log;
	return log;

}

}

/// Echoes the message, and halts on error.
// TODO: recode this...
void Log::start(int level, const std::string & msgSender){

	/// Flush current buffer (Should be done already, but if not...)
	flush();

	msgLevel = level;
	currentSender = &msgSender;

	if (msgLevel > verbosityLevel)
		return;

	if ((verbosityLevel > LOG_INFO) && (verbosityLevel >= msgLevel)){
		const char c = sstr.fill('0');
		sstr.width(5);
		sstr << getRelativeMilliseconds() << ':';
		sstr.width(0);
		sstr.fill(c);

	}


	if (msgLevel <= LOG_CRIT){
		if (VT100)
			sstr << "\033[1;31m";
		sstr << "[FATAL]  ";
	}
	else if (msgLevel <= LOG_ERR){
		if (VT100)
			//sstr << "\033[1;43mFAILED\033[0m"
			sstr << "\033[1;35m";
		sstr << "[ERROR]  ";
	}
	else if (msgLevel <= LOG_WARNING){
		if (VT100)
			//sstr << "\033[1;43mFAILED\033[0m"
			sstr << "\033[1;33m";
		sstr << "[WARNING] ";
	}
	else if (msgLevel <= LOG_NOTICE){
		if (verbosityLevel >= msgLevel){
			if (VT100)
				sstr << "\033[1;29m";
			sstr << "[NOTICE] ";
		}
	}
	else if (msgLevel <= LOG_INFO){
		if (verbosityLevel >= msgLevel)
			sstr << "[INFO]   ";
	}
	else if (msgLevel <= LOG_DEBUG){
		if (verbosityLevel >= msgLevel){
			if (VT100)
				sstr << "\033[1;36m";
			sstr << "[DEBUG]  ";
		}
		//if (verbosityLevel >= msgLevel)
		//      sstr << "[INFO] ";
	}
	else if (msgLevel <= (LOG_DEBUG+1)){
		if (verbosityLevel >= msgLevel){
			if (VT100)
				sstr << "\033[1;34m";
			sstr << "[debug]  ";
		}
	}
	else {
	}


	if (verbosityLevel >= (msgLevel + 1)){
		if (!msgSender.empty())
			sstr << msgSender << ':' << ' ';
	}
	else if (verbosityLevel >= msgLevel) // Hide sender in basic notifications
		if (((msgLevel != LOG_NOTICE) && (msgLevel != LOG_INFO)) || (verbosityLevel >= LOG_DEBUG))
			if (!msgSender.empty())
				sstr << msgSender << ':' << ' ';

	//if (verbosityLevel >= msgLevel)
	//	init(msgLevel, msgSender);


}

void Log::flush(){

	std::string s; // debug

	if (!sstr.str().empty()){
		if (VT100)
			sstr << "\033[0m";
		//cerr << "\033[0m" << std::flush;
		s = sstr.str();
		std::cerr << s << std::endl;
	}

	if (msgLevel <= LOG_ALERT){
		std::cerr << "LOG_ALERT " << sstr.str() << std::endl;
		std::cerr << " Fatal error, quitting." <<  msgLevel << std::endl;
		if (VT100)
			std::cerr << "\033[0m" << std::flush;
		// char *c = (char *)random(); // ???
		// std::cerr << c << std::endl;
		//////
		exit(-1);
	}
	else if (msgLevel <= LOG_ERR){
		msgLevel = LOG_NOTICE;
		if (VT100)
			std::cerr << "\033[0m" << std::flush;
		sstr.str("");
		throw std::runtime_error(s);
	}

	sstr.str("");  // clear!
	msgLevel = LOG_NOTICE;
	currentSender = NULL;
}


Logger::oper Logger::endl;

// Logger(const std::string & funcName, const std::string & className) :
Logger::Logger(const char *funcName, const std::string & className): //const char *className):
	monitor(getLog()), errorType(LOG_NOTICE), time(getLog().getMilliseconds()){
	// prefix(className + (funcName.empty()?"":":")+funcName),
	setPrefix(funcName, className.c_str());
}


	// Logger(Log &log, const std::string & funcName, const std::string & className) :
Logger::Logger(Log &log, const char *funcName, const std::string & className): // char *className):
	monitor(log), errorType(LOG_NOTICE), time(log.getMilliseconds()){
	// prefix(className + (funcName.empty()?"":":")+funcName),
	setPrefix(funcName, className.c_str());
}

void Logger::setPrefix(const char *functionName, const char *name){

	//std::stringstream sstr;

	if (name){
		const char * s2 = strrchr(name, '/');
		if (s2 == NULL)
			s2 = name;
		else
			++s2;

		/// Start from s2, because dir may contain '.'
		const char * s3 = strrchr(s2, '.');
		if (s3 == NULL)
			//sstr << s2;
			prefix.assign(s2);
		else
			prefix.assign(s2, s3-s2);
			//sstr.put("s2", size_t(s3-s2));

		prefix.append(":");
		//sstr << ':';
	}
	//sstr << ;
	prefix.append(functionName);
}

Logger & Logger::timestamp(const std::string & label){
	init(LOG_DEBUG);
	*(this) << "TIME#" << label << Logger::endl;
	return *this;
}

Logger & Logger::timestamp(){
	init(LOG_DEBUG);
	*(this) << "TIME# " << time;
	return *this;
}

}

// Drain
