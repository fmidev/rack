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

	// Kludge. Check if last command was other than 1) file read or 2) odim assignment ( --/path:key=value )
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
		if (resources.scriptParser.autoExec > 0){  // => go on with other inputs
			mout.warn() << "Read error, file=" << this->value << mout.endl;
		}
		else {
			mout.error() << "Read error, file=" << this->value << mout.endl;
			//exit(1);
		}
		return;
	}

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

	if (mout.isDebug(2)){
		mout.debug() << "input data:" << mout.endl;
		hi5::Hi5Base::writeText(srcTmp, std::cerr);
	}

	DataTools::updateAttributes(srcTmp); // could be replaced, see below; only elangle needed at this point?



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
		if (AUTO_EXEC || ProductBase::appendResults.isUnset() || resources.cartesianHi5.isEmpty()){
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
		//DataTools::updateAttributes(*resources.currentHi5, ODIM().getCoordinatePolicy());
	}
	else {

		mout.info() << "Polar product" << mout.endl;
		resources.setSource(resources.inputHi5, *this);
		resources.currentHi5 =      & resources.inputHi5;
		resources.currentPolarHi5 = & resources.inputHi5;
		DataTools::updateCoordinatePolicy(srcTmp, RackResources::polarLeft);

		//if (ProductOp::appendResults.empty()){ // TODO ..or..
		//if ((!multipleInput) || (resources.scriptParser.autoExec>0)){ // TODO ..or..
		// SINGLE_INPUT ||
		if ((AUTO_EXEC || resources.inputHi5.isEmpty()) || !APPEND_INPUT){
			resources.inputHi5.swap(srcTmp);
		}
		else {
			appendPolarH5(srcTmp, resources.inputHi5);
		}
		//DataTools::updateAttributes(*resources.currentHi5, PolarODIM().getCoordinatePolicy());
	}

	DataTools::updateAttributes(*resources.currentHi5);
	//mout.warn() << resources.currentHi5->data.dataSet.properties << mout.endl;
}

void CmdInputFile::appendCartesianH5(HI5TREE & srcRoot, HI5TREE & dstRoot) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;
	if (ProductBase::appendResults.is(BaseODIM::DATASET)){
		attachCartesianH5(srcRoot, dstRoot);
	}
	else if (ProductBase::appendResults.isIndexed()){

		if (srcRoot.isEmpty())
			mout.warn() << " srcRoot empty" << mout.endl;

		if (srcRoot.getChildren().size() > 1)
			mout.note() << " srcRoot has several datasets" << mout.endl;


		ODIMPathElem parent(BaseODIM::DATASET);
		DataTools::getLastChild(dstRoot, parent);
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
	//DataTools::getLastChild(dst, p);

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){

		ODIMPathElem p(it->first); // possibly: what, where, how
		if (p.isIndexed()){
			//++p.index;
			//ODIMPathElem p(s);
			DataTools::getNextChild(dst, p);
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

	std::list<std::string> srcPaths;
	DataSelector srcSelector(ProductBase::appendResults+"[0-9]+$");
	srcSelector.setParameters(resources.select); //??
	resources.select.clear();

	const DataSelector dstSelector(ProductBase::appendResults+"[0-9]+$");


	DataSelector::getPaths(src, srcSelector, srcPaths);
	for (std::list<std::string>::const_iterator it=srcPaths.begin(); it != srcPaths.end(); ++it){
		mout.warn() << "Appending from src:" << *it << mout.endl;
		std::string path = "dataset1";
		DataSelector::getNextOrdinalPath(dst, dstSelector, path);  //
		mout.warn() << "Appending to   dst:" << path << mout.endl;
		dst(path).swap(src(*it));
	}
	*/
}


void CmdInputFile::appendPolarH5(HI5TREE & srcRoot, HI5TREE & dstRoot) const {

	drain::Logger mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	// Data or quality data
	// static const drain::RegExp dataTypeMatcher("^/?((data[0-9]+)/)?((data|quality)[0-9]+)/?$");
	//static const drain::RegExp dataTypeMatcher("^/?((data[0-9]+)/)?((data|quality)[0-9]+)/?$");
	//static const drain::RegExp dataTypeMatcher(".*(data|dataset)([0-9]+)/(data|quality)([0-9]+)/?$");
	// static const drain::RegExp dataTypeMatcher(".*((data|dataset)[0-9]+)/((data|quality)[0-9]+)/?$");
	static const drain::RegExp dataTypeMatcher("^(|(/data[0-9]+))/((data|quality)[0-9]+)/?$");
	static const DataSelector dataSelector("(data|quality)[0-9]+/?$"); // = above std::string dataTypeMatcher.regExpString

	// Raise selection?
	RackResources & resources = getResources();

	/// Common dataSetSelector for srcRoot and dstRoot
	DataSelector dataSetSelector("dataset[0-9]+/?$");
	dataSetSelector.setParameters(resources.select); //??
	resources.select.clear();
	/*  In future, dataselector could/should handle:
	 *  dataset=1-2,4
	 *  data=1-2
	 *  quality=1-2
	 */


	std::list<std::string> srcPaths;
	DataSelector::getPaths(srcRoot, dataSetSelector, srcPaths);

	std::map<double,std::string> dstPaths;
	DataSelector::getPathsByElevation(dstRoot, dataSetSelector, dstPaths);


	mout.debug() << "traverse paths" << mout.endl;
	/// Traverse the child paths of srcRoot dataset[i]
	for (std::list<std::string>::const_iterator it = srcPaths.begin(); it != srcPaths.end(); ++it){

		const std::string & srcDataSetPath = *it;

		HI5TREE & srcDataSet = srcRoot(srcDataSetPath);  // clumsy, should be without leading '/'

		const VariableMap & where = srcDataSet["where"].data.attributes; // (it->second)["where"].data.attributes;
		//	if (where.hasKey("elangle"));
		const double elangle = where["elangle"];

		mout.debug() << " Considering " << srcDataSetPath <<  " (" << elangle << ')' << mout.endl;

		std::map<double,std::string>::const_iterator eit = dstPaths.find(elangle);

		if (eit == dstPaths.end()){ // Elevation angle (elangle) of this input dataset is not found in dstRoot, so add it.

			ODIMPathElem path(BaseODIM::DATASET);
			DataTools::getNextChild(dstRoot, path);
			mout.note() << "Appending to path=" << path << mout.endl;
			// Creates dstRoot[path]
			//dstRoot[path].getChildren().swap( srcDataSet.getChildren() );
			HI5TREE & dstDataSet = dstRoot[path];
			dstDataSet.swap(srcDataSet);
		}
		else { // elangle is found in dstRoot.

			const std::string & dstDataSetPath = eit->second;

			mout.note() << " Combining datasets with same elevation ("<< elangle << "): src:" << srcDataSetPath <<  " <--> dst:" << dstDataSetPath << mout.endl;

			HI5TREE & dstDataSet = dstRoot(dstDataSetPath);

			std::map<std::string, std::string> srcQuantityPaths;
			DataSelector::getPathsByQuantity(srcDataSet, dataSelector, srcQuantityPaths);

			std::map<std::string, std::string> dstQuantityPaths;
			DataSelector::getPathsByQuantity(dstDataSet, dataSelector, dstQuantityPaths);

			/// Traverse quantities, assigning each to new / existing dstPaths in current structure.
			for (std::map<std::string,std::string>::const_iterator qit = srcQuantityPaths.begin(); qit != srcQuantityPaths.end(); ++qit) {

				const std::string & quantity = qit->first; // DBZH, or even QIND or CLASS
				const std::string & srcPath  = qit->second;
				mout.debug() << " -> " << srcPath <<  " [" << quantity << ']' << mout.endl;


				ODIMPath opath(srcPath);
				mout.debug() << "ODIMPath: " << opath << " size=" << opath.size() << mout.endl;

				ODIMPathElem child = opath.back();

				opath.pop_back();
				ODIMPathElem parent = opath.back();

				mout.debug() << "ODIMPath: child='" << child  << "', parent='" << parent << "'" << mout.endl;

				const bool QUALITY = (child.group == BaseODIM::QUALITY);
				const bool LOCAL   = (parent.group != BaseODIM::ROOT); // not true root, but empty /dataset1//quality1


				//mout.debug() << "Handling " << srcPath << " => " << srcPathParent << " / "  << srcPathChild << mout.endl;
				mout.debug() << "Handling src:" << srcPath << " => dst:" << parent << " / "  << child << mout.endl;
				mout.debug(1) << "LOCAL=" << (int)LOCAL << ", QUALITY=" << (int)QUALITY << mout.endl;

				if (QUALITY){

					std::string dataQuantity;  // DBZH, VRAD etc.

					if (LOCAL){
						mout.debug() << "checking " << parent << "/what:quantity" << mout.endl;
						dataQuantity = srcDataSet[parent]["what"].data.attributes["quantity"].toStr();
						//mout.warn() << "data quantity: " << dataQuantity << mout.endl;
					}
					else {
						mout.info() << "updating GLOBAL quality: " << quantity << mout.endl;

					}
					// NOW, dataQuantity is empty, unless local
					//mout.warn() << "data quantityD: " << dataQuantity << mout.endl;
					//mout.info() << "retrieve common data: " << quantity << mout.endl;

					DataSet<PolarSrc> sDataSet(srcDataSet);
					PlainData<PolarSrc> & srcQind  = sDataSet.getQualityData2("QIND",  dataQuantity);
					PlainData<PolarSrc> & srcClass = sDataSet.getQualityData2("CLASS", dataQuantity); // Maybe empty

					DataSet<PolarDst> dDataSet(dstDataSet);
					PlainData<PolarDst> & dstQind  = dDataSet.getQualityData2("QIND",  dataQuantity);
					PlainData<PolarDst> & dstClass = dDataSet.getQualityData2("CLASS", dataQuantity); // Maybe empty (but created if srcClass non-empty).

					if (quantity == "QIND"){  // append (combine) dataset-level qind
						mout.note() << "updating QIND and CLASS" << mout.endl;

						// Todo: if no CLASS
						if (srcClass.data.isEmpty()){
							mout.warn() << "CLASS empty srcPath=" << srcPath << mout.endl;
						}
						QualityCombinerOp::updateOverallQuality(srcQind, srcClass,	dstQind, dstClass);

						const drain::VariableMap & srcHow   = srcQind.tree["how"].data.attributes;
						const std::string srcArgs = srcHow["task_args"];

						drain::VariableMap & dstHow   = dstQind.tree["how"].data.attributes;
						dstHow["task_args"] << srcArgs;
						continue;

					}
					else if (quantity == "CLASS"){
						// Handled with "QIND", right above.
						mout.debug(1) << "combining CLASS skipped (handled by QIND, if found)" << mout.endl;
						continue;
					}
					else {
						mout.note() << "updating Q quantity="<< quantity << "  " << srcDataSetPath << ':' << srcPath << " dstDataSetPath => " << dstDataSetPath << mout.endl;
						if (srcClass.data.isEmpty()){
							PlainData<PolarSrc> & srcProb = sDataSet.getQualityData(quantity);
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
						// continues below
						if (LOCAL){
							mout.note() << "LOCAL (specific) Q data" << quantity << ", for quantity=" << dataQuantity << mout.endl;
							Data<PolarSrc> & s         = sDataSet.getData(dataQuantity);
							PlainData<PolarSrc> & qSrc = s.getQualityData(quantity);

							Data<PolarDst> & d         = dDataSet.getData(dataQuantity);
							PlainData<PolarDst> & qDst = d.getQualityData(quantity);
							qDst.odim.updateFromMap(qSrc.odim);
							qDst.data.copyDeep(qSrc.data); // kludge
							qDst.updateTree2();
							continue;
						}
					}

				}

				// Substantial quantity or quality other than QIND or CLASS
				std::map<std::string,std::string>::const_iterator dit;
				/*
				if (QUALITY && LOCAL)
					dit = dstQuantityPaths.find(dataQuantity);
				else
				*/
					dit = dstQuantityPaths.find(quantity);

				if (dit == dstQuantityPaths.end()){ // Not found => add (append)

					ODIMPathElem dstParentPath(parent.group);
					if (BaseODIM::isIndexed(parent.group)) // != BaseODIM::ROOT , practically
						DataTools::getNextChild(dstDataSet, dstParentPath);

					ODIMPathElem dstChildPath(child.group); // BaseODIM::DATA or BaseODIM::QUALITY

					if (QUALITY){
						mout.debug() << "Adding Q quantity " << '[' << quantity << "]" << mout.endl;
						mout.debug(1) << "Initial target path " << dstParentPath << " / " << dstChildPath << mout.endl;
					}
					else {
						mout.debug() << "Adding quantity " << '[' << quantity << "]" << mout.endl;
					}

					if (DataTools::getNextChild(dstDataSet, dstChildPath)){
						//++dstChildPath.index;
						mout.debug() << "Target path " << dstParentPath << " / " << dstChildPath << mout.endl;
						//mout.note() << "NEW: Target path2: " << dstChildPath << mout.endl;
					}
					else {
						mout.error() << "could not find path for dst=" << dstParentPath << '|' << dstChildPath << "?" << mout.endl;
					}

					dstDataSet[dstParentPath][dstChildPath].swap(srcDataSet(srcPath));

					srcDataSet.erase(srcPath); // otherwise empty /data6 Groups remains
					//mout.note() << "Erased" << mout.endl;
				}
				else {
					// Problem: original LOCAL quality data would be swapped...
					mout.note() << "Not overwriting: src: " << srcDataSetPath << '/' << srcPath << '(' << elangle << ')' << '[' << quantity <<  "] -> dst:" << dstDataSetPath << '/' << dit->second<< mout.endl;
				}
			}

			// for (std::map<std::string,std::string>::const_iterator it = dstQuantityPaths.begin(); it != dstQuantityPaths.end(); ++it)
			// std::cout << '\t' << it->first << '\t' << it->second << std::endl;
			continue;  // IMPORTANT
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
	std::string pathSearch = "dataset[0-9]+/?$";
	std::string dataSetPath = "dataset1";
	DataSelector::getLastOrdinalPath(resources.inputHi5, pathSearch, dataSetPath);

	/// Search new data[n] in the dataset found
	pathSearch = dataSetPath+"/data[0-9]+/?$";
	std::string dataPath = dataSetPath + "/data1"; // needed for recognition
	DataSelector::getLastOrdinalPath(resources.inputHi5, pathSearch, dataPath);

	mout.debug() << "Found path " << dataPath << mout.endl;
	if (!resources.inputHi5(dataPath)["data"].data.dataSet.isEmpty()){
		mout.debug() << "Path " << dataPath << "/data contains data already, searching further..." << mout.endl;
		DataSelector::getNextOrdinalPath(resources.inputHi5, pathSearch, dataPath);
	}
	mout.debug() << "Final path " << dataPath << mout.endl;


	HI5TREE & dst = resources.inputHi5(dataPath);
	drain::image::Image & dstImage = dst["data"].data.dataSet;
	drain::image::File::read(dstImage, fullFilename);
	//const drain::image::Geometry & g = dstImage.getGeometry();

	drain::VariableMap & rootAttributes = resources.inputHi5["what"].data.attributes;
	drain::Variable & object = rootAttributes["object"];
	if (object.isEmpty()){
		mout.note() << "Assuming polar data" << mout.endl;
		object = "PVOL";
	}

	DataTools::updateAttributes(resources.inputHi5);
	mout.debug() << "props: " <<  dstImage.properties << mout.endl;

	if (object.toStr()=="COMP"){
		CartesianODIM odim; //(dstImage.properties);
		deriveImageODIM(dstImage, odim);  // generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIM::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIM::DATASET>(odim, resources.inputHi5(dataSetPath)); // $ odim.copyToDataSet(resources.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIM::ROOT>(odim, resources.inputHi5); // $ odim.copyToRoot(resources.inputHi5);
	}
	else {
		PolarODIM odim;
		deriveImageODIM(dstImage, odim);   // TODO generalize in ODIM.h (or obsolete already)
		ODIM::copyToH5<ODIM::DATA>(odim, dst); // $ odim.copyToData(dst);
		ODIM::copyToH5<ODIM::DATASET>(odim, resources.inputHi5(dataSetPath)); // $odim.copyToDataSet(resources.inputHi5(dataSetPath));
		ODIM::copyToH5<ODIM::ROOT>(odim, resources.inputHi5); // $ odim.copyToRoot(resources.inputHi5);
	}

	DataTools::updateAttributes(resources.inputHi5);


}


} // namespace rack



// Rack
