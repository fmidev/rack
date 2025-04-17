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

#include <drain/Log.h>
#include <drain/Type.h>
#include <ostream>
//#include <set>

#include <stdexcept>

#include <drain/util/ReferenceMap.h>

#include "Data.h"
#include "Quantity.h"


namespace rack {


class QuantityMap : public std::map<std::string, Quantity> {

public:

	typedef std::map<std::string, Quantity> map_t;

	/// Default constructor
	QuantityMap();

	/// Copy constructor
	QuantityMap(const QuantityMap & m);

	/// Constructor for ... ?
	// QuantityMap(const map_t & m);

	/// Constructor for init lists.
	QuantityMap(const std::initializer_list<std::pair<std::string, Quantity> > & inits);

	// New 2025: a set of frequently needed conf tables
	// const Quantity & TH;
	const Quantity & DBZ;
	const Quantity & VRAD;
	const Quantity & ZDR;
	const Quantity & RHOHV;
	const Quantity & KDP;
	// const Quantity & PHIDP;
	const Quantity & QIND;
	const Quantity & PROB; // Rack
	const Quantity & FUZZY; // Rack

	inline
	QuantityMap & operator=(const std::initializer_list<std::pair<std::string, Quantity> > & inits){
		assign(inits);
		return *this;
	}

	void assign(const std::initializer_list<std::pair<std::string, Quantity> > & inits);

	/// Checks if an exact match, without checking variants, is found.
	/**
	 *   \see hasQuantity()
	 */
	inline
	bool hasKey(const std::string & key) const {
		return find(key) != end();  // revised 2025
	}

	/// Checks if an exact match or a variant, is found.
	/**
	 *   \see hasKey()
	 */
	inline
	bool hasQuantity(const std::string & key) const {
		// return find(key) != end();  // revised 2025
		return retrieve(key) != end(); // revised 2025
	}

	/// Tries to find a quantity, first by exact match, or then among variants.
	/**
	 *   First, uses map_t::find(). If a match is found, iterator pointing to it is returned.
	 *   If not, the map is revisited and all the variants are tested as well.
	 *
	 *   \return - a valid iterator or map_t::end()
	 *
	 */
	iterator retrieve(const std::string & key);

	/// Tries to find a quantity, first by exact match, or then among variants.
	/**
	 *   First, uses map_t::find(). If a match is found, iterator pointing to it is returned.
	 *   If not, the map is revisited and all the variants are tested as well.
	 *
	 *   \return - a valid iterator or map_t::end()
	 *
	 */
	const_iterator retrieve(const std::string & key) const;

	const Quantity & get(const std::string & key) const;

	Quantity & get(const std::string & key);

	/// Sets default values of given quantity without assigning the quantity. Optionally overrides with user values.
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
				mout.warn("quantity neither given nor set already" );
			}
		}

		if (!typeSet){
			mout.warn("conf for " , quantity , "[" , dstData.odim.type , "] not found" );
		}
		// Redesign all this...
		dstData.data.setType(dstData.odim.type);
		//dstData.data.scaling.set(dstData.odim.scaling);
		//dstData.data.setScaling(dstData.odim.scaling.scale, dstData.odim.scaling.offset);
		dstData.data.setScaling(dstData.odim.scaling); // needed?

		//if ((dstData.odim.quantity == "QIND") || (dstData.odim.quantity == "PROB")){
		if ((q == "QIND") || (q == "PROB")){
			//dstData.data.setOptimalScale(0.0, 1.0);
			//dstData.data.setPhysicalRange(0.0, 1.0, true);
			dstData.data.getScaling().setPhysicalRange(0.0, 1.0); // note: does not change scaling
		}
		else if (q == "CLASS"){
			//dstData.data.setOptimalScale(0.0, 1.0);
			drain::image::Image & img = dstData.data;
			img.setPhysicalRange(0.0, img.getConf().getTypeMax<double>());
			//dstData.data.getScaling().setPhysicalRange(0.0, 1.0); // note: does not change scaling
		}


		if (dstData.data.getName().empty())
			dstData.data.setName(dstData.odim.quantity);

		mout.debug2("final scaling for " , dstData.odim.quantity , '[' , quantity , ']' , dstData.data.getScaling() );

		return typeSet;
	}

	/// Checks if data
	template <class M>
	inline
	bool isNormalized(const M odim) const {
		const Quantity & q = get(odim.quantity);
		if (!q.defaultType){
			drain::Logger mout("QuantityMap", __FUNCTION__);
			mout.warn("no default type for quantity:" , odim.quantity );
			return false;
		}
		return EncodingODIM::haveSimilarEncoding(odim, q.get(q.defaultType));
	}

	/// Output
	std::ostream & toStream(std::ostream & ostr) const;


};

inline
std::ostream & operator<<(std::ostream & ostr, const QuantityMap & map){
	return map.toStream(ostr);
}

QuantityMap & getQuantityMap();



}  // namespace rack


#endif

// Rack
