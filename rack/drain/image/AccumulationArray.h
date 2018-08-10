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
#ifndef ACCUMULATIONARRAY_H_
#define ACCUMULATIONARRAY_H_

#include <limits>
#include <math.h>

#include <stdexcept>

//#include "util/Data.h"
//#include "util/Variable.h"
//#include "util/DataScaling.h"

#include "Point.h"
#include "ImageT.h"
#include "Coordinates.h"

//#include "AccumulationRules.h"

#include "AccumulationConverter.h"


// TODO: image/
/** See also radar::Compositor
 * 
 */
namespace drain
{

namespace image
{



class AccumulationMethod;
/*
class AverageRule;
class WeightedAverageRule;
*/



/// General-purpose image compositing.
/*!
  Applied pre-scaling for the data; by default scale=1 and bias=0. Weights are assumed to take values between 0...1.
  In the extraction stage, the data can be scaled.

  \section Main features

  AccumulationArray has four layers:
	-# data - main cumulative quantity;
	-# weight - weight ie. significance of the cumulant;
	-# count - number of samples added (unsigned int);
	-# dataSquared - sum of squared quantities added;

  Injection. The main loop iterates radar (image) data points of type <Ti>, 
  mapping each point to a single point in the target array of type <T>;
  hence the name injector.

  Cumulation. The compositing starts by initiating the target image,
  see open().
  Each input radar image is projected on the target image cumulatively,
  see execute(). Finally, the cumulation must be completed by
  normalizing the result with the number/weight of
  cumulated inputs, see close().
  Hence, the target which should be seen.

  The weights are stored in the first alphaChannel (index 0) of the
  target image. If the target has a second image channel, also
  the (weighted) squared values are cumulated.  

  In cumulation, ...
  \f[
  x = \sqrt[p]{\frac{\sum_i {q_i}^r x_i^p}{\sum_i {q_i}^r } }
  \f]

  The default type of target image, that is, cumulation array, is
  double. If no (float valued) weighting  is applied, target image
  could be long int as well.
 */
class AccumulationArray 
{
public:

	friend class AccumulationMethod;
	//LinearScaling dataScaling;
	//LinearScaling dataScaling;

	//double MINIMUM_NEW;
	//double NO_DATA_NEW;

	/// Default constructor. The channels are DATA, COUNT, WEIGHT, WEIGHT2
	AccumulationArray(unsigned int width = 0, unsigned int height = 0){
		//, const std::string & method="OVERWRITE", double p=1.0, double r=1.0, double gain=1.0, double offset=0.0) {
		setGeometry(width, height);
	};

	/// Destructor.
	virtual ~AccumulationArray(){};

	/// Changes the geometry.
	virtual
	void setGeometry(unsigned int width, unsigned int height);

	/// Resets the accumulation array values to undetectValue. Does not change the geometry.
	void clear();

	/// NEW. Applied by setMethod(Variable )
	// AccumulationMethod * accumulationRule;

	//void setAccumulationRule(const AccumulationMethod & rule);


	/// Returns the width of the accumulation array.
	inline
	unsigned int getWidth() const { return width; };

	/// Returns the height of the accumulation array.
	inline
	unsigned int getHeight() const { return height; };




	/// Given the code of an extracted quantity, returns the name of the quantity
	// todo static!
	//std::string getQuantityName(char c) const;



	bool debug;

	/// User-defined metadata. Needed?
	// VariableMap properties;

	inline
	size_t address(const size_t &i, const size_t &j){ return data.address(i,j); };

//protected:

	/// Accumulation array for actual data: \f$ sum x_i \f$, \f$ sum w_i x_i \f$ or generally \f$ sum w_i^r x_i^p \f$
	ImageT<double> data;

	/// Accumulation array for weights (\f$ sum w_i \f$)
	ImageT<double> weight;

	/// Accumulation array (\f$ N \f$ in \f$ sum_i^N \f$)
	ImageT<unsigned int> count;

	/// Accumulation array for auxiliary data; typically \f$ sum w_i^r x_i^{2p} \f$ for std.deviation or difference (in Overwrite)
	// actual data \f$ sum w_i^r x_i^{2p} \f$
	ImageT<double> data2;

	//inline
	//void setConverter(AccumulationConverter & c) const { this->converter = &c; };

	//mutable AccumulationConverter * converter;

protected:

	//AccumulationConverter defaultConverter;

	unsigned int width;  // TODO: size_t ?
	unsigned int height;

	CoordinateHandler2D coordinateHandler;

};

std::ostream & operator<<(std::ostream &ostr, const AccumulationArray &cumulator);


}

}

#endif /* Cumulator_H_ */

// Drain
