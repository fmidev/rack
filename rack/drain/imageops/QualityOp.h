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
#ifndef QUALITY_OP_H_
#define QUALITY_OP_H_


#include "ImageOp.h"

namespace drain
{

namespace image
{


class QualityOp : public ImageOp {

public:

	QualityOp(const std::string & name, const std::string & description) : ImageOp(name, description){
	};

	/// Essentially same as in ImageOp?
	virtual
	void makeCompatible(const ImageFrame & src, Image & dst) const;

	// TODO:

	/*
	virtual
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const {
		ImageTray<const Channel> srcTray;
		srcTray.setChannels(src, srcAlpha);
		ImageTray<Channel> dstTray;
		dstTray.setChannels(dst, dstAlpha);
		traverseChannels(srcTray, dstTray);
	};
	*/
};



class QualityThresholdOp : public QualityOp {

public:

	inline
	QualityThresholdOp(double threshold=0.0, double replace=NAN) :
		QualityOp(__FUNCTION__, "Threshold data with quality") {
		parameters.reference("threshold", this->threshold = threshold, "0.0...1.0");
		parameters.reference("replace", this->replace = replace);
	};

	virtual inline
	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
		traverseChannelsSeparately(src, dst);
			// traverseChannelsEqually(src, dst);
		// traverseChannelsRepeated(src, dst);
		// traverseMultiChannel(src, dst);
	}

	virtual
	void traverseChannel(const Channel &src, const Channel &srcAlpha, Channel &dst, Channel &dstAlpha) const;

	double threshold;
	double replace;

};


}
}

#endif /*QUALITY_OP_H_*/

// Drain
