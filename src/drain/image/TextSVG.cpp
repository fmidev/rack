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
 
/*
 * TextSVG.cpp
 *
 *      Author: mpeura
 */

#include "LayoutSVG.h"
#include "TextSVG.h"

namespace drain {

namespace image {

const drain::ClassXML TextBox::TEXTBOX("TEXTBOX");

const std::string TextBox::LINE_HEIGHT("data-line-height");

const std::string TextBox::FLIP_FUNCTION_JS =
		"function flipTextBox(elem, horzFlip, vertFlip){\n"
		"  const adapterGroups = elem.getElementsByClassName('ADAPTER');\n"
//		"  const adapterGroups = elem.getElementsByTagName('g');\n"
		"  if (adapterGroups.length == 0){ return }\n"
		" \n"
		"  const adapterGroup = adapterGroups[0];\n"
		" \n"
		" var textAnchor = 'middle';\n"
		"  if (horzFlip===null) horzFlip=0;\n"
		"  if (horzFlip < 0){\n"
		"     textAnchor='end';}\n"
		"  else if (horzFlip > 0){\n"
		"     textAnchor='start';}\n"
		" \n"
		"  if (vertFlip===null) vertFlip=0;\n"
		"  var dy = 50; // elem.getAttribute('')\n"
		"  if (vertFlip < 0){\n"
		"  		dy = -dy;}\n"
		"  else if (vertFlip > 0){\n"
		"  		dy = 0;}\n"
		"  else {\n"
		"  		dy = -dy/2;}\n"
		"  adapterGroup.style.textAnchor = textAnchor;\n"
		"  adapterGroup.setAttribute('transform', 'translate(0,'+dy+')');\n"
		" \n"
		"}\n"
		;

//TextBox::TextBox(ImagePanel &imagePanel, const drain::Frame2D<int> & geom) : imagePanel(imagePanel), textGroup(imagePanel.getOverlayGroup().addChild(svg::GROUP)) {
TextBox::TextBox(TreeSVG & group, const drain::Frame2D<int> & geom) :
		textGroup(group.addChild(svg::GROUP)),
		adapterGroup(textGroup.addChild(svg::GROUP)) {
	init();
}

// imagePanel(textBox.imagePanel),
// TODO: if still needed, fix class to contain host group
TextBox::TextBox(TextBox &textBox) : textGroup(textBox.textGroup), adapterGroup(textBox.adapterGroup) {
	init();
	drain::Logger(__FILE__, __FUNCTION__).suspicious<LOG_ERR>("");
}


void TextBox::init(){

	textGroup->setId(TEXTBOX, NodeSVG::getNewIndex());
	textGroup->addClass(TEXTBOX);
	adapterGroup->addClass(LayoutSVG::ADAPTER);
	adapterGroup->setStyle(StyleXML::TEXT_ANCHOR, "middle");

	// Background is the anchor and also the reference for
	/*
	TreeSVG & backGround = getBackground();
	backGround->addClass(RackSVG::ElemClass::BACKGROUND, TEXTBOX);
	backGround->setGeometry(10,20);
	*/

	// The first VERT anchor is effectively BACKGROUND rectangle (set explicitly by addLine), and after that, the previous TEXT line:
	// Row by row:
	// textGroup->setDefaultAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS);
	// alignVert.set(AlignSVG::BOTTOM, MutualAlign::Topol::OUTSIDE);

	// The HORZ anchor is always the (edge or the centre) of BACKGROUND rectangle.
	//textGroup->setDefaultAlignAnchor<AlignBase::Axis::HORZ>(RackSVG::ElemClass::BACKGROUND);
	// alignHorz.set(AlignSVG::LEFT, MutualAlign::Topol::INSIDE);
	// textGroup->addClass(AlignSVG::NEUTRAL);     // compound bbox not affected (well... vertically it should?)
	// textGroup->addClass(AlignSVG::INDEPENDENT); // no anchoring here please

}

void TextBox::setLineHeight(const svg::coord_t height){
	textGroup->set(LINE_HEIGHT, height);
	Variable & fontSize = textGroup->getStyle("font-size");
	if (fontSize == 0){
		fontSize = (3*height)/4;
	}
};

void TextBox::setFontSize(const svg::coord_t height){
	textGroup->setStyle("font-size", height);
	// svg::coord_t lineHeight = textGroup->get(LINE_HEIGHT);
	FlexibleVariable & lineHeight = textGroup->get(LINE_HEIGHT);
	if (lineHeight == 0){
		lineHeight = 4*height/3;
	}
};

/*
void TextBox::setWidth(const svg::coord_t w){
	getBackground()->setWidth(w);
}

void TextBox::setHeight(const svg::coord_t h){
	getBackground()->setHeight(h);
}

void TextBox::setGeometry(const drain::Frame2D<int> & geom){
	textGroup->setGeometry(geom);
	//getBackground()->setGeometry(geom);
}
*/

// Todo: "opposite": setAlign(T... )
void TextBox::setLocation(const drain::Point2D<int> & point){
	// textGroup->setLocation(point);        // Future design
	// getBackground()->setLocation(point);  // Deprecating design
	textGroup->setAlign(AlignSVG::FIXED);
	textGroup->transform.translate.set(point);
}

TreeSVG & TextBox::addLineAligned(const std::string & line, char edge) const {


	drain::Logger mout(__FILE__, __FUNCTION__);

	//TreeSVG & textLine = textGroup.addChild()(svg::TEXT);
	TreeSVG & group = adapterGroup;

	float fontSize =  group->getStyle("font-size", 0.0f);
	if (fontSize == 0.0f){
		fontSize = group->get(LINE_HEIGHT, 20);
		group->setStyle("font-size", fontSize);
	}

	const int lineHeight =  group->get(LINE_HEIGHT, (4*fontSize)/3);

	/*
	if (line.empty()){
		// TODO: consider allowing empty line
		return; //  textLine;
	}
	*/


	CompleteHorzAlign alignHorz(AlignSVG::CENTER, MutualAlign::OUTSIDE);  // , MutualAlign::INSIDE);
	CompleteVertAlign alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);

	std::list<std::string> parts;
	drain::StringTools::split(line, parts, '|');
	switch (parts.size()) {
	case 0:
		mout.warn("String split failed for line=", line);
		break;
	case 1:
		// alignHorz.set(AlignSVG::CENTER);
		alignHorz.reset();
		break;
	case 2:
		alignHorz.set(AlignSVG::LEFT, MutualAlign::OUTSIDE);
		break;
	default:
		alignHorz.set(AlignSVG::LEFT, MutualAlign::OUTSIDE);
		mout.warn("Text contained several alignment markers '|'");
		break;
	}

	TreeSVG *lastElem = nullptr;

	for (const std::string & part: parts){ // Maximally two.

		if (!part.empty()){

			TreeSVG & text = group.addChild()(svg::TEXT);
			lastElem = &text;
			text->addClass(TEXTBOX);

			//text->setFontSize(fontSize, (15*fontSize)/10);
			text->setHeight(lineHeight);
			text->setMargin(lineHeight/4);
			text->setText(part);

			text->setAlign(alignHorz);
			text->setAlign(alignVert);

			// text[svg::TITLE](svg::TITLE)->setText(part);
			// The first VERT align (stored in alignVert) has now been used, so next element is on the same row.
			alignVert.set(AlignSVG::TOP, MutualAlign::INSIDE);
		}

		// If there is another part, align it RIGHT
		alignHorz.set(AlignSVG::RIGHT, MutualAlign::OUTSIDE);
	}

	if (lastElem == nullptr){
		lastElem = & group.addChild()(svg::TEXT);
		lastElem->addChild()->setComment("Parsing text argument failed");
	}
	return *lastElem;
}


void TextBox::addLines(const std::string & lines, char separator, char edge) const {

	std::list<std::string> finalLines;

	drain::StringTools::split(lines,finalLines, separator);

	for (std::string & line: finalLines){

		if (line.empty()){
			// TODO: allow empty line
			continue;
		}

		addLineAligned(line, edge);
	}

}


TreeSVG & TextBox::getBackground() const {

	std::string BACKGROUND = "BACKGROUND";

	if (adapterGroup.hasChild(BACKGROUND)){
		return adapterGroup[BACKGROUND](svg::RECT);
	}
	else {
		TreeSVG & background = adapterGroup.prependChild(BACKGROUND);
		background->setType(svg::RECT);
		background->addClass(BACKGROUND);
		return background;
	}
	// return textGroup[RackSVG::ElemClass::BACKGROUND](svg::RECT);
}


}  // image::


}  // drain::



