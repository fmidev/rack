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
#ifndef GradientOP_H_
#define GradientOP_H_


#include <math.h>

#include "ImageOp.h"

namespace drain
{

namespace image
{


/**! Computes spatial horizontal derivative, dx.
 *    src(i,j)-src(i-1,j). 
 * 
 */
class GradientOp : public ImageOp
{
public:


protected:

	GradientOp(const std::string & name, const std::string & description, double scale=1.0, double bias=0.0, int span=2) :
			ImageOp(name, description){
		parameters.reference("scale", this->scale = scale);
		parameters.reference("bias", this->bias = bias);
		parameters.reference("span", this->span = span);
	};

	virtual
	void traverse(const Image & src, Image & dst, int diLow, int djLow, int diHigh,int djHigh) const;

	double bias;
	double scale;
	int span;

	//mutable ImageCoordinateHandler coordinateHandler;


};


/**! Computes spatial horizontal derivative, dx.
 *    src(i,j)-src(i-1,j). 
 * 
 */
class GradientHorizontalOp : public GradientOp
{
public:
	
	GradientHorizontalOp(double scale=1.0, double bias=0.0, int span=2) : GradientOp("GradientHorizontalOp", "Horizontal intensity Gradient", scale, bias, span){};

protected:

	void traverse(const Image &src,Image &dst) const;

};

/**! Computes spatial vertical derivative, dy.
 *    src(i,j)-src(i,j-1). 
 * 
 */
class GradientVerticalOp : public GradientOp
{
public:
	
	GradientVerticalOp(double scale=1.0, double bias=0.0, int span=2) : GradientOp("GradientVerticalOp","Vertical intensity Gradient", scale, bias, span){};

protected:

	void traverse(const Image & src, Image & dst) const;

};

class LaplaceOp : public GradientOp {

protected:

	LaplaceOp(const std::string & name, const std::string & description) :
		GradientOp(name, description){
	}

protected:

	void traverse(const Image &src, Image &dst,
			int diLow, int djLow, int diHigh, int djHigh) const;

};

/**! Computes second horizontal derivative, dy2.
 *    src(i,j)-src(i,j-1).
 *
 */
class LaplaceHorizontalOp : public LaplaceOp
{
public:

	LaplaceHorizontalOp() : LaplaceOp("LaplaceHorizontalOp", "Second horizontal derivative"){};

protected:

	void traverse(const Image & src, Image & dst) const;

};
/**! Computes second vertical derivative, dy2.
 *    src(i,j)-src(i,j-1).
 *
 */
class LaplaceVerticalOp : public LaplaceOp
{
public:

	LaplaceVerticalOp() : LaplaceOp("LaplaceVerticalOp","Second vertical gradient"){};

protected:

	void traverse(const Image & src, Image & dst) const;

};

}
}


#endif /*GradientOP_H_*/

// Drain
