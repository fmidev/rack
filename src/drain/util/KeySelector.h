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

#ifndef QUANTITY_SELECTOR_H_
#define QUANTITY_SELECTOR_H_

#include <set>
#include <list>
#include <map>
#include <stdexcept>


#include <drain/util/KeyMatcher.h>


namespace drain {



/// Utility for selecting a quantity label    Applied by DataSelector.
class KeySelector : public std::list<KeyMatcher> { // string temporary ?

public:

	typedef std::list<drain::KeyMatcher> list_t;

	/// Basic constructor
	template<typename ... TT>
	inline
	KeySelector(const TT &... args){
		// reset not needed
		addKey(args...);
	};

	/// Copy constructor. Copies the list of quantities.
	inline
	KeySelector(const KeySelector &slct) : std::list<KeyMatcher>(slct) {
	};


	/// Destructor
	virtual inline
	~KeySelector(){};


	/// Define the list of accepted quantities.
	/**
	 *
	 *  \param arg  - this first arg can be also a comma-separated list of quantities - literals or regExps.
	 *  \param args - separate list of quantities - literals or regExps.
	 */
	template <typename ...TT>
	void setKey(const std::string & arg, const TT & ... args){
		//clear();
		setKeys(arg); // split and adapt
		addKey(args...);
	}

	/// Define the list of accepted quantities as a string.
	/**
	 *  \param args - comma-separated list of quantities: literals or regexps.
	 *  \param separators - can be also ",:" and contain ':' if that is not used in the regexps.
	 */
	void setKeys(const std::string & args); // , const std::string & separators = ","

	/// Define a syntax for quantity key. Will be checked if listed quantity keys do not match.
	/**
	 *  \param arg  - regular expression, like "^DBZ[HV]?C?$
	 *  \param args - regular expressions of further quantities
	 */

	//template <typename T, typename ...TT>
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
		return !empty();
		//return (regExp.isSet() || !quantities.empty()) ;
	}

	inline
	const list_t & getList() const {
		return *this;
	}

	void toStream(std::ostream & ostr) const ;


protected:

	// inline
	// void setQuantity(){};

	inline
	void addKey(){};

	void adaptKey(const std::string & s);

	/// Data quantity (excluding quality data, like QIND or CLASS)
	// mutable // called by updateBean()?
	// drain::RegExp regExp;

	// mutable
	// list_t quantities;


};


inline
std::ostream & operator<<(std::ostream & ostr, const KeySelector & selector){
	selector.toStream(ostr);
	return ostr;
}

DRAIN_TYPENAME(KeySelector);

} // drain::

#endif // QUANTITY_MATCHER
