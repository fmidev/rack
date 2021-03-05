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


/// Virtual base class for horizontal and vertical intensity derivatives
/**!
 *
 */
// TODO: consider changing "span=2" to "radius=1"
class DifferentialOp : public ImageOp
{

public:

	virtual
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void getDstConf(const ImageConf & src, ImageConf & dst) const;
	//void makeCompatible(const ImageFrame & src, Image & dst) const;


protected:

	inline
	DifferentialOp(const std::string & name, const std::string & description, size_t channels=1, int radius=1) : // , double scale=1.0, double bias=0.0
			ImageOp(name, description), channels(channels) {
		//parameters.link("span",  this->span = span, "pix");
		parameters.link("radius",  this->radius = radius, "pix");
		parameters.link("LIMIT",  this->LIMIT = true, "0|1");
	};

	inline
	DifferentialOp(const DifferentialOp & op) : ImageOp(op), radius(op.radius), channels(op.channels), LIMIT(op.LIMIT)  {
		parameters.copyStruct(op.getParameters(), op, *this);
	};


	// double bias;
	// double scale;
	// int span;
	int radius;

	const size_t channels;

public:

	/// Applied only for unsigned dst types
	bool LIMIT;

protected:


	// Consider image range limited by 1 pix, to skip CoordinateHandler::validate() .
	virtual
	void traverse(const Channel & src, Channel & dst, int di, int dj) const = 0;


	inline
	void traverseHorz(const Channel & src, Channel & dst) const {
		traverse(src, dst, radius, 0);
	}

	inline
	void traverseVert(const Channel & src, Channel & dst) const {
		traverse(src, dst, 0, radius);
	}

};


///  Computes spatial horizontal derivatives (dx and dy).
/**!
 *    src(i,j)-src(i-1,j).
 *
 \~exec
  drainage shapes.png --iGradient 1 --channels 3    -o gradient-rg.png
  drainage shapes.png --iGradient 2 --channels 3    -o gradient-rg2.png
  drainage shapes.png --iGradient 1 --iMagnitude ''  -o gradient.png
 \~
 */
class GradientOp : public DifferentialOp
{
public:

	GradientOp(int radius=1) : DifferentialOp(__FUNCTION__,
			"Computes horizontal and vertical derivatives: df/di and df/dj.", 2, radius) { // , scale, bias
	};

	GradientOp(const GradientOp & op) : DifferentialOp(op){
	}


protected:

	GradientOp(const std::string & name, const std::string & description, size_t channels=1 ,  int radius=1) :
		DifferentialOp(name, description, channels, radius){
	};

	virtual
	void traverse(const Channel & src, Channel & dst, int di, int dj) const;

};

/**! Computes spatial horizontal derivative, dx.
 *    src(i,j)-src(i-1,j).
 *
 \~exec
  drainage shapes.png --iGradientHorz +1 -o gradientHorzRight.png
  drainage shapes.png --iGradientHorz -1 -o gradientHorzLeft.png
 \~
 */
class GradientHorzOp : public GradientOp
{
public:
	
	GradientHorzOp(int radius=1) :
		GradientOp(__FUNCTION__, "Horizontal intensity difference", 1, radius){};

	inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		traverseHorz(src, dst);
	}

};

/**! Computes spatial vertical derivative, dy.
 *    src(i,j+radius)-src(i,j-radius).
 * 
 \~exec
  drainage shapes.png --iGradientVert +1  -o gradientVertDown.png
  drainage shapes.png --iGradientVert -1  -o gradientVertUp.png
 \~
 */
class GradientVertOp : public GradientOp
{
public:
	
	inline
	GradientVertOp(int radius=1) : // double scale=1.0, double bias=0.0,
		GradientOp(__FUNCTION__, "Vertical intensity difference", 1 , radius){}; // , scale, bias

	inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		traverseVert(src, dst);
	}

};


/// Computes second intensity derivatives.
/**!
 *    src(i,j)-src(i,j-1).
 *
 \~exec
   drainage shapes.png --iLaplace 1  --channels 3     -o laplace-rg.png
   drainage shapes.png --iLaplace 1  --iMagnitude ''  -o laplace.png
   drainage shapes.png --iLaplace -1 --iMagnitude ''  -o laplaceNeg.png
 \~
 */
class LaplaceOp : public DifferentialOp {

public:

	LaplaceOp(int radius=1) : DifferentialOp(__FUNCTION__, "Second intensity derivatives, (df/di)^2 and (df/dj)^2", 2, radius){
	}

protected:

	LaplaceOp(const std::string & name, const std::string & description, int radius=1) : DifferentialOp(name, description, 1, radius){
	}

	virtual
	void traverse(const Channel &src, Channel &dst, int di, int dj) const;
	//void traverse(const ImageFrame &src, ImageFrame &dst, int diLow, int djLow, int diHigh, int djHigh) const;

};

/// Computes second horizontal derivative, dx2.
/**

     src(i+di,j)-src(i-di,j)^2.

 \~exec
   drainage shapes.png --iLaplaceHorz +1  -o laplaceHorz.png
   drainage shapes.png --iLaplaceHorz -1 -o laplaceHorz2.png
 \~

 */
class LaplaceHorzOp : public LaplaceOp
{
public:

	inline
	LaplaceHorzOp(int radius=1) : LaplaceOp(__FUNCTION__, "Second horizontal differential"){
		this->radius = radius;
	};

	inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		traverseHorz(src, dst);
	};

};

///  Computes second vertical derivative, dy2.
/**

Computes second vertical derivative:

  dy2 = src(i,j)-src(i,j-1)^2.

\~exec
  drainage shapes.png --iLaplaceVert +1  -o laplaceVert.png
  drainage shapes.png --iLaplaceVert -1 -o laplaceVert2.png
\~

 */
class LaplaceVertOp : public LaplaceOp
{
public:

	inline
	LaplaceVertOp(int radius=1) : LaplaceOp(__FUNCTION__, "Second vertical differential"){
		this->radius = radius;
	};


	inline
	void traverseChannel(const Channel &src, Channel & dst) const {
		traverseVert(src, dst);
	};

};

}
}


#endif /*GradientOP_H_*/

// Drain
