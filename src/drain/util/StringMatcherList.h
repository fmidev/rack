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

#ifndef DRAIN_STRING_MATCHER_LIST
#define DRAIN_STRING_MATCHER_LIST

#include <set>
#include <list>
#include <map>
#include <stdexcept>

// #include <drain/Log.h>
#include <drain/Sprinter.h>
#include <drain/String.h>
#include "StringMatcher.h"


namespace drain {



/// Utility for selecting a quantity label    Applied by DataSelector.
template <class T=std::string>
class StringMatcherList : public std::list<T> { // string temporary ?

public:

	typedef T matcher_t;

	typedef std::list<T> list_t;

	/// Basic constructor
	template<typename ... TT>
	inline
	StringMatcherList(const TT &... args){
		// reset not needed
		addKey(args...);
	};

	/// Copy constructor. Copies the list of quantities.
	inline
	StringMatcherList(const StringMatcherList<T> &slct) : list_t(slct) {
	};


	/// Destructor
	virtual inline
	~StringMatcherList(){};

	StringMatcherList operator=(const StringMatcherList<T> &l){
		setKeys(l);
		return *this;
	}

	void setKeys(const StringMatcherList<matcher_t> & l){
		this->clear();
		for (const matcher_t & key: l) {
			this->push_back(key);
		}
	}

	template <class T2>
	void setKeys(const std::list<T2> & l){
		this->clear();
		for (const T2 & key: l) {
			adaptKey(key);
		}
	}

	/// Define the list of accepted quantities.
	/**
	 *
	 *  \param arg  - this first arg can be also a comma-separated list of quantities - literals or regExps.
	 *  \param args - separate list of quantities - literals or regExps.
	 */
	template <typename ...TT>
	void setKeys(const std::string & arg, const TT & ... args){
		//clear();
		insertKeys(arg); // split and adapt
		addKey(args...);
	}


	/// Append keys to existing list.
	template <typename ...TT>
	void addKey(const std::string & arg, const TT & ... args){
		adaptKey(arg);
		addKey(args...);
	}


	/// Check if \c key is accepted
	/**
	 *
	 *  \param key - string to be checked.
	 *  \param defaultResult - if selector (list) is empty, return this value.
	 */
	bool test(const std::string & key, bool defaultResult = true) const;

	inline
	bool isSet() const {
		return !this->empty();
		//return (regExp.isSet() || !quantities.empty()) ;
	}

	inline
	const list_t & getList() const {
		return *this;
	}

	inline
	void toStream(std::ostream & ostr) const {
		drain::Sprinter::sequenceToStream(ostr, *this, drain::Sprinter::cmdLineLayout);
	}

protected:

	/// Define the list of accepted quantities as a string.
	/**
	 *  \param args - comma-separated list of quantities: literals or regexps.
	 *  \param separators - can be also ",:" and contain ':' if that is not used in the regexps.
	 */
	void insertKeys(const std::string & args); // , const std::string & separators = ","

	inline
	void addKey(){};

	void adaptKey(const std::string & s);

	/// Data quantity (excluding quality data, like QIND or CLASS)
	// mutable // called by updateBean()?
	// drain::RegExp regExp;

	// mutable
	// list_t quantities;


};

template <class T>
void StringMatcherList<T>::insertKeys(const std::string & args){ //, const std::string & separators){
	// drain::Logger mout(__FILE__, __FUNCTION__);
	this->clear();
	std::vector<std::string> argv;
	//const char s =
	drain::StringTools::split(args, argv, ",:"); //separators);
	for (const std::string & arg: argv){
		// mout.attention("adapting: ", arg, " split by '", separators, "'");
		adaptKey(arg);
	}

}


/** In future, the recommended way to define desired/accepted quantities is a comma-separated list of keys.
 *
 */
template <class T>
void StringMatcherList<T>::adaptKey(const std::string & s){

	if (s.empty()){
		return;
	}
	else {

		matcher_t matcher(s);
		for (const matcher_t & m: *this){
			if (m == matcher){  // two matchers are equal if they accept and reject equally.
				// exists already, dont append...
				return;
			}
		}

		this->push_back(matcher);
		// Save copying?
		/*
		push_back(KeyMatcher());
		back().set(s);
		*/
	}

}



template <class T>
bool StringMatcherList<T>::test(const std::string & key, bool defaultResult) const {

	if (this->empty()){
		return defaultResult; // Alternative: could need empty k - but on the other hand, why add it in a list, as it accepts anything.
	}
	else {
		for (const auto & k: *this){
			if (k == key){  //
				return true;
			}
		}
	}

	return false;

}

template <class T>
inline
std::ostream & operator<<(std::ostream & ostr, const StringMatcherList<T> & selector){
	selector.toStream(ostr);
	return ostr;
}

typedef StringMatcherList<StringMatcher> KeySelector;

DRAIN_TYPENAME(KeySelector);

} // drain::

#endif // QUANTITY_MATCHER
