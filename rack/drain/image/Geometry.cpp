/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
