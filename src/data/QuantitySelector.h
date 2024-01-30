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

/// Utility for selecting a quantity label    Base class for DataSelector.
class QuantitySelector { // string temporary ?

public:

	template<typename ... TT>
	inline
	QuantitySelector(const TT &... args){
		setQuantity(args...);
	};

	virtual inline
	~QuantitySelector(){};

	template <typename ...TT>
	void setQuantity(const TT & ... args){
		clear();
		addQuantity(args...);
	}


	void setQuantityRegExp(const std::string & r);

	template <typename T, typename ...TT>
	void addQuantity(const T & arg, const TT & ... args){
		adaptQuantity(arg);
		addQuantity(args...);
	}

	inline
	void clear(){
		regExp.clear();
		quantities.clear();
	}

	bool testQuantity(const std::string & s) const;

	inline
	bool isSet() const {
		return (regExp.isSet() || !quantities.empty()) ;
	}

	/// Returns the fistt (the primary) quantity.
	//Deprecating? Consider quantityMatcher
	inline
	const std::string & getQuantity() const { // rename getQuantities
		// static const std::string empty;
		if (quantities.empty()){
			return regExp.toStr();
		}
		else {
			return quantities.front();
		}
	}

	inline
	const QuantitySelector & getQuantitySelector() const {
		return *this;
	}

	// TODO add filter, allowing ODIMPathElem::group_t == QUALITY
	static
	void getQuantityMap(const Hi5Tree & srcDataset, ODIMPathElemMap & m);

	typedef std::list<std::string> quantity_list;

	void toStream(std::ostream & ostr) const ;



protected:


	void addQuantity();


	virtual inline
	void appendQuantity(const drain::RegExp & r){
		// DETECT regexp
		// regExp.set(r);
	}


private:

	void adaptQuantity(const std::string & s);

	/// Data quantity (excluding quality data, like QIND or CLASS)
	mutable // called by updateBean()?
	drain::RegExp regExp;

	//mutable
	quantity_list quantities;


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
