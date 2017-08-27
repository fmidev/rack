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

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdlib.h>
#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>

//#include <ctime>
#include <sys/time.h>

#include <syslog.h>
/*
#define	LOG_EMERG	0	// system is unusable //
#define	LOG_ALERT	1	// action must be taken immediately //  RACK examples
#define	LOG_CRIT	2	// critical conditions //
#define	LOG_ERR		3	// error conditions //              //  File read/write failed
#define	LOG_WARNING	4	// warning conditions //            //  HDF5 empty or contains no relevant data
#define	LOG_NOTICE	5	// normal but significant condition //  No volumes used for a composite
#define	LOG_INFO	6	// informational //                 //  Read/Write File, start/end of operator
#define	LOG_DEBUG	7	// debug-level messages             //  Store variable
                   10                                       //  Write control files (function input, output)
                   11                                       //  Write intermediate results in files
                   15                                       //  Debug 1D loops
                   20                                       //  Debug 2D loops
 */


/*
#ifdef __GNUC__
	inline std::string className(const std::string & prettyFunction)
	{
		const size_t colons = prettyFunction.find("::");
		if (colons == std::string::npos){
			return "::";
		}
		else {
			const size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
			const size_t end = colons - begin;
			return prettyFunction.substr(begin,end);
		}
}
#define CLASSNAME className(__PRETTY_FUNCTION__)
#else
#define CLASSNAME __FILE__
#endif
*/

namespace drain {

// using namespace std;


// extern unsigned int Debug;  // getting obsolete

class MonitorSource;

/// Handler for notifications sent by
/**
 *  \code
 *  debug.setVerbosity(level);
 *  debug.error() << " overflow error, value=" << 5 << '\n';
 *  debug.endl();
 *  \endcode
 *
 *  \code
 *  debug.error() << " overflow error, value=" << 5 << debug.endl();
 *  \endcode
 *
 * \code
 * Monitor monitor;
 * MonitorSource mout(monitor, "FunctionName") ;
 * \endcode
 */
/// TODO: add direct feed, perhaps Monitor::error() could instantiate a source?
class Monitor  { //: public MonitorBase {

public:

	///
	/**
	 *   \par level - verbosity
	 */
	Monitor(int verbosityLevel = LOG_WARNING) : VT100(true), _verbosityLevel(verbosityLevel), _currentLevel(LOG_NOTICE), // !!
	_currentSender(NULL) {
		resetTime();
	};

	Monitor(const Monitor &m) : VT100(true), _verbosityLevel(m._verbosityLevel), _currentLevel(m._currentLevel), // !!
			_currentSender(NULL) {
		resetTime();
	};

	bool VT100;

	//
	inline
	void setVerbosity(int level){ _verbosityLevel = level; };

	inline
	int getVerbosity(){ return _verbosityLevel; };

	//MonitorSource s;


	/// Echoes the message, and halts on error.
	inline
	void start(int level, const std::string & sender){

		/// Flush current buffer.
		flush();

		if ((_verbosityLevel >= LOG_INFO) && (level <= _verbosityLevel)){
			const char c = sstr.fill('0');
			sstr.width(5);
			sstr << getRelativeMilliseconds() << ':';
			sstr.width(0);
			sstr.fill(c);
		}

		if (level <= LOG_ERR){
			if (VT100)
				//sstr << "\033[1;43mFAILED\033[0m"
				sstr << "\033[1;31m";
			sstr << "[ERROR] ";
		}
		else if (level <= LOG_WARNING){
			if (VT100)
				//sstr << "\033[1;43mFAILED\033[0m"
				sstr << "\033[1;33m";
			sstr << "[WARNING] ";
		}
		else if (level <= LOG_NOTICE){
			if (level <= _verbosityLevel){
				if (VT100)
					sstr << "\033[1;29m";
				sstr << "[NOTICE] ";
			}
		}
		else if (level <= LOG_INFO){
			if (level <= _verbosityLevel)
				sstr << "[INFO] ";
		}
		else {
		}

		if (level <= _verbosityLevel)
			if ( !sender.empty() )
				sstr << sender << ':' << ' ';

		//if (level <= _verbosityLevel)
		//	_init(level, sender);
		_currentLevel  = level;
		_currentSender = &sender;

	};

	template <class T>
	inline
	void handle(int level, const T & message, const std::string & sender){

		if (( &sender != _currentSender ) || (level != _currentLevel)){
			start(level, sender);
		}

		//cerr << "monitor input: " << message << '\n';

		if (level <= _verbosityLevel)
			sstr << message;

		//cerr << "handle: " << message << '|';
	}

	inline
	void flush(){

		std::string s;

		if (sstr.str().length() > 0){
			if (VT100)
				sstr << "\033[0m";
			//cerr << "\033[0m" << std::flush;
			s = sstr.str();
			std::cerr << s << std::endl;
			sstr.str("");  // clear!
		}

		if (_currentLevel <= LOG_ALERT){
			std::cerr << " Fatal error, quitting." << std::endl;
			if (VT100)
				std::cerr << "\033[0m" << std::flush;
			exit(-1);
		}
		else if (_currentLevel <= LOG_ERR){
			_currentLevel = LOG_NOTICE;
			if (VT100)
				std::cerr << "\033[0m" << std::flush;
			throw std::runtime_error(s);
		}

		_currentLevel = LOG_NOTICE;
		_currentSender = NULL;
	}


	inline
	long getRelativeMilliseconds(){
		return getMilliseconds() - _millisecondsStart;
	}

	inline
	long getMilliseconds(){
		gettimeofday(&_time, NULL);
		return (_time.tv_sec * 1000) + (_time.tv_usec / 1000);
	}

	inline
	void resetTime(){ _millisecondsStart = getMilliseconds(); };


	/*
		template <class T>
		Monitor &operator<<(const T x) {
			//cerr << "source input:" << x << '\n';
			_monitor.handle(_errorType, x, _prefix);
			return *this;
		}
	 */



protected:

	int _verbosityLevel;
	std::stringstream sstr;

	int _currentLevel;
	const void * _currentSender;

	mutable timeval _time;
	long _millisecondsStart;

	//time_t _time;
};

extern Monitor monitor;


/// MonitorSourc e is the means for a function or any program segment to "connect" to a Monitor.
/**
 *
 */
class MonitorSource { //: public MonitorBase {

public:

	MonitorSource(const std::string & className = "", const std::string & funcName = "") :
		_monitor(monitor), _prefix(className + (funcName.empty()?"":":")+funcName), _errorType(LOG_NOTICE) {};

	MonitorSource(Monitor &monitor, const std::string & className = "", const std::string & funcName = "") :
		_monitor(monitor), _prefix(className + (funcName.empty()?"":":")+funcName), _errorType(LOG_NOTICE) {};

	/*
		MonitorSource(Monitor &monitor, const std::string &name = "") :
			_monitor(monitor), _prefix(name), _errorType(LOG_NOTICE) {
		};
	 */

	/*
		inline
		void setLocalName(const std::string & name){
			_prefix = _className + ':' + name;
		};

		inline
		void setClassName(const std::string & name){
			_className = name;
		};

		inline
		void setFunctionName(const std::string & name){
			_functionName = name;
		};
	 */


	inline
	bool isLevel(int l){ return (l <= _monitor.getVerbosity()); };

	inline
	bool isDebug(int l=0){ return (_monitor.getVerbosity() >= (LOG_DEBUG+l)); };


	/// Quits immediately, dumps pending messages.
	inline
	MonitorSource & quit(){ _init(LOG_EMERG); return *this; };

	/// Quits immediately, dumps pending messages.
	inline
	MonitorSource & fatal(){ _init(LOG_CRIT); return *this; };

	/// Echoes
	inline
	MonitorSource & error(){ _init(LOG_ERR);  return *this; };

	inline
	MonitorSource & warn(){ _init(LOG_WARNING); return *this; };

	/// For top-level information
	inline
	MonitorSource & note(){ _init(LOG_NOTICE); return *this; };

	inline
	MonitorSource & info(){ _init(LOG_INFO); return *this; };


	/// Send a short [INFO] preceeded with a time stamp.
	MonitorSource & timestamp(const std::string & label);

	/// Send a longer [INFO] preceeded with a time stamp.
	MonitorSource & timestamp();

	/**
	 *  Internally, this value is added to \c LOG_DEBUG.
	 *  - LOG_DEBUG + 0 - print general debugging information
		    - LOG_DEBUG + 1 - print names for top-level functions entered
		    - LOG_DEBUG + 5 - print names for functions entered
		    - LOG_DEBUG + 5 - include time info
		    - 10 - save results as files
		    - 15 - save intermediate results as files
		    - 20 - dump mass data (eg. coordinates during an image traversal)
	 */
	inline
	MonitorSource & debug(unsigned int level = 0){_init(LOG_DEBUG + level); return *this; };


	template <class T>
	MonitorSource &operator<<(const T & x) {
		//cerr << "source input:" << x << '\n';
		_monitor.handle(_errorType, x, _prefix);
		return *this;
	}


	typedef bool oper;
	static oper endl;

	/// Flush operation
	MonitorSource &operator<<(const oper & op){

		_monitor.flush();
		// cerr << "ERROR TYPE=" << _errorType << '\n';
		return *this;
		// TODO: consider return void!
	}


protected:

	// consider _className  and _prefix => _memberName
	void _init(int errorType){
		//cerr << "INIT:" << errorType << "/" << _prefix << '\n';
		_errorType = errorType;
		_monitor.start(errorType, _prefix);
	};

	Monitor & _monitor;
	mutable std::string _prefix;
	std::string _className;
	std::string _functionName;
	//std::string _localName;
	int _errorType;
};

//std::ostream &operator<<(std::ostream &ostr, const Monitor &error);






}

#endif /* DEBUG_H_ */

// Drain
