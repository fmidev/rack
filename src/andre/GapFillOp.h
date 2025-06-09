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

#ifndef GapFillerOP_H_
#define GapFillerOP_H_

#include <drain/image/ImageFile.h>
#include <drain/util/Fuzzy.h>
#include <drain/image/Intensity.h>
//#include <drain/image/MathOpPack.h>
#include "radar/Geometry.h"
#include "RemoverOp.h"





namespace rack {

//using namespace drain::image;
//using namespace hi5;

/// The base class for operators removing detected anomalies by overriding low-quality pixels with neighboring high-quality pixels."
/**

 */
class GapFillOpBase: public RemoverOp {

public:

	// Todo: windowConf ? with double?
	int widthM = 0;
	float heightD = 0;

	// Originally for GapFillOpRec, but for combined op...
	int  loops = 0;
	double expansionCoeff = 1.0;

protected:

	// "Removes detected anomalies by overriding low-quality pixels with neighboring high-quality pixels."

	GapFillOpBase(const std::string &name, const std::string &description) :
		RemoverOp(name, description){
		dataSelector.setQuantities("^DBZH$");
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
		parameters.link("width",  this->widthM = width,   "meters");
		parameters.link("height", this->heightD = height, "degrees");
		//parameters.link("loops", this->loops = 0, "N");
		//parameters.link("expansionCoeff", this->expansionCoeff = 1, "window extension(1..2)");
		parameters.link("qualityThreshold", this->qualityThreshold = 0.1, "0.0...1.0");
	};


protected:

	// virtual void processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	virtual
	void processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
						PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQIND) const;

	double qualityThreshold;

};


/// Recursive, 'splinic' gap filler based on fast averaging.
class GapFillRecOp : public GapFillOpBase {

public:

	//"width=1500m,height=5deg,loops=3,decay=0.9"
	GapFillRecOp(int width=1500, float height=5.0, int loops=3) : //, float decay=0.9) :
		GapFillOpBase(__FUNCTION__, "Recursive, 'splinic' gap filler.") {
		parameters.link("width", this->widthM = width,   "meters");
		parameters.link("height",this->heightD = height, "degrees");
		parameters.link("loops", this->loops = loops,   "N");
		//link("decay", this->decay, decay, "0..1");
		//initialize();
	};

	//int loops;
	//float decay;

protected:

	//virtualvoid processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const;

	virtual
	void processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
					PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQIND) const;

};



} // rack::

#endif /* GAPFILLOP_H_ */

