/*


    Copyright 2011-2013  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

   Created on: Sep 30, 2010

*/
#ifndef ODIM_QUANTITY
#define ODIM_QUANTITY

#include <ostream>
//#include <set>

#include <stdexcept>

#include <drain/util/ReferenceMap.h>

#include <drain/util/Log.h>
#include <drain/util/Type.h>
#include "ODIM.h"
#include "Data.h"

namespace rack {

/// Structure for defining
/**
 *
 */
class Quantity : public std::map<char,EncodingODIM> {

public:

	/// Default storage type
	char defaultType;

	/// True, if a value corresponding a very small (unmeasurable) value has been defined.
	bool hasUndetectValue;

	/// A physical value corresponding a very small (unmeasurable) value has been defined.
	/*
	 *  A physical value that represents all the measurement results that fall below a detection limit.
	 */
	double undetectValue;

	/// Default constructor
	Quantity(): defaultType('\0'), hasUndetectValue(false), undetectValue(0.0) {
	}

	/// Declare encoding (a storage type and scaling) for this quantity.
	/*
	 *
	 *  Set defaultType, if unset.
	 */
	EncodingODIM & set(char typecode);


	/// Retrieve the scaling for a given storage type.
	inline
	const EncodingODIM & get(char typecode = '\0') const {

		if (!typecode)
			typecode = defaultType;

		const const_iterator it = find(typecode);

		if (it != end()){ // null ok
			return it->second;
		}
		else {
			//drain::Logger mout("Quantity", __FUNCTION__);
			//mout.warn() << "undefined code for this quantity, code=" << typecode << mout.endl;
			// TODO return default
			static EncodingODIM empty;
			return empty;
		}

	}

	/// Retrieve the scaling for a given storage type.
	inline
	const EncodingODIM & get(const std::string & t) const {
		if (t.length() != 1)
			//hrow (std::runtime_error(t + "<= illegal std::string in EncodingODIM::"+__FUNCTION__+" line "+__LINE__));
			throw (std::runtime_error(t+" <= illegal std::string, "+ __FUNCTION__));
		else
			return get(t.at(0));
	}


	/// Sets neutral value for purposes of interpolation.
	inline
	void setZero(double value){
		hasUndetectValue = true;
		undetectValue = value;
	}

	/// Sets neutral value for purposes of interpolation.
	inline
	void unsetZero(){
		hasUndetectValue = false;
		undetectValue = std::numeric_limits<double>::min();
	}


	/// Sets gain=1, offset=0, undetect=type_min, nodata=type_max. Sets type, if unset.
	/*
	static
	inline
	void setTypeDefaults(EncodingODIM & dst, const std::string & type = "", const std::string & values = ""){

		if (dst.type.empty())
			dst.type = type;

		dst.gain = 1.0;
	    dst.offset = 0.0;

	    if (!type.empty()){
	    	dst.undetect = drain::Type::getMin<double>(type);
	    	dst.nodata   = drain::Type::call<drain::typeMax,double>(type);
	    }
	    else {
	    	dst.undetect = drain::Type::getMin<double>(dst.type);
	    	dst.nodata   = drain::Type::call<drain::typeMax,double>(dst.type);
	    }

		dst.setValues(values);

	}
	*/

	/// Saves type and sets the type of the actual data array as well.
	/*
	template <class D, class T>
	static
	inline
	void setTypeDefaults(PlainData<D> & dstData, const T & type, const std::string & values = ""){
		setTypeDefaults(dstData.odim, type, values);
		dstData.data.setType(type);
	}
	*/




	/// Print declared encodings (storage types and scalings)
	inline
	std::ostream & toOstr(std::ostream & ostr) const {
		for (const_iterator it = begin(); it != end(); ++it){
			if (it->first == defaultType)
				ostr << " *";
			ostr << '\t' << it->second;
			if (drain::Type::call<drain::typeIsInteger>(it->first)){
				ostr << " (min=" << it->second.getMin() << ')';
			}
			ostr << '\n';
		}
		if (hasUndetectValue)
			ostr << '\t' << "virtual zero=" << undetectValue << '\n';
		return ostr;
	}


};

inline
std::ostream & operator<<(std::ostream & ostr, const Quantity & q){
	return q.toOstr(ostr);
}



}  // namespace rack


#endif
