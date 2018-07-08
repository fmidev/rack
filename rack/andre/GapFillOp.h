/**

    Copyright 2010-   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack library for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef GapFillerOP_H_
#define GapFillerOP_H_

#include <drain/util/Fuzzy.h>
#include <drain/image/Intensity.h>
//#include <drain/image/MathOpPack.h>
#include <drain/image/File.h>

#include "radar/Geometry.h"
#include "RemoverOp.h"





namespace rack {

using namespace drain::image;
using namespace hi5;

/// The base class for operators removing detected anomalies by overriding low-quality pixels with neighboring high-quality pixels."
/**

 */
class GapFillOpBase: public RemoverOp {

public:


	int width;
	float height;

protected:

	// "Removes detected anomalies by overriding low-quality pixels with neighboring high-quality pixels."
	GapFillOpBase(const std::string &name, const std::string &description) :
		RemoverOp(name, description){
		//pathFilter = "^.*/data[0-9]+/data$";
		//propertyFilter["what:quantity"] = "DBZ.*";
		dataSelector.quantity = "^DBZH$";
	};


	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {
		throw std::runtime_error(name +"::"+__FUNCTION__+ " not implemented");
	};



};


/// A simple gap filler based on distance transformation.
/**
 *
 */
class GapFillOp : public GapFillOpBase {

public:

	/** Default constructor.
	 * \param windowWidth  - width of the neighbourhood window; in metres
	 * \param windowHeight - height of the neighbourhood window; in degrees
	 */
	GapFillOp(int width = 1500, float height = 5.0) :
		GapFillOpBase(__FUNCTION__,"Removes low-quality data with gap fill based on distance transformation.") {
		parameters.reference("width",  this->width = width,   "meters");
		parameters.reference("height", this->height = height, "degrees");
	};

	//"width=1500m,height=5deg"
	/*
	GapFillOp(const std::string & p) : GapFillOpBase("GapFillDist","Gap filling based on distance transform.") {
		reference("width", width);
		reference("height", height);
		//initialize();
	};
	 */

protected:

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;
	//void filterImage(const PolarODIM & odim, Image &data, Image &quality) const;

};


/// Recursive, 'splinic' gap filler based on fast averaging.
class GapFillRecOp : public GapFillOpBase {

public:

	//"width=1500m,height=5deg,loops=3,decay=0.9"
	GapFillRecOp(int width=1500, float height=5.0, int loops=3) : //, float decay=0.9) :
		GapFillOpBase(__FUNCTION__,"Recursive, 'splinic' gap filler.") {
		parameters.reference("width", this->width = width,   "meters");
		parameters.reference("height",this->height = height, "degrees");
		parameters.reference("loops", this->loops = loops,   "N");
		//reference("decay", this->decay, decay, "0..1");
		//initialize();
	};

	int loops;
	//float decay;

protected:

	virtual
	void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;
	// void filterImage(const PolarODIM & odim, Image & data, Image & quality) const;



};



}

#endif /* GAPFILLOP_H_ */
