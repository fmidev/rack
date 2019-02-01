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

#ifndef MAGICKDRAIN_H_
#define MAGICKDRAIN_H_





#ifdef DRAIN_MAGICK_yes
#include <Magick++.h>
#endif

// In compilation, use "Magick*-config" to get libs and includes right.


#include "Image.h"


namespace drain
{

namespace image
{

class MagickDrain
{
public:
	//MagickDrain();
	//virtual ~MagickDrain();
	
#ifdef DRAIN_MAGICK_yes
	/// Converts drain::image to MagickImage
	/// Does not support str types than <unsigned char>
	template<class T>
	static void convert(const ImageT<T> &drainImage, Magick::ImageT &magickImage);
	
	/// Converts MagickImage to drain::image
	/// Does not support str types than <unsigned char>
	template<class T>
	static void convert(Magick::ImageT &magickImage, ImageT<T> &drainImage);
	
#endif
	
};



#ifdef DRAIN_MAGICK_yes

// Looks like Magick inverts alpha channel
template <class T>
void MagickDrain::convert(const ImageT<T> &drainImage, Magick::ImageT &magickImage) {

	Magick::ImageType type = Magick::TrueColorMatteType;

	int imageChannels = drainImage.getImageChannelCount();
	int alphaChannels = drainImage.getAlphaChannelCount();

	if (alphaChannels > 1){
		std::cerr << "Warning: multiple alpha channel image, using 1st only \n";
		std::cerr << "  Image geometry:" << drainImage.getGeometry() << '\n';
		alphaChannels = 1;
	}

	const ImageT<T> *red = NULL, *green = NULL, *blue = NULL, *alpha = NULL;

	//std::string toOStr;
	//drainImage.getGeometry().toString(toOStr);
	//std::cout << "MagickDrain: " << drainImage.getGeometry() << std::endl;
	/*
	std::cerr << "img.chs:" << imageChannels  << std::endl;
	std::cerr << "alpha.chs:" << alphaChannels  << std::endl;
	std::cerr << "Extra debug:"  << std::endl;
	drainImage.debug();
	 */

	switch (imageChannels){
	case 0:
		if (alphaChannels == 0){
			std::cerr << "Error: zero channel image!\n";
		}
		else {
			type = Magick::GrayscaleType;
			red   =& drainImage.getChannel(0);
			green =& drainImage.getChannel(0);
			blue  =& drainImage.getChannel(0);
		}
		break;
	case 1:
		type = Magick::GrayscaleType;
		red   =& drainImage.getChannel(0);
		green =& drainImage.getChannel(0);
		blue  =& drainImage.getChannel(0);
		if (alphaChannels > 0) {
			type = Magick::GrayscaleMatteType;
			alpha =& drainImage.getAlphaChannel();
		}
		break;
	case 2:
		if (!alphaChannels) {
			type  =  Magick::GrayscaleMatteType;
			std::cerr << "Notice: 2 channel image, storing 2nd as alpha channel \n";
			red   =& drainImage.getChannel(0);
			green =& drainImage.getChannel(0);
			blue  =& drainImage.getChannel(0);
			alpha =& drainImage.getChannel(1);
		}
		else {
			std::cerr << "Notice: (2+alpha ) channel image, creating 'RGAA' image instead of 'RGBA'.\n";
			type = Magick::TrueColorMatteType;
			red   =& drainImage.getChannel(0);
			green =& drainImage.getChannel(1);
			blue  =& drainImage.getAlphaChannel();
			alpha =& drainImage.getAlphaChannel();
		}
		break;
	case 3:
		type  = Magick::TrueColorType;
		red   =& drainImage.getChannel(0);
		green =& drainImage.getChannel(1);
		blue  =& drainImage.getChannel(2);
		if (alphaChannels){
			type  =  Magick::TrueColorMatteType;
			alpha =& drainImage.getAlphaChannel();
		}
		break;
	default:
		type  =  Magick::TrueColorMatteType;
		red   =& drainImage.getChannel(0);
		green =& drainImage.getChannel(1);
		blue  =& drainImage.getChannel(2);

		if (alphaChannels){
			std::cerr << "Warning: (" << imageChannels << "+alpha) channel image, using (3+alpha). \n";
			alpha =& drainImage.getAlphaChannel();
		}
		else if (imageChannels == 4){
			std::cerr << "Notice: 4 channel image, storing 4th as alpha channel \n";
			alpha =& drainImage.getChannel(3);
		};
		imageChannels = 3;	// WHY?
	}


	const int width = drainImage.getGeometry().getWidth();
	const int height = drainImage.getGeometry().getHeight();


	//Magick::Image magickImage(Magick::Geometry(width,height),Magick::Color("black"));


	try {
		magickImage.classType(Magick::DirectClass);  // here?
		magickImage.size(Magick::Geometry(width,height));
		//magickImage.read("cow.png");

		magickImage.modifyImage(); // actually: _prepare_to_ modify
		magickImage.type(type);   //  The order copied from Magick examples


		Magick::PixelPacket *pixel_cache = magickImage.getPixels(0,0,width,height);


		Point2D<> p;
		int &i = p.x;
		int &j = p.y;

		// TODO: const unsigned drainBits = std::numeric_limits<unsigned char>::is_integer ?  std::numeric_limits<unsigned char>::max() :  //
		const unsigned int drainMax = std::numeric_limits<unsigned char>::max();
		const int shiftBits = magickImage.depth() - 8;  // ????


		int rowAddress = 0;
		int address = 0;

		for (j=0; j<height; j++){
			rowAddress = j*width;

			for (i=0; i<width; i++){
				address = i + rowAddress;
				pixel_cache[address].red =   (red->at(i,j)   << shiftBits);
				pixel_cache[address].green = (green->at(i,j) << shiftBits);
				pixel_cache[address].blue =  (blue->at(i,j)  << shiftBits);
				if (alpha != NULL)
					//pixel_cache[address].opacity = ((alpha->at(i,j))  << shiftBits);  //WARNING!!
					// Looks like Magick does NOT invert alpha channel IN THIS DIRECTION
					pixel_cache[address].opacity = ((drainMax-alpha->at(i,j))  << shiftBits);  //WARNING!!
			}
		}
		//std::cerr << "synching" << std::endl;
		//pixel_cache[rowAddress + 10 + width] = Magick::Color("green");

		magickImage.syncPixels();
	}
	catch (Magick::Error& e) {
		// because 'Error' is derived from the standard C++ std::exception, it has a 'what()' method
		std::cerr << "a Magick++ error occurred: " << e.what() << std::endl;
	}
	catch ( ... ) {
		std::cerr << "MagickDrain: an unhandled error has occurred; exiting application." << std::endl;
		exit(1);
	}


	// std::cerr << "magickImage.type = " << magickImage.type() << std::endl;

	// Store comments as KEY=VALUE pairs
	std::stringstream sstr;
	std::map<std::string,Data>::const_iterator it;
	for (it = drainImage.properties.begin(); it != drainImage.properties.end(); it++){
		sstr << it->first << '=' << it->second << '\n';
	}
	magickImage.comment(sstr.toStr());


}
#endif



#ifdef DRAIN_MAGICK_yes

// Looks like Magick inverts alpha channel
template <class T>
void  MagickDrain::convert(Magick::ImageT &magickImage, ImageT<T> &drainImage) {

	const int w = magickImage.columns();
	const int h = magickImage.rows();

	//drainImage.setGeometry(w,h)

	// TODO: redChannel = &drainImage.at(0,0,0);

	switch (magickImage.type()){
	case Magick::GrayscaleType:
		drainImage.setGeometry(w,h,1);
		magickImage.write(0,0,w,h,"I",Magick::CharPixel,&drainImage.at(0,0));
		break;
	case Magick::GrayscaleMatteType:
		drainImage.setGeometry(w,h,1,1);
		magickImage.write(0,0,w,h,"I",Magick::CharPixel,&drainImage.at(0,0,0));
		magickImage.write(0,0,w,h,"A",Magick::CharPixel,&drainImage.at(0,0,1));
		break;
		//    case Magick::RGBColorspace:
	case Magick::PaletteType: // just test status
	case Magick::TrueColorType:
		drainImage.setGeometry(w,h,3);
		magickImage.write(0,0,w,h,"R",Magick::CharPixel,&drainImage.at(0,0,0));
		magickImage.write(0,0,w,h,"G",Magick::CharPixel,&drainImage.at(0,0,1));
		magickImage.write(0,0,w,h,"B",Magick::CharPixel,&drainImage.at(0,0,2));
		break;
	case Magick::PaletteMatteType:
	case Magick::TrueColorMatteType:
		drainImage.setGeometry(w,h,3,1);
		magickImage.write(0,0,w,h,"R",Magick::CharPixel,&drainImage.at(0,0,0));
		magickImage.write(0,0,w,h,"G",Magick::CharPixel,&drainImage.at(0,0,1));
		magickImage.write(0,0,w,h,"B",Magick::CharPixel,&drainImage.at(0,0,2));
		magickImage.write(0,0,w,h,"A",Magick::CharPixel,&drainImage.at(0,0,3));
		//      magickImage.write(0,0,w,h,"A",Magick::CharPixel,&drainImage.alphaChannel(0)[0]);
		break;
		//    default:
	default:
		std::stringstream sstr;
		sstr << "operator<<(image,magickImage) : Magick type " << magickImage.type() << " not handled.";
		throw std::runtime_error(sstr.toStr());
	}

	// TODO contradictory!
	if (drainImage.getAlphaChannelCount()>0){
		//Image<> & alpha = drainImage.getAlphaChannel();
		//NegateOp<>().process(alpha,alpha);  // Looks like Magick inverts alpha channel IN THIS DIRECTION.
		//ScaleOp<>(-1.0,255).process(alpha,alpha);  // Looks like Magick inverts alpha channel
	}

	std::stringstream sstr(magickImage.comment());  // dont touch sstr!!
	drainImage.properties.reader.read(sstr);

	if (drain::Debug > 5){ // TODO static (does not work)
			std::cerr << "read magickImage.type  = " << magickImage.type() << '\n';
			std::cerr << "comment='" << magickImage.comment() << "'\n";
			std::cerr << "::::::::::::::::::::\n";
			std::cerr << drainImage.properties;
	}


}

#endif




}  // image

}  // drain

#endif /*MAGICKDRAIN_H_*/
//#endif // ImageMagick

// Drain
