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

#include "PixelVectorOp.h"

namespace drain
{

namespace image
{

void PixelVectorOp::getDstConf(const ImageConf &src, ImageConf & dst) const {
//void PixelVectorOp::makeCompatible(const ImageFrame &src, Image &dst) const  {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.debug3() << "src:" << src << mout.endl;

	// if (!dst.typeIsSet())		dst.setType<unsigned short>();
	dst.setArea(src.getGeometry());
	dst.setChannelCount(1);
	//dst.setGeometry(src.getWidth(), src.getHeight(), 1);

	mout.note() << "dst:" << dst << mout.endl;

}


void GrayOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	const size_t channels = src.size();

	std::vector<double> coeff(channels);

	drain::Referencer ref;
	ref.link(coeff);
	ref.fillArray = true;
	ref = coefficients; // magic

	double sum=0.0;
	for (double c: coeff){
		sum += c;
	}

	if (sum==0.0){
		mout.error() << "zero-sum coeffs: " << ref << " (from " << coefficients << ")" << mout;
		return;
	}

	for (double &c: coeff){
		c /= sum;
	}

	mout.special() << "coeffs: " << ref << " (from " << coefficients << ")" << mout;


	Channel & dstChannel = dst.get();

	const size_t width  = dstChannel.getWidth();
	const size_t height = dstChannel.getHeight();

	size_t a;
	for (size_t j = 0; j < height; j++) {
		for (size_t i = 0; i < width; i++) {

			sum = 0.0;
			a = dstChannel.address(i,j);
			/*
			for (const Channel & srcChannel : src){
				sum += srcChannel.get<double>(a);
			}
			*/
			for (size_t k = 0; k < channels; k++){
				sum += coeff[k] * src.get(k).get<double>(a);
			}
			dstChannel.put(a, sum); // TODO: scale, limit?
		}
	}

	//drain::StringTools::split(coefficients, coeff, ",");
	//const size_t channels = src.size();
	/*
	if (coeff.size() < channels){

	}
	*/
}


}  // image::
}  // drain::



// Drain
