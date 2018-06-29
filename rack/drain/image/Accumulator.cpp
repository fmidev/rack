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

//#include <drain/image/File.h>  // debugging

#include "util/Log.h"
//#include "util/DataScaling.h"
#include "util/Type.h"

// #include "File.h"  // debugging

#include "Accumulator.h"



// TODO: image/
/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{


void Accumulator::setMethod(const std::string & name, const std::string & params){  //const Variable & parameters

	Logger mout("Accumulator",__FUNCTION__);

	if (name == "AVG"){
		mout.warn() << "'AVG' deprecating, using 'AVERAGE'" << mout.endl;
		setMethod("AVERAGE", params);
		return;
	}
	else if (name == "MIN"){
		mout.warn() << "'MIN' deprecating, using 'MINIMUM'" << mout.endl;
		setMethod("MINIMUM", params);
		return;
	}
	else if (name == "MAX"){
		mout.warn() << "'MAX' deprecating, using 'MAXIMUM'" << mout.endl;
		setMethod("MAXIMUM", params);
		return;
	}
	else if (name == "OVERWRITE"){
		mout.warn() << "'OVERWRITE' deprecating, using 'LATEST'" << mout.endl;
		setMethod("LATEST", params);
		return;
	}


	std::map<std::string, AccumulationMethod &>::iterator it = methods.find(name);

	if (it != methods.end()){
		methodPtr = &(it->second);
		methodPtr->setParameters(params);
	}
	else {
		//this->toStream(std::cerr);
		throw std::runtime_error(std::string("Accumulator::setMethod: unknown method: ") + name);
	}

}

void Accumulator::setMethod(const std::string & method){

	const size_t i = method.find(',');

	if (i == std::string::npos)
		setMethod(method, "" );  // Variable()
	else
		setMethod(method.substr(0, i), method.substr(i+1) );  // Variable(method.substr(i+1))

}

void Accumulator::addData(const Image & srcData, const AccumulationConverter & converter, double priorWeight, int iOffset, int jOffset){

	Logger mout("Accumulator",__FUNCTION__);

	const unsigned int width  = srcData.getWidth();
	const unsigned int height = srcData.getHeight();

	mout.debug() << width << 'x' << height << '@' << iOffset << ',' << jOffset << mout.endl;

	size_t a;
	double value;

	double weight;
	converter.encodeWeight(priorWeight);  // important


	mout.debug() << converter << mout.endl;

	Point2D<int> p;
	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			p.setLocation(iOffset+i,jOffset+j);
			if (coordinateHandler.handle(p.x,p.y) == CoordinateHandler2D::UNCHANGED){
				a = srcData.address(i,j);
				value  = srcData.get<double>(a);
				weight = priorWeight;
				if (converter.decode(value, weight)){
					add(data.address(p.x, p.y), value, weight);
				}
				/*  OLD
				if (converter.decode(value)){
					add(data.address(p.x, p.y), value, priorWeight);
				}
				*/
			}
		}
	}
}

void Accumulator::addData(const Image & src, const Image & srcQuality, const AccumulationConverter & converter, double priorWeight, int iOffset, int jOffset){

	const unsigned int width  = src.getWidth();
	const unsigned int height = src.getHeight();

	size_t a;
	double value;
	double weight;
	Point2D<int> p;
	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			p.setLocation(iOffset+i,jOffset+j);
			if (coordinateHandler.handle(p.x,p.y) == CoordinateHandler2D::UNCHANGED){
				a = src.address(i,j);
				value  = src.get<double>(a);
				weight = srcQuality.get<double>(a);
				if (converter.decode(value, weight)){
					add(data.address(p.x, p.y), value, priorWeight * weight);
				}
			}
		}
	}
}



void Accumulator::extractField(char field, const AccumulationConverter & coder, Image & dst) const {

	Logger mout("Accumulator",__FUNCTION__);


	if ((dst.getWidth() != width) || (dst.getHeight() != height)){
		mout.info() << "resizing " << dst.getWidth() << 'x' << dst.getHeight() << " => " << width << 'x' << height << mout.endl;
		dst.setGeometry(width, height);
	}


	if (!dst.typeIsSet())
		dst.setType<unsigned char>();

	mout.debug(1) << "field " << field << mout.endl;

	switch (field){
		case 'd':
			methodPtr->extractValue(coder, dst);
			break;
		case 'w':
			methodPtr->extractWeight(coder, dst);
			break;
		case 'c':
			methodPtr->extractCount(coder, dst);
			break;
		case 's':
			methodPtr->extractDev(coder, dst);
			//methodPtr->extractDev(dst, params.scale, params.bias, params.NODATA);
			break;
		case 'D':
			methodPtr->extractDevInv(coder, dst);
			//methodPtr->extractDev(dst, params.scale, params.bias, params.NODATA);
			break;
		default:
			mout.error() << "unknown (unimplemented) field " << field << mout.endl;
			//throw std::runtime_error(std::string("Accumulator::extractField: unknown field code '") + field + "'");
	}

}



std::ostream & operator<<(std::ostream & ostr, const Accumulator & accumulator){
	accumulator.toStream(ostr);
	//ostr << cumulator.getMethodStr() << '['<< cumulator.getP()  << ',' << cumulator.getR() << ']' << " gain,offset:" << cumulator.getGain() << ',' << cumulator.getOffset() << ' ';
	return ostr;
}

}

}

// Drain
