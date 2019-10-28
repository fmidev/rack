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
	this->palettePtr = &palette;
}


void PaletteOp::registerSpecialCode(const std::string code, double f) {

	Logger mout(getImgLog(), getName(), __FUNCTION__);

	if ((this->palettePtr == NULL) || (this->palettePtr->empty()))
		//throw std::runtime_error("PaletteOp::setSpecialCode: palette not set (null)");
		mout.error() << "no paletted loaded or linked " << mout.endl;

	std::map<std::string,PaletteEntry >::const_iterator it = palettePtr->specialCodes.find(code);
	if (it != palettePtr->specialCodes.end())
		specialCodes[f] = it->second;
	else {
		mout.debug() << *palettePtr << mout.endl;
		//mout.note() << palettePtr->specialCodes << mout.endl;
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


void PaletteOp::makeCompatible(const ImageFrame &src, Image &dst) const {

	Logger mout(getName(), __FUNCTION__);

	if (palettePtr->empty()){
		mout.warn() << " no palette loaded " << mout.endl;
		dst.setGeometry(src.getGeometry());
		return;
	}

	const ChannelGeometry & colours = palettePtr->getChannels();

	mout.debug() << colours.getImageChannelCount() << ':' << colours.getAlphaChannelCount() << mout.endl;

	dst.setGeometry(src.getWidth(), src.getHeight(), colours.getImageChannelCount(), colours.getAlphaChannelCount());

	mout.debug() << "dst: " << dst << mout.endl;

}


void PaletteOp::help(std::ostream & ostr) const {

	ImageOp::help(ostr);

	for (std::map<double,PaletteEntry >::const_iterator cit = specialCodes.begin(); cit != specialCodes.end(); ++cit){
		ostr << cit->first << '=' << cit->second << '\n';
	}
	for (std::map<std::string,PaletteEntry >::const_iterator pit = palettePtr->specialCodes.begin(); pit != palettePtr->specialCodes.end(); ++pit){
		ostr << "'" << pit->first << "'=" << pit->second << '\n';
	}
}

void PaletteOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);

	// mout.debug() << "Starting" << mout.endl;

	mout.debug(1) << src << mout.endl;
	mout.debug(1) << dst << mout.endl;

	const Channel & srcChannel = src.get(0);

	const size_t width  = dst.getGeometry().getWidth();
	const size_t height = dst.getGeometry().getHeight();
	const ChannelGeometry paletteChannels = palettePtr->getChannels();

	mout.debug() << paletteChannels << mout.endl;

	// Note: colouring involves also alpha, if found, so channelCount includes alpha channel(s)

	size_t channelCount = dst.getGeometry().getChannelCount();

	if (channelCount > paletteChannels.getChannelCount()){
		mout.note() << "dst has " << channelCount << " colours (channels), using that of palette: " << paletteChannels.getChannelCount() << mout.endl;
		channelCount = paletteChannels.getChannelCount();
	}
	else if (channelCount < paletteChannels.getChannelCount()){
		mout.note() << "palette has " << paletteChannels.getImageChannelCount() << " colours (channels), using only that of dst: " << channelCount << mout.endl;
	}


	const bool hasSpecialCodes = !specialCodes.empty();  // for PolarODIM nodata & undetected

	double d;
	Palette::const_iterator it;      // lower bound
	Palette::const_iterator itLast;  // upped bound

	size_t k;
	Palette::const_iterator cit;
	//std::map<double,PaletteEntry >::const_iterator cit;
	for (size_t  i = 0; i < width; ++i) {

		for (size_t j = 0; j < height; ++j) {

			d = srcChannel.get<double>(i,j);
			if (hasSpecialCodes){  // PolarODIM

				cit = specialCodes.find(d);
				if (cit != specialCodes.end()){
					for (k = 0; k < channelCount; ++k)
						dst.get(k).put(i,j, cit->second.color[k]);
					continue;
				}
			}

			d = scale * d + offset;

			itLast = palettePtr->begin();
			for (it = palettePtr->begin(); it != palettePtr->end(); ++it){
				if (it->first > d)
					break;
				itLast = it;
			}

			for (k = 0; k < channelCount; ++k)
				dst.get(k).put(i,j, itLast->second.color[k]);

		}
	}


	if (paletteChannels.getAlphaChannelCount() == 0){
		if ((src.getGeometry().getAlphaChannelCount()>0) && (dst.getGeometry().getAlphaChannelCount()>0)){
			mout.info() << "Copying original (1st) alpha channel" << mout.endl;
			CopyOp().traverseChannel(src.getAlpha(), dst.getAlpha());
		}
	}


}

/*
void PaletteOp::processOLD(const ImageFrame &src,Image &dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	initialize();


	makeCompatible(src,dst);

	const size_t width  = dst.getWidth();
	const size_t height = dst.getHeight();
	const ChannelGeometry channels = palettePtr->getChannels();
	//const unsigned int channels = palette.hasAlpha() ? 4 : 3;

	const bool hasSpecialCodes = !specialCodes.empty();  // for PolarODIM nodata & undetected



	double d;
	Palette::const_iterator it;      // lower bound
	Palette::const_iterator itLast;  // upped bound
	//std::map<double,std::vector<double> >::iterator
	unsigned int k;
	//int code;
	std::map<double,PaletteEntry >::const_iterator cit;
	for (size_t  i = 0; i < width; ++i) {
		//std::cerr << "Palette: " << i << '\t' << '\n';
		for (size_t j = 0; j < height; ++j) {

			if (hasSpecialCodes){  // PolarODIM
				//code = src.get<double>(i,j);
				cit = specialCodes.find(src.get<double>(i,j));
				if (cit != specialCodes.end()){
					for (k = 0; k < channels.getChannelCount(); ++k)
						dst.put(i,j,k, cit->second.color[k]);
					continue;
				}
			}

			d = scale * src.get<double>(i,j) + offset;

			// TODO: stl::lower_bound
			itLast = palettePtr->begin();
			for (it = palettePtr->begin(); it != palettePtr->end(); ++it){
				if (it->first > d)
					break;
				itLast = it;
			}

			for (k = 0; k < channels.getChannelCount(); ++k)
				dst.put(i,j,k, itLast->second.color[k]);


		}
	}


	if (channels.getAlphaChannelCount() == 0){
		if ((src.getAlphaChannelCount()>0) && (dst.getAlphaChannelCount()>0)){
			mout.info() << "Copying original alpha channel" << mout.endl;
			CopyOp().traverseChannel(src.getAlphaChannel(), dst.getAlphaChannel());
		}
	}


}
*/


}

}

// Drain
