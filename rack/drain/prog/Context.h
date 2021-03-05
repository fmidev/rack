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

#ifndef DRAIN_CONTEXT_H_
#define DRAIN_CONTEXT_H_


#include <unistd.h>  // process id, PID

//#include <map>
#include <set>
#include <fstream>
//#include "drain/util/Debug.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/Log.h"
//#include "drain/util/BeanLike.h"
//#include "drain/util/ReferenceMap.h"

#include "drain/util/Cloner.h"

#include "drain/util/StatusFlags.h"
#include "drain/util/StringMapper.h"
#include "drain/util/VariableMap.h" // statusMap



namespace drain {



class Context {

public:

	Context() : id(++counter){
		statusMap["ID"] = id;        // constant
		statusMap["PID"] = getpid(); // constant
		statusMap["statusFlags.dictionary"] = statusFlags.dictionary.toStr();
	}

	/// Defines unique id.
	Context(const Context & ctx) : id(++counter + 100*ctx.id){
		log.setVerbosity(ctx.log.getVerbosity());
		statusMap["ID"] = id;        // constant
		statusMap["PID"] = getpid(); // constant
		statusMap["statusFlags.dictionary"] = statusFlags.dictionary.toStr();
	}

	long int getId(){
		return id;
	}


	virtual
	~Context(){
		if (logFileStream.is_open())
			logFileStream.close();
	}

	Log log;

	/// Optional log filename (syntax): when defined, automatically opened by CommandBank::run()
	/**
	 *
	 */
	std::string logFile;
	std::ofstream logFileStream;

	/// Optional facility for compact bookkeeping of issues, also in running a thread.
	StatusFlags statusFlags;

	/// A long description of context variables and other resources.
	/**
	 *   The basic version saves id, status flags and (expanded) logFile name
	 */
	virtual inline
	drain::VariableMap & getStatusMap(){
		updateStatus();
		return statusMap;
	};


	inline
	const Variable & getStatus(const std::string & key) const {
		updateStatus();
		return statusMap[key];
		//return static_cast<T>(statusMap[key]);
	};

	// non-virtual

	template <class T>
	inline
	void setStatus(const std::string & key, const T & value){
		statusMap[key] = value;
	};


	/// Report status. Typically, report final status of a thread to its base context.
	/**
	 * 	Derived classes may handle and reset some flags before reporting.
	 */
	virtual
	inline
	void report(StatusFlags & flags){
		if (statusFlags.value > 0){
			flags.set(statusFlags.value);
		}
	}


	const long int id;

protected:

	//drain::FlexVariableMap statusMap;
	mutable
	drain::VariableMap statusMap;

	void updateStatus() const {
		//statusMap["logFile"]     = logFile;
		statusMap["statusFlags"] = statusFlags.value;
		statusMap["statusKeys"] = statusFlags.getKeys();
		//return statusMap;
	};

private:

	static long int counter;


	//drain::VariableMap statusMap;
};


/// Utilities
/*
// See Rack resources.sh
*/
class ContextKit {

public:

	inline
	ContextKit() : expandVariables(false) {
	};

	// virtual	drain::FlexVariableMap & getStatus();

	//
	bool expandVariables;

	///
	/**
	 *   \see drain::CmdFormat
	 *   \see drain::CmdFormatFile
	 */
	std::string formatStr;

	// mutable drain::StringMapper statusFormatter;

};

class SmartContext : public Context, public ContextKit {

public:

	SmartContext(){
		//linkStatusVariables();
	};

	SmartContext(const SmartContext & ctx) : Context(ctx), ContextKit(ctx){
		//linkStatusVariables();
	}

	/// A lisiting of context variables and other resources.
	/**
	 *   This basic version contains id, status flags and (expanded) logFile name
	 */
	//virtual drain::FlexVariableMap & getStatus();

private:

	//void linkStatusVariables();

};


typedef ClonerBase<Context> ContextClonerBase;
//class ContextClonerBase : public ClonerBase<Context>{
//};

/**
 *
 *  \param BC - Context base class (Context or SmartContext)
 *
 */

/*
template <class C, class BC=Context>
class ContextCloner : public Cloner<BC,C> {
};
*/

class Contextual {

public:

	/// Sets internal contextPtr to NULL.
	inline
	Contextual() : contextPtr(NULL) {};

	/// Copies base context (even null) of the source. Notice that the actual instance may be of derived class.
	inline
	Contextual(const Contextual & src) : contextPtr(src.contextPtr){ // or always null?
		//setExternalContext(src.getBaseContext()); ? not logical, consider C1 and C2(C1);
	};

	/// Copies internal contextPtr.
	inline
	Contextual(Context & ctx){
		setExternalContext(ctx);
	};


	/// Sets internal contextPtr to the static source.
	template <class C>
	void setContext(){
		contextPtr = & getCloner<C>().get();
	};

	/// Sets internal contextPtr to outside target.
	inline
	void setExternalContext(Context & ctx){
		contextPtr = &ctx;
	};

	/// True, if contextPtr has been set.
	inline
	bool contextIsSet() const {
		return (contextPtr != NULL);
	};

	/// If context has been set, returns it through a  cast to base class Context.

	/// Returns the linked context.
	/**
	 *  Risky: The actual object may be a derived class.
	 */
	template <class T=Context> // ,class BC> //
	T & getContext() const { // int logLevel = LOG_WARNING
		if (contextIsSet()){
			return (T &)*contextPtr;
		}
		else {
			//Logger mout(__FILE__, __FUNCTION__);
			//mout.log(logLevel) << "context not set" << mout.endl;
			return getCloner<T>().getSourceOrig();
		}
	}

	template <class T> // ,class BC>
	static
	Cloner<Context,T> & getCloner(){
		static Cloner<Context,T> cloner;
		return cloner;
	}

protected:

	// Note: common base class. The actual object may be a derived class.
	Context *contextPtr;

};

/// Adds class-specific convenience functions
template <class C> //, class BC=Context>
class SuperContextual : public Contextual{

public:

	//SuperContextual(){}

	//SuperContextual(const SuperContextual &) : baseCtx(getCloner<T>().get()) {}


	typedef drain::Cloner<Context,C> ctx_cloner_t;

	// Static?
	static inline
	ctx_cloner_t & getContextCloner(){
		return getCloner<C>();
	}

	//static
	//T & baseCtx = getContextCloner().get();


	static inline
	C & baseCtx() {
		return getContextCloner().getSourceOrig();
	}



};


} /* namespace drain */

#endif /* DRAIN_CONTEXT_H_ */

// Rack
