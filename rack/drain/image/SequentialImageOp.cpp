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

#include "SequentialImageOp.h"

namespace drain
{

namespace image
{



const int SequentialImageOp::MAX_16B(0xffff);


void SequentialImageOp::traverse(const Image &src,const Image &src2, Image &dst) const {

	drain::MonitorSource mout(iMonitor, name+"(SequentialImageOp)", __FUNCTION__);

	//const bool floatType = (src.isFloatType() || src2.isFloatType() || dst.isFloatType());
	/*
	if (dst.isEmpty()){
		makeCompatible(src,dst);
	}
	 */

	if ( (src.getGeometry() == src2.getGeometry()) && (src.getGeometry()==dst.getGeometry()) ){
		traverseSequentially(src, src2, dst);
	}
	else {
		traverseSpatially(src, src2, dst);
	}
}

void SequentialImageOp::traverseSequentially(const Image &src,const Image &src2, Image &dst) const {

	drain::MonitorSource mout(iMonitor, name+"(SequentialImageOp)", __FUNCTION__);

	// makeCompatible(src,dst); // unneeded, done by filter()
	initializeParameters(src,src2,dst);

	Image::const_iterator s  = src.begin(); // FIXME  separate const_iterator and iterator
	Image::const_iterator s2 = src2.begin();
	Image::iterator d=dst.begin();
	Image::iterator dEnd=dst.end();

	if (src.isFloatType() || src2.isFloatType() || dst.isFloatType()){
		mout.debug(1) << "Float" << mout.endl;
		while ( d != dEnd){
			//*d = dst.limit<double>(filterValueD(*s,*s2));
			*d = filterValueD(*s,*s2);
			++d;
			++s;
			++s2;
		}
	}
	else {
		mout.debug(2) << "Int[" << dst.getMin<int>() << ',' << dst.getMax<int>() << ']' << mout.endl;
		while ( d != dEnd){
			*d = dst.limit<int>(filterValueI(*s,*s2));
			++d;
			++s;
			++s2;
		}
	}

}

void SequentialImageOp::traverseSpatially(const Image &src,const Image &src2, Image &dst) const {

	drain::MonitorSource mout(iMonitor, name+"(SequentialImageOp)", __FUNCTION__);

	initializeParameters(src, src2, dst);

	const size_t width1    = src.getWidth();
	const size_t height1   = src.getHeight();
	const size_t channels1 = src.getChannelCount();
	CoordinateHandler2D handler1(width1, height1, src.getCoordinatePolicy());

	const size_t width2    = src2.getWidth();
	const size_t height2   = src2.getHeight();
	const size_t channels2 = src2.getChannelCount();
	CoordinateHandler2D handler2(width2, height2, src2.getCoordinatePolicy());


	const size_t width    = std::max(width1, width2);
	const size_t height   = std::max(height2, height2);
	const size_t channels = std::max(channels1, channels2);


	Point2D<int> p1;
	Point2D<int> p2;
	if (src.isFloatType() || src2.isFloatType() || dst.isFloatType()){
		mout.debug() << "Float" << mout.endl;
		for (size_t k = 0; k < channels; ++k) {
			size_t k1 = k % channels1;
			size_t k2 = k % channels2;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					p1.setLocation(i,j);
					handler1.handle(p1);
					p2.setLocation(i,j);
					handler2.handle(p2);
					//dst.put(i,j, dst.limit<double>(filterValueD(src.get<float>(p1.x, p1.y, k1), src2.get<float>(p2.x, p2.y, k2))));
					dst.put(i,j, filterValueD(src.get<double>(p1.x, p1.y, k1), src2.get<double>(p2.x, p2.y, k2)));
				}
			}
		}
	}
	else {
		mout.debug() << "Int" << mout.endl;
		for (size_t k = 0; k < channels; ++k) {
			size_t k1 = k % channels1;
			size_t k2 = k % channels2;
			for (size_t i = 0; i < width; ++i) {
				for (size_t j = 0; j < height; ++j) {
					p1.setLocation(i,j);
					handler1.handle(p1);
					p2.setLocation(i,j);
					handler2.handle(p2);
					dst.put(i,j, dst.limit<int>(filterValueI(src.get<int>(p1.x, p1.y, k1), src2.get<int>(p2.x, p2.y, k2))));
				}
			}
		}
	}

}


}

}


// Drain
