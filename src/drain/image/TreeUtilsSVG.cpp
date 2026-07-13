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


#include <drain/StringBuilder.h>
#include <drain/util/Output.h> // Debugging

#include "TreeUtilsSVG.h"

namespace drain {

namespace image {


DRAIN_ENUM_DICT(TreeUtilsSVG::Roles) = {
		DRAIN_ENUM_ENTRY(drain::image::TreeUtilsSVG::Roles, MAIN),
};



/*
void surround(TreeSVG & group, const TreeSVG::path_elem_t & childKey){

	static const TreeSVG::path_elem_t cageElem("cage");
	static const TreeSVG::path_elem_t dummyElem("dummy");


	TreeSVG & cage  = group[cageElem](svg::GROUP);
	cage->addClass("cage");
	TreeSVG & dummy = cage[dummyElem](svg::GROUP);
	dummy->addClass("dungeon");
	// dummy->setText("Hello!");
	dummy.swap(group[childKey]);

}
*/




int RelativePathSetterSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) {

	TreeSVG & node = tree(path);

	if (node->typeIs(svg::SVG) || node->typeIs(svg::GROUP)){
		return 0; // continue (traverse also children)
	}
	else if (node->typeIs(svg::IMAGE)){
		drain::image::TreeSVG & imageNode = tree(path);
		const std::string & imagePath = imageNode->getUrl(); //imageNode->get("xlink:href");

		if (drain::StringTools::startsWith(imagePath, dir)){
			// Strip directory part from the imagePath, replace with prefix
			drain::Logger mout(__FILE__, __FUNCTION__);
			std::string p = prefix + imagePath.substr(dir.size());
			mout.info("Modifying: ", imagePath, " -> ", p);
			imageNode->setUrl(p);
		}
		else {
			// mout.attention("could not set relative path: ", p, " href:", imagePath);
		}
	}
	return 1; // skip this subtree
}


const std::string MetaDataCollectorSVG::LOCAL("LOCAL");

const std::string MetaDataCollectorSVG::SHARED("SHARED");

/// Iterate children and their attributes: check which attributes (key and value) are shared by all the children.
/**
 *  Collects meta data written upon creating image panels.
 *
 *  General - move to TreeUtilsSVG
 */
int MetaDataCollectorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & current = tree(path);

	if (!current->typeIs(svg::GROUP, svg::SVG)){ // At least METADATA must be skipped...
		if (current->isUndefined()){
			//mout.suspicious(current.data);
			mout.suspicious("Undefined element at '", path, "' == setting id");
			current->setId(path);
		}
		return 0;
	}


	/// Statistics: computer count for each (key,value) pair, for example (what:time, "20240115")
	// typedef std::map<std::string, unsigned short> variableStat_t;

	// Statistics of "<key>=<value>" entries.
	variableStat_t attributeValueCounts;

	/// Number of children having (any) metadata.
	int childCount = 0;

	/// Iterate children and their attributes: mark down all the (key,value) pairs.
	for (const auto & entry: current.getChildren()){
		const TreeSVG & child = entry.second;
		if (child.hasChild(svg::METADATA) && !child->hasClass("legend")){ // or has "data"? Is this for Palette? Generalize somehow...
			++childCount;
			for (const auto & attr: child[svg::METADATA]->getAttributes()){
				std::string s = drain::StringBuilder<>(attr.first,'=',attr.second);
				++attributeValueCounts[s];
			}
		}
	}

	if (childCount == 0){
		return 0;
	}

	// There are metadata, so investigate...


	TreeSVG & debugSharedBase = current[svg::DESC](svg::DESC);
	// debugSharedBase->addClass("SUPER_SHARED"); // just a marker.
	// TreeSVG & debugShared = debugSharedBase["cmt"](svg::COMMENT);

	if (mout.isLevel(LOG_DEBUG)){
		TreeSVG & debugAll = current["description"](svg::DESC);
		debugAll->set("COUNT", childCount);
		debugAll->setText("All"); //CTXX
	}
	/*
		TreeSVG & debugExplicit = current["rejected"](svg::DESC);
		debugExplicit->addClass("EXPLICIT");
		debugAll->ctext += drain::sprinter(stat).str();
	 */

	// Better order this way - METADATA elems (LOCAL and SHARED) will be together.
	TreeSVG & metadata = current[svg::METADATA](svg::METADATA);


	mout.pending<LOG_DEBUG>("pruning: ", drain::sprinter(attributeValueCounts), path.str());

	// If this group level has variable entries ABC=123, DEF=456, ... , "lift" them from the lower level, ie prune them there
	for (const auto & e: attributeValueCounts){

		mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);
		// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;

		if (e.second == childCount){ // This attribute value is shared by all the children.

			mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

			debugSharedBase.addChild() = e.first;

			// Split the entry back to (key,value) pair and update "upwards", ie. from children to this level.
			std::string key, value;
			drain::StringTools::split2(e.first, key, value, '=');
			metadata->set(key, value); // NOTE: forced strings (later, consider type dict?)

			// Actual pruning, "downwards".

			for (auto & entry: current.getChildren()){

				TreeSVG & child = entry.second;

				if (child.hasChild(svg::METADATA)){

					TreeSVG & childMetadata = child[svg::METADATA]; //(svg::METADATA);
					//
					childMetadata->addClass("LOCAL");
					childMetadata->removeAttribute(key);

					//TreeSVG & childMetadata2 = child[PanelConfSVG::ElemClass::SHARED_METADATA](svg::METADATA);
					TreeSVG & childMetadata2 = child[MetaDataCollectorSVG::SHARED](svg::METADATA);
					childMetadata2->addClass(MetaDataCollectorSVG::SHARED);
					childMetadata2->set(key, value);

				}
			}
			// }

		}
		else {
			mout.reject<LOG_DEBUG>('\t', e.first, ' ', path.str());
			// debugExplicit->ctext += e.first;
		}
	}

	/*
	if (current.hasChild("ADAPTER")){

		TreeSVG & adapterMetadata = current["ADAPTER"][svg::METADATA];

		for (const auto & entry: metadata->getAttributes()){
			adapterMetadata->set(entry.first, entry.second);
		}
		// adapterMetadata->set("time", "123456");
		// mout.warn(adapterMetadata->getAttributes());
	}
	*/


	return 0;

}

const std::string FloaterSVG::FLOATING("FLOATING");

int FloaterSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.experimental("check: ", path);
	// std::list<TreeSVG::key_t> floating;
	TreeSVG & parent = tree(path);
	// TreeSVG swapper;

	TreeSVG *endMarker = nullptr;
	for (auto & t: parent){

		if (&t.second == endMarker){
			// All FLOATING elems now lifted.
			return 1;
		}

		if (t.second->hasClass(FLOATING)){
			//floating.push_back(t.first);
			t.second->removeClass(FLOATING); // prevent infinite loop...
			t.second->addClass("FLOATED");
			mout.experimental("Lifting floating element at: ", path, " -> ", t.first);
			//TreeSVG & dummy = swapper.addChild(t.first)(svg::COMMENT);
			if (endMarker == nullptr){
				endMarker = & parent.addChild();
				(*endMarker)->setComment("floats lifted");
			}
			TreeSVG & dummy = parent.addChild();
			dummy->setType(t.second->getNativeType());
			// dummy->setAttribute("data-SWAP", t.first);
			t.second.swap(dummy);
			t.second->swapSVG(dummy); // Attributes, Classes, CSS,..
			/*
			 * dummy->set(t.second->getAttributes());
			dummy->setDefaultAlignAnchor<AlignBase::Axis::HORZ>(t.second->getDefaultAlignAnchor<AlignBase::Axis::HORZ>());
			dummy->setDefaultAlignAnchor<AlignBase::Axis::VERT>(t.second->getDefaultAlignAnchor<AlignBase::Axis::VERT>());
			dummy->setMyAlignAnchor<AlignBase::Axis::HORZ>(t.second->getMyAlignAnchor<AlignBase::Axis::HORZ>());
			dummy->setMyAlignAnchor<AlignBase::Axis::VERT>(t.second->getMyAlignAnchor<AlignBase::Axis::VERT>());
			dummy->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
			*/
			//dummy->
			// TODO: implement addClasses()
			//dummy->addClass(t.second->getClasses());
			dummy->addClass(t.second->getClasses());
			/*
			for (auto & cls: t.second->getClasses()){
				dummy->addClass(cls);
			}
			*/
			// t.second->removeClass(FLOATING); // prevent infinite loop...
			t.second->setType(svg::COMMENT);
			t.second->setComment("Swiped away");
			mout.experimental("Swapper: -> ", t.first);
		}
	}

	return 0;

}

const std::string OverlayMoverSVG::OVERLAY("OVERLAY");

int OverlayMoverSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){
	// if not GROUP
	return 0;
}

//
int OverlayMoverSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.experimental("check: ", path);
	TreeSVG & parent = tree(path);

	//return 0;

	if (parent->typeIs(svg::GROUP)){

		for (auto & src: parent){ // .getChildren()){

			if (src.second->typeIs(svg::GROUP) && src.second->hasClass(OVERLAY)){

				if (src.second.empty()){
					mout.suspicious("Empty OVERLAY elem at child=", src.first,  " of GROUP ", path);
					continue;
				}

				// Reiterate: search for target position
				for (auto & src2: parent){
					// could also check direct descendants:
					// if (src2.first == OVERLAY){ ...
					if (src2.second->typeIs(svg::GROUP)){

						if (src2.second.hasChild(OVERLAY)){

							TreeSVG & dst = src2.second[OVERLAY];
							dst.swap(src.second);
							dst->swapSVG(src.second);

							src.second->removeClass(OVERLAY);
							src.second.addChild()->setComment("Removed overlay");
							/*
							std::cout << "DUMPING: " << src.first << '\n';
							drain::TreeUtils::dump(src.second, std::cout);
							std::cout << "DUMPING: " << dstChild.first << '\n';
							drain::TreeUtils::dump(dstChild.second, std::cout);
							 */
							return 0;
						}
					}

				}

			}

		}

	}

	return 1;

}


const std::string ClipperSVG::CLIPPED("CLIPPED");


TreeSVG & ClipperSVG::getClippingRect(TreeSVG & root, size_t width, size_t height){

	TreeSVG & defs = UtilsXML::getHeaderObject(root, svg::DEFS);

	StringBuilder<'_'> id(svg::CLIP_PATH, width, height);

	TreeSVG & clip = defs[id.str()];

	//if (!defs.hasChild(id.str())){
	if (clip->isUndefined()){
		clip->setType(svg::CLIP_PATH);
		clip->setId(id);
		TreeSVG & rect = clip[svg::RECT](svg::RECT);
		rect->setWidth(width);
		rect->setHeight(height);
	}

	return clip;
}

int ClipperSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.experimental("check: ", path);

	TreeSVG & t = tree(path);
	if (t->hasClass(CLIPPED)){

		mout.experimental<LOG_INFO>("clipping elements under: ", path);
		const svg::coord_t w = t->getWidth();
		const svg::coord_t h = t->getHeight();
		if ((w==0) || (h==0)){
			mout.reject<LOG_WARNING>("Clipping skipped for ", w, 'x', h, " element at ", path);
			return 1;
		}
		// TODO: try relying on tree == root  (and forget separate root)
		TreeSVG & clip = getClippingRect(this->root, t->getWidth(), t->getHeight());
		t->set("clip-path", drain::StringBuilder<>("url(#", clip->getId(), ")").str());
	}

	return 0;
}



const std::string MaskerSVG::MASK_ID = "data-mask";

const ClassXML MaskerSVG::COVER("COVER");


const drain::FlexibleVariable & MaskerSVG::createMaskId(TreeSVG & group){
	const drain::StringBuilder<'_'> maskId(svg::MASK, group->getId());
	group->set(MASK_ID, maskId.str());
	return group->get(MASK_ID);
}

TreeSVG & MaskerSVG::getMask(TreeSVG & root, const std::string & maskId){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::TreeSVG & defs = drain::UtilsXML::getHeaderObject(root, svg::DEFS);
	//drain::image::TreeSVG & mask = defs[group->get(MASK_ID, "default")](svg::MASK);
	drain::image::TreeSVG & mask = defs[maskId]; //(svg::MASK);

	drain::UtilsXML::ensureStyle(root, COVER, { // drain::ClassXML(svg::MASK)
			{"fill",   "gray"},
			{"opacity", 0.5},
	}
	);

	// if (!mask.hasChild(svg::RECT)){
	// Ensure background.
	if (mask->isUndefined()){
		mask->setType(svg::MASK);
		mask->setId(maskId);
		// maskUnits="userSpaceOnUse" maskContentUnits="userSpaceOnUse"
		mask->set("maskUnits", "userSpaceOnUse");
		mask->set("maskContentUnits", "userSpaceOnUse");
		drain::image::TreeSVG & comment = mask.addChild()(svg::COMMENT);
		comment->setText("This base RECT is also the reference for (width, height)");
		drain::image::TreeSVG & rect = mask[svg::RECT](svg::RECT);
		rect->setWidth(128);
		rect->setHeight(128);
		rect->set("fill", "white");
	}


	return mask;
}


drain::image::TreeSVG & MaskerSVG::updateMask(drain::image::TreeSVG & mask, int width, int height, const NodeSVG & node){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// drain::image::TreeSVG & rect = mask[svg::RECT](svg::RECT);
	mask[svg::RECT]->setFrame(width, height);
	// rect->setWidth(width);
	// rect->setHeight(height);

	// Punch hole
	drain::image::TreeSVG & hole = mask.addChild();
	if (node.typeIsSet()){
		hole.data = node;
		hole->clearClasses();
		hole->set("fill", "black"); // Later, consider other style(s), like gradient fill?
	}
	else {
		hole->setType(svg::COMMENT);
		hole->setText("unimplemented mask? ", mask->getId(), "/", node.getId());
	}
	return hole;
}

TreeSVG & MaskerSVG::createMask(TreeSVG & root, TreeSVG & group, int width, int height, const NodeSVG & node){

	const drain::FlexibleVariable & maskId = createMaskId(group);

	std::string s;
	if (group.hasChild(svg::DESC)){
		s = group[svg::DESC].data.getText();
	}
	else {
		s = group->getId();
	}

	drain::image::TreeSVG & mask = getMask(root, maskId);
	if ((width != 0) && (height != 0)){
		TreeSVG & hole = updateMask(mask, width, height, node);
		hole[svg::DESC](svg::DESC)->setText(s);
	}

	// c = mask.prependChild(s)(svg::COMMENT);
	// comment->setText("reference: ", s);

	return mask;
}

void MaskerSVG::addCoverRect(const TreeSVG & mask, TreeSVG & group) {
	drain::image::TreeSVG & cover = group.addChild(drain::image::svg::RECT)(drain::image::svg::RECT);
	cover->set("mask", drain::StringBuilder<>("url(#", mask->getId(), ")").str());
	cover->setFrame(mask[svg::RECT]->getBoundingBox().getFrame());
	cover->addClass(COVER);
}

/*
void MaskerSVG::linkMask(const TreeSVG & mask, TreeSVG & obj) {
	obj->set("mask", drain::StringBuilder<>("url(#", mask->getId(), ")").str());
	obj->setFrame(mask[svg::RECT]->getBoundingBox().getFrame());
	obj->addClass(COVER);
}
*/


int MaskerSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	TreeSVG & group = tree(path);

	if (group->hasAttribute(MASK_ID)){
		const drain::image::TreeSVG & mask = getMask(tree, group->get(MASK_ID));

		if (group->typeIs(drain::image::svg::GROUP)){
			//drain::image::TreeSVG & rect = group.prependChild(drain::Enum<drain::image::svg::tag_t>::dict.getKey(drain::image::svg::RECT))(drain::image::svg::RECT);
			//drain::image::TreeSVG & rect = group.prependChild(drain::image::svg::RECT)(drain::image::svg::RECT);

			addCoverRect(mask, group);
			//drain::image::TreeSVG & rect = group.addChild(drain::image::svg::RECT)(drain::image::svg::RECT);
			//linkMask(mask, rect);
		}
		else {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.fail("Adding MASK ", mask->getId(), " to element of type '", group->getNativeType(), "' not supported");
		}
	}

	return 0;
};

/**
 *
 */
/*

int  AttributeCheckerXML::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){

	TreeSVG & current = tree(path);

	TreeSVG::node_data_t::map_t & attributes = current->getAttributes();
	for (const std::string & key: attributes.getKeyList()){
		std::string keyChecked(key);
		bool CHANGED = false;
		for (char &c: keyChecked){
			if ( ((c>='a') && (c<='z')) || ((c>='A') && (c<='Z')) || ((c>='0') && (c<='9')) || (c=='_') || (c=='-')){
				// ok
			}
			else {
				CHANGED = true;
				c = '_';
			}
		}
		if (CHANGED){
			attributes[keyChecked] = attributes[key];
			attributes.erase(key);
		}
	};

	return 0;

}
*/


}  // image::



}  // drain::
