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
	inline // EncodingODIM
	Quantity & add(const std::string & key){ // char typecode = 'C'
		Quantity & q = (*this)[key];
		q.set('C');
		return q;
	}

	inline // EncodingODIM
	EncodingODIM & set(const std::string & key, char typecode){ // char typecode = 'C'
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
	bool hasQuantity(const std::string & key) const {
		return find(key) != end();
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
			// Warning: if this is declared (modified), further instances will override and hence confuse

			static Quantity empty;
			return empty;
		}

	}

	/// Sets default values of given quantity - but not the quantity itself. Optionally overrides with user values.
	/**
	 *  \param quantity - the quantity according to which the encoding will be set.
	 *  \param values - str values, comma-separated
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
	 *
	 *  \tparam D - data source type (PolarSrc, CartesianDst, ...)
	 *
	 *  \param quantity - the quantity according to which the encoding will be set; if empty, current quantity will be used
	 *  \param values   - comma-separated settings to override default values
	 *
	 *  \return - true, if type could be set / derived
	 */
	template <class D>
	bool setQuantityDefaults(PlainData<D> & dstData, const std::string & quantity = "", const std::string & values = "") const {

		drain::Logger mout("QuantityMap", __FUNCTION__);

		const std::string & q = !quantity.empty() ? quantity : dstData.odim.quantity;

		const bool typeSet = setQuantityDefaults(dstData.odim, q, values);

		if (dstData.odim.quantity.empty()){
			if (!q.empty()){
				dstData.odim.quantity = q;
			}
			else {
				mout.warn() << "quantity neither given nor set already" << mout.endl;
			}
		}

		if (!typeSet){
			mout.warn() << "conf for " << quantity << "[" << dstData.odim.type << "] not found" << mout.endl;
		}
		// Redesign all this...
		dstData.data.setType(dstData.odim.type);
		dstData.data.setScaling(dstData.odim.gain, dstData.odim.offset);

		//if ((dstData.odim.quantity == "QIND") || (dstData.odim.quantity == "PROB")){
		if ((q == "QIND") || (q == "PROB")){
			//dstData.data.setOptimalScale(0.0, 1.0);
			dstData.data.getScaling().setPhysicalRange(0.0, 1.0); // note: does not change scaling
		}
		else if (q == "CLASS"){
			//dstData.data.setOptimalScale(0.0, 1.0);
			drain::image::Image & img = dstData.data;
			img.getScaling().setPhysicalRange(0.0, img.getEncoding().getTypeMax<double>());
			//dstData.data.getScaling().setPhysicalRange(0.0, 1.0); // note: does not change scaling
		}


		if (dstData.data.getName().empty())
			dstData.data.setName(dstData.odim.quantity);

		mout.debug(1) << "final scaling for " << dstData.odim.quantity << '[' << quantity << ']' << dstData.data.getScaling() << mout.endl;

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
