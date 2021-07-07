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


void PaletteOp::registerSpecialCode(const std::string & code, double f) {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	if ((this->palettePtr == NULL) || (this->palettePtr->empty()))
		//throw std::runtime_error("PaletteOp::setSpecialCode: palette not set (null)");
		mout.error() << "no paletted loaded or linked " << mout.endl;

	Palette::spec_t::const_iterator it = palettePtr->specialCodes.find(code);
	if (it != palettePtr->specialCodes.end()){
		specialCodes[f] = it->second; // copy
		specialCodes[f].value = f;
	}
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

void PaletteOp::getDstConf(const ImageConf &src, ImageConf &dst) const {
//void PaletteOp::makeCompatible(const ImageFrame &src, Image &dst) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	if (palettePtr->empty()){
		mout.warn() << " no palette loaded " << mout.endl;
		dst.setGeometry(src.getGeometry());
		return;
	}

	// const ChannelGeometry & colours = palettePtr->getChannels();
	// mout.debug() << colours << mout;

	// TODO: what if src image has alpha channel?
	const size_t alphaChannelCount = dst.getAlphaChannelCount();

	dst.setArea(src);
	dst.setChannelCount(palettePtr->getChannels());


	if (dst.getAlphaChannelCount() != alphaChannelCount){
		mout.warn() << "dst alpha channel count changed from " <<alphaChannelCount << " to " << dst.getAlphaChannelCount() << mout;
	}


	if (src.hasAlphaChannel() && !dst.hasAlphaChannel()){
		mout.unimplemented() << "src has alpha channel, but palette not" << mout;
	}

	//dst.setGeometry(src.getWidth(), src.getHeight(), colours.getImageChannelCount(), colours.getAlphaChannelCount());

	mout.debug() << "dst: " << dst << mout.endl;

}


void PaletteOp::help(std::ostream & ostr) const {

	ImageOp::help(ostr);

	//for (std::map<double,PaletteEntry >::const_iterator cit = specialCodes.begin(); cit != specialCodes.end(); ++cit){
	for (Palette::cont_t::const_iterator cit = specialCodes.begin(); cit != specialCodes.end(); ++cit){
		ostr << cit->first << '=' << cit->second << '\n';
	}
	for (std::map<std::string,PaletteEntry >::const_iterator pit = palettePtr->specialCodes.begin(); pit != palettePtr->specialCodes.end(); ++pit){
		ostr << "'" << pit->first << "'=" << pit->second << '\n';
	}
}

void PaletteOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__); //REPL this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);

	// mout.debug() << "Starting" << mout.endl;

	mout.debug2() << src << mout.endl;
	mout.debug2() << dst << mout.endl;

	const Channel & srcChannel = src.get(0);
	if (src.size() > 1){
		mout.note() << "src has " << src.size() << " > 1 channels, using first " << mout.endl;
	}

	mout.debug() << "srcChannel " << srcChannel   << mout.endl;

	const size_t width  = dst.getGeometry().getWidth();
	const size_t height = dst.getGeometry().getHeight();
	const ChannelGeometry paletteChannels = palettePtr->getChannels();

	mout.debug() << paletteChannels << mout.endl;

	// Note: colouring involves also alpha, if found, so channelCount includes alpha channel(s)

	size_t channelCount = dst.getGeometry().channels.getChannelCount();

	if (channelCount > paletteChannels.getChannelCount()){
		mout.note() << "dst has " << channelCount << " colours (channels), using that of palette: " << paletteChannels.getChannelCount() << mout.endl;
		channelCount = paletteChannels.getChannelCount();
	}
	else if (channelCount < paletteChannels.getChannelCount()){
		mout.note() << "palette has " << paletteChannels.getImageChannelCount() << " colours (channels), using only that of dst: " << channelCount << mout.endl;
	}


	const bool SPECIAL_CODES = !specialCodes.empty();  // for PolarODIM nodata & undetected

	const Palette & pal = *palettePtr;

	//const std::type_info & type  = srcChannel.getType();
	const ImageConf & encoding = srcChannel.getConf();

	//const ValueScaling & scaling = srcChannel.getScaling();
	const ValueScaling scaling(scale, offset);

	//mout.warn() << "src    scaling " << encoding.scaling << mout.endl;
	//mout.warn() << "src getScaling " << srcChannel.getScaling() << mout.endl;
	// encoding.scaling.
	//mout.warn() << "src    scaling " << scaling << mout.endl;

	const bool SCALED = scaling.isScaled();
	const bool UCHAR  = (encoding.getType() == typeid(unsigned char));      // && !SCALED;
	const bool USHORT = (encoding.getType() == typeid(unsigned short int)) ;// && !SCALED;

	// const bool DST_USHORT =
	// intensity (gray level)
	double d;
	// lower bound
	Palette::const_iterator it;
	// upped bound
	Palette::const_iterator itLast;

	size_t k;
	Palette::const_iterator cit;

	if (UCHAR || USHORT){

		// This is needed for "normalising" quantities if desired (eg. Doppler wind, unamambiguous range)
		ValueScaling sc(scale, offset);
		//sc.setPhysicalScale(typeid(unsigned char), min, max);

		mout.warn() << "first entry: " << sprinter(*pal.begin()) << mout;
		mout.warn() << "last  entry: " << sprinter(*pal.rbegin()) << mout;

		const Palette::lookup_t & lut = pal.createLookUp(encoding.getType(), sc);

		mout.info() << "created look-up table[" << lut.size() << "] for input: " << drain::Type::getTypeChar(encoding.getType()) << mout.endl;
		mout.info() << "scaling: " << encoding.getScaling() << " => " << sc << mout.endl;

		if (mout.isDebug(2)){
			for (size_t i=0; i<lut.size(); ++i){
				mout.note() << i << "\t" << lut[i]->first << "\t" << sc.fwd(i) << "\t" << pal.retrieve(sc.fwd(i))->first  << mout.endl;
			}
		}

		for (size_t  i = 0; i < width; ++i) {

			for (size_t j = 0; j < height; ++j) {

				d = srcChannel.get<double>(i,j);

				// Check if special handling is needed
				if (SPECIAL_CODES){  // PolarODIM
					cit = specialCodes.find(d);
					if (cit != specialCodes.end()){
						for (k = 0; k < channelCount; ++k)
							dst.get(k).put(i,j, cit->second.color[k]);
						continue;
					}
				}

				// If needed, prescale to fit number of entries
				if (UCHAR){
					itLast = lut[static_cast<int>(d)];
				}
				else { // if (USHORT){
					itLast = lut[static_cast<int>(d) >> lut.bitShift];
				}

				for (k = 0; k < channelCount; ++k)
					dst.get(k).put(i,j, itLast->second.color[k]);

			}
		}
	}
	else {

		mout.warn() << "using (slow) retrieval: " << srcChannel << mout.endl;
		mout.note() << "scaling: " << scaling << mout.endl;
		//mout.warn() << "using (slow) retrieval, scaled=" << SCALED << ',' << drain::Type::getTypeChar(encoding.getType()) << ", " << scaling << mout.endl;

		if (mout.isDebug(1)){
			mout.debug() << "first entries of palette: " << mout.endl;
			mout.debug() << '\n';
			for (size_t  i = 0; i < 25; ++i) {
				mout << i << '>' << scaling.fwd(i) << '\t';
				itLast = pal.retrieve(scaling.fwd(i));
				mout << itLast->first << '\t';
				for (k = 0; k < channelCount; ++k)
					mout << itLast->second.color[k] << '\t';
				mout << '\n';
			}
			mout << mout.endl;
		}

		for (size_t  i = 0; i < width; ++i) {

			for (size_t j = 0; j < height; ++j) {

				d = srcChannel.get<double>(i,j);

				if (SPECIAL_CODES){  // PolarODIM
					cit = specialCodes.find(d);
					if (cit != specialCodes.end()){
						for (k = 0; k < channelCount; ++k)
							dst.get(k).put(i,j, cit->second.color[k]);
						continue;
					}
				}

				if (SCALED){
					itLast = pal.retrieve(scaling.fwd(d));
				}
				else {
					itLast = pal.retrieve(d);
				}

				for (k = 0; k < channelCount; ++k)
					dst.get(k).put(i,j, itLast->second.color[k]);

			}
		}
	}




	if (paletteChannels.getAlphaChannelCount() == 0){
		if ((src.getGeometry().channels.getAlphaChannelCount()>0) && (dst.getGeometry().channels.getAlphaChannelCount()>0)){
			mout.info() << "Copying original (1st) alpha channel" << mout.endl;
			CopyOp().traverseChannel(src.getAlpha(), dst.getAlpha());
		}
	}


}

/*
void PaletteOp::processOLD(const ImageFrame &src,Image &dst) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	initialize();


	makeCompatible(src,dst);

	const size_t width  = dst.getWidth();
	const size_t height = dst.getHeight();
	const ChannelGeometry channels = palettePtr->getChannels();
	//const unsigned int channels = palette.hasAlpha() ? 4 : 3;

	const bool SPECIAL_CODES = !specialCodes.empty();  // for PolarODIM nodata & undetected



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

			if (SPECIAL_CODES){  // PolarODIM
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
