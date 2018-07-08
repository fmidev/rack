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
#ifndef CART_ODIM_STRUCT
#define CART_ODIM_STRUCT

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
	CartesianODIM(group_t initialize = ALL) : ODIM(initialize) {
		init(initialize);
	};

	inline
	CartesianODIM(const CartesianODIM & odim) : ODIM(ALL) {
		initFromMap(odim);
	};

	template <class T>
	CartesianODIM(const std::map<std::string,T> & m) : ODIM(ALL){
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
	CartesianODIM(const drain::image::Image & img, const std::string & quantity="") : ODIM(ALL) {
		initFromImage(img, quantity);
	};




	/// Sets number of cols (xsize) and number of rows (ysize). Recomputes xscale and yscale.
	/**
	 *  \note Rescaling xscale and yscale is based on integers.
	 *  \see Composite::updateGeoData() on how they are originally set
	 */
	void setGeometry(size_t cols, size_t rows);
	/*
	{
		xsize = cols;
		ysize = rows;
	}
	*/

	/// WHERE
	std::string projdef;
	long xsize;
	long ysize;
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
	const drain::Rectangle<double> & getBoundingBoxD(){
		bboxD.xLowerLeft  = LL_lon;
		bboxD.yLowerLeft  = LL_lat;
		bboxD.xUpperRight = UR_lon;
		bboxD.yUpperRight = UR_lat;
		return bboxD;
	}

private:

	virtual // must
	void init(group_t initialize=ALL);

	mutable drain::Rectangle<double> bboxD;

};

// Consider single-radar odim



}  // namespace rack


#endif
