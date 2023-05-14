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
#ifndef CART_ODIM_STRUCT
#define CART_ODIM_STRUCT

#include "drain/image/GeoFrame.h"

#include "ODIM.h"

namespace rack {


/// Metadata structure for Cartesian radar data products (single-radar or composites).
/**
 *
 */
// ? Proj4 proj could be part of this? No...
class CartesianODIM : public ODIM {

public:

	inline
	CartesianODIM(group_t initialize =ODIMPathElem::ALL_LEVELS) : ODIM(initialize) {
		init(initialize);
	};

	inline
	CartesianODIM(const CartesianODIM & odim) : ODIM(ODIMPathElem::ALL_LEVELS) {
		initFromMap(odim);
	};

	template <class T>
	CartesianODIM(const std::map<std::string,T> & m) : ODIM(ODIMPathElem::ALL_LEVELS){
		initFromMap(m);
	}

	/*
	inline
	CartesianODIM(const CartesianODIM & odim, const drain::image::Image & img){
		initFromMap(odim);
		initFromImage(img);
	};
	*/


	inline
	CartesianODIM(const drain::image::Image & img, const std::string & quantity="") : ODIM(ODIMPathElem::ALL_LEVELS) {
		initFromImage(img, quantity);
	};




	/// Sets number of cols (geometry.width == xsize) and number of rows (geometry.height == ysize). Recomputes xscale and yscale.
	/**
	 *  \note Rescaling xscale and yscale is based on integers.
	 *  \see Composite::updateGeoData() on how they are originally set
	 */
	void setGeometry(size_t cols, size_t rows);

	/// Updates size, projection and bounding box.
	void updateGeoInfo(const drain::image::GeoFrame & geoFrame);

	/// WHERE
	std::string projdef;
	double xscale;
	double yscale;

	//drain::Rectangle<double> bboxD ?
	double LL_lat;
	double LL_lon;
	double UL_lat;
	double UL_lon;
	double LR_lat;
	double LR_lon;
	double UR_lat;
	double UR_lon;

	/// How cartesian data are processed, according to Table 12
	std::string camethod;

	/// Radar nodes (Table 9) which have contributed data to the composite, e.g. “’searl’, ’noosl’, ’sease’, ’fikor”’
	std::string nodes;

	/// This is needed for palette operations (of single-radar Cartesian products).
	//double NI;


	inline
	const drain::Rectangle<double> & getBoundingBoxD() const {
		bboxD.lowerLeft.x  = LL_lon;
		bboxD.lowerLeft.y  = LL_lat;
		bboxD.upperRight.x = UR_lon;
		bboxD.upperRight.y = UR_lat;
		return bboxD;
	}

private:

	virtual // must
	void init(group_t initialize = ODIMPathElem::ALL_LEVELS);

	mutable drain::Rectangle<double> bboxD;

};

// Consider single-radar odim



}  // namespace rack


#endif

// Rack
