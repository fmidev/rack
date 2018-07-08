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
#include "ProjectionFrame.h"

namespace drain
{

ProjectionFrame::ProjectionFrame()
{
}

ProjectionFrame::~ProjectionFrame()
{
}


void ProjectionFrame::setGeometry(const unsigned int &width,const unsigned int & height)
{
		this->width  = width;
		this->height = height;
		updateScaling();
}

void ProjectionFrame::setBoundingBox(double xLL,double yLL,double xUR,double yUR) 
{
 	xLowerLeft = xLL;
    yLowerLeft = yLL;
    xUpperRight = xUR;
    yUpperRight = yUR;
    updateScaling();
    
}

void ProjectionFrame::updateScaling() 
{
 	//static const double D2R = M_PI / 180.0;
    
   	 double xLL = yLowerLeft * DEG_TO_RAD;
     double yLL = yLowerLeft * DEG_TO_RAD;
     double xUR = xUpperRight * DEG_TO_RAD;
     double yUR = yUpperRight * DEG_TO_RAD;
    
     //double zDummy;

	 projectFwd(xLL,yLL);
	 projectFwd(xUR,yUR);
		
	 xOffset = xLL;
	 yOffset = yLL;
		
	 if ((xUR - xLL) != 0)
	 	xScale = width / (xUR - xLL);
	 	
	 if ((yUR - yLL) != 0)
	 	yScale = height / (yUR - yLL);
}

	
}
