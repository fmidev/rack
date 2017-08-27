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
#include "Point.h"
#include "Window.h"

namespace drain
{

namespace image
{

IdentityFunctor WindowConfig::idFtor;
/*

void Window::setSrc(const Image &image){
	src.setView(image);
	srcWidth = image.getWidth();
	srcHeight = image.getHeight();
	coordinateHandler.setLimits(srcWidth, srcHeight);
	if (coordinateHandler.getPolicy().xOverFlowPolicy != CoordinatePolicy::UNDEFINED)
		coordinateHandler.setPolicy(image.getCoordinatePolicy());
	scaleResult = src.getMax<float>()/dst.getMax<float>(); // TODO change order?

}

void Window::setDst(Image &image){
	dst.setView(image);
	scaleResult = src.getMax<float>()/dst.getMax<float>();
}

// FIXME  VIRHE TÄSSÄ?
void Window::setSrcWeight(const Image &image){
	srcWeight.setView(image);
}

void Window::setDstWeight(Image & image){
	dstWeight.setView(image);
}

void Window::toStream(std::ostream &ostr) const {
	ostr << "Window: " << width << 'x' <<  height << ' ';
	ostr << '[' << iMin << ',' << jMin << ',' << iMax << ',' << jMax << ']';
	//ostr << " within " << srcWidth << 'x' <<  srcHeight << '\n';
	ostr << '@' << location << '\n';
	ostr << coordinateHandler << '\n';
	ostr << "src: " << src << '\n';
	ostr << "dst: " << dst << '\n';
	ostr << "srcW:" << srcWeight << '\n';
	ostr << "dstW:" << dstWeight << '\n';
}
*/








}

}


// Drain
