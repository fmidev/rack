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

// #include "drain/util/Time.h"
#include <drain/image/ImageFile.h>
#include "FilePng.h"
#include "FilePnm.h"



namespace drain
{

namespace image
{

int ImageFile::index(0);

void ImageFile::read(Image &img, const std::string & path){

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	/*
	mout.warn("path: ", path);
	mout.special("PNM: ", FilePnm::fileInfo.checkPath(path));
	mout.special("PNG: ", FilePng::fileInfo.checkPath(path));
	*/

	if (FilePnm::fileInfo.checkPath(path)){
		mout.debug("file format: PNM" );
		FilePnm::read(img, path); // , commentReader
	}
	else {
		if (FilePng::fileInfo.checkPath(path))
			mout.debug("file format: PNG" );
		else
			mout.warn("unrecognized extension, assuming PNG" );
		FilePng::read(img, path); // , commentReader
	}
}


void ImageFile::readFrame(ImageFrame &img, const std::string & path){

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);


	if (FilePng::fileInfo.checkPath(path)){
		mout.debug("file format: PNG" );
		FilePng::read(img, path);
	}
	else if (FilePnm::fileInfo.checkPath(path)){
		mout.debug("file format: PBM/PNM" );
		FilePnm::readFrame(img, path);
	}
	else {
		mout.warn("Unrecognized extension, assuming png" );
		FilePng::read(img, path);
	}

}




//static void read(Image<unsigned char> &image,const std::string &path);
void ImageFile::write(const ImageFrame &img,const std::string &path){

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	FilePath filePath(path);

	if (FilePng::fileInfo.checkExtension(filePath.extension)){
		mout.debug("file format: PNG" );
		FilePng::write(img, path);
	}
	else if (FilePnm::fileInfo.checkExtension(filePath.extension)){
		mout.debug("file format: PNM" );
		FilePnm::write(img, path);
	}
	else {
		mout.warn("unrecognized extension, assuming png" );
		FilePng::write(img, path);
	}
}


void ImageFile::writeIndexed(const ImageFrame &image, const std::string & pathPrefix, int i, int digits){

	if (i >= 0){
		ImageFile::index = i;
	}

	std::stringstream sstr;
	sstr << pathPrefix;
	sstr.width(digits);
	sstr.fill('0');
	sstr << index << ".png";
	FilePng::write(image, sstr.str());

	++index;

}


} // image::

} // drain::

