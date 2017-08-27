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

#include <set>
#include <map>
#include <ostream>

#include "util/Debug.h"
//#include "util/Data.h"
#include "util/RegExp.h"

#include "image/Image.h"
#include "image/File.h"

#include "image/FastOpticalFlowOp.h"

//  ******  NEW scheme ******

#include "Command.h"
#include "CommandPack.h"

#include "CommandRegistry.h"


//#include  "Drainage.h"

#include "Commands-ImageTools.h"
#include "Commands-ImageMotion.h"



namespace drain {

using namespace image;


class CmdMotionTimeFormat : public SimpleCommand<std::string> {
    public: //re 


	CmdMotionTimeFormat() : SimpleCommand<std::string>(__FUNCTION__, "Set time format for motion", "format", "?????") {
	};

	virtual
	void exec() const {
		/*
		drain::MonitorSource mout("CmdMotionTimeFormat", __FUNCTION__);
		//mout.note() << "file:" << value << mout.endl;
		Drainage & r = getDrainage();
		Image & dst = r.getSrc();
		CoordinatePolicy p;
		Variable v;
		v.setType<size_t>();
		v = value;
		switch (v.getElementCount()) {
			case 4:
				//dst.setCoordinatePolicy(v.get<int>(0), v.get<int>(1),v.get<int>(2),v.get<int>(3));
				p.yOverFlowPolicy  = v.get<int>(3);
				p.xOverFlowPolicy  = v.get<int>(2);
				p.yUnderFlowPolicy = v.get<int>(1);
				p.xUnderFlowPolicy = v.get<int>(0);
				break;
			case 1:
				//dst.setCoordinatePolicy(v.get<int>(0), v.get<int>(0),v.get<int>(0),v.get<int>(0));
				p.yOverFlowPolicy  = v.get<int>(0);
				p.xOverFlowPolicy  = v.get<int>(0);
				p.yUnderFlowPolicy = v.get<int>(0);
				p.xUnderFlowPolicy = v.get<int>(0);
				break;
			default:
				mout.error() << "Wrong number of parameters (not 1 or 4): " << v.getElementCount() << " (" << value << ")" << mout.endl;
				break;
		}
		dst.setCoordinatePolicy(p);
		*/
	}

};


DrainageImageMotion::DrainageImageMotion(const std::string & section, const std::string & prefix) : CommandGroup(section, prefix) {

	static CommandEntry<CmdMotionTimeFormat> mTimeFormat("timeFormat");

}



class CmdOpticalFlowOLD : public BasicCommand {
    public: //re 

	// size_t width; 0UL
	// size_t height;

	FastOpticalFlowOp op;

	CmdOpticalFlowOLD() : BasicCommand(__FUNCTION__, "Optical flow") {

		parameters.reference("width", op.conf.width=3, "pix");
		parameters.reference("height", op.conf.height=3, "pix");

		parameters.reference("gradPow", op.gradPow=1.0, "scalar");
		//parameters.reference("gradWidth", op.gradWidth, 1.0, "pix");

		//parameters.reference("imageChannels", imageChannelCount, 1UL, "");
		//parameters.reference("alphaChannels", alphaChannelCount, 0UL, "");

	};

	/*
	virtual
	void exec() const {
		drain::MonitorSource mout(name, __FUNCTION__);
		mout.warn() << "op:" << op << mout.endl;

		Drainage & drainage = getDrainage();

		Image motion(typeid(double));

		Image diff(typeid(double));
		op.computeDerivativeImage(drainage.getDst(), drainage.getSrc(), diff); // notice: src = latest loaded image
		File::write(diff,   "oflow-diff.png");
		op.filter(diff, motion);
		File::write(motion, "oflow-motion.png");
	}
	*/

		/*
		const std::string & timeFormat =  options["mTimeFormat"];
		if (!timeFormat.empty()){
			Time tPrev,t;
			std::cerr << srcPrev.getName() << '\n';
			tPrev.setTime(srcPrev.getName(),timeFormat);
			std::cerr << src.getName() << '\n';
			t.setTime(src.getName(),timeFormat);
			std::cerr << "offset:";
			int offsetMinutes = (t.getTime() - tPrev.getTime())/60;
			if (drain::Debug > 0){
				std::cerr << " Time offset: " << offsetMinutes << " mins.\n";
			}
			if (offsetMinutes != 0)
				motion.properties["offsetMinutes"] = offsetMinutes;
			else
				motion.properties.erase("offsetMinutes");
		}


		Image diff(typeid(double));
		FastOpticalFlowOp op;
		op.setParameters(value);
		op.computeDerivativeImage(srcPrev,src,diff); // notice: src = latest loaded image
		op.filter(diff,motion);

		Image tmp;  // debug

		if (mout.isDebug(10)){
			// ScaleOp(1,0).filter(motion.getAlphaChannel(),tmp);
			File::write(tmp,"oflow-wFinalOutA.png");
		}
		//ScaleOp<double,unsigned char>(1,0).filter(motion.getChannel(3),tmp);
		//File::write(tmp,"oflow-wFinalOut.png");


		// Post-processing
		const std::string mFilterParams = options["mFilterParams"];
		if (!mFilterParams.empty()){
			RecursiveRepairerOp op;
			op.setParameters(mFilterParams);
			op.filter(motion,motion);
			if (drain::Debug > 4){
				// ScaleOp(1,0).filter(motion.getAlphaChannel(),tmp);
				File::write(tmp,"oflow-wFinalOutRec.png");
			}
		}


		*/


};


//CommandAdapter<CmdView> cmdView(getDrainage().generalCommands, "view",'V');









} // drain::


// Drain
