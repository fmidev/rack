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

#ifndef DRAIN_LOG_H_
#define DRAIN_LOG_H_

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

class Logger;

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
 * Log monitor;
 * Logger mout(monitor, "FunctionName") ;
 * \endcode
 */
/// TODO: add direct feed, perhaps Log::error() could instantiate a source?
class Log  { //: public LogBase {

public:

	///
	/**
	 *   \par level - verbosity
	 */
	Log(int verbosityLevel = LOG_WARNING) : VT100(true), verbosityLevel(verbosityLevel), msgLevel(LOG_NOTICE), // !!
	currentSender(NULL) {
		resetTime();
		//std::cerr << "start monitor, level=" << verbosityLevel << std::endl;
	};

	Log(const Log &m) : VT100(true), verbosityLevel(m.verbosityLevel), msgLevel(m.msgLevel), // !!
			currentSender(NULL) {
		resetTime();
	};


	bool VT100;

	//
	inline
	void setVerbosity(int level){ verbosityLevel = level; };

	inline
	int getVerbosity(){ return verbosityLevel; };

	//Logger s;


	/// Echoes the message, and halts on error.
	// TODO: recode this...
	void start(int level, const std::string & sender);

	template <class T>
	inline
	void handle(int level, const T & message, const std::string & sender){

		/// Start new message, if new sender or new level
		if (( &sender != currentSender ) || (level != msgLevel)){
			start(level, sender);
		}

		//cerr << "monitor input: " << message << '\n';
		if (level <= verbosityLevel)
			sstr << message;
		//cerr << "handle: " << message << '|';

	}


	void flush();

	void flush(int level, const std::string & prefix, const std::stringstream & sstr);

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
		Log &operator<<(const T x) {
			//cerr << "source input:" << x << '\n';
			monitor.handle(errorType, x, prefix);
			return *this;
		}
	 */


	int verbosityLevel;


protected:

	std::stringstream sstr;

	int msgLevel;
	const void * currentSender;

	mutable timeval _time;
	long _millisecondsStart;

	//time_t _time;
};

//extern Log monitor;

Log & getLog();

namespace image {

Log & getImgLog();

}  // namespace image




/// LogSourc e is the means for a function or any program segment to "connect" to a Log.
/**
 *
 */
class Logger { //: public LogBase {

public:

	//Logger(const std::string & className = "", const std::string & funcName = "");
	//Logger(const char *funcName, const char *className = NULL);
	Logger(const char *funcName, const std::string & name = "");
	//Logger(const char *funcName, const char * name);

	Logger(Log &log, const char *funcName, const std::string & name = ""); //const char *className = NULL);

	~Logger(){
		// Change absolute start time to elapsed time.
		time = monitor.getMilliseconds() - time;
		timestamp();
	}



	inline
	bool isLevel(int l){
		return (l <= monitor.getVerbosity());
	};

	inline
	bool isDebug(int l=0){
		return (monitor.getVerbosity() >= (LOG_DEBUG+l));
	};


	/// Quits immediately, dumps pending messages.
	inline
	Logger & quit(){
		initMessage(LOG_EMERG);
		return *this;
	};

	/// Quits immediately, dumps pending messages.
	inline
	Logger & alert(){
		initMessage(LOG_ALERT);
		return *this;
	};

	/// Quits immediately, dumps pending messages.
	inline
	Logger & fatal(){
		initMessage(LOG_CRIT);
		return *this;
	};

	/// Echoes
	inline
	Logger & error(){
		initMessage(LOG_ERR);
		return *this;
	};

	inline
	Logger & warn(){
		initMessage(LOG_WARNING);
		return *this;
	};

	/// Feature has been removed. Special type of Logger::warn().  \see Logger::deprecating().
	inline
	Logger & obsolete(){
		initMessage(LOG_WARNING);
		message << "OBSOLETE. ";
		return *this;
	};

	/// For top-level information
	inline
	Logger & note(){
		initMessage(LOG_NOTICE);
		return *this;
	};

	///  Feature will be removed. Special type of Logger::note(). \see Logger::obsolete().
	inline
	Logger & deprecating(){
		initMessage(LOG_NOTICE);
		message << "DEPRECATING. ";
		return *this;
	};

	inline
	Logger & info(){
		initMessage(LOG_INFO);
		return *this;
	};


	/**
	 *  Internally, this value is added to \c LOG_DEBUG.
	 *  - LOG_DEBUG + 0 - print general debugging information
		    - LOG_DEBUG + 1 - print names for top-level functions entered
		    - LOG_DEBUG + 5 - print names for functions entered
		    - LOG_DEBUG + 5 - include time toOStr
		    - 10 - save results as files
		    - 15 - save intermediate results as files
		    - 20 - dump mass data (eg. coordinates during an image traversal)
	 */
	inline
	Logger & debug(unsigned int level = 0){
		initMessage(LOG_DEBUG + level);
		return *this;
	};

	inline
	Logger & log(unsigned int level){
		initMessage(level);
		return *this;
	};

	/// Send a short [INFO] preceded with a time stamp.
	Logger & timestamp(const std::string & label);

	/// Send a longer [INFO] preceded with a time stamp.
	Logger & timestamp();


	template <class T>
	Logger &operator<<(const T & x) {
		//cerr << "source input:" << x << '\n';
		// OLD: monitor.handle(errorType, x, prefix);
		if (errorType <= monitor.getVerbosity())
			message << x;
		//else message << "//" << x;
		return *this;
	}




	typedef void * oper;
	static oper endl;

	/// Flush operation
	//Logger &operator<<(const oper & op){
	inline
	Logger &operator<<(oper op){
		// std::cerr << "(" << prefix << message.str() << std::endl;
		monitor.flush(errorType, prefix, message);
		//monitor.flush();
		return *this;
	}


	std::stringstream message;

protected:

	/**
	 *  \param name - explicitly given classname like "Composite" or __FILE__
	 *  \param name - __FILE__
	 */
	void setPrefix(const char *functionName, const char * name);

	void initMessage(int errorType);


	Log & monitor;
	//mutable
	std::string prefix;

	int errorType;
	time_t time;

};

//std::ostream &operator<<(std::ostream &ostr, const Log &error);


}

//const std::string FEELU(__FILE__);

#endif /* DEBUG_H_ */

// Drain
