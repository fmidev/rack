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

#include "drain/util/BeanLike.h"
#include "drain/util/Range.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/RegExp.h"
#include "drain/util/Sprinter.h"
#include "drain/util/Variable.h"

#include "ODIM.h"
#include "ODIMPathTools.h"
#include "ODIMPathMatcher.h"
#include "PolarODIM.h" // elangle


namespace rack {

class QuantityMatcher : protected drain::RegExp {

public:

	const std::string & value;

	inline
	QuantityMatcher(const std::string & s = "") : value(regExpString), isRegExp(false){
		set(s);
	}

	inline
	QuantityMatcher(const QuantityMatcher & matcher) : value(regExpString), isRegExp(false){
		set(matcher.value);
	}

	void set(const std::string & s);

	inline
	bool operator==(const std::string &s) const {
		return test(s);
	}

	inline
	bool operator==(const char *s) const {
		return test(s);
	}

	bool test(const std::string & s) const;

	const std::string & getType() const;

	/// Checks if the key conforms to ODIM convention: DBZH, VRAD, etc. (capital letters, underscores)
	/**
	 *  This is used for example by PaletteOp
	 *
	 *	static bool validateKey(const std::string & key);
	 *
	 */

protected:

	bool isRegExp;


};


inline
std::ostream & operator<<(std::ostream & ostr, const QuantityMatcher & m){
	ostr << m.value;
	return ostr;
}


/// Utility for selecting a quantity label    Base class for DataSelector.
class QuantitySelector : public std::list<QuantityMatcher> { // string temporary ?

public:

	//typedef std::list<std::string> quantity_list;
	typedef std::list<QuantityMatcher> quantity_list;

	/// Basic constructor
	template<typename ... TT>
	inline
	QuantitySelector(const TT &... args){
		// reset not needed
		addQuantity(args...);
	};

	/// Copy constructor. Copies the list of quantities.
	inline
	QuantitySelector(const QuantitySelector &slct) : std::list<QuantityMatcher>(slct) {
	};


	/// Destructor
	virtual inline
	~QuantitySelector(){};


	/// Define the list of accepted quantities.
	/**
	 *
	 *  \param arg  - this first arg can be also a comma-separated list of quantities - literals or regExps.
	 *  \param args - separate list of quantities - literals or regExps.
	 */
	template <typename ...TT>
	void setQuantity(const std::string & arg, const TT & ... args){
		//clear();
		setQuantities(arg); // split and adapt
		addQuantity(args...);
	}

	/// Define the list of accepted quantities as a string.
	/**
	 *  \param args - comma-separated list of quantities: literals or regexps.
	 *  \param separators - can be also ",:" and contain ':' if that is not used in the regexps.
	 */
	void setQuantities(const std::string & args); // , const std::string & separators = ","

	/// Define a syntax for quantity key. Will be checked if listed quantity keys do not match.
	/**
	 *  \param r - regular expression, like "^DBZ[HV]?C?$
	 */
	// void setQuantityRegExp(const std::string & r);
	template <typename T, typename ...TT>
	void addQuantity(const T & arg, const TT & ... args){
		adaptQuantity(arg);
		addQuantity(args...);
	}


	bool testQuantity(const std::string & s) const;

	inline
	bool isSet() const {
		return !empty();
		//return (regExp.isSet() || !quantities.empty()) ;
	}

	/// Returns the fistt (the primary) quantity.
	//Deprecating? Consider quantityMatcher
	inline
	const std::string & getQuantity() const { // rename getQuantities
		static const std::string dummy;

		if (empty()){
			return dummy;
			//return regExp.toStr();
		}
		else {
			//return quantities.front().value;
			return front().value;
		}
	}

	/*
	inline
	const QuantitySelector & getQuantitySelector() const {
		return *this;
	}
	*/

	inline
	const quantity_list & getList() const {
		return *this;
		//return quantities;
	}

	/*
	inline
	const drain::RegExp & getRegExp() const {
		return regExp;
	}
	*/

	// TODO add filter, allowing ODIMPathElem::group_t == QUALITY
	static
	void getQuantityMap(const Hi5Tree & srcDataset, ODIMPathElemMap & m);

	void toStream(std::ostream & ostr) const ;


protected:

	// inline
	// void setQuantity(){};

	inline
	void addQuantity(){};

	void adaptQuantity(const std::string & s);

	/// Data quantity (excluding quality data, like QIND or CLASS)
	//mutable // called by updateBean()?
	//drain::RegExp regExp;

	//mutable
	// quantity_list quantities;


};


inline
std::ostream & operator<<(std::ostream & ostr, const QuantitySelector & selector){
	//drain::Sprinter::toStream(ostr, selector.quantities, drain::Sprinter::cmdLineLayout);
	// ostr << ", order=" << selector.order.str;
	selector.toStream(ostr);
	return ostr;
}

} // rack::

#endif /* DATASELECTOR_H_ */
