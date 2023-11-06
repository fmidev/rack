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
 * Log.h
 *
 *  Created on: Aug 20, 2010
 *      Author: mpeura
 */

#include <string.h> // strrchr()
#include <stdexcept>

#include "Log.h"

namespace drain {


void Notification::set(const std::string & key, int vt100color){
	this->key = key;
	if (vt100color > 0){
		std::stringstream sstr; //"\033[1;31m";//"\033[0m";
		sstr << "\033[1;" << vt100color << 'm';
		this->vt100color = sstr.str();
	}
	else
		this->vt100color = "\033[0m"; // VT100_RESET
}


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

/*
#define	LOG_EMERG	0	// system is unusable //
#define	LOG_ALERT	1	// action must be taken immediately //  RACK examples
#define	LOG_CRIT	2	// critical conditions //
#define	LOG_ERR		3	// error conditions //              //  File read/write failed
#define	LOG_WARNING	4	// warning conditions //            //  HDF5 empty or contains no relevant data
#define	LOG_NOTICE	5	// normal but significant condition //  No volumes used for a composite
#define	LOG_INFO	6	// informational //                 //  Read/Write File, start/end of operator
#define	LOG_DEBUG	7	// debug-level messages             //  Store variable
*/

const Log::notif_dict_t & Log::getDict(){
	static Log::notif_dict_t dict;
	if (dict.empty()){
		//dict.resize(64);
		dict.resize(16);
		dict[LOG_EMERG].set("EMERG", 101);
		dict[LOG_ALERT].set("ALERT", 91);
		dict[LOG_CRIT].set("CRIT", 41);
		dict[LOG_ERR].set("ERROR", 31); // note ERR -> ERROR
		dict[LOG_WARNING].set("WARNING", 33);
		dict[LOG_NOTICE].set("NOTICE", 29);
		dict[LOG_INFO].set("INFO", 0);
		dict[LOG_DEBUG].set("DEBUG", 35);
		dict[LOG_DEBUG+1].set("DEBUG2", 34);
		dict[LOG_DEBUG+2].set("DEBUG3", 90);
	}
	return dict;
}

/*
void Notification::set(const std::string & key, const std::string & vt100color){
	this->key = key;
	this->vt100color = vt100color;
}
*/

void Log::setVerbosity(const std::string & level){

	if (level == "0"){
		setVerbosity(0);
		return;
	}

	// const size_t i = atoi(level.c_str());
	level_t i=0;
	std::stringstream sstr(level);
	sstr >> i;
	// std::cout << "setVerbosity: " << level << ':' << i << '\n';
	if (i > 0){
		setVerbosity(i);
		return;
	}

	const drain::Log::notif_dict_t & dict = getDict();
	for (size_t i=0; i<dict.size(); ++i){
		const drain::Notification & notif = dict[i];
		//if (!notif.key.empty())
		// std::cout << i << '=' << notif.key << '\n';
		if (notif.key == level){
			setVerbosity(i);
			return;
		}
	}
	//std::cout << "failed:" << level << '\n';

	throw std::runtime_error(level + " - no such error level"); // + drain::sprinter(getDict()));
}

void Log::flush(level_t level, const std::string & prefix, const std::stringstream & sstr){

	if (level > verbosityLevel)
		return; // !

	static
	const notif_dict_t & dict = getDict();

	if (level < dict.size()){
		const Notification & n = dict[level];
		flush(level, n, prefix, sstr);
	}
	else {
		const static Notification unknown("???", 35);
		flush(level, unknown, prefix, sstr);
	}


}

void Log::flush(level_t level, const Notification & notif, const std::string & prefix, const std::stringstream & sstr){


	if (level > verbosityLevel)
		return; // !

	std::ostream & ostr = *ostrPtr; // std::cerr;

	if (VT100)
		ostr << notif.vt100color;

	ostr << '['; ;
	ostr.width(8);
	ostr << notif.key << ']' << ' ';

	// Future extension (thread id?)
	// if (id > 0)	ostr << id << ' ';

	if (!prefix.empty() && (level < (verbosityLevel)))
		ostr << prefix << ':' << ' ';

	//if (!sstr.eof())
	//sstr.gcount()
	std::streambuf *buf = sstr.rdbuf();

	if (buf->in_avail() > 0) // IMPORTANT! Otherwiae corrupts (swallows) output stream
		ostr << buf;
	//ostr << sstr.str();

	if (VT100)
		ostr << "\033[0m"; // END
	ostr << '\n';

	if (level <=  LOG_EMERG){ // 0
		ostr << " Fatal error, quitting." << std::endl;
		exit(-1);
	}
	else if ((level <= LOG_ERR) && (level < (verbosityLevel))){
		std::stringstream sstr2;
		//sstr2 << sstr.rdbuf();
		sstr2 << prefix << ':' << ' ' << sstr.str();
		throw std::runtime_error(sstr2.str());
	}

}


bool Logger::TIMING = false;
char Logger::MARKER = '@'; // for filtering output

Logger::oper Logger::endl;

Logger::Logger(const char *filename, const std::string & className):
				message(*this),
				timing(false),
				monitor(getLog()),
				level(LOG_NOTICE),
				time(getLog().getMilliseconds()),
				notif_ptr(NULL){
	setPrefix(filename, className.c_str());
}

Logger::Logger(Log &log, const char *filename, const std::string & className):
				message(*this),
				timing(false),
				monitor(log),
				level(LOG_NOTICE),
				time(log.getMilliseconds()),
				notif_ptr(NULL){
	setPrefix(filename, className.c_str());
}

/*
Logger & Logger::timestamp(const std::string & label){
	//initMessage(LOG_DEBUG);
	// *(this) << "TIME#" << label << Logger::endl;
	timing = TIMING;
	std::cerr << "TIMING:" << MARKER << "<li>" << label << "...<ol>" << '\n';
	//std::cerr << << label;
	return *this;
}

Logger & Logger::timestamp(){
	return timestamp(this->prefix);
	//initMessage(LOG_DEBUG);
	//  *(this) << "TIME# " << time;
	// return *this;
}
*/

Logger::~Logger(){
	// Change absolute start time to elapsed time.
	//timestamp(); // ?
	endTiming();
}

void Logger::setPrefixOLD(const char *functionName, const char *name){

	if (name){

		if (*name == '\0')
			return;

		const char * s2 = strrchr(name, '/');
		if (s2 == nullptr)
			s2 = name;
		else
			++s2;

		/// Start from s2, because dir may contain '.'
		const char * s3 = strrchr(s2, '.');
		if (s3 == nullptr)
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

/*
void Logger::initMessage(level_t level){
	notif_ptr = NULL;
	this->level = level;
	message.str("");
}

void Logger::initMessage(level_t level, const Notification & notif){
	notif_ptr = &notif;
	this->level = level;
	message.str("");
}
*/



}

// Drain
