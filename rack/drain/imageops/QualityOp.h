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
