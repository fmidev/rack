/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

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
