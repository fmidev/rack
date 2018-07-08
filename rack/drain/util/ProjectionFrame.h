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
