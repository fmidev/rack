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

#include <drain/Log.h>
#include <sstream>
#include "BlenderOp.h"
#include "CopyOp.h"
#include "ImageOpBank.h"

namespace drain {

namespace image {



void BlenderOp::traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const {
	//void BlenderOp::traverseChannel(const Channel &src, const Channel &srcWeight, Channel & dst, Channel & dstWeight) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__); //REPL getImgLog(), this->name, std::string(__FUNCTION__)); // +"(src,srcW, dst,dstW"

	mout.debug("start" );
	// File::write(src.get(), getName() + "0.png");
	mout.info(getParameters() ); //

	initializeParameters(src.get(), dst.get());


	if (loops == 0){
		//mout.error("zero loops, use copy instead..." );
		mout.note("zero loops, only copying data" );
		CopyOp copy;
		copy.traverseChannels(src, dst);
		//dst.get().copyData(src.get());
		return;
	}

	if (src.hasAlpha() ^ dst.hasAlpha()){
		mout.warn("src: " , src );
		mout.warn("dst: " , dst );
		mout.error("src/dst: only either has alpha" );
		return;
	}


	const bool WEIGHTED = src.hasAlpha();

	unsigned short loopsFinal = loops;

	UniCloner<ImageOp> cloner(getImageOpBank());

	const drain::SmartMap<std::string> & spreaderAliases = WEIGHTED ? getSmootherAliasMap<true>() : getSmootherAliasMap<false>();

	//ImageOp & spreaderOp = getSmoother(spreaderKey, WEIGHTED, loopsFinal);
	std::string spreaderName;
	std::string spreaderParams;

	drain::StringTools::split2(spreader, spreaderName, spreaderParams, "/");
	ImageOp & spreaderOp = cloner.getCloned(spreaderAliases.get(spreaderName, spreaderName)); //

	// getSmoother(spreaderKey, WEIGHTED, loopsFinal);
	const bool SPREADER_WINDOW = spreaderOp.getParameters().hasKey("width");

	Frame2D<int> frame(conf.frame);
	// Set width,height only if applicable...
	if (SPREADER_WINDOW){
		mout.info("frame: " , conf.frame , " -> " , frame );
		spreaderOp.setParameter("width", frame.tuple());
		//spreaderOp.setParameter("height", frame.height);
	}
	spreaderOp.setParameters(spreaderParams, ':', '/');

	mout.info("spreader: " , spreaderOp ); // .getName() << ' ' << spreader.getName()


	const drain::SmartMap<std::string> & blenderAliases = WEIGHTED ? getMixerAliasMap<true>() : getMixerAliasMap<false>();

	double coeff;
	drain::StringTools::convert(blender, coeff);

	std::string blenderKey = (coeff > 0.0) ? "blend" : blender;

	ImageOp & mixerOp = cloner.getCloned(blenderAliases.get(blenderKey, blenderKey));
	mout.info("blender: " , mixerOp ); // .getName() << ' ' << spreader.getName()

	//const Variable v(key, typeid(double));
	if (coeff > 0.0){
		mixerOp.setParameter("coeff", coeff);
	}


	ImageTray<Channel> tmp;
	Image tmpImg;
	if (loopsFinal > 1){
		//tmpImg.setConf(src.getConf())
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

		mout.info("pre-round (src->dst):  " , loop );
		//std::stringstream sstr;
		//sstr << getName() << loop;
		//File::write(src.get(), sstr.str() + "S1.png");
		spreaderOp.traverseChannels(src, dst);
		//File::write(src.get(), sstr.str() + "S2.png");
		if (loopsFinal == 0){
			mout.info("distance op, skipping remaining loops, loops=" , loops );
			return;
		}
		mixerOp.traverseChannels(src, dst);
		++loop;

	}
	else { // even

		mout.info("pre-round (A:src->tmp):  " , loop );
		spreaderOp.traverseChannels(src, tmp);
		mixerOp.traverseChannels(src, tmp);
		++loop;

		if (SPREADER_WINDOW){
			frame.width  *= expansionCoeff;
			frame.height *= expansionCoeff;
			spreaderOp.setParameter("width", frame);
		}

		mout.debug("pre-round (B:tmp->dst):  " , loop );
		spreaderOp.traverseChannels(tmp, dst);
		mixerOp.traverseChannels(src, dst);
		++loop;

	}


	// Repeat remaining 2N loops
	while (loop < loopsFinal){ // dst->tmp->dst

		if (SPREADER_WINDOW){
			frame.width  *= expansionCoeff;
			frame.height *= expansionCoeff;
			mout.special("frame -> " , frame );
			spreaderOp.setParameter("width", frame);
		}
		mout.info("round (dst->tmp):  " , loop );
		spreaderOp.traverseChannels(dst, tmp);
		mixerOp.traverseChannels(src, tmp);
		++loop;

		if (SPREADER_WINDOW){
			frame.width  *= expansionCoeff;
			frame.height *= expansionCoeff;
			spreaderOp.setParameter("width", frame);
		}

		mout.info("round (tmp->dst):  " , loop );
		spreaderOp.traverseChannels(tmp, dst);
		mixerOp.traverseChannels(src, dst);
		++loop;

	}


	mout.debug("end" );

}

/*
void BlenderOp::getSmootherAliasMap(drain::SmartMap<std::string> & aliasMap, bool weighted) const {
	//static drain::SmartMap<std::string> aliasMap;
	aliasMap["a"] = "average";
	aliasMap["f"] = "flowAverage"; // magnitude/energy saving
	aliasMap["g"] = "gaussianAverage";
	aliasMap["d"] = weighted ? "distanceTransformFill"    : "distanceTransform";
	aliasMap["D"] = weighted ? "distanceTransformFillExp" : "distanceTransformExp";
}
*/

/*
ImageOp & BlenderOp::getSmoother(const std::string & key, bool weighted, unsigned short & loops) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);
	const drain::SmartMap<std::string> & aliasMap = weighted ? getSmootherAliasMap<true>() : getSmootherAliasMap<false>();

	ImageOpBank & bank = getImageOpBank();

	if (key == "blender"){
		mout.error("blender cannot call itself: key=" , key );
	}

	ImageOp & op = bank.getComplete(key, ':', '/', aliasMap);
	if (op.getName().find("DistanceTransform") == 0){ // +"Fill" +"Exp"/"Linear"
		if (loops > 1)
			mout.note("discarding further loops (" , loops , "), inapplicable with " , op.getName() );
		loops = 0;
	}

	return op;

}
*/

/*
ImageOp & BlenderOp::getMixer(const std::string & key, bool weighted) const {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	ImageOpBank & bank = getImageOpBank();

	double coeff;
	drain::StringTools::convert(key, coeff);


	const drain::SmartMap<std::string> & aliasMap = weighted ? getMixerAliasMap<true>() : getMixerAliasMap<false>();

	//const Variable v(key, typeid(double));
	if (coeff > 0.0){
		//mout.debug() <<
		ImageOp & op = bank.get(aliasMap["mix"]);
		op.setParameter("coeff", coeff);
		return op;
	}
	else {
		ImageOp & op = bank.getComplete(key,':','/', aliasMap); // basically mix possible, but direct coeff (above) preferred
		return op;
	}

}
*/


}  // namespace image

}  // namespace drain


// Drain
