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
void AccumulationMethod::initDst(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

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

	mout.debug(name, " extracting...");

	initDst(accArray, coder, dst, crop);


	// Marker for bins having data (count > 0), but unusable value,
	const double noReadingMarker = coder.getNoReadingMarker();
	// const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();

	// Marker for bins without data (count = 0)
	const double noDataMarker    = coder.getNoDataMarker();

	//const drain::image::AreaGeometry & area = dst.getGeometry();
	const drain::image::AreaGeometry & area = accArray.getGeometry();
	if (area.height == 0){

	}

	double value;
	double weight;
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
		else
			dst.put(addr, noDataMarker);

	}
}



void AccumulationMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	initDst(accArray, coder, dst, crop);

	//double value = 0.0;
	double weight;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		//value  = accArray.data.at(i);
		weight = accArray.weight.at(i);
		coder.encodeWeight(weight);
		dst.put(i, weight);
	}
}


void AccumulationMethod::extractCount(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	//LinearScaling scaling(gain, offset);
	initDst(accArray, coder, dst, crop);

	double count;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		count = static_cast<double>(accArray.count.at(i));
		coder.encodeCount(count);
		dst.put(i, count);
	}

}

void AccumulationMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	double stdDev;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		stdDev = static_cast<double>(accArray.count.at(i));  //???
		coder.encodeStdDev(stdDev);
		dst.put(i, stdDev);
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

	initDst(accArray, coder, dst, crop);

	double diff;
	const double noData   = coder.getNoDataMarker();
	//coder.
	//unsigned int count = 0;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		switch (accArray.count.at(i)) {
			case 2:
				diff = static_cast<double>(accArray.data2.at(i));
				coder.encodeDiff(diff);
				dst.put(i, diff);
				break;
			case 1:
				// dst.put(i, diff);
				dst.put(i, noData);
				break;
			default:
				dst.put(i, noData);
				break;
		}
		/*
		if (accArray.count.at(i) > 0){
		  diff = static_cast<double>(accArray.data2.at(i));
		  coder.encodeDiff(diff);
		  dst.put(i, diff);
		}
		else {
			dst.put(i, noData);
		}
		*/
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

	initDst(accArray, coder, dst, crop);

	unsigned int count;
	double value;
	double weight;

	// Marker for bins having data (count > 0), but unusable value,
	const double noReadingMarker = coder.getNoReadingMarker();
	// const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();
	const double noDataMarker   = coder.getNoDataMarker();


	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accArray.count.at(i);
		if (count > 0){
			weight = accArray.weight.at(i);
			if (weight > 0.0){
				value = accArray.data.at(i) / weight;  // because count is incremented also at undetectValue weight
				// if (accArray.weight.at(i) > 0.0){
				// value = accArray.data.at(i) / static_cast<double>(count);
				weight = 1.0;
				coder.encode(value, weight);  // WEIGHT unused
				dst.put(i, value );
			}
			else {
				dst.put(i, noReadingMarker); //minValue);
			}
		}
		else
			dst.put(i, noDataMarker);

	}
}

void AverageMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	initDst(accArray, coder, dst, crop);

	double count;
	double weight = 0.0;
	coder.encodeWeight(weight);
	const double weight0 = weight;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accArray.count.at(i);
		if (count > 0.0){
			/// Problem: with undetectValue weight, only counter has been incremented
			weight = accArray.weight.at(i)/count;
			coder.encodeWeight(weight);
			dst.put(i, weight );
		}
		else
			dst.put(i, weight0);

	}

}


void AverageMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	initDst(accArray, coder, dst, crop);

	double count; // actually weight!
	double x = 0.0;
	coder.encodeStdDev(x);
	const double stdDev0 = x;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accArray.weight.at(i);
		if (count > 0.0){
			x = accArray.data.at(i)/count;
			x = accArray.data2.at(i)/count - x*x;
			//dst.put(i, scaling.forward(accArray.data2.at(i)/count - value*value) );
			coder.encodeStdDev(x);
			dst.put(i, x);
		}
		else {
			dst.put(i, stdDev0);
		}

	}

}



void WeightedAverageMethod::updateBean() const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

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

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	// mout.warn() << " start..." << mout.endl;

	initDst(accArray, coder, dst, crop);

	double value;
	double weight;

	const double minWeight = USE_R ? pow(0.0001, r) : 0.0001;
	const double noDataCode   = coder.getNoDataMarker();
	const double minCode = Type::call<typeMin, double>(dst.getType()); //dst.scaling.getMin<double>();

	const size_t s = dst.getVolume();

	for (size_t i = 0; i < s; ++i) {

		if (accArray.count.at(i) > 0){  // use count, not weight! ("undetected" values still "measured", yet with undetectValue weight)

			weight = accArray.weight.at(i);

			if (weight > minWeight){

				// New scheme:
				value = accArray.data.at(i) / weight;

				// NEW 2017: if p==1, allow negative values (and exponent r in weight)
				if (USE_P){
					value = pow(value, pInv) + bias;
				}
				//value += bias;  // 2017 moved above

				coder.encode(value, weight);
				dst.put(i, value);
			}
			else {
				dst.put(i, minCode);  // "undetect"
				//dst.put(i, coder.undetectValue);
			}
			//value = dataScaling.forward( accArray.data.at(i)/weight );
		}
		else {
			dst.put(i, noDataCode);
		}

		// if ((i%WIDTH)==j) std::cerr << "(" << value << ',' << weight << "\n";

		// type.limit(value);  // needed?
		//dst.put(i, value);


	}
}





void WeightedAverageMethod::extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	initDst(accArray, coder, dst, crop);

	double weight;

	//const double rInv = (USE_R) ? 1.0/r : 1.0;  // div by undetectValue on some compilers?

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		//count = accArray.count.at(i);
		weight = accArray.weight.at(i);
		// if ((i%1026)==0) std::cerr << "weight: " << weight << '\t';
		if (weight > 0.0){ // 001){

			// scale
			// TODO: FIX!
			weight = weight / static_cast<double>(accArray.count.at(i));

			if (USE_R)
				weight = pow(weight, rInv);
		}
		// if ((i%1026)==0) std::cerr << weight << '\t';
		coder.encodeWeight(weight);
		// if ((i%1026)==0) std::cerr << weight << '\n';
		dst.put(i, weight);
	}

}


void WeightedAverageMethod::extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const {

	initDst(accArray, coder, dst, crop);

	double count;
	double weight;
	double value = 0.0;
	double value2;
	coder.encodeStdDev(value);
	const double stdDev0 = value;

	const double pInv2 = pInv*pInv;


	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accArray.count.at(i);
		if (count > 0.0){

			weight = accArray.weight.at(i);
			if (weight > 0.0){

				if (USE_P) {
					value  = pow(accArray.data.at(i)/weight, pInv);  // what about bias?
					value2 = pow(accArray.data2.at(i)/weight, pInv2);
				}
				else {
					value  = accArray.data.at(i)/weight;
					value2 = accArray.data2.at(i)/weight;
				}

				value = value2 - value*value;
				coder.encodeStdDev(value);
				dst.put(i, value);

			}
			else
				dst.put(i, stdDev0);
		}
		else {
			dst.put(i, stdDev0);
		}

	}

}


void MaximumWeightMethod::add(AccumulationArray & accArray, const size_t i, double value, double weight) const {
	//void MaximumWeightMethod::add(const size_t i, const double & value, const double & weight) const {

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
