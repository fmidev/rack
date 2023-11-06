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

#include "drain/util/Log.h"
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

void AccumulationArray::setGeometry(size_t width, size_t height){

	geometry.set(width, height);

	data.setGeometry(geometry);
	weight.setGeometry(geometry);
	count.setGeometry(geometry);
	data2.setGeometry(geometry);

	//coordinateHandler.setBounds(width,height);
	coordinateHandler.setLimits(width,height);
}

void AccumulationArray::clear(){
	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	data.clear();
	weight.clear();
	count.clear();
	data2.clear();
}




std::ostream & operator<<(std::ostream & ostr, const AccumulationArray & accArray){
	ostr << "AccumulationArray geom: " << accArray.getGeometry() << ' ';
	return ostr;
}



}

}

// Drain
