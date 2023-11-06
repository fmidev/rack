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
#include "drain/util/Log.h"
#include "QuadTreeOp.h"

namespace drain {

namespace image {

void QuadTreeOp::getDstConf(const ImageConf & src, ImageConf & dst) const {
//void QuadTreeOp::makeCompatible(const ImageFrame & src, Image & dst) const {
	const size_t w = this->width  ? this->width  : dst.getWidth();
	const size_t h = this->height ? this->height : dst.getHeight();
	dst.setGeometry(w, h, src.getImageChannelCount(), src.getAlphaChannelCount());
}

void QuadTreeOp::process(const ImageFrame & src, ImageFrame & dst) const {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const size_t widthSrc  = src.getWidth();
	const size_t heightSrc = src.getHeight();
	//const size_t channels  = src.getChannelCount();
	const size_t widthDst  = dst.getWidth();
	const size_t heightDst = dst.getHeight();

	//CoordinateHandler2D handler1(width, height, src.getCoordinatePolicy());
	if (this->interpolation.empty()){
		mout.error() << "interpolation method unset" << mout.endl;
		return;
	}
	// const char intMethod = interpolation[0];  see Resize()?

	const Point2D<double> aspect(static_cast<double>(widthSrc)/static_cast<double>(widthDst), static_cast<double>(heightSrc)/static_cast<double>(heightDst));

	//drain::Rectangle<int> nearestCoords;  // for bilinear
	Point2D<double> p;
	Point2D<int> pLo;
	Point2D<int> pHi;
	Point2D<double> coeffLo;
	Point2D<double> coeffHi;

	/*
	int i2, j2;
	for (size_t k = 0; k < channels; ++k) {
		switch (intMethod) {
		case 'n':
			for (size_t i = 0; i < widthDst; ++i) {
				i2 = (i*widthSrc)/widthDst;
				for (size_t j = 0; j < heightDst; ++j) {
					j2 = (j*heightSrc)/heightDst;
					dst.put(i,j, src.get<double>(i2,j2));
				}
			}
			break;
		case 'b':
			for (size_t i = 0; i < widthDst; ++i) {
				p.x = aspect.x * static_cast<double>(i);
				coeffLo.x = getRounds(p.x, pLo.x, pHi.x);
				coeffHi.x = 1.0 - coeffLo.x;
				for (size_t j = 0; j < heightDst; ++j) {
					p.y = aspect.y * static_cast<double>(j);
					coeffLo.y = getRounds(p.y, pLo.y, pHi.y);
					coeffHi.y = 1.0 - coeffLo.y;
					dst.put(i, j,
							coeffLo.y*(coeffLo.x*src.get<double>(pLo.x, pLo.y) + coeffHi.x*src.get<double>(pHi.x, pLo.y)) +
							coeffHi.y*(coeffLo.x*src.get<double>(pLo.x, pHi.y) + coeffHi.x*src.get<double>(pHi.x, pHi.y)) );
					//getRoundedCoords(p, pLo, pHi);
				}
			}
			break;
		default:
			mout.error() << "unknown interpolation method: " << interpolation << mout.endl;
		}
	}
	*/
}



void QDTshrinkBy2(const Image & src, Image & dst){

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();

	const size_t width2  = width/2;
	const size_t height2 = height/2;

	dst.setType(src.getType());
	dst.setGeometry(width2, height2);

	size_t i1, j1;
	for (size_t j=0; j < height2; ++j){
		j1 = j*2;
		for (size_t i=0; i < width2; ++i){
			i1 = i*2;
			dst.put(i,j, (src.get<int>(i1,j1) + src.get<int>(i1+1,j1) + src.get<int>(i1,j1+1) + src.get<int>(i1+1,j1+1) ) / 4);
		}
	}


}

void QDTexpand(const Image & src, Image & dst){

	const size_t width1  = src.getWidth();
	const size_t height1 = src.getHeight();

	const size_t width  = dst.getWidth();
	const size_t height = dst.getHeight();

	for (size_t j=0; j < height; ++j){
		for (size_t i=0; i < width; ++i){
			dst.put(i,j, src.get<int>((i*width1)/width, (j*height1)/height) );
		}
	}


}

void QDTmatch(const Image & img1, const Image & img2, Image & u, Image & v){

	size_t width  = img1.getWidth();
	size_t height = img1.getHeight();

	if (width > 32){

	}


	long unsigned int diff, diffWin;
	int d;
	int diWin;
	int djWin;

	for (size_t j=2; j < height-3; ++j){
		for (size_t i=2; i < width-3; ++i){


			/// Iterate 3 x 3 displacements
			diffWin = 10000000;
			diWin = 0;
			djWin = 0;
			for (short dj=-1; dj <= 1; ++dj){
				for (short di=-1; di <= 1; ++di){

					// Compute fit for this displacement (di,dj)
					diff = 0;
					for (short l=-1; l <= 1; ++l){
						for (short k=-1; k <= 1; ++k){
							d = img2.get<int>(i+di+k,j+dj+l) - img1.get<int>(i+k,j+l);
							diff += d*d;
						}
					}

					if (diff < diffWin){
						diWin = di;
						djWin = dj;
						diffWin = diff;
					}

				}
			}

			u.put(i,j, diWin);
			v.put(i,j, djWin);


		}
	}

}



}  // namespace image

}  // namespace drain

// Drain
