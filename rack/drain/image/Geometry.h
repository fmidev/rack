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

namespace drain
{

namespace image
{

/// Rectangle of width x height .
/*
struct AreaGeometryStruct {

	inline
	AreaGeometryStruct() : width(0), height(0), area(0){};

	size_t width;
    size_t height;
    size_t area;
};
*/

/// Rectangle of width x height .
class AreaGeometry { //: protected AreaGeometryStruct {

public:

	//inline
	//AreaGeometry() : width(0), height(0), area(0){};

	//AreaGeometry(size_t width, size_t height=1) : width(width), height(height), area(width*height){};
	inline
	AreaGeometry(size_t width=0, size_t height=0)  : width(0), height(0), area(0){
		setArea(width, height?height:width);
	};

	/*
	inline
	operator const AreaGeometryStruct & () const {
		return *this;
	}

	inline
	const AreaGeometryStruct & getAreaGeometryStruct() const {
		return *this;
	}
	*/

    void setWidth(size_t width);

    void setHeight(size_t height);

    void setArea(size_t width, size_t height);

    void setArea(const AreaGeometry & g){
    	setArea(g.getWidth(), g.getHeight());
    }

    inline
    size_t getWidth() const { return width; };

    inline
    size_t getHeight() const { return height; };

	inline
	size_t getArea() const { return area; };

	inline
	void toOStr(std::ostream &ostr) const {
		ostr << getWidth() << "×" << getHeight();
	}

	size_t width;
    size_t height;

protected:

    size_t area;

	/*
	size_t width;
    size_t height;
    size_t area;
	*/

    virtual
    void update();


};

inline
std::ostream & operator<<(std::ostream &ostr, const AreaGeometry & g){
	g.toOStr(ostr);
	return ostr;
}


class ChannelGeometry
{
public:

	inline
	ChannelGeometry() : imageChannelCount(0), alphaChannelCount(0), channelCount(0) {};

    void setChannelCount(size_t imageChannelCount, size_t alphaChannelCount = 0);
    void setAlphaChannelCount(size_t alphaChannelCount);

	inline
	size_t getChannelCount() const { return channelCount; };

	inline
	size_t getImageChannelCount() const { return imageChannelCount; };

	inline
	size_t getAlphaChannelCount() const { return alphaChannelCount; };

	inline
	void toOStr(std::ostream &ostr) const {
		if (getAlphaChannelCount())
			ostr << '(' << getImageChannelCount() << '+' << getAlphaChannelCount() << ')';
		else
			ostr << getImageChannelCount();
	}

protected:

    size_t imageChannelCount;
    size_t alphaChannelCount;
    size_t channelCount;

    virtual
    void update();

};

inline
std::ostream & operator<<(std::ostream &ostr, const ChannelGeometry & g){
	g.toOStr(ostr);
	return ostr;
}

/*! The basic idea is to encode dimensions directly as a std::vector<int>; the number of elements is 
 *  the number of dimensions. Each element states the discrete coordinate space.
 * 
 * 
 *  For example, a three-channel image having width 640 and height 400 would have dimension vector 
 *  [640,400,3]. The coordinates will be: x \in {0,1,...,639}, y \in {0,1,...,399} and z \in {0,1,2}.
 *  The size of the vector of unlimited, allowing hypermatrices of any size. 
 * 
 */
class Geometry : public AreaGeometry, public ChannelGeometry {

public:

	/// Constructor with dimensions. Channel count is one by default, allowing construction with width and height.
    /**
	 */
    Geometry(size_t width=0, size_t height=0, size_t channelCount=1, size_t alphaChannelCount=0);
	
    Geometry(const Geometry &g);

	//Geometry(const std::vector<int> &vector, int a = 0); 

    virtual ~Geometry();

	//bool
    inline
    void setGeometry(const Geometry &g){
    	setGeometry(g.width, g.height, g.imageChannelCount, g.alphaChannelCount);
    }
    
    //bool
    void setGeometry(size_t width, size_t height, size_t imageChannelCount = 1,size_t alphaChannelCount = 0);
    
	
	inline
	size_t getVolume() const { return volume; };

    // to-be-protected? :


	// needed?
    template <class T>
    Geometry & operator=(const std::vector<T> &v);
    
    //inline
    //Geometry & operator=(const Geometry &g);
    inline
    bool operator==(const Geometry &g) const {
    	return ((width==g.getWidth()) && (height==g.getHeight())
    			&& (imageChannelCount==g.getImageChannelCount()) && (alphaChannelCount==g.getAlphaChannelCount()));
    };

    inline
    bool operator!=(const Geometry &g) const {
    	return !((*this)==g);
    };
    
    inline
    void toOStr(std::ostream &ostr) const {
    	AreaGeometry::toOStr(ostr);
    	if (getChannelCount() != 1){
    		ostr << "×";
    		ChannelGeometry::toOStr(ostr);
		}
	}

    std::string & toString(std::string & s) const;


protected:

    /// area and volume
    virtual
    void update();

    size_t volume;

    	
};



/// Not ready.
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
			height = v[1];
			break;
		case 1:
			width = v[0];
			break;
		default:
			break;
	}
	channelCount = imageChannelCount + alphaChannelCount;
	/*
	for (int i = 0; i < d; ++i) {
		if (i<d)
			(*this)[i] = vField[i];
		else
			(*this)[i] = 0;
	}
	*/
	update();
	return (*this);
}
    
inline
std::ostream & operator<<(std::ostream &ostr, const Geometry &geometry){
	geometry.toOStr(ostr);
	return ostr;
}
    

}

}

#endif /*GEOMETRY_H_*/

// Drain
