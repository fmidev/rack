/**

    Copyright 2001 - 2014  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */
#ifndef ACCUMULATORGEO_H_
#define ACCUMULATORGEO_H_

//#include "Geometry.h"

#include "util/Rectangle.h"

#include "util/Proj4.h"  // for geographical projection of radar data bins

//#include "image/Image.h"
#include "Accumulator.h"
#include "GeoFrame.h"



namespace drain
{

namespace image
{



class AccumulatorGeo : public Accumulator,  public GeoFrame {

public:

	// Consider setFrameGeometry (protect this?)
	virtual
	inline
	void setGeometry(unsigned int width, unsigned int height) {
		GeoFrame::setGeometry(width, height);
		// See allocate below, calling Accumulator::setGeometry(width, height);
	}


	/// This should be called after setGeometry, unless the projection is used as a frame.
	virtual
	inline
	void allocate(){
		AccumulationArray::setGeometry(frameWidth,frameHeight);
	};

	inline
	void addUnprojected(double lon, double lat, double value, double weight){
		int i,j;
		size_t a;
		deg2pix(lon,lat, i,j);
		if ( (i>=0) && (i<static_cast<int>(width)) && (j>=0) && (j< static_cast<int>(height)) ){
			a = data.address(i, j);
			// std::cerr << "addUnprojected:" << i << ',' << j << '\t' << value << '/' << weight << '\n';
			add(a, value, weight);
		}
		/*
		else {
			std::cerr << "addUnprojected: outside, " << lat << ',' << lon << '\t' << value << '/' << weight << '\n';
		}
		*/
	};


};


std::ostream & operator<<(std::ostream &ostr, const AccumulatorGeo &cumulatorGeo);

} // ::image
} // ::drain

#endif // AccumulatorGeo
