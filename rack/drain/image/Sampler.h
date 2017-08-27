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
#ifndef SAMPLER_H_
#define SAMPLER_H_


#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include "util/ReferenceMap.h"
#include "util/StringMapper.h"
#include "image/Image.h"

namespace drain
{

namespace image
{

// using namespace std;

/// Interprets data values for Sampler.
/**
 *   Sampler moves SamplePicker by calling its setLocation(). Then, calls its getValue.
      Picks and scales invidual values. Checks validity etc.
 */
struct SamplePicker {

	inline
	SamplePicker(ReferenceMap & ref) : ref(ref) {
	}

	~SamplePicker(){};

	void setSize(int w, int h){
		width = w;
		height = h;
	}

	/// Sets current position in image coordinates. To be redefined to compute also geographical projections, for example.
	virtual
	inline
	void setPosition(int i, int j) const {
		current_i  = i; //static_cast<double>(i);
		current_j  = j; // 10.0*static_cast<double>(j);
		current_j2 = height-1 - j;
	}


	/// Horizontal coordinate.
	mutable int current_i;

	/// Vertical coordinate.
	mutable int current_j;

	/// Vertical inversed coordinate.
	mutable int current_j2;

	int width;
	int height;

	virtual
	void writeHeader(const std::string & commentPrefix, std::ostream & ostr) const {};

	ReferenceMap & ref;

};


class ImageReader : public SamplePicker {
    public: //re 

	inline
	ImageReader(ReferenceMap & ref) : SamplePicker(ref) {//static int dummy;
		ref.reference("i", current_i);
		ref.reference("j", current_j);
		ref.reference("j2", current_j2);
	}

	inline
	bool getValue(const Image & image, double & value) const {
		value = image.get<double>(current_i, current_j);
		return true;
	}


};

/// Utility for sampling images (2D data), outputting formatted text data.
/**
 *   If the commentChar has been defined, the first line contains the applied format std::string (default or user-defined).
 */
struct Sampler  {

	Sampler() : iStep(10), jStep(0), iStart(-1), jStart(-1), iEnd(-1), jEnd(0), commentChar("#"), skipVoid(0), voidMarker("void data"), variableMap(true) { // ensure ordered  //   missingValueChar('-'),
	};

	int iStep;
	int jStep;
	int iStart;
	int jStart;
	int iEnd;
	int jEnd;

	/// Escape std::string for prefixing text no to be handled as data values.
	std::string commentChar;

	/// Skip lines, if contain missing values.
	int skipVoid;
	std::string voidMarker;

	ReferenceMap variableMap;

	//void toOstr();

	/// Main function
	/**
	 *  \tparam T - applied data, for which H::getValue(T &data) is called.
	 *  \tparam H - applied data, for which
	 */
	template <class T, class H>
	void sample(std::map<std::string, const T &> images, const H & picker, const std::string &formatStr, std::ostream & ostr = std::cout){ // std::string format copy ok

		///
		int i;
		int j;

		/// No referencing here. (Some applications would not like it.)
		//  Set them in Picker.
		// variableMap.reference("i", i);
		// variableMap.reference("j", j);

		const bool FORMAT = !formatStr.empty();
		std::string format = drain::String::replace(formatStr, "\\n", "\n"); // "\\n", "\n");
		format = drain::String::replace(format, "\\t", "\t");




		// Service: associate file labels with data
		std::map<std::string, double> values;
		for (typename std::map<std::string, const T &>::const_iterator it = images.begin(); it != images.end(); ++it){
			const std::string & key = it->first;
			variableMap.reference(key, values[key]);
		}

		if (!FORMAT){
			std::stringstream sstr;
			const std::list<std::string> & keys = variableMap.getKeyList();
			char separator = 0;
			for (std::list<std::string>::const_iterator it=keys.begin(); it!=keys.end(); ++it){
				//if (!format.empty())
				if (separator)
					sstr << ',';
				else
					separator = ',';
				sstr << '{' << *it << '}';
			}
			format = sstr.str();
		}

		const int iStep  = this->iStep;
		const int jStep  = (this->jStep > 0) ? this->jStep : iStep;

		/// Write header
		if (!commentChar.empty()){
			//ostr << commentChar << " TEST\n";
			picker.writeHeader(commentChar, ostr);
			ostr << commentChar << " size='" << picker.width << 'x' << picker.height << "'\n";
			const int iN = picker.width/iStep;
			const int jN = picker.height/jStep;
			ostr << commentChar << " iStep=" << iStep << "\n";
			ostr << commentChar << " jStep=" << jStep << "\n";
			ostr << commentChar << " sampleRows=" << iN << "\n";
			ostr << commentChar << " sampleCols=" << jN << "\n";
			ostr << commentChar << " samples=" << (iN*jN) << "\n";
			ostr << commentChar << " format='" << format << "'\n";
		}


		drain::StringMapper formatter("[a-zA-Z0-9_]+");
		formatter.parse(format);


		int iStart = this->iStart;
		if (iStart < 0)
			iStart = iStep/2;

		int jStart = this->jStart;
		if (jStart < 0)
			jStart = jStep/2;

		int iEnd   = this->iEnd;
		if (iEnd < iStart)
			iEnd = picker.width-1;

		int jEnd   = this->jEnd;
		if (jEnd < jStart)
			jEnd = picker.height-1;


		/// i,j  scope variables
		double x;
		bool dataOk;
		for (j = jStart; j<jEnd; j+=jStep){
			for (i = iStart; i<iEnd; i+=iStep){

				picker.setPosition(i, j);

				// SLOW, but works... // TODO speedup with iterator
				dataOk = true;
				for (typename std::map<std::string, const T &>::const_iterator it = images.begin(); it != images.end(); ++it){
					const std::string & quantity = it->first;
					const T & data = it->second;
					if (!picker.getValue(data, x)){ // data OK
						dataOk = false;
					}
					values[quantity] = x;
				}

				if (dataOk || (skipVoid==0)){
					formatter.toStream(ostr, variableMap);
					if  ((!dataOk) && (!commentChar.empty()))
						ostr << ' ' << commentChar <<  voidMarker;
					ostr << '\n';
				}
			}
			// formatter.expand(map, true);
		}
	}


};





/*


template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::image::Point2D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ']';
    return ostr;
}

template <class T>
std::ostream &operator<<(std::ostream &ostr,const drain::image::Point3D<T> &p)
{
	ostr << '[' << p.x << ',' << p.y << ',' << p.z << ']';
    return ostr;
}
*/


}
}


#endif /*POINT_H_*/

// Drain
