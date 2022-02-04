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


#include <drain/image/ImageFile.h>
#include "drain/util/Fuzzy.h"
#include "drain/imageops/ImageModifierPack.h"

#include "hi5/Hi5Write.h"
//#include "data/ODIM.h"
//#include "main/rack.h"

//#include "drain/imageops/SegmentAreaOp.h"
// #include "drain/image/"
#include "TestOp.h"



namespace rack {

//void RemoverOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
//		PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstQuality) const {

// Does not use srcData or srcQuality
// void TestOp::processData(const PlainData<PolarSrc> & srcData, const PlainData<PolarSrc> & srcQuality,
// PlainData<PolarDst> & dstData, PlainData<PolarDst> & dstProb) const {

void TestOp::processDataSet(const DataSet<PolarSrc> & srcDataSet, PlainData<PolarDst> & dstProb, DataSet<PolarDst> & cache) const {


	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.warn() << *this << mout;

	mout.debug() << "start" << mout.endl;
	// mout.debug() << *this << mout.endl;
	// mout.debug2() << "=>odimIn: " << srcData.odim << mout.endl;
	PlainData<PolarDst> & dstData = cache.getData("DBZH"); // drain::RegExp(dataSelector.quantity));

	drain::image::ImageBox boxOp;
	boxOp.iRange = this->iRange;
	boxOp.jRange = this->jRange;

	boxOp.setParameter("value", this->value); //  dstData.odim.scaleInverse(this->value));
	boxOp.process(dstData.data);

	boxOp.setParameter("value", this->prob); // dstProb.odim.scaleInverse(this->prob));
	boxOp.process(dstProb.data);

	mout.warn() << boxOp << mout;

}


}

// Rack
