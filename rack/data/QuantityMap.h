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
#ifndef ODIM_QUANTITY_MAP
#define ODIM_QUANTITY_MAP

#include <ostream>
//#include <set>

#include <stdexcept>

#include <drain/util/ReferenceMap.h>

#include <drain/util/Debug.h>
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
				//drain::MonitorSource mout("Quantity", __FUNCTION__);
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
	 *
	 */
	bool setQuantityDefaults(EncodingODIM & dst, const std::string & quantity, const std::string & values = "") const;

	///
	/**
	 *  \param quantity - the quantity according to which the encoding will be set.
	 *  \param values - other values, comma-separated
	 */
	template <class D>
	inline
	bool setQuantityDefaults(PlainData<D> & dstData, const std::string & quantity = "", const std::string & values = "") const {

		const std::string & q = !quantity.empty() ? quantity : dstData.odim.quantity;

		bool result = setQuantityDefaults(dstData.odim, q, values);

		if (dstData.odim.quantity.empty()){
			dstData.odim.quantity = q;
		}

		if (result){
			dstData.data.setType(dstData.odim.type);
		}

		return result;
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

// Rack
