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

#include "util/Range.h"
#include "util/ReferenceMap.h"
#include "util/StringMapper.h"
#include "image/Image.h"

namespace drain
{

namespace image
{

/// Interprets data values for Sampler.
/**
 *   A Sampler instance moves a SamplePicker instance by calling its setLocation(). Then, calls its getValue().
     SamplePicker picks and scales invidual values, checks their validity and so on.
 */
struct SamplePicker {

	/**
	 *  \param ref - flexible container in which the values in each location will be written.
	 */
	inline
	SamplePicker(ReferenceMap & ref) : ref(ref) {
	}

	virtual inline
	~SamplePicker(){};

	void setSize(int w, int h){
		width = w;
		height = h;
	}

	/// Sets current position in image coordinates. To be redefined to compute also geographical projections, for example.
	virtual
	inline
	void setPosition(int i, int j) const {
		current_i  = i;
		current_j  = j;
		current_j2 = height-1 - j;
	}

	/// Optional utility. Called prior to writing the actual data to output stream.
	virtual
	void writeHeader(const std::string & commentPrefix, std::ostream & ostr) const {};


	/// Horizontal coordinate.
	mutable int current_i;

	/// Vertical coordinate.
	mutable int current_j;

	/// Vertical inversed coordinate.
	mutable int current_j2;

	int width;
	int height;


	ReferenceMap & ref;

};

/// Reads image channels, returning scaled (physical) values.
/**
 *
 */
class ImageReader : public SamplePicker {

public:

	inline
	ImageReader(ReferenceMap & ref) : SamplePicker(ref) {//static int dummy;
		ref.reference("i", current_i = 0);
		ref.reference("j", current_j = 0);
		ref.reference("j2", current_j2 = 0);
	}

	inline
	bool getValue(const ImageFrame & image, double & value) const {
		//value = image.get<double>(current_i, current_j);
		// if (mode==SCALED ?
		value = image.getScaled(current_i, current_j);
		//value = image.get<double>(current_i, current_j);
		//std::cerr << current_i << ',' << current_j << '\t' << value << '\n';
		return true;
	}


};

/// Utility for sampling images (2D data), outputting formatted text data.
/**
 *   If the commentChar has been defined, the first line contains the applied format std::string (default or user-defined).
 */
class Sampler  {

public:

	//Sampler() : iStep(10), jStep(0), iRange(-1,-1), jRange(-1,-1), iStart(-1), jStart(-1), iEnd(-1), jEnd(0),
	/*
	Sampler() : iStep(10), jStep(0), iRange(-1,-1), jRange(-1,-1), iStart(iRange.vect[0]), jStart(jRange.vect[0]), iEnd(iRange.vect[1]), jEnd(jRange.vect[1]),
			commentChar("#"), skipVoid(0), voidMarker("void data") {
	};
	*/

	Sampler() : iStep(10), jStep(0), iRange(-1,-1), jRange(-1,-1), commentChar("#"), skipVoid(0), voidMarker("void data") {
	};


	int iStep;
	int jStep;

	drain::Range<int> iRange;
	drain::Range<int> jRange;
	/*
	int & iStart;
	int & jStart;
	int & iEnd;
	int & jEnd;
	*/

	/// Escape std::string for prefixing text no to be handled as data values.
	std::string commentChar;

	/// Skip lines, if contain missing values.
	int skipVoid;
	std::string voidMarker;

	/// Interface that links coordinates and image data.
	mutable
	ReferenceMap variableMap;

	/// Main function
	/**
	 *
	 *  \tparam D - applied data type (e.g. Channel or ImageFrame), for which P::getValue(T &data) is implemented.
	 *  \tparam P - picker derived from class Picker, implements getValue(T &data).
	 *
	 *  Modifies this->variableMap by adding an entry corresponding to the key of each data source.
	 *
	 *  No referencing of coordinates is done by Sampler because applications might prefer
	 *  derived variables like j2=(height-1-j) or geographical coordinates.
     *  Set them in Picker.
	 */
	template <class D, class P>
	void sample(const std::map<std::string, D> & images, const P & picker, const std::string & formatStr, std::ostream & ostr = std::cout) const { // std::string format copy ok

		drain::Logger mout("Sampler", __FUNCTION__);

		mout.debug(1) << "variables (initially): " << variableMap.getKeys() << mout.endl;

		const bool FORMAT = !formatStr.empty();
		std::string format = drain::StringTools::replace(formatStr, "\\n", "\n"); // "\\n", "\n");
		format = drain::StringTools::replace(format, "\\t", "\t");

		// Service: associate file keys with data
		// mout.debug() << "check minus" << mout.endl;
		std::map<std::string, double> values;
		static const std::string minusStr("-");
		for (typename std::map<std::string, D>::const_iterator it = images.begin(); it != images.end(); ++it){
			const std::string & key = it->first;
			/*
			if (it->second.isEmpty())  // not required from D!
				mout.warn() << key << ": empty image " << it->second << mout.endl;
			*/
			mout.debug(1) << "referencing: " << key << ',' << minusStr << key << mout.endl;
			variableMap.reference(key, values[key]=0);
			variableMap.reference(minusStr+key, values[minusStr+key]=0);
		}
		mout.debug() << "variables: " << variableMap << mout.endl;


		/// If format not explicitly set, use all the variables => create default format.
		if (!FORMAT){
			mout.debug() << "constructing default format (all the quantities)" << mout.endl;
			std::stringstream sstr;
			const std::list<std::string> & keys = variableMap.getKeyList();
			char separator = 0;
			for (std::list<std::string>::const_iterator it=keys.begin(); it!=keys.end(); ++it){
				if (!it->empty()){
					if (it->at(0) != '-'){
						if (separator)
							sstr << separator;
						else
							separator = ',';
						sstr << '$' << '{' << *it << '}';
					}
				}
				else {
					mout.warn() << "empty quantity" << mout.endl;
				}
			}
			format = sstr.str();
		}

		const int iStep  = this->iStep;
		const int jStep  = (this->jStep > 0) ? this->jStep : iStep;

		/// Write header, if commentChar has been set.
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
			if (!formatStr.empty())
				ostr << commentChar << " format='" << formatStr << "'\n"; // formatStr instead of format, to save double slash \\n \\t
			else
				ostr << commentChar << " format='" << format << "'\n";
		}

		// Note: supports leading minus sign
		drain::StringMapper formatter("-?[a-zA-Z0-9_]+"); // WAS: "-?[a-zA-Z0-9_]+" with odd "-?"
		formatter.parse(format);
		//mout.warn() << "formatter " << formatter << mout.endl;


		int iStart = this->iRange.min; // this->iStart;
		if (iStart < 0)
			iStart = iStep/2;

		int jStart = this->jRange.min; //jStart;
		if (jStart < 0)
			jStart = jStep/2;

		int iEnd   = this->iRange.max; // iEnd;
		if (iEnd < iStart)
			iEnd = picker.width-1;

		int jEnd   = this->jRange.max; // jEnd;
		if (jEnd < jStart)
			jEnd = picker.height-1;


		// Main loop: traverse image area with (i,j)
		// But no referencing here because some applications might prefer derived variables like
		// j2=(height-1-j) or geographical coords.
		// => Set them in SamplePicker.
		double x;
		bool dataOk;
		for (int j = jStart; j<jEnd; j+=jStep){
			for (int i = iStart; i<iEnd; i+=iStep){

				picker.setPosition(i, j);

				// SLOW, but works... // TODO speedup with iterator
				dataOk = true;
				for (typename std::map<std::string, D>::const_iterator it = images.begin(); it != images.end(); ++it){
					const std::string & quantity = it->first;
					const D & data = it->second;
					if (!picker.getValue(data, x)){
						dataOk = false;
					}
					//else if (x != 0) std::cerr << x << '\t';
					values[quantity] = x;
					values[minusStr+quantity] = -x;
				}

				if (dataOk || (skipVoid==0)){
					formatter.toStream(ostr, variableMap, true);
					if  ((!dataOk) && (!commentChar.empty()))
						ostr << ' ' << commentChar <<  voidMarker;
					ostr << '\n';
				}
			}
			// formatter.expand(map, true);
		}
		mout.debug() << "last values: " << variableMap << mout.endl;
		//mout.warn() << "formatter " << formatter << mout.endl;

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
