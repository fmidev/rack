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
#ifndef Vertical_ODIM
#define Vertical_ODIM

#include <ostream>
#include <cmath>
#include <set>
//#include <drain/util/Options.h>
#include <drain/util/ReferenceMap.h>


#include "hi5/Hi5.h"

#include "ODIM.h"

namespace rack {



/**
 *  For Cross-sections
 *
 */

class VerticalCrossSectionODIM : public ODIM {
public:

	// Notice: these are also in CartesianODIM

	/// Number of pixels in horizontal dimension
	//long xsize;
	int64_t xsize; // Recommended by ODIM. Here a compiler test...

	/// Number of pixels in vert dimension
	long ysize;

	/// Horizontal resolution in m
	double xscale;

	/// Vertical resolution in m
	double yscale;


	/// Minimum height in meters above mean sea level
	double minheight;

	/// Maximum height in meters above mean sea level
	double maxheight;

	/// Nyquist velocity
	double NI;

	inline
	VerticalCrossSectionODIM(group_t initialize = ALL) : ODIM(initialize) {
		init(initialize);
	};

	VerticalCrossSectionODIM(const ODIM & odim){
		initFromMap(odim);
	};

	inline
	void setGeometry(size_t cols, size_t rows){
		xsize = cols;
		ysize = rows;
	}

protected:

	virtual
	void init(group_t initialize=ALL);

};


class VerticalProfileODIM : public VerticalCrossSectionODIM {  // TODO VerticalODIM as a base, without xsize,ysize etc.
public:

	// where (radar site)
	double lon;
	double lat;
	double height;

	// where (profile)
	long int levels;
	double interval;
	double minRange;
	double range;  // was 'maxRange', should be 'range'

	// where
	double startaz; // non-stardard for profile
	double stopaz; // non-stardard for profile
	long int azSlots;  // non-stardard


	inline
	VerticalProfileODIM(group_t initialize = ALL) : VerticalCrossSectionODIM(initialize) {
		init(initialize);
	}

	inline
	VerticalProfileODIM(const VerticalProfileODIM & odim){
		initFromMap(odim);
	}

	template <class T>
	VerticalProfileODIM(const std::map<std::string,T> & m){
		initFromMap(m);
	}

	VerticalProfileODIM(const drain::image::Image & img, const std::string & quantity=""){
		initFromImage(img, quantity);
	}

protected:

	virtual
	void init(group_t initialize=ALL);

};


class RhiODIM : public VerticalCrossSectionODIM {
public:

	// where (radar site)
	double lon;
	double lat;
	//double height;


	// where
	double az_angle; // non-stardard for profile
	double angles; // non-stardard for profile


	//long int azSlots;  // non-stardard

	double minRange; // nonstandard
	double range;

	inline
	RhiODIM(group_t initialize = ALL) : VerticalCrossSectionODIM(initialize) {
		init(initialize);
	};

	RhiODIM(const RhiODIM & odim){
		initFromMap(odim);
	};

	template <class T>
	RhiODIM(const std::map<std::string,T> & m){
		initFromMap(m);
	}

	RhiODIM(const drain::image::Image & img, const std::string & quantity=""){
		initFromImage(img, quantity);
	};

protected:

	virtual
	void init(group_t initialize=ALL);
};



}  // namespace rack


#endif
