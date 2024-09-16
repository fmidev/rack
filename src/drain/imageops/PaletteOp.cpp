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
#include <list>
#include "PaletteOp.h"

namespace drain
{

namespace image
{


// drain::Bank<Palette> PaletteOp::paletteBank;
//std::map<std::string,drain::image::Palette> PaletteOp::paletteMap;

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

/*
Palette & PaletteOp::ensurePalette(const std::string & key){

	#pragma omp critical
	{
		if (!paletteBank.has(key)){
			Palette & p = paletteBank.add<Palette>(key); // Bank could support derived <OtherPalette>!
			p.comment = key;
			p.title = key+"...";
		}
		std::cout << __FILE__ << '/' << __FUNCTION__ << ':' << key << '\n';
	}
	return paletteMap[key];
}
*/



Palette & PaletteOp::loadPalette(const std::string & key){

	Logger mout(__FILE__, __FUNCTION__);
	// Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	//Palette & palette = getPaletteMap()[key];
	Palette & palette = getPaletteMap().get(key);
	#pragma omp critical
	{
		palette.load(key, true);
		if (palette.title.empty()){
			mout.debug("Palette had no title, added: ", key);
			palette.title = key;
		}
		//setPalette(palette);
	}
	return palette;
}


Palette & PaletteOp::getPalette(const std::string & key) {

	Logger mout(__FILE__, __LINE__, __FUNCTION__);

	//Palette & palette = getPaletteMap()[key];
	Palette & palette = getPaletteMap().get(key);

	if (key.empty()){
		mout.experimental("Returning generic palette (empty quantity: [])");
	}
	else if (palette.empty()){

		/*
		const std::size_t i = key.find('/');
		if (i != std::string::npos){
			mout.unimplemented("black and white");
			Palette & paletteOrig = getPaletteMap().get(key.substr(0, i));
			mout.error("unimplemented");
		}
		*/


		palette.load(key, true);
		if (palette.empty()){
			mout.warn("Empty palette [", key, "] // ", palette.comment);
		}
	}
	else {
		mout.experimental("Using cached palette [", key, "]");
	}
	// setPalette(palette);
	return palette;
	/*
	if (paletteBank.has(key)){
		mout.experimental("Using cached palette [", key, "]");
		return paletteBank.get(key);
	}
	else {
		Palette & palette = loadPalette(key);
		return palette;
	}
	*/
}

//std::map<std::string,drain::image::Palette>
PaletteMap & PaletteOp::getPaletteMap(){
	static
	PaletteMap paletteMap;
	return paletteMap;
};



// Add label
void PaletteOp::registerSpecialCode(const std::string & code, double d) {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	if ((this->palettePtr == NULL) || (this->palettePtr->empty())){
		//throw std::runtime_error("PaletteOp::setSpecialCode: palette not set (null)");
		mout.error("no paletted loaded or linked " );
	}

	Palette::spec_t::const_iterator it = palettePtr->specialCodes.find(code);
	if (it != palettePtr->specialCodes.end()){
		mout.experimental("Notice: global(?) palette modified with special code: ", d, '=', it->second);
		specialCodes[d] = it->second; // copy
		// specialCodes[d].value = d; NEW 2023
	}
	else {
		mout.debug(*palettePtr);
		//mout.note(palettePtr->specialCodes );
		mout.warn("could not find entry: ", code, '(', d, ')');
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
		mout.warn(" no palette loaded " );
		dst.setGeometry(src.getGeometry());
		return;
	}

	// const ChannelGeometry & colours = palettePtr->getChannels();
	// mout.debug(colours );

	// TODO: what if src image has alpha channel?
	const size_t alphaChannelCount = dst.getAlphaChannelCount();

	dst.setArea(src.getGeometry());
	dst.setChannelCount(palettePtr->getChannels());

	if (dst.getAlphaChannelCount() != alphaChannelCount){
		mout.warn("dst alpha channel count changed from " ,alphaChannelCount , " to " , dst.getAlphaChannelCount() );
	}

	if (src.hasAlphaChannel() && !dst.hasAlphaChannel()){
		mout.unimplemented("src has alpha channel, but palette not" );
	}

	//dst.setGeometry(src.getWidth(), src.getHeight(), colours.getImageChannelCount(), colours.getAlphaChannelCount());

	mout.debug("dst: ", dst);

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

	drain::Logger mout(__FILE__, __FUNCTION__); //REPL this->name+"(ImageOp::)[const ChannelTray &, ChannelTray &]", __FUNCTION__);

	// mout.debug("Starting" );

	mout.debug2(src );
	mout.debug2(dst );

	const Channel & srcChannel = src.get(0);
	if (src.size() > 1){
		mout.note("src has " , src.size() , " > 1 channels, using first " );
	}

	mout.debug("srcChannel " , srcChannel   );

	const size_t width  = dst.getGeometry().getWidth();
	const size_t height = dst.getGeometry().getHeight();
	const ChannelGeometry paletteChannels = palettePtr->getChannels();

	mout.debug(paletteChannels );

	// Note: colouring involves also alpha, if found, so channelCount includes alpha channel(s)

	size_t channelCount = dst.getGeometry().channels.getChannelCount();

	if (channelCount > paletteChannels.getChannelCount()){
		mout.note("dst has " , channelCount , " colours (channels), using that of palette: " , paletteChannels.getChannelCount() );
		channelCount = paletteChannels.getChannelCount();
	}
	else if (channelCount < paletteChannels.getChannelCount()){
		mout.note("palette has " , paletteChannels.getImageChannelCount() , " colours (channels), using only that of dst: " , channelCount );
	}


	const bool SPECIAL_CODES = !specialCodes.empty();  // for PolarODIM nodata & undetected

	const Palette & pal = *palettePtr;

	//const std::type_info & type  = srcChannel.getType();
	const ImageConf & encoding = srcChannel.getConf();

	//const ValueScaling & scaling = srcChannel.getScaling();
	const ValueScaling scaling(scale, offset);

	//mout.warn("src    scaling " , encoding.scaling );
	//mout.warn("src getScaling " , srcChannel.getScaling() );
	// encoding.scaling.
	//mout.warn("src    scaling " , scaling );

	const bool SCALED = scaling.isScaled();
	const bool UCHAR  = (encoding.getType() == typeid(unsigned char));      // && !SCALED;
	const bool USHORT = (encoding.getType() == typeid(unsigned short int)) ;// && !SCALED;

	// const bool DST_USHORT =
	// intensity (gray level)
	double d;
	// lower bound
	// Palette::const_iterator it;
	// upped bound
	Palette::const_iterator pit;

	size_t k;
	size_t address;
	// Palette::const_iterator cit;

	if (UCHAR || USHORT){

		// This is needed for "normalising" quantities if desired (eg. Doppler wind, unamambiguous range)
		ValueScaling sc(scale, offset);

		mout.special("first entry: ", sprinter(*pal.begin())  );
		mout.special("last  entry: ", sprinter(*pal.rbegin()) );

		const Palette::lookup_t & lut = pal.createLookUp(encoding.getType(), sc);

		mout.info("created look-up table[", lut.size(), "] for input: ", drain::Type::getTypeChar(encoding.getType()) );
		mout.info("scaling: ", encoding.getScaling(), " => ", sc);

		if (mout.isDebug(2)){
			for (size_t i=0; i<lut.size(); ++i){
				mout.note(i, "\t", lut[i]->first, "\t", sc.fwd(i), "\t", pal.retrieve(sc.fwd(i))->first );
			}
		}

		for (size_t  i = 0; i < width; ++i) {

			for (size_t j = 0; j < height; ++j) {

				address = srcChannel.address(i,j);
				d = srcChannel.get<double>(address);

				// Check if special handling is needed
				// TODO: special codes could be in lut?
				if (SPECIAL_CODES){  // PolarODIM
					pit = specialCodes.find(d);
					if (pit != specialCodes.end()){
						for (k = 0; k < channelCount; ++k)
							dst.get(k).put(i,j, pit->second.color[k]);
						continue;
					}
				}

				// If needed, prescale to fit number of entries
				if (UCHAR){
					pit = lut[static_cast<int>(d)];
				}
				else { // if (USHORT){
					pit = lut[static_cast<int>(d) >> lut.bitShift];
				}

				for (k = 0; k < channelCount; ++k)
					dst.get(k).put(address, pit->second.color[k]);

			}
		}
	}
	else {

		mout.warn("Not uint8b or uint16b, hence using (slow) retrieval: ", srcChannel);
		mout.note("scaling: " , scaling);
		//mout.warn("using (slow) retrieval, scaled=" , SCALED , ',' , drain::Type::getTypeChar(encoding.getType()) , ", " , scaling );

		if (mout.isDebug(1)){
			mout.debug("first entries of palette: " );
			mout.debug() << '\n';
			for (size_t  i = 0; i < 25; ++i) {
				mout << i << '>' << scaling.fwd(i) << '\t';
				pit = pal.retrieve(scaling.fwd(i));
				mout << pit->first << '\t';
				for (k = 0; k < channelCount; ++k)
					mout << pit->second.color[k] << '\t';
				mout << '\n';
			}
			mout << mout.endl;
		}

		for (size_t  i = 0; i < width; ++i) {

			for (size_t j = 0; j < height; ++j) {

				d = srcChannel.get<double>(i,j);

				if (SPECIAL_CODES){  // PolarODIM
					pit = specialCodes.find(d);
					if (pit != specialCodes.end()){
						for (k = 0; k < channelCount; ++k)
							dst.get(k).put(i,j, pit->second.color[k]);
						continue;
					}
				}

				if (SCALED){
					pit = pal.retrieve(scaling.fwd(d));
				}
				else {
					pit = pal.retrieve(d);
				}

				for (k = 0; k < channelCount; ++k)
					dst.get(k).put(i,j, pit->second.color[k]);

			}
		}
	}




	if (paletteChannels.getAlphaChannelCount() == 0){
		if ((src.getGeometry().channels.getAlphaChannelCount()>0) && (dst.getGeometry().channels.getAlphaChannelCount()>0)){
			mout.info("Copying original (1st) alpha channel" );
			CopyOp().traverseChannel(src.getAlpha(), dst.getAlpha());
		}
	}


}

/*
void PaletteOp::processOLD(const ImageFrame &src,Image &dst) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

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
			mout.info("Copying original alpha channel" );
			CopyOp().traverseChannel(src.getAlphaChannel(), dst.getAlphaChannel());
		}
	}


}
*/


}

}

// Drain
