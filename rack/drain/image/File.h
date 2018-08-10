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
#ifndef DRAIN_FILE_H_
#define DRAIN_FILE_H_

#include "MagickDrain.h"
#include "FilePng.h"

#include <string>

#include "ImageChannel.h"
#include "Image.h"
#include "ImageT.h"


namespace drain
{
namespace image
{

// using namespace std;

class File
{
public:
	//((File(const std::string &filename);

	//virtual ~File();
	inline
	static void read(Image &img, const std::string &path){
		FilePng::read(img, path);
	}

	inline
	static void read(ImageFrame &img, const std::string &path){
		FilePng::read(img, path);
	}

	template <class T>
	static void read(ImageT<T> &img,const std::string &path){
#ifdef DRAIN_MAGICK_yes
		Magick::ImageT magickImage;
		magickImage.read(path);
		MagickDrain::convert(magickImage,img);
#else
		// Todo PNM support
		FilePng::read(img,path);
#endif
	}


	//static void read(Image<unsigned char> &image,const std::string &path);
	inline
	static void write(const ImageFrame &img,const std::string &path){
		FilePng::write(img, path);
	}


	template <class T>
	static void write(const ImageT<T> &img,const std::string &path){
#ifdef DRAIN_MAGICK_yes

		Magick::ImageT magickImage;
		magickImage.size("1x1");
		magickImage.magick("RGBA");

		if (Debug > 1)
			std::cerr << "Converting image to Magick image." << std::endl;

		MagickDrain::convert(img,magickImage);

		if (Debug > 0)
			std::cerr << "Writing image" << img.getName() << " to "<< path << std::endl;

		magickImage.write(path);
#else
		FilePng::write(img,path);
#endif
	}

	//static void write(const Image<unsigned char> &image,const std::string &path);

	
};


}  // image

}  // drain

#endif /*FILE_H_*/

// Drain
