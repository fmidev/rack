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

#include <algorithm>
#include <sstream>

#include "Geometry.h"
//#include "DistanceTransformOp.h"

namespace drain
{

namespace image
{


void AreaGeometry::setWidth(size_t w){
	width = w;
	update();
}

void AreaGeometry::setHeight(size_t h){
	height = h;
	update();
}

void AreaGeometry::setArea(size_t w, size_t h){
	width = w;
	height = h;
	update();
}

void AreaGeometry::update(){
	area = width * height;
}


void ChannelGeometry::setChannelCount(size_t i, size_t a){
	imageChannelCount = i;
	alphaChannelCount = a;
	update();
}


void ChannelGeometry::setAlphaChannelCount(size_t a){
	alphaChannelCount = a;
	update();
}

void ChannelGeometry::update(){
	channelCount = imageChannelCount+alphaChannelCount;
}

// using namespace std;
	
// variableMap ei onnistunut (width, height, chC)
/*
Geometry::Geometry() : width(0), height(0), channelCount(0), imageChannelCount(0), alphaChannelCount(0)
{
	setGeometry(0,0,0,0);
}
*/

Geometry::Geometry(size_t width, size_t height, size_t imageChannelCount, size_t alphaChannelCount)
{
	setGeometry(width, height, imageChannelCount, alphaChannelCount);
}

Geometry::Geometry(const Geometry & g){
	setGeometry(g);
}

Geometry::~Geometry(){
}



void Geometry::setGeometry(size_t width, size_t height, size_t imageChannelCount, size_t alphaChannelCount){

	setArea(width, height);
	setChannelCount(imageChannelCount, alphaChannelCount);
	//const bool change  = (this->width != width) || (this->height != height) ||
	//		(this->imageChannelCount != imageChannelCount) || (this->alphaChannelCount != alphaChannelCount);

	/*
	this->width = width;
	this->height = height;
	this->imageChannelCount = imageChannelCount;
	this->alphaChannelCount = alphaChannelCount;
	this->channelCount = imageChannelCount + alphaChannelCount;
	*/
	update();
	//return change;
} 
	





void Geometry::update(){
	AreaGeometry::update();
	ChannelGeometry::update();
	volume = area * channelCount;
}


/*
std::ostream & operator<<(std::ostream &ostr, const Geometry & g) {

	
	ostr << g.getWidth() << "×" << g.getHeight();
	if (g.getChannelCount() != 1){
		ostr << "×";
		if (g.getAlphaChannelCount() > 0){
			ostr << '(' << g.getImageChannelCount() << '+' << g.getAlphaChannelCount() << ')';
		}
		else {
			ostr << g.getImageChannelCount();
		}
	}
	return ostr;
}
*/
 
std::string &Geometry::toString(std::string & s) const
{
    std::stringstream sstr;
    toOStr(sstr);
	//sstr << *this;
	s = sstr.str();
    return s;
}


}

}

// Drain
