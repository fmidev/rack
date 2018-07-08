/**

    Copyright 2001 - 2016  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


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
#ifndef IMAGE_MODIFIER_PACK_H
#define IMAGE_MODIFIER_PACK_H

#include "util/Histogram.h"
#include "image/Sampler.h"

#include "ImageMod.h"

namespace drain {

namespace image {





class ImageChannels : public ImageMod {

public:

	ImageChannels() : ImageMod(__FUNCTION__, "Redefine channel geometry. See also --geometry") {

		parameters.reference("imageChannels", imageChannelCount = 1UL);
		parameters.reference("alphaChannels", alphaChannelCount = 0UL);

	};

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
		parameters.reference("policy", value, "<xUF>[,<yUF>[,<xOF>,<yOF>]]");
		parameters.separator = 0;
	};

	/// Sets the policy
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const;

	std::string value;

protected:

	mutable CoordinatePolicy policy;

};


/// Changes the type of an existing image
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


		refMap.reference("type", type);
		//refMap.reference("scale", scaling.scale);
		refMap.reference("min", scaling.minPhysValue, "physical_value");
		refMap.reference("max", scaling.maxPhysValue, "physical_value");

		parameters.reference("request", request, refMap.getKeys());
		parameters.separator = 0;

	};

	virtual
	void initialize(Image & dst) const;

	virtual
	void traverseChannel(Channel & dst) const {};
	//virtual
	//void process(Image & dst) const;

protected:

	std::string request;


	mutable drain::ReferenceMap refMap;

	mutable std::string type;
	mutable ImageScaling scaling;
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
 */
class ImageFill : public ImageMod {

public:

	ImageFill() : ImageMod(__FUNCTION__, "Fill the image with intensity <value>[,<green>,<blue>[,alpha]]. See also 'plotfile'.") {
			 // "value", "0", "<value>[,<green>,<blue>[,alpha]]") {
		parameters.separator = 0;
		parameters.reference("value", value);
	};

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
		parameters.reference("bins", bins = 256);
	};

	virtual
	void traverseChannel(Channel & dst) const;

	// virtual
	// void traverseChannels(ImageTray<Channel> & dst) const;

	mutable drain::Histogram histogram;

protected:


	size_t bins;
	//std::string value;

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

		parameters.reference("width", width = 0UL, "pix");
		parameters.reference("heigh", height = 0UL, "pix");
		parameters.reference("imageChannels", imageChannelCount = 1UL);
		parameters.reference("alphaChannels", alphaChannelCount = 0UL);

	};

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
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192  -o dots.png
drainage --geometry 256,256,1 --plot 96,96,255 --plot 160,96,208  --plot 128,160,192 --target S --copy f -o dots-16b.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192  -o dots-rgba.png
drainage --geometry 256,256,3,1 --plot 96,96,255,64,32,255  --plot 160,96,64,255,32,208  --plot 128,160,64,32,255,192 --target S --copy f -o dots-rgba-16b.png
\endcode
 *
 */
class ImagePlot: public ImageMod {

public:

	ImagePlot() : ImageMod(__FUNCTION__, "Set intensity at (i,j) to (f1,f2,f3,...)."){
		// See 'plotFile' and 'fill'.",	"value", "0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]" ) {
		parameters.separator = 0;
		parameters.reference("value", value="0,0,0", "<i>,<j>,<f1>[,f2,f3,alpha]");

	};

	virtual
	void traverseChannels(ImageTray<Channel> & dst) const;

protected:

	std::string value;
};


class ImagePlotFile: public ImageMod {

public:

	ImagePlotFile() : ImageMod(__FUNCTION__, "Plots a given file.  See 'plot'."){
		parameters.reference("filename", filename = "",  "string");
	};

	virtual
	void traverseFrame(ImageFrame & dst) const;
	//void traverseChannel(Channel & dst) const {};

protected:

	std::string filename;

};

/// Traverses image, returning samples
/**
  As convenience, provides also a filename
 \code
 drainage gray.png  --sample file=samples-gray.txt
 drainage image.png --format '{i},{j}\t {0},{-2}' --sample 50,50,file=samples.txt
 \endcode
 */
class ImageSampler : public ImageMod {

public:

	/// Default constructor.
	ImageSampler();
	// WARN: sampler not allocated upon this point.
	//	parameters.reference("file", filename = "",  "string");

	/// Runs Sampler on the given image.
	/**
	 *  \param dst - target image
	 */
	virtual
	void process(Image & dst) const;

	inline
	const Sampler & getSampler(){ return sampler; };

	virtual inline
	const std::string & getFormat() const {return format;};

	virtual inline
	std::string & getFormat(){return format;};

protected:

	ImageSampler(const std::string & name, const std::string & description) : ImageMod(name, description){
		//setReferences();
	}


	void setReferences();

	//
	mutable Sampler sampler;

	/// Optional utility for commands using direct file output. The default constructor does not reference this.
	std::string filename;

	/// Output format, e.g. '{LON} {LAT} {AMVU} {AMVV} {QIND}'
	std::string format;

};



}  // namespace image

}  // namespace drain


#endif /* IMAGE_OP_H_ */
