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


#include <drain/util/Debug.h>
//#include <drain/util/Histogram.h>
#include <drain/util/RegExp.h>

#include <drain/prog/Command.h>

/*
#include <drain/image/Image.h>
#include <drain/image/Sampler.h>
*/

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/Commands-ImageTools.h>

#include "andre/QualityCombinerOp.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Read.h"
#include "data/ODIM.h"
//#include "product/DataConversionOp.h"

//#include "commands.h"

#include "fileio.h"
#include "fileio-read.h"






namespace rack {


void CmdInputFile::exec() const {

	drain::MonitorSource mout(name, __FUNCTION__); // getResources().mout;

	//std::cerr << name << ':' << __FUNCTION__ << '\t' << fullFilename << std::endl;

	mout.timestamp("BEGIN_FILEREAD");
	mout.note() << "reading: '" << value << "'" << mout.endl;

	RackResources & resources = getResources();

	std::string fullFilename = resources.inputPrefix + value;

	resources.inputOk = true;

	const CommandRegistry & r = drain::getRegistry();
	//mout.warn() << "lastCommand: '" << CommandRegistry::index << r.getLastCommand() << "'" << mout.endl;

	// Kludge. Check if last command was other than 1) file read or 2) odim assignment ( --/path:key=value )
	inputComplete = (r.getLastCommand() != this->name) && (r.getLastCommand() != "CmdSetODIM");
	// mout.warn() << "inputComplete: " << (int)inputComplete << mout.endl;
	// mout.warn() << "autoExec:      " << (int)cmdAutoExec.exec << mout.endl;

	try {

		if (h5FileExtension.test(this->value)){
			if (inputComplete || (resources.scriptParser.autoExec>0)){
				readFileH5(fullFilename);
			}
			else {
				readFileAndAppendH5(fullFilename);
			}
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

	Variable & v = resources.inputHi5["what"].data.attributes["object"];
	if (!v.typeIsSet())
		v.setType<std::string>();
	const std::string object = v;
	//const std::string object = resources.inputHi5["what"].data.attributes["object"];

	if (object == "COMP"){

		mout.info() << "type: Cartesian product" << mout.endl;

		resources.setSource(resources.cartesianHi5, *this);

		resources.cartesianHi5.clear();
		resources.cartesianHi5.getChildren().swap(resources.inputHi5.getChildren());

		RackResources::updateCoordinatePolicy(resources.cartesianHi5, RackResources::limit);
		resources.currentHi5 = & resources.cartesianHi5;
		resources.currentPolarHi5 = NULL;
		//std::cerr << *resources.currentHi5 << std::endl;
		if (!resources.composite.isMethodSet()){
			const std::string m = resources.cartesianHi5["how"].data.attributes["camethod"];
			if (!m.empty()){
				mout.info() << "adapting compositing method: " << m << mout.endl;
				//const AccumulationMethod & method = getResources().composite.getMethod(m);
				resources.composite.setMethod(m);  // TODO: try-catch if invalid?
			}
			else {
				mout.note() << "no compositing method (how:camethod) in metadata of '" << value << "', consider --cMethod " << mout.endl;
			}

		}

	}
	else {
		mout.info() << "type: Polar data/product" << mout.endl;
		resources.currentHi5 = & resources.inputHi5;
		resources.currentPolarHi5 = & resources.inputHi5;
		//RackResources::updateCoordinatePolicy(resources.polarHi5, RackResources::polarLeft);
		RackResources::updateCoordinatePolicy(*resources.currentPolarHi5, RackResources::polarLeft);
	}

	mout.timestamp("END_FILEREAD");

	resources.getUpdatedStatusMap();

	if (resources.scriptParser.autoExec > 0){
		mout.info() << "auto execution..." << (int)resources.inputOk << mout.endl;
		if (resources.inputOk || (resources.scriptParser.autoExec > 1))
			resources.scriptExec.run();
	}

}

/// Reads hdf5 file
void CmdInputFile::readFileH5(const std::string & fullFilename) const {
	// drain::MonitorSource & mout = getResources().mout;
	drain::MonitorSource mout(name, __FUNCTION__);
	RackResources & resources = getResources();
	resources.setSource(resources.inputHi5, *this);
	hi5::Reader::readFile(fullFilename, resources.inputHi5, resources.inputSelect);
	DataSelector::updateAttributes(resources.inputHi5);
	mout.debug() << "completed" << mout.endl;
}



/// Reads hdf5 file and appends it to H5 structure. Works only with volume or sweep data (having elevations).
void CmdInputFile::readFileAndAppendH5(const std::string & fullFilename) const {  // TODO

	drain::MonitorSource mout(name, __FUNCTION__);
	mout.debug() << "start" << mout.endl;

	RackResources & resources = getResources();

	HI5TREE src;
	hi5::Reader::readFile(fullFilename, src, resources.inputSelect); //, 0);  // 0 = read no attributes or datasets (yet)

	if (mout.isDebug(2)){
		mout.debug() << "input data:" << mout.endl;
		//src.writeText(std::cerr);
		hi5::Hi5Base::writeText(src, std::cerr);
	}

	DataSelector::updateAttributes(src); // could be replaced, see below; only elangle needed at this point?


	/// Common dataSetSelector for src and dst
	DataSelector dataSetSelector;
	dataSetSelector.setParameters(resources.select); //??
	resources.select.clear();
	//dataSetSelector.path = "/(data|quality)[0-9]+/?$";
	dataSetSelector.path = "dataset[0-9]+/?$";

	std::map<double,std::string> dstPaths;
	DataSelector::getPathsByElevation(resources.inputHi5, dataSetSelector, dstPaths);

	std::list<std::string> srcPaths;
	DataSelector::getPaths(src, dataSetSelector, srcPaths);



	//const drain::RegExp dataTypeMatcher("^/?(data[0-9]/)?(data|quality)[0-9]+/?$");
	const drain::RegExp dataTypeMatcher("^/?((data[0-9]+)/)?((data|quality)[0-9]+)/?$");
	static const DataSelector dataSelector("(data|quality)[0-9]+/?$"); // = above std::string dataTypeMatcher.regExpString

	mout.debug() << "traverse paths" << mout.endl;
	/// Traverse the child paths of src dataset[i]
	for (std::list<std::string>::const_iterator it = srcPaths.begin(); it != srcPaths.end(); ++it){

		const std::string & srcDataSetPath = *it;

		HI5TREE & srcDataSet = src(srcDataSetPath);

		const VariableMap & where = srcDataSet["where"].data.attributes; // (it->second)["where"].data.attributes;

		//if (true){
		//std::string dstDataSetPath; //

		if (where.hasKey("elangle")){

			resources.setSource(resources.inputHi5, *this);

			const double elangle = where["elangle"];
			//mout.warn() << " Considering " << srcDataSetPath <<  " (" << elangle << ')' << mout.endl;
			std::map<double,std::string>::const_iterator eit = dstPaths.find(elangle);

			if (eit != dstPaths.end()){ // elangle is found in dst.
				//dstDataSetPath = eit->second; // TODO
				const std::string & dstDataSetPath = eit->second;
				//}
				//else dstDataSetPath = "dataset2";
				//const std::string & dstDataSetPath = eit->second;
				HI5TREE & dst = resources.inputHi5(dstDataSetPath);

				// TODO d and q separately
				//mout.warn() << " adding" << it->first << '(' << elangle << ')' << " to " << dstDataSetPath << mout.endl;

				std::map<std::string, std::string> dstDataPaths;
				DataSelector::getPathsByQuantity(dst, dataSelector, dstDataPaths);

				std::map<std::string, std::string> srcDataPaths;
				DataSelector::getPathsByQuantity(srcDataSet, dataSelector, srcDataPaths);

				/// Traverse quantities, assigning each to new / existing dstPaths in current structure.
				for (std::map<std::string,std::string>::const_iterator qit = srcDataPaths.begin(); qit != srcDataPaths.end(); ++qit) {

					const std::string & quantity = qit->first;
					const std::string & srcPath  = qit->second;

					if (dataTypeMatcher.execute(srcPath) != 0){
						mout.warn() << "non-standard path name, skipping: " << srcPath << mout.endl;
						continue;
					}


					const std::string & srcPathParent = dataTypeMatcher.result.at(2); // "data<i>" or empty
					const std::string & srcPathChild  = dataTypeMatcher.result.at(3);  // "data<i>" or "quality<i>"
					const std::string & srcDataType   = dataTypeMatcher.result.at(4);  // "data"  or "quality"

					const bool QUALITY = (srcDataType=="quality");
					const bool LOCAL   = !srcPathParent.empty();

					mout.debug() << "Handling " << srcPath << " => " << srcPathParent << " / "  << srcPathChild << mout.endl;
					mout.debug(1) << "LOCAL=" << (int)LOCAL << ", QUALITY=" << (int)QUALITY << mout.endl;

					if (QUALITY){

						std::string dataQuantity;  // DBZH, VRAD etc.

						if (LOCAL){
							std::string dataQuantity = srcDataSet[srcPathParent]["what"].data.attributes["quantity"];
							mout.info() << "updating LOCAL quality: " << srcPath  << '[' << quantity << "]" << ", main quantity: " << srcPathParent  << '[' << dataQuantity << "]" << mout.endl;
						}
						else {
							mout.info() << "updating GLOBAL quality " << quantity << mout.endl;
						}

						DataSetSrc<PolarSrc> sDataSet(srcDataSet);
						DataSetDst<PolarDst> dDataSet(dst);

						mout.info() << "retrieve common data" << quantity << mout.endl;

						// Rautalankaa...
						PlainData<PolarSrc> & srcQind  = sDataSet.getQualityData("QIND",  dataQuantity);
						PlainData<PolarSrc> & srcClass = sDataSet.getQualityData("CLASS", dataQuantity); // Maybe empty

						PlainData<PolarDst> & dstQind  = dDataSet.getQualityData("QIND",  dataQuantity);
						PlainData<PolarDst> & dstClass = dDataSet.getQualityData("CLASS", dataQuantity); // Maybe empty (but created if srcClass non-empty).

						if (quantity == "QIND"){  // append (combine) dataset-level qind
							mout.note() << "updating QIND and CLASS" << mout.endl;

							// Todo: if no CLASS
							QualityCombinerOp::updateOverallQuality(srcQind, srcClass,	dstQind, dstClass);
							const drain::VariableMap & srcHow   = srcQind.tree["how"].data.attributes;
							const std::string srcArgs = srcHow["task_args"];

							drain::VariableMap & dstHow   = dstQind.tree["how"].data.attributes;
							dstHow["task_args"] << srcArgs;
							continue;

						}
						else if (quantity == "CLASS"){
							// mout.debug(1) << "combining CLASS skipped (handled by QIND)" << mout.endl;
							// Handled with "QIND", right above.
							continue;
						}
						else {
							mout.note() << "updating quality quantity="<< quantity << "  " << srcDataSetPath << ':' << srcPath << " dstDataSetPath => " << dstDataSetPath << mout.endl;
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
								mout.note() << "Found quality data (CLASS), updates done on that, not on orig quality=" << quantity << mout.endl;
							}
							// dst[dit->second].getChildren().swap( srcDataSet[srcPath].getChildren() );
							//continue;
						}

					}

					// subst. quantity or quality other than QIND or CLASS
					std::map<std::string,std::string>::const_iterator dit = dstDataPaths.find(quantity);
					if (dit == dstDataPaths.end()){ // Not found => add (append)
						// Always data, but quality included for now...
						std::string p = QUALITY ? "quality1" : "data1";
						const std::string regexp =  QUALITY ? (LOCAL ? "data[0-9]+/quality[0-9]+$" : "^/?quality[0-9]+$") : "data[0-9]+$";  // data[0-9]+$  or  quality[0-9]+$
						//p = p+"1";  // data1 or quality1
						DataSelector::getNextOrdinalPath(dst, regexp, p);  //
						mout.note() << "Adding src:" << srcDataSetPath << srcPath << '(' << elangle << ')' << '[' << quantity << "] -> dst:" << dstDataSetPath << '/' << p << mout.endl;
						// Creates dst[p]
						dst(p).getChildren().swap( srcDataSet(srcPath).getChildren() );
						//if (!QUALITY)
						srcDataSet.erase(srcPath); // otherwise empty /data6 Groups remains
						//mout.note() << "Erased" << mout.endl;
					}
					else {
						// Problem: original LOCAL quality data would be swapped...
						mout.note() << "Not overwriting: src: " << srcDataSetPath << '/' << srcPath << '(' << elangle << ')' << '[' << quantity <<  "] -> dst:" << dstDataSetPath << '/' << dit->second<< mout.endl;
						/*
								std::list<std::string> qualityPaths;
								l.push_back("data");
								l.push_back("what");
								l.push_back("where");
						 */
						//dst[dit->second].getChildren().swap( srcDataSet[srcPath].getChildren() );
					}
					//}
				}

				// for (std::map<std::string,std::string>::const_iterator it = dstDataPaths.begin(); it != dstDataPaths.end(); ++it)
				// std::cout << '\t' << it->first << '\t' << it->second << std::endl;
				continue;  // IMPORTANT
			}
			else {
				std::string path = "dataset1";
				DataSelector::getNextOrdinalPath(resources.inputHi5, "dataset[0-9]+/?$", path);  //
				//mout.note() << " Appending elangle=" << elangle << " to path=" << path << mout.endl;
				mout.note() << "Appending to path=" << path << mout.endl;
				// Creates dst[p]
				resources.inputHi5[path].getChildren().swap( srcDataSet.getChildren() );
			}


		}
		else {
			// TODO: refine with
			//ProductOp::appendResults
			mout.note() << "Input file contains no elangle, assuming Cartesian data" << mout.endl;
			/// resources.inputHi5

			resources.setSource(resources.cartesianHi5, *this);

			if (ProductOp::appendResults){ // default append


				std::string path = "dataset1";

				DataSelector::getNextOrdinalPath(resources.inputHi5, "dataset[0-9]+/?$", path);  //
				mout.note() << "Appending to: " << path << mout.endl;

				// Creates inputHi5[path]
				resources.inputHi5[path].getChildren().swap( srcDataSet.getChildren() );

			}
			else {

				resources.inputHi5.getChildren().swap(resources.cartesianHi5.getChildren());
				resources.cartesianHi5.clear();

				std::map<std::string, std::string> srcDataPaths;
				DataSelector::getPathsByQuantity(srcDataSet, dataSelector, srcDataPaths);

				//HI5TREE & dst = resources.inputHi5["dataset1"];
				//std::map<std::string, std::string> dstDataPaths;
				//DataSelector::getPathsByQuantity(dst, dataSelector, dstDataPaths);

				for (std::map<std::string,std::string>::const_iterator qit = srcDataPaths.begin(); qit != srcDataPaths.end(); ++qit) {
					//mout.note() << "Reading " << qit->first << '=' << qit->second << mout.endl;
					// Kludge "function". Also above
					if (qit->second.find("quality") != std::string::npos){
						mout.note() << "Skipping quality field " << qit->first << mout.endl;
						continue;
					}
					HI5TREE * newdata;
					if (true){
						DataSetDst<CartesianDst> dDataSet( resources.inputHi5["dataset1"] );
						Data<CartesianDst> & dataDst = dDataSet.getData(qit->first);
						if (dataDst.data.isEmpty()){
							mout.info() << "Adding " << qit->first << mout.endl;
						}
						else {
							mout.note() << "Overriding " << qit->first << mout.endl;
						}
						newdata = & dataDst.tree;
					}
					srcDataSet(qit->second).getChildren().swap(newdata->getChildren());
					//resources.inputHi5().
				}
			}


		}

		//mout.error() << "WHAT " << mout.endl;

	} // traverse: srcPaths

	DataSelector::updateAttributes(resources.inputHi5);  // This is needed, though should be unneeded... Check later.
}

void CmdInputFile::readTextFile(const std::string & fullFilename) const  {
	std::ifstream ifstr;
	ifstr.open(fullFilename.c_str());
	hi5::Hi5Base::readText(getResources().inputHi5, ifstr);
	ifstr.close();
	DataSelector::updateAttributes(getResources().inputHi5);
	// Hi5Base::writeText(getResources().inputHi5, std::cout);
	// mout.error() << "getResources().inputHi5" << mout.endl;
}

void CmdInputFile::readImageFile(const std::string & fullFilename) const {

	drain::MonitorSource mout(name, __FUNCTION__); // = getResources().mout;

	RackResources & resources = getResources();

	std::list<std::string> paths;
	DataSelector::getPaths(resources.inputHi5, paths, "/data$");

	/// Find the first empty data.
	std::list<std::string>::iterator itEmptyData = paths.end();
	std::list<std::string>::iterator it = paths.begin();
	while (it != paths.end()){
		if (resources.inputHi5(*it).data.dataSet.isEmpty()){
			itEmptyData = it;
			//mout.warn() << "Checking... " << *it << mout.endl;
			break;
		}
		++it;
	}

	//mout.error() << "found path=" << *itepath << mout.endl;
	std::string path;
	if (itEmptyData != paths.end()){
		path = *itEmptyData; // clumsy copy
		mout.info() << "Storing input data to empty data at path=" << *itEmptyData << mout.endl;
	}
	else {
		path = "dataset1";  // default
		//DataSelector::getNextOrdinalPath(getResources().inputHi5,"/dataset.*/data[0-9]+$", path);
		DataSelector::getLastOrdinalPath(getResources().inputHi5, "dataset[0-9]+$", path);
		mout.debug() << " Found existing dataset: " << path << mout.endl;
		const std::string searchPath(path+"/data[0-9]+$");
		path += "/data1";
		DataSelector::getNextOrdinalPath(getResources().inputHi5, searchPath, path);
		//path += "/data";
		mout.info() << "Storing data in path: " << path << mout.endl;
	}

	HI5TREE & dst = resources.inputHi5(path);
	drain::image::Image & dstImage = dst["data"].data.dataSet;
	drain::image::File::read(dstImage, fullFilename);
	// Note: png variables have been read, hence could be basically assigned?
	drain::VariableMap & where = dst["where"].data.attributes;
	if (! where.hasKey("nbins"))
		where["nbins"] = (long)dstImage.getWidth();

	if (! where.hasKey("nrays"))
		where["nrays"] = (long)dstImage.getHeight();

	if (! where.hasKey("rscale"))
		where["rscale"] = 500;  // Remember to change this in practise.


	drain::VariableMap & what = dst["what"].data.attributes;  // todo setQuantityDefaults("DBZH",type);
	if (! what.hasKey("gain"))
		what["gain"] = 1.0;
	if (! what.hasKey("offset"))
		what["offset"] = 0;
	if (! what.hasKey("undetect"))
		what["undetect"] = dstImage.getMin<double>();
	if (! what.hasKey("nodata"))
		what["nodata"] = dstImage.getMax<double>();

	DataSelector::updateAttributes(resources.inputHi5);
	//DataSelector::updateAttributes(dst);
}



} // namespace rack

// Rack
