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
//#include <stdexcept>

//#include <ctime>
#include <sys/time.h>

#include <syslog.h>


#include <vector>

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

struct Notification {

	Notification(const std::string & key="", int vt100color=35){
		set(key, vt100color);
	};

	//void set(const std::string & key="", const std::string & vt100color="");

	void set(const std::string & key, int vt100color);


	std::string key;
	std::string vt100color;

};

// extern unsigned int Debug;  // getting obsolete
typedef std::vector<Notification> notif_dict_t;

/// Sender of log messages.
class Logger;

/// Handler for notifications sent by a Logger.
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
class Log  {

public:

	/// Log verbosity level type
	typedef unsigned short level_t;


	///
	/**
	 *   \par level - verbosity
	 */
	Log(std::ostream & ostr=std::cerr, int verbosityLevel=LOG_WARNING) : VT100(true), ostrPtr(&ostr), verbosityLevel(verbosityLevel)
	{
		resetTime();
		//std::cerr << "start monitor, level=" << verbosityLevel << std::endl;
	};

	Log(const Log &m) : VT100(m.VT100), ostrPtr(m.ostrPtr), verbosityLevel(m.verbosityLevel) {
		resetTime();
	};

	inline
	void setOstr(std::ostream & ostr){
		ostrPtr = &ostr;
	}

	//
	inline
	void setVerbosity(level_t level){
		verbosityLevel = level;
	};

	inline
	int getVerbosity() const {
		return verbosityLevel;
	};

	void flush(level_t level, const std::string & prefix, const std::stringstream & sstr);

	//void flush(level_t level, const Notification & notif, const std::string & prefix, const std::ostream & sstr);
	void flush(level_t level, const Notification & notif, const std::string & prefix, const std::stringstream & sstr);


	inline
	long getRelativeMilliseconds(){
		return getMilliseconds() - millisecondsStart;
	}

	static inline
	long getMilliseconds(){
		timeval time;
		gettimeofday(&time, NULL);
		return (time.tv_sec * 1000) + (time.tv_usec / 1000);
	}

	inline
	void resetTime(){
		millisecondsStart = getMilliseconds();
	};

	bool VT100;

	static
	const notif_dict_t & getDict();

protected:

	std::ostream *ostrPtr;

	level_t verbosityLevel;

	long millisecondsStart;

	//drain::Dictionary2<int, Notification> dict;

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

	typedef Log::level_t level_t;

	/// Start logging,
	/**
	 *  \param funcName - name of the finction, often assigned as standard C macro __FUNCTION__ .
	 *  \param name - specifier of the source, file name for example (to be basenamed).
	 */
	Logger(const char *funcName, const std::string & name = "");
	//Logger(const char *funcName, const char * name);

	/// Start logging,
	/**
	 *  \param funcName - name of the finction, often assigned as standard C macro __FUNCTION__ .
	 *  \param name - specifier of the source, file name for example (to be basenamed).
	 */
	Logger(Log &log, const char *funcName, const std::string & name = ""); //const char *className = NULL);

	~Logger(){
		// Change absolute start time to elapsed time.
		time = monitor.getMilliseconds() - time;
		timestamp();
	}

	/// Returns true, if the log monitor level is at least l.
	/**
	 *   Compares the argument to  the threshold of the receiving Log monitor,
	 *   not to the current level of this logger.
	 */
	inline
	bool isLevel(level_t l){
		return (monitor.getVerbosity() >= l);
	};

	/// Returns true, if the debug level of the monitor is at least l.
	/**
	 *   Compares the argument to  the threshold of the receiving Log monitor,
	 *   not to the current level of this logger.
	 */
	inline
	bool isDebug(level_t l=0){
		return isLevel(LOG_DEBUG+l);
		//return (monitor.getVerbosity() >= (LOG_DEBUG+l));
	};



	/// Quits immediately, dumps pending messages.
	template<typename ... TT>
	inline
	Logger & quit(const TT &... args){
		initMessage<LOG_EMERG>();
		flush(args...);
		return *this;
	};

	/// Quits immediately, dumps pending messages.
	template<typename ... TT>
	inline
	Logger & alert(const TT &... args){
		initMessage<LOG_ALERT>();
		flush(args...);
		return *this;
	};

	/// Quits immediately, dumps pending messages.
	template<typename ... TT>
	inline
	Logger & critical(const TT &... args){
		initMessage<LOG_CRIT>();
		flush(args...);
		return *this;
	};

	/// Echoes
	/*
	inline
	Logger & error(){
		return initMessage<LOG_ERR>();
	};
	*/

	template<typename ... TT>
	inline
	Logger & error(const TT &... args){
		// error();
		initMessage<LOG_ERR>();
		flush(args...);
		return *this;
	};

	/// Light error, command execution probable. Special type of Logger::warn().
	/*
	inline
	Logger & warn(){
		return initMessage<LOG_WARNING>();
	};
	*/

	/// Possible error, but execution can continue.
	template<typename ... TT>
	inline
	Logger & warn(const TT &... args){
		// warn();
		initMessage<LOG_WARNING>();
		flush(args...);
		return *this;
	};


	/// Possible error, but execution can continue. Special type of Logger::warn().
	template<typename ... TT>
	inline
	Logger & fail(const TT &... args){
		static const Notification notif(__FUNCTION__, 33);
		initMessage<LOG_WARNING>(notif);
		flush(args...);
		return *this;
	};

	/// Feature has been removed. Special type of Logger::warn().  \see Logger::deprecating().
	//  Valid alternative should be displayed.
	template<typename ... TT>
	inline
	Logger & obsolete(const TT &... args){
		static const Notification notif(__FUNCTION__, 35);
		initMessage<LOG_WARNING>(notif);
		flush(args...);
		return *this;
	};


	/// For top-level information
	template<typename ... TT>
	inline
	Logger & note(const TT &... args){
		//note();
		initMessage<LOG_NOTICE>();
		flush(args...);
		return *this;
	};

	///  Feature will be removed. Special type of Logger::note(). \see Logger::obsolete().
	template<typename ... TT>
	inline
	Logger & deprecating(const TT &... args){
		static const Notification notif(__FUNCTION__, 33);
		initMessage<LOG_NOTICE>(notif);
		flush(args...);
		return *this;
	};

	///  Feature will be removed. Special type of Logger::note(). \see Logger::obsolete().
	template<typename ... TT>
	inline
	Logger & unimplemented(const TT &... args){
		static const Notification notif(__FUNCTION__, 35);
		initMessage<LOG_NOTICE>(notif);
		flush(args...);
		return *this;
	};


	template<typename ... TT>
	inline
	Logger & special(const TT &... args){
		static const Notification notif(__FUNCTION__, 36);
		initMessage<LOG_NOTICE>(notif);
		flush(args...);
		return *this;
	};

	/*
	inline
	Logger & experimental(){
		static const Notification notif(__FUNCTION__, 94);
		return initMessage<LOG_NOTICE>(notif);
		// return *this;
	};
	*/

	template<typename ... TT>
	inline
	Logger & experimental(const TT &... args){
		static const Notification notif(__FUNCTION__, 94);
		initMessage<LOG_NOTICE>(notif);
		flush(args...);
		return *this;
	};

	template<typename ... TT>
	inline
	Logger & ok(const TT &... args){
		static const Notification notif(__FUNCTION__, 32);
		initMessage<LOG_INFO>(notif);
		flush(args...);
		return *this;
	};

	template<typename ... TT>
	inline
	Logger & success(const TT &... args){
		static const Notification notif(__FUNCTION__, 92);
		initMessage<LOG_NOTICE>(notif);
		flush(args...);
		return *this;
	};

	template<typename ... TT>
	inline
	Logger & info(const TT &... args){
		initMessage<LOG_INFO>();
		flush(args...);
		return *this;
	};


	/// Debug information.
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
	template<typename ... TT>
	inline
	Logger & debug(const TT &... args){
		initMessage<LOG_DEBUG>();
		flush(args...);
		return *this;
	};

	/// Debug information.
	template<typename ... TT>
	inline
	Logger & debug2(const TT &... args){
		initMessage<LOG_DEBUG+1>();
		flush(args...);
		return *this;
	};

	template<typename ... TT>
	inline
	Logger & debug3(const TT &... args){
		initMessage<LOG_DEBUG+2>();
		flush(args...);
		return *this;
	};



	inline
	Logger & debug(level_t level){
		static const Notification notif("DEBUG*", 49);
		return initMessage<LOG_DEBUG+1>(notif); // obsolete
		//return *this;
	};

	inline
	Logger & log(level_t level){
		return initMessage(level); // avoid
		//return *this;
	};

	/// Send a short [INFO] preceded with a time stamp.
	Logger & timestamp(const std::string & label);

	/// Send a longer [INFO] preceded with a time stamp.
	Logger & timestamp();


	/// Simple assignment and direct flush
	template <class T>
	Logger &operator=(const T & x) {
		message.str();
		this->operator <<(x);
		monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	}

	/// Direct

	Logger &operator<<(const std::ostream & sstr) {
		// TODO if (!message.empty())
		if (level <= monitor.getVerbosity())
			message << sstr.rdbuf();
		return *this;
	}



	template <class T>
	Logger &operator<<(const T & x) {
		//cerr << "source input:" << x << '\n';
		if (level <= monitor.getVerbosity())
			message << x;
		return *this;
	}

	typedef void * oper;
	static oper endl;

	/// Handling flush operator
	inline
	Logger &operator<<(oper op){
		monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	}

	/// NEW: sending "mout" insread of "mout.endl" Handling flush operator
	inline
	Logger &operator<<(const Logger & l){

		if (&l != this){
			message << " NOTE: Logger" << __FUNCTION__ << " flush with non-this Logger";
		}
		monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	}


	std::stringstream message;

protected:

	Log & monitor;

	std::string prefix;

	level_t level;
	time_t time;

	const Notification * notif_ptr;

	/**
	 *  \param name - explicitly given classname like "Composite" or __FILE__
	 *  \param name - __FILE__
	 */
	void setPrefix(const char *functionName, const char * name);

	template <level_t L>
	Logger & initMessage(const Notification & notif){
		this->notif_ptr = & notif;
		this->level = L;
		this->message.str("");
		return *this;
	}
	//void initMessage(level_t level);

	template <level_t L>
	Logger & initMessage(){

		static
		const Notification & notif = Log::getDict()[L];

		this->notif_ptr = &notif;
		this->level = L;
		this->message.str("");
		return *this;
	}

	inline
	Logger & initMessage(level_t level){
		this->notif_ptr = & Log::getDict()[level];
		this->level = level;
		this->message.str("");
		return *this;
	}


	template<typename T, typename ... TT>
	inline
	Logger & flush(const T & arg, const TT &... rest){
		*this << arg;
		flush(rest...);
		return *this;
	};

	template<typename T>
	inline
	Logger & flush(const T & arg){
		*this << arg;
		monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	};

	inline
	Logger & flush(){
		//monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	};

	/*
	inline
	Logger & flush(){
		monitor.flush(level, *notif_ptr, prefix, message);
		return *this;
	};
	*/


};

//std::ostream &operator<<(std::ostream &ostr, const Log &error);


}

//const std::string FEELU(__FILE__);

#endif /* DEBUG_H_ */

// Drain
