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
#ifndef DRAIN_SAMPLER_H_
#define DRAIN_SAMPLER_H_


#include <vector>
#include <iostream>
#include <sstream>
#include <string>

#include <drain/FlexibleVariable.h>


#include "drain/util/BeanLike.h"
#include "drain/util/IosFormat.h"
#include "drain/util/Range.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/StringMapper.h"
#include "drain/image/Image.h"

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
	//SamplePicker(ReferenceMap & variableMap) : variableMap(variableMap) {
	SamplePicker(ReferenceMap2<> & variableMap) : variableMap(variableMap) {
		infoMap["width"].link(width  = 0);
		infoMap["height"].link(height = 0);
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

	// TODO: min/max coord control, like CoordHandler for checking min and max.
	//bool checkPosition()

	/// Optional utility. Called prior to writing the actual data to output stream.
	//virtual
	//void writeHeader(char commentPrefix, std::ostream & ostr) const {};


	/// Horizontal coordinate.
	mutable int current_i = 0;

	/// Vertical coordinate.
	mutable int current_j = 0;

	/// Vertical inversed coordinate.
	mutable int current_j2 = 0;

	int width = 0;
	int height = 0;

	/// Information to be should in output file header.
	FlexVariableMap infoMap;

	/// Container for image data, including data derived thereof. Updated in every image position (i,j).
	/// Formatted and displayed in each row or the output file.
	ReferenceMap2<> & variableMap;

};

/// Reads image channels, returning scaled (physical) values.
/**
 *
 */
class ImageReader : public SamplePicker {

public:

	inline
	ImageReader(ReferenceMap2<> & ref) : SamplePicker(ref) {//static int dummy;
		variableMap.link("i", current_i = 0);
		variableMap.link("j", current_j = 0);
		variableMap.link("j2", current_j2 = 0);
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
 *   If the commentChar has been defined, the first line contains the applied format std::string (default or user-defined)
 *   and other metadata.
 */
class Sampler : public BeanLike {

public:

	Sampler();

	inline
	Sampler(const Sampler &sampler) : BeanLike(sampler){
		parameters.copyStruct(sampler.getParameters(), sampler, *this);
	};

	// Named conf, to separate ImageMod::parameters (BeanLike)
	//ReferenceMap conf;

	int iStep = 10;
	int jStep = 0;

	drain::Range<int> iRange = {-1,1};
	drain::Range<int> jRange = {-1,1};

	/// Escape std::string for prefixing text no to be handled as data values.
	std::string commentPrefix = "#";


	// was already DEPRECATING...
	/// Skip lines, if contain missing values.
	bool skipVoid = false ;
	//std::string skipVoid = "false";
	std::string voidComment = "void";

	// NEW
	std::string handleVoid = "null"; // "false","skip"


	/// Interface that links coordinates and image data.
	mutable
	ReferenceMap2<> variableMap;

	/// Returns character, also supporting numeric ASCII values between 32 and 128.
	char getCommentChar() const;

	/// Use given format or generate default "${var},${var2}, ..."
	std::string getFormat(const std::string & formatStr) const;


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

		drain::Logger mout(__FUNCTION__, getName());

		if (images.empty()){
			mout.error("no image data (channels) provided: " );
			return;
		}

		const bool SKIP_VOID = (handleVoid == "skip") || (skipVoid == true);

		const bool MARK_VOID = (handleVoid != "skip");

		double voidMarker = NAN;
		if (handleVoid != "null"){
			StringTools::convert(handleVoid, voidMarker);
		}

		// Save initial formatting data

		//mout.special("fieldWidth: " , fieldWidth , ", fillChar=" , fillChar );
		mout.debug2("variables (initially): " , variableMap.getKeys() );

		/// Final formatter defined here, to copy ostr precision etc.
		// Note: supports leading minus sign
		drain::StringMapper formatter("-?[a-zA-Z0-9_]+"); // WAS: "-?[a-zA-Z0-9_]+" with odd "-?"
		// format = drain::StringTools::replace(format, "\\n", "\n");
		// format = drain::StringTools::replace(format, "\\t", "\t");
		std::string format = getFormat(formatStr);
		formatter.parse(format, true); // convert escaped, eg. "\\n" -> "\n"
		formatter.iosFormat.copyFrom(ostr);
		mout.special("iosFormat: " , formatter.iosFormat );

		// std::string format = getFormat(formatStr);

		// Service: associate file keys with data
		// mout.debug("check minus" );
		std::map<std::string, double> values;
		static const std::string minusStr("-");
		for (typename std::map<std::string, D>::const_iterator it = images.begin(); it != images.end(); ++it){
			const std::string & key = it->first;
			mout.debug2("referencing: " , key , ',' , minusStr , key );
			variableMap.link(key, values[key]=0);
			variableMap.link(minusStr+key, values[minusStr+key]=0);
		}
		mout.debug("variables: " , variableMap );



		const int iStep  = this->iStep;
		const int jStep  = (this->jStep > 0) ? this->jStep : iStep;

		const char commentChar = getCommentChar();
		if (commentChar){
			/// Write header, if commentChar has been set.

			// ostr << commentChar << " TEST\n";
			// picker.writeHeader(commentChar, ostr);
			// ostr << commentChar << " size='" << picker.width << 'x' << picker.height << "'\n";

			ostr << commentChar << commentChar << " input properties " << '\n';

			for (const auto & entry: picker.infoMap){
				ostr << commentChar << ' ' << entry.first << '=';
				//it->second.valueToJSON(ostr);
				Sprinter::toStream(ostr, entry.second, Sprinter::jsonLayout);
				ostr << '\n';
			}

			ostr << commentChar << commentChar << " sampling parameters " << '\n';

			for (const auto & entry: parameters){
				ostr << commentChar << ' ' << entry.first << '=';
				// it->second.valueToJSON(ostr);
				Sprinter::toStream(ostr, entry.second, Sprinter::jsonLayout);
				ostr << '\n';
			}
			if (!formatStr.empty())
				ostr << commentChar << " format='" << formatStr << "'\n"; // formatStr instead of format, to save double slash \\n \\t
			else
				ostr << commentChar << " format='" << format << "'\n";

			ostr << commentChar << commentChar << " resulting geometry " << '\n';
			const int iN = picker.width/iStep;
			const int jN = picker.height/jStep;
			ostr << commentChar << " rows=" << iN << "\n";
			ostr << commentChar << " rols=" << jN << "\n";
			ostr << commentChar << " samples=" << (iN*jN) << "\n";


		}


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
		// SamplePicker has set variableMap references (ie. geographical coords j2=(height-1-j) ).
		double x;
		bool dataOk;
		for (int j = jStart; j<=jEnd; j+=jStep){
			for (int i = iStart; i<=iEnd; i+=iStep){

				picker.setPosition(i, j);

				// SLOW, but works... // TODO speedup with iterator
				dataOk = true;
				for (typename std::map<std::string, D>::const_iterator it = images.begin(); it != images.end(); ++it){
					const std::string & quantity = it->first;
					const D & data = it->second;
					if (!picker.getValue(data, x)){
						dataOk = false;
						if (MARK_VOID){
							x = voidMarker;
						}
					}
					//else if (x != 0) std::cerr << x << '\t';

					values[quantity] = x;
					values[minusStr+quantity] = -x;
				}

				if (dataOk || !SKIP_VOID){
				//if (dataOk || (skipVoid==0)){
					//formatter.toStream(ostr, variableMap, true);
					formatter.toStream(ostr, variableMap, -1); // leaves ${variable} ?
					if  ((!dataOk) && (commentChar))
						ostr << ' ' << commentChar <<  voidComment;
					ostr << '\n';
				}
			}
			// formatter.expand(map, true);
		}
		mout.debug("last values: " , variableMap );
		//mout.warn("formatter " , formatter );

	}


};



}
}


#endif /*POINT_H_*/

// Drain
