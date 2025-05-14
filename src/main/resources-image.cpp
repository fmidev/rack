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


template <>
const drain::EnumDict<rack::SvgInclude>::dict_t  drain::EnumDict<rack::SvgInclude>::dict = {
		DRAIN_ENUM_ENTRY(rack::SvgInclude, NONE),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, PNG),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, SVG),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, TXT),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, ALL),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, NEXT),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, SKIP),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, ON),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, OFF),
		DRAIN_ENUM_ENTRY(rack::SvgInclude, UNKNOWN),
};


template <>
const drain::EnumDict<rack::PanelConfSVG::ElemClass>::dict_t  drain::EnumDict<rack::PanelConfSVG::ElemClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, GROUP),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE),
		// DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, SIDE_PANEL),
		DRAIN_ENUM_ENTRY(rack::PanelConfSVG::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};



template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::PanelConfSVG::ElemClass & key) const {
	// std::string(".")+
	return hasChild(EnumDict<rack::PanelConfSVG::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/**
 *
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & value) const {
	// std::string(".")+
	return (*this)[EnumDict<rack::PanelConfSVG::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::PanelConfSVG::ElemClass & key){
	// std::string(".")+
	return (*this)[EnumDict<rack::PanelConfSVG::ElemClass>::dict.getKey(key, false)];
}

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::GraphicsContext::ElemClass & key) const {
	// std::string(".")+
	return hasChild(EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/ **
 *
 * /
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & value) const {
	// std::string(".")+
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key){
	// std::string(".")+
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, false)];
}
*/

/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x){
	return (*this)[std::string(".")+EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(x, false)];
}
 */

}



using namespace drain;
using namespace drain::image;

namespace rack {

/*
const drain::SelectorXMLcls PanelConfSVG::clsTITLE(PanelConfSVG::ElemClass::TITLE);
const drain::SelectorXMLcls PanelConfSVG::clsIMAGE_TITLE(PanelConfSVG::ElemClass::IMAGE_TITLE);
const drain::SelectorXMLcls PanelConfSVG::clsGROUP_TITLE(PanelConfSVG::ElemClass::GROUP_TITLE);
const drain::SelectorXMLcls PanelConfSVG::clsMAIN_TITLE(PanelConfSVG::ElemClass::MAIN_TITLE);
const drain::SelectorXMLcls PanelConfSVG::clsTIME(PanelConfSVG::ElemClass::TIME);
const drain::SelectorXMLcls PanelConfSVG::clsLOCATION(PanelConfSVG::ElemClass::LOCATION);
const drain::SelectorXMLcls PanelConfSVG::clsIMAGE_BORDER(PanelConfSVG::ElemClass::IMAGE_BORDER);
*/

GraphicsContext::GraphicsContext() {
}

GraphicsContext::GraphicsContext(const GraphicsContext & ctx) {
	svgPanelConf.absolutePaths = ctx.svgPanelConf.absolutePaths;
	svgPanelConf.groupTitleSyntax = ctx.svgPanelConf.groupTitleSyntax;
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

Palette & ImageContext::getPalette(const std::string & key){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//paletteKey = key;

	// NEW: try direct and the QM based, only after that load
	PaletteMap & pmap = PaletteOp::getPaletteMap();

	/// Attempt 1: exact match of \c key
	mout.special("Attempt 1: with exact key [", key, "]");
	PaletteMap::iterator it = pmap.find(key);
	if (it != pmap.end()){
		paletteKey = key;
		return it->second;
	}

	/// Attempt 2: exact match of \c key

	mout.special("Attempt 2: try quantities associated with  [", key, "]");

	const QuantityMap & qm = getQuantityMap();
	QuantityMap::const_iterator qit = qm.retrieve(key);
	if (qit != qm.end()){

		if (qit->first != key){
			mout.pending("Trying general quantity key: [", qit->first, "]");
			it = pmap.find(qit->first);
			if (it != pmap.end()){
				mout.ok("Found palette using general quantity key: [", qit->first, "]");
				//mout.ok("Found palette using general quantity key: [", qit->first, "]: \n", qit->second);
				paletteKey = it->first;
				return it->second;
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
				return it->second;
			}
		}

		mout.warn("Could not find palette with  [", key, "] or [", qit->first, "] (with its variants) ");

	}



	try {
		return PaletteOp::getPalette(key);
	}
	catch (const std::exception & e){

		mout.fail("Tried everything...");

		/*
		mout.special("Failed with [", key, "], now trying QM search...");


		const QuantityMap & qm = getQuantityMap();

		mout.special("Failed with [", key, "], now trying QM search...");

		QuantityMap::const_iterator it = qm.retrieve(key);
		if (it != qm.end()){
			mout.special("Found: [", it->first, "]: \n", it->second);
			if (it->first != key){
				mout.special("Trying QM [", it->first, "]");
				Palette & p = PaletteOp::getPalette(it->first);
				mout.accept<LOG_NOTICE>("Found [", it->first, "] ->", p.comment);
				return p;
			}
			else {
				mout.warn("Problem: [", it->first, "] != ", key);
			}
		}
		*/
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

