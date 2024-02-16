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
#ifndef IMAGE_CONF_H_
#define IMAGE_CONF_H_ "ImageConf 2,  2021/03 Markus.Peura@fmi.fi"

#include <drain/image/CoordinatePolicy.h>
#include <stddef.h>  // size_t

#include "../util/Caster.h"
#include "../util/ValueScaling.h"

#include "Geometry.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{



class Encoding :  public drain::ValueScaling {  //{ //public drain::Caster,

public:

	inline
	Encoding(const std::type_info & t = typeid(unsigned char)){ // : scalingPtr(this) {
		setType(t);
	}

	inline
	Encoding(const Encoding & encoding){ // : scalingPtr(this) {
		setEncoding(encoding);
		//setType(encoding.caster.getType());
	}

	/// Return type and scaling
	inline
	const Encoding & getEncoding() const {
		// NOTE: scaling will not be local (scalingPtr)
		return *this;
	}

	/// Return type and scaling
	inline
	Encoding & getEncoding(){
		// NOTE: scaling will not be local (scalingPtr)
		return *this;
	}


	/// Set type and scaling
	inline
	void setEncoding(const Encoding & e){
		setType(e.getType());
		setScaling(e);
		setPhysicalRange(e.getPhysicalRange());
	}

	Encoding & operator=(const Encoding & e){
		setEncoding(e);
		return *this;
	}

	/// In base class(es), mainly for storing storage type. In derived classes, also for value conversions.
	Caster caster;

	// Size of the storage type (1 for 8 bits, 2 for 16 bits, etc.)
	size_t byteSize;

	/// Information of the current type.
	std::string type;  // synch?

	/// Linear scaling
	// drain::ValueScaling & scaling;

	/// Get the storage type
	inline
	const std::type_info & getType() const {
		return caster.getType();
	}

	/// Get the storage type
	inline
	bool typeIsSet() const {
		return caster.typeIsSet();
	}

	/// Set storage type
	void setType(const std::type_info & t){

		if (t == typeid(bool)){
			//mout.warn("storage type 'bool' not supported, using 'unsigned char'" );
			setType(typeid(unsigned char)); // re-invoke
		}
		else if (t == typeid(std::string)){
			//mout.error("storage type 'std::string' not applicable to images" );
			//setType(typeid(unsigned char));
			throw std::runtime_error("storage type 'std::string' not applicable to images");
			return;
		}

		caster.setType(t);
		byteSize = caster.getElementSize();
		type.resize(1);
		type.at(0) = drain::Type::getTypeChar(caster.getType());

	}

	/// Convenience
	template <class T>
	inline
	void setType(){
		setType(typeid(T));
	}


	template <class T>
	inline
	typename drain::typeLimiter<T>::value_t getLimiter() const {
		return drain::Type::call<drain::typeLimiter<T> >(caster.getType());
	}



	/// Returns the size in bytes of the storage type (1 for unsigned char, 2 for 16-bit types, etc.)
	inline
	size_t getElementSize() const {
		return byteSize;
	};



	/// Sets channel specific scaling instead of shared (image-level) scaling
	/*
	inline
	void useOwnScaling() {
		if (scalingPtr != this){
			scalingPtr = this;
		}
	}

	/// Channels may have scaling different from the main image. Typically, alpha
	inline
	bool hasOwnScaling() const {
		return scalingPtr == this; //& conf.getScaling();
	}
	 */

	/*
	virtual inline
	const drain::ValueScaling & getScaling() const {
		return *this;
		//return *scalingPtr;
	}
	*/

	/// Facilitates modifications provided directly bydrain::ValueScaling object.
	/**
	 *   Requesting modifiable scaling implies providing a "safe", separate scaling.
	 *   In the case of channels, this means changing from shared image-level scaling
	 *   to channel specific scaling.
	 */
	/*
	virtual inline
	drain::ValueScaling & getScaling(){
		//useOwnScaling();
		return *this; // return *scalingPtr; // conf; // *scalingPtr; //
	}

	virtual inline
	void setScaling(const drain::ValueScaling & scaling){
		//useOwnScaling();
		ValueScaling::assign(scaling);
	}
	*/

	/// Set linear scaling
	/*
	virtual inline
	void setScaling(double scale, double offset){
		//useOwnScaling();
		ValueScaling::set(scale, offset); // virtual IMPORTANT for channels/view
	}
	*/

	/*
	virtual inline
	void linkScaling(const drain::ValueScaling & scaling){
		scalingPtr =  &scaling;
	}
	*/




	inline
	void setPhysicalRange(const Range<double> &range, bool rescale=false){ // , const std::string &unit ?
		//conf.physRange.assign(range);
		getScaling().setPhysicalRange(range);
		if (rescale)
			setOptimalScale();
	}

	/// Sets the supported range for physical values. Does not change scaling or type.
	inline
	void setPhysicalRange(double min, double max, bool rescale=false){ // , const std::string &unit ?
		//conf.physRange.set(min,max);
		getScaling().setPhysicalRange(min,max);
		if (rescale)
			setOptimalScale();
	}


	inline
	void setOptimalScale(){
		getScaling().setOptimalScale(getType());
	}

	inline
	void setOptimalPhysicalScale(double min, double max){ // , const std::string &unit ?
		setPhysicalRange(min, max, true);
	}


	/// Returns the minimum value supported by the current storage type.
	/**
	 *   Does not check the actual image data.
	 *
	 *   \see getMax()
	 *   \see getLimiter()
	 *   \see setType()
	 */
	template <class T>
	inline
	T getTypeMin() const {
		return Type::call<typeMin, T>(caster.getType());
	}

	/// Returns the maximum value supported by the current storage type. \see setType()
	/**
	 *   Does not check the actual image data.
	 *
	 *   \see getMin()
	 *   \see getLimiter()
	 *   \see setType()
	 */
	template <class T>
	inline
	T getTypeMax() const {
		return Type::call<typeMax, T>(caster.getType());
	}




	/// Returns the actual or guessed maximum physical value,
	/**
	 *  Is physical range is unset, and the storage type is a "small" integer, the value is guessed.
	 */
	inline
	double requestPhysicalMax(double defaultMax = static_cast<double>(std::numeric_limits<short int>::max())) const {
		const ValueScaling & scaling = getScaling();
		if (scaling.isPhysical())
			return scaling.getMaxPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return scaling.fwd(Type::call<typeMax, double>(t));
			else
				return defaultMax;
		}
	}

	/// Returns the actual or guessed minimum physical value,
	/**
	 *  Is physical range is unset, and the storage type is a "small" integer, the value is guessed.
	 */
	// ( Used at least by optical flow.)
	inline
	double requestPhysicalMin(double defaultMin = static_cast<double>(std::numeric_limits<short int>::min())) const {
		const ValueScaling & scaling = getScaling();
		if (scaling.isPhysical())
			return scaling.getMinPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return scaling.fwd(Type::call<typeMin, double>(t));
			else
				return defaultMin;
		}
	}

private:

	//drain::ValueScaling const * scalingPtr;

};


/// Struct for image (excluding data)
class ImageConf : public Encoding, public Geometry {

public:


	inline
	ImageConf(const drain::Type & t=typeid(unsigned char), size_t width=0, size_t height=0, size_t imageChannels=1, size_t alphaChannels=0) :
		Encoding(t),
		Geometry(width,height ? height : width, imageChannels, alphaChannels)
	{
	}


	inline
	ImageConf(const ImageConf & conf) : Encoding(conf), Geometry(conf), coordinatePolicy(conf.getCoordinatePolicy()) {
	}


	inline
	void setConf(const ImageConf & conf){
		setEncoding(conf);
		setGeometry(conf);
		setCoordinatePolicy(conf.getCoordinatePolicy());
	}


	ImageConf & operator=(const ImageConf & conf){
		setConf(conf);
		return *this;
	}


	/// Does not set any CoordinateHandler object.
	template <class T>
	inline
	void setCoordinatePolicy(const T & policy){
		coordinatePolicy.set(policy);
	}

	inline
	void setCoordinatePolicy(EdgePolicy::index_t xUnderFlowPolicy, EdgePolicy::index_t yUnderFlowPolicy, EdgePolicy::index_t xOverFlowPolicy, EdgePolicy::index_t yOverFlowPolicy){
		coordinatePolicy.set(xUnderFlowPolicy, yUnderFlowPolicy, xOverFlowPolicy, yOverFlowPolicy);
	}

	inline
	const CoordinatePolicy & getCoordinatePolicy() const {
		return coordinatePolicy;
	}

	/// Rules to handle under- and overflows of horizontal and vertical coordinates
	CoordinatePolicy coordinatePolicy;



};



inline
std::ostream & operator<<(std::ostream &ostr, const ImageConf & conf){

	//ostr << ' ';
	if (conf.getChannelCount() > 1)
		ostr << conf.getGeometry(); // todo w x h (1+0)
	else
		ostr << conf.area;
	ostr << ' ' << Type::getTypeChar(conf.getType()) << '@' << (conf.getElementSize()*8) << 'b';
	//const drain::ValueScaling & s = conf; // .scaling;
	const drain::ValueScaling & s = conf.getScaling(); // .scaling;
	if (s.isScaled() || s.isPhysical()){
		ostr << "*(" << s << ")";
	}
	//ostr << (conf.hasOwnScaling() ? '!' : '&');
	ostr << ' ' << 'c' << conf.coordinatePolicy;
	return ostr;
}

} // image::
} // drain::

#endif /* IMAGE_CONF_H_*/

// Drain
