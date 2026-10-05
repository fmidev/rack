/*

MIT License

Copyright (c) 2026 FMI Open Development / Markus Peura, first.last@fmi.fi

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
#include "drain/util/Output.h"

#include "DataTools.h"
#include "DataSelector.h"

#include "DataModifier.h"

namespace rack {

void DataModifier::markTree(Hi5Tree &src, bool EXCLUDE, ODIMPathElem::group_t groupFilter){

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	for (auto & entry: src) {

		//if (!entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
		if (entry.first.belongsTo(groupFilter)){
			entry.second.data.exclude = EXCLUDE;
			// Recursion
			markTree(entry.second, EXCLUDE, groupFilter);
		}
	}

}

void DataModifier::markPath(Hi5Tree &src, const Hi5Tree::path_t & path, bool EXCLUDE){
	//drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// Traverse path...
	Hi5Tree *ptr = &src; // Rare!
	for (const Hi5Tree::path_t::elem_t & elem: path){
		ptr->data.exclude = EXCLUDE;
		if (!EXCLUDE){
			// ...  and also mark attached WHAT, WHERE, HOW groups included.
			for (auto & entry: *ptr){
				if (entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
					entry.second.data.exclude = false;
				}
			}
		}
		ptr = & (*ptr)[elem];
	}
	ptr->data.exclude = EXCLUDE;

}



void DataModifier::remove(Hi5Tree &dst, const DataSelector & selector){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// Step 0
	mout.debug2("delete existing structures marked 'exclude'");
	hi5::Hi5Base::deleteExcluded(dst);

	ODIMPathList paths;
	selector.getPaths(dst, paths);

	mout.debug("deleting ", paths.size(), " substructures");
	for (const ODIMPath & path: paths){
		mout.debug("deleting: ", path);
		dst.erase(path);
	}

	// handleEmptyGroups(dst, false);

}


void DataModifier::keep(Hi5Tree &dst, const DataSelector & selector){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug2("delete existing structures marked 'exclude'");
	// There shouldn't be many; consider warning if something was really deleted.
	hi5::Hi5Base::deleteExcluded(dst);

	// mout.attention(DRAIN_LOG(selector));
	markIncluded(dst, selector);

	// DataTools::superDump(dst); // debug

	hi5::Hi5Base::deleteExcluded(dst);
	// std::cerr << "once more!\n";
	// DataTools::superDump(dst);


}

void DataModifier::markIncluded(Hi5Tree &dst, const DataSelector & selector){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// Initially, mark all groups excluded, except WHAT, WHERE and HOW groups.

	markTree(dst, true, ODIMPathElem::DATA_GROUPS| ODIMPathElem::ARRAY);
	// markTree(dst, true, ODIMPathElem::DATA_GROUPS);
	// markTree(dst, true, ODIMPathElem::ALL_GROUPS);

	// DEBUG: DataTools::superDump(dst);
	// mout.special("include: ", DRAIN_LOG(selector));
	ODIMPathList savedPaths;
	selector.getPaths(dst, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

	for (const ODIMPath & path: savedPaths){

		// mout.accept<LOG_NOTICE>("include path and subtree of: ", path);
		// Mark included, along the path only
		markPathIncluded(dst, path);
		// Mark included full subtrees.
		markTree(dst(path), false, ODIMPathElem::ALL_GROUPS);

	}



}


int DataModifier::removeEmptyGroups(Hi5Tree & dst, const ODIMPath & path){ // bool REMOVE,
//int DataModifier::handleEmptyGroups(Hi5Tree & dst, const ODIMPath & path){ // bool REMOVE,


	drain::Logger mout(__FILE__, __FUNCTION__);

	// Collect paths of empty groups.
	ODIMPathList paths;

	// Debugging: check if empty groups remain.
	int count = 0;

	for (auto & entry: dst(path).getChildren()){
		if (entry.first.belongsTo(ODIMPathElem::DATA_GROUPS)){
			++count;
			const ODIMPath p(path, entry.first);
			int c = removeEmptyGroups(dst, p); //handleEmptyGroups(dst, REMOVE, p);
			if (c==0){
				paths.push_back(p);
			}
		}
		else if (entry.first.is(ODIMPathElem::ARRAY)){
			if (entry.second.data.empty()){
				++count;
			}
		}
		else {
			++count;
		}
	}

	/*
	if (count == 0){
		if (!REMOVE){
			mout.info("Empty groups remaining at ", path);
			mout.hint<LOG_INFO>("Add path argument or remove empty groups with additional '--delete empty'");
		}
	}*/

	// if (REMOVE){

		// dst(path).clearChildren() could corrupt iteration at upper stack level.
	for (const ODIMPath & p: paths){
		mout.debug("Removing empty group: ", p);
		dst.erase(p);
		// mout.attention("Removing empty group: DONE");
	}
	//}

	return count;

}



}  // rack::
