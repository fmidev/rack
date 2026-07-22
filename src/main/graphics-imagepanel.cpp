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
#include <drain/image/MouseXML.h>
#include "data/SourceODIM.h"
// #include "resources-svg.h"
#include "graphics-imagepanel.h"
#include "graphics-radar.h"


using namespace drain;
using namespace drain::image;

namespace rack {





ImagePanel::ImagePanel(TreeSVG & group, const drain::image::Image & src, const drain::FilePath & filePath) : imagePanelGroup(group){

	if (imagePanelGroup->isUndefined()){
		imagePanelGroup->setType(svg::GROUP);
	}
	else if (!imagePanelGroup->typeIs(svg::GROUP)){
		// ToStream
		drain::Logger(__FILE__, __FUNCTION__).error("TreeSVG imagePanelGroup not of type GROUP");
	}

	imagePanelGroup->addClass(RackSVG::ElemClass::IMAGE_PANEL);

	TreeSVG & image = getImage(src, filePath);
	imagePanelGroup.addChild()->setComment("Added image:", image->getId());

};

// const drain::Frame2D<drain::image::svg::coord_t>

ImagePanel::ImagePanel(TreeSVG & group, const drain::Frame2D<int> & geom) : imagePanelGroup(group){

	if (imagePanelGroup->isUndefined()){
		imagePanelGroup->setType(svg::GROUP);
	}
	else if (!imagePanelGroup->typeIs(svg::GROUP)){
		// ToStream
		drain::Logger(__FILE__, __FUNCTION__).error("TreeSVG imagePanelGroup not of type GROUP");
	}

	imagePanelGroup->addClass(RackSVG::ElemClass::IMAGE_PANEL);
	TreeSVG & image = getImage();
	// imagePanelGroup[svg::IMAGE];
	imagePanelGroup.addChild()->setComment("Added image:", image->getId());

	if (!geom.empty()){
		imagePanelGroup->setFrame(geom);
		getOverlayGroup()->setFrame(geom);
	}

};


ImagePanel::ImagePanel(TreeSVG & imagePanelGroup, const drain::FilePath & filePath, const drain::Frame2D<drain::image::svg::coord_t> & geom) : imagePanelGroup(imagePanelGroup){

	if (imagePanelGroup->isUndefined()){
		imagePanelGroup->setType(svg::GROUP);
	}
	else if (!imagePanelGroup->typeIs(svg::GROUP)){
		// ToStream
		drain::Logger(__FILE__, __FUNCTION__).error("TreeSVG imagePanelGroup not of type GROUP");
	}

	// if (!(filePath.empty() && geom.empty())){
	getImage(filePath, geom);
	// }
};



TreeSVG& ImagePanel::getOverlayGroup() const {
	drain::image::TreeSVG & overlayGroup = getUniqueElem(imagePanelGroup, RackSVG::ElemClass::OVERLAY, svg::GROUP);
	// overlayGroup->addClass(Graphic::GRID);
	if (overlayGroup->getId().empty()){
		imagePanelGroup->setId();
		overlayGroup->setId("overlay_", imagePanelGroup->getId());
	}
	return overlayGroup;
	//return getUniqueElem(imagePanelGroup, RackSVG::ElemClass::OVERLAY, svg::GROUP);
}


TreeSVG & ImagePanel::getImage(const drain::FilePath & filepath, const drain::Frame2D<drain::image::svg::coord_t> & geom) const {


	TreeSVG & image = imagePanelGroup[svg::IMAGE];

	// imagePanelGroup.addChild()->setComment("NEW request ");

	if (image->isUndefined() ){

		// imagePanelGroup.addChild()->setComment("NEW ", drain::TypeName<ImagePanel>::str());
		// imagePanelGroup.addChild()->setComment("NEW ", __FUNCTION__);
		imagePanelGroup->setId(filepath.tail);

		// TreeSVG & overlayGroup = getOverlayGroup();

		//if (!(filepath.extension.empty() && geom.empty())){
		if (! filepath.extension.empty()){
			imagePanelGroup->setDefaultAlignAnchor(svg::IMAGE);
			imagePanelGroup.addChild()->setComment("Image added by ", __FUNCTION__);

			image->setType(svg::IMAGE);
			// image->setId(filepath.tail);
			// image->setId(ctx.currentImagePanel);
			image->setUrl(filepath);
			// image->setFrame(geom);
			//image.addChild()->setComment("NEW ", __FUNCTION__);
			image[svg::TITLE](svg::TITLE) = filepath.tail;

		}
		else {
			// Smells like vector graphics
			// overlayGroup->addClass(Graphic::GRID);
			image->setType(svg::DESC);
			image->setText("Super vector group by ", __FUNCTION__, " - image removed here ");
			imagePanelGroup.addChild()->setComment("Vector B image added by ", __FUNCTION__);
		}


		if (!geom.empty()){
			image->setFrame(geom);
			getBackGround()->setFrame(geom);
			getImageBorder()->setFrame(geom);
		}
	}

	return image;
};

TreeSVG & ImagePanel::getImage(const drain::image::Image & src, const drain::FilePath & filepath) const { // what about prefix?


	drain::Logger mout(__FILE__, __FUNCTION__);


	TreeSVG & imageElem = getImage(filepath, src.getGeometry().getAreaGeometry());

	//drain::image::TreeSVG & mouseGroup = getMouseListenerFrame(); // MOUSE properties really needed?
	drain::image::TreeSVG & geoElem = getImageBorder();

	// practical...
	if (src.properties.hasKey("where:EPSG")){
		geoElem->addClass("GEOREF");
		geoElem->set("data-epsg", src.properties["where:EPSG"]);
	}

	if (src.properties.hasKey("where:BBOX_native")){
		geoElem->addClass("GEOREF");
		geoElem->set("data-bbox", src.properties["where:BBOX_native"]);
	}

	// Metadata:
	TreeSVG & metadata = getMetadata(); // imagePanelGroup[svg::METADATA](svg::METADATA);

	// Note assign: char * -> string  , "where:lat", "where:lon"
	if (src.properties.hasKey("what:source")){
		SourceODIM odimSrc(src.properties.get("what:source",""));
		metadata->set("NOD", odimSrc.NOD);
		metadata->set("PLC", odimSrc.PLC);
		mout.debug(DRAIN_LOG(odimSrc));
	}

	// TODO: 1) time formatting 2) priority (startdate, starttime)
	for (const std::string key: {
		"what:date", "what:time", "what:product", "what:prodpar", "what:quantity",
		"where:elangle", "where:lon", "where:lat", "where:EPSG", // "where:projdef",
		"how:camethod",
		"prevCmdKey"}){ // consider other than prevCmd (product or so)

		if (src.properties.hasKey(key)){
			size_t i = key.find(':');
			if (i == std::string::npos){
				metadata->set(key, src.properties[key]);
			}
			else {
				metadata->set(key.substr(i+1), src.properties[key]);
			}
		}

	}


	TreeSVG & description = imageElem[svg::DESC](svg::DESC);
	// mout.attention(drain::sprinter(metadata->getAttributes()));
	description->set(metadata->getAttributes());
	//mout.attention(drain::sprinter(description->getAttributes()));
	//description->set("MIKA", "MÄKI");

	return imageElem;

}


TreeSVG & ImagePanel::getUniqueElem(TreeSVG & parent, RackSVG::ElemClass cls, svg::tag_t type) const {

	if (!parent.hasChild(cls)){
		// parent.addChild()->setComment("NEW ", __FUNCTION__, ':', cls, '(', type, ')');
	}

	TreeSVG & elem = parent[cls];

	if (elem->isUndefined()){
		elem->setType(type);
		elem->addClass(cls);
		//elem.addChild()->setComment("NEW ", __FUNCTION__, '/', cls);
		//if ((type == svg::RECT) && (parent.hasChild(svg::IMAGE))){
		if (parent.hasChild(svg::IMAGE)){
			if (elem->typeIs(svg::RECT,svg::GROUP)){
				// drain::SelectXML<svg::tag_t>(elem->getTag(), cls).str()
				drain::Logger(__FILE__, __FUNCTION__).suspicious("Not RECT: ", ':', elem->getTag(), '.', cls, '(', type, ')');
			}
			elem->setMyAlignAnchor(svg::IMAGE);
			elem->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
			// "Alternatively":
			//elem->setFrame(imagePanelGroup[svg::IMAGE]->getBoundingBox().getFrame());
			elem->setFrame(parent[svg::IMAGE]->getBoundingBox().getFrame());
		}
	}

	return elem;

}

TreeSVG & ImagePanel::getUniqueElem(TreeSVG & parent, svg::tag_t type) const {

	if (!parent.hasChild(type)){
		// parent.addChild()->setComment("NEW2 ", __FUNCTION__, '(', type, ')');
	}

	TreeSVG & elem = parent[type];

	if (elem->isUndefined()){
		elem->setType(type);

		// if ((type == svg::RECT) && (parent.hasChild(svg::IMAGE))){
		if (parent.hasChild(svg::IMAGE)){
			if (elem->typeIs(svg::GROUP, svg::RECT)){ // , svg::IMAGE
				elem->setMyAlignAnchor(svg::IMAGE);
				elem->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
				// "Alternatively":
				elem->setFrame(imagePanelGroup[svg::IMAGE]->getBoundingBox().getFrame());
			}
			else {
				drain::Logger(__FILE__, __FUNCTION__).reject("Not RECT: ", elem->getTag());
			}
		}
	}

	return elem;

}

drain::image::TreeSVG& ImagePanel::getMouseListenerFrame() const {

	imagePanelGroup->addClass(RackSVG::ElemClass::MOUSE);

	TreeSVG & mouseListenerFrame = getImageBorder(); // same!
	mouseListenerFrame->addClass(MouseXML::ElemClass::MOUSE_LISTENER);
	mouseListenerFrame->setStyle("fill", "lightblue");       //
	mouseListenerFrame->setStyle("fill-opacity", 0.1);
	// mouseListenerLayer.addChild()->setComment("Mouse interaction");

	return mouseListenerFrame;
}



drain::image::TreeSVG& ImagePanel::getDataImage(const drain::FilePath & filepath,
		const drain::Frame2D<drain::image::svg::coord_t> & geom) const {

	TreeSVG & overlay = getOverlayGroup();
	drain::image::TreeSVG & dataImage = getUniqueElem(overlay, RackSVG::ElemClass::DATA_ARRAY, svg::IMAGE); // imagePanelGroup[RackSVG::ElemClass::DATA_ARRAY];

	dataImage->addClass(LayoutSVG::FIXED);
	dataImage->addClass(LayoutSVG::NEUTRAL);
	dataImage->addClass(LayoutSVG::INDEPENDENT);
	dataImage->addClass("MOUSE_VALUE_DATA");
	dataImage->setLocation(20,10);
	// dataImage->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	dataImage.addChild()->setComment(__FUNCTION__, " - RGB encoded data not for viewing as an image");

	drain::image::TreeSVG & desc = dataImage[svg::DESC](svg::DESC);
	desc->setText("Data channels: 0[red]: 8 higher bits, 1[green]: 8 lower bits, 2(blue) unused");

	if (!filepath.empty()){
		if ((!dataImage->getUrl().empty()) && (dataImage->getUrl() != filepath.str())){
			drain::Logger(__FILE__, __FUNCTION__).warn("changing url ", dataImage->getUrl(), " to ", filepath);
		}
		dataImage->setUrl(filepath);
	}

	if (!geom.empty()){
		dataImage->setFrame(geom);
	}
	else {
		// Copy
		dataImage->setFrame(getImage()->getBoundingBox().getFrame());
	}

	return dataImage;

}

drain::image::TreeSVG & ImagePanel::getVectorOverlayGroup(const std::string & key, const drain::Frame2D<drain::image::svg::coord_t> & geom) const {

	drain::image::TreeSVG & overlayGroup = getOverlayGroup();

	if (key.empty()){
		std::string k;
		drain::image::TreeSVG::generateKey(overlayGroup, k);
		if (k.empty()){
			std::runtime_error(__FUNCTION__);
		}
		return getVectorOverlayGroup(k, geom);
	}


	TreeSVG & vectorGroup =  overlayGroup[key](svg::GROUP);
	vectorGroup.addChild()->setCommentSafe(key);
	if (vectorGroup->getId().empty()){
		std::string id;
		drain::StringTools::getSafeKey(key, id, "-",",:");
		vectorGroup->setId(id);
	}
	vectorGroup->addClass(LayoutSVG::NEUTRAL);
	vectorGroup->addClass(LayoutSVG::INDEPENDENT);

	if (!geom.empty()){
		vectorGroup->setFrame(geom);
		vectorGroup->addClass(ClipperSVG::CLIPPED);
	}
	else if (overlayGroup.hasChild(RackSVG::BACKGROUND_RECT)){
		vectorGroup->setFrame(overlayGroup[RackSVG::BACKGROUND_RECT]->getBoundingBox().getFrame());
	}
	else if (imagePanelGroup.hasChild(svg::IMAGE)){
		vectorGroup->setFrame(imagePanelGroup[svg::IMAGE]->getBoundingBox().getFrame());
	}
	else {
		vectorGroup->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
		vectorGroup.addChild("No explicit geometry available, relying on Alignment (Anchor)");
	}

	if (!vectorGroup->getBoundingBox().getFrame().empty()){
		overlayGroup->addClass(ClipperSVG::CLIPPED);
	}

	//vectorGroup->addClass(ClipperSVG::CLIPPED);

	return vectorGroup;
}

drain::image::TreeSVG & ImagePanel::getSourceSpecificGroup(const std::string & source) const {

	if (!source.empty()){
		drain::image::TreeSVG & subGroup = imagePanelGroup[source](svg::GROUP);
		subGroup->setAttribute("data-source", source);
		return subGroup;
	}
	else {
		drain::Logger(__FILE__, __FUNCTION__).warn("Argument ('source') empty");
		std::string s;
		drain::image::TreeSVG::generateKey(imagePanelGroup, s);
		// Should not be empty...
		return getSourceSpecificGroup(s);
	}

	// return panel[statusMap.get("what:source", "unknown-source")](svg::GROUP);
}




} // rack::

