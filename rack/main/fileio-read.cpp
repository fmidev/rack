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


#include <drain/util/Log.h>
#include <drain/util/RegExp.h>

#include <drain/prog/Command.h>

#include <drain/prog/CommandRegistry.h>

#include "andre/QualityCombinerOp.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "data/ODIM.h"
#include "fileio.h"
#include "fileio-read.h"






namespace rack {


void CmdInputFile::exec() const {

	drain::Logger mout(name, ""); // __FUNCTION__

	//std::cerr << name << ':' << __FUNCTION__ << '\t' << fullFilename << std::endl;

	mout.timestamp("BEGIN_FILEREAD");
	mout.note() << "reading: '" << value << "'" << mout.endl;

	RackResources & resources = getResources();

	std::string fullFilename = resources.inputPrefix + value;

	resources.inputOk = true;

	//const CommandRegistry & r = drain::getRegistry();
	//mout.warn() << "lastCommand: '" << CommandRegistry::index << r.getLastCommand() << "'" << mout.endl;

	// Kludge. Check if last command was str than 1) file read or 2) odim assignment ( --/path:key=value )
	// inputComplete = (r.getLastCommand() != this->name) && (r.getLastCommand() != "CmdSetODIM");
	// mout.warn() << "inputComplete: " << (int)inputComplete << mout.endl;
	// mout.warn() << "autoExec:      " << (int)cmdAutoExec.exec << mout.endl;

	try {

		if (h5FileExtension.test(this->value)){
			readFileH5(fullFilename);
			//resources.setSource(*resources.currentHi5, *this); wronk
		}
		else if (pngFileExtension.test(this->value)){
			readImageFile(fullFilename);
		}
		else if (textFileExtension.test(this->value))
			readTextFile(fullFilename);
		else {
			mout.error() << "Unrecognizable as a filename: " << this->value << mout.endl;
		}

	}
	catch (std::exception & e) {
		resources.inputOk = false;
		mout.debug() << e.what() << mout.endl;
		if (resources.scriptParser.autoExec > 0){  // => go on with str inputs
			mout.warn() << "Read error, file=" << this->value << mout.endl;
		}
		else {
			mout.error() << "Read error, file=" << this->value << mout.endl;
			//exit(1);
		}
		return;
	}

	mout.debug(3) << "resources.getUpdatedStatusMap()" << mout.endl;
	resources.getUpdatedStatusMap();

	mout.timestamp("END_FILEREAD");


	if (resources.scriptParser.autoExec > 0){
		mout.info() << "auto execution..." << (int)resources.inputOk << mout.endl;
		if (resources.inputOk || (resources.scriptParser.autoExec > 1))
			resources.scriptExec.run();
	}


}



/// Reads hdf5 file and appends it to H5 structure. Works only with sweeps (SCAN), volume (PVOL) or Cartesian data (COMP) (having elevations).
void CmdInputFile::readFileH5(const std::string & fullFilename) const {  // TODO

	drain::Logger mout(name, ""); // __FUNCTION__
	mout.debug() << "start" << mout.endl;

	RackResources & resources = getResources();

	// InputSelect needed?
	HI5TREE srcTmp;
	hi5::Reader::readFile(fullFilename, srcTmp, resources.inputSelect); //, 0);  // 0 = read no attributes or datasets (yet)



	if (mout.isDebug(6)){
		mout.debug(2) << "input data:" << mout.endl;
		hi5::Hi5Base::writeText(srcTmp, std::cerr);
	}


	//DataTools::updateAttributes(srcTmp); // could be replaced, see below; only elangle needed at this point?
	//FlexVariableMap debugMap;
	DataTools::updateAttributes(srcTmp); // could be replaced, see below; only elangle needed at this point?
	//mout.warn() << "debugMap: " << debugMap << mout.endl;


	/// True, if user seems to provide
	const CommandRegistry & r = drain::getRegistry();
	const bool APPEND_INPUT = (r.getLastCommand() == this->name) || (r.getLastCommand() == "CmdSetODIM");
	const bool AUTO_EXEC    = (resources.scriptParser.autoExec > 0);

	mout.debug() << "Derive file type (what:object)" << mout.endl;
	//drain::VariableMap & what = srcTmp["what"].data.attributes;
	drain::Variable & object = srcTmp["what"].data.attributes["object"];
	if (object.isEmpty()){
		mout.warn() << "/what:object empty, assuming polar volume, 'PVOL'" << mout.endl;
		object = "PVOL";
	}

	if (object.toStr() == "COMP"){

		mout.info() << "Cartesian" << mout.endl;

		//DataTools::updateCoordinatePolicy(srcTmp, RackResources::limit);
		resources.setSource(resources.cartesianHi5, *this);
		resources.currentHi5 = & resources.cartesianHi5;

		// SINGLE_INPUT ||
		//if (AUTO_EXEC || ProductBase::appendResults.isUnset() || resources.cartesianHi5.isEmpty()){
		if (AUTO_EXEC || ProductBase::appendResults.isRoot() || resources.cartesianHi5.isEmpty()){
			resources.cartesianHi5.swap(srcTmp);
			//mout.note() << resources.cartesianHi5 << mout.endl;
		}
		else if (ProductBase::appendResults.isIndexed()){
			mout.note() << "Cartesian, append mode: " << ProductBase::appendResults << mout.endl;
			appendCartesianH5(srcTmp, resources.cartesianHi5);
		}
		else {
			mout.error() << "unsupported mode for ProductOp::appendResults=" << ProductBase::appendResults << mout.endl;
		}

		if (!resources.composite.isMethodSet()){
			//const std::string m = resources.cartesianHi5["how"].data.attributes["camethod"];
			const drain::Variable & m = resources.cartesianHi5["how"].data.attributes["camethod"];
			if (!m.isEmpty()){
				mout.info() << "adapting compositing method: " << m << mout.endl;
				resources.composite.setMethod(m.toStr());  // TODO: try-catch if invalid?
			}
			else {
				mout.note() << "no compositing method (how:camethod) in metadata of '" << value << "', consider --cMethod " << mout.endl;
			}

		}
		DataTools::updateAttributes(*resources.currentHi5);
	}
	else {

		mout.info() << "Polar product" << mout.endl;
		resources.setSource(resources.inputHi5, *this);
		resources.currentHi5 =      & resources.inputHi5;
		resources.currentPolarHi5 = & resources.inputHi5;
		//DataTools::updateCoordinatePolicy(srcTmp, RackResources::polarLeft);

		//if (ProductOp::appendResults.empty()){ // TODO ..or..
		//if ((!multipleInput) || (resources.scriptParser.autoExec>0)){ // TODO ..or..
		// SINGLE_INPUT ||
		if ((AUTO_EXEC || resources.inputHi5.isEmpty()) || !APPEND_INPUT){
			resources.inputHi5.swap(srcTmp);
		}
		else {
			appendPolarH5(srcTmp, resources.inputHi5);
		}

		DataTools::updateAttributes(*resources.currentHi5);
		DataTools::updateCoordinatePolicy(resources.inputHi5, RackResources::polarLeft);

			}


	//mout.warn() << resources.currentHi5->data.dataSet.properties << mout.endl;

	if (false){

		mout.warn() << "eka" << mout.endl;
		Data<PolarDst> eka((*resources.currentHi5)["dataset1"]["data2"]);
		mout.note() << eka << mout.endl;

		mout.warn() << "moka" << mout.endl;
		Data<PolarDst> moka(eka);
		mout.note() << moka << mout.endl;

		/*
		mout.warn() << "toka" << mout.endl;
		const DataSet<PolarSrc> konsta((*resources.currentHi5)["dataset1"], drain::RegExp("H"));  //REGEXP
		mout.note() << "konsta:\n" << konsta << mout.endl;
		const PlainData<PolarSrc> & aapeli = konsta.getData("TH");
		mout.warn() << aapeli << mout.endl;
		*/
		DataSet<PolarDst> muuttio((*resources.currentHi5)["dataset1"], drain::RegExp("Å"));
		mout.warn() << "pair" << mout.endl;
		DataSet<PolarDst>::value_type valu("KEPATH", eka);
		mout.warn() << "sertti" << mout.endl;
		muuttio.insert(valu);
		//moka.odim.quantity  = "XXXXX"; OK, but overridden by eka later
		eka.odim.offset = 12.3;


		/*
		DataSet<PolarDst> muuttio((*resources.currentHi5)["dataset1"]);
		Data<PolarDst> & uusi = muuttio.getData("SUURO");
		PlainData<PolarDst> & laatu = uusi.getQualityData("LAATU");
		laatu.initialize(typeid(unsigned short),123, 456);
		mout.warn() << uusi << mout.endl;
		mout.debug() << laatu << mout.endl;

		DataSet<PolarDst> tupla((*resources.currentHi5)["dataset1"]);
		Data<PolarDst> & rouhio = tupla.getData("SUURIMO");
		rouhio.initialize(typeid(unsigned char),123, 456);
		rouhio.odim.gain = 1.23456789;
		rouhio.odim.elangle = 12345.678;

		mout.warn() << rouhio << mout.endl;
		 */
	}

	mout.debug() << "end" << mout.endl;

}

void CmdInputFile::appendCartesianH5(HI5TREE & srcRoot, HI5TREE & dstRoot) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;
	if (ProductBase::appendResults.is(ODIMPathElem::DATASET)){
		attachCartesianH5(srcRoot, dstRoot);
	}
	else if (ProductBase::appendResults.isIndexed()){

		if (srcRoot.isEmpty())
			mout.warn() << " srcRoot empty" << mout.endl;

		if (srcRoot.getChildren().size() > 1)
			mout.note() << " srcRoot has several datasets" << mout.endl;


		ODIMPathElem parent(ODIMPathElem::DATASET);
		DataSelector::getLastChild(dstRoot, parent);
		if (parent.getIndex()==0)
			parent.index = 1;

		HI5TREE & dst = dstRoot[parent];

		for (HI5TREE::iterator it = srcRoot.begin(); it != srcRoot.end(); ++it){

			ODIMPathElem s(it->first); // possibly: what, where, how
			HI5TREE & src = it->second;

			if (s.isIndexed()){
				mout.note() << " appending " << s << " => " << parent << mout.endl;
				attachCartesianH5(src, dst);
			}
			else {
				mout.note() << " replacing " << it->first << mout.endl;
				dst[it->first].swap(it->second);   // overwrite what, where,
			}

		}
	}
	else {
		mout.warn() << " could not find path with append=" << ProductBase::appendResults << ", swapping anyway" << mout.endl;
		dstRoot.swap(srcRoot);
	}

}

void CmdInputFile::attachCartesianH5(HI5TREE & src, HI5TREE & dst) const {

	drain::Logger mout(name, __FUNCTION__);

	//ODIMPathElem p(g);
	//DataSelector::getLastChild(dst, p);

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){

		ODIMPathElem p(it->first); // possibly: what, where, how
		if (p.isIndexed()){
			//++p.index;
			//ODIMPathElem p(s);
			DataSelector::getNextChild(dst, p);
			mout.note() << " appending " << p << mout.endl;
			dst[p].swap(it->second);
		}
		else {
			mout.note() << " replacing " << it->first << mout.endl;
			dst[it->first].swap(it->second);   // overwrite what, where,
		}

		src[it->first].data.noSave = true;

		//else
		//	mout.warn() << " could not find path /dataset[N], result=" << p << mout.endl;
		//HI5TREE & dst
	}


/*
	RackResources & resources = getResources();

	std::list<ODIMPath> srcPaths;
	DataSelector srcSelector(ProductBase::appendResults+"[0-9]+$");
	srcSelector.setParameters(resources.select); //??
	resources.select.clear();

	const DataSelector dstSelector(ProductBase::appendResults+"[0-9]+$");


	 srcSelector.getPathsNEW(src, srcPaths); // RE2
	for (std::list<ODIMPath>::const_iterator it=srcPaths.begin(); it != srcPaths.end(); ++it){
		mout.warn() << "Appending from src:" << *it << mout.endl;
		std::string path = "dataset1";
		DataSelector::getNextOrdinalPath(dst, dstSelector, path);  //
		mout.warn() << "Appending to   dst:" << path << mout.endl;
		dst(path).swap(src(*it));
	}
	*/
}

void CmdInputFile::updateQuality(HI5TREE & src, HI5TREE & dst) const {

	drain::Logger mout(name, __FUNCTION__);

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
			mout.debug(2) << "Combining CLASS skipped (handled by QIND, if found)" << mout.endl;
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
				mout.warn() << "src QIND empty for  ["<< quantity << "] " << srcChild;
				mout << " (update under constr.)" << mout.endl;
				const PlainData<PolarSrc> srcProb(src[srcChild]);  // TODO: resources and quality code?
				QualityCombinerOp::updateOverallDetection(srcProb, dstQind, dstClass, quantity, (short unsigned int)123);
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

void CmdInputFile::appendPolarH5(HI5TREE & srcRoot, HI5TREE & dstRoot) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	RackResources & resources = getResources();

	/// Common dataSetSelector for srcRoot and dstRoot
	DataSelector dataSetSelector;
	dataSetSelector.setParameters(resources.select); //??
	resources.select.clear();

	// Consider generalization for Carts
	std::map<double,ODIMPath> srcPaths;
	dataSetSelector.getPathsNEW(srcRoot, srcPaths, ODIMPathElem::DATASET);

	std::map<double,ODIMPath> dstPaths;
	dataSetSelector.getPathsNEW(dstRoot, dstPaths, ODIMPathElem::DATASET); // RE2


	mout.debug() << "traverse paths" << mout.endl;
	/// Traverse the child paths of srcRoot dataset[i]
	for (std::map<double,ODIMPath>::const_iterator it = srcPaths.begin(); it != srcPaths.end(); ++it){

		const double   & elangle = it->first;
		const ODIMPath & srcDataSetPath = it->second;

		HI5TREE & srcDataSet = srcRoot(srcDataSetPath);  // clumsy, should be without leading '/'

		mout.debug() << " Considering " << srcDataSetPath <<  " (" << elangle << ')' << mout.endl;

		std::map<double,ODIMPath>::const_iterator eit = dstPaths.find(elangle);

		if (eit == dstPaths.end()){ // New elangle, add this \c dataset directly.

			ODIMPathElem child(ODIMPathElem::DATASET);
			DataSelector::getNextChild(dstRoot, child);
			mout.info() << "New elangle (" << elangle << "), appending to path=" << child << mout.endl;
			// Create empty dstRoot[path] and swap it...
			HI5TREE & dstDataSet = dstRoot[child];
			dstDataSet.swap(srcDataSet);
		}
		else { // elangle is found in dstRoot.

			static const DataSelector dataSelector;

			const ODIMPath & dstDataSetPath = eit->second;

			mout.note() << "Combining datasets of elevation ("<< elangle << "): src:" << srcDataSetPath <<  " => dst:" << dstDataSetPath << mout.endl;

			HI5TREE & dstDataSet = dstRoot(dstDataSetPath);

			//typedef std::map<std::string, ODIMPath> quantity_map;
			typedef std::map<std::string, ODIMPathElem> quantity_map;


			quantity_map srcQuantityPaths;
			DataSelector::getChildren(srcDataSet, srcQuantityPaths, ODIMPathElem::DATA);  // consider children

			quantity_map dstQuantityPaths;
			DataSelector::getChildren(dstDataSet, dstQuantityPaths, ODIMPathElem::DATA);

			/// Traverse quantities, assigning each to new / existing dstPaths in current structure.
			for (quantity_map::const_iterator qit = srcQuantityPaths.begin(); qit != srcQuantityPaths.end(); ++qit) {

				const std::string & quantity  = qit->first; // DBZH, or QIND or CLASS
				const ODIMPathElem & srcChild = qit->second;  //srcPath.back();
				HI5TREE & srcData = srcDataSet[srcChild];

				quantity_map::const_iterator dit = dstQuantityPaths.find(quantity);
				if (dit == dstQuantityPaths.end()){
					//mout.debug() << "New quantity, adapt the full data group (with or without quality)" << mout.endl;
					ODIMPathElem dstChild(ODIMPathElem::DATA);
					DataSelector::getNextChild(dstDataSet, dstChild);
					mout.note() << "Add new quantity " <<  quantity << " => " << dstDataSetPath << '|' << dstChild << mout.endl;
					HI5TREE & dstData = dstDataSet[dstChild];
					srcData.swap(dstData);
					//continue;
				}
				else {
					mout.note() << "Already [" <<  quantity << "] => " << dstDataSetPath << '|' << dit->second << ", updating its quality" << mout.endl;
					HI5TREE & dstData = dstDataSet[dit->second];
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

	drain::Logger mout(name, __FUNCTION__); // = getResources().mout;
	std::ifstream ifstr;
	ifstr.open(fullFilename.c_str());
	if (ifstr.fail()){
		mout.error() << "nyt feilaa" << mout.endl;
		ifstr.close();
		return;
	}
	hi5::Hi5Base::readText(getResources().inputHi5, ifstr);
	ifstr.close();
	DataTools::updateAttributes(getResources().inputHi5);
	// Hi5Base::writeText(getResources().inputHi5, std::cout);
	// mout.error() << "getResources().inputHi5" << mout.endl;
}

void CmdInputFile::readImageFile(const std::string & fullFilename) const {

	drain::Logger mout(name, __FUNCTION__); // = getResources().mout;

	RackResources & resources = getResources();

	/// Search last dataset
	ODIMPathElem dataSetElem(ODIMPathElem::DATASET);
	DataSelector::getLastChild(resources.inputHi5, dataSetElem);
	// std::string pathSearch = "dataset[0-9]+/?$";
	// std::string dataSetPath = "dataset1";
	// DataSelector::getLastOrdinalPath(resources.inputHi5, pathSearch, dataSetPath);

	/// Search new data[n] in the dataset found
	ODIMPathElem dataElem(ODIMPathElem::DATA);
	DataSelector::getLastChild(resources.inputHi5[dataSetElem], dataElem);
	//pathSearch = dataSetPath+"/data[0-9]+/?$";
	//std::string dataPath = dataSetPath + "/data1"; // needed for recognition
	//DataSelector::getLastOrdinalPath(resources.inputHi5, pathSearch, dataPath);


	mout.warn() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	if (!resources.inputHi5[dataSetElem][dataElem]["data"].data.dataSet.isEmpty()){
		mout.debug() << "Path " << dataSetElem << '>' << dataElem << "/data contains data already, searching further..." << mout.endl;
		//DataSelector::getNextOrdinalPath(resources.inputHi5, pathSearch, dataPath);
		++dataElem.index;
	}
	mout.warn() << "Found path " << dataSetElem << '>' << dataElem << mout.endl;
	// mout.debug() << "Final path " << dataPath << mout.endl;


	HI5TREE & dst = resources.inputHi5[dataSetElem][dataElem];
	drain::image::Image & dstImage = dst["data"].data.dataSet;
	drain::image::File::read(dstImage, fullFilename);
	//const drain::image::Geometry & g = dstImage.getGeometry();

	drain::VariableMap & rootAttributes = resources.inputHi5["what"].data.attributes;
	drain::Variable & object = rootAttributes["object"];
	if (object.isEmpty()){
		mout.note() << "Assuming polar data" << mout.endl;
		object = "PVOL";
	}

	DataTools::updateAttributes(resources.inputHi5); // [dataSetElem] enough?
	mout.debug() << "props: " <<  dstImage.properties << mout.endl;

	if (object.toStr()=="COMP"){
		CartesianODIM odim; //(dstImage.properties);
		deriveImageODIM(dstImage, odim);  // generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, resources.inputHi5[dataSetElem]); // $ odim.copyToDataSet(resources.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, resources.inputHi5); // $ odim.copyToRoot(resources.inputHi5);
	}
	else {
		PolarODIM odim;
		deriveImageODIM(dstImage, odim);   // TODO generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIMPathElem::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIMPathElem::DATASET>(odim, resources.inputHi5[dataSetElem]); // $odim.copyToDataSet(resources.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIMPathElem::ROOT>(odim, resources.inputHi5); // $ odim.copyToRoot(resources.inputHi5);
	}

	DataTools::updateAttributes(resources.inputHi5);


}


} // namespace rack



// Rack
