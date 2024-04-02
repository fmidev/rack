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

//#include "drain/image/File.h"  // debugging

#include <drain/Log.h>
#include <drain/Type.h>
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

	//Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	Logger mout(__FILE__, __FUNCTION__);

	if (name == "AVG"){
		mout.deprecating("'AVG' => using 'AVERAGE'");
		setMethod("AVERAGE", params);
		return;
	}
	else if (name == "MIN"){
		mout.deprecating("'MIN' => using 'MINIMUM'" );
		setMethod("MINIMUM", params);
		return;
	}
	else if (name == "MAX"){
		mout.deprecating("'MAX' => using 'MAXIMUM'" );
		setMethod("MAXIMUM", params);
		return;
	}
	else if (name == "OVERWRITE"){
		mout.note("'OVERWRITE' => using 'LATEST'" );
		setMethod("LATEST", params);
		return;
	}


	AccMethodBank & bank = getAccMethodBank();

	try {
		// Basically, could be cloned every time, but no use if no parameters
		methodPtr = & bank.get(name);
		if (methodPtr->hasParameters()){
			mout.debug("cloning!");
			mout.debug2("params orig. ", methodPtr->getParameters());
			methodPtr = & bank.clone(name);
			mout.debug2("params cloned ", methodPtr->getParameters());
			methodPtr->setParameters(params);
			mout.debug2("params modif. ", methodPtr->getParameters());
		}
	}
	catch (const std::exception & e) {
		//mout.attention(bank.getKeys());
		mout.note("Use: ", drain::sprinter(bank.getKeys(), "|"));
		mout.error("unknown method: ", name);
	}

	//return *methodPtr;


}

void Accumulator::setMethod(const std::string & method){

	if (method.empty())
		return ;

	std::string s1, s2;
	drain::StringTools::split2(method, s1, s2, ',');
	setMethod(s1, s2);

	/*
	const size_t i = method.find(',');
	if (i == std::string::npos)
		return setMethod(method, "" );  // Variable()
	else
		return setMethod(method.substr(0, i), method.substr(i+1) );  // Variable(method.substr(i+1))
	*/
}

void Accumulator::addData(const Image & srcData, const AccumulationConverter & converter, double priorWeight, int iOffset, int jOffset){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const unsigned int width  = srcData.getWidth();
	const unsigned int height = srcData.getHeight();
	// mout.debug(width , 'x' , height , '@' , iOffset , ',' , jOffset );
	size_t a;
	double value;
	double weight;
	converter.encodeWeight(priorWeight);  // important
	//mout.warn("converter: " , converter );
	Point2D<int> p;
	const CoordinateHandler2D & coordHandler = accArray.getCoordinateHandler();

	// mout.attention("coordHandler: ", coordHandler);

	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			p.setLocation(iOffset+i,jOffset+j);
			if (coordHandler.handle(p.x,p.y) == CoordinateHandler2D::UNCHANGED){
				a = srcData.address(i,j);
				value  = srcData.get<double>(a);
				weight = priorWeight;
				if (converter.decode(value, weight)){
					add(accArray.data.address(p.x, p.y), value, weight);
				}
			}
		}
	}
}

void Accumulator::addData(const Image & src, const Image & srcQuality, const AccumulationConverter & converter, double priorWeight, int iOffset, int jOffset){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const unsigned int width  = src.getWidth();
	const unsigned int height = src.getHeight();
	size_t a;
	double value;
	double weight;
	Point2D<int> p;
	const CoordinateHandler2D & coordHandler = accArray.getCoordinateHandler();

	//mout.attention("coordHandler: ", coordHandler);


	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			p.setLocation(iOffset+i,jOffset+j);
			if (coordHandler.handle(p.x,p.y) == CoordinateHandler2D::UNCHANGED){
				a = src.address(i,j);
				value  = src.get<double>(a);
				weight = srcQuality.get<double>(a);
				if (converter.decode(value, weight)){
					add(accArray.data.address(p.x, p.y), value, priorWeight * weight);
				}
			}
		}
	}
}

void Accumulator::addData(const Image & src, const Image & srcQuality, const Image & srcCount, const AccumulationConverter & converter){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const unsigned int width  = src.getWidth();
	const unsigned int height = src.getHeight();
	size_t a;
	double value;
	double weight;

	mout.unimplemented("count probably NOT taken into account");

	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {
			a = src.address(i,j);
			value  = src.get<double>(a);
			weight = srcQuality.get<double>(a);
			if (converter.decode(value, weight)){
				add(a, value, weight, srcCount.get<unsigned int>(a));
			}
		}
	}
}


void Accumulator::extractField(char field, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);


	// mout.attention("Crop: ", crop);
	drain::Rectangle<int> finalCrop(crop);
	initDst(coder, dst, finalCrop);

	if (crop != finalCrop){
		mout.note("final crop [LL UR]: ", finalCrop, ", dst geom:", dst.getGeometry());
	}


	/*
	if ((dst.getWidth() != accArray.getWidth()) || (dst.getHeight() != accArray.getHeight())){
		mout.attention("Deprecating code - resize handled better by initDst");
		mout.info("resizing ", dst.getGeometry(), " [2D] => ", accArray.getGeometry());
		//dst.setGeometry(accArray.getWidth(), accArray.getHeight());

		dst.setGeometry(accArray.getGeometry());
	}
	*/


	// Storage type selection for fields str than 'data'. (???)
	if (!dst.typeIsSet()){
		if ((field >= 'a') && (field <= 'z'))
			dst.setType<unsigned char>();
		else {
			dst.setType<double>();
			// field = field-'A'+'a';
		}
	}

	mout.debug2("field ", field);

	switch (field){
		case 'd':
		case 'D':
			methodPtr->extractValue(accArray, coder, dst, finalCrop);
			break;
		case 'w':
		case 'W':
			methodPtr->extractWeight(accArray, coder, dst, finalCrop);
			break;
		case 'c':
		case 'C':
			methodPtr->extractCount(accArray, coder, dst, finalCrop);
			break;
		case 's':
		case 'S':
			//mout.warn(coder );
			methodPtr->extractDev(accArray, coder, dst, finalCrop);
			//methodPtr->extractDev(dst, params.scale, params.bias, params.NODATA);
			break;
		default:
			mout.error("unknown (unimplemented) field ", field);
			//throw std::runtime_error(std::string("Accumulator::extractField: unknown field code '") + field + "'");
	}

}

void Accumulator::initDst(const AccumulationConverter & coder, Image & dst, drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (crop.upperRight.x < crop.lowerLeft.x){
		mout.error("Negative direction in vertical crop coordinate (lowerLeft upperRight): ", crop);
	}

	if (crop.upperRight.y > crop.lowerLeft.y){
		mout.error("Positive direction in vertical crop coordinate (lowerLeft upperRight): ", crop);
	}

	/*
	//mout.attention("Crop image coords: ", cropImage);
	if (cropImage.isInside(-1, 0) || cropImage.isInside(0, -1)){
		mout.error("Crop area ", cropImage, " exceeds composite ");
	}
	if (cropImage.isInside(composite.getFrameWidth(), 0) || cropImage.isInside(0, composite.getFrameHeight())){
		mout.error("Crop area ", cropImage, " exceeds composite ");
	}
	*/

	if (!dst.typeIsSet()){
		if (!coder.type.empty()){
			dst.setType(coder.type.at(0));
		}
		else {
			mout.error("default output type and image type unset.");
		}
	}


	if (crop.empty()){
		mout.debug("Empty crop request - using whole array");
		dst.setGeometry(accArray.getWidth(), accArray.getHeight());
		// return false
	}
	else {
		Rectangle<int> finalCrop(0, accArray.getHeight()-1, accArray.getWidth()-1, 0);

		if (crop == finalCrop){
			mout.warn("Crop area equals array scope (", finalCrop, "), discarding it.");
			crop.clear();
			dst.setGeometry(accArray.getWidth(), accArray.getHeight());
			return;
		}

		finalCrop.crop(crop);

		if (finalCrop != crop){
			mout.warn("Adjusted crop area: ", crop, " -> ", finalCrop);
		}
		crop.set(finalCrop);
		mout.experimental("applying cropped (", crop ,") view of ", accArray.getGeometry());
		const int w = ::abs(crop.getWidth());
		const int h = ::abs(crop.getHeight());
		dst.setGeometry(w+1, h+1);
		// return true
	}

}


std::ostream & Accumulator::toStream(std::ostream & ostr) const {
//std::ostream & operator<<(std::ostream & ostr, const Accumulator & accumulator){

	ostr << "Accumulator ("<< accArray.getGeometry() << ") ";
	ostr << " ["<< getMethod() << "] ";
	/*
	for (std::map<std::string, AccumulationMethod &>::const_iterator it = methods.begin(); it != methods.end(); it++)
		ostr << it->second << ',';
	*/
	//ostr << '\n';
	//return ostr;

	//accumulator.toStream(ostr);
	//ostr << cumulator.getMethodStr() << '['<< cumulator.getP()  << ',' << cumulator.getR() << ']' << " gain,offset:" << cumulator.getGain() << ',' << cumulator.getOffset() << ' ';
	return ostr;
}

}

}

// Drain
