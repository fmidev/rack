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

// Rack
