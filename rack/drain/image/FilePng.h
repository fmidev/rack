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
#ifndef FILEPNG_H_
#define FILEPNG_H_


#include <string>
#include <iostream>
#include <fstream>
#include <exception>

#include <png.h>

#include "Image.h"
// #include "util/Time.h"


namespace drain
{
namespace image
{

// using namespace std;

/// For reading and writing images in PNG format
/** A lighter alternative for Magick++ which supports tens of image formats.
 *  Portable Network Graphics (PNG) format is a widely adapted, flexible image format
 *  for gray level and color images. It supports lossless compression and alpha channels.
 */
class FilePng
{
public:


	/// Reads a png file to an image.
	/** Reads a png file to drain::Image.
	 *  Converts indexed (palette) images to RGB or RGBA.
	 *  Scales data to 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	 *
	 */
	static
	void read(Image &image, const std::string &path, int png_transforms = 0);  //(PNG_TRANSFORM_PACKING || PNG_TRANSFORM_EXPAND));  16 >> 8?

	/// Writes image to a png file.
	/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
	 *  Writes in 8 or 16 bits, according to template class.
	 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
	*/
	static void write(const Image &image,const std::string &path);


};

}

}

#endif /*FILEPng_H_*/

// Drain
