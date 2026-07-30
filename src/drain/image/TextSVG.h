/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
 * TreeSVG.h
 *
 *  Created on: Nov, 2024
 *      Author: mpeura
 */

#ifndef DRAIN_TEXT_SVG
#define DRAIN_TEXT_SVG

#include <string>

// #include "drain/Enum.h"
// #include "drain/util/XML.h"
// #include "drain/util/Frame.h"
// #include "Alignment.h"
#include "TreeSVG.h"
// #include "AlignSVG.h"


namespace drain {

namespace image {


class TextBox {

public:

	static
	const drain::ClassXML TEXTBOX;

	//TextBox(ImagePanel &imagePanel, const drain::Frame2D<int> & geom = {0,0});
	/**
	 *  \param group - host
	 */
	TextBox(TreeSVG & group); // , const drain::Frame2D<int> & geom = {0,0});

	TextBox(TreeSVG & group, const TreeSVG::path_elem_t & identifier); // , const drain::Frame2D<int> & geom = {0,0});

	/**
	 *  This should be rarely needed...
	 */
	TextBox(TextBox &textBox);

	void setLineHeight(const svg::coord_t w);

	void setFontSize(const svg::coord_t w);

	TreeSVG & addLineAligned(const std::string & s, char edge='|') const ;

	/// Adds a TEXT elem, containing concatenated arguments.
	template <char SEP=0, class ...TT>
	TreeSVG & addLine(const TT &... args) const {
		return addLineAligned(drain::StringBuilder<SEP>(args...).str(), '|');
	}

	void addLines(const std::string & lines, char separator='\n', char edge='|') const;


	// Unused
	void setLocation(const drain::Point2D<int> & point);

	TreeSVG & getBackground() const;

	/*
	// Unused
	void setGeometry(const drain::Frame2D<int> & geom);

	// Unused
	void setWidth(const svg::coord_t w);

	// Unused
	void setHeight(const svg::coord_t h);



	*/

	// Top-level element.
	TreeSVG & textGroup;

	// Main group, immediately under textGroup.
	TreeSVG & adapterGroup;
	// AnchorElem sharedHorzAnchor;

	// ??
	// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz; // (AlignSVG::CENTER, MutualAlign::OUTSIDE)
	// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert;

	static
	const std::string FLIP_FUNCTION_JS;


protected:

	// svg::coord_t lineHeight = 0;

	void init();

	/// Adds a TEXT elem, containing concatenated arguments.
	template <class ...TT>
	TreeSVG & addLineOLD(const TT &... args);

	static
	const std::string LINE_HEIGHT;

};



/*
template <char SEP,class ... TT>
void TextBox::addLine(const TT &... args) const {


	drain::Logger mout(__FILE__, __FUNCTION__);

	//TreeSVG & textLine = textGroup.addChild()(svg::TEXT);
	TreeSVG & group = adapterGroup;

	float fontSize =  group->getStyle("font-size", 0.0f);
	if (fontSize == 0.0f){
		fontSize = group->get(LINE_HEIGHT, 20);
		group->setStyle("font-size", fontSize);
	}

	const int lineHeight =  group->get(LINE_HEIGHT, (4*fontSize)/3);


	const std::string line = drain::StringBuilder<SEP>(args...).str();

	if (line.empty()){
		// TODO: consider allowing empty line
		return; //  textLine;
	}


	CompleteHorzAlign alignHorz(AlignSVG::CENTER, MutualAlign::OUTSIDE);  // , MutualAlign::INSIDE);
	CompleteVertAlign alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
	// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz(AlignSVG::CENTER, MutualAlign::OUTSIDE);  // , MutualAlign::INSIDE);
	// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);

	std::list<std::string> parts;
	drain::StringTools::split(line, parts, '|');
	switch (parts.size()) {
	case 1:
		alignHorz.set(AlignSVG::CENTER);
		break;
	case 2:
		alignHorz.set(AlignSVG::LEFT);
		break;
	default:
		alignHorz.set(AlignSVG::LEFT);
		mout.warn("Text contained several alignment markers '|'");
		break;
	}

	for (const std::string & part: parts){ // Maximally two.

		if (!part.empty()){

			TreeSVG & text = group.addChild()(svg::TEXT);
			// text->addClass(TEXTBOX);

			//text->setFontSize(fontSize, (15*fontSize)/10);
			text->setHeight(lineHeight);
			text->setMargin(lineHeight/4);
			text->setText(part);

			//text->setMyAlignAnchor<AlignBase::Axis::HORZ>(RackSVG::ElemClass::BACKGROUND);

			text->setAlign(alignHorz);

			// text->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS); // For the first element, this is "labelAnchor"
			text->setAlign(alignVert);

			// text[svg::TITLE](svg::TITLE)->setText(part);
			// The first VERT align (stored in alignVert) has now been used, so next element is on the same row.
			alignVert.set(AlignSVG::TOP, MutualAlign::INSIDE);
		}

		// If there is another part, align it RIGHT
		alignHorz.set(AlignSVG::RIGHT);
	}

}
*/


} // image::

}  // drain::

/*
template<class ... TT>
TreeSVG & TextBox::addLineOLD(const TT &... args) {


	const bool FIRST = textGroup.empty();
	TreeSVG & textLine = textGroup.addChild()(svg::TEXT);
	textLine->setTextSafe(args...);
	textLine->setFontSize(13,17);

	// HORZ alignment, always:
	// textLine->setMyAlignAnchor<AlignBase::HORZ>(RackSVG::ElemClass::BACKGROUND);
	textLine->setAlign(alignHorz,     MutualAlign::Topol::INSIDE);

	if (FIRST){
		// textLine->setMyAlignAnchor<AlignBase::VERT>(RackSVG::ElemClass::BACKGROUND);
		textLine->setAlign(AlignSVG::TOP, MutualAlign::Topol::INSIDE);
	}
	else {
		// textLine->setMyAlignAnchor<AlignBase::VERT>(AnchorElem::PREVIOUS); // default, remove..
		textLine->setAlign(AlignSVG::BOTTOM, MutualAlign::Topol::OUTSIDE);
	}


	return textLine;
}
*/

/*
	if (sharedHorzAnchor.isSet()) {
		textLine->setMyAlignAnchor<AlignBase::Axis::HORZ>(sharedHorzAnchor);
	} else if (!FIRST) {
		textLine->setMyAlignAnchor<AlignBase::Axis::HORZ>(AnchorElem::PREVIOUS);
	}
	*/

	/*  SAVE, for further flips...
	 *
	textLine->setAlign(alignHorz);
	// VERT alignment
	if (FIRST) {
		// textLine->setMyAlignAnchor<AlignBase::Axis::VERT>(sharedHorzAnchor);
		textLine->setAlign(alignVert);
	} else {
		textLine->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS);
		AlignBase::Pos pos = alignVert.pos;
		if (alignVert.topol == MutualAlign::Topol::INSIDE) {
			// In this case, invert the direction, because:
			// - if first elem was at the top, continue down.
			// - if first elem was at the bottom, continue up.
			pos = AlignBase::flip(pos);
		}
		textLine->setAlign(AlignBase::Axis::VERT, pos,
				MutualAlign::Topol::OUTSIDE);
	}
	*/


#endif // DRAIN_TEXT_SVG

