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


//#include <fstream>
#include "drain/image/ImageFile.h"
#include "drain/imageops/ImageOpBank.h"
#include "drain/imageops/ResizeOp.h"

//#include "CommandRegistry.h"
//#include "CommandPack.h"
#include "CommandOpticalFlow.h"

namespace drain {


void CmdOpticalFlowBase::getDiff(size_t width, size_t height, double max, ImageTray<Channel> & channels) const {

		//static Image diff(typename bean_t::data_t);
		static Image diff(typeid(double));
		diff.setName("Diff");
		//diff.initialize(typeid(double), width, height, 3, 1);
		diff.setCoordinatePolicy(EdgePolicy::LIMIT);
		diff.setGeometry(width, height, bean.getDiffChannelCount());

		//diff.setPhysicalScale(-max,+max);
		// ===
		diff.setPhysicalRange(-max, +max, true);
		//diff.setOptimalScale();

		//diff.s

		//diff.setPhysicalScale(-max*max, max*max);
		channels.setChannels(diff);

		static Image diffQuality(typeid(double));
		diffQuality.setName("DiffQuality");
		diffQuality.setGeometry(width, height);
		//diffQuality.setPhysicalScale(0.0,1.0);
		diff.setPhysicalRange(0.0, 1.0, true);
		//diff.setOptimalScale();

		channels.setAlpha(diffQuality.getChannel(0));

}

void CmdOpticalFlowBase::debugChannels(const ImageTray<const Channel> & channels, int i, int j) const {

	// drain::Logger mout(__FILE__, __FUNCTION__);

	std::cerr << "'" << channels.get().getName() << "':\n";

	const Geometry & g = channels.getGeometry();

	if (i < 0)
		i = g.getWidth()/2;
	if (j < 0)
		j = g.getHeight()/2;

	std::cerr << "(" << i << ",\t" << j << ")\t";

	for (ImageTray<const Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		const Channel & c = it->second;
		std::cerr << c.getScaled(i,j) << '\t';
	}
	if (channels.hasAlpha())
		std::cerr << '{' << channels.getAlpha().getScaled(i,j) << '}';
	std::cerr << '\n';

}


void CmdOpticalFlowBase::exec() const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	/// Input images: intensity
	ImageTray<const Channel> src;
	getSrcData(src); // may be smoothed at this point!

	const size_t w = src.getGeometry().getWidth();
	const size_t h = src.getGeometry().getHeight();
	// int debugI=180, debugJ=100;
	// for debugChannels()
	mout.debug("src0 " , src.get(0) );
	mout.debug("src1 " , src.get(1) );
	// debugChannels(src, debugI, debugJ);

	mout.info("Computing differentials (to be used as input for the actual algorithm)" );
	/// Differences (output from x, input for the actual algorithm)
	ImageTray<Channel> diff;
	const double max = src.get().getConf().requestPhysicalMax(100.0);
	mout.note("guessing physical max: " , max );
	//ImageScaling diffScaling;
	getDiff(w, h, max*max, diff); // oflow2 = pow2 :-D
	mout.debug("diff images: \n" , diff );

	// bean.computeDifferentials(src.get(0), src.get(1), diff); // notice: src = latest loaded image
	bean.computeDifferentials(src, diff); // notice: src = latest loaded image

	mout.debug2("computed diff\n" , diff );
	//debugChannels(diff, debugI, debugJ);
	//drain::image::File::write(src.get(0), "oflow-src0.png");
	//drain::image::File::write(diff.get(0), "diff-dx.png");
	/*
	if (mout.isDebug(20)){
		bean.writeDebugImage(src.get(0), "oflow-src0.png");
		bean.writeDebugImage(src.get(1), "oflow-src1.png");
		bean.writeDebugImage(diff.get(0), "diff-dx.png", 0.05, 0.5);
		bean.writeDebugImage(diff.get(1), "diff-dy.png", 0.05, 0.5);
		bean.writeDebugImage(diff.get(2), "diff-dt.png", 0.5, 0.5);
	}
	*/

	mout.info("Deriving motion field" );

	/// Main output (uField,vField, q)
	ImageTray<Channel> motion;

	const AreaGeometry & origArea = getGeometry();
	getMotion(origArea.getWidth(), origArea.getHeight(), motion);
	mout.debug("Allocated motion arrays\n" , motion );

	mout.debug("MAIN LOOP" );
	if (bean.optResize()){
		mout.debug2("motion fields to be resized, using tmp" );
		// imitate actual data, motion
		Image tmp(typeid(double), w, h, motion.getGeometry().channels.getImageChannelCount(),  motion.getGeometry().channels.getAlphaChannelCount());
		ImageTray<Channel> motionDst;
		motionDst.setChannels(tmp);

		bean.traverseChannels(diff, motionDst); // Src

		// ImageTray<const Channel> motionSrc;
		// motionSrc.setChannels(tmp);
		mout.info("converting motion fields back to original size: " , origArea );
		drain::image::ResizeOp op;
		op.setGeometry(origArea);

		/// AMVU
		op.setScale(origArea.getWidth(), w);
		op.traverseChannel(tmp.getChannel(0), motion.get(0));
		/// AMVV
		op.setScale(origArea.getHeight(), h);
		op.traverseChannel(tmp.getChannel(1), motion.get(1));
		/// QIND
		op.setScale(1.0); // ?
		op.traverseChannel(tmp.getAlphaChannel(), motion.getAlpha());
		// drain::image::ResizeOp(origArea.getWidth(), origArea.getHeight()).traverseChannels(motionSrc, motion);
	}
	else {
		mout.debug2("direct computation to motion data" );
		bean.traverseChannels(diff, motion); // Src
	}

	//debugChannels(motion, debugI, debugJ);
	//debugChannels()
	// drain::image::File::write(motion.get(0), "oflow-motion-horz.png");

	// Final debugging
	if (mout.isDebug(20)){
		drain::image::ImageFile::write(motion.get(0), "oflow-motion-horz.png");
		drain::image::ImageFile::write(motion.get(1), "oflow-motion-vert.png");
		drain::image::ImageFile::write(motion.getAlpha(), "oflow-motion-weight.png");
	}

	mout.info("Completed" );
}


} /* namespace drain */



/*
		const string & timeFormat =  options["mTimeFormat"];
		if (!timeFormat.empty()){
			Time tPrev,t;
			cerr << srcPrev.getName() << '\n';
			tPrev.setTime(srcPrev.getName(),timeFormat);
			cerr << src.getName() << '\n';
			t.setTime(src.getName(),timeFormat);
			cerr << "offset:";
			int offsetMinutes = (t.getTime() - tPrev.getTime())/60;
			if (drain::Debug > 0){
				cerr << " Time offset: " << offsetMinutes << " mins.\n";
			}
			if (offsetMinutes != 0)
				motion.properties["offsetMinutes"] = offsetMinutes;
			else
				motion.properties.erase("offsetMinutes");
		}

 */


// Rack
