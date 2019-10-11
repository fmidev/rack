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
#include <map>
#include <ostream>
#include <iomanip>



#include <drain/util/Log.h>
#include <drain/util/RegExp.h>

#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>

#include <drain/prog/CommandRegistry.h>

#include "rack.h"
#include "product/DataConversionOp.h"
#include "data/ODIM.h"
#include "hi5/Hi5.h"
#include "commands.h"
#include "data/DataCoder.h"
#include "data/EchoClass.h"

namespace rack {


///
/* General commands.
 *
 */


/// Select input data for next command.
/**

Most commands apply implicit input data selection criteria,
typically involving quantities and elevation angle(s).
Often, the first data array matching the criteria are used.

One can explicitly change the criteria with \c --select (\c -s) command .
For example, in data conversions one may wish to focus on VRAD data, not all the data.

The selection command can be applied practically with all the commands processing data.
In computing meteorological products (\ref products), it affects the following product only.
In case of anomaly detectors (\ref andrepage), it applies to all subsequent operators.

Note that some processing commands may not support explicit data selection.

\~remark

will neglect some selection arguments, eg. CAPPI product will always use \c quantity=DBZH.

-# \b Deprecating: \c path (regular expression): paths of the ODIM structure, eg. \c "/dataset[0-9]+$"
-# \c quantity (regular expression): ODIM attribute \c quantity, eg. \c "DBZ.*"
-# \c index (positive integer): first occurrence to be considered (makes sense with dataset selections)
-# \c count (positive integer): \c index+count occurrences will be considered (makes sense with dataset selections)
-# \c elangle (double:double): elevation angle range, use colon (:) as separator; single value can be issued as well.
-# \b Deprecating: \c elangleMin (double): minimum elevation angle
-# \b Deprecating: \c elangleMax (double): maximum elevation angle
Some parameters are deprecating, hence it is better to use explicit keys is assignments, for example:
 --code
rack volume.h5 --select 'dataset=2:5,quantity=DBZH'                 <commands>
rack volume.h5 --select 'quantity=DBZH,elangle=0.5:4.0'   <commands>
 --endcode
\~



*/
class CmdSelect : public BasicCommand {

public:

	CmdSelect() : BasicCommand(__FUNCTION__, "Data selection for the next operation."){
		parameters.append(test.getParameters());
		//parameters.reference("selector", getResources().select, DataSelector().getParameters().getKeys());
		//parameters.separator = 0;
	};

	virtual
	void setParameters(const std::string & params, char assignmentSymbol='=') {
		drain::Logger mout(getName(), __FUNCTION__);
		getResources().select = params;
		//BasicCommand::setParameters(params, assignmentSymbol);
		test.deriveParameters(params);  // just for checking, also group syntax (dataset:data:...)
		//mout.note() << params << " => " << test << '|' << test.groups.value << mout.endl;
	}

private:

	mutable DataSelector test;

};

/// Set selection criteria strictly to one \c quantity .
/**
Selecting quantities only is frequently needed, so there is a dedicated command \c --selectQuantity (\c -Q ) which
accepts comma-separated simple patterns (with * and ?) instead of regular expressions. For example, \c -Q \c DBZH*,QIND
is equal to \c --select \c quantity='^(DBZH.*|QIND)$' .
*/
class CmdSelectQuantity : public SimpleCommand<std::string> {

public:

	CmdSelectQuantity() : SimpleCommand<std::string>(__FUNCTION__, "Like --select quantity=... but with patterns (not regexps)", "quantities","","quantity[,quantity2,...]"){

	};

	inline
	void exec() const {

		drain::Logger mout(getName(), __FUNCTION__);

		const std::string v = StringTools::replace(StringTools::replace(StringTools::replace(value,",","|"), "*",".*"), "?", "[.]");
		mout.debug() << v << mout.endl;
		RackResources & r = getResources();
		r.select = "quantity=^(" + v + ")$";
		r.dataOk = true;
		//getRegistry().run("select", "quantity=^(" + vField + ")$");
	}

};






class CmdConvert : public BasicCommand {

public:

	CmdConvert() : BasicCommand(__FUNCTION__, "Convert --select'ed data to scaling and markers set by --encoding") {
	};


	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		RackResources & resources = getResources();
		if (resources.currentHi5 == &resources.cartesianHi5){
			convertCurrentH5<CartesianODIM>();
		}
		else if (resources.currentHi5 == resources.currentPolarHi5) {
			convertCurrentH5<PolarODIM>();
		}
		else {
			mout.warn() << " currentHi5 neither Polar nor Cartesian " << mout.endl;
		}
		resources.currentImage = NULL;
		resources.currentGrayImage = NULL;

	};

protected:

	template <class OD>
	void convertCurrentH5() const {

		RackResources & resources = getResources();

		DataConversionOp<OD> op;
		op.setEncodingRequest(resources.targetEncoding);
		resources.targetEncoding.clear();

		// mout.debug() << op.encodingRequest << mout.endl;
		// mout.warn() << op << mout.endl;
		op.dataSelector.setParameters(resources.select);
		resources.select.clear();

		op.processH5(*resources.currentHi5, *resources.currentHi5);
		DataTools::updateInternalAttributes(*resources.currentHi5);

	}

};

class CmdCreateDefaultQuality : public BasicCommand {

public:

	CmdCreateDefaultQuality() : BasicCommand(__FUNCTION__, "Creates default quality field. See --undetectQuality"){
		parameters.reference("quantitySpecific", quantitySpecific=false, "[0|1]");
	};

	bool quantitySpecific;

	template <class OD>
	void processStructure(HI5TREE & dst, const ODIMPathList & paths, const drain::RegExp & quantityRegExp) const {

		drain::Logger mout(getName(), __FUNCTION__);

		typedef DstType<OD> DT; // ~ PolarDst, CartesianDst

		const QuantityMap & qmap = getQuantityMap();

		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
			mout.info() << *it  << mout.endl;
			HI5TREE & dstDataSetH5 = dst(*it);
			DataSet<DT> dstDataSet(dstDataSetH5, quantityRegExp);
			if (quantitySpecific){
				for (typename DataSet<DT>::iterator it2 = dstDataSet.begin(); it2!=dstDataSet.end(); ++it2){
					mout.debug() << '\t' << it2->first  << mout.endl;
					Data<DT> & dstData = it2->second;
					PlainData<DT> & dstQuality = dstData.getQualityData();
					if (!dstQuality.data.isEmpty())
						mout.warn() << "quality data exists already, overwriting" << mout.endl;
					dstData.createSimpleQualityData(dstQuality, 1.0, 0.0, DataCoder::undetectQualityCoeff);
					qmap.setQuantityDefaults(dstQuality, "QIND");
					dstQuality.data.setScaling(dstQuality.odim.gain, dstQuality.odim.offset);
					//@ dstQuality.updateTree();
				}
			}
			else {
				Data<DT> & dstData = dstDataSet.getFirstData();
				PlainData<DT> & dstQuality = dstDataSet.getQualityData();
				if (!dstQuality.data.isEmpty())
					mout.warn() << "quality data exists already, overwriting" << mout.endl;
				dstData.createSimpleQualityData(dstQuality, 1.0, 0.0, DataCoder::undetectQualityCoeff);
				qmap.setQuantityDefaults(dstQuality, "QIND");
				dstQuality.data.setScaling(dstQuality.odim.gain, dstQuality.odim.offset);
				//@ dstQuality.updateTree();
			}
			//@  DataTools::updateInternalAttributes(dstDataSetH5);

		}


	}

	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		//RackResources & resources = getResources();

		DataSelector selector(resources.select);
		resources.select.clear();
		const drain::RegExp quantityRegExp(selector.quantity);
		selector.quantity.clear();
		//if (selector.path.empty()) selector.path = "dataset[0-9]+$";  // OLD

		ODIMPathList paths;
		selector.getPaths(*resources.currentHi5, paths, ODIMPathElem::DATASET); // RE2


		if (resources.currentHi5 == resources.currentPolarHi5){
			processStructure<PolarODIM>(*resources.currentHi5, paths, quantityRegExp);
		}
		else if (resources.currentHi5 == &resources.cartesianHi5){
			processStructure<CartesianODIM>(*resources.currentHi5, paths, quantityRegExp);
		}
		else {
			drain::Logger mout(getName(), __FUNCTION__);
			mout.warn() << "no data, or data structure other than polar volume or Cartesian" << mout.endl;
		}

		DataTools::updateInternalAttributes(*resources.currentHi5);
	};

};


/**

Examples:
\include example-delete.inc

Notice that \c --elangle applies to volume data only, and essentially selects \c dataset  groups.
Similarly, \c quantity selects \c data (and \c quality)  groups.
If selection parameters of both levels are issued in the same command,
implicit \c AND function applies in selection.

*/
class CmdDelete : public BasicCommand {  // public SimpleCommand<std::string> {

public:

	CmdDelete() : BasicCommand(__FUNCTION__, "Deletes selected parts of h5 structure."){
		parameters.append(selector.getParameters());
	};

	virtual
	void setParameters(const std::string & params, char assignmentSymbol='=') {

		// drain::Logger mout(getName(), __FUNCTION__);
		selector.deriveParameters(params);  // just for checking, also group syntax (dataset:data:...)

		selector.convertRegExpToRanges(); // transitional op for deprecated \c path

	}

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		//mout.debug() << "group mask: " << groupFilter << ", selector: " << selector << mout.endl;

		HI5TREE & dst = *getResources().currentHi5;

		// Step 0
		mout.info() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);

		mout.debug() << "selector: " << selector << mout.endl;

		ODIMPathList paths;
		selector.getPaths(dst, paths);
		mout.info() << "deleting " << paths.size() << " substructures" << mout.endl;
		for (ODIMPathList::const_reverse_iterator it = paths.rbegin(); it != paths.rend(); it++){
			mout.debug() << "deleting: " << *it << mout.endl;
			dst.erase(*it);
		}
	};

private:

	DataSelector selector;


};


///
/**

Metadata groups (\c what, \c where, \c how) are preserved or deleted together with their
parent groups.

Examples:
\include example-keep.inc

 */
class CmdKeep : public BasicCommand { // : public SimpleCommand<std::string> {

public:

	/// Keeps a part of the current h5 structure, deletes the rest. Path and quantity are regexps.
	CmdKeep() : BasicCommand(__FUNCTION__, "Keeps selected part of data structure, deletes rest."){
		parameters.append(selector.getParameters());
	};

	virtual
	void setParameters(const std::string & params, char assignmentSymbol='=') {
		//drain::Logger mout(getName(), __FUNCTION__);
		//selector.groups = ODIMPathElem::ALL_GROUPS; //DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY;
		selector.deriveParameters(params);  // just for checking, also group syntax (dataset:data:...)
		selector.convertRegExpToRanges(); // transition support...
	}

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		//RackResources & resources = getResources();
		HI5TREE & dst = *getResources().currentHi5;

		// Step 0
		mout.info() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);



		ODIMPathList paths;

		DataSelector preselector;

		//dst.getPaths(paths);
		preselector.getPaths(dst, paths, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY );
		mout.info() << "data structure contains " << paths.size() << " paths " << mout.endl;

		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); it++){
			mout.debug(1) << "set noSave: " << *it << mout.endl;
			dst(*it).data.noSave = true;
		}

		//ODIMPathElem::group_t groupFilter = selector.deriveParameters(value);
		mout.note() << "selector for saved paths: " << selector << '|' << selector.groups << mout.endl;

		ODIMPathList savedPaths;
		selector.getPaths(dst, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		//mout.info() << "set save" << mout.endl;
		const ODIMPathElem::group_t groupMask = selector.groups; //getGroupMask();

		for (ODIMPathList::iterator it = savedPaths.begin(); it != savedPaths.end(); it++){
			mout.debug(1) << "set save: " << *it << mout.endl;
			ODIMPath p;
			for (ODIMPath::iterator pit = it->begin(); pit != it->end(); pit++){
				p << *pit;
				// mout.debug(2) << " \t\t" << p << mout.endl;
				if (!pit->belongsTo(groupMask)){
					if (dst(p).hasChild(odimARRAY)){
						dst(p)[odimARRAY].data.noSave = true;
						dst(p)[odimARRAY].data.dataSet.resetGeometry();
						mout.debug(1) << "clearing " << p << " // data" << mout.endl;
					}
				}
				dst(p).data.noSave = false;
			}

		}

		// Traverse all the paths
		// reverse_iterator, because dataset groups (descendants) become deleted before dataset itself.
		for (ODIMPathList::const_reverse_iterator it = paths.rbegin(); it != paths.rend(); it++){

			if (dst(*it).data.noSave){
				mout.debug(2) << "deleting: " << *it << mout.endl;
				dst.erase(*it);
			}
			else
				mout.debug() << "saving: " << *it << mout.endl;
		}


	};

private:

	DataSelector selector;


};





/// Rename or move data groups and attributes.
/**

    \param src - source path consisting at least of the group path
    \param dst - full destination path or only ':' followed by attribute key

    A full path consist of a slash-separated group path elements followed by an attribute key separated by colon.
    For example: \c /dataset1/data2/what:gain .


    Examples:
	\include example-move.inc

 */
class CmdMove :  public BasicCommand {

public:

	/// Default constructor
	CmdMove() : BasicCommand(__FUNCTION__, "Rename or move data groups and attributes."){
		parameters.reference("src", pathSrc = "", "/group/group2[:attr]");
		parameters.reference("dst", pathDst = "", "/group/group2[:attr]");
	};

	// TODO: recognize attr,attr vs path,path
	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();
		if (resources.currentHi5 == NULL){
			mout.error() << "current Hi5 == NULL" << mout.endl;
			return;
		}


		HI5TREE & dstRoot = *resources.currentHi5;

		ODIMPath path1;
		std::string attr1;
		// std::string attr1value; // debug
		// std::string attr1type;// debug
		hi5::Hi5Base::parsePath(pathSrc, path1, attr1); //, attr1value, attr1type);
		mout.debug() << "path:  " << path1 << ", size=" << path1.size() << mout.endl;

		/*
		mout.warn() << "path:  " << path1 << mout.endl;
		mout.warn() << "attr:  " << attr1 << mout.endl;
		mout.warn() << "value: " << attr1value << mout.endl;
		mout.warn() << "type:  " << attr1type  << mout.endl;
		return;
		 */

		if (!dstRoot.hasPath(path1)) // make it temporary (yet allocating)
			mout.warn() << "nonexistent path: " << path1 << mout.endl;

		ODIMPath path2;
		std::string attr2;
		hi5::Hi5Base::parsePath(pathDst, path2, attr2);

		mout.debug() << "path1: " << path1 << '(' << path1.size() << ')' << " : " << attr1 << mout.endl;
		mout.debug() << "path2: " << path2 << '(' << path2.size() << ')' << " : " << attr2 << mout.endl;


		if (attr1.empty()){ // RENAME PATHS (swap two paths)

			//hi5::Hi5Base::parsePath(pathDst, path2, attr2);
			if (!attr2.empty()){
				mout.error() << "cannot move path '" << path1 << "' to attribute '" << attr2 << "'" << mout.endl;
				return;
			}

			mout.debug() << "renaming path '" << path1 << "' => '" << path2 << "'" << mout.endl;
			//mout.warn() << dstRoot << mout.endl;
			HI5TREE & dst1 = dstRoot(path1);
			if (!dstRoot.hasPath(path2)) // make it temporary (yet allocating it for now)
				dstRoot(path2).data.noSave = true;
			HI5TREE & dst2 = dstRoot(path2);

			const bool noSave1 = dst1.data.noSave;
			const bool noSave2 = dst2.data.noSave;
			dst2.swap(dst1);
			dst1.data.noSave = noSave2;
			dst2.data.noSave = noSave1;
			dst2.data.attributes.swap(dst1.data.attributes);
			//dstRoot.erase(path1);
			//DataTools::updateInternalAttributes(dst1); // recurse subtrees
			//DataTools::updateInternalAttributes(dst2); // recurse subtrees

		}
		else { // Rename attribute


			if (path2.empty())
				path2 = path1;

			if (attr2.empty())
				attr2 = attr1;

			mout.debug() << "renaming attribute (" << path1 << "):" << attr1 << " => (" << path2 << "):" << attr2 << mout.endl;

			HI5TREE & dst1 = dstRoot(path1);
			if (!dst1.data.attributes.hasKey(attr1)){
				mout.warn() <<  "attribute '" << attr1 << "' not found, path='" << path1 << "'" << mout.endl;
			}
			HI5TREE & dst2 = dstRoot(path2);
			dst2.data.attributes[attr2] = dst1.data.attributes[attr1];
			dst1.data.attributes.erase(attr1);
			//DataTools::updateInternalAttributes(dst2);
			mout.debug() << "dst1 attributes now: " << dst1.data.attributes << mout.endl;
			mout.debug() << "dst2 attributes now: " << dst2.data.attributes << mout.endl;
			//dstRoot.erase(path1);
		}

		DataTools::updateInternalAttributes(dstRoot);
		//mout.error() << "argument <from,to> syntax error; regexp: " << pathSplitter.toStr() << mout.endl;

		pathSrc = "";
		pathDst = "";
	}

protected:

	mutable
	std::string pathSrc;

	mutable
	std::string pathDst;




};

//class CmdRename : public BasicCommand {
class CmdRename : public CmdMove {

public:

	//CmdRename() : BasicCommand(__FUNCTION__, "Move/rename paths and attributes"){};

	void exec() const {
		drain::Logger mout(name, __FUNCTION__);
		mout.warn() << "deprecated command, use '--move' instead" << mout.endl;
		CmdMove::exec();
	};

};




class CmdCompleteODIM : public BasicCommand {

public:

	CmdCompleteODIM() : BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	template <class OD>  // const drain::VariableMap & rootProperties,
	void complete(HI5TREE & dstH5, OD & od) const {

		drain::Logger mout(getName(), __FUNCTION__);

		typedef DstType<OD> DT;

		DataSelector selector; // todo implement --select

		//mout.debug() << "start upd" << mout.endl;

		DataTools::updateInternalAttributes(dstH5); //, drain::FlexVariableMap());
		// DataTools::updateInternalAttributes(dstH5);

		OD rootODIM(dstH5.data.dataSet.getProperties());

		mout.debug() << "odim: " << rootODIM << mout.endl;


		for (HI5TREE::iterator it = dstH5.begin(); it != dstH5.end(); ++it){

			mout.debug() << "considering: " << it->first << mout.endl;

			if (it->first.is(ODIMPathElem::DATASET) && selector.dataset.isInside(it->first.getIndex())){

				//mout.note() << '@' << it->first << mout.endl;
				DataSet<DT> dstDataSet(it->second);
				//mout.note() << '%' << it->first << mout.endl;
				//return;

				for (typename DataSet<DT>::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){

					mout.debug() << dit->first << " :" << dit->second << mout.endl;

					PlainData<DT> & dstData = dit->second;

					//mout.warn() << "prop: " << dstData.data.properties << mout.endl;

					dstData.odim.updateFromMap(dstData.data.properties); // assume UpdateMetadata
					//mout.warn() << "ODIM: " << dstData.odim << mout.endl;

					if (!dstData.data.isEmpty()){
						const size_t w = dstData.data.getWidth();
						const size_t h = dstData.data.getHeight();
						dstData.odim.type = drain::Type::getTypeChar(dstData.data.getType());
						dstData.odim.setGeometry(w, h);
						//rootODIM.setGeometry(w, h);    // for Cartesian root-level xsize,ysize
						//dataSetODIM.setGeometry(w, h); // for Polar  dataset-level nbins, nrays
						if (dstData.odim.quantity.empty()){
							dstData.odim.quantity = dit->first;
						}
						if (dstData.odim.gain == 0){
							mout.info() << "setting quantity defaults [" << dstData.odim.quantity << ']' << mout.endl;
							getQuantityMap().setQuantityDefaults(dstData.odim); //, dstData.odim.quantity);
						}
					}

					//mout.note() << "dstData.updateTree: " << dit->first << mout.endl;

					dstData.updateTree2();
					rootODIM.update(dstData.odim);
				}
			}
		}

		/*
		if (rootODIM.date.empty())
			rootODIM.date = rootODIM.startdate;
		if (rootODIM.time.empty())
			rootODIM.time = rootODIM.starttime;
		 */
		ODIM::copyToH5<ODIMPathElem::ROOT>(rootODIM, dstH5);

	}

	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		if (resources.inputHi5["what"].data.attributes["object"].toStr() == "COMP"){
			resources.inputHi5.swap(resources.cartesianHi5);
			resources.currentHi5 = &resources.cartesianHi5;
		}

		if (resources.currentHi5 == resources.currentPolarHi5){
			PolarODIM odim;
			complete(*resources.currentHi5, odim);
		}
		else if (resources.currentHi5 == &resources.cartesianHi5){
			CartesianODIM odim;
			complete(*resources.currentHi5, odim);
		}
		else {

		}


	};

};


/**
 *    --/dataset1/what:nbins=210
 *
 *    Future feature: Select properties with \c --select , then
 *    \code
 *    rack volume.h5 --select data=2:4  --/what:undetect=244
 *    \endcode
 *
 */
class CmdSetODIM : public SimpleCommand<std::string> {

public:

	CmdSetODIM() : SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		RackResources & resources = getResources();

		if (value.find_first_of("?*()^$") != std::string::npos){
			mout.warn() << "RegExp support temporarily supressed from this version" << mout.endl;
			return;
		}
		mout.warn() << "value: " << value << mout.endl;
		hi5::Hi5Base::readTextLine(*(resources.currentHi5), value);

		DataTools::updateInternalAttributes(*(resources.currentHi5));


		// List of paths in which assignments are repeated.
		// A single path, unless search key is a regexp, see below.
		/*
		ODIMPathList pathList;

		const std::string & s = value;

		const size_t i = s.find(':');
		const size_t j = s.find('=');

		const std::string groupPath = s.substr(0, i);
		const std::string attributeKey = (i != std::string::npos) ? s.substr(i+1,j-i) : ""; // WAS substr(i), not (i+1) => if non-empty, contains ':' as prefix
		const std::string value = s.substr(j+1);

		mout.debug(5) << "Group path:" << groupPath << ' ';
		mout << "Attribute:"  << attributeKey << ' ';
		mout << "Value:"      << value << mout.endl;

		HI5TREE & currentHi5 = *(getResources().currentHi5);
		 */

		/*
		if (groupPath.find_first_of(".?*[]") == std::string::npos){
			pathList.push_front(groupPath);
		}
		else {
			getResources().currentHi5->getPaths(pathList);
			//pathList.getPaths(*getResources().currentHi5, groupPath); // RE2
			// mout.warn()  << " --/ multiple, ok" << mout.endl;
		}

		//static 		const drain::RegExp attributeGroup("^((.*)/)?(what|where|how)$");  // $1,$2: one step above "where","what","how"

		for (ODIMPathList::const_iterator it = pathList.begin(); it != pathList.end(); ++it){
			// mout.warn()  << " multiple: key=" << *it << "|"  << attributeKey << mout.endl;
			//hi5::Hi5Base::readTextLine(currentHi5, it->toStr()+attributeKey, value);
			hi5::Hi5Base::readTextLine(currentHi5, *it, attributeKey, value);

			if (it->back().belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			//if (attributeGroup.execute(*it) == 0){ // match
				ODIMPath parent = *it;
				parent.pop_back();
				// mout.warn() << "update attributes under: " <<  attributeGroup.result[2] << mout.endl;
				//DataTools::updateInternalAttributes(currentHi5(attributeGroup.result[2]));  // one step above where,what,how
				DataTools::updateInternalAttributes(currentHi5(parent));  // one step above where,what,how
			}
			else {
				//mout.warn() << "update attributes under: " <<  *it << mout.endl;
				DataTools::updateInternalAttributes(currentHi5(*it));
			}

		}
		 */

	};

};






// Various dumps


class CmdDumpEchoClasses : public BasicCommand {

public:

	CmdDumpEchoClasses() : BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
	};

	void exec() const {

		const classtree_t & t = getClassTree();

		drain::JSON::write(t);

		// t.dumpContents();
		// t.dump();
		/*
		typedef classtree_t::path_t path_t;
		typedef std::list<path_t> pathlist_t;
		pathlist_t paths;
		t.getPaths(paths);
		for (pathlist_t::iterator it = paths.begin(); it != paths.end(); ++it) {
			it->separator = '.';
			std::cout << *it << ": "  << t(*it).data << '\n';
		}
		 */
		//std::cout << t.hasPath("met") << ':' << t.hasPath("met/hail") << ':' << t.hasPath("met.hail") << '\n';


	}

};

class CmdDumpMap : public BasicCommand {

public:

	std::string filter;
	std::string filename;

	//options["dumpMap"].syntax = "<regexp>[:<file>]"
	CmdDumpMap() : BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
		parameters.separator = ':'; //s = ":";
		parameters.reference("filter", filter = "", "regexp");
		parameters.reference("filename", filename = "", "std::string");
	};

	void exec() const {

		// TODO filename =resources.outputPrefix + values[1];

		drain::RegExp r(filter);
		// std::ostream *ostr = &std::cout;
		std::ofstream ofstr;
		if (filename != ""){
			//std::string filename = options.get("outputFile","out")+extension;
			std::cout << "opening " << filename << '\n';
			//std::string outFileName =resources.outputPrefix + filename;
			ofstr.open(filename.c_str(),std::ios::out);
			// ostr = & ofstr;
		}
		throw std::runtime_error(name + ": unimplemented SingleParamCommand");
		ofstr.close();

	};

};







class CmdHelpRack : public CmdHelp {
public:
	CmdHelpRack() : CmdHelp(std::string(__RACK__) + " - a radar data processing program", "Usage: rack <input> [commands...] -o <outputFile>") {};
};


class CmdHelpExample : public SimpleCommand<std::string> {

public:

	CmdHelpExample() : SimpleCommand<std::string>(__FUNCTION__, "Dump example of use and exit.", "keyword", "") {
	};


	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		std::ostream & ostr = std::cout;

		const CommandRegistry::map_t & map = getRegistry().getMap();

		const std::string key = value.substr(value.find_first_not_of('-'));
		//mout.warn() << key << mout.endl;

		CommandRegistry::map_t::const_iterator it = map.find(key);

		if (it != map.end()){

			const Command & d = it->second;
			const ReferenceMap & params = d.getParameters();

			ostr << "--" << key << ' ';
			if (!params.getKeyList().empty()){
				ostr << "'";
				params.getValues(ostr);
				ostr << "'";
			}
			ostr << '\n';
			//ostr << '\n';
			// ostr << "Default values: ";
			// params.getValues(ostr);
			//ostr << '\n';

			//ostr << "  # Parameters:\n";
			const std::list<std::string> keys = params.getKeyList();
			const std::map<std::string,std::string> & units = params.getUnitMap();
			for (std::list<std::string>::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
				ostr << "  # " << *kit << '=';
				ostr << params[*kit];
				ostr << ' ';
				const std::map<std::string,std::string>::const_iterator uit = units.find(*kit);
				if (uit != units.end())
					if (!uit->second.empty())
						ostr << '[' << uit->second << ']';
				ostr << '\n';
			}
			//ostr << '\n';  ostr << "# Modifiable encoding:\n";

		}
		else {

		}

		exit(0);
	};




};
// static CommandEntry<CmdHelpExample> cmdHelpExample("helpExample", 0);

class CmdJSON : public SimpleCommand<std::string> { //BasicCommand {
public:

	CmdJSON() : SimpleCommand<std::string>(__FUNCTION__, "Dump to JSON.", "property", "", ""){
	};

	virtual
	void run(const std::string & params = "") {

		drain::Logger mout(name, __FUNCTION__);

		CommandRegistry & reg = getRegistry();

		std::ostream & ostr = std::cout;

		if (params.empty()){
			reg.toJSON(ostr);
		}
		else {
			if (!reg.has(params)){
				mout.error() << "no such key: " << params << mout.endl;
				return;
			}

			const drain::Command & command = reg.get(params);
			const ReferenceMap & m = command.getParameters();
			const ReferenceMap::keylist_t & keys = m.getKeyList();
			ostr << "{";
			ostr << "  \"parameters\": {";

			char sep=0;

			//for (ReferenceMap::const_iterator it = m.begin(); it!=m.end(); ++it){
			for (ReferenceMap::keylist_t::const_iterator it = keys.begin(); it!=keys.end(); ++it){
				if (sep)
					ostr << sep;
				else
					sep = ',';
				ostr << "\n  \"" << *it << "\": {\n";
				//ostr << std::setw(10) << std::left << "\"type\": \"";
				//ostr << "    \"type\": \"";
				const drain::Referencer & entry = m[*it];
				if (entry.isString()) {
					ostr << "string";
				}
				else {
					ostr << Type::call<drain::simpleName>(entry.getType());
				}
				ostr << "\",\n";

				ostr << "    \"value\": ";
				entry.valueToJSON(ostr);
				ostr << "\n";
				ostr << "  }";
			}
			ostr << "  }";
			if (!command.getType().empty()){
				ostr << ",\n  \"output\": \"" << command.getType() << "\"";
			}
			ostr << "\n}\n";
		}

		//std::cout << std::setw(10) << "Voila!" << std::endl;
	};

};


class CmdFormatOut : public SimpleCommand<std::string> {

public:

	CmdFormatOut() : SimpleCommand<std::string>(__FUNCTION__, "Dumps the formatted std::string to a file or stdout.", "filename","","std::string") {
		//parameters.separators.clear();
		//parameters.reference("filename", filename, "");
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		std::string & format = cmdFormat.value;
		format = drain::StringTools::replace(format, "\\n", "\n");
		format = drain::StringTools::replace(format, "\\t", "\t");

		CommandRegistry & reg = getRegistry();
		reg.statusFormatter.parse(format);
		//mout.warn() << "before expansion: " << r.statusFormatter << mout.endl;

		reg.statusFormatter.expand(resources.getUpdatedStatusMap()); // needed? YES
		//mout.warn() << r.getStatusMap() << mout.endl;
		//mout.warn() << r.statusFormatter << mout.endl;


		if ((value == "")||(value == "-"))
			std::cout << reg.statusFormatter;
		else {
			const std::string outFileName = getResources().outputPrefix + value;
			mout.info() << "writing " << outFileName << mout.endl;
			std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			if (ofstr)
				ofstr << reg.statusFormatter;
			else
				mout.warn() << "write error: " << outFileName << mout.endl;
			//strm.toStream(ofstr, cmdStatus.statusMap.exportMap());
			ofstr.close();
		}

		//mout.warn() << "after expansion: " << r.statusFormatter << mout.endl;
		//r.statusFormatter.debug(std::cerr, r.getStatusMap());

	};

};


class CmdStatus : public BasicCommand {

public:

	CmdStatus() : BasicCommand(__FUNCTION__, "Dump information on current images.") {
	};

	void exec() const {

		std::ostream & ostr = std::cout; // for now...

		const drain::VariableMap & statusMap = getResources().getUpdatedStatusMap();

		for (drain::VariableMap::const_iterator it = statusMap.begin(); it != statusMap.end(); ++it){
			ostr << it->first << '=' << it->second << ' ';
			it->second.typeInfo(ostr);
			ostr << '\n';
		}
		// ostr << statusMap << std::endl;

	};



};


/// Facility for validating and storing desired technical (non-meteorological) user parameters.
/*
 *  Quick-checks values immediately.
 *  To consider: PolarODIM and CartesianODIM ?
 */
class CmdEncoding : public BasicCommand {

public:

	inline
	CmdEncoding() : BasicCommand(__FUNCTION__, "Sets encodings parameters for polar and Cartesian products, including composites.") {

		parameters.separator = ','; // s = ",";
		parameters.reference("type", odim.type = "C", "storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)");
		parameters.reference("gain", odim.gain = 0.0, "scaling coefficient");
		parameters.reference("offset", odim.offset = 0.0, "bias");
		parameters.reference("undetect", odim.undetect = 0.0, "marker");
		parameters.reference("nodata", odim.nodata = 0.0, "marker");

		/// Polar-specific
		parameters.reference("rscale", odim.rscale = 0.0, "metres");
		parameters.reference("nrays", odim.nrays = 0L, "count");
		parameters.reference("nbins", odim.nbins = 0l, "count");

		/// Experimental, for image processing
		parameters.reference("quantity", odim.quantity = "", "string");

		//getQuantityMap().setTypeDefaults(odim, "C");
		odim.setTypeDefaults("C"); // ??
		//odim.setTypeDefaults(typeid(unsigned char));
		//std::cerr << "CmdEncoding.odim:" << odim << std::endl;
		//std::cerr << "CmdEncoding.pars:" << parameters << std::endl;
	};


	virtual  //?
	inline
	void run(const std::string & params){

		Logger mout(getName(), __FUNCTION__);

		try {

			/// Main action: store it for later use (by proceeding commands).
			getResources().targetEncoding = params;

			/// Also check and warn of unknown parameters
			parameters.setValues(params);  // sets type, perhaps, hence set type defaults and override them with user defs

			// mout.note() << "pars: " << parameters << mout.endl;
			// mout.note() << "odim: " << odim << mout.endl;
			odim.setTypeDefaults();

			// Reassign (to odim).
			parameters.setValues(params);
			//mout.note() << "Re-assigned parameters: " << parameters << mout.endl;

		}
		catch (const std::runtime_error & e) {

			mout.warn() << "Could not set odim" << mout.endl;
			mout.note() << "pars: " << parameters << mout.endl;
			mout.warn() << "odim: " << odim << mout.endl;
			mout.error() << e.what() << mout.endl;

		}

		// std::cerr << "CmdEncoding.odim: " << odim << std::endl;
		// std::cerr << "CmdEncoding.pars: " << parameters << std::endl;

	};


private:

	/// Facility for validating the parameters supplied by the user.
	PolarODIM odim;



};
// extern CommandEntry<CmdEncoding> cmdEncoding;





class CmdAutoExec : public BasicCommand {

public:

	CmdAutoExec() : BasicCommand(__FUNCTION__, "Execute script automatically after each input. See --script") {
		parameters.reference("exec", getResources().scriptParser.autoExec = -1, "0=false, 1=true, -1=set to true by --script");
	}

};


class CmdDataOk : public BasicCommand {

public:

	CmdDataOk() : BasicCommand(__FUNCTION__, "Status of last select."){
		parameters.reference("flag", getResources().dataOk = true);
	};
};


class UndetectWeight : public BasicCommand {  // TODO: move to general commands, leave warning here

public:

	UndetectWeight() : BasicCommand(__FUNCTION__, "Set the relative weight of data values assigned 'undetect'."){
		parameters.reference("weight", DataCoder::undetectQualityCoeff, "0...1");
	};


};

/// Default handler for requests without own handler. Handles options that are recognized as 1) files to be read or 2) ODIM properties to be assigned in current H5 structure.
/**
 *   ODIM properties can be set with
 *   - \c --/dataset2/data2/what:quantity=DBZH
 *
 */
class CmdDefaultHandler : public BasicCommand {
public:


	CmdDefaultHandler() : BasicCommand(getRegistry().DEFAULT_HANDLER, "Delegates plain args to --inputFile and args of type --/path:attr=value to --setODIM."){};  // getRegistry().DEFAULT_HANDLER, 0,

	virtual  //?
	inline
	void run(const std::string & params){

		Logger mout(getName());

		mout.debug(1) << "params: " << params << mout.endl;

		/// Syntax for recognising text files.
		static const drain::RegExp odimSyntax("^--?(/.+)$");

		if (params.empty()){
			mout.error() << "Empty parameters" << mout.endl;
		}
		else if (odimSyntax.execute(params) == 0) {
			//mout.warn() << "Recognised --/ hence running applyODIM" << mout.endl;
			mout.debug(1) << "assign: " << odimSyntax.result[1] << mout.endl;
			getRegistry().run("setODIM", odimSyntax.result[1]);
		}
		else {
			if (params.at(0) == '-'){
				mout.error() << "Unknown parameter (or invalid filename): " << params << mout.endl;
			}
			try {
				mout.debug(1) << "Assuming filename, trying to read." << mout.endl;
				getRegistry().run("inputFile", params);
			} catch (std::exception & e) {
				mout.error() << "could not handle params='" << params << "'" << mout.endl;
			}
		}

	};

};





class CmdCheckType : public BasicCommand {

public:

	CmdCheckType() : BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	void exec() const {
		PolarODIM::checkType(*getResources().currentHi5);
	};

};




class CmdVersion : public BasicCommand {

public:

	CmdVersion() : BasicCommand(__FUNCTION__, "Displays software version and quits."){};

	void exec() const {

		std::cout << __RACK_VERSION__ << '\n';
		std::cout << __RACK__ << ' ' << __RACK_VERSION__ << ' ' << __DATE__ << '\n';

		std::cout << DRAIN_IMAGE << '\n';

		unsigned int majnum=0, minnum=0, relnum=0;
		H5get_libversion(&majnum, &minnum, &relnum);  // error messages
		std::cout << "HDF5 " << majnum << '.' << minnum << '.' << relnum << '\n';

		std::cout << "GeoTIFF support: ";
#ifdef  GEOTIFF_NO
		std::cout << "no"<< '\n';
#else
		std::cout << "yes" <<  '\n';
#endif
		std::cout << '\n';


	};

};


class OutputDataVerbosity : public BasicCommand {

public:

	OutputDataVerbosity() : BasicCommand(__FUNCTION__, "Determines if also intermediate results (1) are saved. Replacing --aStore ?") {
		parameters.reference("level", ProductBase::outputDataVerbosity = 0, "0=default,1=intermediate results|2=extra debug results");
	};

	void exec() const {
		drain::Logger mout(name, __FUNCTION__);
		mout.warn() << "deprecating option, use '--store 2' instead" << mout.endl;
	};

};
// static CommandEntry<OutputDataVerbosity> dataVebose("verboseData");



class CmdAppend : public SimpleCommand<std::string> {  //public BasicCommand {

public:

	CmdAppend() : SimpleCommand<std::string>(__FUNCTION__, "Append inputs/outputs instead of overwriting.", "path", "", "<empty>|dataset|data"){

	}

	virtual
	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		if (value.empty()){
			ProductBase::appendResults.set(ODIMPathElem::ROOT); //?
		}
		else if (value == "dataset")
			ProductBase::appendResults.set(ODIMPathElem::DATASET);
		else if (value == "data") // This is needed to distinguish between /data123 and /data
			ProductBase::appendResults.set(ODIMPathElem::DATA);
		else {
			ProductBase::appendResults.set(value); // possibly "data4" or "dataset7"
			mout.warn() << "check path validity: "<< value << "'" << mout.endl;
		}

		/*
		if (!ODIMPath::isIndexed(ProductBase::appendResults.getType()) && ! (ProductBase::appendResults != ODIMPathElem::NONE)){
			mout.warn() << "illegal path elem '"<< value << "'" << mout.endl;
		}
		 */

	}

	//std::string path;

};


// NOTE: order changed
class CmdStore : public BasicCommand {

public:

	CmdStore() : BasicCommand(__FUNCTION__, "Set how intermediate and final outputs are stored. See --append"){
		parameters.reference("intermediate", ProductBase::outputDataVerbosity = 0, "store intermediate images");
		//parameters.reference("append",  ProductBase::appendResults = "", "|data|dataset");
		parameters.reference("append",  append, "|data|dataset (deprecated)");
	};

	//int level;
	std::string append;

	virtual
	void exec() const {
		drain::Logger mout(name, __FUNCTION__);
		DetectorOp::STORE = (ProductBase::outputDataVerbosity>0);
		if (!append.empty()){
			ProductBase::appendResults.set(append);
			mout.warn() << "option 'append' is deprecating, use --append <path> instead." << mout.endl;
		}
	};

};


class CmdQuantityConf : public BasicCommand {

public:

	CmdQuantityConf(): BasicCommand(__FUNCTION__, "1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair") {
		//parameters.separator = ',';
		parameters.reference("quantity:type", quantityType = "", "quantity (DBZH,VRAD,...) and storage type (C,S,d,f,...)");
		//parameters.reference("type", odim.type, "C", "storage type (C,S,d,f,...)");
		parameters.reference("gain", odim.gain = 0.0, "scaling coefficient");
		parameters.reference("offset", odim.offset = 0.0, "bias");
		parameters.reference("undetect", odim.undetect = 0.0, "marker");
		parameters.reference("nodata", odim.nodata = 0.0, "marker");
		parameters.reference("zero", zero = std::numeric_limits<double>::min(), "value");// what about max?

	}

	inline
	void run(const std::string & params = ""){

		Logger mout(getName(), __FUNCTION__);

		setParameters(params);
		const size_t i = quantityType.find(':');
		const std::string quantity = quantityType.substr(0, i);
		const std::string type = (i != std::string::npos) ? quantityType.substr(i+1) : std::string("");
		const std::string args = (i != std::string::npos) ? params.substr(i+1) : std::string();

		QuantityMap & m = getQuantityMap();
		mout.debug() << quantity << '(' << type << ')' << mout.endl;

		if (quantity.empty()){
			if (getResources().select.empty()){
				// No quantity given, dump all the quantities
				std::cout << "Quantities:\n";
				std::cout << m << std::endl;
			}
			else {
				// Select desired quantities. Note: on purpose, getResources().select not cleared by this operation
				DataSelector selector;
				selector.setParameters(getResources().select);
				const drain::RegExp regExp(selector.quantity);
				bool match = false;
				for (QuantityMap::const_iterator it = m.begin(); it != m.end(); ++it){
					if (regExp.test(it->first)){
						match = true;
						mout.warn() << it->first << " matches " << regExp << mout.endl;
						editQuantityConf(it->first, type, args);
					}
				}
				if (!match)
					mout.warn() << "no matches with. " << regExp << mout.endl;
			}
		}
		else {
			editQuantityConf(quantity, type, args);
		}

		quantityType.clear();
		zero = std::numeric_limits<double>::min();  // what about max?

	}

protected:

	std::string quantityType;

	//std::string quantityType;


	double zero;
	EncodingODIM odim;


	void editQuantityConf(const std::string & quantity, const std::string & type, const std::string & args){

		Logger mout(getName(), __FUNCTION__);

		QuantityMap & m = getQuantityMap();
		Quantity & q = m[quantity];

		if (zero != std::numeric_limits<double>::min()){
			if (std::isnan(zero))        // TODO: convert toStr to double NaN
				q.unsetZero();
			else
				q.setZero(zero);
		}

		if (type.length() == 1){

			mout.debug(1) << "setting default type " << type << mout.endl;

			const char typecode = type.at(0);
			q.defaultType = typecode;
			q.set(typecode).setScaling(1.0, 0); //.updateValues(params);
			//const size_t i = params.find(',');
			if (!args.empty()){
				mout.debug(1) << "setting other params " << args << mout.endl;
				q.set(typecode).updateValues(args);
			}
			//q[typecode].updateValues(params);
			//q[typecode].type = type; // setc by set() above
			//std::cout << "setting default type " << type << '\t';

			mout.debug() << "set default type for :" << quantity << '\n' << q << mout.endl;
		}
		else {  // No type given, dump quantity conf
			std::cout << quantity << '\n';
			//if (params != quantity)
			//	std::cout << " *\t" << q.get(q.defaultType) << '\n';
			//else
			std::cout << q;
		}

	}

};

//
struct CmdVerbose2 : public BasicCommand {

	int level;
	int imageLevel;

	CmdVerbose2() : BasicCommand(__FUNCTION__, "Set verbosity level") {
		parameters.reference("level", level = 5);
		parameters.reference("imageLevel", imageLevel = 4);
	};

	inline
	void exec() const {
		drain::getLog().setVerbosity(level);
		drain::image::getImgLog().setVerbosity(imageLevel);
	};

};

class CmdExpandVariables2 : public drain::CmdExpandVariables {

public:

	inline
	void exec() const {
		getResources().getUpdatedStatusMap();
		drain::CmdExpandVariables::exec();
	};

};


CommandModule::CommandModule(){ //

	CommandRegistry & registry = getRegistry();
	RackResources & resources = getResources();

	static RackLetAdapter<CmdDefaultHandler> cmdDefaultHandler(registry.DEFAULT_HANDLER);
	registry.add(resources.scriptParser, "script");

	static ScriptExec scriptExec(resources.scriptParser.script);
	registry.add(scriptExec, "exec");

	static RackLetAdapter<CmdSelect> cmdSelect("select", 's');
	static RackLetAdapter<CmdStatus> cmdStatus;
	static RackLetAdapter<CmdEncoding> cmdTarget("target", 't');  // old
	static RackLetAdapter<CmdEncoding> cmdEncoding("encoding", 'e');  // new
	static RackLetAdapter<CmdFormatOut> cmdFormatOut;


	static RackLetAdapter<drain::CmdDebug> cmdDebug;
	static RackLetAdapter<CmdVerbose2> cmdVerbose("verbose",'v');
	static RackLetAdapter<drain::CommandLoader> commandLoader("execFile");
	//static RackLetAdapter<drain::CmdExpandVariables> expandVariables;
	static RackLetAdapter<CmdExpandVariables2> expandVariables("expandVariables");

	static RackLetAdapter<CmdAutoExec> cmdAutoExec;
	static RackLetAdapter<CmdDataOk> dataOk("dataOk", -1);
	static RackLetAdapter<UndetectWeight> undetectWeight("undetectWeight");

	static RackLetAdapter<CmdSelectQuantity> cmdSelectQuantity("quantity",'Q');
	static RackLetAdapter<CmdCheckType> cmdCheckType;
	static RackLetAdapter<CmdCreateDefaultQuality> cmdCreateDefaultQuality;
	static RackLetAdapter<CmdCompleteODIM> cmdCompleteODIM;
	static RackLetAdapter<CmdConvert> cmdConvert;
	static RackLetAdapter<CmdDelete> cmdDelete;
	static RackLetAdapter<CmdDumpMap> cmdDumpMap; // obsolete?
	static RackLetAdapter<CmdDumpEchoClasses> cmdDumpEchoClasses; // obsolete?

	static RackLetAdapter<CmdHelpRack> help("help", 'h');
	static RackLetAdapter<CmdHelpExample> cmdHelpExample;
	static RackLetAdapter<CmdJSON> cmdJSON;
	static RackLetAdapter<CmdKeep> cmdKeep;
	static RackLetAdapter<CmdRename> cmdRename("rename"); // deprecating
	static RackLetAdapter<CmdMove> cmdMove;
	static RackLetAdapter<CmdSetODIM> cmdSetODIM;
	static RackLetAdapter<CmdVersion> cmdVersion;
	static RackLetAdapter<OutputDataVerbosity> dataVebose("verboseData");

	static RackLetAdapter<CmdAppend>  cmdAppend;
	static RackLetAdapter<CmdStore>  cmdStore;

	static RackLetAdapter<CmdQuantityConf> cmdQuantity;


}

} // namespace rack
