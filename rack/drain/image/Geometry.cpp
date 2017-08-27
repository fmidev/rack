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
	
// using namespace std;
	
// variableMap ei onnistunut (width, height, chC)
/*
Geometry::Geometry() : width(0), height(0), channelCount(0), imageChannelCount(0), alphaChannelCount(0)
{
	setGeometry(0,0,0,0);
}
*/

Geometry::Geometry(size_t width, size_t height, size_t imageChannelCount, size_t alphaChannelCount)
: width(0), height(0), channelCount(0), imageChannelCount(0), alphaChannelCount(0)
{
	setGeometry(width,height,imageChannelCount,alphaChannelCount);
}

Geometry::~Geometry()
{
}

bool Geometry::setGeometry(const Geometry &g){
	return setGeometry(g.getWidth(),g.getHeight(),g.getImageChannelCount(),g.getAlphaChannelCount());
} 


bool Geometry::setGeometry(size_t width, size_t height, size_t imageChannelCount, size_t alphaChannelCount){

	const bool change  = (this->width != width) || (this->height != height) ||
			(this->imageChannelCount != imageChannelCount) || (this->alphaChannelCount != alphaChannelCount);

	this->width = width;
	this->height = height;
	this->imageChannelCount = imageChannelCount;
	this->alphaChannelCount = alphaChannelCount;
	this->channelCount = imageChannelCount + alphaChannelCount;
	update();
	//area = width * height;
	//volume = area*(imageChannelCount + alphaChannelCount);
	return change;
} 
	
void Geometry::setWidth(size_t w){
	//resize(std::max(3u,size()));
	this->width = width;
	//at(0) = w;
	update();
}

void Geometry::setHeight(size_t h){
	//resize(std::max(3u,size()));
	this->height = height;
	//at(1) = h;
	update(); 
}

void Geometry::setChannelCount(size_t imageChannels, size_t alphaChannels){
	
	//resize(std::max(3u,size()));
	this->imageChannelCount = imageChannels;
	this->alphaChannelCount = alphaChannels;
	this->channelCount = imageChannels + alphaChannels;
	//at(2) = imageChannels + alphaChannels;
	update();
}


void Geometry::setAlphaChannelCount(size_t alphaChannels){
	setChannelCount(getImageChannelCount(),alphaChannels);
}


void Geometry::update(){
	
	//resize(std::max(3u,size()));
	
	//width = at(0);
	//height = at(1);
	//channelCount = at(2);
	
	imageChannelCount = channelCount - alphaChannelCount; 
	area = width*height;
	volume = area * channelCount;
}


std::ostream & operator<<(std::ostream &ostr,const Geometry &geometry) {

	std::string separator("");
	
	ostr << geometry.getWidth() << "×" << geometry.getHeight() << "×";
	ostr << '(' << geometry.getImageChannelCount() << '+' << geometry.getAlphaChannelCount() << ')';
	//ostr << "w=" << geometry.getWidth() << ", h=" <<  geometry.getHeight()  << ", ";
	//ostr << "iC=" << geometry.getImageChannelCount() << ", aC=" <<  geometry.getAlphaChannelCount()  << ", ";
    //ostr << " A=" << geometry.getArea() << ", V=" <<  geometry.getVolume()  << " ";
		
	return ostr;
}
 
std::string &Geometry::toString(std::string & s) const
{
    std::stringstream sstr;
	sstr << *this;
	s = sstr.str();
    return s;
}


}

}

// Drain
