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

#include <drain/Log.h>
#include <string.h> // strrchr()
#include <stdexcept>

#include <drain/SmartMapTools.h>

namespace drain {

/*
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
*/

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
const Log::status_dict_t Log::statusDict = {
		{LOG_EMERG, "EMERGENGY"},
	//	{LOG_ALERT,   "ALERT"},
	//	{2, "CRITICAL"},
		{LOG_ERR,     "ERROR"},
		{LOG_WARNING, "WARNING"},
		{LOG_NOTICE,  "NOTE"},
		{LOG_INFO,    "INFO"},
		{LOG_DEBUG,   "DEBUG"}
};

#ifdef VT100
bool Log::USE_VT100(true);
#else
bool Log::USE_VT100(false);
#endif

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

	// const drain::Log::status_dict_t & dict = statusDict;
	for (const auto & entry: statusDict){
		if (level == entry.second){
			setVerbosity(entry.first);
			return;
		}

	}

	/*
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
	*/
	//std::cout << "failed:" << level << '\n';

	throw std::runtime_error(level + " - no such error level"); // + drain::sprinter(getDict()));
}

void Log::flush(level_t level, const std::string & prefix, const std::stringstream & sstr){
	static const Notification notif("UNDEFINED", TextStyle::RED, TextStyle::BOLD);

	if (level > verbosityLevel)
		return; // !

	flush(level, notif, prefix, sstr);

	/*
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
	*/

}

void Log::flush(level_t level, const Notification & notif, const std::string & prefix, const std::stringstream & sstr){


	if (level > verbosityLevel)
		return; // !

	std::ostream & ostr = *ostrPtr; // std::cerr;

	if (USE_VT100)
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

	if (USE_VT100)
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

Logger::~Logger(){
	// Change absolute start time to elapsed time.
	//timestamp(); // ?
	endTiming();
}


Notification & getNotif(Logger::level_t level){

	typedef std::map<int,Notification> notif_dict_t;
	static notif_dict_t notifDict;

	notif_dict_t::iterator it = notifDict.find(level);
	if (it != notifDict.end()){
		return it->second;
	}
	else {
		const std::string key =  MapTools::get(Log::statusDict, level);
		it = notifDict.insert(notifDict.begin(), notif_dict_t::value_type(level, Notification(key)));
		//Notification & notif = notifDict[level];
		//notif.key = MapTools::get(Log::statusDict, level, "Msg");
		//MapTools::get(Log::statusDict, level, notif.key);
		// notif.vt100color =
		return it->second;
	}

}

Logger & Logger::initMessage(level_t level){
	// static const Notification notif(__FUNCTION__); // TextStyle::DIM, TextStyle::ITALIC);
	//this->notif_ptr = & notif; // Log::getDict()[level];
	static std::map<int,Notification> notifDict;

	this->notif_ptr = & getNotif(level);
	this->level = level;
	this->message.str("");
	this->message << std::boolalpha;
	return *this;
}



}

// Drain
