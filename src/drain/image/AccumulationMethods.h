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
#ifndef ACCUMULATION_H_
#define ACCUMULATION_H_

#include <limits>
#include <math.h>

#include <stdexcept>

#include <drain/util/Bank.h>
#include <drain/util/BeanLike.h>
#include <drain/util/Point.h>
#include <drain/util/Rectangle.h>
#include "ImageT.h"
#include "CoordinatePolicy.h"
#include "AccumulationArray.h"

/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{


/// Function for accumulating data: maximum, average, weighted average etc.
/**
 *   Connections to measurement devices:
 *   -# No data measured (\c nodata) - if \c count is undetectValue
 *   -# Data measured, but precision etc. threshold not exceeded (\c undetect ) - if \c weight is undetectValue
 *
 */
class AccumulationMethod : public BeanLike {
public:

	inline
	AccumulationMethod() :  BeanLike("UNDEFINED", __FUNCTION__)  {
	};

	inline
	AccumulationMethod(const AccumulationMethod &method) : BeanLike(method.getName(), method.getDescription()){
	};

	virtual
	~AccumulationMethod(){};




	/// Adds a weighted value to the accumulation array
	/**
	 *   \par i - precomputed address in the array
	 *   \par value  - value to be added
	 *   \par weight - weight of the value
	 *
	 *   Notice that not all the rules apply the weights.
	 *   Semantically, the weights should reflect the importance, confidence or relevance of the value.
	 */
	virtual inline
	void add(AccumulationArray & accArray, const size_t i, double value, double weight) const {
		throw std::runtime_error("AccumulationMethod::add - method/rule UNDEFINED");
	};


	/// Adds 'count' copies of a weighted value to the accumulation array
	virtual inline
	void add(AccumulationArray & accArray, const size_t i, double value, double weight, unsigned int count) const {
		add(accArray, i, value, weight);
	};



	/// Retrieve the accumulated values from the accumulation matrix back to a data array.
	/**
	 *  In this context, the "value" refers to the main object of interest (measurement, prediction etc).
	 *
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 *  \par NEGLIBLE - if value is less than minValue (or weight is undetectValue but bin count non-undetectValue), this value is applied.
	 *  \par NODATA   - if bin count is undetectValue that is, there is no data in a bin, this value is applied.
	 */
	virtual
	void extractValue(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;


	/// Retrieves the (average) weight of the accumulated values.
	/**
	 * 	\param accArray - data source
	 * 	\param coder    - logic for scaling and encoding the result
	 *  \pararm dst - target array in which the data are stored.
	 */
	virtual
	void extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	/// Retrieves the count of values accumulated.
	/**
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 */
	virtual
	void extractCount(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	/// Retrieves the standard deviation of the accumulated values.
	/**
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 *  \par NODATA   - if bin count is undetectValue that is, there is no data in a bin, this value is applied.
	 */
	virtual
	void extractDev(const AccumulationArray & accArray,  const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	//virtual
	//void extractDevInv(const AccumulationConverter & coder, Image & dst) const;

	//const std::string name;

	/*
	virtual
	std::ostream & toStream(std::ostream & ostr) const {
		ostr << name;
		if (!parameters.empty())
			ostr << " [" << parameters << "]";
		return ostr;
	};
	*/

protected:

	/// Sets variables that depend on public parameters. Called by setParameters().
	//virtual
	//void updateInternalParameters(){};

	void initDstOLD(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop) const ;


	// AccumulationMethod(const std::string & name, AccumulationArray & c) :  BeanLike(name, __FUNCTION__), accumulationArray(c)  {
	//};

	AccumulationMethod(const std::string & name) :  BeanLike(name, __FUNCTION__) {
	};

	//AccumulationArray & accumulationArray;

};

/*
inline
std::ostream & operator<<(std::ostream & ostr, const AccumulationMethod & accumulationMethod){
	return accumulationMethod.toStream(ostr);
}
*/




class OverwriteMethod : public AccumulationMethod {

public:

	//OverwriteMethod(AccumulationArray & c) : AccumulationMethod("LATEST", c) {};
	OverwriteMethod() : AccumulationMethod("LATEST") {};

	virtual
	void add(AccumulationArray & accArray, const size_t i, double value, double weight) const;

	/// Special defimition: difference
	virtual
	void extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;


};

class MaximumMethod : public AccumulationMethod {

public:

	//MaximumMethod(AccumulationArray & c) : AccumulationMethod("MAXIMUM", c) {};
	inline
	MaximumMethod() : AccumulationMethod("MAXIMUM") {};

	virtual
	void add(AccumulationArray & accArray, const size_t i, double value, double weight) const;

};

class MinimumMethod : public AccumulationMethod {

public:

	//MinimumMethod(AccumulationArray & c) : AccumulationMethod("MINIMUM", c) {};
	inline
	MinimumMethod() : AccumulationMethod("MINIMUM") {};

	virtual
	void add(AccumulationArray & accArray, const size_t i, double value, double weight) const;

};


class AverageMethod : public AccumulationMethod {

public:

	//AverageMethod(AccumulationArray & c) : AccumulationMethod("AVERAGE", c) {};
	inline
	AverageMethod() : AccumulationMethod("AVERAGE") {};

	virtual
	void add(AccumulationArray & accArray,  const size_t i, double value, double weight) const;

	virtual // inline
	void add(AccumulationArray & accArray,  const size_t i, double value, double weight, unsigned int count) const;

	virtual
	void extractValue(const AccumulationArray & accArray,  const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	virtual
	void extractWeight(const AccumulationArray & accArray,  const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	virtual
	void extractDev(const AccumulationArray & accArray,  const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

};



class WeightedAverageMethod : public AccumulationMethod {


public:

	inline
	WeightedAverageMethod() : AccumulationMethod("WAVG"), pInv(1.0), USE_P(false), rInv(1.0), USE_R(false) {
		parameters.link("p", p = 1.0);
		parameters.link("r", r = 1.0);
		parameters.link("bias", bias = 0.0);
		updateBean();
	};

	inline
	WeightedAverageMethod(const WeightedAverageMethod & method) : AccumulationMethod(method), pInv(1.0), USE_P(false), rInv(1.0), USE_R(false) {
		parameters.copyStruct(method.parameters, method, *this);
		/*
		parameters.link("p", p = 1.0);
		parameters.link("r", r = 1.0);
		parameters.link("bias", bias = 0.0);
		*/
		updateBean();
	};


	virtual
	void updateBean() const;

	virtual
	void add(AccumulationArray & accArray,  const size_t i, double value, double weight) const;

	virtual inline
	void add(AccumulationArray & accArray,  const size_t i, double value, double weight, unsigned int count) const;

	virtual
	void extractValue(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	virtual
	void extractWeight(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	virtual
	void extractDev(const AccumulationArray & accArray, const AccumulationConverter & coder, Image & dst, const drain::Rectangle<int> & crop = {0,0,0,0}) const;

	/*
	virtual
	std::ostream & toStream(std::ostream & ostr) const {
		//ostr << name << '(' << p << ',' << r << ',' << dataScaling << ')';
		ostr << name << '(' << p << ',' << r << ',' << bias << ')';
		return ostr;
	};
	*/

protected:

	double bias = 0.0;

	/// Power for data values
	double p = 1.0;
	mutable	double pInv = 1.0;
	mutable bool USE_P = false;

	/// Power for weights
	double r = 1.0;
	mutable	double rInv = 1.0;
	mutable bool USE_R = false;

};


class MaximumWeightMethod : public AccumulationMethod {

public:

	//MaximumWeightMethod(AccumulationArray & c) : AccumulationMethod("MAXW", c) {};
	inline
	MaximumWeightMethod() : AccumulationMethod("MAXW") {};

	virtual
	void add(AccumulationArray & accArray, const size_t i, double value, double weight) const;

};


typedef drain::Bank<AccumulationMethod> AccMethodBank;

extern
AccMethodBank & getAccMethodBank();


}

}

#endif /* Cumulator_H_ */

// Drain
