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
#include "util/TypeUtils.h"

#include "File.h"  // debugging

#include "AccumulationMethods.h"




/** See also radar::Composite
 * 
 */
namespace drain
{

namespace image
{

void AccumulationMethod::initDst(const AccumulationConverter & coder, Image & dst) const {

	if (!dst.typeIsSet()){
		if (!coder.type.empty())
			dst.setType(coder.type.at(0));
		else
			throw std::runtime_error(name + "::(AccumulationMethod::_initDst): default output type and image type unset.");
	}
	dst.setGeometry(accumulationArray.getWidth(), accumulationArray.getHeight());

}

void AccumulationMethod::extractValue(const AccumulationConverter & coder, Image & dst) const {

	Logger mout("AccumulationMethod", __FUNCTION__);

	mout.debug() << name <<  " extracting..." << mout.endl;

	initDst(coder, dst);

	const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();
	const double noData   = coder.getNoDataMarker();

	double value;
	double weight;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {
		if (accumulationArray.count.at(i) > 0){
			weight = accumulationArray.weight.at(i);
			if (weight > 0.0){
				value  = accumulationArray.data.at(i);
				coder.encode(value, weight);
				dst.put(i, value);
			}
			else
				dst.put(i, minValue);
		}
		else
			dst.put(i, noData);

	}
}



void AccumulationMethod::extractWeight(const AccumulationConverter & coder, Image & dst) const {

	initDst(coder, dst);

	//double value = 0.0;
	double weight;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		//value  = accumulationArray.data.at(i);
		weight = accumulationArray.weight.at(i);
		coder.encodeWeight(weight);
		dst.put(i, weight);
	}
}


void AccumulationMethod::extractCount(const AccumulationConverter & coder, Image & dst) const {

	//LinearScaling scaling(gain, offset);

	double count;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		count = static_cast<double>(accumulationArray.count.at(i));
		coder.encodeCount(count);
		dst.put(i, count);
	}

}

void AccumulationMethod::extractDev(const AccumulationConverter & coder, Image & dst) const {

	double stdDev;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		stdDev = static_cast<double>(accumulationArray.count.at(i));  //???
		coder.encodeStdDev(stdDev);
		dst.put(i, stdDev);
	}

}





void OverwriteMethod::add(const size_t i, double value, double weight) const{
	//coder.decode(value, weight);
	//double value = vField;
	//double weight = w;
	//if (w > 0.0){
	unsigned int count = ++accumulationArray.count.at(i);
	if (weight > 0.0){
		if (count > 1) {
			// Special feature: the diffence between last and current value is saved in data2.
			accumulationArray.data2.at(i) = accumulationArray.data.at(i) - value;
		}
		accumulationArray.data.at(i)   = value;
		accumulationArray.weight.at(i) = weight;
	}
	//}


}

void OverwriteMethod::extractDev(const AccumulationConverter & coder, Image & dst) const {

	double diff;
	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i){
		//if (accumulationArray.count.at(i) > 0){
		diff = static_cast<double>(accumulationArray.data2.at(i));
		coder.encodeDiff(diff);
		dst.put(i, diff);
		//}
	}

}




//void MaximumMethod::add(const size_t i, const double & value, const double & weight) const{
void MaximumMethod::add(const size_t i, double value, double weight) const {

	// Note:  data initialized to undetectValue, but value may be negative. So, unconditional initialization with count==0.
	// Note2: weight must be non-undetectValue, because undetectValue weight data values may be large/undefined.
	if (weight > 0.0){
		if ( (accumulationArray.weight.at(i) == 0.0) || (value > (accumulationArray.data.at(i))) ){
			accumulationArray.data.at(i)   = value;
			accumulationArray.weight.at(i) = weight;
		}
	}

	++accumulationArray.count.at(i); //  = 1;
}

void MinimumMethod::add(const size_t i, double value, double weight) const{

	// "Weight control" is needed, because data initialized with undetectValue, but 'value' can be negative.
	if (weight > 0.0)
		if ( (accumulationArray.weight.at(i) == 0.0) || (value < accumulationArray.data.at(i)) ){
			accumulationArray.data.at(i)   = value;
			accumulationArray.weight.at(i) = weight;
		}

	accumulationArray.count.at(i)  = 1;

}


void AverageMethod::add(const size_t i, double value, double weight) const {

	accumulationArray.count.at(i)  += 1;
	if (weight > 0.0){
		accumulationArray.data.at(i)   += value;
		accumulationArray.weight.at(i) += 1.0; //weight;
		accumulationArray.data2.at(i)  += value*value;
	}

}

void AverageMethod::add(const size_t i, double value, double weight, unsigned int count) const {

	accumulationArray.count.at(i)  += count;
	if (weight > 0.0){
		double c = static_cast<double>(count);
		accumulationArray.data.at(i)   += c*value;
		accumulationArray.weight.at(i) += c;
		accumulationArray.data2.at(i)  += c*value*value;
	}

}


void AverageMethod::extractValue(const AccumulationConverter & coder, Image & dst) const {

	initDst(coder, dst);

	unsigned int count;
	double value;
	double weight;

	const double minValue = Type::call<typeMin, double>(dst.getType()); // dst.scaling.getMin<double>();
	const double noData   = coder.getNoDataMarker();


	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accumulationArray.count.at(i);
		if (count > 0){
			weight = accumulationArray.weight.at(i);
			if (weight > 0.0){
				value = accumulationArray.data.at(i) / weight;  // because count is incremented also at undetectValue weight
				// if (accumulationArray.weight.at(i) > 0.0){
				// value = accumulationArray.data.at(i) / static_cast<double>(count);
				weight = 1.0;
				coder.encode(value, weight);  // WEIGHT unused
				dst.put(i, value );
			}
			else {
				dst.put(i, minValue);
			}
		}
		else
			dst.put(i, noData);

	}
}

void AverageMethod::extractWeight(const AccumulationConverter & coder, Image & dst) const {

	//const LinearScaling scaling(gain, offset);

	double count;
	double weight = 0.0;
	coder.encodeWeight(weight);
	const double weight0 = weight;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accumulationArray.count.at(i);
		if (count > 0.0){
			/// Problem: with undetectValue weight, only counter has been incremented
			weight = accumulationArray.weight.at(i)/count;
			coder.encodeWeight(weight);
			dst.put(i, weight );
		}
		else
			dst.put(i, weight0);

	}

}


void AverageMethod::extractDev(const AccumulationConverter & coder, Image & dst) const {

	double count; // actually weight!
	double x = 0.0;
	coder.encodeStdDev(x);
	const double stdDev0 = x;

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accumulationArray.weight.at(i);
		if (count > 0.0){
			x = accumulationArray.data.at(i)/count;
			x = accumulationArray.data2.at(i)/count - x*x;
			//dst.put(i, scaling.forward(accumulationArray.data2.at(i)/count - value*value) );
			coder.encodeStdDev(x);
			dst.put(i, x);
		}
		else {
			dst.put(i, stdDev0);
		}

	}

}



//void WeightedAverageMethod::setParameters(const Variable & parameters){
void WeightedAverageMethod::updateInternalParameters(){ //const std::string & params){

	/// AccumulationMethod::setParameters(params);

	Logger mout(name, __FUNCTION__);

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

	/*
	USE_P = (p != 1.0);
	pInv = (p > 0.0) ? 1.0/p : 1.0;

	USE_R = (r > 0.0); // and (r != 1.0)
	rInv  = (r > 0.0) ? 1.0/r : 1.0;
	 */
	//this->parameters = parameters;

	//std::cerr << *this << std::endl;
}


void WeightedAverageMethod::add(const size_t i, double value, double weight) const {

	accumulationArray.count.at(i) += 1;

	// NO WEIGHTING if  r==0.0 ( WAVG,p,0 === AVG )
	if (weight > 0.0){

		// NEW 2017: if p==1, allow negative values (and exponent r in weight)
		if (USE_P)
			value = pow(value - bias, p);

		if (USE_R)
			weight = pow(weight, r);
		// else weight = 0.123;
		//weight = 1.0;
		accumulationArray.weight.at(i) += weight; // mean weight will reflect input weights
		// Now, virtually weight=1.0; // = weight^0
		accumulationArray.data.at(i)  += weight*value;
		accumulationArray.data2.at(i) += weight*value*value;
	}
	/// else (r==0, weight==0) just ++count, see above.

}

void WeightedAverageMethod::add(const size_t i, double value, double weight, unsigned int count) const {

	accumulationArray.count.at(i) += count;

	// NO WEIGHTING if  r==0.0 ( WAVG,p,0 === AVG )
	if (weight > 0.0){

		double c = static_cast<double>(count);

		// NEW 2017: if p==1, allow negative values (and exponent r in weight)
		if (USE_P)
			value = pow(value - bias, p);

		if (USE_R)
			weight = pow(weight, r);

		accumulationArray.weight.at(i) += c*weight; // mean weight will reflect input weights
		accumulationArray.data.at(i)   += c*weight*value;
		accumulationArray.data2.at(i)  += c*weight*value*value;
	}
	/// else (r==0, weight==0), and just ++count, see above.

}




void WeightedAverageMethod::extractValue(const AccumulationConverter & coder, Image & dst) const {

	Logger mout(name, __FUNCTION__);
	// mout.warn() << " start..." << mout.endl;

	initDst(coder, dst);

	double value;
	double weight;

	const double minWeight = USE_R ? pow(0.0001, r) : 0.0001;
	const double noDataCode   = coder.getNoDataMarker();
	const double minCode = Type::call<typeMin, double>(dst.getType()); //dst.scaling.getMin<double>();

	const size_t s = dst.getVolume();

	for (size_t i = 0; i < s; ++i) {

		if (accumulationArray.count.at(i) > 0){  // use count, not weight! ("undetected" values still "measured", yet with undetectValue weight)

			weight = accumulationArray.weight.at(i);

			if (weight > minWeight){

				// New scheme:
				value = accumulationArray.data.at(i) / weight;

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
			//value = dataScaling.forward( accumulationArray.data.at(i)/weight );
		}
		else {
			dst.put(i, noDataCode);
		}

		// if ((i%WIDTH)==j) std::cerr << "(" << value << ',' << weight << "\n";

		// type.limit(value);  // needed?
		//dst.put(i, value);


	}
}





void WeightedAverageMethod::extractWeight(const AccumulationConverter & coder, Image & dst) const {

	double weight;

	//const double rInv = (USE_R) ? 1.0/r : 1.0;  // div by undetectValue on some compilers?

	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		//count = accumulationArray.count.at(i);
		weight = accumulationArray.weight.at(i);
		// if ((i%1026)==0) std::cerr << "weight: " << weight << '\t';
		if (weight > 0.0){ // 001){

			// scale
			weight = weight / static_cast<double>(accumulationArray.count.at(i));

			if (USE_R)
				weight = pow(weight, rInv);
		}
		// if ((i%1026)==0) std::cerr << weight << '\t';
		coder.encodeWeight(weight);
		// if ((i%1026)==0) std::cerr << weight << '\n';
		dst.put(i, weight);
	}

}


void WeightedAverageMethod::extractDev(const AccumulationConverter & coder, Image & dst) const {

	double count;
	double weight;
	double value = 0.0;
	double value2;
	coder.encodeStdDev(value);
	const double stdDev0 = value;

	const double pInv2 = pInv*pInv;


	const size_t s = dst.getVolume();
	for (size_t i = 0; i < s; ++i) {

		count = accumulationArray.count.at(i);
		if (count > 0.0){

			weight = accumulationArray.weight.at(i);
			if (weight > 0.0){

				if (USE_P) {
					value  = pow(accumulationArray.data.at(i)/weight, pInv);  // what about bias?
					value2 = pow(accumulationArray.data2.at(i)/weight, pInv2);
				}
				else {
					value  = accumulationArray.data.at(i)/weight;
					value2 = accumulationArray.data2.at(i)/weight;
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


void MaximumWeightMethod::add(const size_t i, double value, double weight) const {
	//void MaximumWeightMethod::add(const size_t i, const double & value, const double & weight) const {

	if (weight >= accumulationArray.weight.at(i)){
		accumulationArray.data.at(i)   = value;
		accumulationArray.weight.at(i) = weight;
		accumulationArray.count.at(i)  = 1;
	}

	return;
}




}

}

// Drain
