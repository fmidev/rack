/**

    Copyright 2011  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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

#include "ImageFrame.h"

namespace drain {


namespace image {


//drain::Log iLog;

//ImageScaling ImageFrame::dummyScaling;

void ImageFrame::setStorageType(const std::type_info &type){

	Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);

	if (type == typeid(void)){
		unsetType();
	}
	else if (type == typeid(bool)){
		mout.warn() << "storage type 'bool' not supported, using 'unsigned char'" << mout.endl;
		setStorageType(typeid(unsigned char)); // re-invoke
	}
	else if (type == typeid(std::string)){
		mout.error() << "storage type 'std::string' not applicable to images" << mout.endl;
		//setType(typeid(unsigned char));
	}
	else {
		caster.setType(type);
		byteSize = caster.getByteSize();
		segmentBegin.setType(type);
		segmentEnd.setType(type);
	}


};


/**  Changes the image to view another image instead of its own.
 */
void ImageFrame::setView(const ImageFrame & src, size_t channelStart, size_t channelCount, bool catenate){

	try {
		setStorageType(src.getType());
	}
	catch (...) {
		throw std::runtime_error("setView: source type undefined");
	}

	if (buffer.size() > byteSize){
		throw std::runtime_error("setView: buffer not empty; non-empty image cannot view another image.");
	}

	// TODO re/move?
	/*
	if (!src.getName().isEmpty()){
		std::stringstream sstr;
		sstr << std::string("&") << src.getName();
		if (channelCount==1)
			sstr << '[' << channelStart << ']';
		setName(sstr.str());
	}
	*/
	std::stringstream sstr;
	sstr << src.getName() << '[' << channelStart;
	if (channelCount > 1)
		sstr << ':' << (channelStart+channelCount-1);
	sstr << ']';
	setName(sstr.str());

	scalingPtr    = & src.getScaling();
	propertiesPtr = & src.properties;

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

	bufferPtr = & src.bufferPtr[address(channelStart*geometry.getArea()) * byteSize];
	segmentBegin = & bufferPtr[address(0)];
	segmentBegin.setType(src.getType());  // TODO

	segmentEnd   = & bufferPtr[address(geometry.getVolume()) * byteSize];
	segmentEnd.setType(src.getType()); // TODO

	//channelVector.clear();
	//updateChannelVector(); infinite loop?

	//scaling.setLimits(src.scaling.getMin<double>(), src.scaling.getMax<double>());
	//scaling.setPhysicalRange(src.getScaling().getMinPhys(), src.getScaling().getMaxPhys());
	//Limits(src.scaling.getMin<double>(), src.scaling.getMax<double>());
	//scaling.setScale(src.getScaling().getScale());
	setCoordinatePolicy(src.getCoordinatePolicy());
}



/// Prints images geometry, buffer size and type information.
void ImageFrame::toOStr(std::ostream & ostr) const {

	ostr << "Image '"<< getName() << "' ";
	if (isView())
		ostr << '@';
	ostr << ' ' << geometry << ' ' << Type::getTypeChar(getType()) << '@' << (getByteSize()*8) << 'b';
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
	//ostr << ' ' << (size_t) segmentBegin << ' ' << (size_t) segmentEnd << ' ';


}


size_t ImageFrame::getChannelIndex(const std::string & index) const {

	// consider: overflow check
	// consider: conv to lower case

	if (index.empty())
		return 0;

	switch (index.at(0)) {
	case 'r':  // red
		return 0;
	case 'g':  // green
		return 1;
	case 'b':  // blue
		return 2;
	case 'a':  // alpha
		return getImageChannelCount();
	default:
		/// Number
		std::stringstream sstr(index);
		size_t i;
		sstr >> i;
		if ((i == 0) && (index != "0"))
			throw std::range_error(index + "<-- Image::getChannelIndex: unknown channel symbol");
		else
			return i;
	}


}







}  // image::
}  // drain::
