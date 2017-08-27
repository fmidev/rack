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
#ifndef PROJECTIONFRAME_H_
#define PROJECTIONFRAME_H_

#include <math.h>
#include "Proj4.h"

namespace drain
{


/// Extends the projection functionalities towards an image, that is, a bounded two-dimensional region.
class ProjectionFrame : public Proj4
{
public:
	ProjectionFrame();
	virtual ~ProjectionFrame();
	
	 /// Sets the bounding box in destination projection's (typically lat lon) lower left and upper right coords. 
    /*! Applied by projectScaled.
     */ 
   	void setBoundingBox(double xLL,double yLL,double xUR,double yUR);   // TODO rename latlon
    
    /// Sets image geometry.
    /*! Applied by projectScaled.
     */ 
   	void setGeometry(const unsigned int &width,const unsigned int & height);
   
    /// Projection scaled with image geometry and bounding box.
    // TODO: hide pj_*
    inline
    void projectScaled(double & x, double & y) const
    {
    	pj_transform(projSrc, projDst, 1, 1, &x, &y, NULL );
    	x = (x-xOffset) * xScale;
		y = (y-yOffset) * yScale;
    };
    
protected:

	void updateScaling(); 
			
	unsigned int width;
    unsigned int height;
    
    double xLowerLeft;
    double yLowerLeft;
    double xUpperRight;
    double yUpperRight;
    
	double xOffset;
	double yOffset;
	double xScale;
	double yScale;
   
	
};

}

#endif /*PROJECTIONFRAME_H_*/

// Drain
