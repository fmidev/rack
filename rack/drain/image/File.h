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
#include "FilePnm.h"

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

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		if (FilePng::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNG" << mout.endl;
			FilePng::read(img, path);
		}
		else if (FilePnm::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNM" << mout.endl;
			FilePnm::read(img, path);
		}
		else {
			mout.warn() << "unrecognized extension, assuming png" << mout.endl;
			FilePng::read(img, path);
		}
	}

	inline
	static void read(ImageFrame &img, const std::string &path){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);
		if (FilePng::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNG" << mout.endl;
			FilePng::read(img, path);
		}
		else if (FilePnm::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNM" << mout.endl;
			FilePnm::read(img, path);
		}
		else {
			mout.warn() << "unrecognized extension, assuming png" << mout.endl;
			FilePng::read(img, path);
		}
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
	static inline
	void write(const ImageFrame &img,const std::string &path){

		drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

		if (FilePng::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNG" << mout.endl;
			FilePng::write(img, path);
		}
		else if (FilePnm::fileNameRegExp.test(path)){
			mout.debug() << "file format: PNM" << mout.endl;
			FilePnm::write(img, path);
		}
		else {
			mout.warn() << "unrecognized extension, assuming png" << mout.endl;
			FilePng::write(img, path);
		}
	}

	/*
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
	*/

	/// Writes image to a file, naming it: prefix + index + ".png", using desired number of leading zeros.
	/** Utility function esp. for debugging
	 *
	 *  \param image - image
	 *  \param pathPrefix - leading part of the path: directory and filename prefix.
	 *
	 *
	 */
	static void writeIndexed(const ImageFrame &image, const std::string & pathPrefix, int i=-1, int digits=3);

protected:

	/// Sets target type and geometry
	/**
	 *  In the case of ImageFrame (typically, an image channel), type and geometry have to match already;
	 *  otherwise an exception is thrown.
	 *
	 *  \tparam T - Image or ImageFrame
	 */
	template <class T>
	static
	void initialize(T &, const std::type_info & t, const Geometry & g);

	static
	int index;

};


}  // image

}  // drain

#endif /*FILE_H_*/

// Drain
