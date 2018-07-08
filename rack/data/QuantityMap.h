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
#ifndef ODIM_QUANTITY_MAP
#define ODIM_QUANTITY_MAP

#include <ostream>
//#include <set>

#include <stdexcept>

#include <drain/util/ReferenceMap.h>

#include <drain/util/Log.h>
#include <drain/util/Type.h>
//#include "ODIM.h"
#include "Data.h"
#include "Quantity.h"


namespace rack {


class QuantityMap : public std::map<std::string, Quantity> {

public:


	QuantityMap(){
		initialize();
	}

	void initialize();

	/// Sets quantity encoding. If gain=0.0, default values for the given type will be set as defined in drain::Type
	//void set(const std::string & key, char type, double gain, double offset, double undetect, double nodata);
	//Quantity & set(const std::string & key);

	/// Sets quantity encoding for a non-negative quantity.
	/*
	 *  If offset not given, for unsigned storage types the lowest mapped value
	 *  will be 0.0, encoded with byte value 1. Byte value 0 set for \c undetect.
	 *
	 */
	//void set(const std::string & key, char type='C', double gain=1.0, double offset=NAN);
	inline
	EncodingODIM & set(const std::string & key, char typecode='C'){
		Quantity & q = (*this)[key];
		return q.set(typecode);
	}

	/*
	inline
	EncodingODIM & set(const std::string & key, const EncodingODIM & src){
		Quantity & q = (*this)[key];
		char typecode = src.type.empty() ? 'C' : src.type.at(0);
		return q.set(typecode);
	}
	*/
	//void set(const std::string & key, const Quantity & q);

	inline
	Quantity & copy(const std::string & key, const Quantity & dst){
		Quantity & q = (*this)[key];
		//char typecode = src.type.empty() ? 'C' : src.type.at(0);
		q = dst;
		return q;
	}




	inline
	const Quantity & get(const std::string & key) const {

			const const_iterator it = find(key);

			if (it != end()){ // null ok
				return it->second;
			}
			else {
				//drain::Logger mout("Quantity", __FUNCTION__);
				//mout.warn() << "undefined quantity=" << key << mout.endl;
				static Quantity empty;
				return empty;
			}

	}

	inline
	Quantity & get(const std::string & key) {

		const iterator it = find(key);

		if (it != end()){ // null ok
			return it->second;
		}
		else {
			static
			Quantity empty;
			return empty;
		}

	}

	/// Sets quantity with default values, optionally overridden with by user values.
	/**
	 *  \param quantity - the quantity according to which the encoding will be set.
	 *  \param values - other values, comma-separated
	 *
	 *   \return - true, if type could be set / derived
	 */
	bool setQuantityDefaults(EncodingODIM & dst, const std::string & quantity, const std::string & values = "") const;

	inline
	bool setQuantityDefaults(ODIM & dst) const {
		return setQuantityDefaults(dst, dst.quantity);
	}

	///
	/**
	 *  \param quantity - the quantity according to which the encoding will be set.
	 *  \param values - other values, comma-separated

	 *  \return - true, if type could be set / derived
	 */
	template <class D>
	bool setQuantityDefaults(PlainData<D> & dstData, const std::string & quantity = "", const std::string & values = "") const {

		drain::Logger mout("QuantityMap", __FUNCTION__);

		const std::string & q = !quantity.empty() ? quantity : dstData.odim.quantity;

		const bool typeSet = setQuantityDefaults(dstData.odim, q, values);

		if (dstData.odim.quantity.empty()){
			dstData.odim.quantity = q;
		}

		if (!typeSet){
			mout.warn() << "conf for " << quantity << "[" << dstData.odim.type << "] not found" << mout.endl;
		}
		// Redesign all this...
		dstData.data.setType(dstData.odim.type);
		dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);

		if ((dstData.odim.quantity == "QIND") || (dstData.odim.quantity == "PROB")){
			//dstData.data.setOptimalScale(0.0, 1.0);
			dstData.data.getScaling().setPhysicalRange(0.0, 1.0); // note: does not change scaling
		}

		return typeSet;
	}

	/// Checks if data
	template <class M>
	inline
	bool isNormalized(const M odim) const {
		const Quantity & q = get(odim.quantity);
		if (!q.defaultType){
			drain::Logger mout("QuantityMap", __FUNCTION__);
			mout.warn() << "no default type for quantity:" << odim.quantity << mout.endl;
			return false;
		}
		return EncodingODIM::haveSimilarEncoding(odim, q.get(q.defaultType));
	}

	/*
	inline
	bool hasQuantity(const std::string & quantity) const {
		return (find(quantity) == end());
	}
	*/


	inline
	std::ostream & toOstr(std::ostream & ostr) const {
		for (const_iterator it = begin(); it != end(); ++it){
			ostr << it->first << '\n' << it->second; // << '\n';
		}
		return ostr;
	}



};

inline
std::ostream & operator<<(std::ostream & ostr, const QuantityMap & map){
	return map.toOstr(ostr);
}

QuantityMap & getQuantityMap();



}  // namespace rack


#endif
