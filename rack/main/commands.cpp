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



#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/util/Input.h"

#include "drain/image/Image.h"
#include "drain/image/TreeSVG.h"

#include "drain/prog/CommandRegistry.h"

#include "rack.h"

#include "data/ODIM.h"
#include "data/ODIMValidator.h"
#include "data/DataCoder.h"
#include "data/EchoClass.h"
#include "hi5/Hi5.h"
#include "product/DataConversionOp.h"

#include "commands.h"

namespace rack {


///
/* General commands.
 *
 */


/// Tool for selecting data for next command(s), based on paths, quantities and elevations.
/**

Most commands apply implicit input data selection criteria,
typically involving data paths, quantities and/or elevation angles.

Like in general in \b Rack, the parameters of \c --select are ordered, meaning that they can be issued as
a comma-separated
string without explicit key names, as long as they are given in order.
Some remarks on parameters:

- \c path consists of slash '/' separated \e path \e selection \e elements:
  - a leading single slash '/', if rooted matching is desired ie. leading parts of the paths are tested; otherwise trailing parts
  - \c what , \c where, \c how - groups containing attributes
  - <c> dataset<min>:<max> </c>,  <c> data<min>:<max></c> , <c> quality<min>:<max></c>   - indexed groups containing subgroups for actual data and attributes
  - \c data - unindexed groups containing actual data arrays
  - \e combined \e selection \e elements created by concatenating above elements with pipe '|' representing logical OR function (requires escaping on command line)
    - example: <c>what|where|dataset1:3</c>
  - in index ranges, values can be omitted, using invoking default minimum (1) and maximum (0xffff = 65535) as follows:
    - <c>data:</c> equals <c>data1:65535</c>; further the colon ':' can be omitted for \c dataset and \c quality (but not for \c data: , to bypass naming conflict inherent in ODIM )
    - <c>data<index></c> equals <c> data<index>:<index></c> (ie. exact match)
    - <c>data<index>:</c> equals <c> data<index>:65535</c>
    - <c>data:<index></c> equals <c> data1:<index></c>
  - in each combined selection element, only one index range can be given, referring to all indexed elements, and it must be given as the last segment
    - example: <c>what|data|quality1:5</c> matches <c>what</c>, <c>data1:5</c> and <c>quality1:5</c>
    - example: <c>what|where|dataset1:3/what|data1:8</c> -- index ranges \e can \e vary on different levels, ie. in slash-separated elements
- \c quantity is a regular expression
   - example: <c>^DBZH$</c>, accepting \c DBZH only
   - example: <c>(DBZH|TH)</c> accepts \c DBZH and \c TH , but also \c DBZHC and \c WIDTH ...
   - future option: two regular expressions separated by a slash, the latter regExp determining the desired quality quantities
- \c elangle defines the range of antenna elevations accepted, range limits included
    - unlike with path selectors, >c>elangle=<angle></c> abbreviates <c>elangle=<angle>:90</c> (not <c>elangle=<angle>:<angle></c>)
    - notice that radar metadata may contain real(ized) values like 1.000004723, use \c count=1 to pick a single one within a range
- \c count is the upper limit of accepted indices of \c dataset ; typically used with \c elangle

The selection functionality is best explained with examples.

\~remark
./test-content.sh
\~

\include example-select.inc
Note that character escaping is needed for '|' on command line.


Often, the first data array matching the criteria are used.


One can explicitly change the criteria with \c --select (\c -s) command .
For example, in data conversions one may wish to focus on certain quantity -- like \c DBZH or \c VRAD -- not all the data.

The selection command can be applied the most commands processing data.
In computing meteorological products (\ref products) an compositing, it affects the following product only.
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
		//parameters.link("selector", getResources().select, DataSelector().getParameters().getKeys());
		//parameters.separator = 0;
	};

	virtual
	void setParameters(const std::string & params, char assignmentSymbol='=') {
		drain::Logger mout(__FUNCTION__, getName());
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

		drain::Logger mout(__FILE__, getName());

		//const std::string v = StringTools::replace(StringTools::replace(StringTools::replace(value,",","|"), "*",".*"), "?", "[.]");
		/*
		std::string v = value;
		if (v.find('/') != std::string::npos){
			mout.warn() << "short form -Q  supports no slash '/', use --select quantity=" << v << mout.endl;
		}
		StringTools::replace(getTransTable(), v);
		//mout.warn() << v << mout.endl;
		 */
		std::string quantity;
		std::string qualityQuantity;

		StringTools::split2(value, quantity, qualityQuantity,"/");
		StringTools::replace(getTransTable(), quantity);
		StringTools::replace(getTransTable(), qualityQuantity);

		RackResources & resources = getResources();
		if (qualityQuantity.empty()){
			//mout.warn() << "s"  << mout.endl;
			resources.select = "quantity=^(" + quantity + ")$";
		}
		else {
			mout.warn() << "quantity-specific quality ["<< quantity << "]: check unimplemented for ["<< quantity << "]" << mout.endl;
			resources.select = "path=data:/quality:,quantity=^(" + qualityQuantity + ")$";  //???
		}


		resources.errorFlags.unset(RackResources::DATA_ERROR); // resources.dataOk = false;
		//getRegistry().run("select", "quantity=^(" + vField + ")$");
	}

protected:

	static
	const std::map<std::string,std::string> & getTransTable(){

		static std::map<std::string,std::string> m;

		if (m.empty()){
			m[","] = "|";
			m["*"] = ".*";
			m["?"] = ".";
			//m["?"] = "[.]";
		}

		return m;
	}

};






class CmdConvert : public BasicCommand {

public:

	CmdConvert() : BasicCommand(__FUNCTION__, "Convert --select'ed data to scaling and markers set by --encoding") {
	};


	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout; = resources.mout;

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

	CmdCreateDefaultQuality() : BasicCommand(__FUNCTION__, "Creates default quality field. See --undetectWeight and --aDefault"){
		parameters.link("quantitySpecific", quantitySpecific=false, "[0|1]");
	};

	bool quantitySpecific;

	template <class OD>
	void processStructure(Hi5Tree & dst, const ODIMPathList & paths, const drain::RegExp & quantityRegExp) const {

		drain::Logger mout(__FUNCTION__, getName());

		typedef DstType<OD> DT; // ~ PolarDst, CartesianDst

		const QuantityMap & qmap = getQuantityMap();

		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
			mout.info() << *it  << mout.endl;
			Hi5Tree & dstDataSetH5 = dst(*it);
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
					dstQuality.data.setScaling(dstQuality.odim.scale, dstQuality.odim.offset);
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
				dstQuality.data.setScaling(dstQuality.odim.scale, dstQuality.odim.offset);
				//@ dstQuality.updateTree();
			}
			//@  DataTools::updateInternalAttributes(dstDataSetH5);

		}


	}

	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		DataSelector selector(ODIMPathElem::DATASET);
		selector.setParameters(resources.select);
		resources.select.clear();
		const drain::RegExp quantityRegExp(selector.quantity);
		selector.quantity.clear();
		//if (selector.path.empty()) selector.path = "dataset[0-9]+$";  // OLD

		ODIMPathList paths;
		selector.getPaths3(*resources.currentHi5, paths); //, ODIMPathElem::DATASET); // RE2


		if (resources.currentHi5 == resources.currentPolarHi5){
			processStructure<PolarODIM>(*resources.currentHi5, paths, quantityRegExp);
		}
		else if (resources.currentHi5 == &resources.cartesianHi5){
			processStructure<CartesianODIM>(*resources.currentHi5, paths, quantityRegExp);
		}
		else {
			drain::Logger mout(__FUNCTION__, getName());
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

		// drain::Logger mout(__FUNCTION__, getName());
		selector.pathMatcher.setElems(ODIMPathElem::DATASET, ODIMPathElem::DATA);
		//selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElem::DATASET, 0, 0xffff);
		//selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElem::DATA, 0, 0xffff);
		//selector.deriveParameters(params);  // just for checking, also group syntax (dataset:data:...)
		selector.setParameters(params);  // just for checking, also group syntax (dataset:data:...)
		//selector.convertRegExpToRanges(); // transitional op for deprecated \c path

	}

	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout;

		//mout.debug() << "group mask: " << groupFilter << ", selector: " << selector << mout.endl;

		Hi5Tree & dst = *getResources().currentHi5;

		// Step 0
		mout.info() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);

		mout.debug() << "selector: " << selector << ", matcher=" << selector.pathMatcher << mout.endl;

		ODIMPathList paths;
		selector.getPaths3(dst, paths);
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
		drain::Logger mout(__FUNCTION__, getName());
		//selector.groups = ODIMPathElem::ALL_GROUPS; //DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY;
		selector.pathMatcher.clear(); //= "";
		//selector.deriveParameters(params);  // just for checking, also group syntax (dataset:data:...)
		selector.setParameters(params);  // just for checking, also group syntax (dataset:data:...)
		// selector.convertRegExpToRanges(); // transition support...
	}

	void exec() const {

		drain::Logger mout(__FUNCTION__, getName());

		//RackResources & resources = getResources();
		Hi5Tree & dst = *getResources().currentHi5;

		// Step 0
		mout.debug() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);

		for (Hi5Tree::iterator it = dst.begin(); it != dst.end(); ++it){

			if (it->first.is(ODIMPathElem::DATASET)){

				it->second.data.noSave = true;

				for (Hi5Tree::iterator dit = it->second.begin(); dit != it->second.end(); ++dit){
					//if (dit->first.is(ODIMPathElem::DATA)){
					if (dit->first.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
						dit->second.data.noSave = true;
						// Note: also for empty data keep attributes (esp. what:quantity)
						// because quantity-specific quality may be searched for
						for (Hi5Tree::iterator ait = dit->second.begin(); ait != dit->second.end(); ++ait){
							if (!ait->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS))// | ODIMPathElem::ARRAY))
								ait->second.data.noSave = true;
						}
					}
				}
			}

		}

		//hi5::Hi5Base::writeText(dst, std::cerr);

		mout.debug(1) << "selector for saved paths: " << selector << mout.endl;

		ODIMPathList savedPaths;
		selector.getPaths3(dst, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		for (ODIMPathList::iterator it = savedPaths.begin(); it != savedPaths.end(); it++){

			//mout.warn() << "set save: " << *it << mout.endl;

			ODIMPath p;
			for (ODIMPath::iterator pit = it->begin(); pit != it->end(); pit++){
				p << *pit;
				dst(p).data.noSave = false;
			}

			/// Accept also tail (attribute groups)
			if (p.back().belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
				Hi5Tree & d = dst(p);
				for (Hi5Tree::iterator dit = d.begin(); dit != d.end(); dit++){
					//mout.warn() << "also save: " << p << '|' << dit->first << mout.endl;
					if (dit->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS | ODIMPathElem::ARRAY))
						dit->second.data.noSave = false;
				}
			}

		}

		//hi5::Hi5Base::writeText(dst, std::cerr);

		hi5::Hi5Base::deleteNoSave(dst);

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
		parameters.link("src", pathSrc = "", "/group/group2[:attr]");
		parameters.link("dst", pathDst = "", "/group/group2[:attr]");
	};

	// TODO: recognize attr,attr vs path,path
	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();
		if (resources.currentHi5 == NULL){
			mout.error() << "current Hi5 == NULL" << mout.endl;
			return;
		}


		Hi5Tree & dstRoot = *resources.currentHi5;

		ODIMPath path1;
		std::string attr1;
		// std::string attr1value; // debug
		// std::string attr1type;// debug
		std::string value;
		hi5::Hi5Base::parsePath(pathSrc, path1, attr1, value);
		mout.debug() << "path:  " << path1 << ", size=" << path1.size() << mout.endl;

		if (!value.empty()){
			mout.warn() << "value (" << value << ") not empty in " << path1 << mout.endl;
		}

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
		hi5::Hi5Base::parsePath(pathDst, path2, attr2, value);
		if (!value.empty()){
			mout.warn() << "value (" << value << ") not empty in " << path2 << mout.endl;
		}


		if (pathDst.at(0) == ':'){
			mout.note() << " renaming attribute only" << mout.endl;
			path2 = path1;
		}


		mout.debug() << "path1: " << path1 << '(' << path1.size() << ')' << " : " << attr1 << mout.endl;
		mout.debug() << "path2: " << path2 << '(' << path2.size() << ')' << " : " << attr2 << mout.endl;


		if (attr1.empty()){ // RENAME PATHS (swap two paths)

			if (!attr2.empty()){
				mout.error() << "cannot move path '" << path1 << "' to attribute '" << attr2 << "'" << mout.endl;
				return;
			}

			mout.debug() << "renaming path '" << path1 << "' => '" << path2 << "'" << mout.endl;
			//mout.warn() << dstRoot << mout.endl;
			Hi5Tree & dst1 = dstRoot(path1);
			if (!dstRoot.hasPath(path2)) // make it temporary (yet allocating it for now)
				dstRoot(path2).data.noSave = true;
			Hi5Tree & dst2 = dstRoot(path2);

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

			Hi5Tree & dst1 = dstRoot(path1);
			if (!dst1.data.attributes.hasKey(attr1)){
				mout.warn() <<  "attribute '" << attr1 << "' not found, path='" << path1 << "'" << mout.endl;
			}
			Hi5Tree & dst2 = dstRoot(path2);
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
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.warn() << "deprecated command, use '--move' instead" << mout.endl;
		CmdMove::exec();
	};

};




class CmdCompleteODIM : public BasicCommand {

public:

	CmdCompleteODIM() : BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	template <class OD>  // const drain::VariableMap & rootProperties,
	void complete(Hi5Tree & dstH5, OD & od) const {

		drain::Logger mout(__FUNCTION__, getName());

		typedef DstType<OD> DT;

		//DataSelector selector; // todo implement --select

		//mout.debug() << "start upd" << mout.endl;

		DataTools::updateInternalAttributes(dstH5); //, drain::FlexVariableMap());
		// DataTools::updateInternalAttributes(dstH5);

		OD rootODIM(dstH5.data.dataSet.getProperties());

		mout.debug() << "odim: " << rootODIM << mout.endl;


		for (Hi5Tree::iterator it = dstH5.begin(); it != dstH5.end(); ++it){

			mout.debug() << "considering: " << it->first << mout.endl;

			if (it->first.is(ODIMPathElem::DATASET)){ // && selector.dataset.contains(it->first.getIndex())){

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
						if (dstData.odim.scale == 0){
							mout.info() << "setting quantity defaults [" << dstData.odim.quantity << ']' << mout.endl;
							getQuantityMap().setQuantityDefaults(dstData.odim); //, dstData.odim.quantity);
						}
					}

					//mout.note() << "dstData.updateTree: " << dit->first << mout.endl;

					dstData.updateTree2();
					rootODIM.updateLenient(dstData.odim);
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

		drain::Logger mout(__FUNCTION__, __FILE__);

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




/// Modifies metadata (data attributes).
/**

\~exec
	make setODIM.hlp
\~

\include setODIM.hlp

\param assignment - source path consisting of group path, attribute key and value

A full path consist of a slash-separated group path elements followed by an attribute key separated by colon.

Examples:
\include example-assign.inc

 */
class CmdSetODIM : public SimpleCommand<std::string> {

public:

	CmdSetODIM() : SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		if (value.find_first_of("?*()^$") != std::string::npos){
			mout.warn() << "RegExp support suppressed from this version" << mout.endl;
			return;
		}

		mout.debug() << "value: " << value << mout.endl;

		Hi5Tree & src = *(resources.currentHi5);

		static
		const drain::RegExp re("^(.*(data|what|where|how)):([a-zA-Z0-9_]+(=.+)?)$");

		std::string assignment;
		DataSelector s(resources.select);
		resources.select.clear();

		std::vector<std::string> result;
		if (re.execute(value, result)){
			s.pathMatcher.set(value);
			mout.debug() << "pathMatcher: " << s.pathMatcher << mout.endl;
		}
		else {
			s.pathMatcher.set(result[1]);
			mout.info()  << "pathMatcher: " << s.pathMatcher << mout.endl;
			mout.debug() << "selector: " << s << mout.endl;
			assignment = result[3];
			mout.debug(1) << "assignment:  " << assignment    << mout.endl;
		}


		ODIMPathList paths;
		s.getPaths3(src, paths);
		if (paths.empty()){
			mout.debug() << "no paths found, so trying creating one:" << s.pathMatcher  << mout.endl;
			mout.debug(1) << "isSingle:  " << s.pathMatcher.isSingle() << mout.endl;
			ODIMPath path;
			s.pathMatcher.extract(path);
			paths.push_back(path);
		}

		for (ODIMPathList::const_iterator it=paths.begin(); it!= paths.end(); ++it) {
			Hi5Tree & d = src(*it);
			if (assignment.empty()){
				mout.debug() << *it << mout.endl;
			}
			else {
				mout.info() << *it << '=' << assignment << mout.endl;
				hi5::Hi5Base::assignAttribute(d, assignment);
			}
		}

		/// OLD
		// hi5::Hi5Base::readTextLine(*(resources.currentHi5), value);
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

		Hi5Tree & currentHi5 = *(getResources().currentHi5);
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


/*
class CmdDumpEchoClasses : public BasicCommand {

public:

	CmdDumpEchoClasses() : BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
	};

	void exec() const {

		const classtree_t & t = getClassTree();

		drain::JSONtree::writeJSON(t);


	}

};
*/

class CmdDumpMap : public BasicCommand {

public:

	std::string filter;
	std::string filename;

	//options["dumpMap"].syntax = "<regexp>[:<file>]"
	CmdDumpMap() : BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
		parameters.separator = ':'; //s = ":";
		parameters.link("filter", filter = "", "regexp");
		parameters.link("filename", filename = "", "std::string");
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

		drain::Logger mout(__FUNCTION__, __FILE__);

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

		drain::Logger mout(__FUNCTION__, __FILE__);

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


			const ReferenceMap::unitmap_t & u = m.getUnitMap();

			//ostr << "# " << m << mout.endl;


			ostr << "{\n";
			JSONwriter::indent(ostr, 2);
			ostr << "\"title\": \"" << command.getDescription() << '"' << ',' << '\n';
			JSONwriter::indent(ostr, 2);
			ostr << "\"variables\": {";

			/*
			JSONwriter::mapElementsToStream(m, m.getKeyList(), ostr, 4);
			ostr << '\n';
			JSONwriter::indent(ostr, 2);
			ostr << "}\n";
			*/
			//m.toJSON(ostr, 1);

			const ReferenceMap::keylist_t & keys = m.getKeyList();

			char sep=0;

			//JSONtree::tree_t & vars = tree["variables"];

			for (ReferenceMap::keylist_t::const_iterator it = keys.begin(); it!=keys.end(); ++it){

				const drain::Referencer & entry = m[*it];

				if (sep)
					ostr << sep;
				else
					sep = ',';

				ostr << '\n';

				JSONwriter::indent(ostr, 4);
				ostr << "\"" << *it << "\": {\n";

				ReferenceMap::unitmap_t::const_iterator uit = u.find(*it);
				if (uit != u.end()){
					JSONwriter::indent(ostr, 6);
					ostr << "\"title\": ";
					JSONwriter::toStream(uit->second, ostr);
					ostr << ',' << '\n';
				}

				JSONwriter::indent(ostr, 6);
				ostr << "\"value\": ";
				JSONwriter::toStream(entry, ostr);
				ostr << '\n';

				JSONwriter::indent(ostr, 4);
				ostr << '}';

			}

			//JSONwriter::toStream(tree, ostr);


			ostr << '\n';
			JSONwriter::indent(ostr, 2);
			ostr << '}'; // variables



			/*
			if (!command.getType().empty()){
				ostr << ",\n  \"output\": \"" << command.getType() << "\"";
			}
			*/
			//JSONwriter::indent(ostr, 2);
			//ostr << "}\n";

			ostr << "\n}\n";
		}

		//std::cout << std::setw(10) << "Voila!" << std::endl;
	};

};


class CmdFormatOut : public SimpleCommand<std::string> {

public:

	CmdFormatOut() : SimpleCommand<std::string>(__FUNCTION__, "Dumps the formatted std::string to a file or stdout.", "filename","","std::string") {
		//parameters.separators.clear();
		//parameters.link("filename", filename, "");
	};

	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__); // = resources.mout;

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


		if ((value == "")||(value == "-")){
			std::cout << reg.statusFormatter;
		}
		else if (value == "image"){
			//resources.
			if (!resources.select.empty())
				resources.setCurrentImage(resources.select);
			else if (resources.currentImage == NULL){
				mout.warn() << "current image unset, using first applicable path"  << mout.endl;
				mout.note() << "consider -Q <quantity> or --select <params>"  << mout.endl;
				ODIMPath p = resources.setCurrentImage(DataSelector(ODIMPathElem::DATASET, ODIMPathElem::DATA));
				if (!p.empty()){
					mout.note() << "guessed path: " << p << mout.endl;
				}
			}

			if (resources.currentImage){
				drain::image::Image *ptr = (drain::image::Image *)resources.currentImage;
				std::stringstream sstr;
				sstr << reg.statusFormatter;
				ptr->properties[""] = sstr.str();
			}
			else {
				mout.error() << "failed in setting/guessing current image"  << mout.endl;
			}
		}
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
		ostr << "errorFlags: " << getResources().errorFlags << std::endl;

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

		parameters.separator = ',';
		parameters.link("type", odim.type = "C", "storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)");
		parameters.link("gain", odim.scale = 0.0, "scaling coefficient");
		parameters.link("offset", odim.offset = 0.0, "bias");
		parameters.link("undetect", odim.undetect = 0.0, "marker");
		parameters.link("nodata", odim.nodata = 0.0, "marker");

		/// Polar-specific
		parameters.link("rscale", odim.rscale = 0.0, "metres");
		parameters.link("nrays", odim.geometry.height = 0L, "count");
		parameters.link("nbins", odim.geometry.width = 0l, "count");

		/// Experimental, for image processing
		parameters.link("quantity", odim.quantity = "", "string");

		//getQuantityMap().setTypeDefaults(odim, "C");
		odim.setTypeDefaults("C"); // ??
		//odim.setTypeDefaults(typeid(unsigned char));
		//std::cerr << "CmdEncoding.odim:" << odim << std::endl;
		//std::cerr << "CmdEncoding.pars:" << parameters << std::endl;
	};


	virtual  //?
	inline
	void run(const std::string & params){

		Logger mout(__FUNCTION__, getName());

		try {

			/*
			std::string test1;
			std::string test2;
			drain::ReferenceMap refmap;
			refmap.link("type", test1);
			refmap.link("gain", test2);
			refmap.setValues(params);
			mout.warn() << "refmap: " << refmap << mout.endl;

			mout.warn() << "p keys: " << parameters.getKeys() << mout.endl;
			*/

			/// Main action: store the value for later commands.
			getResources().targetEncoding = params;

			/// Also check and warn of unknown parameters
			parameters.setValues(params);  // sets type, perhaps, hence set type defaults and override them with user defs

			//mout.note() << "(user) params: " << params << mout.endl;
			//mout.note() << "parameters:    " << parameters << mout.endl;
			mout.debug() << "odim: " << odim << mout.endl;
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
		parameters.link("exec", getResources().scriptParser.autoExec = -1, "0=false, 1=true, -1=set to true by --script");
	}

};


/*
class CmdDataOk : public BasicCommand {

public:

	CmdDataOk() : BasicCommand(__FUNCTION__, "Status of last select."){
		parameters.link("flag", getResources().dataOk = true);
	};
};
*/

class CmdErrorFlags : public SimpleCommand<std::string> {

public:

	CmdErrorFlags() : SimpleCommand<std::string>(__FUNCTION__, "Status of last select.", "flags"){
		//parameters.link("flags", value);
	};

	virtual inline
	void setParameters(const std::string & params, char assignmentSymbol='=') {
		Logger mout(__FUNCTION__, getName());

		RackResources & resources = getResources();
		if (params.empty() || (params == "0")){
			resources.errorFlags.reset();
		}
		else {
		  resources.errorFlags = params;
		}

		value = params; // TODO: resources.errorFlags.toStr()
	}

protected:



};


class UndetectWeight : public BasicCommand {  // TODO: move to general commands, leave warning here

public:

	UndetectWeight() : BasicCommand(__FUNCTION__, "Set the relative weight of data values assigned 'undetect'."){
		parameters.link("weight", DataCoder::undetectQualityCoeff, "0...1");
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

		Logger mout(__FUNCTION__, getName());

		mout.debug(1) << "params: " << params << mout.endl;

		/// Syntax for recognising text files.
		static
		const drain::RegExp odimSyntax("^--?(/.+)$");

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


class CmdValidate : public SimpleCommand<std::string>  {

public:

	CmdValidate() : SimpleCommand<std::string>(__FUNCTION__, "Read CVS file ",
			"filename", "", "<filename>.cvs"){ //, inputComplete(true) {
	};

	void exec() const;


};

void CmdValidate::exec() const {

	drain::Logger mout(__FILE__, getName());

	ODIMValidator validator;

	drain::Input infile(value);
	std::istream & istr = infile;
	std::string line;
	while (getline(istr, line)){
		//line = drain::StringTools::trim(line, " \n\t\r");
		validator.push_back(ODIMNodeValidator());
		ODIMNodeValidator & nodeValidator = validator.back();
		nodeValidator.assign(line);
		mout.debug(1) << "L: " << line          << mout.endl;
		mout.debug(1) << "V: " << nodeValidator << mout.endl;
		line.clear();
	}

	RackResources & resources = getResources();

	if (resources.currentHi5){

		ODIMPathList dataPaths;

		resources.currentHi5->getPaths(dataPaths); // ALL

		for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){

			ODIMPath p;
			p << ODIMPathElem::ROOT;
			p.appendPath(*it);

			ODIMValidator::const_iterator wit;

			if (p.back().is(ODIMPathElem::ARRAY))
				wit = validator.validate(p, H5I_DATASET);
			else
				wit = validator.validate(p, H5I_GROUP);

			if (wit == validator.end()){
				mout.warn() << "REJECT path: " << p << mout.endl;
				return;
			}
			else {
				mout.debug(1) << "RegExp: " << wit->pathRegExp.toStr() << mout.endl; //.toStr()
				mout.debug() << "ACCEPT path: " << p << mout.endl;
			}

			const Hi5Tree & t = (*resources.currentHi5)(*it);
			if (t.data.dataSet.isEmpty()){
				// std::cout << " GROUP" << '\n';
				const VariableMap & a = t.data.attributes;
				for (VariableMap::const_iterator ait=a.begin(); ait!=a.end(); ++ait){
					std::string attributePath(p);
					attributePath.push_back(p.separator);
					attributePath.append(ait->first);
					ODIMValidator::const_iterator wit = validator.validate(attributePath, H5I_ATTR);
					if (wit == validator.end()){
						mout.warn() << "UNKNOWN attribute: " << attributePath << mout.endl;
						continue;
					}
					else {
						mout.debug(1) << "ACCEPT attribute path: " << attributePath << mout.endl;
					}
					const std::type_info & rType = wit->basetype.getType();
					const std::type_info & aType = ait->second.getType();

					// Compose variable info: <path>:<key>=<value> <type>.
					std::stringstream sstr;
					sstr <<  p << ':' << ait->first << '=';
					ait->second.valueToJSON(sstr);
					if (ait->second.isString())
						sstr << " string";
					else
						sstr << ' ' << drain::Type::call<drain::complexName>(aType);

					/// Type test
					if (aType == rType){
						mout.debug() << "COMPLIANT attribute type: " << sstr.str() << mout.endl;
					}
					else {
						sstr << ", should be " << drain::Type::call<drain::complexName>(rType);
						if ((drain::Type::call<drain::typeIsScalar>(aType) == drain::Type::call<drain::typeIsScalar>(rType)) ||
								(drain::Type::call<drain::typeIsInteger>(aType) == drain::Type::call<drain::typeIsInteger>(rType)) ||
								(drain::Type::call<drain::typeIsFloat>(aType) == drain::Type::call<drain::typeIsFloat>(rType))){
							mout.info() << "Slightly INCOMPLIANT attribute type: " << sstr.str() << mout.endl;
						}
						else if ((drain::Type::call<drain::typeIsScalar>(aType) && !drain::Type::call<drain::typeIsScalar>(rType))){
							mout.note() << "Moderately INCOMPLIANT attribute type: " << sstr.str() << mout.endl;
						}
						else {
							mout.warn() << "INCOMPLIANT attribute type: " << sstr.str() << mout.endl;
						}
					}

					/// Value test
					if (wit->valueRegExp.isSet()){
						sstr << " regExp='" <<  wit->valueRegExp.toStr() << "'";
						if (wit->valueRegExp.test(ait->second)){ // convert on the fly
							mout.debug() << "COMPLIANT attribute value: " << sstr.str() << mout.endl;
						}
						else {
							mout.warn() << "INCOMPLIANT attribute value: " << sstr.str() << mout.endl;
						}
					}
				}
			}
			else {
				mout.debug() << "ACCEPT data: " << p << mout.endl;
			}

		}
	}
	else
		mout.warn() << "no current H5 data" << mout.endl;

}




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
		parameters.link("level", ProductBase::outputDataVerbosity = 0, "0=default,1=intermediate results|2=extra debug results");
	};

	void exec() const {
		drain::Logger mout(__FUNCTION__, __FILE__);
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

		drain::Logger mout(__FUNCTION__, __FILE__);

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
		parameters.link("intermediate", ProductBase::outputDataVerbosity = 0, "store intermediate images");
		//parameters.link("append",  ProductBase::appendResults = "", "|data|dataset");
		parameters.link("append",  append, "|data|dataset (deprecated)");
	};

	//int level;
	std::string append;

	virtual
	void exec() const {
		drain::Logger mout(__FUNCTION__, __FILE__);
		DetectorOp::STORE = (ProductBase::outputDataVerbosity>0);
		if (!append.empty()){
			ProductBase::appendResults.set(append);
			mout.warn() << "option 'append' is deprecating, use --append <path> instead." << mout.endl;
		}
	};

};


class CmdQuantityConf : public BasicCommand {

public:

	// Odim used for interface only. Zero used internally.
	CmdQuantityConf(): BasicCommand(__FUNCTION__, "1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair") {
		//parameters.separator = ',';
		parameters.link("quantity:type", quantityType = "", "quantity (DBZH,VRAD,...) and storage type (C,S,d,f,...)");
		//parameters.link("type", odim.type, "C", "storage type (C,S,d,f,...)");
		parameters.link("gain", odim.scale = 0.0, "scaling coefficient");
		parameters.link("offset", odim.offset = 0.0, "bias");
		parameters.link("undetect", odim.undetect = 0.0, "marker");
		parameters.link("nodata", odim.nodata = 0.0, "marker");
		parameters.link("zero", zero = std::numeric_limits<double>::min(), "value");// what about max?

	}

	inline
	void run(const std::string & params = ""){

		Logger mout(__FUNCTION__, getName());

		setParameters(params); // used for syntacx checking (AND zero)
		// std::string prefix; // dummy; same as quantityType, eg. "DBZH" or "DBZH:S"
		std::string quantity;
		std::string args;
		drain::StringTools::split2(params, quantity, args, ":");
		std::string type;
		drain::StringTools::split2(quantityType, quantity, type, ":");

		//const size_t i = quantityType.find(':');
		//const std::string quantity = quantityType.substr(0, i);
		//const std::string type = (i != std::string::npos) ? quantityType.substr(i+1) : std::string("");
		//const std::string args = (i != std::string::npos) ? params.substr(i+1) : std::string();

		mout.debug() << quantity << "(" << type << "): " << args << mout.endl;

		QuantityMap & m = getQuantityMap();

		if (!quantity.empty()){
			editQuantityConf(quantity, type, args);
		}
		else {
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

		quantityType.clear();
		zero = std::numeric_limits<double>::min();  // what about max?

	}

protected:

	std::string quantityType;

	EncodingODIM odim;

	double zero;


	void editQuantityConf(const std::string & quantity, const std::string & type, const std::string & args){

		Logger mout(__FUNCTION__, getName());

		if (quantity.empty()){
			mout.error() << "quantity empty" << mout.endl;
			return;
		}

		mout.debug() << quantity << "(" << type << "): " << args << mout.endl;

		QuantityMap & m = getQuantityMap();
		Quantity & q = m[quantity];

		if (zero != std::numeric_limits<double>::min()){
			if (std::isnan(zero)) // DOES NOT WORK.. convert toStr to double NaN
				q.unsetZero();
			else
				q.setZero(zero);
		}

		if (type.length() == 1){

			mout.debug(1) << "setting default type " << type << mout.endl;

			const char typecode = type.at(0);
			q.defaultType = typecode;

			if (!args.empty()){
				// Note: resets scaling; expect it to be reset here
				EncodingODIM & currentEncoding = q.set(typecode);
				currentEncoding.setScaling(1.0, 0); //.updateValues(params);

				EncodingODIM newEncoding(q.get(typecode));
				newEncoding.addShortKeys();
				newEncoding.setValues(args);
				mout.debug() << "setting other params " << newEncoding << mout.endl;

				currentEncoding.updateFromCastableMap(newEncoding);
				//q.set(typecode).updateValues(args);
			}

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
		parameters.link("level", level = 5);
		parameters.link("imageLevel", imageLevel = 4);
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
	//static RackLetAdapter<CmdDataOk> dataOk("dataOk", -1);
	static RackLetAdapter<CmdErrorFlags> errorFlags;
	static RackLetAdapter<UndetectWeight> undetectWeight("undetectWeight");

	static RackLetAdapter<CmdSelectQuantity> cmdSelectQuantity("quantity",'Q');
	static RackLetAdapter<CmdCheckType> cmdCheckType;
	static RackLetAdapter<CmdValidate> cmdValidate;
	static RackLetAdapter<CmdCreateDefaultQuality> cmdCreateDefaultQuality;
	static RackLetAdapter<CmdCompleteODIM> cmdCompleteODIM;
	static RackLetAdapter<CmdConvert> cmdConvert;
	static RackLetAdapter<CmdDelete> cmdDelete;
	static RackLetAdapter<CmdDumpMap> cmdDumpMap; // obsolete?
	//static RackLetAdapter<CmdDumpEchoClasses> cmdDumpEchoClasses; // obsolete?

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
