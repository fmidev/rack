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

#include "Image.h"

namespace drain {


namespace image {

drain::Monitor iMonitor;


/**  Changes the image to view another image instead of its own.
 */
void Image::setView(const Image & src, size_t channelStart, size_t channelCount, bool catenate){

	try {
		setType(src.getType());
	}
	catch (...) {
		throw std::runtime_error("setView: source type undefined");
	}

	if (buffer.size() > byteSize){
		throw std::runtime_error("setView: buffer not empty; non-empty image cannot view another image.");
	}

	if (!src.getName().empty()){
		std::stringstream sstr;
		sstr << std::string("&") << src.getName();
		if (channelCount==1)
			sstr << '[' << channelStart << ']';
		setName(sstr.str());
	}

	//setScalingMode(src.getScalingMode());

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

		/// Alpha channel range
		const size_t a0 = std::max(i,k0);
		const size_t a1 = std::max(i,k1);

		geometry.setGeometry(src.getWidth(), src.getHeight(), i1-i0, a1-a0);
	}

	bufferPtr = & src.bufferPtr[address(channelStart*geometry.getArea()) * byteSize];
	segmentBegin = & bufferPtr[address(0)];
	segmentBegin.setType(src.getType());  // TODO

	segmentEnd   = & bufferPtr[address(geometry.getVolume()) * byteSize];
	segmentEnd.setType(src.getType()); // TODO
	channelVector.clear();

	setLimits(src.getMin<double>(), src.getMax<double>());
	setScale(src.getScale());
	setCoordinatePolicy(src.getCoordinatePolicy());
}

size_t Image::getChannelIndex(const std::string & index) const {

	if (index.empty())
		return 0;

	//int i = -1;
	switch (index.at(0)) {
	/*
		case 'F':
			return 0;
			break;
		case 'f':
			return 0;
			break;
		case 'i':  // image channels (excluding alpha)
			return 0;
			break;
	 */
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

void Image::setView(const Image & src, const std::string & view) {

	const char v = view.empty() ? 'F' : view.at(0);

	switch (v) {
	case 'F':
		setView(src, 0, src.getChannelCount(), true);
		break;
	case 'f':
		setView(src);
		break;
	case 'i':  // image channels (excluding alpha)
		setView(src, 0, src.getImageChannelCount());
		break;
	default:
		setView(src, src.getChannelIndex(view));
	/*
	case 'r':  // red
		setView(src, 0);
		break;
	case 'g':  // green
		setView(src, 1);
		break;
	case 'b':  // blue
		setView(src, 2);
		break;
	case 'a':  // alpha
		setView(src.getAlphaChannel());
		break;
	default:
		/// Number
		std::stringstream sstr(view);
		size_t i;
		sstr >> i;
		if ((i == 0) && (view != "0"))
			throw range_error(view + "<-- Image::setView: unknown channel symbol");
		else
			setView(src, i, 1);
		break;
		*/
	}
}

Image & Image::getAlphaChannel(){

	if (getAlphaChannelCount() == 0)
		setAlphaChannelCount(1);

	const size_t a = getImageChannelCount();

	return getChannel(a);
}

const Image & Image::getAlphaChannel() const {

	if (getAlphaChannelCount() == 0)
		throw std::range_error("ImageBase::getAlphaChannel: no alpha channel");

	const size_t a = getImageChannelCount();

	return getChannel(a);
}

Image & Image::getChannel(const size_t &i){

	if (i > getChannelCount())
		throw std::range_error("ImageBase::getChannel: too large channel index");

	updateChannelVector();

	return channelVector[i];
}

const Image & Image::getChannel(const size_t &i) const {

	if (i > getChannelCount())
		throw std::range_error("ImageBase::getChannel: too large channel index");

	updateChannelVector();

	return channelVector[i];
}


void Image::updateChannelVector() const {

	const size_t n = getChannelCount();

	if (channelVector.size()<n){ // why not != ???
		channelVector.resize(n);
		for (size_t k = 0; k < n; ++k) {
			Image & channel = channelVector[k];
			channel.setView(*this,k,1);
			channel.geometry.setChannelCount(1,0); // 2012: Force pure image channels
			// 2016 copy scaling;
			channel.setLimits(this->minValue, this->maxValue);
			channel.setScale(this->scale);
		}
	};

}



/// Prints images geometry, buffer size and type information.
void Image::info(std::ostream & ostr) const {

	ostr << "Image '"<< getName() << "' ";
	if (isView())
		ostr << '@';
	ostr << ' ' << geometry << ' ' << Type::getTypeChar(getType()) << '@' << (getByteSize()*8) << 'b';
	ostr << ' ' << '[' << getMin<double>() <<  ',' << getMax<double>() << ']' << ' ' << getScale() << ' ';
	ostr << 'C' << getCoordinatePolicy();

	//width << 'x' << height  << 'x' << channels << '\n'
	/*
	if (!isView()) {
		ostr << "Buffer: "<< buffer.size();
		ostr << " type=" << getType().name() << " (" << getByteSize() << "byte=" << (getByteSize()*8) << "bit) ";
		ostr << " [" << getMin<double>() << ',' << getMax<double>() << ']';
		ostr << " coordPolicy " << getCoordinatePolicy() << '\n';
	}
	else
		ostr << " [view]\n";
	 */
	//	ostr << " [view]=" << isView() << "\n";
	//ostr << getWidth() << 'X' << getHeight() << '\n';
	//ostr << (long)bufferPtr << '\n';
	//ostr << (long) &buffer[0] << '\n';

}

/// Prints images geometry, buffer size and type information,
//  ///// and dumps the array contents.
void Image::dump(std::ostream & ostr) const {
	info(ostr);
	ostr << "begin=" << (long)begin() << '\n';
	ostr << "end=  " << (long)end() << '\n';

	/*
	ostr << "BEGIN{";
	for (iterator it = begin(); it != end(); ++it){
		//ostr << (float)*it;
		ostr << *it << ',';
	}
	ostr << "}END\n";

	for (size_t j=0; j<geometry.getHeight(); j++){
		ostr << "Row "<< j << '\n';
		for (size_t i=0; i<geometry.getWidth(); i++)
			//ostr << i << '|' << ' '; // << image.get<double>(i,j) << '\t';
			ostr << get<double>(i,j) <<'\t';
		ostr << '\n';
	};

	ostr << "buffer:\n";
	for (size_t j=0; j< geometry.getVolume(); j++){
		ostr << j << '\t';
		for (size_t i=0; i<byteSize; i++){
			ostr << (int)buffer[j*byteSize+i] << ',';
		}
		ostr << '\n';
	}
	*/

}

}  // image::
}  // drain::

// Drain
