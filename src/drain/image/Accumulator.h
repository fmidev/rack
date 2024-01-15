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
#ifndef ACCUMULATOR_H_
#define ACCUMULATOR_H_

#include <drain/image/CoordinatePolicy.h>
#include <limits>
#include <math.h>

#include <stdexcept>

#include "drain/util/Point.h"
#include "drain/util/Rectangle.h"
#include "drain/util/Variable.h"

#include "ImageT.h"
#include "AccumulationArray.h"
#include "AccumulationMethods.h"
#include "AccumulationConverter.h"



// TODO: image/
/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{




// New
/**
 *
 *   Accumulator contains five built-in accumulation methods (principles):
 *   -# overwrite
 *   -# maximum
 *   -# average
 *   -# weighted average
 *   -# maximum weight
 *   The methods can be extended user defined methods with addMethod() .
 *
 */
//class Accumulator : public AccumulationArray {
class Accumulator  {
public:

	/// Todo: export
	AccumulationArray accArray;

	inline
	Accumulator() :	methodPtr(&undefinedMethod) {

		/// TODO: a static map?
		/*
		addMethod(overwriteMethod);
		addMethod(maximumMethod);
		addMethod(minimumMethod);
		addMethod(averageMethod);
		addMethod(weightedAverageMethod);
		addMethod(maximumWeightMethod);
		*/
		// setMethod(_overwriteMethod);

	};

	inline
	Accumulator(const Accumulator & acc) : methodPtr(&undefinedMethod) {
		setMethod(acc.getMethod()); // if unset, sets unset... ie. default method.
		accArray.setGeometry(acc.accArray.getGeometry());
	}


	virtual inline
	~Accumulator(){};

	/// Set method to some of the predefined methods
	void setMethod(const std::string & method);

	/// Set method to some of the predefined methods
	void setMethod(const std::string & name, const std::string & params);

	/// Copies the method and its parameters.
	/**  That is, does not copy the target (accumulator &).
	 *   This limits to those already registered in AccMethodBank.
	 */
	inline
	void setMethod(const AccumulationMethod & method){
		setMethod(method.getName(), method.getParameters().getValues());
	}

	inline
	bool isMethodSet() const {
		return (methodPtr != & undefinedMethod);
	}

	inline
	const AccumulationMethod & getMethod() const {
		return *methodPtr;
	}

	inline
	AccumulationMethod & getMethod() {
		return *methodPtr;
	}

	/*
	void addMethod(AccumulationMethod & method){
		// std::cerr << "addMethod:" << method.name << '\t' << method << '\n';
		methods.insert(std::pair<std::string, AccumulationMethod &>(method.getName(), method));
	}
	*/

	/// Adds decoded data that applies natural scaling.
	inline
	void add(const size_t i, double value, double weight) {
		methodPtr->add(accArray, i, value, weight);
	}

	/// Adds decoded data that applies natural scaling.
	inline
	void add(const size_t i, double value, double weight, unsigned int count) {
		methodPtr->add(accArray, i, value, weight, count);
	}



	/// Add (accumulate) data with given prior weight
	/**
	 *  \param src - input data
	 */
	void addData(const Image & src, const AccumulationConverter & converter, double weight = 1.0, int iOffset=0, int jOffset=0);

	/// Add (accumulate) data weighted with input quality.
	/**
	 *  \param src - input data
	 *  \param srcQuality - quality field of input data
	 */
	void addData(const Image & src, const Image & srcQuality, const AccumulationConverter & converter, double weight = 1.0, int iOffset=0, int jOffset=0);


	/// Add (accumulate) data weighted with input quality and count; each data(i,j) contributes count(i,j) times.
	/**
	 *  \param src - input data
	 *  \param srcQuality - quality field of input data
	 *  \param srcCount - counts of input data (makes difference in resulting weights)
	 *
	 *  Count is bypassed when necessary; for example, when the accumulation method is MAXIMUM.
	 */
	void addData(const Image & src, const Image & srcQuality, const Image & srcCount, const AccumulationConverter & converter);
	// Could be easily added: ... double weight = 1.0, int iOffset=0, int jOffset=0

	/// Extracts the accumulated quantity or secondary quantities like weight and standard deviation.
	/**
	 *
	 * \param field - data layer to be extracted
	 *  - d = data;
	 *  - w = weight (8bit uchar); W = with same storage type as data
	 *  - c = count (8bit uchar);
	 *  - s = standard deviation, unscaled
	 *
	 */
	void extractField(char field, const AccumulationConverter & converter, Image & dst, const drain::Rectangle<int> & crop) const;


	virtual
	std::ostream & toStream(std::ostream & ostr) const;

public:

	/// A Some predefined methods, that can be set with setMethod(const std::string & key).
	//  std::map<std::string, AccumulationMethod &> methods;

protected:

	AccumulationMethod undefinedMethod;

	/*
	AccumulationMethod undefinedMethod;
	OverwriteMethod overwriteMethod;
	MaximumMethod   maximumMethod;
	MinimumMethod minimumMethod;
	AverageMethod   averageMethod;
	WeightedAverageMethod weightedAverageMethod;
	MaximumWeightMethod maximumWeightMethod;
	*/

	AccumulationMethod * methodPtr;

	// Initialize destination image to match the accumulation array - cropped if requested.
	/*
	 * \param dst – destination image to be cropped if cropArea supplied
	 * \param cropArea – sub-area of the accumulation array; empty if no cropping requested.
	 *
	 * If cropArea geometry equals that of the accumulation array, it will be cleared and discarded in processing.
	 *
	 */
	void initDst(const AccumulationConverter & coder, Image & dst, drain::Rectangle<int> & cropArea) const;


};

inline
std::ostream & operator<<(std::ostream &ostr, const Accumulator & accumulator){
	return accumulator.Accumulator::toStream(ostr);
}


}  // image::

}  // drain::

#endif /* Cumulator_H_ */

// Drain
