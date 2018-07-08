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
#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <iostream>
#include <vector>

namespace drain
{

namespace image
{

/// Rectangle of width x height .
class AreaGeometry
{
public:

	inline
	AreaGeometry() : width(0), height(0), area(0){};

    void setWidth(size_t weight);

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

protected:

	size_t width;
    size_t height;
    size_t area;

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
 *  Three standard "copies" are defined for convenience; KLUDGE!
 *  # geometry.width  = geometry[0]
 *  # geometry.height = geometry[1]
 *  # geometry.channelCount = geometry[2]  
 *
 *  To guarantee this, the minimum size of the vector will always be at least three (3).
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
