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
#include "drain/util/Output.h"
#include "drain/util/JSONwriter.h"
#include "drain/util/Sprinter.h"

#include "drain/image/Image.h"
#include "drain/image/TreeSVG.h"

#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"


#include "rack.h"
#include "data/ODIM.h"
#include "data/ODIMValidator.h"
#include "data/DataCoder.h"
#include "data/EchoClass.h"
#include "hi5/Hi5.h"

#include "product/DataConversionOp.h"
#include "andre/DetectorOp.h"

#include "resources.h"
#include "commands.h"

namespace rack {


///
/* General commands.
 *
 */

class CmdBaseSelective : public drain::SimpleCommand<> {

protected:

	CmdBaseSelective(const std::string & name, const std::string & description) :
		drain::SimpleCommand<>(name, description, "selector", DataSelector().getParameters().getKeys()){
		//parameters.separator = 0;
	};

	/*
	CmdBaseSelective(const CmdBaseSelective & cmd) : drain::SimpleCommand<>(cmd){
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};
	*/

protected:


};






/// Tool for selecting data for next command(s), based on paths, quantities and elevations.
/**

Most commands apply implicit input data selection criteria, typically involving data paths, quantities and/or elevation angles.

Like in general in \b Rack, the parameters of \c --select are ordered, meaning that they can be issued as
a comma-separated string without explicit key names, as long as they are given in order.
The parameters are:

- \c path consisting of slash '/' separated \e path \e selection \e elements:
  - a leading single slash '/', if rooted matching is desired ie. leading parts of the paths are tested; otherwise trailing parts
  - \c what , \c where, \c how - groups containing attributes
  - <c> dataset{min}:{max} </c>,  <c> data{min}:{max}</c> , <c> quality{min}:{max}</c>   - indexed groups containing subgroups for actual data and attributes
  - \c data - unindexed groups containing actual data arrays
  - \e combined \e selection \e elements created by concatenating above elements with pipe '|' representing logical OR function (requires escaping on command line)
    - example: <c>what|where|dataset1:3</c>
  - in index ranges, values can be omitted, using invoking default minimum (1) and maximum (0xffff = 65535) as follows:
    - <c>data:</c> equals <c>data1:65535</c>; further the colon ':' can be omitted for \c dataset and \c quality (but not for \c data: , to bypass naming conflict inherent in ODIM )
    - <c>data{index}</c> equals <c> data{index}:{index}</c> (ie. exact match)
    - <c>data{index}:</c> equals <c> data{index}:65535</c>
    - <c>data:{index}</c> equals <c> data1:{index}</c>
  - in each combined selection element, only one index range can be given, referring to all indexed elements, and it must be given as the last segment
    - example: <c>what|data|quality1:5</c> matches <c>what</c>, <c>data1:5</c> and <c>quality1:5</c>
    - example: <c>what|where|dataset1:3/what|data1:8</c> -- index ranges \e can \e vary on different levels, ie. in slash-separated elements
- \c quantity is a regular expression
   - example: <c>^DBZH$</c>, accepting \c DBZH only
   - example: <c>(DBZH|TH)</c> accepts \c DBZH and \c TH , but also \c DBZHC and \c WIDTH ...
   - future option: two regular expressions separated by a slash, the latter regExp determining the desired quality quantities
- \c elangle defines the range of antenna elevations accepted, range limits included
    - unlike with path selectors, <c>elangle={angle}</c> abbreviates <c>elangle={angle}:90</c> (not <c>elangle={angle}:{angle}</c>)
    - notice that radar metadata may contain real(ized) values like 1.000004723, use \c count=1 to pick a single one within a range
- \c count is the upper limit of accepted indices of \c dataset ; typically used with \c elangle

The selection functionality is best explained with examples.

\~remark
./test-content.sh
\~

\include example-select.inc
Note that escaping special characters like '|' is often on command line.


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
/// NOTE: test is good for checking immediately. But \c value is needed to store
class CmdSelect : public  CmdBaseSelective { //drain::BasicCommand {

public:

	CmdSelect() : CmdBaseSelective(__FUNCTION__, "Data selection for the next operation."){
	};


	/// Only checks the validity of selector.
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, getName());

		// mout.warn() << "setting: " << value << mout.endl;
		DataSelector  test;

		try {

			// ImageSelector itest;
			if (ctx.log.getVerbosity() > LOG_DEBUG){
				mout.special() << "testing: " << value << mout.endl;
				test.setParameters(value);
				mout.special() << "testing: => " <<  test << mout.endl;
				ODIMPathList paths;
				const Hi5Tree & src = ctx.getMyHi5();
				test.getPaths(src, paths);
				mout.special() << "path count => " <<  paths.size() << mout.endl;
			}

			// std::cerr << __FILE__ << drain::sprinter(paths) << '\n';
			ctx.select = value;

		}
		catch (const std::exception &e) { // consider generalising this
			mout.warn()  << "keys: " << test.getParameters().getKeys() << mout.endl;
			mout.warn()  << "msg: "  << e.what() << mout.endl;
			mout.error() << "error in: " << value << mout.endl;
		}
		// mout.special() << getName() << ctx.getId() << ':' << ctx.select << mout.endl;

	};


};

/// Set selection criteria strictly to one \c quantity .
/**
Selecting quantities only is frequently needed, so there is a dedicated command \c --selectQuantity (\c -Q ) which
accepts comma-separated simple patterns (with * and ?) instead of regular expressions. For example, \c -Q \c DBZH*,QIND
is equal to \c --select \c quantity='^(DBZH.*|QIND)$' .
*/
class CmdSelectQuantity : public  drain::SimpleCommand<std::string> {

public:

	CmdSelectQuantity() : drain::SimpleCommand<std::string>(__FUNCTION__, "Like --select quantity=... but with patterns (not regexps)", "quantities","","quantity[,quantity2,...]"){

	};

	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, getName().c_str());

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

		drain::StringTools::split2(value, quantity, qualityQuantity,"/");
		drain::StringTools::replace(getTransTable(), quantity);
		drain::StringTools::replace(getTransTable(), qualityQuantity);


		if (qualityQuantity.empty()){
			//mout.warn() << "s"  << mout.endl;
			ctx.select = "quantity=^(" + quantity + ")$";
		}
		else {
			mout.warn() << "quantity-specific quality ["<< quantity << "]: check unimplemented for ["<< quantity << "]" << mout.endl;
			ctx.select = "path=data:/quality:,quantity=^(" + qualityQuantity + ")$";  //???
		}


		ctx.statusFlags.unset(drain::StatusFlags::DATA_ERROR); // resources.dataOk = false;
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






class CmdConvert : public  drain::BasicCommand {

public:

	CmdConvert() :  drain::BasicCommand(__FUNCTION__, "Convert --select'ed data to scaling and markers set by --encoding") {
	};


	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = resources.mout; = resources.mout;

		if (ctx.currentHi5 == &ctx.cartesianHi5){
			convertCurrentH5<CartesianODIM>();
		}
		else if (ctx.currentHi5 == ctx.currentPolarHi5) {
			convertCurrentH5<PolarODIM>();
		}
		else {
			mout.warn() << " currentHi5 neither Polar nor Cartesian " << mout.endl;
		}
		ctx.currentImage = NULL;
		ctx.currentGrayImage = NULL;

	};

protected:

	template <class OD>
	void convertCurrentH5() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		DataConversionOp<OD> op;
		op.setEncodingRequest(ctx.targetEncoding);
		ctx.targetEncoding.clear();

		// mout.debug() << op.encodingRequest << mout.endl;
		// mout.warn() << op << mout.endl;
		op.dataSelector.consumeParameters(ctx.select);

		op.processH5(*ctx.currentHi5, *ctx.currentHi5);
		DataTools::updateInternalAttributes(*ctx.currentHi5);

	}

};

class CmdCreateDefaultQuality : public drain::SimpleCommand<bool> { //public  drain::BasicCommand {

public:

	CmdCreateDefaultQuality() :  drain::SimpleCommand<bool> (__FUNCTION__,
			"Creates default quality field. See --undetectWeight and --aDefault", "quantitySpecific", false){
		//parameters.link("quantitySpecific", quantitySpecific=false, "[0|1]");
	};

	CmdCreateDefaultQuality(const CmdCreateDefaultQuality & cmd): drain::SimpleCommand<bool>(cmd){

	}
	//bool quantitySpecific;

	template <class OD>
	void processStructure(Hi5Tree & dst, const ODIMPathList & paths, const drain::RegExp & quantityRegExp) const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		typedef DstType<OD> DT; // ~ PolarDst, CartesianDst

		const QuantityMap & qmap = getQuantityMap();

		const bool & quantitySpecific = value;

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
					//dstQuality.data.setScaling(dstQuality.odim.scaling.scale, dstQuality.odim.scaling.offset);
					dstQuality.data.setScaling(dstQuality.odim.scaling);// needed?
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
				//dstQuality.data.setScaling(dstQuality.odim.scaling.scale, dstQuality.odim.scaling.offset);
				dstQuality.data.setScaling(dstQuality.odim.scaling); // needed?
				//@ dstQuality.updateTree();
			}
			//@  DataTools::updateInternalAttributes(dstDataSetH5);

		}


	}

	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		DataSelector selector(ODIMPathElem::DATASET);
		selector.consumeParameters(ctx.select);

		const drain::RegExp quantityRegExp(selector.quantity);
		selector.quantity.clear();
		//if (selector.path.empty()) selector.path = "dataset[0-9]+$";  // OLD

		ODIMPathList paths;
		selector.getPaths(*ctx.currentHi5, paths); //, ODIMPathElem::DATASET); // RE2


		if (ctx.currentHi5 == ctx.currentPolarHi5){
			processStructure<PolarODIM>(*ctx.currentHi5, paths, quantityRegExp);
		}
		else if (ctx.currentHi5 == &ctx.cartesianHi5){
			processStructure<CartesianODIM>(*ctx.currentHi5, paths, quantityRegExp);
		}
		else {
			drain::Logger mout(ctx.log, __FUNCTION__, getName());
			mout.warn() << "no data, or data structure other than polar volume or Cartesian" << mout.endl;
		}

		DataTools::updateInternalAttributes(*ctx.currentHi5);
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
class CmdDelete : public CmdBaseSelective {

public:

	CmdDelete() :  CmdBaseSelective(__FUNCTION__, "Deletes selected parts of h5 structure."){
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		//Hi5Tree & dst = *getResources().currentHi5;
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = resources.mout;

		DataSelector selector(ODIMPathElem::DATASET, ODIMPathElem::DATA);
		selector.setParameters(value);
		//selector.pathMatcher.setElems(ODIMPathElem::DATASET, ODIMPathElem::DATA);

		mout.warn() << "selector: " << selector << mout.endl;
		//mout.debug() << "group mask: " << groupFilter << ", selector: " << selector << mout.endl;

		Hi5Tree & dst = *ctx.currentHi5;

		// Step 0
		mout.info() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);

		mout.debug() << "selector: " << selector << ", matcher=" << selector.pathMatcher << mout.endl;

		ODIMPathList paths;
		selector.getPaths(dst, paths);
		mout.info() << "deleting " << paths.size() << " substructures" << mout.endl;
		for (ODIMPathList::const_reverse_iterator it = paths.rbegin(); it != paths.rend(); it++){
			mout.debug() << "deleting: " << *it << mout.endl;
			dst.erase(*it);
		}
	};

private:

	//DataSelector selector;


};


///
/**

Metadata groups (\c what, \c where, \c how) are preserved or deleted together with their
parent groups.

Examples:
\include example-keep.inc

 */
class CmdKeep : public  CmdBaseSelective {

public:

	/// Keeps a part of the current h5 structure, deletes the rest. Path and quantity are regexps.
	CmdKeep() :  CmdBaseSelective(__FUNCTION__, "Keeps selected part of data structure, deletes rest."){
	};



	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);  // *ctx.currentHi5;

		// Step 0
		mout.debug() << "delete existing no-save structures " << mout.endl;
		hi5::Hi5Base::deleteNoSave(dst);

		markNoSave(dst);


		DataSelector selector;
		selector.setParameters(value);

		//hi5::Hi5Base::writeText(dst, std::cerr);

		mout.debug2() << "selector for saved paths: " << selector << mout.endl;

		ODIMPathList savedPaths;
		selector.getPaths(dst, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		for (ODIMPathList::iterator it = savedPaths.begin(); it != savedPaths.end(); it++){

			mout.debug2() << "set save through path: " << *it << mout.endl;
			ODIMPath p;
			for (ODIMPath::iterator pit = it->begin(); pit != it->end(); pit++){
				p << *pit;
				dst(p).data.noSave = false;
			}
			//mout.debug() << "marked for save: " << *it << mout.endl;

			// Accept also tail (attribute groups)
			//if (it->back().isIndexed()){ // belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){ or: DATASET

			Hi5Tree & d = dst(*it);
			for (Hi5Tree::iterator dit = d.begin(); dit != d.end(); dit++){
				if (dit->first.is(ODIMPathElem::ARRAY)){
					mout.debug2() << "also save: " << p << '|' << dit->first << mout.endl;
					// if (dit->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS))
					dit->second.data.noSave = false;
				}
			}
			//
		}

		// debug: hi5::Hi5Base::writeText(dst, std::cerr);

		hi5::Hi5Base::deleteNoSave(dst);

	};

protected:

	// Marks CHILDREN of src for deleting
	void markNoSave(Hi5Tree &src, bool noSave=true) const {

		//drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it) {
			//if (it->first.isIndexed()){
			if (!it->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
				it->second.data.noSave = noSave;
				markNoSave(it->second, noSave);
			}
		}

	}

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
class CmdMove :  public  drain::BasicCommand {

public:

	/// Default constructor
	CmdMove() :  drain::BasicCommand(__FUNCTION__, "Rename or move data groups and attributes."){
		parameters.link("src", pathSrc = "", "/group/group2[:attr]");
		parameters.link("dst", pathDst = "", "/group/group2[:attr]");
	};

	CmdMove(const CmdMove & cmd) :  drain::BasicCommand(cmd){
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};

	// TODO: recognize attr,attr vs path,path
	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		if (ctx.currentHi5 == NULL){
			mout.error() << "current Hi5 == NULL" << mout.endl;
			return;
		}


		Hi5Tree & dstRoot = *ctx.currentHi5;

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

//class CmdRename : public  drain::BasicCommand {
class CmdRename : public CmdMove {

public:

	//CmdRename() :  drain::BasicCommand(__FUNCTION__, "Move/rename paths and attributes"){};

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		mout.obsolete() << "--> '--move' " << mout.endl;
		CmdMove::exec();
	};

};




class CmdCompleteODIM : public  drain::BasicCommand {

public:

	CmdCompleteODIM() : drain::BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	template <class OD>
	void complete(Hi5Tree & dstH5, OD & od) const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		typedef DstType<OD> DT;

		//DataSelector selector; // todo implement --select

		//mout.debug() << "start upd" << mout.endl;

		DataTools::updateInternalAttributes(dstH5); //, drain::FlexVariableMap());
		// DataTools::updateInternalAttributes(dstH5);

		OD rootODIM(dstH5.data.dataSet.getProperties());

		mout.debug() << "Root odim: " << rootODIM << mout.endl;


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
						if (dstData.odim.scaling.scale == 0){
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

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);


		if (ctx.inputHi5[ODIMPathElem::WHAT].data.attributes["object"].toStr() == "COMP"){
			ctx.inputHi5.swap(ctx.cartesianHi5);
			ctx.currentHi5 = &ctx.cartesianHi5;
		}

		if (ctx.currentHi5 == ctx.currentPolarHi5){
			PolarODIM odim;
			complete(*ctx.currentHi5, odim);
		}
		else if (ctx.currentHi5 == &ctx.cartesianHi5){
			CartesianODIM odim;
			complete(*ctx.currentHi5, odim);
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
class CmdSetODIM : public drain::SimpleCommand<std::string> {

public:

	CmdSetODIM() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		if (value.find_first_of("?*()^$") != std::string::npos){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.warn() << "RegExp support suppressed from this version" << mout.endl;
			return;
		}

		if (value.empty()){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.error() << "empty command" << mout.endl;
			//throw std::runtime_error();
			return;
		}

		if (value.at(0) != '/'){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.warn()  << "Could not handle argument '" << value << "': no leading '/' " << mout.endl;
			mout.error() << "Dynamic command handler (" << getName() << ") failed " << mout.endl;
			//throw std::runtime_error();
			return;
		}


		mout.debug() << "value: " << value << mout.endl;

		Hi5Tree & src = *(ctx.currentHi5);

		mout.debug() << "try to track if argument is a path, an attribute, or a combination of both" << mout.endl;

		static
		const drain::RegExp re("^(.*(data|what|where|how)):([a-zA-Z0-9_]+(=.*)?)$");

		std::string assignment;
		DataSelector selector; //(resources.select);
		selector.consumeParameters(ctx.select);

		std::vector<std::string> result;
		if (re.execute(value, result)){
			selector.pathMatcher.assign(value);
			mout.debug() << "pathMatcher: " << selector.pathMatcher << mout.endl;
		}
		else {
			selector.pathMatcher.assign(result[1]);
			mout.info()  << "pathMatcher: " << selector.pathMatcher << mout.endl;
			mout.debug() << "selector: " << selector << mout.endl;
			assignment = result[3];
			mout.debug2() << "assignment:  " << assignment    << mout.endl;
		}


		ODIMPathList paths;
		selector.getPaths(src, paths);
		if (paths.empty()){
			mout.debug() << "no paths found, so trying creating one:" << selector.pathMatcher  << mout.endl;
			mout.debug2() << "isLiteral:  " << selector.pathMatcher.isLiteral() << mout.endl;
			ODIMPath path;
			selector.pathMatcher.extract(path);
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

		DataTools::updateInternalAttributes(*(ctx.currentHi5));

	};

};






// Various dumps


/*
class CmdDumpEchoClasses : public drain::BasicCommand {

public:

	CmdDumpEchoClasses() : drain::BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
	};

	void exec() const {

		const classtree_t & t = getClassTree();

		drain::JSONtree::writeJSON(t);


	}

};
*/

class CmdDumpMap : public drain::BasicCommand {

public:

	std::string filter;
	std::string filename;

	//options["dumpMap"].syntax = "<regexp>[:<file>]"
	CmdDumpMap() : drain::BasicCommand(__FUNCTION__, "Dump variable map, filtered by keys, to std or file.") {
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
		throw std::runtime_error(name + ": unimplemented drain::SingleParamCommand");
		ofstr.close();

	};

};






/*
class CmdHelpRack : public drain::CmdHelp {
public:
	CmdHelpRack() : drain::CmdHelp(std::string(__RACK__) + " - a radar data processing program", "Usage: rack <input> [commands...] -o <outputFile>") {};
};
*/


class CmdHelpExample : public drain::SimpleCommand<std::string> {

public:

	CmdHelpExample() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump example of use and exit.", "keyword", "") {
	};


	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		std::ostream & ostr = std::cout;

		// const drain::CommandRegistry::map_t & map = drain::getRegistry().getMap();
		const drain::CommandBank & cmdBank = drain::getCommandBank();
		const drain::CommandBank::map_t & cmdMap = cmdBank.getMap();
		// drain::getCommandBank().re

		// Light strip (could use
		const std::string key = cmdBank.resolveFull(value);
		//const std::string key = value.substr(value.find_first_not_of('-'));

		if (key != value)
			mout.debug() << "resolved: " << value << " -> "<< key << mout.endl;

		drain::CommandBank::map_t::const_iterator it = cmdMap.find(key);

		if (it != cmdMap.end()){

			const drain::Command & d = it->second->getSource();
			const drain::ReferenceMap & params = d.getParameters();

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
			mout.fail() << "command " << value << " not found" << mout.endl;
		}

		exit(0);
	};




};

// static drain::CommandEntry<CmdHelpExample> cmdHelpExample("helpExample", 0);

class CmdJSON : public drain::SimpleCommand<std::string> { // drain::BasicCommand {
public:

	CmdJSON() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump to JSON.", "property", "", ""){
	};

	virtual
	//void run(const std::string & params = "") {
	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		//drain::CommandRegistry & reg = drain::getRegistry();
		drain::CommandBank & cmdBank = drain::getCommandBank();


		std::ostream & ostr = std::cout;

		if (value.empty()){
			drain::JSONwriter::mapToStream(cmdBank.getMap(), ostr, 2);
			//reg.toJSON(ostr);
		}
		else {
			if (!cmdBank.has(value)){
				mout.error() << "no such key: " << value << mout.endl;
				return;
			}

			const drain::Command & command = cmdBank.get(value);
			const drain::ReferenceMap & m = command.getParameters();


			const drain::ReferenceMap::unitmap_t & u = m.getUnitMap();

			//ostr << "# " << m << mout.endl;


			ostr << "{\n";
			drain::JSONwriter::indent(ostr, 2);
			ostr << "\"title\": \"" << command.getDescription() << '"' << ',' << '\n';
			drain::JSONwriter::indent(ostr, 2);
			ostr << "\"variables\": {";

			/*
			JSONwriter::mapElementsToStream(m, m.getKeyList(), ostr, 4);
			ostr << '\n';
			JSONwriter::indent(ostr, 2);
			ostr << "}\n";
			*/
			//m.toJSON(ostr, 1);

			const drain::ReferenceMap::keylist_t & keys = m.getKeyList();

			char sep=0;

			//JSONtree::tree_t & vars = tree["variables"];

			for (drain::ReferenceMap::keylist_t::const_iterator it = keys.begin(); it!=keys.end(); ++it){

				const drain::Referencer & entry = m[*it];

				if (sep)
					ostr << sep;
				else
					sep = ',';

				ostr << '\n';

				drain::JSONwriter::indent(ostr, 4);
				ostr << "\"" << *it << "\": {\n";

				drain::ReferenceMap::unitmap_t::const_iterator uit = u.find(*it);
				if (uit != u.end()){
					drain::JSONwriter::indent(ostr, 6);
					ostr << "\"title\": ";
					drain::JSONwriter::toStream(uit->second, ostr);
					ostr << ',' << '\n';
				}

				drain::JSONwriter::indent(ostr, 6);
				ostr << "\"value\": ";
				drain::JSONwriter::toStream(entry, ostr);
				ostr << '\n';

				drain::JSONwriter::indent(ostr, 4);
				ostr << '}';

			}

			//drain::JSONwriter::toStream(tree, ostr);


			ostr << '\n';
			drain::JSONwriter::indent(ostr, 2);
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


class CmdEcho : public drain::SimpleCommand<std::string> {

public:

	CmdEcho() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dumps a formatted std::string to stdout.", "format","","std::string") {
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = resources.mout;

		//std::string format = value;
		// format = drain::StringTools::replace(format, "\\n", "\n");
		// format = drain::StringTools::replace(format, "\\t", "\t");
		// std::cout << format << std::endl;
		// std::cout << ctx.getStatus() << std::endl;
		//drain::StringMapper statusFormatter("[a-zA-Z0-9:_]+");
		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(value+'\n', true);
		statusFormatter.toStream(std::cout, ctx.getStatusMap());

	}
};

class CmdFormatOut : public drain::SimpleCommand<std::string> {

public:

	CmdFormatOut() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dumps the formatted std::string to a file or stdout.", "filename","","std::string") {
		//parameters.separators.clear();
		//parameters.link("filename", filename, "");
	};

	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // = resources.mout;

		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(ctx.formatStr, true);

		mout.deprecating() = "Use  -o / --outputFile [file|'-'] ";

		if (value == "log"){
			mout.unimplemented() << "Logging: future option" <<  mout.endl;
			//statusFormatter.toStream(ctx.log.getOstr, ctx.getStatus());
			//std::cout << statusFormatter;
		}
		else if (value == "image"){
			//resources.
			mout.deprecating() << "this command is unneed (--format is sufficient)" << mout.endl;
		}
		else {
			std::string outFileName;
			if ((value == "")||(value == "-")){
				outFileName = "-";
			}
			else
				outFileName = ctx.outputPrefix + value;
			mout.info() << "writing " << outFileName << mout.endl;
			drain::Output ofstr(outFileName);
			//mout.warn() <<  ctx.getStatus() << mout.endl;
			//std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			if (ofstr)
				statusFormatter.toStream(ofstr, ctx.getStatusMap());
			else
				mout.warn() << "write error: " << outFileName << mout.endl;
			//strm.toStream(ofstr, cmdStatus.statusMap.exportMap());
			//ofstr.close();
		}

		//mout.warn() << "after expansion: " << r.statusFormatter << mout.endl;
		//r.statusFormatter.debug(std::cerr, r.getStatusMap());

	};

};


/// Consider long names, and then addShortKeys()
class EncodingBag : public PolarODIM {
public:

	EncodingBag(): PolarODIM(0){
		separator = ',';
		link("type", type = "C", "storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)");
		link("gain", scaling.scale = 0.0, "scaling coefficient");
		link("offset", scaling.offset = 0.0, "bias");
		link("undetect", undetect = 0.0, "marker");
		link("nodata", nodata = 0.0, "marker");

		/// Polar-specific
		link("rscale", rscale = 0.0, "metres");
		link("nrays", area.height = 0L, "count");
		link("nbins", area.width = 0l, "count");

		/// New: for image processing
		link("quantity", quantity = "", "string");

	}

	EncodingBag(const EncodingBag & bag): PolarODIM(0){
		copyStruct(bag, bag, *this);
	}

};

/// Facility for validating and storing desired technical (non-meteorological) user parameters.
/*
 *  Quick-checks values immediately.
 *  To consider: PolarODIM and CartesianODIM ?
 */
class CmdEncoding : protected EncodingBag, public drain::SimpleCommand<> { //BasicCommand {

public:

	inline
	CmdEncoding() : drain::SimpleCommand<>(__FUNCTION__, // {  //drain::BasicCommand(__FUNCTION__,
			"Sets encodings parameters for polar and Cartesian products, including composites.",
			"", "", EncodingBag().getKeys() // NOTE: latent-multiple-key case
	) {
		parameters.separator = 0;
		/*
		parameters.separator = ',';
		parameters.link("type", odim.type = "C", "storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)");
		parameters.link("gain", odim.scaling.scale = 0.0, "scaling coefficient");
		parameters.link("offset", odim.scaling.offset = 0.0, "bias");
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
		 *
		 */
	};


	virtual
	inline
	void exec() const { //(const std::string & params){

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());


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


			EncodingBag enc;

			/// Also check and warn of unknown parameters
			enc.setValues(value);  // sets type, perhaps, hence set type defaults and override them with user defs

			//mout.note() << "(user) params: " << params << mout.endl;
			//mout.note() << "parameters:    " << parameters << mout.endl;
			mout.debug() << "odim: " << enc << mout.endl;
			//odim.setTypeDefaults();

			/// Main action: store the value for later commands.
			ctx.targetEncoding = value;
			mout.debug() << "ctx.targetEncoding " << ctx.targetEncoding << mout.endl;

			// Reassign (to odim).
			//parameters.setValues(params);
			//mout.note() << "Re-assigned parameters: " << parameters << mout.endl;

		}
		catch (const std::runtime_error & e) {

			mout.warn() << "Could not set odim" << mout.endl;
			mout.note() << "pars: " << parameters << mout.endl;
			//mout.warn() << "odim: " << enc << mout.endl;
			mout.error() << e.what() << mout.endl;

		}

		// std::cerr << "CmdEncoding.odim: " << odim << std::endl;
		// std::cerr << "CmdEncoding.pars: " << parameters << std::endl;

	};



};
// extern drain::CommandEntry<CmdEncoding> cmdEncoding;




/*
class CmdAutoExec : public drain::BasicCommand {

public:

	CmdAutoExec() : drain::BasicCommand(__FUNCTION__, "Execute script automatically after each input. See --script") {
		parameters.link("exec", getResources().scriptParser.autoExec = -1, "0=false, 1=true, -1=set to true by --script");
	}

};
*/


/*
class CmdDataOk : public drain::BasicCommand {

public:

	CmdDataOk() : drain::BasicCommand(__FUNCTION__, "Status of last select."){
		parameters.link("flag", getResources().dataOk = true);
	};
};
*/

class CmdErrorFlags : public drain::SimpleCommand<std::string> {

public:

	CmdErrorFlags() : drain::SimpleCommand<std::string>(__FUNCTION__, "Status of last select.", "flags"){
		//parameters.link("flags", value);
	};


	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		//drain::Logger mout(ctx.log, __FUNCTION__, getName());

		if (value.empty() || (value == "0")){
			ctx.statusFlags.reset();
		}
		else {
			ctx.statusFlags.set(value);
		}
	}

protected:



};


class UndetectWeight : public drain::BasicCommand {  // TODO: move to general commands, leave warning here

public:

	UndetectWeight() : drain::BasicCommand(__FUNCTION__, "Set the relative weight of data values assigned 'undetect'."){
		parameters.link("weight", DataCoder::undetectQualityCoeff, "0...1");
	};

	UndetectWeight(const UndetectWeight & cmd) : BasicCommand(cmd){
		parameters.copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK );
	}
};

/// Default handler for requests without own handler. Handles options that are recognized as 1) files to be read or 2) ODIM properties to be assigned in current H5 structure.
/**
 *   ODIM properties can be set with
 *   - \c --/dataset2/data2/what:quantity=DBZH
 *
 */
/*
class CmdDefaultHandler : public drain::SimpleCommand<>{
public:


	//CmdDefaultHandler() : drain::BasicCommand(drain::getRegistry().DEFAULT_HANDLER, "Delegates plain args to --inputFile and args of type --/path:attr=value to --setODIM."){};
	// drain::getRegistry().DEFAULT_HANDLER, 0,
	CmdDefaultHandler() : drain::SimpleCommand<>(__FUNCTION__, "Delegates plain args to --inputFile and args of type --/path:attr=value to --setODIM."){};

	virtual
	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		mout.debug2() << "params: " << value << mout.endl;

		/// Syntax for recognising text files.
		static
		const drain::RegExp odimSyntax("^--?(/.+)$");

		if (value.empty()){
			mout.error() << "Empty parameters" << mout.endl;
		}
		else if (odimSyntax.execute(value) == 0) {
			//mout.warn() << "Recognised --/ hence running applyODIM" << mout.endl;
			mout.warn() << "assign: " << odimSyntax.result[1] << mout.endl;
			drain::getCommandBank().run("setODIM", odimSyntax.result[1], ctx);
			///drain::getRegistry().run("setODIM", odimSyntax.result[1]);

		}
		else if (value.at(0) == '-'){
				mout.error() << "Unknown parameter (or invalid filename): " << value << mout.endl;
		}
		else {
			try {
				mout.debug2() << "Assuming filename, trying to read." << mout.endl;
				drain::getCommandBank().run("inputFile", value, ctx);
				//drain::getRegistry().run("inputFile", params);
			} catch (std::exception & e) {
				mout.error() << "could not handle params='" << value << "'" << mout.endl;
			}
		}

	};

};
*/




class CmdCheckType : public drain::BasicCommand {

public:

	CmdCheckType() : drain::BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		//PolarODIM::checkType(*getResources().currentHi5);
		PolarODIM::checkType(*ctx.currentHi5);
	};

};


class CmdValidate : public drain::SimpleCommand<std::string>  {

public:

	CmdValidate() : drain::SimpleCommand<std::string>(__FUNCTION__, "Read CVS file ",
			"filename", "", "<filename>.cvs"){ //, inputComplete(true) {
	};

	void exec() const;


};

void CmdValidate::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, getName());

	ODIMValidator validator;

	drain::Input infile(value);
	std::istream & istr = infile;
	std::string line;
	while (getline(istr, line)){
		//line = drain::StringTools::trim(line, " \n\t\r");
		validator.push_back(ODIMNodeValidator());
		ODIMNodeValidator & nodeValidator = validator.back();
		nodeValidator.assign(line);
		mout.debug2() << "L: " << line          << mout.endl;
		mout.debug2() << "V: " << nodeValidator << mout.endl;
		line.clear();
	}

	//RackResources & resources = getResources();

	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

	if (!src.isEmpty()){

		ODIMPathList dataPaths;

		src.getPaths(dataPaths); // ALL

		//for (ODIMPathList::const_iterator it = dataPaths.begin(); it != dataPaths.end(); ++it){
		for (ODIMPath & path: dataPaths){

			//ODIMPath p;
			//p.setElems(ODIMPathElem::ROOT);
			//p << *it;

			ODIMValidator::const_iterator wit;

			if (path.back().is(ODIMPathElem::ARRAY))
				wit = validator.validate(path, H5I_DATASET);
			else
				wit = validator.validate(path, H5I_GROUP);

			if (wit == validator.end()){
				mout.warn() << "REJECT path: " << path << mout.endl;
				return;
			}
			else {
				mout.debug2() << "RegExp: " << wit->pathRegExp.toStr() << mout.endl; //.toStr()
				mout.debug() << "ACCEPT path: " << path << mout.endl;
			}

			const Hi5Tree & t = src(path); // (*ctx.currentHi5)(*it);
			if (t.data.dataSet.isEmpty()){
				// std::cout << " GROUP" << '\n';
				// const drain::VariableMap & a = t.data.attributes;
				//for (drain::VariableMap::const_iterator ait=a.begin(); ait!=a.end(); ++ait){
				for (const auto & entry: t.data.attributes){
					std::string attributePath(path);
					attributePath.push_back(path.separator.character);
					attributePath.append(entry.first); //ait->first);
					ODIMValidator::const_iterator wit = validator.validate(attributePath, H5I_ATTR);
					if (wit == validator.end()){
						mout.warn() << "UNKNOWN attribute: " << attributePath << mout.endl;
						continue;
					}
					else {
						mout.debug2() << "ACCEPT attribute path: " << attributePath << mout.endl;
					}
					const std::type_info & rType = wit->basetype.getType();
					const std::type_info & aType = entry.second.getType();

					// Compose variable info: <path>:<key>=<value> <type>.
					std::stringstream sstr;
					sstr << path << ':' << entry.first << '=';
					entry.second.valueToJSON(sstr);
					if (entry.second.isString())
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
						if (wit->valueRegExp.test(entry.second)){ // convert on the fly
							mout.debug() << "COMPLIANT attribute value: " << sstr.str() << mout.endl;
						}
						else {
							mout.warn() << "INCOMPLIANT attribute value: " << sstr.str() << mout.endl;
						}
					}
				}
			}
			else {
				mout.debug() << "ACCEPT data: " << path << mout.endl;
			}

		}
	}
	else
		mout.warn() << "no current H5 data" << mout.endl;

}




class CmdVersion : public drain::BasicCommand {

public:

	CmdVersion() : drain::BasicCommand(__FUNCTION__, "Displays software version and quits."){};

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


class OutputDataVerbosity : public drain::BasicCommand {

public:

	OutputDataVerbosity() : drain::BasicCommand(__FUNCTION__, "Determines if also intermediate results (1) are saved. Replacing --aStore ?") {
		parameters.link("level", ProductBase::outputDataVerbosity = 0, "0=default,1=intermediate results|2=extra debug results");
	};

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		mout.deprecating() = "use '--store 2' instead";
	};

};
// static drain::CommandEntry<OutputDataVerbosity> dataVebose("verboseData");



class CmdAppend : public drain::SimpleCommand<std::string> {  //public drain::BasicCommand {

public:

	CmdAppend() : drain::SimpleCommand<std::string>(__FUNCTION__, "Append inputs/products (empty=overwrite).", "path", "", "<amprty>|dataset[n]|data[n]"){
	}

	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		if (value.empty()){
			ProductBase::appendResults.set(ODIMPathElem::ROOT); // = overwrite, do not append
		}
		else if (value == "dataset")
			ProductBase::appendResults.set(ODIMPathElem::DATASET);
		else if (value == "data") // This is needed to distinguish between /data123 and /data
			ProductBase::appendResults.set(ODIMPathElem::DATA);
		else {
			ProductBase::appendResults.set(value); // possibly "data4" or "dataset7"
			//mout.warn() << "check path validity: "<< value << "'" << mout.endl;
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
class CmdStore : public drain::BasicCommand {

public:

	CmdStore() : drain::BasicCommand(__FUNCTION__, "Set how intermediate and final outputs are stored. See --append"){
		parameters.link("intermediate", ProductBase::outputDataVerbosity = 0, "store intermediate images");
		//parameters.link("append",  ProductBase::appendResults = "", "|data|dataset");
		parameters.link("append",  append, "|data|dataset (deprecated)");
	};


	CmdStore(const CmdStore & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	};

	//int level;
	std::string append;

	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		DetectorOp::STORE = (ProductBase::outputDataVerbosity>0);
		if (!append.empty()){
			ProductBase::appendResults.set(append);
			mout.warn() << "option 'append' is deprecating, use --append <path> instead." << mout.endl;
		}
	};

};


//class CmdQuantityConf : public drain::SimpleCommand<> {//public drain::BasicCommand {
class CmdQuantityConf : public drain::BasicCommand {

public:

	std::string quantity;
	std::string encoding;
	mutable double zero;

	// Odim used for interface only. Zero used internally.

	CmdQuantityConf(): drain::BasicCommand(__FUNCTION__, "1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair") {
		parameters.separator = ':';
		parameters.link("quantity", quantity = "", "quantity (DBZH,VRAD,...)"); // and storage type (C,S,d,f,...)");
		parameters.link("encoding", encoding = "", EncodingODIM().getValues());
		/*
		//parameters.link("type", odim.type, "C", "storage type (C,S,d,f,...)");
		parameters.link("gain", odim.scaling.scale = 0.0, "scaling coefficient");
		parameters.link("offset", odim.scaling.offset = 0.0, "bias");
		parameters.link("undetect", odim.undetect = 0.0, "marker");
		parameters.link("nodata", odim.nodata = 0.0, "marker");
		*/
		parameters.link("zero", zero = std::numeric_limits<double>::max(), "value");// what about max?

	}

	CmdQuantityConf(const CmdQuantityConf & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};

	/*
	CmdQuantityConf(): drain::SimpleCommand<>(__FUNCTION__,
			"1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair") {
	}
	*/

//	void run(const std::string & params = ""){
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		QuantityMap & m = getQuantityMap();

		if (!quantity.empty()){
			// consider moving selector here, try find() first?
			editQuantityConf(quantity);
		}
		else {
			/*
			if (!encoding.empty()){
				mout.fail() << "given encoding... 'quantity' parameter is obligatory" << mout.endl;
				return;
			}
			*/

			// todo: if (selector.consumeParameters(ctx.select)){
			if (ctx.select.empty()){
				// No quantity given, dump all the quantities
				std::cout << "Quantities:\n";
				std::cout << m << std::endl;
			}
			else {
				// Select desired quantities. Note: on purpose, getResources().select not cleared by this operation
				DataSelector selector;
				selector.setParameters(ctx.select);
				const drain::RegExp regExp(selector.quantity);
				bool match = false;
				for (QuantityMap::const_iterator it = m.begin(); it != m.end(); ++it){
					if (regExp.test(it->first)){
						match = true;
						mout.warn() << it->first << " matches " << regExp << mout.endl;
						editQuantityConf(it->first);
					}
				}
				if (!match)
					mout.warn() << "no matches with. " << regExp << mout.endl;
			}
		}

		//quantityType.clear();
		//double zero = std::numeric_limits<double>::min();  // what about max?

	}

protected:

	//std::string quantityType;

	EncodingODIM odim;

	//double zero;


	void editQuantityConf(const std::string & quantity) const { //, const std::string & type, const std::string & args) const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		if (quantity.empty()){
			mout.error() << "quantity empty" << mout.endl;
			return;
		}

		mout.warn() << "[" << quantity << "]: " << encoding << ", zero=" << zero << mout.endl;

		QuantityMap & m = getQuantityMap();
		Quantity & q = m[quantity];

		//mout.warn() << "base: " << q << mout.endl;

		if (zero != std::numeric_limits<double>::max()){
			mout.warn() << "setting zero " << zero << mout.endl;
			if (std::isnan(zero)) // DOES NOT WORK.. convert toStr to double NaN
				q.unsetZero();
			else
				q.setZero(zero);
			zero = std::numeric_limits<double>::max();
		}


		if (!encoding.empty()){
			if (!q.defaultType)
				q.defaultType = 'C';
			// Note: resets scaling; expect it to be reset here
			EncodingODIM odim;
			// Step 1: detect type char
			odim.addShortKeys();
			odim.type = q.defaultType;
			odim.setValues(encoding);
			mout.warn() = odim;
			if (odim.type.length() != 1){
				mout.fail() << "illegal type: " << odim.type << mout.endl;
				return;
			}
			const char typeChar= odim.type.at(0);
			// Step 2: retrieve old values, set current type
			EncodingODIM & currentEncoding = q.set(typeChar);
			// Write over
			currentEncoding.setValues(encoding);
			q.defaultType = typeChar;
			// currentEncoding.updateFromCastableMap(newEncoding);
			//q.set(typecode).updateValues(args);

			mout.warn() << "set default type for :" << quantity << '\n' << q << mout.endl;
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
struct VerboseCmd : public drain::BasicCommand {

	VerboseCmd() : drain::BasicCommand(__FUNCTION__, "Set verbosity level") {
		parameters.link("level", level = 5);
		parameters.link("imageLevel", imageLevel = 4);
	};

	VerboseCmd(const VerboseCmd & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.log.setVerbosity(level); // NEW
		drain::getLog().setVerbosity(level);
		drain::image::getImgLog().setVerbosity(imageLevel);
		std::cerr << "verbosity: " << level << '\n';
		std::cerr << "verbosity: " << ctx.log.getVerbosity() << '\n';
		std::cerr << "verbosity: " << getContext<drain::Context>().log.getVerbosity() << '\n';
		std::cerr << "verbosity: " << getContext<drain::SmartContext>().log.getVerbosity() << '\n';
	};

	int level;
	int imageLevel;

};


class CmdExpandVariables2 : public drain::CmdExpandVariables {

public:

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.getStatusMap();
		drain::CmdExpandVariables::exec();
	};

};

/*
template <class C=drain::Command>
class RackCmdInstaller : public installer.install<C> {
public:

	RackCmdInstaller(char alias = 0): drain::CommandWrapper<C>(alias) {
	};

};
*/

MainModule::MainModule(){ //

	// NEW
	drain::CommandBank & cmdBank = drain::getCommandBank();

	// Bank-referencing commands first
	drain::HelpCmd help(cmdBank);
	installExternal(help, 'h');

	drain::CmdScript script(cmdBank);
	installExternal(script);

	drain::CmdExecFile execFile(cmdBank);
	installExternal<drain::CmdExecFile>(execFile);


	// Independent commands
	//drain::CommandInstaller<> installer(cmdBank);

	install<CmdSelect>('s'); // cmdSelect('s'); //("select", 's');
	install<drain::CmdStatus>(); //  cmdStatus;
	install<drain::CmdLog>(); //  cmdLogFile; // consider abbr. -L ?

	install<CmdEncoding>('e'); // cmdEncoding('e');  //"encoding", 'e');  // new
	install<CmdEncoding>("target");  // alias
	install<CmdFormatOut>(); //  cmdFormatOut;
	install<CmdEcho>(); //  cmdEcho;

	//installer.install<CmdSleep> cmdSleep;

	install<drain::CmdFormat>(); //  cmdFormat;
	install<drain::CmdFormatFile<RackContext> > (); // cmdFormatFile;


	install<drain::CmdDebug>(); //  cmdDebug;
	install<VerboseCmd>('v');  //  cmdVerbose('v');
	// RackLetAdapter<drain::CommandLoaderOLD> commandLoader("execFile");
	//install<drain::CmdExpandVariables> expandVariables;
	install<CmdExpandVariables2>(); //  expandVariables; //("expandVariables");

	//install<CmdAutoExec> cmdAutoExec;
	//install<CmdDataOk> dataOk("dataOk", -1);
	install<CmdErrorFlags>(); //  errorFlags;
	install<UndetectWeight>(); //  undetectWeight; //("undetectWeight");

	// install<CmdSelectQuantity> cmdSelectQuantity("quantity",'Q');
	install<CmdSelectQuantity>('Q'); //cmdSelectQuantity('Q');
	install<CmdCheckType>(); //  cmdCheckType;
	install<CmdValidate>(); //  cmdValidate;
	//install<CmdCreateDefaultQuality> cmdCreateDefaultQuality;
	install<CmdCompleteODIM>(); //  cmdCompleteODIM;
	install<CmdConvert>(); //  cmdConvert;
	install<CmdDelete>(); //  cmdDelete;
	install<CmdDumpMap>(); //  cmdDumpMap; // obsolete?
	//RackLetAdapter<CmdDumpEchoClasses> cmdDumpEchoClasses; // obsolete?

	//RackLetAdapter<CmdHelpRack> help("help", 'h'); // OBSOLETE
	install<CmdHelpExample>(); //  cmdHelpExample; // TODO hide

	install<CmdJSON>(); //  cmdJSON;
	install<CmdKeep>(); //  cmdKeep;
	// install<CmdRename>(); //  cmdRename; //("rename"); // deprecating
	install<CmdMove>();
	install<CmdSetODIM>(); //  cmdSetODIM;
	install<CmdVersion>(); //  cmdVersion;
	//install<OutputDataVerbosity> dataVebose("verboseData");

	install<CmdAppend>(); //   cmdAppend;
	// CommandWrapper<CmdAppend>  cmdAppend2;
	install<CmdStore>(); //   cmdStore;
	//CommandWrapper<CmdStore>  cmdStore2;

	//RackLetAdapter<CmdQuantityConf> cmdQuantity;
	install<CmdQuantityConf>(); //  cmdQuantity2;


	install<CmdCreateDefaultQuality>(); //  cmdCreateDefaultQuality;

}

} // namespace rack
