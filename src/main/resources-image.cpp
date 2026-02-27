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


#include "resources-image.h"


namespace drain {



/*
DRAIN_ENUM_DICT(rack::PanelConfSVG::ElemClass) = {
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE_TITLE),
		// DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, BORDER),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, SIDE_PANEL),
		// DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};
*/



/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::GraphicsContext::ElemClass & key) const {
	// std::string(".")+
	return hasChild(Enum<rack::GraphicsContext::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/ **
 *
 * /
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & value) const {
	// std::string(".")+
	return (*this)[Enum<rack::GraphicsContext::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key){
	// std::string(".")+
	return (*this)[Enum<rack::GraphicsContext::ElemClass>::dict.getKey(key, false)];
}
*/

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x){
	return (*this)[std::string(".")+Enum<rack::GraphicsContext::ElemClass>::dict.getKey(x, false)];
}
 */

}



using namespace drain;
using namespace drain::image;

namespace rack {

/*
const drain::SelectorXMLcls RackSVG::clsTITLE(RackSVG::ElemClass::TITLE);
const drain::SelectorXMLcls RackSVG::clsIMAGE_TITLE(RackSVG::ElemClass::IMAGE_TITLE);
const drain::SelectorXMLcls RackSVG::clsGROUP_TITLE(RackSVG::ElemClass::GROUP_TITLE);
const drain::SelectorXMLcls RackSVG::clsMAIN_TITLE(RackSVG::ElemClass::MAIN_TITLE);
const drain::SelectorXMLcls RackSVG::clsTIME(RackSVG::ElemClass::TIME);
const drain::SelectorXMLcls RackSVG::clsLOCATION(RackSVG::ElemClass::LOCATION);
const drain::SelectorXMLcls RackSVG::clsIMAGE_BORDER(RackSVG::ElemClass::IMAGE_BORDER);
*/

GraphicsContext::GraphicsContext() {
}

GraphicsContext::GraphicsContext(const GraphicsContext & ctx) {
	//svgPanelConf.absolutePaths = ctx.svgPanelConf.absolutePaths;
	svgPanelConf.pathPolicyFlagger = ctx.svgPanelConf.pathPolicyFlagger;
	svgPanelConf.pathPolicy = ctx.svgPanelConf.pathPolicy;
	svgPanelConf.groupIdentifier = ctx.svgPanelConf.groupIdentifier;
	// svgTitles    = ctx.svgTitles;
}






ImageContext::ImageContext(): imagePhysical(true), qualityGroups(ODIMPathElem::NONE),
		currentImage(nullptr), currentGrayImage(nullptr) {
}

ImageContext::ImageContext(const ImageContext & ctx):
		imagePhysical(ctx.imagePhysical),
		qualityGroups(ctx.qualityGroups),
		currentImage(ctx.currentImage),
		currentGrayImage(ctx.currentGrayImage){
}

void ImageContext::refinePalette(Palette & palette){
	drain::Logger mout(__FILE__, __FUNCTION__);
	if (paletteResolution > 0){
		if (paletteResolution > palette.refinement){
			mout.revised("Refining palette to resolution (", paletteResolution, "), (current ", palette.refinement, ")");
			palette.refine(paletteResolution);
		}
		else {
			mout.note("Palette resolution (", paletteResolution, ") smaller than current (", palette.refinement, ") requested, skipping...");
		}
		paletteResolution = 0;
	}
};

Palette & ImageContext::getPalette(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("Requesting current palette: '", paletteKey, "'");

	// TODO: redirect to ImageContext::getPalette(paletteKey) instead:
	Palette & palette =	PaletteOp::getPalette(paletteKey);
	refinePalette(palette);
	return palette;
}

Palette & ImageContext::getPalette(const std::string & key){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//PaletteMap::iterator it = getPaletteEntry(const std::string & key);

	// }

	// PaletteMap::iterator it ImageContext::getPaletteEntry(const std::string & key){

	//paletteKey = key;

	// NEW: try direct and the QM based, only after that load
	PaletteMap & pmap = PaletteOp::getPaletteMap();

	/// Attempt 1: exact match of \c key
	mout.experimental<LOG_INFO>("Attempt 1: with exact key [", key, "]");
	PaletteMap::iterator it = pmap.find(key);
	if (it != pmap.end()){
		paletteKey = key;
		Palette & palette = it->second;
		refinePalette(palette);
		return palette;
	}

	/// Attempt 2: exact match of \c key

	mout.experimental<LOG_INFO>("Attempt 2: try quantities associated with  [", key, "]");

	const QuantityMap & qm = getQuantityMap();
	QuantityMap::const_iterator qit = qm.retrieve(key);
	if (qit != qm.end()){

		if (qit->first != key){
			mout.pending("Trying general quantity key: [", qit->first, "]");
			it = pmap.find(qit->first);
			if (it != pmap.end()){
				mout.success<LOG_NOTICE>("Palette[", qit->first, "] associated with key [", key, "] in quantityConf");
				//mout.ok("Found palette using general quantity key: [", qit->first, "]: \n", qit->second);
				paletteKey = it->first;
				Palette & palette = it->second;
				refinePalette(palette);
				return palette;
				// return it->second;
			}
		}
		else {
			mout.pending("Found [", qit->first, "], but same as original key...");
		}

		mout.pending("Testing quantity selectors of  [", qit->first, "]: ", qit->second.keySelector);

		for (const auto & ksel: qit->second.keySelector){
			mout.pending("Testing  [", ksel.value, "]... ");
			it = pmap.find(ksel.value);
			if (it != pmap.end()){
				paletteKey = it->first;
				Palette & palette = it->second;
				refinePalette(palette);
				return palette;
			}
		}

		mout.warn("Could not find palette with  [", key, "] or [", qit->first, "] (with its variants) ");

	}



	try {
		Palette & palette = PaletteOp::getPalette(key);
		paletteKey = key;
		refinePalette(palette);
		return palette;

		return palette;
	}
	catch (const std::exception & e){

		mout.fail("Tried everything...");
		throw e; // forward...

		//return PaletteOp::getPalette(key);
	}
}




// Note: for example Palette uses this to extract actual quantity
/**
 *  Note: for example Palette uses this to extract actual quantity
 *
 */
// std::string ImageContext::outputQuantitySyntax("${what:quantity}|${cmdKey}(${cmdArgs})");
std::string ImageContext::outputQuantitySyntax("${what:quantity}/${cmdKey}(${cmdArgs})");



void ImageContext::getImageInfo(const drain::image::Image *ptr, Variable & entry) {
	std::stringstream sstr;
	if (ptr != nullptr){
		ptr->toStream(sstr);
	}
	else {
		sstr << "NULL";
	}
	entry = sstr.str();
}

void ImageContext::updateImageStatus(drain::VariableMap & statusMap) const {

	getImageInfo(&colorImage, statusMap["img:colorImage"]);
	getImageInfo(&grayImage, statusMap["img:grayImage"]);
	getImageInfo(currentImage, statusMap["img:currentImage"]);
	getImageInfo(currentGrayImage, statusMap["img:currentGrayImage"]);

}






} // rack::

