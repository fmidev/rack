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

	// Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL __FILE__, __FUNCTION__);

	properties["name"] = name; // ("security risk", if file paths included by default?)

	properties["type"].link(conf.type);

	// properties["width"].link(width);
	// properties["height"].link(height);

	properties["scale"].link(conf.scale); // may be linked "away"
	properties["offset"].link(conf.offset);
	// TODO: properties["physRange"].link(scaling.physRange);
	properties["minPhysValue"].link(conf.physRange.min);
	properties["maxPhysValue"].link(conf.physRange.max);

	properties["coordinatePolicy"].link(getCoordinatePolicy().tuple()).fillArray = true;

	// mout.warn(properties["coordinatePolicy"] );
}

void ImageFrame::setStorageType(const std::type_info &type){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	conf.setType(type);
	segmentBegin.setType(type);
	segmentEnd.setType(type);

}

void ImageFrame::adjustBuffer(){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	/*
	mout.warn("Area: " , area , '=' ,  area.getArea() );
	mout.warn("Chns: " , channels , '=' ,  channels.getChannelCount() );
	mout.warn(geometry , '=' ,  getVolume() , '@' ,  encoding.byteSize );
	*/

	const size_t s = getVolume() * conf.byteSize;

	if (s > 0){
		//mout.warn("size=" , s , "\t = " , getVolume() , '*' , encoding.byteSize );
		//mout.warn(getConf() );
	}

	try {
		if (s > 0)
			buffer.resize(s);
		else {
			// NEW NEW... code checker noticed...
			buffer.resize(1); // unsafe buffer[0] below?
			// NEW
			// buffer.resize(0); // unsafe buffer[0] below?
			// OLD
			// buffer.resize(1);
		}
	}
	catch (const std::runtime_error & e) {
		mout.error("allocating image data failed");
	}

	bufferPtr = &buffer[0];
	segmentBegin = (void *)&(*buffer.begin());
	segmentEnd   = (void *)&(*buffer.end());


}


/**  Changes the image to view another image instead of its own.
 */
void ImageFrame::setView(const ImageFrame & src, size_t channelStart, size_t channelCount, bool catenate){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	try {
		setStorageType(src.getType());
	}
	catch (...) {
		throw std::runtime_error("setView: source type undefined");
	}

	if (buffer.size() > conf.byteSize){// isView() inapplicable, returns false at new images
		throw std::runtime_error("setView: buffer not empty; non-empty image cannot view another image.");
	}

	// getter used, because possibly forwarded view-> view->

	/// ENCODING
	conf.setScaling(src.getScaling());
	//conf.linkScaling(src.getScaling());    // NOTE: links also phys range
	propertiesPtr = & src.getProperties(); // what about scaling etc ref variables?

	// mout.fail("src:  " , src );
	// mout.fail("src.sc: " , src.getScaling() );
	// mout.fail("cnf.sc: " , conf );

	// GEOMETRY
	if (catenate){
		conf.setGeometry(src.getWidth(), src.getHeight()*channelCount,1,0);
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

		conf.setGeometry(src.getWidth(), src.getHeight(), imageChannelCount, alphaChannelCount);

		// Reconsider scaling, if only alpha channel(s)
		/*
		if ((imageChannelCount == 0) && (alphaChannelCount>0)){
			scalingPtr = & src.getAlphaChannel().getScaling();
		}
		*/
	}

	// COORDINATES
	setCoordinatePolicy(src.getCoordinatePolicy());

	// DATA
	bufferPtr    = & (src.bufferPtr[address(channelStart*getArea()) * conf.byteSize]);

	// NOTE: (void *) needed, because bufferPtr is <unsigned char *> while these segment iterators vary.
	segmentBegin = (void *)& (bufferPtr[address(0)]);
	segmentEnd   = (void *)& (bufferPtr[address(getVolume()) * conf.byteSize]);

	segmentBegin.setType(src.getType());
	segmentEnd.setType(src.getType());


	// Compose name by appending index to the source name.
	if (!src.getName().empty()){
		std::stringstream sstr;
		sstr << src.getName() << '[' << channelStart;
		if (channelCount > 1)
			sstr << ':' << (channelStart+channelCount-1);
		sstr << ']';
		setName(sstr.str());
	}
	/*
	std::cerr << __FUNCTION__ << '\n';
	src.toOStr(std::cerr);
	std::cerr << '\n';
	toOStr(std::cerr);
	std::cerr << '\n';
	*/

	/*
	mout.fail("cnf<  : " , conf );
	const ValueScaling & sc = conf.getScaling();
	mout.fail("cnf>  : " , conf );
	*/

}

void ImageFrame::copyData(const ImageFrame & src){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	mout.debug("start" );

	if (getGeometry() != src.getGeometry()){
		mout.warn("conflicting geometries: " , *this );
		mout.error("conflicting geometries: " , src.getGeometry() , " vs " , getGeometry() );
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
	//ostr << getGeometry() << ' ' << Type::getTypeChar(conf.getType()) << '@' << (conf.getElementSize()*8) << 'b' << ' ';

	ostr << getConf();

	/*
	// Scaling is worth displaying only if it "exists" ...
	//const drain::ValueScaling & s = getScaling();
	const Encoding & s = getConf();
	//if (s.isScaled() || s.isPhysical()){
		ostr << "*(" << s << ")";
		// ostr << "*(" << s << ")";
	//}
	ostr << (s.hasOwnScaling() ? '!' : '&') << "\t";
	*/


	/*
	ostr << ' ' << getgeometry << ' ' << Type::getTypeChar(getType()) << '@' << (getEncoding().getElementSize()*8) << 'b';
	//if (typeIsIntegerType() || (scaling.isScaled()))
	const drain::ValueScaling & s = getScaling();
	if (s.isScaled() || s.isPhysical()){
		ostr << "*(";
		getScaling().toStream(ostr);
		ostr << ")";
	}

	//if (scaling.isPhysical())
	//	ostr  << '[' << scaling.getMinPhys() << ',' << scaling.getMaxPhys() << ']';
	//ostr << ' ' << '[' << scaling.getMin<double>() <<  ',' << scaling.getMax<double>() << ']' << ' ' << scaling.getScale() << ' ';
	ostr << ' ' << 'c' << getCoordinatePolicy();
	*/

	ostr << ' ' << (size_t) segmentBegin << ' ' << (size_t) segmentEnd << ' ';


}



}  // image::

template <>
std::ostream & drain::Sprinter::toStream<drain::image::ImageFrame>(std::ostream & ostr, const drain::image::ImageFrame & src, const SprinterLayout & layout) {
	src.toOStr(ostr); // consider using something from the layout
	return ostr;
}


}  // drain::

