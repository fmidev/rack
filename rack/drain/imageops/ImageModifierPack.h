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
#ifndef IMAGE_MODIFIER_PACK_H
#define IMAGE_MODIFIER_PACK_H

#include "drain/util/Rectangle.h"
#include "drain/util/Histogram.h"
#include "drain/image/Sampler.h"

#include "ImageMod.h"

namespace drain {

namespace image {





class ImageChannels : public ImageMod {

public:

	ImageChannels() : ImageMod(__FUNCTION__, "Redefine channel geometry. See also --geometry") {
		parameters.link("imageChannels", imageChannelCount = 1UL);
		parameters.link("alphaChannels", alphaChannelCount = 0UL);
	};

	ImageChannels(const ImageChannels & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}


	virtual
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const {};


protected:

	mutable
	size_t imageChannelCount;

	mutable
	size_t alphaChannelCount;

};



class ImageCoordPolicy : public ImageMod {

public:
	/*
	const int CoordinatePolicy::UNDEFINED(0);
	const int CoordinatePolicy::LIMIT(1);
	const int CoordinatePolicy::WRAP(2);
	const int CoordinatePolicy::MIRROR(3);
	const int CoordinatePolicy::POLAR(4);
	*/

	ImageCoordPolicy() : ImageMod(__FUNCTION__, "Coordinate under/overflow policy: 0=UNDEFINED, 1=LIMIT, 2=WRAP, 3=MIRROR, 4=POLAR"){
		parameters.link("policy", value, "<xUF>[,<yUF>[,<xOF>,<yOF>]]");
		parameters.separator = 0;
	};

	ImageCoordPolicy(const ImageCoordPolicy & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	/// Sets the policy
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const;

	std::string value;

protected:

	mutable CoordinatePolicy policy;

};


/// Changes the type of a target image
/**

\code
  drainage --encoding S --geometry 200,100     -o img-200x100-16b.png
  drainage --encoding S --geometry 200,100,3   -o img-200x100-16b-rgb.png
  drainage --encoding S --geometry 200,100,3,1 -o img-200x100-16b-rgba.png
\endcode
 */

class ImageEncoding : public ImageMod {

public:


	ImageEncoding() : ImageMod(__FUNCTION__, "Set desired target properties") { // TODO

		refMap.link("type", encoding.type);
		//refMap.link("scale", encoding.ValueScaling::tuple());
		refMap.link("scale", encoding.getScaling().tuple()).fillArray = false;
		refMap.link("range", encoding.getPhysicalRange().tuple()).fillArray = false; //physRange);
		//refMap.link("min", this->scaling.physRange.min, "physical_value");
		//refMap.link("max", this->scaling.physRange.max, "physical_value");

		parameters.link("request", request, refMap.getKeys());
		parameters.separator = 0;

	};

	ImageEncoding(const ImageEncoding & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
		refMap.copyStruct(op.refMap, op, *this); // should work? UniTuple instances use local (inside-object) memory
	}

	virtual
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const {
		//drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);
		//mout.fail() << "Feelu" << mout.endl;
	};
	//virtual
	//void process(Image & dst) const;

protected:

	std::string request;


	mutable drain::ReferenceMap refMap;

	//mutable std::string type;
	// mutable drain::ValueScaling scaling;
	mutable drain::image::Encoding encoding;
	//mutable double scale;
	//mutable double minValue;
	//mutable double maxValue;
	// mutable std::string view;
};


///
/**

 \code
    drainage --geometry 100,100,3   --fill 128,192,255      -o filled-rgb.png
    drainage --geometry 100,100,3,1 --fill 128,192,255,192  -o filled-rgba.png
    drainage --geometry 100,100,3 --view g --fill 255 --view f -o filled-green.png
 \endcode

 \see ImagePlot
 \see ImageBox
 */
class ImageFill : public ImageMod {

public:

	ImageFill() : ImageMod(__FUNCTION__, "Fill the image with intensity <value>[,<green>,<blue>[,alpha]]. See also 'plotfile'.") {
			 // "value", "0", "<value>[,<green>,<blue>[,alpha]]") {
		parameters.separator = 0;
		parameters.link("value", value);
	};

	ImageFill(const ImageFill & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}


	virtual
	void traverseChannel(Channel & dst) const;

	virtual
	void traverseChannels(ImageTray<Channel> & dst) const;

protected:

	std::string value;

};

/**
 *
 */
class ImageHistogram : public ImageMod {

public:

	ImageHistogram() : ImageMod(__FUNCTION__, "Compute the image Histogram .") {
		//parameters.separator = 0;
		parameters.link("bins", bins = 256);
		//parameters.link("store", store = true, "save as attribute");
		parameters.link("store", store = "histogram", "attribute name (empty = don't save)");
		parameters.link("filename", filename = "", "<filename>.txt");
		// Todo prefix/comment
	};

	ImageHistogram(const ImageHistogram & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void traverseChannel(Channel & dst) const;

	//virtual	void computeHistogram(const Channel & dst, drain::Histogram & histogram) const;
	//mutable drain::Histogram histogram;
	//protected:

	size_t bins;

	// bool
	std::string store;

	std::string filename;

};


/**

An empty image can be created with:
\code
  drainage --geometry 200,100     -o img-200x100.png
  drainage --geometry 200,100,3   -o img-200x100-rgb.png
  drainage --geometry 200,100,3,1 -o img-200x100-rgba.png
\endcode
 */
class ImageGeometry : public ImageMod {

public:

	inline
	ImageGeometry() : ImageMod(__FUNCTION__, "Create image with given geometry. See also --channels") {

		parameters.link("width", width = 0UL, "pix");
		parameters.link("heigh", height = 0UL, "pix");
		parameters.link("imageChannels", imageChannelCount = 1UL);
		parameters.link("alphaChannels", alphaChannelCount = 0UL);

	};

	ImageGeometry(const ImageGeometry & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const {};

protected:

	size_t width;
	mutable size_t height;
	mutable size_t imageChannelCount;
	mutable size_t alphaChannelCount;


};




/// Plots a single value in an image. The value will be scaled; notice that alpha channel is scaled by default.
/**
\code
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192  -o plot-dots.png
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192 --target S --iCopy f -o plot-dots-16b.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o plot-dots-rgba.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192 --target S --iCopy f -o plot-dots-rgba-16b.png
\endcode
 *
 */
class ImagePlot: public ImageMod {

public:

	ImagePlot() : ImageMod(__FUNCTION__, "Set intensity at (i,j) to (f1,f2,f3,...)."){
		// See 'plotFile' and 'fill'.",	"value", "0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]" ) {
		parameters.separator = 0;
		parameters.link("value", value="0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]");

	};

	ImagePlot(const ImagePlot & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}


	virtual
	void traverseChannels(ImageTray<Channel> & dst) const;

protected:



	std::string value;
};


class ImagePlotFile: public ImageMod {

public:

	ImagePlotFile() : ImageMod(__FUNCTION__, "Plots a given file.  See 'plot'."){
		parameters.link("filename", filename = "",  "string");
	};

	ImagePlotFile(const ImagePlotFile & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void traverseFrame(ImageFrame & dst) const;
	//void traverseChannel(Channel & dst) const {};

protected:

	std::string filename;

};

/// Plots a single value in an image. The value will be scaled; notice that alpha channel is scaled by default.
/**
\code
drainage --geometry 256,256,1 --box 96,96,255 --box 160,96,208  --box 128,160,192  -o box-dots.png
drainage --geometry 256,256,1 --box 96,96,255 --box 160,96,208  --box 128,160,192 --target S --iCopy f -o box-dots-16b.png
drainage --geometry 256,256,3,1 --box 96,96,255,64,32,255  --box 160,96,64,255,32,208  --box 128,160,64,32,255,192  -o box-dots-rgba.png
drainage --geometry 256,256,3,1 --box 96,96,255,64,32,255  --box 160,96,64,255,32,208  --box 128,160,64,32,255,192 --target S --iCopy f -o box-dots-rgba-16b.png
\endcode
 *
 */
class ImageBox: public ImageMod {

public:

	ImageBox() : ImageMod(__FUNCTION__, "Set intensity at (i:i2,j:j2) to (f1,f2,f3,...)."){
		// See 'BoxFile' and 'fill'.",	"value", "0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]" ) {
		//parameters.separator = 0;
		parameters.link("i", iRange.tuple(),"i:i2").fillArray = true;
		parameters.link("j", jRange.tuple(),"j:j2").fillArray = true;
		parameters.link("value", value="0", "<f1>[,f2,f3,alpha]");

	};

	inline
	ImageBox(const ImageBox & op) : ImageMod(op) {
		parameters.copyStruct(op.getParameters(), op, *this);
	}

	virtual
	void traverseChannels(ImageTray<Channel> & dst) const;

protected:

	Range<int> iRange;
	Range<int> jRange;
	std::string value;

};


/// Traverses image, returning samples
/**
  As convenience, provides also a filename
 \code
 drainage gray.png  --sample file=samples-gray.txt
 drainage image.png --format '${i},${j}\t ${0},${-2}' --sample 50,50,file=samples.txt
 \endcode
 */
class ImageSampler : public ImageMod {

public:

	/// Default constructor.
	ImageSampler() : ImageMod(__FUNCTION__, "Extract samples. See --format.") {
		parameters.append(sampler.getParameters());
	}
	//	parameters.link("file", filename = "",  "string");

	ImageSampler(const ImageSampler & op) : ImageMod(op), sampler(op.sampler) {
		//parameters.append(sampler.getParameters());
		parameters.copyStruct(op.sampler.getParameters(), op.sampler, this->sampler);
	};

	/// Runs Sampler on the given image.
	/**
	 *  \param dst - target image
	 */
	virtual
	void process(Image & dst) const;

	inline
	const Sampler & getSampler(){
		return sampler;
	};

	virtual inline
	const std::string & getFormat() const {
		return format;
	};

	/*
	virtual inline
	std::string & getFormat(){
		return format;
	};
	*/

	/// If defined, sampler will directly write to this file. The default constructor does not reference this.
	std::string filename;

protected:

	ImageSampler(const std::string & name, const std::string & description) : ImageMod(name, description){
		parameters.append(sampler.getParameters());
	}


	mutable Sampler sampler;


	/// Output format, e.g. '{LON} {LAT} {AMVU} {AMVV} {QIND}'
	std::string format;

};



}  // namespace image

}  // namespace drain


#endif /* IMAGE_OP_H_ */

// Drain
