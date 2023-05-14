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
#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <iostream>
#include <vector>

#include "drain/util/UniTuple.h"
#include "drain/util/Frame.h"

namespace drain
{

namespace image
{



class ChannelGeometry : public drain::UniTuple<size_t,2>
{
public:

	inline
	ChannelGeometry(size_t images=1, size_t alphas=0) : imageChannels(next()), alphaChannels(next()){ //, channels(0) {
		this->set(images, alphas);
	};

	ChannelGeometry(const ChannelGeometry & g) : imageChannels(next()), alphaChannels(next()){ //, channels(0) {
		this->set(g.tuple());
	};

	template <size_t N>
	ChannelGeometry(drain::UniTuple<size_t,N> & tuple, size_t i) :
	drain::UniTuple<size_t,2>(tuple, i),
	imageChannels(this->next()),
	alphaChannels(this->next()){
		//updateTuple();
	};

	inline
	virtual ~ChannelGeometry(){};


	inline
	ChannelGeometry & operator=(const ChannelGeometry & g){
		set(1,0); // could be init!
		assign(g);
		return *this;
	}

	/*
	template <class T>
	inline
	ChannelGeometry & operator=(const T & g){
		set(1,0); // could be init!
		assign(g);
		return *this;
	}
	*/


	inline
    void setChannelCount(const ChannelGeometry & g){
		assign(g);
	}

	inline
    void setChannelCount(size_t imageChannelCount, size_t alphaChannelCount = 0){
    	this->set(imageChannelCount, alphaChannelCount);
    }

    inline
    void setAlphaChannelCount(size_t a){
    	alphaChannels = a;
    }

    /// Return the number of channels (image and alpha)
	inline
	size_t getChannelCount() const {
		return imageChannels + alphaChannels;
	};


    /// Return the number of actual image channels (excluding alphas)
	inline
	size_t getImageChannelCount() const {
		return imageChannels;
	};

    /// Return the number of alpha channels
	inline
	size_t getAlphaChannelCount() const {
		return alphaChannels;
	};

protected:

    size_t & imageChannels;
    size_t & alphaChannels;

};


typedef Frame2D<size_t> AreaGeometry;


/*! The basic idea is to encode dimensions directly as a std::vector<int>; the number of elements is 
 *  the number of dimensions. Each element states the discrete coordinate space.
 * 
 * 
 *  For example, a three-channel image having width 640 and height 400 would have dimension vector 
 *  [640,400,3]. The coordinates will be: x \in {0,1,...,639}, y \in {0,1,...,399} and z \in {0,1,2}.
 *  The size of the vector of unlimited, allowing hypermatrices of any size. 
 * 
 */
class Geometry : public drain::UniTuple<size_t,4> { //: public AreaGeometry, public ChannelGeometry {

public:

	AreaGeometry area;

	ChannelGeometry channels;

	// condider:
	// CoordinatePolicy policy?

	/// Constructor with dimensions. Channel count is one by default, allowing construction with width and height.
    /**
	 */
    Geometry(size_t width=0, size_t height=0, size_t channels=1, size_t alphas=0) : area(tuple(),0), channels(tuple(),2) {
    	set(width, height, channels, alphas);
    	//updateTuple();
    }
	
    Geometry(const Geometry &g) : area(tuple(),0), channels(tuple(),2) {
    	assign(g.tuple());
    	//updateTuple();
    }

	//Geometry(const std::vector<int> &vector, int a = 0); 

    virtual inline
	~Geometry(){};

    virtual inline // CHECK if virtual needed
	const Geometry & getGeometry() const {
    	return *this;
    }

    virtual inline // CHECK if virtual needed
	Geometry & getGeometry() {
    	return *this;
    }

	template <class T>
	inline
	void setGeometry(const T &g){
		//Geometry & operator=(const T & g){
		set(0,0,1,0); // could be init!
		assign(g);
		//return *this;
	}

	inline
    void setGeometry(const Geometry &g){
    	//setGeometry(g.width, g.height, g.imageChannelCount, g.alphaChannelCount);
    	set(g);
    }
    
    //bool
    void setGeometry(size_t width, size_t height, size_t imageChannelCount = 1,size_t alphaChannelCount = 0){
    	set(width, height, imageChannelCount, alphaChannelCount);
    }
    
    inline
    operator const AreaGeometry &() const {
    	return area;
    }

	
	inline
	size_t getWidth() const {
		return area.getWidth();
	};

	inline
	size_t getHeight() const {
		return area.getHeight();
	};

	template <class T>
	inline
	void setArea(const T & a){
		area.assign(a);
	};

	inline
	void setArea(size_t width, size_t height){
		area.set(width, height);
	};

	inline
	size_t getArea() const {
		return area.getArea();
	};


	/// Set...
	// template <class T>
	/*
	virtual inline
	void setGeometry(const Geometry & g){
		geometry.setGeometry(g);
	}
	*/

	inline
	size_t getChannelCount() const {
		return channels.getChannelCount(); // getGeometry unneeded?
	};

	inline
	void setChannelCount(size_t imageChannels, size_t alphaChannels){
		channels.set(imageChannels, alphaChannels);
	};

	inline
	void setChannelCount(const ChannelGeometry & geom){
		channels.setChannelCount(geom);
	};

	inline
	const size_t getImageChannelCount() const {
		return channels.getImageChannelCount();
	};

	inline
	void setImageChannelCount(size_t imageChannels){
		channels.set(imageChannels);
	};

	inline
	const size_t getAlphaChannelCount() const {
		return channels.getAlphaChannelCount();
	};

	inline
	void setAlphaChannelCount(size_t alphaChannels){
		channels.setAlphaChannelCount(alphaChannels);
	};


	inline
	bool hasAlphaChannel() const {
		return channels.getAlphaChannelCount() > 0;
	};

	/// Returns numeric channel index from "r", "g", "b", or "a" or a non-negative number in string format.
	/**
	 *   Given a numeric channel index, returns it as a number.
	 */
	size_t getChannelIndex(const std::string & index) const;



	inline
	size_t getVolume() const {
		return area.getArea() * channels.getChannelCount();
	};

	inline
	bool isEmpty() const {
		return (getVolume() == 0);
	};



    // to-be-protected? :
	/*
	inline
	Geometry & operator=(const Geometry &g){
		assign(g);
		return *this;
	}

	// needed?
    template <class T>
    inline
    Geometry & operator=(const std::vector<T> &v){
    	assign(v);
    	return *this;
    }
    */
    
    //inline
    //Geometry & operator=(const Geometry &g);
    inline
    bool operator==(const Geometry &g) const {
    	return (tuple() == g.tuple());
    	/*
    	return ((width==g.getWidth()) && (height==g.getHeight())
    			&& (imageChannelCount==g.getImageChannelCount()) && (alphaChannelCount==g.getAlphaChannelCount()));
    	*/
    };

    inline
    bool operator!=(const Geometry &g) const {
    	return !((*this)==g);
    };
    
    /*
    inline
    void toStream(std::ostream &ostr) const {
    	AreaGeometry::toStream(ostr);
    	if (getChannelCount() != 1){
    		ostr << "×";
    		ChannelGeometry::toOStr(ostr);
		}
	}
	*/

    /*
    inline
    std::string  toString(std::string & s) const {
    	return this->tuple().toStr();
    }
    */


protected:

    /// area and volume
    /*
    virtual inline
    void updateTuple(){
    	area.updateTuple();
    	channels.updateTuple();
    }
    */

    //size_t volume;

    	
};



/// Not ready.
/*
template <class T>
Geometry & Geometry::operator=(const std::vector<T> &v){
	int d = v.size();
	imageChannelCount = 1;
	alphaChannelCount = 0;
	switch (d) {
		case 4:
			alphaChannelCount = v[3];
			break;
		case 3:
			imageChannelCount = v[2];
			break;
		case 2:
			this->height = v[1];
			break;
		case 1:
			this->width = v[0];
			break;
		default:
			break;
	}
	channelCount = imageChannelCount + alphaChannelCount;
	update();
	return (*this);
}
    */
    
/*
inline
std::ostream & operator<<(std::ostream &ostr, const Geometry &geometry){
	geometry.toOStr(ostr);
	return ostr;
}
*/
    

}

}

#endif /*GEOMETRY_H_*/

// Drain
