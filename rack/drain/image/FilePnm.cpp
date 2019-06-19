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
/**
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */

#include "FilePnm.h"
#include "util/Time.h"

namespace drain
{

namespace image
{

/// Syntax for recognising image files
const drain::RegExp FilePnm::fileNameRegExp("^((.*/)?([^/]+))\\.(p[bgpn]m)$", REG_EXTENDED | REG_ICASE);


//int FilePng::index(0);

template <>
void FilePnm::initialize(Image & image, const std::type_info & t, const Geometry & g){
	image.initialize(t,g);
}

template <>
void FilePnm::initialize(ImageFrame & image, const std::type_info & t, const Geometry & g){

	if ((image.getType() == t) && (image.getGeometry() == g))
		return;
	else
		std::runtime_error(std::string("FilePng::")+__FUNCTION__+": tried to change ImageFrame geometry");

}


/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::read(ImageFrame & image, std::istream & infile){ // , FileType t

	Logger mout(getImgLog(), "FilePnm", __FUNCTION__);



	mout.warn() << "reading image: " << image << mout.endl;

	const size_t channels = image.getChannelCount();

	if (channels == 1){
		ImageFrame::iterator  it = image.begin();
		const ImageFrame::iterator eit = image.end();
		while ((infile) && (it != eit)){
			*it = infile.get();
			++it;
		}
		if (it != eit){
			mout.warn() << "premature end of file: " << image << mout.endl;
		}
	}
	else if (channels == 3){
		ImageFrame::iterator  rit = image.getChannel(0).begin();
		ImageFrame::iterator  git = image.getChannel(1).begin();
		ImageFrame::iterator  bit = image.getChannel(2).begin();
		while (infile){
			*rit = infile.get();
			++rit;
			*git = infile.get();
			++git;
			*bit = infile.get();
			++bit;
			// TODO CHECK
		}
		/*
		if (rit != eit){
			mout.warn() << "premature end of file: " << image << mout.endl;
		}
		*/
	}
	else {
		mout.error() << "Sorry, this type of PNM format (" << channels << " channels) not implemented" << mout.endl;
	}

	if (infile){
		mout.warn() << "spurious bytes in end of file" << mout.endl;
	}

	//      return true;

}



/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::write(const ImageFrame & image, const std::string & path){

	Logger mout(getImgLog(), "FilePnm", __FUNCTION__);

	if (image.isEmpty()){
		mout.warn() << "empty image, skipping" << mout.endl;
		return;  // -1
	}

	mout.debug() << "Path: " << path << mout.endl;


	const int width    = image.getWidth();
	const int height   = image.getHeight();
	const int channels = image.getChannelCount();

	int color_type = UNDEFINED;
	switch (channels) {
	case 4:
		mout.warn() << "four-channel image, writing channels 0,1,2 only" << mout.endl;
		// no break
	case 3:
		color_type = PGM_RAW;
		break;
	case 2:
		mout.warn() << "two-channel image, writing channel 0" << mout.endl;
		// no break
	case 1:
		color_type = PGM_RAW;
		break;
	case 0:
		mout.warn() << "zero-channel image" << mout.endl;
		//fclose(fp);
		return;
	default:
		mout.error()  << "unsupported channel count: " << channels << mout.endl;
		// throw std::runtime_error(s.toStr());
	}

	mout.error()  << "unimplemented code"<< mout.endl;


	// Copy data to png array.
	mout.info() << "Src: " << image << mout.endl;
	//mout.note() << "Image of type " << image.getType2() << ", scaling: " << image.getScaling() << mout.endl;
	mout.debug(1) << "Copy data to png array, width=" << width << ", height=" << height << " channels=" << channels << mout.endl;
	for (int k = 0; k < channels; ++k) {
		//const double coeff = image.get<png_byte>(i,j,k);
		const Channel & channel = image.getChannel(k);
		mout.debug(2) << " channel " << k << " scaling=" << channel.getScaling() << mout.endl;

		/*
		for (int i = 0; i < width; ++i) {
			i0 = (i*channels + k)*2;
			for (int j = 0; j < height; ++j) {
				// value = static_cast<int>(coeff*channel.get<double>(i,j));
				value = static_cast<int>(conv.inv(channel.get<double>(i,j)));
				data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
				data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
			}
		}
		 */

	}


	mout.debug(1) << "Closing file" << mout.endl;
	//file
}



void FilePnm::writeIndexed(const ImageFrame &image, const std::string & pathPrefix, int i, int digits){

	if (i >= 0){
		//FilePng::index = i;
	}

	std::stringstream sstr;
	sstr << pathPrefix;
	sstr.width(digits);
	sstr.fill('0');
	sstr << i << ".png";
	FilePnm::write(image, sstr.str());

	//++index;

}


} // image::

} // drain::



// Drain
