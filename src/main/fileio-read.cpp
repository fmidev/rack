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


#include <set>
#include <ostream>

#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/util/Input.h"
#include "drain/image/FilePng.h"
#include "drain/image/FilePnm.h"
#include "drain/image/FileTIFF.h"
#include "drain/prog/Command.h"

#include "andre/QualityCombinerOp.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "data/ODIM.h"
#include "fileio.h"
#include "fileio-read.h"






namespace rack {



void CmdInputFile::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, getName().c_str()); // __FILE__

	// mout.timestamp("BEGIN_FILEREAD");

	mout.note() << "reading: "<< value << mout.endl;

	// TODO: expand?
	std::string fullFilename = ctx.inputPrefix + value;

	ctx.statusFlags.unset(drain::StatusFlags::INPUT_ERROR);
	ctx.statusFlags.unset(drain::StatusFlags::DATA_ERROR);
	ctx.unsetCurrentImages();

	//const drain::CommandRegistry & r = drain::getRegistry();
	//mout.warn() << "lastCommand: '" << drain::CommandRegistry::index << r.getLastCommand() << "'" << mout.endl;

	// Kludge. Check if last command was str than 1) file read or 2) odim assignment ( --/path:key=value )
	// inputComplete = (r.getLastCommand() != this->name) && (r.getLastCommand() != "CmdSetODIM");
	// mout.warn() << "inputComplete: " << (int)inputComplete << mout.endl;
	// mout.warn() << "autoExec:      " << (int)cmdAutoExec.exec << mout.endl;
	drain::FilePath path(value);
	const bool NO_EXTENSION = path.extension.empty();
	/*
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
	*/

	try {

		/*
		if (hi5::fileInfo.checkExtension(format)){ // "h5", "hdf", "hdf5"
				handleParams(hdf5Conf, params);
			}
			else if (drain::image::FilePng::fileInfo.checkExtension(format)){
				handleParams(pngConf, params);
			}
			else if (drain::image::FilePnm::fileInfo.checkExtension(format)){
				mout.unimplemented("(no parameters supported for PPM/PGM )");
			}
			else if (drain::image::FileGeoTIFF::fileInfo.checkExtension(format)){ // "tif"
				handleParams(gtiffConf, params);
				//mout.note("keys", gtiffConf.getKeys());
			}
		*/

		if (hi5::fileInfo.checkExtension(path.extension) || NO_EXTENSION){
			if (NO_EXTENSION){
				mout.discouraged("No file extension! Assuming HDF5...");
			}
			readFileH5(fullFilename);
		}
		else if (drain::image::FileTIFF::fileInfo.checkExtension(path.extension)){
			mout.error("Reading TIFF files not supported");
		}
		else if (drain::image::FilePng::fileInfo.checkExtension(path.extension)){ //(IMAGE_PNG || IMAGE_PNM){
			readImageFile(fullFilename);
		}
		else if (drain::image::FilePnm::fileInfo.checkExtension(path.extension)){ //(IMAGE_PNG || IMAGE_PNM){
			readImageFile(fullFilename);
		}
		else if (textFileExtension.test(this->value))
			readTextFile(fullFilename);
		else {
			mout.error("Unrecognized/unsupported file type, filename: '", this->value, "'");
		}

	}
	catch (const std::exception & e) {
		//resources.inputOk = false;
		ctx.statusFlags.set(drain::StatusFlags::INPUT_ERROR);
		mout.debug() << e.what() << mout.endl;
		/*
		if (resources.scriptParser.autoExec > 0){  // => go on with str inputs
			mout.warn() << "Read error, file: " << this->value << mout.endl;
		}
		else {
			mout.error() << "Read error, file: " << this->value << mout.endl;
			//exit(1);
		}
		*/
		return;
	}


	ctx.select.clear(); // NEW: "starts a product pipe". monitor effects of this

	//mout.note() << "resources.getUpdatedStatusMap()" << mout.endl;
	//mout.note() << "ctx.getStatusMap()" << mout;

	drain::VariableMap & vmap = ctx.getStatusMap();

	vmap["inputFileBasename"] = path.basename;

	path.basename.clear();
	vmap["inputDir"] = path.str();
	//mout.note() << path"ctx.getStatusMap() start" << mout;

	// mout.timestamp("END_FILEREAD");
	//mout.warn() << "resources.getUpdatedStatusMap()" << mout.endl;

	//mout.special("END READ thread #", ctx.getId());


}



/// Reads hdf5 file and appends it to H5 structure. Works only with sweeps (SCAN), volume (PVOL) or Cartesian data (COMP) (having elevations).
void CmdInputFile::readFileH5(const std::string & fullFilename) const {  // TODO

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); //REPL name, ""); // __FUNCTION__

	//mout.debug() << "start: " << fullFilename << mout.endl;

	mout.debug("thread #", ctx.getId(), ": reading ", fullFilename);


	// InputSelect needed?
	Hi5Tree srcTmp;
	hi5::Reader::readFile(fullFilename, srcTmp); //, resources.inputSelect); //, 0);  // 0 = read no attributes or datasets (yet)
	//hi5::Reader::readFile(fullFilename, srcTmp, ctx.inputFilter.value); //, resources.inputSelect); //, 0);  // 0 = read no attributes or datasets (yet)

	if (mout.isDebug(6)){
		mout.debug3("input data: ");
		hi5::Hi5Base::writeText(srcTmp, std::cerr);
	}


	DataTools::updateInternalAttributes(srcTmp); // could be replaced, see below; only timestamp needed at this point?

	// const bool SCRIPT_DEFINED = ctx.getStatus("script"); //  ((this->section & TRIGGER_SECTION) && ctx.getStatus("script"));
	// const bool SCRIPT_DEFINED = ctx.SCRIPT_DEFINED;


	mout.debug() << "Derive file type (what:object)" << mout.endl;
	drain::Variable & object = srcTmp[ODIMPathElem::WHAT].data.attributes["object"]; // beware of swap later
	if (object.empty()){
		mout.warn() << "/what:object empty, assuming polar volume, 'PVOL'" << mout.endl;
		object = "PVOL";
	}

	if ((object.toStr() == "COMP") || (object == "IMAGE")){

		mout.info() << "Cartesian [" << object << ']' << mout.endl;

		//DataTools::updateCoordinatePolicy(srcTmp, RackResources::limit);
		ctx.currentHi5 = & ctx.cartesianHi5;

		// Move or append srcTmp to ctx.cartesianHi5
		if (ctx.appendResults.isRoot() || ctx.cartesianHi5.empty()){
			// Move (replace)
			ctx.cartesianHi5.swap(srcTmp);
			//mout.note() << ctx.cartesianHi5 << mout.endl;
			mout.info() << "Swapped: " << ctx.cartesianHi5 << mout.endl;
		}
		else if (ctx.appendResults.isIndexed()){
			mout.note() << "Cartesian, append mode: " << ctx.appendResults << mout.endl;
			appendCartesianH5(srcTmp, ctx.cartesianHi5);
		}
		else {
			mout.error() << "unsupported mode for ProductOp::appendResults=" << ctx.appendResults << mout.endl;
		}

		if (!ctx.composite.isMethodSet()){
			//const std::string m = ctx.cartesianHi5["how"].data.attributes["camethod"];
			const drain::Variable & m = ctx.cartesianHi5[ODIMPathElem::HOW].data.attributes["camethod"];
			if (!m.empty()){
				mout.info() << "adapting compositing method: " << m << mout.endl;
				ctx.composite.setMethod(m.toStr());  // TODO: try-catch if invalid?
			}
			else {
				mout.note() << "no compositing method (how:camethod) in metadata of '" << value << "', consider --cMethod " << mout.endl;
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


		// TODO: force APPEND / REPLACE?
		if (ctx.polarInputHi5.empty() || ctx.SCRIPT_DEFINED){
			if (ctx.SCRIPT_DEFINED)
				mout.info("script defined, reseting previous inputs (if exist)");
				//mout.info("SCRIPT_DEFINED: ", ctx.SCRIPT_DEFINED, " thread:", ctx.getName());
			ctx.polarInputHi5.swap(srcTmp);
		}
		else {
			// "Automatic" append. Consider timestamp difference limit?
			const std::string sourceNew =            srcTmp[ODIMPathElem::WHAT].data.attributes["source"];
			const std::string sourceOld = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["source"]; // Warning: Creates attribute, unless it exists
			//mout.warn("Input: ", sourceNew, " Previous input: ", sourceOld, " same?: ", sourceNew==sourceOld);
			if (sourceNew == sourceOld){
				mout.debug("Unchanged input src '", sourceNew, "' -> update (append) volume");
				appendPolarH5(srcTmp, ctx.polarInputHi5);
			}
			else {
				mout.debug("New input src '", sourceNew, "' (previous '", sourceOld, "') -> replace previous with new");
				ctx.polarInputHi5.swap(srcTmp);
			}
		}

		//mout.warn() << "s" << mout.endl;
		DataTools::updateCoordinatePolicy(ctx.polarInputHi5, RackResources::polarLeft);

	}

	DataTools::updateInternalAttributes(*ctx.currentHi5);

	mout.info("ok thread=",  ctx.getId() );

	mout.debug("end");

}

void CmdInputFile::appendCartesianH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;

	if (ctx.appendResults.is(ODIMPathElem::DATASET)){
		attachCartesianH5(srcRoot, dstRoot);
	}
	else if (ctx.appendResults.isIndexed()){

		if (srcRoot.empty())
			mout.warn() << " srcRoot empty" << mout.endl;

		if (srcRoot.getChildren().size() > 1)
			mout.note() << " srcRoot has several datasets" << mout.endl;


		ODIMPathElem parent(ODIMPathElem::DATASET);
		DataSelector::getLastChild(dstRoot, parent);
		if (parent.getIndex()==0)
			parent.index = 1;

		Hi5Tree & dst = dstRoot[parent];

		//for (Hi5Tree::pair_t & entry: srcRoot){
		for (auto & entry: srcRoot){
		//for (Hi5Tree::iterator it = srcRoot.begin(); it != srcRoot.end(); ++it){

			const ODIMPathElem & elem = entry.first;
			Hi5Tree & src = entry.second;
			// ODIMPathElem s(it->first); // possibly: what, where, how
			// Hi5Tree & src = it->second;

			if (elem.isIndexed()){
				mout.note() << " appending " << elem << " => " << parent << mout.endl;
				attachCartesianH5(src, dst);
			}
			else {
				mout.note() << " replacing " << elem << mout.endl;
				dst[elem].swap(src);   // overwrite what, where,
			}

		}
	}
	else {
		mout.warn() << " could not find path with append=" << ctx.appendResults << ", swapping anyway" << mout.endl;
		dstRoot.swap(srcRoot);
	}

}

void CmdInputFile::attachCartesianH5(Hi5Tree & src, Hi5Tree & dst) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	//ODIMPathElem p(g);
	//DataSelector::getLastChild(dst, p);

	/// TODO: New-iter
	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){

		ODIMPathElem p(it->first); // possibly: what, where, how
		if (p.isIndexed()){
			DataSelector::getNextChild(dst, p);
			mout.note(" appending ", p);
			dst[p].swap(it->second);
		}
		else {
			mout.note() << " replacing " << it->first << mout.endl;
			dst[it->first].swap(it->second);   // overwrite what, where,
		}

		src[it->first].data.noSave = true;

	}


}

// Move to datatools:



/**  Specialized to polar measurement data: time stamps identify the dataset groups.
 *   Note: also DataSelector applied.
 *
 */
// TODO: generalize? Not actually polar...
void CmdInputFile::appendPolarH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.debug("start");
	const drain::Variable & sourceSrc = srcRoot[ODIMPathElem::WHAT].data.attributes["source"];
	const drain::Variable & sourceDst = dstRoot[ODIMPathElem::WHAT].data.attributes["source"];
	// mout.warn() << sourceDst << " => " << sourceSrc << mout.endl;
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


	/// Common dataSetSelector for srcRoot and dstRoot
	DataSelector dataSetSelector;
	if (!ctx.select.empty())
		mout.attention("Applying input selector: ", ctx.select);
	dataSetSelector.consumeParameters(ctx.select); //??
	dataSetSelector.pathMatcher = "dataset:"; // <fix

	/// TIMESTAMP-based order (Consider generalization for Cartesian?)
	//  Consider ODIMPathElem instead ??
	typedef std::map<std::string,ODIMPath> sweepMap;

	sweepMap srcPaths;
	dataSetSelector.getPathsByTime(srcRoot, srcPaths);
	//dataSetSelector.getPaths(srcRoot, srcPaths);

	sweepMap dstPaths;
	dataSetSelector.getPathsByTime(dstRoot, dstPaths);
	// dataSetSelector.getPaths(dstRoot, dstPaths);

	for (const auto & entry: dstPaths){
		mout.info("exists: ", entry.second,"\t (", entry.first, ')');
	}


	mout.debug("traverse paths");

	/// Traverse the child paths of srcRoot dataset[i]
	//for (sweepMap::const_iterator it = srcPaths.begin(); it != srcPaths.end(); ++it){
	for (const auto & srcDSEntry: srcPaths){

		//const sweepMap::key_type & timestamp = srcDSEntry.first; // rename => key
		//const ODIMPath & srcDataSetPath      = srcDSEntry.second;

		Hi5Tree & srcDataSet = srcRoot(srcDSEntry.second);  // clumsy (), could be [] because no leading '/'

		mout.debug("Considering ", srcDSEntry.second, " (", srcDSEntry.first, ')');

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



		/*

		sweepMap::const_iterator eit = dstPaths.find(srcDSEntry.first);

		if (eit == dstPaths.end()){ // New timestamp, add this \c dataset directly.

			ODIMPathElem child(ODIMPathElem::DATASET);
			DataSelector::getNextChild(dstRoot, child);
			mout.info() << "New timestamp (" << srcDSEntry.first << "), appending to path=" << child << mout.endl;
			// Create empty dstRoot[path] and swap it...
			Hi5Tree & dstDataSet = dstRoot[child];
			dstDataSet.swap(srcDataSet);
		}
		else { // timestamp is found in dstRoot.

			static const DataSelector dataSelector; // ?? Needed init?

			const ODIMPath & dstDataSetPath = eit->second;

			Hi5Tree & dstDataSet = dstRoot(dstDataSetPath);

			typedef std::map<std::string, ODIMPathElem> quantity_map;


			mout.note("Combining datasets having the same  (timestamp ", srcDSEntry.first, "): src:", srcDSEntry.second, " => dst:", dstDataSetPath); //"("<< eit->first << ")" << mout.endl;

			// CONFLICT!? Or ok with ODIMPathElem::DATA to use
			quantity_map srcQuantityElems;
			DataSelector::getChildren(srcDataSet, srcQuantityElems, ODIMPathElem::DATA);  // consider children

			quantity_map dstQuantityElems;
			DataSelector::getChildren(dstDataSet, dstQuantityElems, ODIMPathElem::DATA);

			/// traverse DATA QUANTITIES assigning each to new / existing dstPaths in current structure.
			for (const auto & entry: srcQuantityElems) {

				const std::string & quantity  = entry.first; // qit-> // DBZH, or QIND or CLASS
				const ODIMPathElem & srcElem = entry.second;  //srcPath.back();
				Hi5Tree & srcData = srcDataSet[srcElem];

				// NEW
				if ((quantity == "QIND") || (quantity == "CLASS")){
					continue; // handled separately, see futher below
				}

				// NEW: clumsy search.
				for (const auto & dstEntry: dstQuantityElems) {
					const std::string  & dstQuantity  = dstEntry.first; // qit-> // DBZH, or QIND or CLASS
					const ODIMPathElem & dstElem      = dstEntry.second;


					if (quantity == dstQuantity){
						mout.note("Quantity [", quantity, "] already exists in dst: ", dstElem);
						// Note: QIND and CLASS ruled out above
						updateQuality(srcData, dstDataSet[dstElem]);
						continue;
					}
				}

				// WRONG! continue goes on here, "return" would be correct

				ODIMPathElem dstChild(ODIMPathElem::DATA);
				DataSelector::getNextChild(dstDataSet, dstChild);
				mout.note("Add new quantity ",  quantity, " => ", dstDataSetPath, '|', dstChild);
				Hi5Tree & dstData = dstDataSet[dstChild];
				srcData.swap(dstData);

			}


			// Update dataset quantities
			updateQuality(srcDataSet, dstDataSet);

		}
		 */

		// }

	}
}


// const std::string & srcKey,
// const ODIMPathElem & srcElem
void CmdInputFile::updateDataNEW(Hi5Tree & srcData, const std::string & srcKey, Hi5Tree & dstDataSet, const quantity_map & dstQuantityElems) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	// NEW: clumsy search.
	for (const auto & dstEntry: dstQuantityElems) {
		if (srcKey == dstEntry.first){
			mout.note("Data group exists already [", srcKey, "] -- (", dstEntry.second, "), updating quality only");
			// mout.note("-> Only updating local quality of [", srcKey, "] in dst: ", dstEntry.second);
			// Note: QIND and CLASS ruled out above
			updateQuality(srcData, dstDataSet[dstEntry.second]);
			// continue;
			return;
		}
	}


	// Else:
	mout.attention("key [quantity ", srcKey, "] not found, adding by swapping");

	DataSelector::swapData(srcData, dstDataSet, ODIMPathElem::DATA);

}



void CmdInputFile::updateQuality(Hi5Tree & src, Hi5Tree & dst) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	{
		mout.special("Step 1: Append missing quality fields ");  // other than QIND or CLASS

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

		mout.special("Step 2: Join quality fields QIND and CLASS");

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
	DataSelector::getChildren(src, srcQualityPaths, ODIMPathElem::QUALITY);  // consider children

	quantity_map dstQualityPaths;
	DataSelector::getChildren(dst, dstQualityPaths, ODIMPathElem::QUALITY);

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
				mout.warn() << "Empty data for prob. quantity="<< quantity << mout.endl;
				continue;
			}
			mout.note() << "No CLASS data in src, ok. Updating dst CLASS with prob.field; quantity="<< quantity << mout.endl;
			QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, quantity, (short unsigned int)123); // FIX code!
		}
		else {
			mout.info() << "Found quality data (CLASS), updates done on that, not on orig quality=" << quantity << mout.endl;
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
			DataSelector::getNextChild(dst, dstChild);

			mout.note() << "Adding quality ["<< quantity << "] directly to ./" << dstChild  << mout.endl;

			// Create empty dstRoot[path] and swap it...
			dst[dstChild].swap(src[srcChild]);
		}
		else { // elangle is found in dstRoot.
			mout.note() << "UNDONE: Already [" << quantity << "] at " << dit->second  << mout.endl;
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





void CmdInputFile::readTextFile(const std::string & fullFilename) const  {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getResources().mout( ;

	drain::Input ifstr(fullFilename);

	hi5::Hi5Base::readText(ctx.polarInputHi5, ifstr);
	DataTools::updateInternalAttributes(ctx.polarInputHi5);


}

void CmdInputFile::readImageFile(const std::string & fullFilename) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getResources().mout;

	/// Search last dataset
	ODIMPathElem dataSetElem(ODIMPathElem::DATASET);
	DataSelector::getLastChild(ctx.polarInputHi5, dataSetElem);
	if (dataSetElem.getIndex() == 0)
		dataSetElem.index = 1;

	/// Search new data[n] in the dataset found
	ODIMPathElem dataElem(ODIMPathElem::DATA);
	// TODO: append cmd?
	DataSelector::getLastChild(ctx.polarInputHi5[dataSetElem], dataElem);
	if (dataElem.getIndex() == 0)
		dataElem.index = 1;

	mout.debug() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	if (!ctx.polarInputHi5[dataSetElem][dataElem][ODIMPathElem::ARRAY].data.dataSet.isEmpty()){
		mout.debug() << "Path " << dataSetElem << '>' << dataElem << "/data contains data already, searching further..." << mout.endl;
		//DataSelector::getNextOrdinalPath(ctx.inputHi5, pathSearch, dataPath);
		++dataElem.index;
		mout.debug() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	}

	// Perhaps explicitly set already
	std::string object = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["object"];

	mout.warn() << ctx.polarInputHi5[ODIMPathElem::WHAT] << mout;

	Hi5Tree & dst = ctx.polarInputHi5[dataSetElem][dataElem];
	drain::image::Image & dstImage = dst[ODIMPathElem::ARRAY].data.dataSet;
	//mout.special("WHAT");
	drain::image::ImageFile::read(dstImage, fullFilename);
	//const drain::image::Geometry & g = dstImage.getGeometry();

	// Non-const, modifications may follow
	drain::FlexVariableMap & attr = dstImage.properties;


	// No information, img has always (linked) attributes.
	// mout.info() << "Image has metadata: " << drain::Variable(!attr.empty()) << mout.endl;
	//mout.info() << d << mout;

	mout.note() << attr << mout;
		//mout.warn() << "object empty,: '" <<
	drain::Variable & obj = attr["what:object"];
	if (!obj.empty()){
		if (object.empty()){
			if (obj != object){
				mout.warn() << "overwriting what:object '" << object << "' -> '" << obj << "'" << mout;
			}
			object = obj.toStr();
		}
	}
	//std::string object = attr["what:object"].toStr();
	//mout.warn() << "object: '" << object << '[' << Type::getTypeChar(object.getType()) << "]', props: " <<  dstImage.properties << mout.endl;


	if (object.empty()){
		object = ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["object"].toStr();
	}


	if (object.empty()){ // unneeded? See below.
		object = "SCAN";
		mout.note() << "what:object empty, assuming "<< object <<"'" << mout;
	}
	else {
		mout.warn() << "object: " << object << mout;
	}
	/*
	if (object == "COMP"){
		CartesianODIM odim;
		odim.updateFromMap(attr);
		mout.note() << "Composite detected" << mout.endl;
		mout.debug() << odim << mout.endl;
	}
	else if ((object == "SCAN") || (object == "PVOL")){
		PolarODIM odim;
		odim.updateFromMap(attr);
		mout.note() << "Polar scan detected" << mout.endl;
		mout.debug() <<  odim << mout.endl;
	}
	else {
		mout.note() << "what:object not SCAN, PVOL or COMP: rack provides limited support" << mout.endl;
	}
	*/


	DataTools::updateInternalAttributes(ctx.polarInputHi5); // [dataSetElem] enough?
	mout.debug() << "props: " <<  dstImage.properties << mout.endl;

	if ((object == "COMP")|| (object == "IMAGE") ){
		mout.note() << "Cartesian product detected" << mout.endl;
		CartesianODIM odim; //(dstImage.properties);
		deriveImageODIM(dstImage, odim);  // generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, ctx.polarInputHi5[dataSetElem]); // $ odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, ctx.polarInputHi5); // $ odim.copyToRoot(ctx.inputHi5);
		mout.unimplemented() << "swap HDF5 for Cartesian data" << mout;
	}
	else {
		if ((object == "SCAN") || (object == "PVOL")) {
			mout.note() << "Polar object (" << object << ") detected" << mout;
		}
		else {
			ctx.polarInputHi5["what"].data.attributes["object"] = "SCAN";
			mout.warn() << "No what:object in metadata, assuming SCAN (Polar scan)" << mout;
		}
		PolarODIM odim;
		deriveImageODIM(dstImage, odim);   // TODO generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, ctx.polarInputHi5[dataSetElem]); // $odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, ctx.polarInputHi5); // $ odim.copyToRoot(ctx.inputHi5);
	}

	DataTools::updateInternalAttributes(ctx.polarInputHi5);


}


} // namespace rack



// Rack
