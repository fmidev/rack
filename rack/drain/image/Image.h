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
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>


#include "util/Debug.h"
#include "util/Type.h"
#include "util/Castable.h"
#include "util/CastableIterator.h"


#include "util/VariableMap.h"

#include "Geometry.h"
#include "Point.h"
#include "Intensity.h"
#include "ImageFrame.h"



#ifndef IMAGE_BASE23
#define IMAGE_BASE23
#define DRAIN_IMAGE  "drain::Image v3.0 2016.05.14"

// using namespace std;


namespace drain {

namespace image {

extern
drain::Monitor iMonitor;

/// Class for multi-channel digital images. Supports dynamic typing with base types (char, int, float).
/**
 *   Contains an internal buffer for image data. The buffer is essentially a std::vector<char>.
 *   The size of the buffer is dynamically adjusted to the dimensions and bit resolution of the image.
 *
 *   Also, supports STL style iterators.
 */
class Image : public ImageFrame { //, protected Castable {

public:

	/// The constructor for an empty image.
	inline
	Image(const std::type_info & t = typeid(unsigned char), const Geometry & g = Geometry()) : buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//MonitorSource mout(iMonitor, "Image(T, g)", __FUNCTION__);
		setType(t);
		setGeometry(g);
		//mout.warn() << *this << mout.endl;
	};

	/// The constructor for an image of given type and size.
	inline
	Image(const std::type_info & t, size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0) :
	buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//MonitorSource mout(iMonitor, "Image(T, w,h,c,ca)", __FUNCTION__);
		setType(t);  ///
		setGeometry(width, height, channelCount, alphaChannelCount);
		//mout.warn() << *this << mout.endl;
	};

    /// The constructor for \c unsigned \c char image of given size.
	inline
	Image(size_t width, size_t height, size_t channelCount=1, size_t alphaChannelCount=0) :
	buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		//MonitorSource mout(iMonitor, "Image(w,h,c,ca)", __FUNCTION__);
		setType<unsigned char>();  ///
		setGeometry(width, height, channelCount, alphaChannelCount);
		//mout.warn() << *this << mout.endl;
	};

    /// Copies the geometry of another image.
	inline
	Image(const Image & image) :
	buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		copyShallow(image);
	};

	/// Views image, starting at its channel channelStart.
	inline
	Image(const Image & image, size_t channelStart, size_t channelCount) :
	buffer(1), bufferPtr(&buffer[0]), segmentBegin(&buffer[0]), segmentEnd(&buffer[0]) {
		setView(image,channelStart,channelCount);
	};

	typedef CastableIterator const_iterator;  // TODO...
	typedef CastableIterator iterator;


	/// Sets the type and allocates a data buffer.
	template <class T>
	void initialize(size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
		//caster.
		setType<T>();
		setGeometry(width,height,imageChannels,alphaChannels);
		//initialize(typeid(T), width, height, imageChannels, alphaChannels);
	}

	/// Sets the type and allocates a data buffer.
	inline
	//void initialize(const std::type_info &t, size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){
	void initialize(const std::type_info &t, const Geometry & geometry){
		//caster.
		setType(t);
		//setGeometry(width,height,imageChannels,alphaChannels);
		setGeometry(geometry);
	}

	/// Sets the type, resizing the internal buffer if needed.
	template <class T>
	void initialize(){
		initialize<T>(geometry.getWidth(), geometry.getHeight(), geometry.getImageChannelCount(), geometry.getAlphaChannelCount());
	}



	/// Sets the storage type of the image - typically unsigned char, unsigned int or float. Calls setDefaultLimits().
	/* Sets the type of pixel elements of the image.
	 *
	 */
	inline
	void setType(const std::type_info &type){
		MonitorSource mout(iMonitor, "Image", __FUNCTION__);
		if (isView()){
			if (type != getType()){
				mout.error() << "tried to change type of a view (" << getType().name() << ") <- " << type.name() << mout.endl;
			}
			return;
		}
		else {
			if (type == typeid(void)){
				unsetType();
			}
			else if (type == typeid(bool)){
				mout.warn() << "storage type 'bool' not supported, using 'unsigned char'" << mout.endl;
				setType(typeid(unsigned char));
			}
			else if (type == typeid(std::string)){
				mout.error() << "storage type 'std::string' not applicable images" << mout.endl;
				//setType(typeid(unsigned char));
			}
			else {
				caster.setType(type);
				byteSize = caster.getByteSize();
				segmentBegin.setType(type);
				segmentEnd.setType(type);
				setDefaultLimits(); // FIXME risk  0...1 for floats?!?
			}
		}
	};

	/// Sets the storage type of the image - typically unsigned char, unsigned int or float.
	template <class T>
	inline
	void setType(){
		//Castable::setType(typeid(T));
		setType(typeid(T));
	}

	/// Sets type of the image, applying character mapping of Type::setType(char c) .  TODO: drain::Type
	/**

	 */
	template <class T>
	inline
	void setType(const T & t){
		setType(drain::Type::getTypeInfo(t));
	}
	/*
	void setType(char t){
		setType(drain::Type::getTypeInfo(t));
	}
	*/

	inline void unsetType(){
		if (isView())
			throw std::runtime_error("unsetType: illegal for a view");
		caster.unsetType();
	};

	/// Returns the storage type of the image - typically unsigned char, unsigned int or float.
	inline
	const std::type_info &getType() const { return caster.getType(); };

	inline
	drain::Type getType2() const { return drain::Type(caster.getType()); };


	/// Returns the size of intensity object as bytes, typic
	//template <class T>
	//void get(const int &i,const int &j){};

	inline
	const size_t & getByteSize() const { return byteSize; };

	/// Returns true, if type is set.
	inline
	bool typeIsSet() const {
		return caster.typeIsSet(); //Castable::typeIsSet();
	};

	/// Returns true, if type is set and is either float or double.
	// deprecating. use: type_traits is_floating_point
	inline
	bool isFloatType() const {
		//return Type::isFloatType(getType());
		return typeIsSet() && !Type::isIntegralType(getType());
		//return typeIsSet() && (getType()==typeid(float) || getType()==typeid(double));
	};

	/// Returns true, if type is set and is either float or double.
	// TODO: leave it to Type::
	inline
	bool isIntegerType() const {
		return Type::isIntegralType(getType());
		// return typeIsSet() && (getType()!=typeid(float) && getType()!=typeid(double));
	};

	/// Resizes the image, keeps the current type. \see initialize().
	/**
	   \return true if geometry was changed, otherwise false.

	    The buffer will always have at least one element to guarantee a safe call to begin().

	    \see CmdGeometry

	 */
	inline virtual
	bool setGeometry(size_t width, size_t height, size_t imageChannels=1, size_t alphaChannels=0){

		if (!isView()){

			const bool result = geometry.setGeometry(width,height,imageChannels,alphaChannels);

			const size_t s = geometry.getVolume() * byteSize;

			if (s > 0)
				buffer.resize(s);
			else
				buffer.resize(1);

			bufferPtr = &buffer[0];
			segmentBegin = (void *)&(*buffer.begin());
			segmentEnd   = (void *)&(*buffer.end());
			channelVector.clear();

			return result;
		}
		else {
			const Geometry g(width,height,imageChannels,alphaChannels);
			if ((geometry == g)){  // pairwise check
				return true;
			}
			else {
				MonitorSource mout(iMonitor, "Image", __FUNCTION__);
				mout.error() << "Attempt to change geometry of View " << g << " to: " << geometry << mout.endl;
				return false;
			}

		}
	}

	/// Resizes the image, keeps the current type. \see initialize().
	inline
	bool setGeometry(const Geometry &g){
		return setGeometry(g.getWidth(), g.getHeight(), g.getImageChannelCount(), g.getAlphaChannelCount());
	};

	/// Collapses the image to undetectValue size, releasing memory. @see clear().
	void resetGeometry(){
		setGeometry(0,0,0,0);
	}


	inline
	void setChannelCount(size_t ni, size_t na = 0){
		setGeometry(getWidth(), getHeight(), ni, na);
	};


	inline
	void setAlphaChannelCount(size_t k){
		setGeometry(getWidth(), getHeight(), getImageChannelCount(), k);
	};



	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	//template <class T>
	inline
	size_t address(const size_t &i) const {
		return (i);
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 */
	//template <class T>
	inline
	size_t address(const size_t &i, const size_t &j) const {
		return ( + j * geometry.getWidth() + i);
	}

	/// Computes the index location from image coordinates. Does not involve bit resolution.
	/** Practical in precomputing a location when several images are processed in parallel.
	 *
	 *  \T T - should be integral type.
	 */
	//template <class T>
	inline
	size_t address(const size_t &i, const size_t &j, const size_t &k) const {
		return ( + k*geometry.getArea() + j*geometry.getWidth() + i);
	}

	/// Sets the intensity in location \c i to \c x. See \address
	/**
	 *  \param i - location (index)
	 *  \param i - intensity
	 */
	template <class T>
	inline
	void put(const size_t &i, const T & x){
		caster.put( & bufferPtr[address(i) * byteSize], x);
	}

	/// Sets the intensity at location \c i,j to \c x. See address().
	/**
	 *  \param i - coordinate
	 *  \param j - coordinate
	 *  \param x - intensity
	 */
	template <class T>
	inline
	void put(const size_t &i, const size_t &j, const T & x){
		caster.put( & bufferPtr[address(i,j) * byteSize], x);
	}

	/// Sets the intensity at location \c i,j,k to \c x.
	/**
	 *  \param i - coordinate
	 *  \param j - coordinate
	 *  \param k - coordinate
	 *  \param x - intensity
	 *
	 *  See address().
	 */
	template <class T>
	inline
	void put(const size_t &i, const size_t &j, const size_t &k, const T & x){
		// std::cerr << i << ',' << j << ',' << k << ',' << address(i,j,k)*byteSize << '\n';
		// std::cerr << ' ' << (size_t)&buffer[0] << ' ' << (size_t)bufferPtr << '\n';
		caster.put( & bufferPtr[address(i,j,k)*byteSize], x);
	}



	/// Gets the intensity at location \c p=(i,j) .
	/**
	 *  \param p - point in two-dimensions
	 */
	template <class T, class P>
	inline
	void put(const Point2D<P> &p, const T & x){
		caster.put( & bufferPtr[address(p.x,p.y)*byteSize], x);
	}

	template <class T, class P>
	inline
	void putPixel(const Point2D<P> &p, const std::vector<T> & pixel) const {
		static typename std::vector<T>::size_type i;
		for (i = 0; i < pixel.size(); ++i) {
			caster.put( & bufferPtr[address(p.x,p.y,i)*byteSize], pixel[i]);
		}
	}


	/// Gets the intensity at location \c i. See address().
	/**
	 *  \param i - location (index)
	 */
	template <class T>
	inline
	T get(const size_t &i) const {
		return caster.get<T>( & bufferPtr[address(i)*byteSize] );
	}

	/// Gets the intensity at location \c i,j .
	/**
	 *  \param i - coordinate
	 *  \param j - coordinate
	 */
	template <class T>
	inline
	T get(const size_t &i, const size_t &j) const {
		return caster.get<T>( & bufferPtr[address(i,j) * byteSize ] );
	}

	/// Gets the intensity at location \c i,j,k .
	/**
	 *  \param i - coordinate
	 *  \param j - coordinate
	 *  \param k - coordinate
	 */
	template <class T>
	inline
	T get(const size_t &i, const size_t &j, const size_t &k) const {
		return caster.get<T>( & bufferPtr[address(i,j,k) * byteSize] );
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

	template <class T, class P>
	inline
	void getPixel(const Point2D<P> &p, std::vector<T> & pixel) const {
		static typename std::vector<T>::size_type i;
		for (i = 0; i < pixel.size(); ++i) {
			pixel[i] = caster.get<T>( & bufferPtr[address(p.x,p.y,i)*byteSize]);
		}
	}


	Image & getChannel(const size_t &i);
	const Image & getChannel(const size_t &i) const;

	Image & getAlphaChannel();
	const Image & getAlphaChannel() const;


	const std::vector<Image> & getChannelVector() const { updateChannelVector(); return channelVector; };
	//std::vector<Image> & getChannelVector() { _updateChannelVector(); return channelVector; };




	template <class T>
	inline
	T getMin() const { return static_cast<T>(minValue); }

	template <class T>
	inline
	T getMax() const { return static_cast<T>(maxValue); }

	inline
	double getScale() const { return (scale); }

	/// If the intensities of the image correspond to a physical value (like temperature), then the scaling coefficient should be set with this function.
	inline
	void setScale(double scale){ this->scale = scale; }

	/// If the intensities of the image correspond to an absolute value (like count) then the scale should be reset to unity with this function.
	inline
	void useAbsoluteScale(){ this->scale = 1; }


	/// Sets the default min and max values for intensities; 0.0 and 1.0 for floats.
	inline
	void setDefaultLimits(){
		if (isFloatType()){
			minValue = 0.0;
			maxValue = 1.0;
			scale = 1.0;
		}
		else {
			minValue = Type::getMin<double>(getType2());
			maxValue = Type::getMax<double>(getType2());
			scale = 1.0/maxValue;
		}
	};

	// TODO: semantics. Does not set scale? Rename to: setRange ?
	inline
	void setLimits(const double minValue, const double maxValue){
		this->minValue = minValue;
		this->maxValue = maxValue;
	};

	// TODO: _minF ... separate int and float handling
	template <class T>
	inline
	T limit(double x) const {
		if (x > maxValue)
			return static_cast<T>(maxValue);
		else if (x < minValue)
			return static_cast<T>(minValue);
		else
			return static_cast<T>(x);
	}

	template <class T>
	inline
	T limit(int x) const {
		if (x > maxValue)
			return static_cast<T>(maxValue);
		else if (x < minValue)
			return static_cast<T>(minValue);
		else
			return static_cast<T>(x);
	}



	/// Returns the size (width x height x channels).
	// TODO: needed?
	inline
	const size_t getSize() const { return getVolume(); }; // TODO check alphas



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

	/// Returns the pointer to the start of the image array.
	inline
	const void * getBuffer() const {
		return &buffer[0];  /// CHECK?
	};

	/// Returns the pointer to the start of the image array.
	inline
	const void * getBufferCONST() const {  // TODO REMOVE
		return &(buffer[0]);  /// CHECK?
	};

	/// Returns the pointer to the start of the image array.
	inline
	void * getBuffer() {
		return &buffer[0];   /// CHECK?
	};

	/// Returns the pointer to the start of the image array.
	/*
	inline
	void * getBufferNEW() {
		return &(*segmentBegin);   /// CHECK?
	};
	*/

	/// Sets the intensities to undetectValue. Does not change image geometry. See resetGeometry().
	inline
	void clear(){
		fill(0);
	};

	/// Sets the intensities to given value. Does not change image geometry.
	template <class T>
	inline void fill(T x){
		for (iterator it = segmentBegin; it != segmentEnd; ++it)
			*it = x;
	}

	/// Copies the type and geometry of the given image. Does not copy the data.
	inline void copyShallow(const Image & src){
		setType(src.getType());
		setGeometry(src.getGeometry());
		setCoordinatePolicy(src.getCoordinatePolicy());
	}

	/// Copies the type, geometry and data of the given image.
	inline void copyDeep(const Image &src){
		copyShallow(src);
		const_iterator sit = src.begin();
		for (iterator it = begin(); it != end(); ++it,++sit)
			*it = *sit;
	}


	/// Returns true, if the image "points to" another image. For example, channels are views.
	inline
	bool isView() const { return (bufferPtr != &buffer[0]); };

	/// Views the whole image
	inline
	void setView(const Image & target){
		setView(target,0,target.getChannelCount());
	}

	/// Views a single channel. Regardless of target channel type (image or alpha), resulting channel type is image.
	/**
	 *  If you want the type to be alpha channel, call setView(target, imageChannelCount, 1)
	 */
	inline
	void setView(const Image & target, size_t channel){
		setView(target,channel,1);
		geometry.setChannelCount(1,0);
	}

	/// Sets the image to view another image.
	/**
	 */
	void setView(const Image & target, size_t channelStart, size_t channelCount, bool catenate=false);

	///  Sets the image to view another image .
	void setView(const Image & src, const std::string & view);

	size_t getChannelIndex(const std::string & index) const;

	/// Prints images geometry, buffer size and type information.
	void info(std::ostream &ostr = std::cout) const;

	/// Prints images geometry, buffer size and type information, and dumps the array contents. Consider info...
	void dump(std::ostream &ostr = std::cout) const;

	/// Container for user-defined KEY=VALUE metadata.
	VariableMap properties;


	/// Checks if images have a common memory segment.
	inline
	bool hasOverlap(const Image & image) const {
		return ((image.begin() < end()) && (image.end() > begin()));
	}

	// Todo: hasSameSegment
	inline
	bool isSame(const Image & image) const {
		return ((image.begin() == begin()) && (image.end() == end()));
	}



	inline
	void setName(const std::string &s){ name = s; };

	inline
	const std::string & getName() const { return name;};

	// Unprotected?
	std::vector<unsigned char> buffer;

protected:



	void updateChannelVector() const;

	Caster caster;
	unsigned char * bufferPtr;
	size_t byteSize;

	mutable std::vector<Image> channelVector;

	//size_t _offset;

	iterator segmentBegin;
	iterator segmentEnd;

	double minValue;
	double maxValue;
	double scale;

	std::string name;

	// Intensity::scalingMode _scalingMode;  // experimental
};

inline
std::ostream & operator<<(std::ostream &ostr, const Image &image){
	image.info(ostr);
	//ostr << image.getWidth() << 'X' << image.getHeight() << '\n';
	return ostr;
}


//typedef ImageList std::list<Image &>;


}  // image
}  // drain




/*
std::ostream & operator<<(std::ostream &ostr, const Image &image){
  ostr << "Image size: " << image.width << 'x' << image.height << " buffer: "<< image.buffer.size() << '\n';
  ostr << "Buffer type: " << image.getType().name() << "(" << image.getByteSize() << "byte=" << (image.getByteSize()*8) << "bit) \n";
  for (size_t j=0; j<image.height; j++){
    ostr << "Row "<< j << '\n';
    for (size_t i=0; i<image.width; i++)
      //ostr << i << '|' << ' '; // << image.get<double>(i,j) << '\t';
      ostr << image.get<double>(i,j) <<'\t';
    ostr << '\n';
  };
  //std::cerr << " adios \n";// << std::endl;
  return ostr;
}
 */

#endif

// Drain
