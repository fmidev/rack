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
#ifndef ODIM_QUANTITY
#define ODIM_QUANTITY

#include <ostream>
#include <stdexcept>

#include <drain/Log.h>
#include <drain/Type.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/StringMatcherList.h>
#include <drain/util/Range.h>

#include "EncodingODIM.h"

namespace rack {

/// Structure for defining quantity
/**
 *
 */
class Quantity : public std::map<char,EncodingODIM> {

public:

	/// Container for default encodings
	typedef std::map<char,EncodingODIM> map_t;

	/// List type applicable in constructors
	typedef std::list<EncodingODIM> list_t;

	std::string name;

	/// Collection of quantities that can be similarly scaled and encoded
	drain::StringMatcherList<drain::StringMatcher> keySelector; // could be keyMatcher?

	/// Default storage type
	char defaultType = '\0';

	drain::Range<double> physicalRange;

	/// A physical value corresponding a very small (unmeasurable) value has been defined.
	/*
	 *  A physical value that represents all the measurement results that fall below a detection limit.
	 */
	double undetectValue = std::numeric_limits<double>::signaling_NaN();


	/// Default constructor
	Quantity(const std::string & name = "",
			const drain::Range<double> & range = {},
			char defaultType='\0',
			const list_t & l = {}, // brace initializer
			double undetectValue = std::numeric_limits<double>::signaling_NaN());

	/// Constructor accepting variants (related/similar quantities)
	Quantity(const std::string & name,
			const std::list<std::string> & compatibleVariants,
			const drain::Range<double> & range, //  = {},
			char defaultType='\0',
			const list_t & l = {}, // brace initializer
			double undetectValue = std::numeric_limits<double>::signaling_NaN());

	/// Constructor without range (counters can be such)
	/**
	 *
	 */
	Quantity(const std::string & name,
			const std::list<std::string> & compatibleVariants,
			char defaultType,
			const list_t & l = {}, // brace initializer
			double undetectValue = std::numeric_limits<double>::signaling_NaN());


	/*
	inline
	Quantity(): defaultType('\0'), undetectValue(std::numeric_limits<double>::signaling_NaN()) {
	}
	*/

	/// Copy constructor
	inline
	Quantity(const Quantity & quantity):
		map_t(quantity),
		name(quantity.name),
		keySelector(quantity.keySelector),
		defaultType(quantity.defaultType),
		physicalRange(quantity.physicalRange),
		undetectValue(quantity.undetectValue) {
	}

	/*
	inline
	Quantity & operator=(const Quantity & quantity){
		map_t::operator=(quantity);
		variants.setKeys(quantity);
		name = quantity.name;
		defaultType = quantity.defaultType;
		physicalRange = quantity.physicalRange;
		undetectValue = quantity.undetectValue;
		return *this;
	}
	*/

	/// Declare encoding (a storage type and scaling) for this quantity.
	/*
	 *
	 *  Set defaultType, if unset.
	 */
	EncodingODIM & set(char typecode);

	void addEncodings(const list_t & l);


	/// Retrieve the scaling for a given storage type.
	const EncodingODIM & get(char typecode = '\0') const;

	/// Retrieve the scaling for a given storage type.
	const EncodingODIM & get(const std::string & t) const;

	inline
	bool isApplicable(const std::string & key){ // needed?
		return keySelector.test(key, false);
	}

	/// True, if a value corresponding a very small (unmeasurable) value has been defined.
	inline
	bool hasUndetectValue() const {
		return !std::isnan(undetectValue);
	}

	/// Set a value to be used like a real measurement, for example in interpolation.
	/**
	 *
	 *
	 *   \param value - substitute for \c undetect marker value, or signaling_NaN
	 */
	inline
	void setZero(double value){
		undetectValue = value;
	}

	/// Set a value to be used like a real measurement, for example in interpolation.
	/**
	 *   \param value - substitute for \c undetect marker value.
	 */
	void setZero(const std::string & value);

	/// Confirm that no value should be used as a substitute of undetected value.
	inline
	void unsetZero(){
		undetectValue = std::numeric_limits<double>::signaling_NaN();
	}

	/// Sets absolute or typical range of this quantity.
	/**
	 *   For example, for probability the range would be [0.0, 1.0].
	 *   Setting range is useful when data are converted from some storage type to another
	 */
	inline
	void setPhysicalRange(double min, double max = std::numeric_limits<double>::max() ){
		// hasUndetectValue  = true;
		// undetectValue     = min;
		setZero(min);
		physicalRange.set(min, max);
	}

	/// Print declared encodings (storage types and scalings)
	std::ostream & toStream(std::ostream & ostr) const;


};

inline
std::ostream & operator<<(std::ostream & ostr, const Quantity & q){
	return q.toStream(ostr);
}



}  // namespace rack


#endif

// Rack
