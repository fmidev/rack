/*

    Copyright 2001 - 2017  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */


//#include <fstream>
#include "../image/File.h"
#include "../imageops/ImageOpBank.h"
#include "../imageops/ResizeOp.h"

//#include "CommandRegistry.h"
//#include "CommandPack.h"
#include "CommandOpticalFlow.h"

namespace drain {


void CmdOpticalFlowBase::getDiff(size_t width, size_t height, double max, ImageTray<Channel> & channels) const {

		//static Image diff(typename bean_t::data_t);
		static Image diff(typeid(double));
		diff.setName("Diff");
		//diff.initialize(typeid(double), width, height, 3, 1);
		diff.setCoordinatePolicy(CoordinatePolicy::LIMIT);
		diff.setGeometry(width, height, bean.getDiffChannelCount());

		diff.setPhysicalScale(-max,+max);
		//diff.setPhysicalScale(-max*max, max*max);
		channels.setChannels(diff);

		static Image diffQuality(typeid(double));
		diffQuality.setName("DiffQuality");
		diffQuality.setGeometry(width, height);
		diffQuality.setPhysicalScale(0.0,1.0);
		channels.setAlpha(diffQuality.getChannel(0));

}

void CmdOpticalFlowBase::debugChannels(const ImageTray<const Channel> & channels, int i, int j) const {

	// drain::Logger mout(this->getName(), __FUNCTION__);

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

	drain::Logger mout(this->getName(), __FUNCTION__);

	/// Input images: intensity
	ImageTray<const Channel> src;
	getSrcData(src); // may be smoothed at this point!

	const size_t w = src.getGeometry().getWidth();
	const size_t h = src.getGeometry().getHeight();
	// int debugI=180, debugJ=100;
	// for debugChannels()
	mout.debug() << "src0 " << src.get(0) << mout.endl;
	mout.debug() << "src1 " << src.get(1) << mout.endl;
	// debugChannels(src, debugI, debugJ);

	mout.info() << "Computing differentials (to be used as input for the actual algorithm)" << mout.endl;
	/// Differences (output from x, input for the actual algorithm)
	ImageTray<Channel> diff;
	const double max = src.get().requestPhysicalMax(100.0);
	mout.warn() << "guessing physical max: " << max << mout.endl;
	//ImageScaling diffScaling;
	getDiff(w, h, max*max, diff); // oflow2 = pow2 :-D
	mout.debug() << "diff images: \n" << diff << mout.endl;

	// bean.computeDifferentials(src.get(0), src.get(1), diff); // notice: src = latest loaded image
	bean.computeDifferentials(src, diff); // notice: src = latest loaded image

	mout.debug(1) << "computed diff\n" << diff << mout.endl;
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

	mout.info() << "Deriving motion field" << mout.endl;

	/// Main output (uField,vField, q)
	ImageTray<Channel> motion;

	const AreaGeometry & origArea = getGeometry();
	getMotion(origArea.getWidth(), origArea.getHeight(), motion);
	mout.debug() << "Allocated motion arrays\n" << motion << mout.endl;

	mout.debug() << "MAIN LOOP" << mout.endl;
	if (bean.optResize()){
		mout.debug(1) << "motion fields to be resized, using tmp" << mout.endl;
		// imitate actual data, motion
		Image tmp(typeid(double), w, h, motion.getGeometry().getImageChannelCount(),  motion.getGeometry().getAlphaChannelCount());
		ImageTray<Channel> motionDst;
		motionDst.setChannels(tmp);

		bean.traverseChannels(diff, motionDst); // Src

		// ImageTray<const Channel> motionSrc;
		// motionSrc.setChannels(tmp);
		mout.info() << "converting motion fields back to original size: " << origArea << mout.endl;
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
		mout.debug(1) << "direct computation to motion data" << mout.endl;
		bean.traverseChannels(diff, motion); // Src
	}

	//debugChannels(motion, debugI, debugJ);
	//debugChannels()
	// drain::image::File::write(motion.get(0), "oflow-motion-horz.png");

	// Final debugging
	if (mout.isDebug(20)){
		File::write(motion.get(0), "oflow-motion-horz.png");
		File::write(motion.get(1), "oflow-motion-vert.png");
		File::write(motion.getAlpha(), "oflow-motion-weight.png");
	}

	mout.info() << "Completed" << mout.endl;
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

