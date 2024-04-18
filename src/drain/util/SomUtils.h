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
#ifndef SOM_UTILS_H_
#define SOM_UTILS_H_


/*
 g++ drain/examples/Som-example.cpp drain/util/{Caster,Variable,Debug}.cpp drain/image/{Geometry,Coordinates,Image,FilePng}.cpp  -lpng   -o Som-example
 */
#include <drain/image/ImageFile.h>
#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <sstream>

//
#include <stdlib.h>
//#include <cmath>

#include "drain/Variable.h"
#include "drain/image/Image.h"


namespace drain {

/** The Self-Organising Map by Teuvo Kohonen.
 *
 *  The template
 *
 *  The coordinates of the map apply matrix convention ie. \c map[i][j] refers to element j on row i.
 *  The map is implemented a std::vector<vector<T> > .
 */

	/** Generates a vector with values 0...255 sampled from uniform distribution.
	 *
	 */



// Utilities
/*
template <class T>
static
double Som<vector<T> >::defaultNeighborhoodFunction(const int & i, const int & j) const {
	return radius2 / (radius2 + i*i + j*j);
}
*/

/*
template <class T2>
void uniformRandomVector(vector<T2> & x) {
  
  for (int k = 0; k < x.size(); ++k)
    x[k] = static_cast<T2>(rand() & 0xff);
  //x[k] = static_cast<T2>(rand() & 0xffff)/static_cast<T2>(0xf00);
  //x[2] = 101;

}
*/


/// Squared Euclidean distance for vectors and similar
/*
    For vectors and str objects having 
 
template <class T>
double euclideanDistance2(const T & x1, const T & x2) {

	double d, result = 0.0;

	for (int i = 0; i < x1.size(); ++i) {
		d = x1[i]-x2[i];
		result += d*d;
	}

	return result;

}
*/


/*
template <class T>
void vectorMix(const T & x1, const T & x2, double coeff, T & m){ // todo iterate

	for (int k = 0; k < x1.size(); ++k)
		m[k] = (1.0-coeff)*x1[k] + coeff*x2[k];

}
*/

/// Utility to write contents of a vector SOM to a graylevel or RGB image.
template <class T>
void somToImage(const Som< T > & som, drain::image::Image & image, int width=0, int height=0, const std::vector<int> & channels = std::vector<int>()){

	const int depth = som.map[0][0].size();

	std::vector<int> defaultChannels;

	const std::vector<int> & applyChannels = channels.empty() ? defaultChannels : channels;

	if (channels.empty()){
		defaultChannels.resize(depth);
		for (int k = 0; k < depth; ++k) {
			defaultChannels[k] = k;
		}
	}

	const int n = applyChannels.size();

	if ((n != 1) && (n != 3)){
		std::cerr << "Warning: som2PNM: dimension not 1 (gray) or 3 (rgb) " << std::endl;
		return;
	}

	const int w = (width  > 0) ? width  : som.getWidth();
	const int h = (height > 0) ? height : som.getHeight();

	image.setGeometry(w, h, n);
	/*
	cout << "Writing image of " << image.getGeometry() << endl;
	for (int k = 0; k < n; ++k)
		cout << applyChannels[k] << ' ';
	cout << endl;
	*/


	int iMap, jMap;
	for (int j = 0; j < h; ++j) {
		jMap = (j * som.getHeight()) / h;
		for (int i = 0; i < w; ++i) {
			iMap = (i * som.getWidth()) / w;
			for (int k = 0; k < n; ++k) {
				image.put(i,j,k, som.map[jMap][iMap][applyChannels[k]]);
				//image.put(i,j,k, som.map[jMap][iMap][k]);
				//image.put(i,j,);
			}
		}
	}

}







} // drain



/* namespace drain */

#endif /* SOM_H_ */

// Drain
