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

#include "util/ReferenceMap.h"
//#include "util/DataScaling.h"

#include "../util/Point.h"
#include "ImageT.h"
#include "Coordinates.h"
#include "AccumulationArray.h"



// TODO: image/
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
class AccumulationMethod {
public:

	AccumulationMethod(AccumulationArray & c) :  name("UNDEFINED"), accumulationArray(c)  {  //  minValue(std::numeric_limits<double>::min()), minWeight(0.0),
	};

	virtual
	~AccumulationMethod(){};

	/// Comma-separated list of parameters
	inline
	void setParameters(const std::string & p){
		parameters.setValues(p);
		updateInternalParameters();
	}

	/// Comma-separated list of parameters
	template <class T>
	inline
	void setParameter(const std::string & key, const T & value){
		parameters[key] = value;
		updateInternalParameters();
	}


	inline
	const ReferenceMap & getParameters() const {
		return parameters;
	};


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
	void add(const size_t i, double value, double weight) const {
		throw std::runtime_error("AccumulationMethod::add - method/rule UNDEFINED");
	};


	/// Adds 'count' copies of a weighted value to the accumulation array
	virtual inline
	void add(const size_t i, double value, double weight, unsigned int count) const {
		add(i, value, weight);
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
	void extractValue(const AccumulationConverter & coder, Image & dst) const;


	/// Retrieves the (average) weight of the accumulated values.
	/**
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 */
	virtual
	void extractWeight(const AccumulationConverter & coder, Image & dst) const;

	/// Retrieves the count of values accumulated.
	/**
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 */
	virtual
	void extractCount(const AccumulationConverter & coder, Image & dst) const;

	/// Retrieves the standard deviation of the accumulated values.
	/**
	 *  \par dst - target array in which the values are stored.
	 *  \par gain - scaling coefficient applied to each retrived value
	 *  \par offset - additive coefficient applied to each retrieved value
	 *  \par NODATA   - if bin count is undetectValue that is, there is no data in a bin, this value is applied.
	 */
	virtual
	void extractDev(const AccumulationConverter & coder, Image & dst) const;

	//virtual
	//void extractDevInv(const AccumulationConverter & coder, Image & dst) const;

	const std::string name;

	virtual
	std::ostream & toStream(std::ostream & ostr) const {
		ostr << name;
		if (!parameters.empty())
			ostr << " [" << parameters << "]";
		return ostr;
	};

protected:

	/// Sets variables that depend on public parameters. Called by setParameters().
	virtual
	void updateInternalParameters(){};

	void initDst(const AccumulationConverter & coder, Image & dst) const ;

	AccumulationMethod(const std::string & name, AccumulationArray & c) :  name(name), accumulationArray(c)  {
	};

	ReferenceMap parameters;

	AccumulationArray & accumulationArray;

};

inline
std::ostream & operator<<(std::ostream & ostr, const AccumulationMethod & accumulationMethod){
	return accumulationMethod.toStream(ostr);
}




class OverwriteMethod : public AccumulationMethod {

public:

	OverwriteMethod(AccumulationArray & c) : AccumulationMethod("LATEST", c) {};

	virtual
	void add(const size_t i, double value, double weight) const;

	virtual
	void extractDev(const AccumulationConverter & coder, Image & dst) const;


};

class MaximumMethod : public AccumulationMethod {

public:

	MaximumMethod(AccumulationArray & c) : AccumulationMethod("MAXIMUM", c) {};

	virtual
	void add(const size_t i, double value, double weight) const;

};

class MinimumMethod : public AccumulationMethod {

public:

	MinimumMethod(AccumulationArray & c) : AccumulationMethod("MINIMUM", c) {};

	virtual
	void add(const size_t i, double value, double weight) const;

};


class AverageMethod : public AccumulationMethod {

public:

	AverageMethod(AccumulationArray & c) : AccumulationMethod("AVERAGE", c) {};

	virtual
	void add(const size_t i, double value, double weight) const;

	virtual inline
	void add(const size_t i, double value, double weight, unsigned int count) const;

	virtual
	void extractValue(const AccumulationConverter & coder, Image & dst) const;

	virtual
	void extractWeight(const AccumulationConverter & coder, Image & dst) const;

	virtual
	void extractDev(const AccumulationConverter & coder, Image & dst) const;

};



class WeightedAverageMethod : public AccumulationMethod {


public:


	WeightedAverageMethod(AccumulationArray & c) : AccumulationMethod("WAVG", c), bias(0.0), p(1.0), pInv(1.0), r(1.0), rInv(1.0), USE_P(true), USE_R(true) {
		parameters.reference("p", p);
		parameters.reference("r", r);
		parameters.reference("bias", bias);
	};


	virtual
	void updateInternalParameters();
	//void setParameters(const std::string & parameters);

	virtual
	void add(const size_t i, double value, double weight) const;

	virtual inline
	void add(const size_t i, double value, double weight, unsigned int count) const;

	virtual
	void extractValue(const AccumulationConverter & coder, Image & dst) const;

	virtual
	void extractWeight(const AccumulationConverter & coder, Image & dst) const;

	virtual
	void extractDev(const AccumulationConverter & coder, Image & dst) const;

	virtual
	std::ostream & toStream(std::ostream & ostr) const {
		//ostr << name << '(' << p << ',' << r << ',' << dataScaling << ')';
		ostr << name << '(' << p << ',' << r << ',' << bias << ')';
		return ostr;
	};

protected:

	double bias;

	/// Power for data values
	double p;
	double pInv;

	/// Power for weights
	double r;
	double rInv;

	bool USE_P;
	bool USE_R;



};


class MaximumWeightMethod : public AccumulationMethod {

public:

	MaximumWeightMethod(AccumulationArray & c) : AccumulationMethod("MAXW", c) {};

	virtual
	void add(const size_t i, double value, double weight) const;

};



}

}

#endif /* Cumulator_H_ */

// Drain
