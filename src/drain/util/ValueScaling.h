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
#ifndef IMAGE_SCALING2_H_
#define IMAGE_SCALING2_H_ "ImageScaling 0.1,  2017/09 Markus.Peura@fmi.fi"

#include <drain/Type.h>
#include <drain/TypeUtils.h>
#include <drain/UniTuple.h>
#include <stddef.h>  // size_t

#include "Range.h"

//#include "Geometry.h"
//#include "Coordinates.h"


namespace drain
{


//extern drain::Log iLog;

// If the intensities of the image correspond to a physical value (like temperature), this is the recommended way to copy.

/// Linear scaling and physical range for image intensities.
/** Internally,drain::ValueScaling holds variables
 *  - scale
 *  - offset
 *  - physRange.min (optional);
 *	- maxPhysValue (optional);
 *
 * drain::ValueScaling does not store information of storage type (no either on minimum and maximum values supported by storage types).
 *  \see drain::typeLimits
 *
 */
class ValueScaling : public UniTuple<double,2> {  // UniTuple<double,4>{
public:

	/// Multiplicative coefficient \i a in: y = ax + b
	double & scale;

	/// Additive coefficient \i b in: y = ax + b
	double & offset;

	/// Minimum and maximum physical value of the imaged quantity (not limited to corresponding to minCodeValue?).
	drain::Range<double> physRange;


	inline
	ValueScaling(double scale=1.0, double offset = 0.0) : // RANGE ok? double scaleOut=1.0, double offsetOut=0.0) :
		scale(this->next()), offset(this->next()) //, physRange(this->tuple(), 2)
	{
		//setConversionScale(scale, offset, scaleOut, offsetOut);
		setConversionScale(scale, offset, 1.0, 0);
		physRange.set(0,0);
	};

	inline
	ValueScaling(double scale, double offset, const drain::Range<double> & range) : //, double scaleOut=1.0, double offsetOut=0.0) :
		scale(this->next()), offset(this->next()) //, physRange(this->tuple(), 2)
	{
		//setConversionScale(scale, offset, scaleOut, offsetOut);
		setConversionScale(scale, offset, 1.0, 0);
		physRange.set(range); // TODO tune: scaleOut, offsetOut
	};

	inline
	ValueScaling(const drain::ValueScaling & scaling) :
		scale(this->next()), offset(this->next()) //, physRange(this->tuple(), 2)
	{
		this->assignSequence(scaling);
		physRange.set(scaling.physRange);
	};

	//
	inline
	ValueScaling(const drain::UniTuple<double,2> & scaling) :
		scale(this->next()), offset(this->next())
	{
		setScaling(scaling[0], scaling[1]);
	};


	// Conversion scaling
	inline
	ValueScaling(const drain::ValueScaling & scalingIn, const drain::ValueScaling & scalingOut) :
		scale(this->next()), offset(this->next())
	{
		setConversionScale(scalingIn, scalingOut);
		// physRange??? intersection / union
	};

	virtual inline
	~ValueScaling(){};


	inline
	ValueScaling & operator=(const drain::ValueScaling & scaling){
		if (&scaling != this){
			this->assignSequence(scaling);
			physRange.set(scaling.physRange);
		}
		return *this;
	}

	/// Set linear scaling
	virtual inline
	void setScaling(double scale, double offset){
		this->set(scale, offset); // virtual IMPORTANT for channels/view
	}

	virtual
	void setScaling(const ValueScaling & scaling){
		this->assignSequence(scaling);
	}

	/// Get linear scaling
	virtual inline  // LATER: scalingPtr!
	const ValueScaling & getScaling() const {
		return *this;
	}

	/// Get linear scaling
	virtual inline // LATER: scalingPtr!
	ValueScaling & getScaling(){
		return *this;
	}



	/// If the intensities of the image correspond to an absolute value (like count) then the scale should be reset to unity with this function.
	inline
	void setAbsoluteScale(){
		set(1.0, 0.0);
	}

	/// For "small integer" types, resets offset and scale such that maximum code value corresponds to 1.0 (100% intensity).
	/**  Applies to:
	 * 	- \c char
	 * 	- \c unsigned char
	 * 	- \c short int
	 * 	- \c unsigned short int
	 */
	inline
	void setNormalScale(const std::type_info & t){
		if (Type::call<drain::typeIsSmallInt>(t))
			set(1.0/drain::Type::call<drain::typeMax,double>(t), 0.0);
		else  // warn? (esp. for int and long int)
			set(1.0, 0.0); // absolute scale
	}

	/// If the intensities of the image correspond to a physical value (like temperature), then the scaling coefficient should be set with this function.
	inline
	void setConversionScale(double scale, double offset = 0.0, double scaleOut=1.0, double offsetOut=0.0){
		set(scale/scaleOut, (offset - offsetOut)/scaleOut);
	}

	/// Set scaling for which scaling.inv(x) = s2.inv(s1.fwd(x))
	inline
	void setConversionScale(const drain::ValueScaling & s1, const drain::ValueScaling & s2){
		set(s2.scale/s1.scale, (s2.offset-s1.offset) / s1.scale);
	}

	//template <typename T>
	inline
	void setConversionScale(const Range<double> & r1, const Range<double> & r2){
		set(r2.width()/r1.width(), r2.min - r2.width()/r1.width()*r1.min);
	}

	/// If storage type is integer, adjust scale such that resolution is maximized.
	void setOptimalScale(const std::type_info & t);


	/// Sets physical range (min, max) and scales storage type accordingly.
	inline
	void setPhysicalScale(const std::type_info & t, double min, double max){
		setPhysicalRange(min, max);
		setOptimalScale(t);
	}

	/// Sets physical range (min, max) and scales storage type accordingly.
	/**

	 */
	inline
	void setPhysicalScale(const std::type_info & t, const drain::ValueScaling & scaling){
		setPhysicalRange(scaling.getPhysicalRange());
		setOptimalScale(t);
	};

	/// Returns a typical or supported range for physical values.
	inline
	const Range<double> & getPhysicalRange() const { // , const std::string &unit ?
		return physRange;
	}

	/// Returns a typical or supported range for physical values. Modifying the range will not change scaling.
	inline
	Range<double> & getPhysicalRange() { // , const std::string &unit ?
		return physRange;
	}

	/// In integer-valued images, set the physical values corresponding to [0, maxCodeValue]
	inline
	void setPhysicalMax(double max){ // dangerous (after min?)
		setPhysicalRange(0.0, max);
	}

	/// Sets the supported range for physical values. Does not change scaling or type.
	template <class T>
	inline
	void setPhysicalRange(const Range<T> &range){ // , const std::string &unit ?
		physRange.assignSequence(range);
	}

	/// Sets the supported range for physical values. Does not change scaling or type.
	inline
	void setPhysicalRange(double min, double max){ // , const std::string &unit ?
		physRange.set(min,max);
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
	bool isScaled() const {
		return (scale!=1.0) || (offset!=0.0);
	}

	/// Returns the minimum physical value.
	inline
	double getMinPhys() const {
		return physRange.min;
	}

	/// Returns the maximum physical value.
	inline
	double getMaxPhys() const {
		return physRange.max;
	}

	/// Returns true, physical intensity range has been set.
	inline
	bool isPhysical() const {
		return (!physRange.empty());
	}

	/// Sets scale and offset according to physical range and current type.
	/*   short => float
	 *
	 */
	//inline
	void adoptScaling(const drain::ValueScaling & srcScaling, const std::type_info & srcType, const std::type_info & dstType = typeid(void));


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



	inline
	void toStream(std::ostream & ostr) const {
		ostr << scale << ',' << offset;
		if (isPhysical())
			 ostr << " [" << physRange << ']';
	}

	inline
	std::string str() const{
		std::stringstream sstr;
		toStream(sstr);
		return sstr.str();
	}



};

inline
std::ostream & operator<<(std::ostream &ostr, const drain::ValueScaling & s){
	s.toStream(ostr);
	return ostr;
}

} // drain::

#endif

// Drain
