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
#include <set>
#include <ostream>

#include <drain/RegExp.h>

#include <drain/util/Input.h>
#include <drain/image/FilePng.h>
#include <drain/image/FilePnm.h>
#include <drain/image/FileTIFF.h>
#include <drain/prog/Command.h>

#include "andre/QualityCombinerOp.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "data/ODIM.h"
#include "data/ODIMPathTools.h"

#include "fileio.h"
#include "fileio-read.h"


namespace rack {

void CmdInputFile::exec() const {

	RackContext & ctx = getContext<RackContext>();

	ctx.statusFlags.unset(drain::Status::INPUT_ERROR);
	ctx.statusFlags.unset(drain::Status::DATA_ERROR);
	ctx.unsetCurrentImages();

	readFile(value);

}

void CmdInputFile::readFile(const std::string & fileName) const {


	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, getName()); // __FILE__

	if (fileName.empty()){
		mout.error("empty filename");
	}

	mout.note("reading: ", fileName);

	std::string fullFilename;
	ctx.resolveFilePath(ctx.inputPrefix, fileName, fullFilename);

	mout.debug("full path: ", fullFilename);


	//const drain::CommandRegistry & r = drain::getRegistry();
	//mout.warn("lastCommand: '" , drain::CommandRegistry::index , r.getLastCommand() , "'" );
	// Kludge. Check if last command was str than 1) file read or 2) odim assignment ( --/path:key=value )
	// inputComplete = (r.getLastCommand() != this->name) && (r.getLastCommand() != "CmdSetODIM");
	// mout.warn("inputComplete: " , (int)inputComplete );
	// mout.warn("autoExec:      " , (int)cmdAutoExec.exec );

	drain::FilePath path(fileName);
	const bool NO_EXTENSION = path.extension.empty();

	try {

		if (hi5::fileInfo.checkExtension(path.extension) || NO_EXTENSION){
			if (NO_EXTENSION){
				mout.warn(path.dir, '+', path.tail, '+', path.extension);
				path.dir.debug(std::cerr);
				mout.discouraged("No file extension! Assuming HDF5...");
			}
			readFileH5(fullFilename);
		}
		else if (listFileExtension.test(fileName)){
			if (!ctx.inputPrefix.empty()){
				mout.note("Reading .lst file '", fileName, "', omitting prefix: ", ctx.inputPrefix);
			}
			readListFile(fileName);
			// It is better to "save" inputPrefix for inputFiles
			// readListFile(fullFilename);
		}
		else if (drain::image::FileTIFF::fileInfo.checkExtension(path.extension)){
			mout.advice("Writing TIFF files is supported");
			mout.error("Reading TIFF files unsupported");
		}
		else if (drain::image::FilePng::fileInfo.checkExtension(path.extension)){ //(IMAGE_PNG || IMAGE_PNM){
			readImageFile(fullFilename);
		}
		else if (drain::image::FilePnm::fileInfo.checkExtension(path.extension)){ //(IMAGE_PNG || IMAGE_PNM){
			readImageFile(fullFilename);
		}
		else if (textFileExtension.test(fileName)){
			readTextFile(fullFilename);
		}
		else {
			mout.error("Unrecognized/unsupported file type, filename: '", fileName, "'");
		}

	}
	catch (const std::exception & e) {
		//resources.inputOk = false;
		ctx.statusFlags.set(drain::Status::INPUT_ERROR);
		mout.debug(e.what() );
		/*
		if (resources.scriptParser.autoExec > 0){  // => go on with str inputs
			mout.warn("Read error, file: " , this->value );
		}
		else {
			mout.error("Read error, file: " , this->value );
			//exit(1);
		}
		*/
		return;
	}


	if (!ctx.SCRIPT_DEFINED){
		if (!ctx.select.empty()){
			mout.deprecating<LOG_NOTICE>("clearing selector - in future, may be changed");
			ctx.select.clear(); // NEW: "starts a product pipe". monitor effects of this
		}
	}

	//mout.note("resources.getUpdatedStatusMap()" );
	//mout.note("ctx.getStatusMap()" );

	drain::VariableMap & vmap = ctx.getStatusMap();

	vmap["inputFileBasename"] = path.tail;

	path.tail.clear();
	vmap["inputDir"] = path.str();
	//mout.note(path"ctx.getStatusMap() start" );

}



/// Reads hdf5 file and appends it to H5 structure. Works only with sweeps (SCAN), volume (PVOL) or Cartesian data (COMP) (having elevations).
void CmdInputFile::readFileH5(const std::string & fullFilename) const {  // TODO

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	// mout.debug("start: " , fullFilename );

	mout.debug("thread #", ctx.getId(), ": reading ", fullFilename);

	// InputSelect needed?
	Hi5Tree srcTmp;

	if (!ctx.select.empty()){// supply ' ' to clear the inputSelector. (temporary fall-back solution)
		ctx.select = drain::StringTools::trim(ctx.select);
		if (!ctx.select.empty()){
			ctx.inputSelect = ctx.select;
			ctx.select = "";
			mout.experimental("Saved selection (", ctx.inputSelect, ") as input selector.");
		}
		else {
			if (!ctx.inputSelect.empty()){
				mout.experimental("Clearing input selector '", ctx.inputSelect, "'");
			}
			ctx.inputSelect = "";
		}
	}


	if (!ctx.inputSelect.empty()){
		//mout.special() << "Input selector (" << ctx.select << ") applies, pre-reading attributes first:\n";
		mout.info("Input selector (", ctx.inputSelect, ") set -> selective read.");
		mout.debug("First, reading attributes only:\n");
		hi5::Reader::readFile(fullFilename, srcTmp, hi5::Reader::ATTRIBUTES);

		DataTools::updateInternalAttributes(srcTmp); // to support DataSelector with what:quantity and what:elangle

		// Initially, mark all deleted...
		DataTools::markExcluded(srcTmp, true);
		// drain::TreeUtils::dump(srcTmp, std::cout, CmdOutputTree::dataToStream); // true);

		DataSelector selector(ODIMPathElem::DATASET, ODIMPathElem::DATA);  // NO QUALITY?
		selector.setParameters(ctx.inputSelect);
		mout.special<LOG_INFO>("Input selector: ", selector, ", matcher=", selector.getPathMatcher());

		if (selector.getPathMatcher().empty()){
			mout.hint<LOG_NOTICE>("input selector: pathMatcher empty, consider path=data at least?");
		}

		ODIMPathList paths;
		//selector.selectPaths(srcTmp, paths);
		selector.getPaths(srcTmp, paths);

		for (const ODIMPath & path: paths){
			if (srcTmp.hasPath(path)){ // otherwise path query would create one...
				mout.accept("including: ", path); // marking for save...
				DataTools::markExcluded(srcTmp, path, false);
				//srcTmp(path).data.exclude = false;
			}
			else {
				mout.warn("bug: path does not exist: ", path);
			}
		}			//mout.debug("marked for save: " , *it );

		// mout.special("marking 'excluded' completed: ", fullFilename);
		hi5::Reader::readFile(fullFilename, srcTmp, hi5::Reader::EXCLUSIVE | hi5::Reader::ATTRIBUTES | hi5::Reader::DATASETS);
	}
	else {
		hi5::Reader::readFile(fullFilename, srcTmp);
	}


	if (!ctx.inputSelect.empty()){ // or stg like: hi5::Reader::MARKED
		mout.experimental<LOG_DEBUG>("Input selection: removing excluded subtrees");
		hi5::Hi5Base::deleteExcluded(srcTmp);
		// drain::TreeUtils::dump(srcTmp, std::cout, CmdOutputTree::dataToStream); // true
		if (ctx.SCRIPT_DEFINED){
			mout.debug("Script defined -> not clearing input data selector (", ctx.inputSelect, ')');
			//mout.info(ctx.select);
		}
		else {
			mout.info("Clearing input data selector (", ctx.inputSelect, ')');
			ctx.inputSelect.clear();
		}
	}

	if (mout.isDebug(6)){
		mout.debug3("input data: ");
		hi5::Hi5Base::writeText(srcTmp, std::cerr);
	}


	DataTools::updateInternalAttributes(srcTmp); // could be replaced, see below; only timestamp needed at this point?
	// mout.suspicious("data.image.properties:", srcTmp.data.image.properties);

	mout.debug("Derive file type (what:object)");
	drain::Variable & object = srcTmp[ODIMPathElem::WHAT].data.attributes["object"]; // beware of swap later
	if (object.empty()){
		object = "PVOL";
		mout.warn("/what:object empty, assuming polar volume, '", object, "'");
	}

	if ((object == "COMP") || (object == "IMAGE")){

		mout.info("Cartesian [", object, ']');

		//DataTools::updateCoordinatePolicy(srcTmp, RackResources::limit);
		ctx.currentHi5 = & ctx.cartesianHi5;

		// Move or append srcTmp to ctx.cartesianHi5
		if (ctx.appendResults.isRoot() || ctx.cartesianHi5.empty()){
			// Move (replace)
			ctx.cartesianHi5.swap(srcTmp);
			//mout.note(ctx.cartesianHi5 );
			mout.info("Swapped: " , ctx.cartesianHi5 );
		}
		else if (ctx.appendResults.isIndexed()){
			mout.note("Cartesian, append mode: " , ctx.appendResults );
			appendCartesianH5(srcTmp, ctx.cartesianHi5);
		}
		else {
			mout.error("unsupported mode for ProductOp::appendResults=" , ctx.appendResults );
		}

		if (!ctx.composite.isMethodSet()){
			//const std::string m = ctx.cartesianHi5["how"].data.attributes["camethod"];
			const drain::Variable & m = ctx.cartesianHi5[ODIMPathElem::HOW].data.attributes["camethod"];
			if (!m.empty()){
				mout.info("adapting compositing method: " , m );
				ctx.composite.setMethod(m.toStr());  // TODO: try-catch if invalid?
			}
			else {
				mout.note("no compositing method (how:camethod) in metadata of '" , value , "', consider --cMethod " );
			}

		}
		/* consider... but also use cAdd
		if (!ctx.composite.projectionIsSet()){
			ctx.composite.odim.projdef =
			ctx.composite.setProjection(value);
		}
		*/
	}
	else {

		mout.info("Polar product [", object, "] thread=",  ctx.getId() );

		ctx.currentHi5 =      & ctx.polarInputHi5;
		ctx.currentPolarHi5 = & ctx.polarInputHi5;

		if (object == "SCAN"){
			mout.revised<LOG_WARNING>("moving rooted /how-attributes to /dataset<N>/how-attributes");

			for (const char * key: {
					"lowprf", "highprf", "NI", "astart", "rpm", "nsampleH", "nsampleV", "how:scan_index",
					"startazT", "stopazT", "startelA", "stopelA"
			}){

				if (srcTmp[ODIMPathElem::HOW].data.attributes.hasKey(key)){

					const drain::Variable & v = srcTmp[ODIMPathElem::HOW].data.attributes[key];

					for (auto & entry: srcTmp){
						if (entry.first.belongsTo(ODIMPathElem::DATASET)){
							if (!entry.second[ODIMPathElem::HOW].data.attributes.hasKey(key)){
								mout.revised<LOG_INFO>("moving ", key, "=", v, " -> ", entry.first, '/', ODIMPathElem::HOW);
								entry.second[ODIMPathElem::HOW].data.attributes[key] = v;
							}
						}
					}
				}

				if (srcTmp[ODIMPathElem::HOW].data.attributes.hasKey(key)){
					srcTmp[ODIMPathElem::HOW].data.attributes.erase(key);
				}
			}
		}

		// TODO: force APPEND / REPLACE?
		if (ctx.polarInputHi5.empty() || ctx.SCRIPT_DEFINED){
			if (ctx.SCRIPT_DEFINED){
				mout.info("Script defined, resetting previous inputs (if exist)");
			}
			ctx.polarInputHi5.swap(srcTmp);
			ctx.polarInputHi5.data.image.properties.clearVariables();
		}
		else {
			// "Automatic" append. Consider timestamp difference limit?
			const std::string sourceNew =            srcTmp[ODIMPathElem::WHAT].data.attributes["source"];
			const std::string sourceOld = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["source"]; // Warning: Creates attribute, unless it exists
			//mout.warn("Input: ", sourceNew, " Previous input: ", sourceOld, " same?: ", sourceNew==sourceOld);
			if (sourceNew == sourceOld){
				mout.ok<LOG_DEBUG>("Unchanged input src '", sourceNew, "' -> update (append) volume");
				appendPolarH5(srcTmp, ctx.polarInputHi5);
				mout.revised<LOG_WARNING>("ensuring PVOL (instead of SCAN)");
				ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["object"] = "PVOL";
			}
			else {
				mout.ok<LOG_NOTICE>("New input src '", sourceNew, "' (previous '", sourceOld, "') -> replace previous with new");
				ctx.polarInputHi5.swap(srcTmp);
				ctx.polarInputHi5.data.image.properties.clearVariables(); // ? ok
			}
		}

		// ctx.polarInputHi5.data.attributes.clear(); //NEW

		// mout.warn("next: updateCoordinatePolicy");
		// DataTools::updateCoordinatePolicy(ctx.polarInputHi5, RackResources::polarLeft);
		// mout.warn();
	}

	// mout.warn("next: updateInternalAttributes");
	DataTools::updateInternalAttributes(*ctx.currentHi5);

	mout.info("ok thread=",  ctx.getId() );

	mout.debug("end");

}

void CmdInputFile::appendCartesianH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.debug("start" );

	if (ctx.appendResults.is(ODIMPathElem::DATASET)){
		attachCartesianH5(srcRoot, dstRoot);
	}
	else if (ctx.appendResults.isIndexed()){

		if (srcRoot.empty())
			mout.warn(" srcRoot empty" );

		if (srcRoot.getChildren().size() > 1)
			mout.note(" srcRoot has several datasets" );


		ODIMPathElem parent(ODIMPathElem::DATASET);
		ODIMPathTools::getLastChild(dstRoot, parent, true); // <- CREATE
		// if (parent.getIndex()==0)
		//	 parent.index = 1; // NOT NEEDED!

		Hi5Tree & dst = dstRoot[parent];

		//for (Hi5Tree::pair_t & entry: srcRoot){
		for (auto & entry: srcRoot){
		//for (Hi5Tree::iterator it = srcRoot.begin(); it != srcRoot.end(); ++it){

			const ODIMPathElem & elem = entry.first;
			Hi5Tree & src = entry.second;
			// ODIMPathElem s(it->first); // possibly: what, where, how
			// Hi5Tree & src = it->second;

			if (elem.isIndexed()){
				mout.note(" appending " , elem , " => " , parent );
				attachCartesianH5(src, dst);
			}
			else {
				mout.note(" replacing " , elem );
				dst[elem].swap(src);   // overwrite what, where,
			}

		}
	}
	else {
		mout.warn(" could not find path with append=" , ctx.appendResults , ", swapping anyway" );
		dstRoot.swap(srcRoot);
	}

}

void CmdInputFile::attachCartesianH5(Hi5Tree & src, Hi5Tree & dst) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//ODIMPathElem p(g);
	//DataSelector::getLastChild(dst, p);

	/// TODO: New-iter
	//for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){
	for (auto & entry: src){

		ODIMPathElem p(entry.first); // possibly: what, where, how
		if (p.isIndexed()){
			ODIMPathTools::getNextChild(dst, p);
			mout.note(" appending ", p);
			dst[p].swap(entry.second);
		}
		else {
			mout.note("replacing ", entry.first);
			dst[entry.first].swap(entry.second);   // overwrite what, where,
		}

		src[entry.first].data.exclude = true;

	}


}

// Move to datatools:


/// Add DATASET groups of srcRoot to dstRoot.
/**
 *   Specialized to polar measurement data: time stamps identify the dataset groups.
 *   Note: also DataSelector applied.
 *
 */
// TODO: generalize? Not actually polar...
void CmdInputFile::appendPolarH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	mout.debug("start");
	const drain::Variable & sourceSrc = srcRoot[ODIMPathElem::WHAT].data.attributes["source"];
	const drain::Variable & sourceDst = dstRoot[ODIMPathElem::WHAT].data.attributes["source"];
	// mout.warn(sourceDst , " => " , sourceSrc );
	// TODO: consider option to accept different source?
	if (!sourceDst.empty()){
		if (!sourceSrc.empty()){
			if (sourceSrc.toStr() != sourceDst.toStr()){
				mout.warn("what:source changed in creating combined volume:");
				mout.warn("  dst: '", sourceDst, "'");
				mout.warn("  src: '", sourceSrc, "'");
			}
		}
		else {
			mout.warn("what:source empty, assuming: ", sourceDst);
		}
	}

	ODIMPathElemMap srcTimeGroups, dstTimeGroups;
	DataSelector::getTimeMap(srcRoot, srcTimeGroups);
	DataSelector::getTimeMap(dstRoot, dstTimeGroups);

	for (const auto & timeGroup: srcTimeGroups){

		Hi5Tree & srcDataSet = srcRoot[timeGroup.second];

		ODIMPathElemMap::iterator tit = dstTimeGroups.find(timeGroup.first);

		if (tit == dstTimeGroups.end()){
			mout.ok<LOG_INFO>("Adding new dataset, time: ", timeGroup.first);
			// In this form, does not support top-level QUALITY field insertions, neither updates.
			DataSelector::swapData(srcDataSet, dstRoot, ODIMPathElem::DATASET); // check type (DATASET) after filter implemented!
		}
		else {
			mout.attention("DATASET group exists for <", timeGroup.first, "> dst:", tit->second, " <-> src: ", timeGroup.second);

			Hi5Tree & dstDataSet = dstRoot[tit->second];

			ODIMPathElemMap srcQuantityGroups, dstQuantityGroups;
			DataTools::getQuantityMap(srcDataSet, srcQuantityGroups);
			DataTools::getQuantityMap(dstDataSet, dstQuantityGroups);

			for (const auto & quantityGroup: srcQuantityGroups){
				// Hi5Tree & srcData = srcDataSet[quantityGroup.second];
				updateData(srcDataSet[quantityGroup.second], quantityGroup.first, dstDataSet, dstQuantityGroups);
				/*
				ODIMPathElemMap::iterator qit = dstQuantityGroups.find(quantityGroup.first);
				if (qit == dstQuantityGroups.end()){
					mout.note("Adding new DATA group, quantity: ", quantityGroup.first);
				}
				else {
					Hi5Tree & srcData = srcDataSet[quantityGroup];
					mout.attention("DATA group exists for [", quantityGroup.first, "] dst:", qit->second, " <-> src: ", quantityGroup.second);
					updateDataNEW(srcData, quantityGroup.first, dstDataSet, dstQuantityGroups);
				}
				*/
			}

			// Update general quality, i.e. dataset level quality

			//QualityDataSupport<BasicDst> srcQ(srcDataSet);
			if (QualityDataSupport<BasicDst>(srcDataSet).getQuality().empty()){
				mout.debug("No quality fields, ok");
			}
			else {
				mout.special("Updating dataset level quality: '", timeGroup.first, "' -- ", timeGroup.second);
				updateQuality(srcDataSet, dstDataSet);
			}


		}
	}

	return;
}

/*
	/// Common dataSetSelector for srcRoot and dstRoot
	DataSelector dataSetSelector;
	dataSetSelector.setOrder(DataOrder::Crit::TIME, DataOrder::Oper::MIN);
	if (!ctx.select.empty())
		mout.attention("Applying input selector: ", ctx.select);
	dataSetSelector.consumeParameters(ctx.select); //??
	dataSetSelector.pathMatcher = "dataset:"; // <fix (constr?)

	/// TIMESTAMP-based order (Consider generalization for Cartesian?)
	//  Consider ODIMPathElem instead ??
	typedef std::map<std::string,ODIMPath> sweepMap;

	//sweepMap srcPaths;
	//dataSetSelector.getPathsByTime(srcRoot, srcPaths);
	ODIMPathList srcPaths;
	dataSetSelector.getPaths(srcRoot, srcPaths);

	//sweepMap dstPaths;
	// dataSetSelector.getPathsByTime(dstRoot, dstPaths);
	ODIMPathList dstPaths;
	dataSetSelector.getPaths(dstRoot, dstPaths);

	for (const auto & entry: dstPaths){
		//mout.info("exists: ", entry.second,"\t (", entry.first, ')');
		mout.info("exists: ", entry);
	}

	mout.debug("traverse paths");

	/// Traverse the child paths of srcRoot dataset[i]
	//for (const auto & srcDSEntry: srcPaths){
	for (const ODIMPath & path: srcPaths){

		//const sweepMap::key_type & timestamp = srcDSEntry.first; // rename => key
		//const ODIMPath & srcDataSetPath      = srcDSEntry.second;

		Hi5Tree & srcDataSet = srcRoot(path);  // srcDSEntry.second // clumsy (), could be [] because no leading '/'

		mout.unimplemented<LOG_WARNING>("Considering ", path);
		//mout.debug("Considering ", srcDSEntry.second, " (", srcDSEntry.first, ')');


		bool FOUND = false; // CLUMSY!

		// Search for the same key (timestamp)
		for (const auto & dstDSEntry: dstPaths){

			if (dstDSEntry.first == srcDSEntry.first){ // timestamp

				FOUND = true;

				Hi5Tree & dstDataSet = dstRoot(dstDSEntry.second);

				mout.note("Combining datasets having the same key (timestamp ", srcDSEntry.first, "): src:",
						srcDSEntry.second, " => dst:", dstDSEntry.second); //"("<< eit->first << ")" << mout.endl;

				typedef std::map<std::string, ODIMPathElem> quantity_map;

				// CONFLICT!? Or ok with ODIMPathElem::DATA to use
				quantity_map srcQuantityElems;
				DataSelector::getChildren(srcDataSet, srcQuantityElems, ODIMPathElem::DATA);  // consider children

				quantity_map dstQuantityElems;
				DataSelector::getChildren(dstDataSet, dstQuantityElems, ODIMPathElem::DATA);

				/// traverse DATA QUANTITIES assigning each to new / existing dstPaths in current structure.
				for (const auto & srcEntry: srcQuantityElems) {

					// NEW
					if ((srcEntry.first == "QIND") || (srcEntry.first == "CLASS")){
						// actually:
						mout.warn("Quality quantity [", srcEntry.first, "] found in DATA group selector?  @", srcEntry.second );
						// mout.note("Quality field [", srcEntry.first, "] @", srcEntry.first, " treated separately");
						// continue; // handled separately, see futher below
					}
					else {
						updateDataNEW(srcDataSet[srcEntry.second], srcEntry.first, dstDataSet, dstQuantityElems);
					}

				}

				// Update general quality, i.e. dataset level quality
				mout.special("Updating dataset level quality: '", dstDSEntry.first, "' -- ", dstDSEntry.second);
				updateQuality(srcDataSet, dstDataSet);

				// break; NO!
			}

		}

		if (!FOUND){ // Adding new DataSet group
			mout.note("Adding new data: '", srcDSEntry.first, "' -- ", srcDSEntry.second);
			//DataSelector::swapData(srcRoot, srcDSEntry.first, dstRoot);
			DataSelector::swapData(srcRoot, srcDSEntry.second.back(), dstRoot);
			// consider ODIM "swap" (copy) as well.
		}



	}
}
*/

// const std::string & srcKey,
// const ODIMPathElem & srcElem
void CmdInputFile::updateData(Hi5Tree & srcData, const std::string & srcKey, Hi5Tree & dstDataSet, const quantity_map & dstQuantityElems) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// NEW: clumsy search.
	for (const auto & dstEntry: dstQuantityElems) {
		if (srcKey == dstEntry.first){
			mout.ok<LOG_DEBUG>("Data group for [", srcKey, "] found -- (", dstEntry.second, "), updating quality only");
			// mout.note("-> Only updating local quality of [", srcKey, "] in dst: ", dstEntry.second);
			// Note: QIND and CLASS ruled out above (old comment)
			updateQuality(srcData, dstDataSet[dstEntry.second]);
			return;
		}
	}


	// Else:
	mout.ok<LOG_DEBUG>("key [quantity ", srcKey, "] not found, adding by swapping");

	DataSelector::swapData(srcData, dstDataSet, ODIMPathElem::DATA);

}



void CmdInputFile::updateQuality(Hi5Tree & src, Hi5Tree & dst) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


	{
		QualityDataSupport<BasicDst> srcQ(src);
		if (srcQ.getQuality().empty()){
			mout.debug("No quality fields, ok");
			return;
		}
	}


	{
		mout.special<LOG_INFO>("Step 1: Append missing quality fields ");  // other than QIND or CLASS

		//DataTools::updateInternalAttributes(src); // NEEDED?

		QualityDataSupport<BasicDst> srcQ(src);
		QualityDataSupport<BasicDst> dstQ(dst);

		for (auto & srcEntry: srcQ.getQuality()){ // Iterate by quantity

			// if ((srcEntry.first != "QIND") && (srcEntry.first != "CLASS")){
			// mout.attention("special quality information [", srcEntry.first, "] , elangle=", srcEntry.second.odim.elangle);
			// mout.attention("special quality information [", srcEntry.first, "] ");

			if (dstQ.hasQuality(srcEntry.first)){
				mout.note("dst already contains quality field [", srcEntry.first, "], skipping it (here)");
				// TODO: consider override, for fields other than QIND or CLASS? (Probably needs image.reset() )
			}
			else {
				mout.special("dst does not contain quality field [", srcEntry.first, "], adding");
				mout.debug("src field: ", srcEntry.second.odim);
				// PolarODIM tmp(srcEntry.second.odim);
				// Create new
				dstQ.getQualityData(srcEntry.first).odim.updateFromMap(srcEntry.second.odim);
				dstQ.getQualityData(srcEntry.first).getTree().swap(srcEntry.second.getTree());

				mout.debug("dst field: ", dstQ.getQualityData(srcEntry.first).odim);

			}

		}

		//DataTools::updateInternalAttributes(dst); // NEEDED?

	}


	{

		mout.special<LOG_INFO>("Step 2: Join quality fields QIND and CLASS");

		// TODO:
		// Future option: templated Polar/Cart
		typedef PolarSrc T_Src;
		typedef PolarDst T_Dst;

		const QualityDataSupport<T_Src> srcQ(src);
		const PlainData<T_Src> & srcQind  = srcQ.getQualityData("QIND");  // ie.
		const PlainData<T_Src> & srcClass = srcQ.getQualityData("CLASS"); // Maybe empty

		QualityDataSupport<T_Dst> dstQ(dst);

		if (!srcQind.data.isEmpty()){

			PlainData<T_Dst> & dstQind  = dstQ.getQualityData("QIND");

			if (!srcClass.data.isEmpty()){
				mout.special("both src and dst have QIND and CLASS");
				PlainData<T_Dst> & dstClass = dstQ.getQualityData("CLASS");
				// mout.advice("consider --aQualityCombiner"); // QualityCombinerOp::updateOverallQuality
				QualityCombinerOp::updateOverallQuality(srcQind, srcClass,	dstQind, dstClass);
			}
			else {
				mout.special("both src and dst have QIND (but not CLASS)");
			}

		}
		else {
			mout.special("srcQind empty, ok ");
		}

	}

	return;





	const QualityDataSupport<PolarSrc> srcQ(src);
	const PlainData<PolarSrc> & srcQind  = srcQ.getQualityData("QIND");  // ie.
	const PlainData<PolarSrc> & srcClass = srcQ.getQualityData("CLASS"); // Maybe empty



	if (srcQind.data.isEmpty()){
		mout.debug("srcQind empty, skipping update");
		return;
	}
	if (!srcClass.data.isEmpty()){
		// mout.note("updating QIND and CLASS");
	}
	else {
		mout.note("updating QIND (srcQind exists but srcClass empty");
	}


	QualityDataSupport<PolarDst> dstQ(dst);
	PlainData<PolarDst> & dstQind  = dstQ.getQualityData("QIND");
	PlainData<PolarDst> & dstClass = dstQ.getQualityData("CLASS");

	typedef std::map<std::string, ODIMPathElem> quantity_map;

	quantity_map srcQualityPaths;
	ODIMPathTools::getChildren(src, srcQualityPaths, ODIMPathElem::QUALITY);  // consider children

	quantity_map dstQualityPaths;
	ODIMPathTools::getChildren(dst, dstQualityPaths, ODIMPathElem::QUALITY);

	for (quantity_map::const_iterator it = srcQualityPaths.begin(); it != srcQualityPaths.end(); ++it) {

		const std::string & quantity  = it->first; // DBZH, or QIND or CLASS
		const ODIMPathElem & srcChild = it->second;

		if (quantity == "QIND"){  // => update (combine), do not just copy (override)

			mout.note("Updating QIND (and CLASS) with ", srcChild, '[', quantity, ']');
			// Todo: if no CLASS
			if (srcClass.data.isEmpty()){
				mout.warn("CLASS data empty for srcPath=", srcChild);
			}

			QualityCombinerOp::updateOverallQuality(srcQind, srcClass,	dstQind, dstClass);
			continue;

		}
		else if (quantity == "CLASS"){
			mout.debug3("Combining CLASS skipped (handled by QIND, if found)");
			continue;
		}

		/*  CONSIDER
		if (srcClass.data.isEmpty()){ // CLASS
			const PlainData<PolarSrc> & srcProb = sDataSet.getQualityData(quantity);
			if (srcProb.data.isEmpty()){
				mout.warn("Empty data for prob. quantity=", quantity );
				continue;
			}
			mout.note() << "No CLASS data in src, ok. Updating dst CLASS with prob.field; quantity="<< quantity << mout.endl;
			QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, quantity, (short unsigned int)123); // FIX code!
		}
		else {
			mout.info("Found quality data (CLASS), updates done on that, not on orig quality=" , quantity );
		}
		 */

		quantity_map::const_iterator dit = dstQualityPaths.find(quantity);

		if (dit == dstQualityPaths.end()){ // New quantity, simply add this \c dataN directly.

			// if dstChild EMPTY, add it now.
			if (srcClass.data.isEmpty()){
				mout.info() << "src CLASS empty for  ["<< quantity << "] " << srcChild;
				mout << " (update under constr.)" << mout.endl;
			}

			// if dstChild EMPTY, add it now.
			if (srcQind.data.isEmpty()){ // ie not handled above
				mout.note("src QIND empty for [", quantity, "] ", srcChild);
				mout.unimplemented("numeric class index? (update under constr.)");
				const PlainData<PolarSrc> srcProb(src[srcChild]);  // TODO: resources and quality code?
				//QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, quantity, (short unsigned int)123);
				QualityCombinerOp::updateOverallDetection(srcProb.data, dstQind, dstClass, quantity, (short unsigned int)123); // EXPERIMENTAL
			}

			ODIMPathElem dstChild(ODIMPathElem::QUALITY);
			ODIMPathTools::getNextChild(dst, dstChild);

			mout.note("Adding quality [", quantity , "] directly to ./" , dstChild  );

			// Create empty dstRoot[path] and swap it...
			dst[dstChild].swap(src[srcChild]);
		}
		else { // elangle is found in dstRoot.
			mout.note("UNDONE: Already [" , quantity , "] at " , dit->second  );
		}

	}


}



/*
template <class K>
struct DataSetPicker : public std::pair<K,ODIMPathElem> {

	typedef K key_t;
	typedef std::map<K,ODIMPathElem> map_t;

	virtual
	K getKey(const Hi5Tree & src, const ODIMPathElem & elem) = 0;
};

// Elangle
struct PolarDataSetPicker : public DataSetPicker<double> {

	virtual
	~PolarDataSetPicker(){};

	virtual
	double getKey(const Hi5Tree & src, const ODIMPathElem & elem){
		static double empty = -90.0;
		return src[ODIMPathElem::WHERE].data.attributes.get("elangle", empty);
	};

};

struct DataSetPickerByTime : public DataSetPicker<std::string> {

	virtual
	~DataSetPickerByTime(){};

	virtual
	std::string getKey(const Hi5Tree & src, const ODIMPathElem & elem){
		// static double empty = "";
		return src[ODIMPathElem::WHAT].data.attributes["startdate"].toStr() +
			src[ODIMPathElem::WHAT].data.attributes["starttime"].toStr();
	};

};

void pickElems(const Hi5Tree & src, int groupFilter, std::map<std::string, ODIMPathElem> & elems){

	static DataSetPickerByTime picker;

	for (const auto & entry: src){

		if (entry.first.belongsTo(groupFilter)){
			// consider error if exists
			elems[picker.getKey(src, entry.first)] = entry.first;
		}

	}

}
*/



void CmdInputFile::readListFile(const std::string & fullFilename) const  {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout( ;

	drain::FilePath path(fullFilename);

	bool APPEND_COMMANDS = false;
	if (path.extension == "vol"){
		APPEND_COMMANDS = false;
	}
	else if (path.extension == "lst"){
		APPEND_COMMANDS = true;
	}
	else {
		mout.error("unknown list file extension: ", path.extension);
	}


	std::string prefix;
	if (ctx.inputPrefix.empty()){
		prefix = path.dir.str() + '/';
		mout.revised("adding inputPrefix=", ctx.inputPrefix);
		//
	}
	else {
		prefix = ctx.inputPrefix;
	}

	drain::Input input(fullFilename);
	mout.experimental("Reading list: ", fullFilename);

	std::string line;
	while (std::getline((std::ifstream &)input, line)){
		if (!line.empty()){
			// mout.debug2(line );
			if (line.at(0) != '#'){
				// Note: comments also after commands could be stripped,
				// but simple char search is faulty as command args can contain hash '#'
				std::string inputFileName = drain::StringTools::trim(line);
				if (inputFileName == this->value){
					// Todo: use full paths in comparing...
					mout.error("Recursion detected while reading ", fullFilename);
				}
				else {
					if (APPEND_COMMANDS){
						mout.experimental("extending command sequence with: ", inputFileName);
						ctx.addedCommands.add("inputFile", prefix + inputFileName);
						//ctx.addedCommands.add(prefix + inputFileName);
					}
					else {
						// Now expect volume, read sweep(s) directly and append. No script exec after each.
						readFile(prefix + inputFileName);
					}
					//
				}
			}
		}
	}

	//hi5::Hi5Base::readText(ctx.polarInputHi5, ifstr);
	// DataTools::updateInternalAttributes(ctx.polarInputHi5);


}



void CmdInputFile::readTextFile(const std::string & fullFilename) const  {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout( ;

	drain::Input ifstr(fullFilename);

	hi5::Hi5Base::readText(ctx.polarInputHi5, ifstr);
	DataTools::updateInternalAttributes(ctx.polarInputHi5);


}

void CmdInputFile::readImageFile(const std::string & fullFilename) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout;

	/// Search last dataset
	ODIMPathElem dataSetElem(ODIMPathElem::DATASET);
	ODIMPathTools::getLastChild(ctx.polarInputHi5, dataSetElem, true); // <- CREATE
	// if (dataSetElem.getIndex() == 0)
	//	dataSetElem.index = 1;

	/// Search new data[n] in the dataset found
	ODIMPathElem dataElem(ODIMPathElem::DATA);
	// TODO: append cmd?
	ODIMPathTools::getLastChild(ctx.polarInputHi5[dataSetElem], dataElem, true); // <- CREATE
	// if (dataElem.getIndex() == 0)
	//	dataElem.index = 1;

	mout.debug("Found path " , dataSetElem , '>' , dataElem );
	if (!ctx.polarInputHi5[dataSetElem][dataElem][ODIMPathElem::ARRAY].data.image.isEmpty()){
		mout.debug("Path " , dataSetElem , '>' , dataElem , "/data contains data already, searching further..." );
		//DataSelector::getNextOrdinalPath(ctx.inputHi5, pathSearch, dataPath);
		++dataElem.index;
		mout.debug("Found path " , dataSetElem , '>' , dataElem );
	}

	// Perhaps explicitly set already
	std::string object = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes.get("object", "");

	//mout.warn(ctx.polarInputHi5[ODIMPathElem::WHAT]);

	Hi5Tree & dst = ctx.polarInputHi5[dataSetElem][dataElem];
	drain::image::Image & dstImage = dst[ODIMPathElem::ARRAY].data.image;
	//mout.special("WHAT");
	drain::image::ImageFile::read(dstImage, fullFilename);
	//const drain::image::Geometry & g = dstImage.getGeometry();

	// Non-const, modifications may follow
	drain::FlexVariableMap & attr = dstImage.properties;

	// No information, img has always (linked) attributes.
	// mout.info("Image has metadata: " , drain::Variable(!attr.empty()) );
	// mout.info(d );

	// mout.note(attr);
	drain::FlexibleVariable & obj = attr["what:object"];
	if (!obj.empty()){
		if (object.empty()){
			if (obj != object){
				mout.debug("modifying what:object '" , object , "' -> '" , obj , "'" );
			}
			object = obj.toStr();
		}
	}
	//std::string object = attr["what:object"].toStr();
	//mout.warn("object: '" , object , '[' , Type::getTypeChar(object.getType()) , "]', props: " ,  dstImage.properties );


	if (object.empty()){
		// unneeded? See above...
		object = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["object"].toStr();
	}


	if (object.empty()){ // unneeded? See below.
		object = "SCAN";
		mout.note("what:object empty, assuming ", object ,"'" );
	}
	else {
		mout.warn("object: " , object );
	}
	/*
	if (object == "COMP"){
		CartesianODIM odim;
		odim.updateFromMap(attr);
		mout.note("Composite detected" );
		mout.debug(odim );
	}
	else if ((object == "SCAN") || (object == "PVOL")){
		PolarODIM odim;
		odim.updateFromMap(attr);
		mout.note("Polar scan detected" );
		mout.debug(odim );
	}
	else {
		mout.note("what:object not SCAN, PVOL or COMP: rack provides limited support" );
	}
	*/


	DataTools::updateInternalAttributes(ctx.polarInputHi5); // [dataSetElem] enough?
	mout.debug("props: " ,  dstImage.properties );

	if ((object == "COMP")|| (object == "IMAGE") ){
		mout.note("Cartesian product detected" );
		CartesianODIM odim; //(dstImage.properties);
		deriveImageODIM(dstImage, odim);  // generalize in ODIM.h (or obsolete already)
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATASET>(odim, ctx.polarInputHi5[dataSetElem]); // $ odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::updateH5AttributeGroups<ODIMPathElem::ROOT>(odim, ctx.polarInputHi5); // $ odim.copyToRoot(ctx.inputHi5);
		mout.unimplemented("swap HDF5 for Cartesian data" );
	}
	else {
		if ((object == "SCAN") || (object == "PVOL")) {
			mout.note("Polar object (" , object , ") detected" );
		}
		else {
			ctx.polarInputHi5["what"].data.attributes["object"] = "SCAN";
			mout.warn("metadata: what:object=", object, ", assuming SCAN (Polar scan)" );
		}
		PolarODIM odim;
		deriveImageODIM(dstImage, odim);   // TODO generalize in ODIM.h (or obsolete already)
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::updateH5AttributeGroups<ODIMPathElem::DATASET>(odim, ctx.polarInputHi5[dataSetElem]); // $odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::updateH5AttributeGroups<ODIMPathElem::ROOT>(odim, ctx.polarInputHi5); // $ odim.copyToRoot(ctx.inputHi5);
	}

	DataTools::updateInternalAttributes(ctx.polarInputHi5);


}


} // namespace rack



// Rack
