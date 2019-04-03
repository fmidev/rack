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

#include <sstream>
#include "util/Log.h"

#include "BlenderOp.h"
#include "CopyOp.h"
#include "ImageOpBank.h"

namespace drain {

namespace image {



void BlenderOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
	//void BlenderOp::traverseChannel(const Channel &src, const Channel &srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), this->name, std::string(__FUNCTION__)); // +"(src,srcW, dst,dstW"

	mout.debug() << "start" << mout.endl;
	// File::write(src.get(), getName() + "0.png");

	initializeParameters(src.get(), dst.get());


	if (loops == 0){
		//mout.error() << "zero loops, use copy instead..." << mout.endl;
		mout.note() << "zero loops, only copying data" << mout.endl;
		CopyOp copy;
		copy.traverseChannels(src, dst);
		//dst.get().copyData(src.get());
		return;
	}

	if (src.hasAlpha() ^ dst.hasAlpha()){
		mout.warn() << "src: " << src << mout.endl;
		mout.warn() << "dst: " << dst << mout.endl;
		mout.error() << "src/dst: only either has alpha" << mout.endl;
		return;
	}




	const bool WEIGHTED = src.hasAlpha();

	/// Single-pass, and no mixing needed
	//const bool SIMPLE = (smootherKey == 'd')||(smootherKey == 'D');
	// if (SIMPLE && (loops > 1))
	// mout.note() << "single-loop distance op, requested loops=" << loops << mout.endl;

	unsigned short loopsFinal = loops;

	ImageOp & smootherOp = getSmoother(smootherKey, WEIGHTED, loopsFinal);
	// Set width,height leniently
	if (smootherOp.getParameters().hasKey("width"))
		smootherOp.setParameter("width", conf.getWidth());
	if (smootherOp.getParameters().hasKey("height"))
		smootherOp.setParameter("height", conf.getHeight());
	//smootherOp.setParameters();

	mout.info() << "smoother:" << smootherOp << mout.endl; // .getName() << ' ' << smoother.getName()

	ImageOp & mixerOp = getMixer(mixerKey, WEIGHTED);
	if (loops > 0){
		mout.info() << "mixer:  "  << mixerOp    << mout.endl; // .getName() << ' ' << smoother.getName()
	}

	ImageTray<Channel> tmp;
	Image tmpImg;
	if (loopsFinal > 1){
		tmpImg.initialize(src.get().getType(), src.getGeometry());
		//tmpImg.copyShallow(src.get()); // also scaling!
		tmp.setChannels(tmpImg);
	}

	unsigned short loop = 0;

	if (dst.hasAlpha())
		dst.getAlpha().clear();

	//File::write(src.get(), getName() + "Y.png");

	// Process one or two steps, ensuring ending at dst
	if ((loopsFinal==0) || ((loopsFinal&1)==1)){ // odd

		mout.info() << "pre-round (src->dst):  " << loop << mout.endl;
		//std::stringstream sstr;
		//sstr << getName() << loop;
		//File::write(src.get(), sstr.str() + "S1.png");
		smootherOp.traverseChannels(src, dst);
		//File::write(src.get(), sstr.str() + "S2.png");
		if (loopsFinal == 0){
			mout.info() << "distance op, skipping remaining loops, loops=" << loops << mout.endl;
			return;
		}
		mixerOp.traverseChannels(src, dst);
		++loop;

	}
	else { // even

		mout.info() << "pre-round (A:src->tmp):  " << loop << mout.endl;
		smootherOp.traverseChannels(src, tmp);
		mixerOp.traverseChannels(src, tmp);
		++loop;

		mout.debug() << "pre-round (B:tmp->dst):  " << loop << mout.endl;
		smootherOp.traverseChannels(tmp, dst);
		mixerOp.traverseChannels(src, dst);
		++loop;

	}


	// Repeat remaining 2N loops
	while (loop < loopsFinal){ // dst->tmp->dst

		mout.info() << "round (dst->tmp):  " << loop << mout.endl;
		smootherOp.traverseChannels(dst, tmp);
		mixerOp.traverseChannels(src, tmp);
		++loop;

		mout.debug() << "round (tmp->dst):  " << loop << mout.endl;
		smootherOp.traverseChannels(tmp, dst);
		mixerOp.traverseChannels(src, dst);
		++loop;

	}


	mout.debug() << "end" << mout.endl;

}


void BlenderOp::getSmootherAliasMap(drain::SmartMap<std::string> & aliasMap, bool weighted) const {
	//static drain::SmartMap<std::string> aliasMap;
	aliasMap["a"] = "average";
	aliasMap["f"] = "flowAverage"; // magnitude/energy saving
	aliasMap["g"] = "gaussianAverage";
	aliasMap["d"] = weighted ? "distanceTransformFill"    : "distanceTransform";
	aliasMap["D"] = weighted ? "distanceTransformFillExp" : "distanceTransformExp";
}

ImageOp & BlenderOp::getSmoother(const std::string & key, bool weighted, unsigned short & loops) const {

	Logger mout(getImgLog(), this->name, __FUNCTION__);
	drain::SmartMap<std::string> aliasMap;
	getSmootherAliasMap(aliasMap, weighted);
	/*
	drain::SmartMap<std::string> aliasMap;
	aliasMap["a"] = "average";
	aliasMap["f"] = "flowAverage"; // magnitude/energy saving
	aliasMap["g"] = "gaussianAverage";
	aliasMap["d"] = weighted ? "distanceTransformFill"    : "distanceTransform";
	aliasMap["D"] = weighted ? "distanceTransformFillExp" : "distanceTransformExp";
	*/
	ImageOpBank & bank = getImageOpBank();

	if (key == "blender"){
		mout.error() << "blender cannot call itself: key=" << key << mout.endl;
	}

	ImageOp & op = bank.getComplete(key, ':', '/', aliasMap);
	if (op.getName().find("DistanceTransform") == 0){ // +"Fill" +"Exp"/"Linear"
		if (loops > 1)
			mout.note() << "discarding further loops (" << loops << "), inapplicable with " << op.getName() << mout.endl;
		loops = 0;
	}

	return op;

}

ImageOp & BlenderOp::getMixer(const std::string & key, bool weighted) const {

	Logger mout(getImgLog(), this->name, __FUNCTION__);

	ImageOpBank & bank = getImageOpBank();

	drain::SmartMap<std::string> aliasMap;
	aliasMap["b"] = weighted ? "qualityMixer" : "mix";
	aliasMap["m"] = weighted ? "qualityOverride" : "max";

	const Variable v(key, typeid(double));
	if ((double)v > 0.0){
		ImageOp & op = bank.get(aliasMap["b"]).get();
		op.setParameter("coeff", v);
		return op;
	}
	else {
		ImageOp & op = bank.getComplete(key,':','/', aliasMap);
		return op;
	}

	// ImageOp & op = bank.getComplete(key,':','/', aliasMap);
	// return op;

}

void BlenderOp::initRefs() {
	drain::SmartMap<std::string> aliasMap;
	getSmootherAliasMap(aliasMap);
	parameters.reference("smooth", this->smootherKey, aliasMap.toStr()); //"a|g|d|D; avg, gaussianAvg, dist, distExp");
	parameters.reference("mix", this->mixerKey, "b[/coeff:<coeff>]|m; (quality) mix, (quality) max");
	parameters.reference("loops", this->loops, "number of repetitions");
}

}  // namespace image

}  // namespace drain


// Drain
