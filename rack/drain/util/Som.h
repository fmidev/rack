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
#ifndef SOM_H_
#define SOM_H_




/*
 g++ drain/examples/Som-example.cpp drain/util/{Caster,Variable,Debug}.cpp drain/image/{Geometry,Coordinates,Image,FilePng}.cpp  -lpng   -o Som-example
 */
#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <sstream>

//
#include <stdlib.h>
//#include <cmath>
#include "../util/Variable.h"

//#include "../image/Image.h"
//#include "../image/File.h"


// // // using namespace std;
//using namespace drain;

namespace drain {

// Consider 
typedef std::vector<double> SomVector;  // for operator(std::ostream &)

/** The Self-Organising Map by Teuvo Kohonen.
 *
 *  The template
 *
 *  The coordinates of the map apply matrix convention ie. \c map[i][j] refers to element j on row i.
 *  The map is implemented a std::vector<std::vector<T> > .
 */
template <class T = std::vector<double> >
class Som {

public:

	Som(int width=0, int height=0);


	/**
	 * @param height the height recipient set
	 */
	inline
	void setGeometry(int width, int height) {
		this->width = width;
		this->height = height;
		map.resize(height);
		for (int j = 0; j < height; ++j) {
			map[j].resize(width);
		}
	}

	/**  Set each neuron in the map to given value.
	 *
	 */
	void fill(const T & sample){  // rename fill ?

		for (int i=0; i<width; i++){
			for (int j=0; j<height; j++){
				map[j][i] = sample;
			}
		}
	}

	/// Initialise the map with a user-defined function.
	/**
	 *  The user defined function with signature
	 *  \code
	 *     void (* init)(T & x)
	 *  \endcode
	 *  should initialize
	 */
	void initialise(void (* init)(T & x)) {

		for (int i=0; i<width; i++){
			for (int j=0; j<height; j++){
				(*init)(map[j][i]);
			}
		}

	}

	inline
	void setNeighbourhoodRadius(double r){
		radius2 = r*r;
	}

	inline
	void setLearningCoefficient(double c){
		learningCoefficient = c;
	}



	void setDistanceFunction(double (* d)(const T & x1, const T & x2)) {
		distanceFunction = d;
	}

	void setMixingFunction(void (* mix)(const T & x1, const T & x2, double coeff, T & m)) {
		mixingFunction = mix;
	}


	/// The number of rows in the map.
	inline
	int getHeight() const {
		return height;
	}

	/// The number of columns in the map.
	inline
	int getWidth() const {
		return width;
	}

	/// Finds the unit with state closesest to that of sample.
	/**
	 *    \par sample - the state against which the similarity of unit states is compared.
	 *    \par i - the row of the best matching unit
	 *    \par j - the column of the best matching unit
	 */
	void findBestMatchingUnit(const T & sample, int & iBest, int & jBest){

		double d;
		double dBest = std::numeric_limits<double>::max();

		for (int j=0; j<height; j++){
			for (int i=0; i<width; i++){
				d = (*distanceFunction)( sample, map[j][i]);
				if (d < dBest){
					dBest = d;
					iBest = i;
					jBest = j;
				}
			}
		}
	}


	/// Training with a single sample.
	/**  Calls findBestMatchingUnit() and uses *mixingFunction to tune the unit.
	 *
	 */
	void train(const T & x){
		int iBest;
		int jBest;
		findBestMatchingUnit(x, iBest, jBest);
		train(x, iBest, jBest);
		/*
		double f;
		for (int j=0; j<height; ++j){
			for (int i=0; i<width; ++i){
				f = defaultNeighborhoodFunction(i-iBest, j-jBest);
				(*mixingFunction)(map[j][i], x, f, map[j][i]);
			}
		}
		 */
	};

	/**  Training with a single sample.
	 *
	 */
	inline
	void train(const T &x, int iBest, int jBest){
		double f;
		for (int j=0; j<height; ++j){
			for (int i=0; i<width; ++i){
				f = defaultNeighborhoodFunction(i-iBest, j-jBest);
				(*mixingFunction)(map[j][i], x, f, map[j][i]);
			}
		}
	};



	void toStream(std::ostream &ostr) const{

		std::string delimiter = "";

		ostr << title << "\n";
		for (int j=0; j<height; j++){
			for (int i=0; i<width; i++){
				ostr << j << ',' << i  << ':';
				ostr << '\t' << map[j][i] << '\n';
				//delimiter = "";
				//result += "\n";
			};
			ostr << '\n';
		};

	};


	//static
	double defaultNeighborhoodFunction(const int & i, const int & j)  const {
		return radius2 / (radius2 + i*i + j*j);
	}

	static
	double defaultDistanceMetric(const T & x1, const T & x2);

	static
	void defaultMixingFunction(const T & x1, const T & x2, double coeff, T & m);




	/// The actual neural network, two-dimensional map of units.
	std::vector< std::vector<T> > map;

protected:

	/// The similarity metric applied in finding the best-matching unit.
	/**
	 *
	 */
	double (* distanceFunction)(const  T & x1, const T & x2);

	/// Given objects x1 and x1 and a mixing coefficient coeff , outputs mixed object m.
	/**
	 *   One might think of mixing two std::strings, for example.
	 */
	void (* mixingFunction)(const T & x1, const T & x2, double alpha, T & result);


	std::string title;

	// signed, because comparisons will typically involve signed integers
	int width;

	// signed, because comparisons will typically involve signed integers
	int height;

	/// The radius of the neighbourhood kernel applied in training;
	double radius2;

	/// The radius of the neighbourhood kernel applied in training;
	double learningCoefficient;

};



/*
    For vectors and other objects having 
 */
template <class T>
double euclideanDistance2(const T & x1, const T & x2) {

	double d, result = 0.0;

	for (int i = 0; i < x1.size(); ++i) {// todo iterate
		d = x1[i]-x2[i]; // todo dist();
		result += d*d;
	}

	return result;

}

template <class T>
void vectorMix(const T & x1, const T & x2, double coeff, T & m){ // todo iterate

	for (int k = 0; k < x1.size(); ++k)
		m[k] = (1.0-coeff)*x1[k] + coeff*x2[k];

}

template <>
Som<>::Som(int width, int height){
	setGeometry(width, height);
	setDistanceFunction( euclideanDistance2 );    // consider Som<double>
	setMixingFunction( vectorMix ); // consider Som<double>
}



/// For initialising vector Soms
template <class T>
void uniformRandomVector256(std::vector<T> & x) {

	for (int k = 0; k < x.size(); ++k)
		x[k] = static_cast<T>(rand() & 0xff);
	//x[k] = static_cast<T2>(rand() & 0xffff)/static_cast<T2>(0xf00);

}





// Utilities
/*
template <class T>
static
double Som<std::vector<T> >::defaultNeighborhoodFunction(const int & i, const int & j) const {
	return radius2 / (radius2 + i*i + j*j);
}
 */



/*
template <class T>
double Som<T>::defaultDistanceMetric(const T & x1, const T & x2) {

	double d, result = 0.0;

	for (int i = 0; i < x1.size(); ++i) {
		d = x1[i]-x2[i];
		result += d*d;
	}

	return result;

}
 */

//class Som< std::vector<T> >;
/*
template <class T>
double Som< std::vector<T> >::defaultDistanceMetric(const T & x1, const T & x2) {
	return 123.456;
}
 */

/*
template <class T>
void Som<T>::defaultMixingFunction(const T & x1, const T & x2, double coeff, T & m){

	for (int k = 0; k < x1.size(); ++k)
		m[k] = (1.0-coeff)*x1[k] + coeff*x2[k];

}
 */



/// Risk of conflicts?
template <class T>
//std::ostream & operator<<(std::ostream & ostr, const NeuralVector<T> & n){
std::ostream & operator<<(std::ostream & ostr, const std::vector<T> & n){

	for (typename std::vector<T>::const_iterator it = n.begin(); it !=
			n.end(); ++it)
		ostr << *it << ' ';
	//ostr << '\n';
	return ostr;
};

template <class T>
//std::ostream & operator<<(std::ostream & ostr, const NeuralVector<T> & n){
std::ostream & operator<<(std::ostream & ostr, const Som<T> & som){
	som.toStream(ostr);
	return ostr;
};


} // drain



/* namespace drain */

#endif /* SOM_H_ */

// Drain
