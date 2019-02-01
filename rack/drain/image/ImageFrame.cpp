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

#include "ImageFrame.h"

namespace drain {


namespace image {


void ImageFrame::init(){

	properties["name"] = name; // ("security risk", if file paths included by default?)

	properties["type"].link(encoding.type);

	// properties["width"].link(geometry.width);
	// properties["height"].link(geometry.height);

	properties["scale"].link(encoding.scaling.scale);
	properties["offset"].link(encoding.scaling.offset);
	properties["minPhysValue"].link(encoding.scaling.minPhysValue);
	properties["maxPhysValue"].link(encoding.scaling.maxPhysValue);

	properties["coordinatePolicy"].link(coordinatePolicy.v).fillArray = true;


}

void ImageFrame::setStorageType(const std::type_info &type){

	Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);

	encoding.setType(type);
	segmentBegin.setType(type);
	segmentEnd.setType(type);

};

void ImageFrame::adjustBuffer(){

	const size_t s = geometry.getVolume() * encoding.byteSize;

	if (s > 0)
		buffer.resize(s);
	else
		buffer.resize(1);

	bufferPtr = &buffer[0];
	segmentBegin = (void *)&(*buffer.begin());
	segmentEnd   = (void *)&(*buffer.end());


}


/**  Changes the image to view another image instead of its own.
 */
void ImageFrame::setView(const ImageFrame & src, size_t channelStart, size_t channelCount, bool catenate){

	Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);

	try {
		setStorageType(src.getType());
	}
	catch (...) {
		throw std::runtime_error("setView: source type undefined");
	}

	if (buffer.size() > encoding.byteSize){// isView() inapplicable, returns false at new images
		throw std::runtime_error("setView: buffer not empty; non-empty image cannot view another image.");
	}

	// getter used, because possibly forwarded view-> view->
	scalingPtr    = & src.getScaling();
	propertiesPtr = & src.getProperties(); // what about (alpha) channel scaling?

	if (catenate){
		geometry.setGeometry(src.getWidth(), src.getHeight()*channelCount,1,0);
	}
	else {
		const size_t k0 = channelStart;
		const size_t k1 = channelStart + channelCount;
		const size_t i  = src.getImageChannelCount();

		/// Image channel range
		const size_t i0 = std::min(k0,i);
		const size_t i1 = std::min(k1,i);
		const size_t imageChannelCount = i1-i0;

		/// Alpha channel range
		const size_t a0 = std::max(i,k0);
		const size_t a1 = std::max(i,k1);
		const size_t alphaChannelCount = a1-a0;

		geometry.setGeometry(src.getWidth(), src.getHeight(), imageChannelCount, alphaChannelCount);

		// Reconsider scaling, if only alpha channel(s)
		/*
		if ((imageChannelCount == 0) && (alphaChannelCount>0)){
			scalingPtr = & src.getAlphaChannel().getScaling();
		}
		*/
	}

	setCoordinatePolicy(src.getCoordinatePolicy());


	bufferPtr    = & src.bufferPtr[address(channelStart*geometry.getArea()) * encoding.byteSize];

	// NOTE: (void *) needed, because bufferPtr is <unsigned char *> while these segment iterators vary.
	segmentBegin = (void *)& bufferPtr[address(0)];
	segmentEnd   = (void *)& bufferPtr[address(geometry.getVolume()) * encoding.byteSize];

	segmentBegin.setType(src.getType());
	segmentEnd.setType(src.getType());


	// Compose name by appending index to the source name.
	std::stringstream sstr;
	sstr << src.getName() << '[' << channelStart;
	if (channelCount > 1)
		sstr << ':' << (channelStart+channelCount-1);
	sstr << ']';
	setName(sstr.str());

	/*
	std::cerr << __FUNCTION__ << '\n';
	src.toOStr(std::cerr);
	std::cerr << '\n';
	toOStr(std::cerr);
	std::cerr << '\n';
	*/

}

void ImageFrame::copyData(const ImageFrame & src){

	Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	if (getGeometry() != src.getGeometry()){
		mout.error() << "conflicting geometries: " << getGeometry() << " vs. " << src.getGeometry() << mout.endl;
		return;
	}

	const_iterator sit = src.begin();
	for (iterator it = begin(); it != end(); ++it,++sit)
		*it = *sit;

}



/// Prints images geometry, buffer size and type information.
void ImageFrame::toOStr(std::ostream & ostr) const {

	if (isView())
		ostr << "View ";
	else
		ostr << "Image";
	ostr << " '"<< getName() << "'\t";
	ostr << ' ' << geometry << ' ' << Type::getTypeChar(getType()) << '@' << (getEncoding().getByteSize()*8) << 'b';
	//if (typeIsIntegerType() || (scaling.isScaled()))
	const ImageScaling & s = getScaling();
	if (s.isScaled() || s.isPhysical()){
		ostr << "*(";
		getScaling().toOStr(ostr);
		ostr << ")";
	}

	//if (scaling.isPhysical())
	//	ostr  << '[' << scaling.getMinPhys() << ',' << scaling.getMaxPhys() << ']';
	//ostr << ' ' << '[' << scaling.getMin<double>() <<  ',' << scaling.getMax<double>() << ']' << ' ' << scaling.getScale() << ' ';
	ostr << ' ' << 'c' << getCoordinatePolicy();
	ostr << ' ' << (size_t) segmentBegin << ' ' << (size_t) segmentEnd << ' ';


}


size_t ImageFrame::getChannelIndex(const std::string & index) const {

	// consider: conv to lower case

	Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);


	if (index.empty()){
		mout.warn() << "index str empty, returning 0" << mout.endl;
		return 0;
	}

	size_t i = 0;

	switch (index.at(0)) {
	case 'r':  // red
		i = 0;
		break;
	case 'g':  // green
		i =  1;
		break;
	case 'b':  // blue
		i =  2;
		break;
	case 'a':  // alpha
		i =  getImageChannelCount();
		break;
	default:
		/// Number
		std::stringstream sstr(index);
		sstr >> i;
		if ((i == 0) && (index != "0"))
			throw std::range_error(index + "<-- Image::getChannelIndex: unknown channel symbol");
	}

	if (i >= getChannelCount()){
		mout.warn() << "index " << i << " larger than channelCount " << getChannelCount() << mout.endl;
	}

	return i;

}







}  // image::
}  // drain::

// Drain
