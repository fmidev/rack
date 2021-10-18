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
#ifndef IMAGE_FRAME_H_
#define IMAGE_FRAME_H_ "ImageFrame 0.9,  2011.09.25 Markus.Peura@fmi.fi"

#include <stddef.h>  // size_t

#include "drain/util/CastableIterator.h"
#include "drain/util/Sprinter.h"
#include "drain/util/TypeUtils.h"
#include "drain/util/VariableMap.h"
#include "drain/util/ValueScaling.h"

#include "Geometry.h"
#include "Coordinates.h"
#include "ImageLike.h"

namespace drain
{

/// Namespace for images and image processing tools.
namespace image
{

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
	ImageFrame() : propertiesPtr(&properties) { //, scalingPtr(&conf.getScaling()) {
		adjustBuffer();
		init();
	};

	inline
	ImageFrame(const ImageFrame & src) : ImageLike(src), propertiesPtr(&properties) { // , scalingPtr(&conf.getScaling()) {
		adjustBuffer();
		init();
		// copy properties? (no)
	}


	virtual inline
	const ImageConf & getConf() const {
		return conf; // *this;
	}

	/*
	inline
	ImageConf & getConf() {
		return conf; // *this;
	}
	*/

	/// Sets the supported range for physical values and optionally adjusts the scaling for maximal resolution.
	inline
	void setPhysicalRange(const Range<double> &range, bool rescale=false){
		conf.setPhysicalRange(range, rescale);
	}

	/// Sets the supported range for physical values and optionally adjusts the scaling for maximal resolution.
	inline
	void setPhysicalRange(double min, double max, bool rescale=false){
		conf.setPhysicalRange(min, max, rescale);
	}


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
		return ( + j * conf.area.getWidth() + i);
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	inline
	size_t address(size_t i, size_t j, size_t k) const {
		return ( + k*conf.area.getArea() + j*conf.area.getWidth() + i);
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

	// END ImageCore...

	/// Sets the intensity in location \c i to \c x. See \address
	/**
	 *  \param i - location (index)
	 *  \param i - intensity
	 */
	template <class T>
	inline
	void put(size_t i, T x){
		conf.caster.put( & bufferPtr[address(i) * conf.byteSize], x);  // why address(i)?
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
		conf.caster.put( & bufferPtr[address(i,j) * conf.byteSize], x);
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
		conf.caster.put( & bufferPtr[address(i,j,k)*conf.byteSize], x);
	}


	/// Gets the intensity at location \c p=(i,j) .
	/**
	 *  \param p - point in two-dimensions
	 */
	template <class T, class P>
	inline
	void put(const Point2D<P> &p, T x){
		conf.caster.put( & bufferPtr[address(p.x,p.y)*conf.byteSize], x);
	}


	/// Put intensity using original physical value.
	/**
	 *  The value becomes scaled so that it (hopefully) fits in the limits of the storage type, say unsigned char (0...255).
	 */
	// TODO: consider virtual, with Channel::scalingPtr->inv(x) and Image::scaling.inv(x)    Could be just as fast, though...
	inline
	void putScaled(size_t i, size_t j, double x){
		conf.caster.put( & bufferPtr[address(i,j) * conf.byteSize], getScaling().inv(x));
	}




	/// Gets the intensity at location \c i. See address().
	/**
	 *  \param i - location (index)
	 */
	template <class T>
	inline
	T get(size_t i) const {
		return conf.caster.get<T>( & bufferPtr[address(i)*conf.byteSize] );
	}

	/// Gets the intensity at location \c i,j .
	/**
	 *  \param i - column coordinate
	 *  \param j - row coordinate
	 */
	template <class T>
	inline
	T get(size_t i, size_t j) const {
		return conf.caster.get<T>( & bufferPtr[address(i,j) * conf.byteSize ] );
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
		return conf.caster.get<T>( & bufferPtr[address(i,j,k) * conf.byteSize] );
	}

	/// Get intensity in original physical scale.
	/**
	 *  The value becomes scaled so that it (hopefully) fits in the limits of the storage type, say unsigned char (0...255).
	 */
	// TODO: consider virtual, with Channel::scalingPtr->fwd(x) and Image::scaling.fwd(x)    Could be just as fast, though...
	inline
	double getScaled(size_t i, size_t j) const {
		return getScaling().fwd(conf.caster.get<double>( & bufferPtr[address(i,j) * conf.byteSize ] ));
	}

	/// Gets the intensity at location \c p=(i,j) .
	/**
	 *  \param p - point in two-dimensions
	 */
	template <class T,class P>
	inline
	T get(const Point2D<P> &p) const {
		return conf.caster.get<T>( & bufferPtr[address(p.x,p.y) * conf.byteSize ] );
	}


	template <class T, class P>
	inline
	void putPixel(const Point2D<P> &p, const std::vector<T> & pixel) const {
		for (typename std::vector<T>::size_type i = 0; i < pixel.size(); ++i) {
			conf.caster.put( & bufferPtr[address(p.x,p.y,i)*conf.byteSize], pixel[i]);
		}
	}


	template <class T, class P>
	inline
	void getPixel(const Point2D<P> &p, std::vector<T> & pixel) const {
		for (typename std::vector<T>::size_type i = 0; i < pixel.size(); ++i) {
			pixel[i] = conf.caster.get<T>( & bufferPtr[address(p.x,p.y,i)*conf.byteSize]);
		}
	}


	/// Sets the intensities to zero. Does not change image geometry. See resetGeometry().
	// TODO: what about value corresponding to physical min?
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

	/// Copies data. Does not change encoding, geometry, or coordinate policy.
	void copyData(const ImageFrame & src);


	/// Checks if images have a common memory segment.
	inline
	bool hasOverlap(const ImageFrame & image) const {
		return ((image.begin() < end()) && (image.end() > begin()));
	}

	/// Return true, if both frames have same type and are using the same data segment.
	/*
	inline
	bool isSame(const ImageFrame & image) const {
		return (getType() == image.getType()) && hasSameSegment(image);
	}
	*/

	/// Return true, if both frames are using the same data segment.
	/**
	 *   When calling this, consider calling isView() as well.
	 *
	 *   Note that two frames viewing the same segment should also have the same type.
	 *
	 */
	inline
	bool hasSameSegment(const ImageFrame & image) const {
		return ((image.begin() == begin()) && (image.end() == end()));
	}




	/// Container for user-defined KEY=VALUE metadata.
	FlexVariableMap properties;

	virtual inline
	const FlexVariableMap & getProperties() const {
		return *propertiesPtr;
	}

	/*
	virtual inline
	FlexVariableMap & getProperties() {
		return *propertiesPtr;
	}
	*/


	inline
	void setName(const std::string & s){ name = s; };

	inline
	const std::string & getName() const { return name; };

	/// Prints images geometry, buffer size and type information.
	void toOStr(std::ostream &ostr = std::cout) const;

	virtual
	Channel & getChannel(size_t i) = 0;

	virtual
	const Channel & getChannel(size_t i) const = 0;

	virtual
	Channel & getAlphaChannel(size_t i=0) = 0;

	virtual
	const Channel & getAlphaChannel(size_t i=0) const = 0;


	/// Returns numeric channel index from "r", "g", "b", or "a" or a non-negative number in string format.
	/**
	 *   Given a numeric channel index, returns it as a number.
	 */
	//size_t getChannelIndex(const std::string & index) const;

	/// Sets the type and allocates a data buffer.
	//  Experimental for ImageFrame
	virtual inline
	void initialize(const std::type_info &t, const Geometry & geometry){

		if ((conf.getType() == t) && (conf.getGeometry() == geometry))
			return;
		else
			std::runtime_error(std::string("ImageFrame::")+__FUNCTION__+": tried to change ImageFrame geometry");

		// initialize(t, geometry.getWidth(), geometry.getHeight(), geometry.getImageChannelCount(), geometry.getAlphaChannelCount());
	}

	/// Sets the type and allocates a data buffer.
	//  Experimental for ImageFrame (does not change image; throws exception if change requested)
	virtual inline
	void initialize(const std::type_info &t, size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		 initialize(t, Geometry(width, height, imageChannels, alphaChannels));
	}


protected:

	void init();

	/// Resize data buffer to match the geometry.
	void adjustBuffer();

	//std::vector<unsigned char> buffer;

	unsigned char * bufferPtr;

	iterator segmentBegin;
	iterator segmentEnd;

	// Caster caster;

	// Size of the storage type (1 for 8 bits, 2 for 16 bits, etc.)
	//size_t byteSize;

	/// Sets the storage type of the image - typically unsigned char, unsigned int or float. Calls setDefaultLimits().
	/* Sets the type of pixel elements of the image.
	 *
	 */
	void setStorageType(const std::type_info &type);

	inline
	void unsetType(){
		throw std::runtime_error("ImageFrame infinite loop: -> unsetType() ?");
		//setStorageType(typeid(void));
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


protected:

	FlexVariableMap const * propertiesPtr;


	/// Updates channel vector. Copies scaling of the host image.
	/**
	 *   Designed for derived classes, called by setView().
	 */
	virtual inline
	void updateChannelVector() const {};

	std::string name;

	template <class T>
	inline
	T * retrieve(size_t a){
		return (T*) & buffer[ a*conf.byteSize ];
	}

	template <class T>
	inline
	const T * retrieve(size_t a) const {
		return (const T*) & buffer[ a*conf.byteSize ];
	}


	std::vector<unsigned char> buffer; // non-private, for Image::swap().

private:

	//drain::ValueScaling const * scalingPtr;

};



inline
std::ostream & operator<<(std::ostream &ostr, const ImageFrame & src){
	src.toOStr(ostr);
	return ostr;
}



} // image::

template <>
std::ostream & drain::SprinterBase::toStream<drain::image::ImageFrame>(std::ostream & ostr, const drain::image::ImageFrame & src, const SprinterLayout & layout);

} // drain::

#endif /* IMAGE_FRAME_H_*/

// Drain
