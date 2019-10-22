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

#include <sstream>
//#include <strstream>
#include <list>
#include "PaletteOp.h"

namespace drain
{

namespace image
{

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */


/// Creates a gray palette ie. "identity mapping" from gray (x) to rgb (x,x,x).
// TODO T 256, T2 32768
/*
void PaletteOp::setGrayPalette(unsigned int iChannels,unsigned int aChannels,float brightness,float contrast) const {

	const unsigned int colors = 256;
	const unsigned int channels = iChannels+aChannels;

	paletteImage.setGeometry(colors,1,iChannels,aChannels);

	int g;
	//const float origin = (Intensity::min<int>()+Intensity::max<int>())/2.0;

	for (unsigned int i = 0; i < colors; ++i) {

		g = paletteImage.limit<int>( contrast*(static_cast<float>(i)) + brightness);
		for (unsigned int k = 0; k < iChannels; ++k)
			paletteImage.put(i,0,k, g);

		for (unsigned int k = iChannels; k < channels; ++k)
			paletteImage.put(i,0,k,255);  // TODO 16 bit?
	}
}
 */

void PaletteOp::setPalette(const Palette & palette) {
	this->palette = palette;
}


void PaletteOp::setSpecialCode(const std::string code, double f) {

	Logger mout(getImgLog(), getName(), __FUNCTION__);

	//if (this->palette == NULL)
	//	throw std::runtime_error("PaletteOp::setSpecialCode: palette not set (null)");

	std::map<std::string,PaletteEntry >::const_iterator it = palette.specialCodes.find(code);
	if (it != palette.specialCodes.end())
		specialCodes[f] = it->second;
	else {
		mout.note() << palette << mout.endl;
		mout.warn() << "could not find entry: "<< code << '(' << f << ')' << mout.endl;
	}
	//std::cerr << code <<  ": setSpecialCode: could not find entry\n";

}


/*
void PaletteOp::setPalette(const Image &palette) const {
	//const Geometry & gPal = palette.getGeometry();

	const unsigned int width  = palette.getWidth();
	const unsigned int height = palette.getHeight();
	const unsigned int channels = palette.getChannelCount();

	const unsigned int colors = 256;
	paletteImage.setGeometry(colors,1,palette.getImageChannelCount(),palette.getAlphaChannelCount());
	for (unsigned int i = 0; i < colors; ++i) {
		for (unsigned int k = 0; k < channels; ++k) {
			paletteImage.put(i,0,k, palette.get<int>((i*width)/colors,(i*height)/colors,k));
		}
	}
}
 */


void PaletteOp::makeCompatible(const ImageFrame &src,Image &dst) const {
	//const Geometry &gSrc = src.getGeometry();
	//const Geometry &gPal = paletteImage.getGeometry();
	//dst.setGeometry(gSrc.getWidth(),gSrc.getHeight(),gPal.getImageChannelCount(),gPal.getAlphaChannelCount());
	//const unsigned int alphaChannels = max(paletteImage.getAlphaChannelCount(),src.getAlphaChannelCount());
	const unsigned int alphaChannels = palette.hasAlpha() || (src.getAlphaChannelCount()>0) ? 1 : 0;
	dst.setGeometry(src.getWidth(), src.getHeight(), 3, alphaChannels);
}


void PaletteOp::help(std::ostream & ostr) const {

	ImageOp::help(ostr);

	for (std::map<double,PaletteEntry >::const_iterator cit = specialCodes.begin(); cit != specialCodes.end(); ++cit){
		ostr << cit->first << '=' << cit->second << '\n';
	}
	for (std::map<std::string,PaletteEntry >::const_iterator pit = palette.specialCodes.begin(); pit != palette.specialCodes.end(); ++pit){
		ostr << "'" << pit->first << "'=" << pit->second << '\n';
	}
}


void PaletteOp::process(const ImageFrame &src,Image &dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	initialize();


	makeCompatible(src,dst);

	const unsigned int width  = dst.getWidth();
	const unsigned int height = dst.getHeight();
	const unsigned int channels = palette.hasAlpha() ? 4 : 3;

	const bool hasSpecialCodes = !specialCodes.empty();  // for PolarODIM nodata & undetected



	double d;
	Palette::const_iterator it;      // lower bound
	Palette::const_iterator itLast;  // upped bound
	//std::map<double,std::vector<double> >::iterator
	unsigned int k;
	//int code;
	std::map<double,PaletteEntry >::const_iterator cit;
	for (unsigned int i = 0; i < width; ++i) {
		//std::cerr << "Palette: " << i << '\t' << '\n';
		for (unsigned int j = 0; j < height; ++j) {

			if (hasSpecialCodes){  // PolarODIM
				//code = src.get<double>(i,j);
				cit = specialCodes.find(src.get<double>(i,j));
				if (cit != specialCodes.end()){
					for (k = 0; k < channels; ++k)
						dst.put(i,j,k, cit->second.color[k]);
					continue;
				}
			}

			d = scale * src.get<double>(i,j) + offset;

			// TODO: stl::lower_bound
			itLast = palette.begin();
			for (it = palette.begin(); it != palette.end(); ++it){
				if (it->first > d)
					break;
				itLast = it;
			}

			for (k = 0; k < channels; ++k)
				dst.put(i,j,k, itLast->second.color[k]);

			/*
			if (i==j){
				//std::cout <<	"palettex " << src.get<double>(i,j) << "\t =>" << d << "\t => " << it->first << '=' << it->second << '\n';
				dst.put(i,j,0, i*123);
				dst.put(i,j,2, i*53);
			}


			if ((i<40)&&(j<40)){
				dst.put(i,j,0, 255);
				dst.put(i,j,1, i*6);
				dst.put(i,j,2, (40-j)*6);
			}
			dst.put(i,j,0, i+j);
			dst.put(i,j,1, 253*i+257*j);
			*/
		}
	}

	//File::write(dst,"palette-color.png");

	//for (unsigned int k = 0; k < channels; ++k) {
	//	dst.put(i,j,k, paletteImage.get<int>(src.get<int>(i,j)&255,0,k));  // % was not ok! for char?
	//}

	if (!palette.hasAlpha()){
		if ((src.getAlphaChannelCount()>0) && (dst.getAlphaChannelCount()>0)){
			mout.info() << "Copying original alpha channel" << mout.endl;
			CopyOp().traverseChannel(src.getAlphaChannel(), dst.getAlphaChannel());
		}
	}


	//return dst;
}



}

}

// Drain
