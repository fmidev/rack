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

//#include "drain/prog/CommandRegistry.h"

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

	mout.timestamp("BEGIN_FILEREAD");

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
	const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
	const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
	const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
	const bool NO_EXTENSION = path.extension.empty();


	try {


		if (hi5::fileInfo.checkPath(value) || NO_EXTENSION){
			if (NO_EXTENSION){
				mout.discouraged("No file extension! Assuming HDF5...");
			}
			// if (h5FileExtension.test(this->value)){
			readFileH5(fullFilename);
		}
		else if (IMAGE_TIF){
			mout.error("Reading TIFF files not supported");
		}
		else if (IMAGE_PNG || IMAGE_PNM){
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

	//mout.note() << "ctx.getStatusMap() start" << mout;

	mout.timestamp("END_FILEREAD");
	//mout.warn() << "resources.getUpdatedStatusMap()" << mout.endl;


}



/// Reads hdf5 file and appends it to H5 structure. Works only with sweeps (SCAN), volume (PVOL) or Cartesian data (COMP) (having elevations).
void CmdInputFile::readFileH5(const std::string & fullFilename) const {  // TODO

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); //REPL name, ""); // __FUNCTION__

	mout.debug() << "start: " << fullFilename << mout.endl;

	//RackResources & resources = getResources();

	// InputSelect needed?
	Hi5Tree srcTmp;
	hi5::Reader::readFile(fullFilename, srcTmp); //, resources.inputSelect); //, 0);  // 0 = read no attributes or datasets (yet)

	if (mout.isDebug(6)){
		mout.debug3() << "input data:" << mout.endl;
		hi5::Hi5Base::writeText(srcTmp, std::cerr);
	}


	DataTools::updateInternalAttributes(srcTmp); // could be replaced, see below; only timestamp needed at this point?
	//mout.warn() << "updateInternal" << mout.endl;
	//ctx.unsetCurrentImages();

	/// True, if user seems to provide
	// const drain::CommandRegistry & r = drain::getRegistry();
	// const std::string &lastCmd = r.getLastCommand();
	// const bool AUTO_EXEC    = (resources.scriptParser.autoExec > 0);
	// const bool AUTO_EXEC = this->execRoutine; // NEW
	// const bool SCRIPT_DEFINED = false; // (lastCmd == this->name) || (lastCmd == "CmdSetODIM") || (lastCmd == "CmdInputPrefix");
	// const bool APPEND_INPUT = !ctx.statusFlags.isSet(drain::CommandBank::SCRIPT_DEFINED); // TODO: check also if this command is triggering (any more)
	// static const drain::Flagger::value_t TRIGGER_SECTION = drain::Static::get<drain::TriggerSection>().index;
	const bool SCRIPT_DEFINED = ctx.getStatus("script"); //  ((this->section & TRIGGER_SECTION) && ctx.getStatus("script"));


	mout.debug() << "Derive file type (what:object)" << mout.endl;
	drain::Variable & object = srcTmp[ODIMPathElem::WHAT].data.attributes["object"]; // beware of swap later
	if (object.isEmpty()){
		mout.warn() << "/what:object empty, assuming polar volume, 'PVOL'" << mout.endl;
		object = "PVOL";
	}

	if ((object.toStr() == "COMP") || (object == "IMAGE")){

		mout.info() << "Cartesian [" << object << ']' << mout.endl;

		//DataTools::updateCoordinatePolicy(srcTmp, RackResources::limit);
		ctx.currentHi5 = & ctx.cartesianHi5;

		// Move or append srcTmp to ctx.cartesianHi5
		if (ctx.appendResults.isRoot() || ctx.cartesianHi5.isEmpty()){
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
			if (!m.isEmpty()){
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

		mout.info() << "Polar product [" << object << ']' << mout.endl;

		ctx.currentHi5 =      & ctx.inputHi5;
		ctx.currentPolarHi5 = & ctx.inputHi5;

		if (ctx.inputHi5.isEmpty() || SCRIPT_DEFINED){
			ctx.inputHi5.swap(srcTmp);
		}
		else {
			appendPolarH5(srcTmp, ctx.inputHi5);
		}

		//mout.warn() << "s" << mout.endl;
		DataTools::updateCoordinatePolicy(ctx.inputHi5, RackResources::polarLeft);

	}

	DataTools::updateInternalAttributes(*ctx.currentHi5);


	mout.debug() << "end" << mout.endl;

}

void CmdInputFile::appendCartesianH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;

	if (ctx.appendResults.is(ODIMPathElem::DATASET)){
		attachCartesianH5(srcRoot, dstRoot);
	}
	else if (ctx.appendResults.isIndexed()){

		if (srcRoot.isEmpty())
			mout.warn() << " srcRoot empty" << mout.endl;

		if (srcRoot.getChildren().size() > 1)
			mout.note() << " srcRoot has several datasets" << mout.endl;


		ODIMPathElem parent(ODIMPathElem::DATASET);
		DataSelector::getLastChild(dstRoot, parent);
		if (parent.getIndex()==0)
			parent.index = 1;

		Hi5Tree & dst = dstRoot[parent];

		for (Hi5Tree::map_t::value_type & entry: srcRoot){
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

	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){

		ODIMPathElem p(it->first); // possibly: what, where, how
		if (p.isIndexed()){
			DataSelector::getNextChild(dst, p);
			mout.note() << " appending " << p << mout.endl;
			dst[p].swap(it->second);
		}
		else {
			mout.note() << " replacing " << it->first << mout.endl;
			dst[it->first].swap(it->second);   // overwrite what, where,
		}

		src[it->first].data.noSave = true;

	}


}

void CmdInputFile::updateQuality(Hi5Tree & src, Hi5Tree & dst) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	const QualityDataSupport<PolarSrc> srcQ(src);
	const PlainData<PolarSrc> & srcQind  = srcQ.getQualityData("QIND");  // ie.
	const PlainData<PolarSrc> & srcClass = srcQ.getQualityData("CLASS"); // Maybe empty

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

			mout.note() << "Updating QIND (and CLASS) with "<< srcChild << '[' << quantity << ']' << mout.endl;
			// Todo: if no CLASS
			if (srcClass.data.isEmpty()){
				mout.warn() << "CLASS data for srcPath=" << srcChild << mout.endl;
			}

			QualityCombinerOp::updateOverallQuality(srcQind, srcClass,	dstQind, dstClass);
			continue;

		}
		else if (quantity == "CLASS"){
			mout.debug3() << "Combining CLASS skipped (handled by QIND, if found)" << mout.endl;
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
				mout.warn() << "src QIND empty for  ["<< quantity << "] " << srcChild << mout.endl;
				mout.unimplemented() << "numeric class index? (update under constr.)" << mout.endl;
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

void CmdInputFile::appendPolarH5(Hi5Tree & srcRoot, Hi5Tree & dstRoot) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	mout.debug() << "start" << mout.endl;
	const drain::Variable & sourceSrc = srcRoot[ODIMPathElem::WHAT].data.attributes["source"];
	const drain::Variable & sourceDst = dstRoot[ODIMPathElem::WHAT].data.attributes["source"];
	// mout.warn() << sourceDst << " => " << sourceSrc << mout.endl;
	if (!sourceDst.isEmpty()){
		if (!sourceSrc.isEmpty()){
			if (sourceSrc.toStr() != sourceDst.toStr()){
				mout.warn() << "what:source changed in creating combined volume:" << mout.endl;
				mout.warn() << "  dst: '" << sourceDst << "'" << mout.endl;
				mout.warn() << "  src: '" << sourceSrc << "'" << mout.endl;
			}
		}
		else {
			mout.warn() << "what:source empty, assuming: " << sourceDst << mout.endl;
		}
	}


	/// Common dataSetSelector for srcRoot and dstRoot
	DataSelector dataSetSelector;
	dataSetSelector.consumeParameters(ctx.select); //??
	dataSetSelector.pathMatcher = "dataset:"; // <fix
	//resources.select.clear();

	/// TIMESTAMP-based order ( Consider generalization for Carts)
	typedef std::map<std::string,ODIMPath> sweepMap;

	sweepMap srcPaths;
	dataSetSelector.getPaths(srcRoot, srcPaths);

	sweepMap dstPaths;
	dataSetSelector.getPaths(dstRoot, dstPaths);

	for (sweepMap::const_iterator it = dstPaths.begin(); it != dstPaths.end(); ++it){
		mout.info() << "exists: " << it->second <<  "\t (" << it->first << ')' << mout.endl;
	}


	mout.debug() << "traverse paths" << mout.endl;
	/// Traverse the child paths of srcRoot dataset[i]
	for (sweepMap::const_iterator it = srcPaths.begin(); it != srcPaths.end(); ++it){

		//const double   & elangle = it->first;
		const sweepMap::key_type & timestamp = it->first; // rename => key
		const ODIMPath & srcDataSetPath = it->second;

		Hi5Tree & srcDataSet = srcRoot(srcDataSetPath);  // clumsy, should be without leading '/'

		mout.debug() << " Considering " << srcDataSetPath <<  " (" << timestamp << ')' << mout.endl;

		sweepMap::const_iterator eit = dstPaths.find(timestamp);

		if (eit == dstPaths.end()){ // New timestamp, add this \c dataset directly.

			ODIMPathElem child(ODIMPathElem::DATASET);
			DataSelector::getNextChild(dstRoot, child);
			mout.note() << "New timestamp (" << timestamp << "), appending to path=" << child << mout.endl;
			// Create empty dstRoot[path] and swap it...
			Hi5Tree & dstDataSet = dstRoot[child];
			dstDataSet.swap(srcDataSet);
		}
		else { // timestamp is found in dstRoot.

			static const DataSelector dataSelector;

			const ODIMPath & dstDataSetPath = eit->second;

			mout.warn() << "Combining datasets of timestamp ("<< timestamp << "): src:" << srcDataSetPath <<  " => dst:" << dstDataSetPath << "("<< eit->first << ")" << mout.endl;

			Hi5Tree & dstDataSet = dstRoot(dstDataSetPath);

			//typedef std::map<std::string, ODIMPath> quantity_map;
			typedef std::map<std::string, ODIMPathElem> quantity_map;

			// CONFLICT!? Or ok with ODIMPathElem::DATA to use
			quantity_map srcQuantityPaths;
			DataSelector::getChildren(srcDataSet, srcQuantityPaths, ODIMPathElem::DATA);  // consider children

			quantity_map dstQuantityPaths;
			DataSelector::getChildren(dstDataSet, dstQuantityPaths, ODIMPathElem::DATA);

			/// Traverse quantities, assigning each to new / existing dstPaths in current structure.
			for (quantity_map::const_iterator qit = srcQuantityPaths.begin(); qit != srcQuantityPaths.end(); ++qit) {

				const std::string & quantity  = qit->first; // DBZH, or QIND or CLASS
				const ODIMPathElem & srcChild = qit->second;  //srcPath.back();
				Hi5Tree & srcData = srcDataSet[srcChild];

				quantity_map::const_iterator dit = dstQuantityPaths.find(quantity);
				if (dit == dstQuantityPaths.end()){
					//mout.debug() << "New quantity, adapt the full data group (with or without quality)" << mout.endl;
					ODIMPathElem dstChild(ODIMPathElem::DATA);
					DataSelector::getNextChild(dstDataSet, dstChild);
					mout.note() << "Add new quantity " <<  quantity << " => " << dstDataSetPath << '|' << dstChild << mout.endl;
					Hi5Tree & dstData = dstDataSet[dstChild];
					srcData.swap(dstData);
					//continue;
				}
				else {
					mout.note() << "Already [" <<  quantity << "] => " << dstDataSetPath << '|' << dit->second << ", updating its quality" << mout.endl;
					Hi5Tree & dstData = dstDataSet[dit->second];
					// DO not copy data, but quality
					updateQuality(srcData, dstData);
				}

			}

			// Update dataset quantities
			updateQuality(srcDataSet, dstDataSet);

		}


		// }

	}
}





void CmdInputFile::readTextFile(const std::string & fullFilename) const  {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getResources().mout( ;

	drain::Input ifstr(fullFilename);

	hi5::Hi5Base::readText(ctx.inputHi5, ifstr);
	DataTools::updateInternalAttributes(ctx.inputHi5);


}

void CmdInputFile::readImageFile(const std::string & fullFilename) const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = getResources().mout;

	/// Search last dataset
	ODIMPathElem dataSetElem(ODIMPathElem::DATASET);
	DataSelector::getLastChild(ctx.inputHi5, dataSetElem);
	if (dataSetElem.getIndex() == 0)
		dataSetElem.index = 1;

	/// Search new data[n] in the dataset found
	ODIMPathElem dataElem(ODIMPathElem::DATA);
	// TODO: append cmd?
	DataSelector::getLastChild(ctx.inputHi5[dataSetElem], dataElem);
	if (dataElem.getIndex() == 0)
		dataElem.index = 1;

	mout.debug() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	if (!ctx.inputHi5[dataSetElem][dataElem][ODIMPathElem::ARRAY].data.dataSet.isEmpty()){
		mout.debug() << "Path " << dataSetElem << '>' << dataElem << "/data contains data already, searching further..." << mout.endl;
		//DataSelector::getNextOrdinalPath(ctx.inputHi5, pathSearch, dataPath);
		++dataElem.index;
		mout.debug() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	}

	// Perhaps explicitly set already
	std::string object = ctx.inputHi5[ODIMPathElem::WHAT].data.attributes["object"];

	mout.warn() << ctx.inputHi5[ODIMPathElem::WHAT] << mout;

	Hi5Tree & dst = ctx.inputHi5[dataSetElem][dataElem];
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
	if (!obj.isEmpty()){
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
		object = ctx.inputHi5[ODIMPathElem::WHAT].data.attributes["object"].toStr();
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


	DataTools::updateInternalAttributes(ctx.inputHi5); // [dataSetElem] enough?
	mout.debug() << "props: " <<  dstImage.properties << mout.endl;

	if ((object == "COMP")|| (object == "IMAGE") ){
		mout.note() << "Cartesian product detected" << mout.endl;
		CartesianODIM odim; //(dstImage.properties);
		deriveImageODIM(dstImage, odim);  // generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, ctx.inputHi5[dataSetElem]); // $ odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, ctx.inputHi5); // $ odim.copyToRoot(ctx.inputHi5);
		mout.unimplemented() << "swap HDF5 for Cartesian data" << mout;
	}
	else {
		if ((object == "SCAN") || (object == "PVOL")) {
			mout.note() << "Polar object (" << object << ") detected" << mout;
		}
		else {
			ctx.inputHi5["what"].data.attributes["object"] = "SCAN";
			mout.warn() << "No what:object in metadata, assuming SCAN (Polar scan)" << mout;
		}
		PolarODIM odim;
		deriveImageODIM(dstImage, odim);   // TODO generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, ctx.inputHi5[dataSetElem]); // $odim.copyToDataSet(ctx.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, ctx.inputHi5); // $ odim.copyToRoot(ctx.inputHi5);
	}

	DataTools::updateInternalAttributes(ctx.inputHi5);


}


} // namespace rack



// Rack
