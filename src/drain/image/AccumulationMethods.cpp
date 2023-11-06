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

#include <drain/image/ImageFile.h>  // debugging
#include "drain/util/Log.h"
//#include "drain/util/DataScaling.h"
#include "drain/util/Type.h"
#include "drain/util/TypeUtils.h"

#include "AccumulationMethods.h"




/** See also radar::Composite
 * 
 */
namespace drain
{

namespace image
{

AccMethodBank & getAccMethodBank() {

	static AccMethodBank bank;

	if (bank.getMap().empty()){

		// Double: perhaps first retrieved with "", but then cloned with key "IdentityFunctor".
		bank.add<AccumulationMethod>(); // Unset/undefined
		bank.add<MaximumMethod>();
		bank.add<MinimumMethod>();
		bank.add<AverageMethod>();
		bank.add<WeightedAverageMethod>();
		bank.add<MaximumWeightMethod>();
		bank.add<OverwriteMethod>();
		bank.add<OverwriteMethod>("OVERWRITE"); // "alias"

	}

	return bank;

}

// TODO 2023; crop
void AccumulationMethod::initDstOLD(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	if (crop.upperRight.x < crop.lowerLeft.x){
		mout.error("Negative direction in crop image i coordinates: ", crop);
	}

	if (crop.upperRight.y < crop.lowerLeft.y){
		mout.error("Negative direction in crop image j coordinates: ", crop);
	}


	if (!dst.typeIsSet()){
		if (!coder.type.empty())
			dst.setType(coder.type.at(0));
		else
			throw std::runtime_error(name + "::(AccumulationMethod::_initDst): default output type and image type unset.");
	}

	if (crop.empty()){
		//mout.unimplemented("Crop");
		dst.setGeometry(accArray.getWidth(), accArray.getHeight());
	}
	else {
		mout.experimental("applying cropped (", crop ,") view of ", accArray.getGeometry());
		dst.setGeometry(crop.getWidth(), crop.getHeight());
	}

}

void AccumulationMethod::extractValue(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	// mout.attention(name, " extracting...");

	// Marker for bins having data (count > 0), but unusable value,
	const double noReadingMarker = coder.getNoReadingMarker();
	// const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();

	// Marker for bins without data (count = 0)
	const double noDataMarker    = coder.getNoDataMarker();

	//const drain::image::AreaGeometry & area = dst.getGeometry();
	//const drain::image::AreaGeometry & area = accArray.getGeometry();

	double value;
	double weight;

	if (crop.empty()){
		mout.debug("No cropping, computing in direct mode (quick)");
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			if (accArray.count.at(addr) > 0){
				weight = accArray.weight.at(addr);
				if (weight > 0.0){
					value  = accArray.data.at(addr);
					coder.encode(value, weight);
					dst.put(addr, value);
				}
				else
					dst.put(addr, noReadingMarker);
			}
			else {
				dst.put(addr, noDataMarker);
			}
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				if (accArray.count.at(addr) > 0){
					weight = accArray.weight.at(addr);
					if (weight > 0.0){
						value  = accArray.data.at(addr);
						coder.encode(value, weight);
						dst.put(i, j, value);
					}
					else
						dst.put(i, j, noReadingMarker);
				}
				else {
					dst.put(i, j, noDataMarker);
				}

			}
		}


	}



}



void AccumulationMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	//double value = 0.0;
	double weight;

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t i = 0; i < s; ++i) {
			weight = accArray.weight.at(i);
			coder.encodeWeight(weight);
			dst.put(i, weight);
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				weight = accArray.weight.at(addr);
				coder.encodeWeight(weight);
				dst.put(i, j, weight);
			}
		}
	}

}


void AccumulationMethod::extractCount(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double count;
	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr){
			count = static_cast<double>(accArray.count.at(addr));
			coder.encodeCount(count);
			dst.put(addr, count);
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				count = static_cast<double>(accArray.count.at(addr));
				coder.encodeCount(count);
				dst.put(i, j, count);
			}
		}
	}


}

void AccumulationMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double stdDev;

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr){
			stdDev = static_cast<double>(accArray.count.at(addr));  //???
			coder.encodeStdDev(stdDev);
			dst.put(addr, stdDev);
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				stdDev = static_cast<double>(accArray.count.at(addr));  //???
				coder.encodeStdDev(stdDev);
				dst.put(i, j, stdDev);
			}
		}
	}

}





void OverwriteMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const{

	unsigned int count = ++accArray.count.at(i);

	if (weight > 0.0){
		if (count > 1) {
			// Special feature: the diffence between last and current value is saved in data2.
			accArray.data2.at(i) = accArray.data.at(i) - value;
		}
		accArray.data.at(i)   = value;
		accArray.weight.at(i) = weight;
	}

}

void OverwriteMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double diff;
	const double noData   = coder.getNoDataMarker();

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr){
			switch (accArray.count.at(addr)) {
				case 2:
					diff = static_cast<double>(accArray.data2.at(addr));
					coder.encodeDiff(diff);
					dst.put(addr, diff);
					break;
				case 1:
					// dst.put(i, noData);
					// no break;
				default:
					dst.put(addr, noData);
					break;
			}
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				switch (accArray.count.at(addr)) {
				case 2:
					diff = static_cast<double>(accArray.data2.at(addr));
					coder.encodeDiff(diff);
					dst.put(i,j, diff);
					break;
				case 1:
				default:
					dst.put(i,j, noData);
					break;
				}
			}
		}
	}



}




//void MaximumMethod::add(const size_t i, const double & value, const double & weight) const{
void MaximumMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const {

	// Note:  data initialized to undetectValue, but value may be negative. So, unconditional initialization with count==0.
	// Note2: weight must be non-undetectValue, because undetectValue weight data values may be large/undefined.
	if (weight > 0.0){
		if ( (accArray.weight.at(i) == 0.0) || (value > (accArray.data.at(i))) ){
			accArray.data.at(i)   = value;
			accArray.weight.at(i) = weight;
		}
	}

	++accArray.count.at(i); //  = 1;
}

void MinimumMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const{

	// "Weight control" is needed, because data initialized with undetectValue, but 'value' can be negative.
	if (weight > 0.0)
		if ( (accArray.weight.at(i) == 0.0) || (value < accArray.data.at(i)) ){
			accArray.data.at(i)   = value;
			accArray.weight.at(i) = weight;
		}

	accArray.count.at(i)  = 1;

}


void AverageMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const {

	accArray.count.at(i)  += 1;
	if (weight > 0.0){
		accArray.data.at(i)   += value;
		accArray.weight.at(i) += 1.0; //weight;
		accArray.data2.at(i)  += value*value;
	}

}

void AverageMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight, unsigned int count) const {

	accArray.count.at(i)  += count;
	if (weight > 0.0){
		double c = static_cast<double>(count);
		accArray.data.at(i)   += c*value;
		accArray.weight.at(i) += c;
		accArray.data2.at(i)  += c*value*value;
	}

}


void AverageMethod::extractValue(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	unsigned int count;
	double value;
	double weight;

	// Marker for bins having data (count > 0), but unusable value,
	const double noReadingMarker = coder.getNoReadingMarker();
	// const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();
	const double noDataMarker   = coder.getNoDataMarker();

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			count = accArray.count.at(addr);
			if (count > 0){
				weight = accArray.weight.at(addr);
				if (weight > 0.0){
					value = accArray.data.at(addr) / weight;  // because count is incremented also at undetectValue weight
					// if (accArray.weight.at(i) > 0.0){
					// value = accArray.data.at(i) / static_cast<double>(count);
					weight = 1.0;
					coder.encode(value, weight);  // WEIGHT unused
					dst.put(addr, value );
				}
				else {
					dst.put(addr, noReadingMarker); //minValue);
				}
			}
			else {
				dst.put(addr, noDataMarker);
			}
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				count = accArray.count.at(addr);
				if (count > 0){
					weight = accArray.weight.at(addr);
					if (weight > 0.0){
						value = accArray.data.at(addr) / weight;  // because count is incremented also at undetectValue weight
						// if (accArray.weight.at(i) > 0.0){
						// value = accArray.data.at(i) / static_cast<double>(count);
						weight = 1.0;
						coder.encode(value, weight);  // WEIGHT unused
						dst.put(addr, value );
					}
					else {
						dst.put(i, j, noReadingMarker); //minValue);
					}
				}
				else {
					dst.put(i, j, noDataMarker);
				}
			}
		}
	}


}

void AverageMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double count;
	double weight = 0.0;
	coder.encodeWeight(weight);
	const double weight0 = weight;


	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			count = accArray.count.at(addr);
			if (count > 0.0){
				/// Problem: with undetectValue weight, only counter has been incremented
				weight = accArray.weight.at(addr)/count;
				coder.encodeWeight(weight);
				dst.put(addr, weight );
			}
			else {
				dst.put(addr, weight0);
			}
		}

	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				count = accArray.count.at(addr);
				if (count > 0.0){
					/// Problem: with undetectValue weight, only counter has been incremented
					weight = accArray.weight.at(addr)/count;
					coder.encodeWeight(weight);
					dst.put(i, j, weight );
				}
				else {
					dst.put(i, j, weight0);
				}
			}
		}
	}

}


void AverageMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double count; // actually weight!
	double x = 0.0;
	coder.encodeStdDev(x);
	const double stdDev0 = x;


	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			count = accArray.weight.at(addr);
			if (count > 0.0){
				x = accArray.data.at(addr)/count;
				x = accArray.data2.at(addr)/count - x*x;
				//dst.put(i, scaling.forward(accArray.data2.at(i)/count - value*value) );
				coder.encodeStdDev(x);
				dst.put(addr, x);
			}
			else {
				dst.put(addr, stdDev0);
			}
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				count = accArray.weight.at(addr);
				if (count > 0.0){
					x = accArray.data.at(addr)/count;
					x = accArray.data2.at(addr)/count - x*x;
					//dst.put(i, scaling.forward(accArray.data2.at(i)/count - value*value) );
					coder.encodeStdDev(x);
					dst.put(i, j, x);
				}
				else {
					dst.put(i, j, stdDev0);
				}
			}
		}
	}
}



void WeightedAverageMethod::updateBean() const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (p<0.0){
		mout.error() << "p negative" << mout.endl;
	}
	USE_P = (p > 0.0) && (p != 1.0);
	pInv = USE_P ? 1.0/p : 1.0;

	if (r < 0.0){
		mout.error() << "r negative" << mout.endl;
	}
	USE_R = (r > 0.0) && (r != 1.0);
	rInv  = USE_R ? 1.0/r : 1.0;


}


void WeightedAverageMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const {

	accArray.count.at(i) += 1;

	// NO WEIGHTING if  r==0.0 ( WAVG,p,0 === AVG )
	if (weight > 0.0){

		// NEW 2017: if p==1, allow negative values (and exponent r in weight)
		if (USE_P)
			value = pow(value - bias, p);

		if (USE_R)
			weight = pow(weight, r);
		// else weight = 0.123;
		//weight = 1.0;
		accArray.weight.at(i) += weight; // mean weight will reflect input weights
		// Now, virtually weight=1.0; // = weight^0
		accArray.data.at(i)  += weight*value;
		accArray.data2.at(i) += weight*value*value;
	}
	/// else (r==0, weight==0) just ++count, see above.

}

void WeightedAverageMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight, unsigned int count) const {

	accArray.count.at(i) += count;

	// NO WEIGHTING if  r==0.0 ( WAVG,p,0 === AVG )
	if (weight > 0.0){

		double c = static_cast<double>(count);

		// NEW 2017: if p==1, allow negative values (and exponent r in weight)
		if (USE_P)
			value = pow(value - bias, p);

		if (USE_R)
			weight = pow(weight, r);

		accArray.weight.at(i) += c*weight; // mean weight will reflect input weights
		accArray.data.at(i)   += c*weight*value;
		accArray.data2.at(i)  += c*weight*value*value;
	}
	/// else (r==0, weight==0), and just ++count, see above.

}




void WeightedAverageMethod::extractValue(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	double value;
	double weight;

	const double minWeight = USE_R ? pow(0.0001, r) : 0.0001;
	const double noDataCode   = coder.getNoDataMarker();
	const double minCode = Type::call<typeMin, double>(dst.getType()); //dst.scaling.getMin<double>();

	if (crop.empty()){

		const size_t s = dst.getVolume();

		for (size_t addr = 0; addr < s; ++addr) {

			if (accArray.count.at(addr) > 0){  // use count, not weight! ("undetected" values still "measured", yet with undetectValue weight)
				weight = accArray.weight.at(addr);
				if (weight > minWeight){

					// New scheme:
					value = accArray.data.at(addr) / weight;

					// NEW 2017: if p==1, allow negative values (and exponent r in weight)
					if (USE_P){
						value = pow(value, pInv) + bias;
					}
					coder.encode(value, weight);
					dst.put(addr, value);
				}
				else {
					dst.put(addr, minCode);  // "undetect"
					//dst.put(i, coder.undetectValue);
				}
				//value = dataScaling.forward( accArray.data.at(i)/weight );
			}
			else {
				dst.put(addr, noDataCode);
			}
		}

	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				if (accArray.count.at(addr) > 0){  // use count, not weight! ("undetected" values still "measured", yet with undetectValue weight)

					weight = accArray.weight.at(addr);

					if (weight > minWeight){

						// New scheme:
						value = accArray.data.at(addr) / weight;

						// NEW 2017: if p==1, allow negative values (and exponent r in weight)
						if (USE_P){
							value = pow(value, pInv) + bias;
						}
						//value += bias;  // 2017 moved above
						coder.encode(value, weight);
						dst.put(i, j, value);
					}
					else {
						dst.put(i, j, minCode);  // "undetect"
					}
				}
				else {
					dst.put(addr, noDataCode);
				}

			}
		}
	}

}





void WeightedAverageMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double weight;
	//const double rInv = (USE_R) ? 1.0/r : 1.0;  // div by undetectValue on some compilers?

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			weight = accArray.weight.at(addr);
			// if ((i%1026)==0) std::cerr << "weight: " << weight << '\t';
			if (weight > 0.0){ //
				weight = weight / static_cast<double>(accArray.count.at(addr)); // scale TODO: FIX!
				if (USE_R)
					weight = pow(weight, rInv);
			}
			coder.encodeWeight(weight);
			dst.put(addr, weight);
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				weight = accArray.weight.at(addr);
				if (weight > 0.0){ //
					weight = weight / static_cast<double>(accArray.count.at(addr)); // scale TODO: FIX!
					if (USE_R)
						weight = pow(weight, rInv);
				}
				coder.encodeWeight(weight);
				dst.put(i, j, weight);
			}
		}
	}

}


void WeightedAverageMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	Logger mout(getImgLog(), __FUNCTION__, getName());

	double count;
	double weight;
	double value = 0.0;
	double value2;
	coder.encodeStdDev(value);
	const double stdDev0 = value;

	const double pInv2 = pInv*pInv;

	if (crop.empty()){
		const size_t s = dst.getVolume();
		for (size_t addr = 0; addr < s; ++addr) {
			count = accArray.count.at(addr);
			if (count > 0.0){
				weight = accArray.weight.at(addr);
				if (weight > 0.0){
					if (USE_P) {
						value  = pow(accArray.data.at(addr)/weight, pInv);  // what about bias?
						value2 = pow(accArray.data2.at(addr)/weight, pInv2);
					}
					else {
						value  = accArray.data.at(addr)/weight;
						value2 = accArray.data2.at(addr)/weight;
					}
					value = value2 - value*value;
					coder.encodeStdDev(value);
					dst.put(addr, value);
				}
				else
					dst.put(addr, stdDev0);
			}
			else {
				dst.put(addr, stdDev0);
			}
		}
	}
	else {
		mout.special(" crop:", crop, " dst: ", dst.getGeometry());
		size_t addr;
		for (unsigned int j=0; j<dst.getHeight(); ++j) {
			for (unsigned int i=0; i<dst.getWidth(); ++i) {
				addr = accArray.address(crop.lowerLeft.x+i, crop.upperRight.y+j);
				count = accArray.count.at(addr);
				if (count > 0.0){
					weight = accArray.weight.at(addr);
					if (weight > 0.0){
						if (USE_P) {
							value  = pow(accArray.data.at(addr)/weight, pInv);  // what about bias?
							value2 = pow(accArray.data2.at(addr)/weight, pInv2);
						}
						else {
							value  = accArray.data.at(addr)/weight;
							value2 = accArray.data2.at(addr)/weight;
						}
						value = value2 - value*value;
						coder.encodeStdDev(value);
						dst.put(i, j, value);
					}
					else
						dst.put(i, j, stdDev0);
				}
				else {
					dst.put(i, j, stdDev0);
				}
			}
		}
	}
}





void MaximumWeightMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const {

	if (weight >= accArray.weight.at(i)){
		accArray.data.at(i)   = value;
		accArray.weight.at(i) = weight;
		accArray.count.at(i)  = 1;
	}

	return;
}




}

}

// Drain
