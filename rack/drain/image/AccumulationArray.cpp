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

//#include <drain/image/File.h>  // debugging

#include "util/Log.h"
//#include "util/LinearScaling.h"
//#include "util/Type.h"

// #include "File.h"  // debugging

#include "AccumulationArray.h"



// TODO: image/
/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{

void AccumulationArray::setGeometry(unsigned int width, unsigned int height){

	this->width = width;
	this->height = height;

	data.setGeometry(width,height);
	weight.setGeometry(width,height);
	count.setGeometry(width,height);
	data2.setGeometry(width,height);

	//coordinateHandler.setBounds(width,height);
	coordinateHandler.setLimits(width,height);
}

void AccumulationArray::clear(){
	data.clear();
	weight.clear();
	count.clear();
	data2.clear();
}




std::ostream & operator<<(std::ostream & ostr, const AccumulationArray & cumulator){
	ostr << "AccumulationArray " << cumulator.getWidth() << 'x' << cumulator.getHeight() << ' ';
	//ostr << cumulator.getMethodStr() << '['<< cumulator.getP()  << ',' << cumulator.getR() << ']' << " gain,offset:" << cumulator.getGain() << ',' << cumulator.getOffset() << ' ';
	return ostr;
}



}

}
