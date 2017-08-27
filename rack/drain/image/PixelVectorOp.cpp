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



void PixelVectorOp::makeCompatible(const Image &src, Image &dst) const  {

	drain::MonitorSource mout(iMonitor, name+"(PixelVectorOp)", __FUNCTION__);

	mout.debug(2) << "src:" << src << mout.endl;

	if (!dst.typeIsSet())
		//dst.setType(src.getType());
		dst.setType<unsigned short>();

	dst.setGeometry(src.getWidth(), src.getHeight(), 1);

	mout.debug(3) << "dst:" << dst << mout.endl;

}

void PixelVectorOp::traverse(const Image &src1, const Image &src2, Image &dst) const {

	MonitorSource mout(iMonitor, name+"(PixelVectorOp)", __FUNCTION__);

	if (src1.getGeometry() != src2.getGeometry()){
		mout.error() << "src1 and src2 geometry error" << mout.endl;
		return;
	}

	const int width  = src1.getWidth(); //std::min(src1.getWidth(),src2.getWidth());
	const int height = src1.getHeight(); //std::min(src1.getHeight(),src2.getHeight());
	const int channels = src1.getChannelCount(); // std::min(src1.getChannelCount(),src2.getChannelCount());

	const std::vector<Image> & channels1 = src1.getChannelVector();
	const std::vector<Image> & channels2 = src2.getChannelVector();

	/*
	mout.warn() << "src1 channels: " << channels1.size() << mout.endl;
	mout.warn() << "src2 channels: " << channels2.size() << mout.endl;

	mout.warn() << "max src1" << src1.getMax<double>() << mout.endl;
	mout.warn() << "max src2" << src2.getMax<double>() << mout.endl;
	mout.warn() << "max dst " << dst.getMax<double>()  << mout.endl;
	 */

	if (channels1.size() != channels2.size()){
		mout.error() << "src1 and src2 challel geometry error" << mout.endl;
		return;
	}

	//const char mapping = this->mapping.at(0);

	/*
	const bool POWER_UP   = (l != 1.0);
	const bool POWER_DOWN = (lInv != 1.0);
	const bool SCALED     = (halfWidth > 0.0);
	const bool SCALED_INV = (halfWidth < 0.0);

	const double max = static_cast<double>(Intensity::max<double>());
	const double maxOrigin = static_cast<double>(Intensity::maxOrigin<double>());
	 */

	double x=0.0, sum=0.0;

	mout.debug(2) << *this << mout.endl;

	mout.debug(2) << "src1: " << src1  << mout.endl;
	mout.debug(2) << "src2: " << src2 << mout.endl;
	mout.debug(2) << "dst:  " << dst  << mout.endl;

	FunctorBank & functorBank = getFunctorBank();

	if (!functorBank.has(functorName)){
		//functorBank.help(std::cerr);
		mout.error() << "functor not found: " << functorName << mout.endl;
		return;
	}

	UnaryFunctor & functor = functorBank.get(functorName).clone();

	//mout.warn() << functor.getName() << ':' << functor.getDescription() << mout.endl;
	functor.setScale(dst.getMax<double>(), 0.0);
	functor.setParameters(functorParams);
	mout.debug(1) << functor.getName() << ':' << functor << mout.endl;

	double coeff = (rescale>0.0) ? 1.0/rescale : 1.0/static_cast<double>(channels);
	size_t a;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {

			sum = 0.0;
			a = dst.address(i,j);
			for (int k = 0; k < channels; k++){
				x = getValue( channels1[k].get<double>(a) , channels2[k].get<double>(a) );
				sum += x;
			}
			if (SQRT)
				dst.put(a, functor(sqrt(coeff*sum)));
			else
				dst.put(a, functor(coeff*sum));
		}
	}

	// debug
	/*
		src1.debug();
		src2.debug();
		for (int k = 0; k < channels; k++){
			std::cout << "PixelVectorOp, k=" << k << '\t';
			channels1[k].debug();
			channels2[k].debug();
			x = getValue( channels1[k].at(0,0) , channels2[k].at(0,0) );
			std::cout << " oka " << x << '\n';
		}
	 */

	/*
	for (int j = 0; j < height; j++) {
		//std::cout << "Pixv " << j << '\n';
		for (int i = 0; i < width; i++) {

			sum = 0.0;
			for (int k = 0; k < channels; k++){
				x = getValue( channels1[k].get<double>(i,j) , channels2[k].get<double>(i,j) );
				if (POWER_UP)
					sum += pow(abs(x),l);
				else
					sum += abs(x);  // TODO ?
			}

			if (POWER_DOWN)
				sum = pow(sum,lInv);


			if (SCALED){
				dst.put(i,j, max*(1.0-halfWidth/(halfWidth+sum)));
			}
			else if (SCALED_INV){
				dst.put(i,j, max*(halfWidth/(halfWidth-sum)));
			}
			else {
				switch (mapping) {
				case 'l':
					dst.put(i,j, sum);
					break;
				case 'i':
					dst.put(i,j, maxOrigin - sum);
					break;
				default:
					std::string error("PixelVectorOp: unknown mapping: ");
					throw (std::runtime_error(error + mapping));
					break;
				}

			}
		}
	}
	*/

	// mout.warn() << "ok" << mout.endl;

	/*
	if (TMP_NEEDED){
		dst.setGeometry(width,height,1);
		dst.copyDeep(dstR);
		//CopyOp().filter(dstR,dst);
	};
	*/
}


}  // image::
}  // drain::



// Drain
