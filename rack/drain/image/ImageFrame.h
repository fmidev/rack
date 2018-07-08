/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef IMAGE_FRAME_H_
#define IMAGE_FRAME_H_ "ImageFrame 0.9,  2011.09.25 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "util/CastableIterator.h"
#include "util/VariableMap.h"
#include "util/TypeUtils.h"

#include "Geometry.h"
#include "Coordinates.h"
#include "ImageLike.h"
#include "ImageScaling.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{

//extern
//drain::Log iLog;


class Channel;

/// Image with static geometry
/**
 *    ImageFrame is an image which has predefined area and channel geometry.
 *    Its pixel array is readable and writable, supporting 2D (i,j) and 3D (i,j,k) coordinates.
 *
 *    Instead of ImageFrame, one should use MultiChannel that provides extracting separate
 *    channels with getChannel() and  getAlphaChannel() .
 *
 *
 */
class ImageFrame : public ImageLike {

public:

	typedef CastableIterator const_iterator;  // TODO...
	typedef CastableIterator iterator;

	inline
	ImageFrame() : buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]), byteSize(0),
			scalingPtr(&scaling), propertiesPtr(&properties){
	};

	ImageFrame(const ImageFrame & src) : buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]), byteSize(0),
			scalingPtr(&scaling), propertiesPtr(&properties){
		scaling.set(src.getScaling());
		// copy properties? (no)
	}


	/// Returns true, if type is set.
	inline
	bool typeIsSet() const {
		return caster.typeIsSet(); //Castable::typeIsSet();
	};

	/// Returns the storage type of the image (typically unsigned char, unsigned int or float).
	/*
	 *
	 *
	 *   \see getMin()
	 *   \see getMax()
	 *   \see getLimiter()
	 *
	 */
	inline
	const std::type_info & getType() const { return caster.getType(); };

	/// Returns the type of the image as drain::Type utility class.
	inline
	drain::Type getType2() const { return drain::Type(caster.getType()); };


	/// Returns true, if type is set and is either float or double.
	// deprecating. use: type_traits is_floating_point
	inline
	bool isFloatType() const {
		return typeIsSet() && !Type::call<drain::typeIsInteger>(getType());
	};

	/// Returns true, if type is set and is either float or double.
	inline
	bool isIntegerType() const {
		return Type::call<drain::typeIsInteger>(caster.getType());
		//return Type::call<drain::typeIsInteger>(getType());
	};

	/// Returns a pointer to a static limiter function for the values of the current storage type.
	/**
	 *  \tparam T - base type, typically int or double, should be larger than the expected storage type of the image.
	 *
	 *  If storage type has been changed, the pointer should be re-assigned.
	 *
	 *   \see getMin()
	 *   \see getMax()
	 *   \see setType()
	 */
	template <class T>
	inline
	typename drain::typeLimiter<T>::value_t getLimiter() const {
		return drain::Type::call<drain::typeLimiter<T> >(caster.getType());
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
	T getMin() const {
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
	T getMax() const {
		return Type::call<typeMax, T>(caster.getType());
	}

	/// Returns the size in bytes of the storage type (1 for unsigned char, 2 for 16-bit types, etc.)
	inline
	const size_t & getByteSize() const { return byteSize; };

	/// Returns the size as pixel volume (width x height x channels).
	inline
	size_t getSize() const { return getVolume(); }; // TODO check alphas


	/// Returns iterator pointing to the first image element.
	inline
	const iterator & begin() {
		return segmentBegin;
	}

	/// Returns the iterator pointing to the element after the last element of the image buffer.
	inline
	const iterator & end() {
		return segmentEnd;
	}


	/// Returns iterator pointing to the first image element.
	inline
	const const_iterator & begin() const {
		return segmentBegin;
	}

	/// Returns the iterator pointing to the element after the last element of the image buffer.
	inline
	const const_iterator & end() const {
		return segmentEnd;
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	// TODO where is this actually needed? If in ImageViews, then should be virtual here.
	inline
	size_t address(size_t i) const {
		return (i);
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	// Note: this may change if box view is implemented in future. (viewBox width,height, offset i,j)
	inline
	size_t address(size_t i, size_t j) const {
		return ( + j * geometry.getWidth() + i);
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	inline
	size_t address(size_t i, size_t j, size_t k) const {
		return ( + k*geometry.getArea() + j*geometry.getWidth() + i);
	}

	/// Returns the pointer to the start of the image array.
	inline
	const void * getBuffer() const {
		return & buffer[0];  /// CHECK?
	};

	/// Returns the pointer to the start of the image array.
	inline
	void * getBuffer() {
		return &buffer[0];   /// CHECK?
	};



	virtual inline
	const ImageScaling & getScaling() const {
		return *scalingPtr;
	}


	// END ImageCore...

	/// Sets the intensity in location \c i to \c x. See \address
	/**
	 *  \param i - location (index)
	 *  \param i - intensity
	 */
	template <class T>
	inline
	void put(size_t i, T x){
		caster.put( & bufferPtr[address(i) * byteSize], x);  // why address(i)?
	}

	/// Sets the intensity at location \c i,j to \c x. See address().
	/**
	 *  \param i - column coordinate
	 *  \param j - row coordinate
	 *  \param x - intensity
	 */
	template <class T>
	inline
	void put(size_t i, size_t j, T x){
		caster.put( & bufferPtr[address(i,j) * byteSize], x);
	}

	/// Sets the intensity at location \c i,j,k to \c x.
	/**
	 *  \param i - column coordinate
	 *  \param j - row coordinate
	 *  \param k - channel coordinate
	 *  \param x - intensity
	 *
	 *  See address().
	 */
	template <class T>
	inline
	void put(size_t i, size_t j, size_t k, T x){
		caster.put( & bufferPtr[address(i,j,k)*byteSize], x);
	}


	/// Gets the intensity at location \c p=(i,j) .
	/**
	 *  \param p - point in two-dimensions
	 */
	template <class T, class P>
	inline
	void put(const Point2D<P> &p, T x){
		caster.put( & bufferPtr[address(p.x,p.y)*byteSize], x);
	}


	/// Put intensity using original physical value.
	/**
	 *  The value becomes scaled so that it (hopefully) fits in the limits of the storage type, say unsigned char (0...255).
	 */
	// TODO: consider virtual, with Channel::scalingPtr->inv(x) and Image::scaling.inv(x)    Could be just as fast, though...
	inline
	void putScaled(size_t i, size_t j, double x){
		caster.put( & bufferPtr[address(i,j) * byteSize], getScaling().inv(x));
	}




	/// Gets the intensity at location \c i. See address().
	/**
	 *  \param i - location (index)
	 */
	template <class T>
	inline
	T get(size_t i) const {
		return caster.get<T>( & bufferPtr[address(i)*byteSize] );
	}

	/// Gets the intensity at location \c i,j .
	/**
	 *  \param i - column coordinate
	 *  \param j - row coordinate
	 */
	template <class T>
	inline
	T get(size_t i, size_t j) const {
		return caster.get<T>( & bufferPtr[address(i,j) * byteSize ] );
	}

	/// Gets the intensity at location \c i,j,k .
	/**
	 *  \param i - column coordinate
	 *  \param j - row coordinate
	 *  \param k - channel coordinate
	 */
	template <class T>
	inline
	T get(size_t i, size_t j, size_t k) const {
		return caster.get<T>( & bufferPtr[address(i,j,k) * byteSize] );
	}

	/// Get intensity in original physical scale.
	/**
	 *  The value becomes scaled so that it (hopefully) fits in the limits of the storage type, say unsigned char (0...255).
	 */
	// TODO: consider virtual, with Channel::scalingPtr->fwd(x) and Image::scaling.fwd(x)    Could be just as fast, though...
	inline
	double getScaled(size_t i, size_t j) const {
		return getScaling().fwd(caster.get<double>( & bufferPtr[address(i,j) * byteSize ] ));
	}

	/// Gets the intensity at location \c p=(i,j) .
	/**
	 *  \param p - point in two-dimensions
	 */
	template <class T,class P>
	inline
	T get(const Point2D<P> &p) const {
		return caster.get<T>( & bufferPtr[address(p.x,p.y) * byteSize ] );
	}

public:

	template <class T, class P>
	inline
	void putPixel(const Point2D<P> &p, const std::vector<T> & pixel) const {
		for (typename std::vector<T>::size_type i = 0; i < pixel.size(); ++i) {
			caster.put( & bufferPtr[address(p.x,p.y,i)*byteSize], pixel[i]);
		}
	}


	template <class T, class P>
	inline
	void getPixel(const Point2D<P> &p, std::vector<T> & pixel) const {
		//static typename std::vector<T>::size_type i;
		for (typename std::vector<T>::size_type i = 0; i < pixel.size(); ++i) {
			pixel[i] = caster.get<T>( & bufferPtr[address(p.x,p.y,i)*byteSize]);
		}
	}

public:

	/// Sets the intensities to undetectValue. Does not change image geometry. See resetGeometry().
	inline
	void clear(){
		fill(0);
	};

	/// Sets the intensities to given value. Does not change image geometry.
	template <class T>
	inline
	void fill(T x){
		for (iterator it = segmentBegin; it != segmentEnd; ++it)
			*it = x;
	}

	/// Copies the type, geometry, coordinate policy and data of the given image.
	inline
	void copyData(const ImageFrame & src){

		Logger mout(getImgLog(), "ImageFrame", __FUNCTION__);

		if (getGeometry() != src.getGeometry()){
			mout.error() << "conflicting geometries: " << getGeometry() << " vs. " << src.getGeometry() << mout.endl;
			return;
		}

		const_iterator sit = src.begin();
		for (iterator it = begin(); it != end(); ++it,++sit)
			*it = *sit;
	}






	/*
	inline
	void setOptimalScale(double min, double max){
		scaling.setOptimalScale(getType(), min, max);
	}
	*/

	/// Returns the actual or guessed maximum physical value,
	/**
	 *   Returns the actual or guessed maximum physical value,
	 *
	 */
	inline // double defaultMax = std::numeric_limits<double>::max()
	double requestPhysicalMax(double defaultMax = static_cast<double>(std::numeric_limits<short int>::max())) const {

		const ImageScaling & s = getScaling();

		if (s.isPhysical())
			return s.getMaxPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return s.fwd(Type::call<typeMax, double>(t));
			else
				return defaultMax;
		}
	}

	/// Returns the actual or guessed maximum physical value,
	/**
	 *   Returns the actual or guessed maximum physical value,
	 *
	 */
	inline
	double requestPhysicalMin(double defaultMin = static_cast<double>(std::numeric_limits<short int>::min())) const {

		const ImageScaling & s = getScaling();

		if (s.isPhysical())
			return s.getMinPhys();
		else {
			const std::type_info & t = getType();
			if (Type::call<drain::typeIsSmallInt>(t))
				return s.fwd(Type::call<typeMin, double>(t));
			else
				return defaultMin;
		}
	}


	inline
	void setScaling(const ImageScaling &s) {
		useOwnScaling();
		scaling.set(s);
	}

	inline
	void setScaling(double scale, double offset=0.0) {
		useOwnScaling();
		scaling.setScale(scale, offset);
	}


	/// Facilitates modifications provided directly by ImageScaling object.
	ImageScaling & getScaling(){
		useOwnScaling();
		return scaling;
	}

	inline
	void useOwnScaling() {
		if (scalingPtr != &scaling){
			scaling.set(*scalingPtr); // copies also physical range
		}
		scalingPtr = &scaling;
	}


	/// Container for user-defined KEY=VALUE metadata.
	VariableMap properties;

	virtual inline
	const SmartMap<Variable> & getProperties() const {
		return *propertiesPtr;
	}


	inline
	void setName(const std::string & s){ properties["name"] = s; };

	inline
	const drain::Variable & getName() const { return properties["name"];};

	/// Prints images geometry, buffer size and type information.
	void toOStr(std::ostream &ostr = std::cout) const;

	/// Checks if images have a common memory segment.
	inline
	bool hasOverlap(const ImageFrame & image) const {
		return ((image.begin() < end()) && (image.end() > begin()));
	}

	// Todo: hasSameSegment
	inline
	bool isSame(const ImageFrame & image) const {
		return ((image.begin() == begin()) && (image.end() == end()));
	}

	/// Returns numeric channel index from "r", "g", "b", or "a" or a non-negative number in string format.
	/**
	 *   Given a numeric channel index, returns it as a number.
	 */
	size_t getChannelIndex(const std::string & index) const;

	inline
	Channel & operator[](size_t i){
		return getChannel(i);
	}

	inline
	const Channel & operator[](size_t i) const {
		return getChannel(i);
	}

	virtual
	Channel & getChannel(size_t i) = 0;

	virtual
	const Channel & getChannel(size_t i) const = 0;

	virtual
	Channel & getAlphaChannel(size_t i=0) = 0;

	virtual
	const Channel & getAlphaChannel(size_t i=0) const = 0;


protected:  // core

	std::vector<unsigned char> buffer;

	unsigned char * bufferPtr;

	iterator segmentBegin;
	iterator segmentEnd;

	Caster caster;

	// Size of the storage type (1 for 8 bits, 2 for 16 bits, etc.)
	size_t byteSize;

	/// Sets the storage type of the image - typically unsigned char, unsigned int or float. Calls setDefaultLimits().
	/* Sets the type of pixel elements of the image.
	 *
	 */
	void setStorageType(const std::type_info &type);

	inline
	void unsetType(){
		setStorageType(typeid(void));
	};

	/// Sets the image to view the data and scaling of another image.
	/**
	 *   Scaling to that of the source (src).
	 */
	void setView(const ImageFrame & src, size_t channelStart, size_t channelCount, bool catenate=false);

	/// Returns true, if the image "points to" another image. For example, channels are views.
	inline
	bool isView() const {
		return (bufferPtr != &buffer[0]);
	};  // TODO: first channel not

	ImageScaling const * scalingPtr;

	// A single channel may have specific scaling, or scaling linked to main image object.
	ImageScaling scaling;

	VariableMap const * propertiesPtr;

protected:

	/// Updates channel vector. Copies scaling of the host image.
	/**
	 *   Designed for derived classes, called by setView().
	 */
	virtual inline
	void updateChannelVector() const {};


};

inline
std::ostream & operator<<(std::ostream &ostr, const ImageFrame & src){
	src.toOStr(ostr);
	return ostr;
}



} // image::
} // drain::

#endif /* IMAGE_FRAME_H_*/
