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

//
// #include <drain/image/MouseXML.h>
// #include <drain/image/LayoutSVG.h>

// #include "data/SourceODIM.h"
#include "resources-svg.h"
#include "graphics-base.h"

//using namespace drain;
//using namespace drain::image;

namespace rack {


GraphicsContext::GraphicsContext() {
}

GraphicsContext::GraphicsContext(const GraphicsContext & ctx) {
	//svgPanelConf.absolutePaths = ctx.svgPanelConf.absolutePaths;
	svgPanelConf.pathPolicyFlagger = ctx.svgPanelConf.pathPolicyFlagger;
	svgPanelConf.pathPolicy = ctx.svgPanelConf.pathPolicy;
	svgPanelConf.groupIdentifier = ctx.svgPanelConf.groupIdentifier;
	// svgTitles    = ctx.svgTitles;
}


drain::image::TreeSVG & GraphicsContext::getMainGroup(){ // , const std::string & name

	drain::Logger mout(__FILE__, __FUNCTION__);

	static std::string MAIN("MAIN");

	// Ensure STYLE elem and definitions
	RackSVG::addStyle(getStyle());

	TreeSVG & mainGroup = svgTrack[MAIN];

	if (mainGroup -> isUndefined()){
		mainGroup->setType(svg::GROUP);
		mainGroup->addClass(MAIN); // TitleCreatorSVG::visitPostfix
		mainGroup->addClass(LayoutSVG::ADAPTER); // needed?
	}

	return mainGroup;

}

drain::image::TreeSVG & GraphicsContext::getOnLoadScript(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	static const std::string onload_fnc_name("rack_onload");

	mout.experimental<LOG_INFO>("Adding mouse interaction: SVG onload=", onload_fnc_name, "()");

	svgTrack->set("onload", onload_fnc_name+"()"); // perhaps repeatedly

	return drain::UtilsXML::ensureJavaScriptFunction(svgTrack, onload_fnc_name)[svg::JAVASCRIPT_SCOPE](svg::JAVASCRIPT_SCOPE);
	// return drain::UtilsXML::ensureJavaScriptFunctionScope(ctx.svgTrack, onload_fnc_name);

}

drain::image::TreeSVG & GraphicsContext::getJavaScriptDefs(){

	drain::image::TreeSVG & scope = drain::UtilsXML::getHeaderObject(svgTrack, svg::SCRIPT, "Rack");
	if (scope.empty()){
		scope->setText("const Rack = {cls:{}, test:{}};\n");
	}

	return scope;

}

drain::image::TreeSVG & GraphicsContext::addJavaScripsDef(const std::string & key, const std::string & section){

	std::string varName = section.empty() ? key : section+'.'+key;
	drain::image::TreeSVG & line = getJavaScriptDefs()[varName](svg::CTEXT);

	if (section == "cls"){
		line->setText("Rack.", varName, '=', drain::sprinter("."+key, drain::Sprinter::jsLayout).str(), ";\n");
	}
	else {
		line->setText("Rack.", varName, '=', drain::sprinter(key, drain::Sprinter::jsLayout).str(), ";\n");
	}
	return line;
}

/** Intermediate group "hiding" translation that moves upper left corner of the object to the origin.
 *
 */
drain::image::TreeSVG & GraphicsContext::ensureAdapterGroup(drain::image::TreeSVG & group){

	// Enum<drain::image::LayoutSVG::GroupType>::dict::getKey(LayoutSVG::ADAPTER)

	drain::image::TreeSVG & adapterGroup = group[LayoutSVG::ADAPTER];

	if (adapterGroup->isUndefined()){
		adapterGroup->setType(svg::GROUP);
		adapterGroup->addClass(LayoutSVG::ADAPTER);
		adapterGroup->transform.translate.set(0,0); // DOES NOTHING?
		adapterGroup.addChild()->setComment("Frame containing objects to be moved (translated) collectively");
		// adapterGroup[svg::IMAGE]->setType(svg::GROUP);
	}

	return adapterGroup;

}

bool GraphicsContext::consumeAlignRequest(drain::image::NodeSVG & node){

	// TODO: return flags of set aligns?

	drain::Logger mout(__FILE__, __FUNCTION__);
	bool CHANGE = false;

	if (anchorHorz.isSet()){
		node.setMyAlignAnchor<AlignBase::HORZ>(anchorHorz);
		anchorHorz.clear();
		CHANGE = true;
	}

	if (anchorVert.isSet()){
		node.setMyAlignAnchor<AlignBase::VERT>(anchorVert);
		anchorVert.clear();
		CHANGE = true;
	}

	//if (ctx.alignHorz.pos != AlignBase::UNDEFINED_POS){
	if (alignHorz.isSet()){ // experimental...
		node.setAlign(AlignBase::HORZ, alignHorz.pos, alignHorz.getOrDefault(MutualAlign::INSIDE));  // simplify
		node.addClass(LayoutSVG::INDEPENDENT); // why is this? hmm explicitly, specifically aligned element is "separate"
		// mout.attention("updated Horz align: ",  node); // , " -> all:", node.getAlignStr()
		alignHorz.reset();
		alignHorz.set(AlignBase::UNDEFINED_POS, MutualAlign::INSIDE);
		//mout.attention(" HORZ state now: ", ctx.alignHorz);
		CHANGE = true;
	}


	//if (ctx.alignVert.pos != AlignBase::UNDEFINED_POS){
	if (alignVert.isSet()){ // experimental...
		node.setAlign(AlignBase::VERT, alignVert.pos, alignVert.getOrDefault(MutualAlign::INSIDE)); // simplify
		node.addClass(LayoutSVG::INDEPENDENT); // why is this? hmm explicitly, specifically aligned element is "separate"
		// mout.attention("updated Vert align: ",  node); //  " -> all:", node.getAlignStr()
		alignVert.reset();
		alignVert.set(AlignBase::UNDEFINED_POS, MutualAlign::INSIDE);
		// mout.attention(" VERT state now: ", ctx.alignVert);
		CHANGE = true;
	}

	return CHANGE;
}



drain::image::TreeSVG & GraphicsContext::getCurrentAdapterGroup(){ // what about prefix?

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & stackedGroup = getStackedGroup();
	drain::image::TreeSVG & adapterGroup = ensureAdapterGroup(stackedGroup);

	return adapterGroup;

}



drain::image::TreeSVG & GraphicsContext::getImagePanelGroup(const drain::FilePath & filepath, bool unique){

	drain::Logger mout(__FILE__, __FUNCTION__);


	// For each image an own group is created to contain also title TEXT's etc.
	drain::image::TreeSVG & adapterGroup = getCurrentAdapterGroup();

	drain::StringBuilder<'_'> identifier("imagePanel", filepath.tail, filepath.extension);

	if (unique){
		// The same background image may be linked repeatedly, yet creating a new image panel every time.
		identifier.add(NodeSVG::getNewIndex());
	}


	drain::StringTools::getSafeKey(identifier, currentImagePanel, "-", ".,:+");


	// imageGroup -> addClass(ClipperSVG::CLIPPED);
	TreeSVG & imagePanelGroup = adapterGroup[currentImagePanel];
	if (imagePanelGroup->isUndefined()){
		imagePanelGroup->setType(svg::GROUP);
		//imagePanelGroup->setId(ctx.currentImagePanel); // thread safety
		//imagePanelGroup->addClass(RackSVG::ElemClass::IMAGE_PANEL);
		imagePanelGroup->addClass("IMAGE_PANEL"); // remove!
	}

	// Experimental cmd location.
	consumeAlignRequest(imagePanelGroup);

	// Reserve the first slot for an image, to be completed later.
	imagePanelGroup[svg::IMAGE];


	return imagePanelGroup;

}

drain::image::TreeSVG & GraphicsContext::getImagePanelGroup(){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::TreeSVG & adapterGroup = getCurrentAdapterGroup();

	if (!currentImagePanel.empty()){
		mout.revised<LOG_NOTICE>("completing: ", currentImagePanel);
		TreeSVG & imagePanelGroup = adapterGroup[currentImagePanel];
		if (consumeAlignRequest(imagePanelGroup)){
			mout.revised<LOG_NOTICE>("handled alignRequest: ", currentImagePanel);
		}
		return imagePanelGroup;
	}
	else {
		TreeSVG & imagePanelGroup = adapterGroup["pending"];
		imagePanelGroup->setId("floating", drain::image::NodeSVG::getNewIndex());
		imagePanelGroup->addClass(FloaterSVG::FLOATING);
		imagePanelGroup->addClass(LayoutSVG::FIXED);
		imagePanelGroup->addClass(LayoutSVG::INDEPENDENT);
		imagePanelGroup->addClass(LayoutSVG::NEUTRAL);
		imagePanelGroup->setFrame(100,100);
		// Experimental cmd location.
		if (consumeAlignRequest(imagePanelGroup)){
			mout.revised<LOG_NOTICE>("handled PENDING alignRequest");
		}
		return imagePanelGroup;
	}


	// TreeSVG & imagePanelGroup = adapterGroup[ctx.currentImagePanel];
	// return imagePanelGroup;

}

drain::image::TreeSVG & GraphicsContext::getImagePanelGroup(const drain::Frame2D<int> & frame){
	TreeSVG & imagePanelGroup = getImagePanelGroup();
	if (!frame.empty()){
		imagePanelGroup->setFrame(frame);
	}

	// imagePanelGroup->addClass(drain::image::LayoutSVG::COMPOUND);
	return imagePanelGroup;
}




} // rack::

