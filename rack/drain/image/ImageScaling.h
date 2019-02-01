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
#ifndef IMAGE_SCALING_H_
#define IMAGE_SCALING_H_ "ImageScaling 0.1,  2017/09 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "../util/TypeUtils.h"

#include "Geometry.h"
#include "Coordinates.h"


namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{

//extern drain::Log iLog;

// If the intensities of the image correspond to a physical value (like temperature), this is the recommended way to copy.

/// Linear scaling and physical range for image intensities.
/** Internally, ImageScaling holds variables
 *  - scale
 *  - offset
 *  - minPhysValue (optional);
 *	- maxPhysValue (optional);
 *
 *  ImageScaling does not store information of storage type (no either on minimum and maximum values supported by storage types).
 *  \see drain::typeLimits
 *
 */
class ImageScaling {
public:

	ImageScaling() : scale(1.0), offset(0.0), minPhysValue(0.0), maxPhysValue(0.0) {};

	inline
	ImageScaling(const ImageScaling & scaling){
		set(scaling);
	};

	/// Forward scaling: given encoded value x, returns corresponding value (possibly physically meaningful).
	inline
	double fwd(double x) const {
		return scale*x + offset;
	}

	/// Inverse scaling: given physically meaningful value y, returns the corresponding code value.
	inline
	double inv(double y) const {
		return (y - offset) / scale;
	}

	/// Copy scale and offset as well as physical range, if applied.
	/**
	 * If the intensities of the image correspond to a physical value (like temperature), this is the recommended way to copy.
	 *
	 */
	inline
	void set(const ImageScaling & scaling){
		setScale(scaling.getScale(), scaling.getOffset());
		setPhysicalRange(scaling.minPhysValue, scaling.maxPhysValue);
	};


	/// If the intensities of the image correspond to a physical value (like temperature), then the scaling coefficient should be set with this function.
	inline
	void setScale(double scale, double offset = 0.0){
		this->scale  = scale;
		this->offset = offset;
	}


	/// If the intensities of the image correspond to an absolute value (like count) then the scale should be reset to unity with this function.
	inline
	void setAbsoluteScale(){
		setScale(1.0, 0.0);
	}

	/// For "small" integral types, resets offset and scale such that maximum code value corrensponds to 1.0 (100% intensity).
	/**  Applies to:
	 * 	- \c char
	 * 	- \c unsigned char
	 * 	- \c short int
	 * 	- \c unsigned short int
	 */
	inline
	void setNormalScale(const std::type_info & t){

		if (Type::call<drain::typeIsSmallInt>(t))
			setScale(1.0/drain::Type::call<drain::typeMax,double>(t), 0.0);
		else  // warn? (esp. for int and long int)
			setScale(1.0, 0.0); // absolute scale

	}

	/// Set scaling for which scaling.inv(x) = scaling2.inv(scaling1.fwd(x))
	inline
	void setConversionScale(const ImageScaling & scaling1, const ImageScaling & scaling2){
		// this->scale  = scaling1.getScale() / scaling2.getScale();
		// this->offset = (scaling1.getOffset() - scaling2.getOffset()) / scaling2.getScale();
		this->scale  = scaling2.getScale() / scaling1.getScale();
		this->offset = (scaling2.getOffset() - scaling1.getOffset()) / scaling1.getScale();;
	}

	/// If storage type is integer, adjust scale such that resolution is maximized.
	void setOptimalScale(const std::type_info & t);


	/// Sets physical range (min, max) and scales storage type accordingly.
	inline
	void setPhysicalScale(const std::type_info & t, double min, double max){
		//Logger mout(getImgLog(), "ImageScaling", __FUNCTION__);
		setPhysicalRange(min, max);
		setOptimalScale(t);
		//mout.warn() << "type=" << toStr() << mout.endl;
	}

	/// Sets physical range (min, max) and scales storage type accordingly.
	/**

	 */
	inline
	void setPhysicalScale(const std::type_info & t, const ImageScaling & scaling){
		setPhysicalRange(scaling.minPhysValue, scaling.maxPhysValue);
		setOptimalScale(t);
	};


	/// In integer-valued images, set the physical values corresponding to [0, maxCodeValue]
	inline
	void setPhysicalMax(double max){
		setPhysicalRange(0.0, max);
	}

	/// Sets the supported range for physical values. Does not change scaling or type.
	inline
	void setPhysicalRange(double min, double max){ // , const std::string &unit ?
		minPhysValue = min;
		maxPhysValue = max;
	}

	/// Returns the intensity scaling factor. See set setScale()
	inline
	double getScale() const { return scale; }

	/// Returns the intensity scaling offset, ie. b in a*x + b . See set setScale()
	inline
	double getOffset() const { return offset; }

	/// Returns true, if scaling has effect ie. scale!=1.0 or offset!=0.0
	/**
	 *   Note: returns true if scale==0. Maybe false would be better.
	 */
	inline
	bool isScaled() const { return (scale!=1.0) || (offset!=0.0); }

	/// Returns the minimum physical value.
	inline
	double getMinPhys() const { return minPhysValue; }

	/// Returns the maximum physical value.
	inline
	double getMaxPhys() const { return maxPhysValue; }

	/// Returns true, physical intensity range has been set.
	inline
	bool isPhysical() const { return (minPhysValue != maxPhysValue);  }

	/// Sets scale and offset according to physical range and current type.
	/*   short => float
	 *
	 */
	//inline
	void adoptScaling(const ImageScaling & srcScaling, const std::type_info & srcType, const std::type_info & dstType = typeid(void));



	inline
	void toOStr(std::ostream & ostr) const {
		ostr << scale << ',' << offset;
		if (isPhysical())
			 ostr << ' ' << '[' << minPhysValue << ',' << maxPhysValue << ']';
	}

	inline
	std::string toStr() const{
		std::stringstream sstr;
		toOStr(sstr);
		return sstr.str();
	}


	/// Multiplicative coefficient \i a in: y = ax + b
	double scale;
	/// Additive coefficient \i b in: y = ax + b
	double offset;

	/// Minimum physical value of the imaged quantity, corresponding to minCodeValue.
	double minPhysValue;
	/// Maximum physical value of the imaged quantity, corresponding to minCodeValue.
	double maxPhysValue;


};

inline
std::ostream & operator<<(std::ostream &ostr, const ImageScaling & s){
	s.toOStr(ostr);
	return ostr;
}


} // image::
} // drain::

#endif

// Drain
