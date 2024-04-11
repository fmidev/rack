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

#include <cassert>

#include <set>
#include <map>
#include <ostream>
#include <iomanip>
#include <unistd.h>
#ifndef  USE_GEOTIFF_NO
#include <geotiff.h>
#endif

#include <drain/Log.h>
#include <drain/RegExp.h>
#include <drain/Sprinter.h>


#include "drain/util/JSON.h"
#include "drain/util/Input.h"
#include "drain/util/Output.h"
#include "drain/util/Proj6.h"
#include "drain/util/TreeUtils.h"

#include "drain/image/Image.h"
#include "drain/image/TreeSVG.h"

#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"



#include "data/DataCoder.h"
#include "data/ODIM.h"
#include "data/ODIMValidator.h"
// #include "data/VariableFormatterODIM.h"
#include "hi5/Hi5.h"

#include "product/DataConversionOp.h"
#include "andre/DetectorOp.h"

#include "rack.h"
#include "resources.h"
#include "commands.h"


namespace rack {


/// Utility. Consider long names, and then addShortKeys()
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

///
/* General commands.
 *
 */

class CmdBaseSelective : public drain::SimpleCommand<std::string> {

protected:

	CmdBaseSelective(const std::string & name, const std::string & description) :
		drain::SimpleCommand<>(name, description, "selector", drain::sprinter(DataSelector().getParameters()).str()){
		// TODO: "list out" the sub-parameter help. See --help select
	};

	CmdBaseSelective(const CmdBaseSelective & cmd) : drain::SimpleCommand<>(cmd){
	};


	mutable
	DataSelector mySelector;

};


/// Select parts of hierarchical data using path, quantity, elevation angle and PRF mode as selection criteria.
/**

	This command determines the data to be applied in subsequent input read or product generation commands.

	In \b Rack, many operations implictly select a subset of available data instead of using it all.
	For example, in computing a Pseudo CAPPI image, \c DBZH data from single-PRF sweeps is used by default.

	\b Synopsis
	\include select.hlp

	Remarks on the options:

	- \c path argument defines a path \e segment to be matched, with desired index ranges (example: \c dataset2:4/data3:8 )
	- in \c path argument, a leading slash fixes the matching to the root (example: \c /dataset:/data: ), otherwise the tail part is matched
    - \c quantity argument accepts a list of strings separated by semicolon ':'
    - each item in a \c quantity argument list can be a literal string like \c DBZH or a regular experession like <code>^DBZ[HV]?$</code>

    See example sets below!

	This option is useful in selecting data for:

	- partial reading a large input file (see \ref fileiopage ).
	- a meteorological product (see \ref productspage ).
	- compositing (see \ref compositespage ).
	- saving images (PNG, GeoTIFF, PPM; see \ref imagespage ).
	- other data dumps, useful in debugging, for example

	\subsubsection CmdSelect_exm Examples

	The following command lines illustrate usage of \c --select in product generation and image output.

	\include example-select.inc

	The following examples are less realistic than those above, but are used in unit testing of the program code.

	\include example-select-test.inc

	\see CmdDelete
	\see CmdKeep
	\see CmdSetODIM
	\see #rack::CmdSelect (code)
	\see #rack::DataSelector (code)

*/
class CmdSelect : public CmdBaseSelective { //drain::BasicCommand {

public:

	CmdSelect() : CmdBaseSelective(__FUNCTION__, "Data selector for the next computation"){
		//parameters.append(testSelector.getParameters());

	};

	CmdSelect(const CmdSelect & cmd) : CmdBaseSelective(cmd) {
		//parameters.append(testSelector.getParameters());
		parameters.updateFromMap(cmd.getParameters());
	};


	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		//drain::Logger mout(ctx.log, getName().c_str(), __FUNCTION__);
		drain::Logger mout(ctx.log, getName().c_str(), __FUNCTION__);
		mySelector.reset();
		// consider "reset" as a special argument.
		try {
			mySelector.setParameters(value); // will alert early
			// mout.special<LOG_NOTICE>("ctx.selector: ", ctx.superSelector.getQuantity());
			// ctx.superSelector.getQuantitySelector();
		} catch (const std::exception & e) {
			mout.error("syntax error in selection string: ", value);
		}
		mout.special<LOG_NOTICE>("ctx.selector: ", mySelector);
		// mout.special<LOG_INFO>("ctx.selector, quantity: ", ctx.superSelector.getQuantitySelector());
		// mout.special<LOG_INFO>("ctx.selector, quality: ",  ctx.superSelector.getQualitySelector());
		ctx.select = value;
		//mout.warn("ctx.select=", ctx.select, "...");
	}

private:

	// DataSelector testSelector;
	// std::string value;

};



/// Tool for selecting data for next command(s), based on paths, quantities and elevations.
/**

Most commands apply implicit input data selection criteria, typically involving data paths, quantities and/or elevation angles.

Like in general in \b Rack, the parameters of \c --select are positional (ordered), meaning that they can be issued as
a comma-separated string without explicit names (keywords), as long as they are given in order. The parameters are:

- \c path is a string of path elements, separated by slash '/' and consisting of:
  - a leading single slash '/', if rooted matching is desired ie. leading parts of the paths are tested; otherwise trailing parts
  - \c what , \c where, \c how - ie. the groups containing attributes
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
    - notice that metadata may contain floating point values like 1.000004723, use \c count=1 to pick a single one within a range
- \c count is the upper limit of accepted indices of \c dataset ; typically used with \c elangle
- \c order defines criteria \c (DATA|TIME|ELANGLE) and operation \c (MIN|MAX) in selecting data, applied with \c count. Note that the lowest elevations can be measured the latest,
     appearing as \c dataset 's with highest indices.
- \c dualPRF determines if sweeps using dual pulse repetition frequency is required (1), accepted like single one (0), or excluded (-1).

The selection functionality is best explained with examples.

\~remark
./test-content.sh
\~

\include example-select.inc
Note that escaping special characters like '|' is often required on command line.


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

/*
class CmdSelectOld : public  CmdBaseSelective { //drain::BasicCommand {

public:

	CmdSelectOld() : CmdBaseSelective(__FUNCTION__, "Data selection for the next operation."){
	};


	/// Only checks the validity of selector.
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, getName());

		//mout.warn("setting: " , value );
		DataSelector  test;

		try {
			// NOTE: test is good for checking immediately. But \c value is needed to store

			// ImageSelector itest;
			if (ctx.log.getVerbosity() > LOG_DEBUG){
				mout.special("testing: " , value );
				test.setParameters(value);
				mout.special("testing: => " ,  test );
				ODIMPathList paths;
				const Hi5Tree & src = ctx.getMyHi5();
				test.getPaths(src, paths);
				mout.special("path count => " ,  paths.size() );
				mout.debug3("paths: " ,  drain::sprinter(paths) );
			}

			// std::cerr << __FILE__ << drain::sprinter(paths) << '\n';
			ctx.select = value;

		}
		catch (const std::exception &e) { // consider generalising this
			mout.warn("keys: " , test.getParameters().getKeys() );
			mout.warn("msg: "  , e.what() );
			mout.error("error in: " , value );
		}
		// mout.special(getName() , ctx.getId() , ':' , ctx.select );

	};


};
*/

/// Set selection criteria strictly to one \c quantity .
/**
Selecting quantities only is frequently needed, so there is a dedicated command \c --selectQuantity (\c -Q ) which
accepts comma-separated simple patterns (with * and ?) instead of regular expressions. For example, \c -Q \c DBZH*,QIND
is equal to \c --select \c quantity='^(DBZH.*|QIND)$' .
*/
class CmdSelectQuantity : public  drain::SimpleCommand<std::string> {

public:

	CmdSelectQuantity() : drain::SimpleCommand<std::string>(__FUNCTION__, "Like --select quantity=... with patterns (not regexps)", "quantities","","quantity[,quantity2,...]"){

	};

	inline
	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, getName().c_str());

		// NEW
		std::string checkedValue(value);
		//ctx.superSelector.reset();
		mySelector.reset();

		try {
			// Setting test selector here will alert early
			mySelector.setQuantities(checkedValue); // ","
			//ctx.superSelector.getQuantitySelector();
		} catch (const std::exception & e) {
			mout.obsolete(checkedValue,  " - pattern matching has been replaced by RegExp matching: * -> .*, ? -> .? etc.");
			mout.warn("msg: ", e.what());
			drain::StringTools::replace(transTable, checkedValue);
			//mout.warn("syntax error in selection string: ", value);
			try {
				mout.special("retrying with '", checkedValue, "'");
				mySelector.setQuantities(checkedValue);
				//ctx.superSelector.getQuantitySelector();
			} catch (const std::exception & e) {
				mout.warn("msg: ", e.what());
				mout.error("syntax error in selection string: ", value, " or ", checkedValue);
				ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR); // resources.dataOk = false;
				return;
			}
		}
		/*
		mout.special<LOG_NOTICE>("ctx.selector: ", ctx.superSelector);
		mout.special<LOG_INFO>("ctx.selector, quantity: ", ctx.superSelector.getQuantitySelector());
		mout.special<LOG_INFO>("ctx.selector, quality: ",  ctx.superSelector.getQualitySelector());
		mout.special<LOG_NOTICE>("ctx.selector: qualities: ", ctx.superSelector.getQuantity());
		*/
		ctx.select = std::string("quantity=") + checkedValue;
		mout.special<LOG_DEBUG>("revised ctx.select: ", ctx.select);

		ctx.statusFlags.unset(drain::StatusFlags::DATA_ERROR); // resources.dataOk = false;

	}

protected:

	static const std::map<std::string,std::string> transTable; // = {{",", "|"}};

	//mutable
	//DataSelector testSelector;

	mutable
	DataSelector mySelector; // TODO: join with SelectiveBase?
};

const std::map<std::string,std::string> CmdSelectQuantity::transTable = {{",", "|"}, {"*",".*"}, {"?","."}};

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
class CmdSet : public drain::SimpleCommand<std::string> {

public:

	CmdSet() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set general-purpose variables",
			"assignment", "", "key=value") {
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		// std::list<std::string args>... multiple possible, but separator escape problem?
		std::string k;
		std::string v;
		drain::StringTools::split2(value, k, v, '=');

		ctx.getStatusMap(false)[k] = v;

	};

};

/// Convert data with desired encoding
/**

	Volume data can be converted to desired scale and encoding with \c --encoding followed by \c --convert .
	If the input file contains several quantities, the target quantity can be selected with \c --select (\c -s) command:

	\code
	rack volume-double.h5 -Q DBZH --encoding C,0.5,-32 --convert  -o volume-new.h5
	rack volume-uint16.h5 -Q DBZH --encoding d --convert -Q VRAD --encoding C,0.025,-7.65506,0,255 --convert -o volume-new.h5
	\endcode

	\see #rack::CmdEncoding (code)
	\see #rack::CmdSetODIM (code)

*/
class CmdConvert : public  drain::BasicCommand {

public:

	CmdConvert() :  drain::BasicCommand(__FUNCTION__, "Convert --select:ed data to scaling and markers set by --encoding") {
	};


	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		if (ctx.currentHi5 == &ctx.cartesianHi5){
			mout.debug("Cartesian data");
			convertCurrentH5<CartesianODIM>();
		}
		else if (ctx.currentHi5 == ctx.currentPolarHi5) {
			mout.debug("Cartesian data");
			convertCurrentH5<PolarODIM>();
		}
		else {
			mout.warn("currentHi5 neither Polar nor Cartesian ");
		}
		ctx.currentImage = nullptr;
		ctx.currentGrayImage = nullptr;

	};

protected:

	template <class OD>
	void convertCurrentH5() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout; = resources.mout;

		DataConversionOp<OD> op;
		op.setEncodingRequest(ctx.targetEncoding);
		ctx.targetEncoding.clear();

		mout.debug("op.targetEncoding: ", op.targetEncoding);
		// mout.warn(op );
		op.dataSelector.consumeParameters(ctx.select);
		mout.debug(op.dataSelector);

		op.processH5(*ctx.currentHi5, *ctx.currentHi5);
		DataTools::updateInternalAttributes(*ctx.currentHi5);

	}

};

class CmdCreateDefaultQuality : public  drain::BasicCommand { // public drain::SimpleCommand<bool> { //

public:

	double dataQuality;
	double undetectQuality;
	double nodataQuality;

	// drain::SimpleCommand<bool>
	CmdCreateDefaultQuality() :  drain::BasicCommand(__FUNCTION__,
			"Creates default quality field. See --undetectWeight and --aDefault"){ //, "quantitySpecific", false){
		//parameters.link("quantitySpecific", quantitySpecific=false, "[0|1]");
		parameters.link("data",     dataQuality=std::numeric_limits<double>::quiet_NaN(), "0..1");
		parameters.link("undetect", undetectQuality=std::numeric_limits<double>::quiet_NaN(), "0..1");
		parameters.link("nodata",   nodataQuality=std::numeric_limits<double>::quiet_NaN(), "0..1");
		//parameters.link("quantity", quantity="", "ODIM code");
	};

	CmdCreateDefaultQuality(const CmdCreateDefaultQuality & cmd) : drain::BasicCommand(cmd),
			dataQuality(std::numeric_limits<double>::quiet_NaN()),
			undetectQuality(std::numeric_limits<double>::quiet_NaN()),
			nodataQuality(std::numeric_limits<double>::quiet_NaN())
			{
		parameters.copyStruct(cmd.parameters, cmd, *this);
	}


	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		DataSelector selector(ODIMPathElem::DATASET);
		selector.consumeParameters(ctx.select);

		if (!selector.quantityIsSet()){
			selector.setQuantities("^DBZH");
			mout.note("selector quantity unset, setting " , selector.getQuantity() );
		}

		//const drain::RegExp quantityRegExp(selector.getQuantity());
		const QuantitySelector & slct = selector.getQuantitySelector();

		Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);

		ODIMPathList paths;
		selector.getPaths(dst, paths);

		mout.note(std::isnan(dataQuality) );
		mout.note(std::isnan(undetectQuality) );
		mout.note(std::isnan(nodataQuality) );

		if (& dst == ctx.currentPolarHi5){
			processStructure<PolarODIM>(dst, paths, slct);
		}
		else if (& dst == &ctx.cartesianHi5){
			processStructure<CartesianODIM>(dst, paths, slct);
		}
		else {
			drain::Logger mout(ctx.log, __FUNCTION__, getName());
			mout.warn("no data, or data structure other than polar volume or Cartesian" );
		}

		DataTools::updateInternalAttributes(dst);

	};


	template <class OD>
	//void processStructure(Hi5Tree & dstRoot, const ODIMPathList & paths, const drain::RegExp & slct) const {
	void processStructure(Hi5Tree & dstRoot, const ODIMPathList & paths, const QuantitySelector & slct) const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		typedef DstType<OD> DT; // PolarDst or CartesianDst

		//const QuantityMap & qmap = getQuantityMap();

		//const bool & quantitySpecific = value;


		//for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){
		for (const ODIMPath & path: paths){

			if (path.empty()){
				mout.warn("Empty path, something went wrong." );
				continue;
			}

			if (path.back().belongsTo(ODIMPathElem::DATA | ODIMPathElem::DATASET)){
			}

			mout.warn(path );
			Hi5Tree & dst = dstRoot(path);
			//QualityDataSupport<DT> quality(dst);

			if (path.back().is(ODIMPathElem::DATASET)){
				DataSet<DT> dstDataSet(dst, slct);
				PlainData<DT> & dstData = dstDataSet.getFirstData();
				PlainData<DT> & dstQuality = dstDataSet.getQualityData();
				dstData.createSimpleQualityData(dstQuality, dataQuality, undetectQuality, nodataQuality);
			}
			else if (path.back().is(ODIMPathElem::DATA)){
				Data<DT> dstData(dst);
				PlainData<DT> & dstQuality = dstData.getQualityData();
				dstData.createSimpleQualityData(dstQuality, dataQuality, undetectQuality, nodataQuality);
			}
			else {
				mout.warn("Path " , path , " typically contains no /quality groups, skipping." );
				mout.note("Consider --select dataset or --select data: (note colon)." );
			}

			//mout.warn(quality  );

			/*
			DataSet<DT> dstDataSet(dstDataSetH5, slct);
			if (quantitySpecific){
				for (typename DataSet<DT>::iterator it2 = dstDataSet.begin(); it2!=dstDataSet.end(); ++it2){
					mout.debug('\t' , it2->first  );
					Data<DT> & dstData = it2->second;
					PlainData<DT> & dstQuality = dstData.getQualityData();
					if (!dstQuality.data.isEmpty())
						mout.warn("quality data exists already, overwriting" );
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
					mout.warn("quality data exists already, overwriting" );
				dstData.createSimpleQualityData(dstQuality, 1.0, 0.0, DataCoder::undetectQualityCoeff);
				qmap.setQuantityDefaults(dstQuality, "QIND");
				//dstQuality.data.setScaling(dstQuality.odim.scaling.scale, dstQuality.odim.scaling.offset);
				dstQuality.data.setScaling(dstQuality.odim.scaling); // needed?
				//@ dstQuality.updateTree();
			}
			//@  DataTools::updateInternalAttributes(dstDataSetH5);
			*/


		}


	}

};


/// Remove parts of a hierarchical data structure.
/**

 	\param selection - desired parts to be deleted (syntax explained in \ref CmdSelect)

	\b Synopsis
	\include delete.hlp

	\b Examples
	\include example-delete.inc

	Notice that \c --elangle applies to volume data only, and essentially selects \c dataset  groups.
	Similarly, \c quantity selects \c data (and \c quality)  groups.
	If selection parameters of both levels are issued in the same command,
	implicit \c AND function applies in selection.

	\see CmdKeep
	\see modifypage

*/
class CmdDelete : public CmdBaseSelective {

public:

	CmdDelete() :  CmdBaseSelective(__FUNCTION__, "Deletes selected parts of h5 structure."){
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __LINE__, getName());

		// Hi5Tree & dst = *ctx.currentHi5; // well, consider
		Hi5Tree & dst =  ctx.getHi5(RackContext::CURRENT, RackContext::PRIVATE);

		if (value == "empty"){
			handleEmptyGroups(ctx, dst, true); // true=delete
			return;
		}

		DataSelector selector(ODIMPathElem::DATASET, ODIMPathElem::DATA);
		selector.setParameters(value);

		mout.info("selector: ", selector );


		// Step 0
		mout.debug("delete pre-existing no-save structures ");
		hi5::Hi5Base::deleteExcluded(dst);

		// Selecting paths
		ODIMPathList paths;
		selector.getPaths(dst, paths);

		mout.info("Deleting ", paths.size(), " substructures");
		for (const ODIMPath & path: paths){
			mout.debug("deleting: ", path);
			dst.erase(path);
		}

		handleEmptyGroups(ctx, dst);

	};

protected:

	///
	static
	int handleEmptyGroups(RackContext & ctx, Hi5Tree & dst, bool remove=false, const ODIMPath & path = ODIMPath()){

		drain::Logger mout(ctx.log, __FILE__, __LINE__, __FUNCTION__);

		// mout.special("considering ", path);

		ODIMPathList paths;

		int count = 0;

		for (auto & entry: dst(path).getChildren()){
			if (entry.first.belongsTo(ODIMPathElem::DATA_GROUPS)){
				++count;
				const ODIMPath p(path, entry.first);
				int c = handleEmptyGroups(ctx, dst, remove, p);
				if (c==0){
					paths.push_back(p);
				}
			}
			else if (entry.first.is(ODIMPathElem::ARRAY)){
				if (entry.second.data.empty()){
					++count;
				}
			}
		}

		if (count == 0){
			if (!remove){
				mout.info("Empty groups remaining at ", path);
				mout.hint<LOG_INFO>("Add path argument or remove empty groups with additional '--delete empty'");
			}
		}

		if (remove){

			// Note: dst(path).clearChildren() could corrupt iteration at upper stack level.

			for (const ODIMPath & p: paths){
				mout.debug("Removing empty group: ", p);
				dst.erase(p);
				// mout.attention("Removing empty group: DONE");
			}
		}

		return count;

	}

};


/// Remove parts of a hierarchical data structure.
/**

 	\param selection - desired parts to be kept (syntax explained in \ref CmdSelect)

	Metadata groups (\c what, \c where, \c how) are preserved or deleted together with their
	parent groups.

	\b Synopsis
	\include keep.hlp

	Examples:
	\include example-keep.inc

	\see CmdDelete
	\see modifypage

 */
class CmdKeep : public  CmdBaseSelective {

public:

	/// Keeps a part of the current h5 structure, deletes the rest. Quantity is a regular expression.
	CmdKeep() :  CmdBaseSelective(__FUNCTION__, "Keeps selected part of data structure, deletes rest."){
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		Hi5Tree & dst = ctx.getHi5(RackContext::CURRENT);  // *ctx.currentHi5;

		// Step 0
		mout.debug("delete existing no-save structures ");
		hi5::Hi5Base::deleteExcluded(dst);

		// Initially, mark all paths excluded.
		DataTools::markExcluded(dst, true);
		//hi5::Hi5Base::markExcluded(dst);

		DataSelector selector;
		selector.setParameters(value);

		//hi5::Hi5Base::writeText(dst, std::cerr);

		mout.debug2("selector for saved paths: ", selector);

		ODIMPathList savedPaths;
		selector.getPaths(dst, savedPaths); //, ODIMPathElem::DATASET | ODIMPathElem::DATA | ODIMPathElem::QUALITY);

		for (const ODIMPath & path: savedPaths){

			mout.debug2("set save through path: ", path);
			DataTools::markExcluded(dst, path, false);
			/*
			ODIMPath p;
			for (const ODIMPathElem & elem: path){
				p << elem;
				dst(p).data.exclude = false;
			}
			*/
			//mout.debug("marked for save: " , *it );
			// Accept also tail (attribute groups)
			//if (it->back().isIndexed()){ // belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){ or: DATASET
			// Hi5Tree & d = dst(path);

			for (auto & entry: dst(path)){
				if (entry.first.is(ODIMPathElem::ARRAY)){
					mout.debug2("also save: ", path, '|', entry.first);
					// if (dit->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS))
					entry.second.data.exclude = false;
				}
			}

			//
		}

		// debug: hi5::Hi5Base::writeText(dst, std::cerr);

		hi5::Hi5Base::deleteExcluded(dst);

	};

};





/// Rename or move data groups and attributes.
/**

    \param src - source path consisting at least of the group path
    \param dst - full destination path or only ':' followed by attribute key

    A full path consist of a slash-separated group path elements followed by an attribute key separated by colon.
    For example: \c /dataset1/data2/what:gain .

	\include move.hlp

    Examples:
	\include example-move.inc

	\see CmdDelete
	\see CmdKeep
	\see modifypage

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

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		if (ctx.currentHi5 == NULL){
			mout.error("current Hi5 == NULL" );
			return;
		}


		Hi5Tree & dstRoot = *ctx.currentHi5;

		/*
		ODIMPathMatcher matcher;
		std::string s;
		drain::StringTools::split2(pathSrc, matcher, s, ":");
		mout.experimental("Resolved path:", pathSrc, " => ", matcher);
		*/

		// NEW
		/*
		DataSelector selector;
		selector.consumeParameters(ctx.select);
		mout.debug(selector);
		selector.getPaths(src, paths);
		*/

		/*
		mout.warn("path:  " , path1 );
		mout.warn("attr:  " , attr1 );
		mout.warn("value: " , attr1attrValue );
		mout.warn("type:  " , attr1type  );
		return;
		 */

		ODIMPath path1;
		//std::string path1;
		std::string attr1;
		// std::string attr1value; // debug
		// std::string attr1type;  // debug
		std::string attrValue;
		hi5::Hi5Base::parsePath(pathSrc, path1, attr1, attrValue);

		hi5::Hi5Base::parsePath(pathSrc, path1, attr1, attrValue);

		//mout.debug(pathSrc, " -> path:  " , path1 , ", size=" , path1.size() );
		mout.debug(pathSrc, " -> path1: " , path1 , '(' , path1.size() , ')' , " attr1:" , attr1 , " ~", attrValue);

		if (!attrValue.empty()){
			mout.warn("value (" , attrValue , ") not empty in " , path1 );
		}


		if (!dstRoot.hasPath(path1)) // make it temporary (yet allocating)
			mout.warn("nonexistent path: " , path1 );

		ODIMPath path2;
		std::string attr2;
		hi5::Hi5Base::parsePath(pathDst, path2, attr2, attrValue);
		if (!attrValue.empty()){
			mout.warn("value (" , attrValue , ") not empty in " , path2 );
		}


		if (pathDst.at(0) == ':'){
			mout.note(" renaming attribute only" );
			path2 = path1;
		}


		mout.debug(pathDst, " -> path2: " , path2 , '(' , path2.size() , ')' , " attr2:" , attr2 , " ~", attrValue);


		if (attr1.empty()){ // RENAME PATHS (swap two paths)

			if (!attr2.empty()){
				mout.error("cannot move path '" , path1 , "' to attribute '" , attr2 , "'" );
				return;
			}

			mout.debug("renaming (swapping) paths '" , path1 , "' => '" , path2 , "'" );
			//mout.warn(dstRoot );
			Hi5Tree & dst1 = dstRoot(path1);
			if (!dstRoot.hasPath(path2)) // make it temporary (yet allocating it for now)
				dstRoot(path2).data.exclude = true;
			Hi5Tree & dst2 = dstRoot(path2);

			const bool exclude1 = dst1.data.exclude;
			const bool exclude2 = dst2.data.exclude;
			dst2.swap(dst1);
			dst1.data.exclude = exclude2;
			dst2.data.exclude = exclude1;
			dst2.data.attributes.swap(dst1.data.attributes);

			// dstRoot.erase(path1); ?
			dstRoot(path1).data.exclude = true;
			//DataTools::updateInternalAttributes(dst1); // recurse subtrees
			//DataTools::updateInternalAttributes(dst2); // recurse subtrees

		}
		else { // Rename attribute


			if (path2.empty())
				path2 = path1;

			if (attr2.empty())
				attr2 = attr1;

			mout.debug("renaming attribute (" , path1 , "):" , attr1 , " => (" , path2 , "):" , attr2 );

			Hi5Tree & dst1 = dstRoot(path1);
			if (!dst1.data.attributes.hasKey(attr1)){
				mout.warn("attribute '" , attr1 , "' not found, path='" , path1 , "'" );
			}
			Hi5Tree & dst2 = dstRoot(path2);
			dst2.data.attributes[attr2] = dst1.data.attributes[attr1];
			dst1.data.attributes.erase(attr1);
			//DataTools::updateInternalAttributes(dst2);
			mout.debug("dst1 attributes now: " , dst1.data.attributes );
			mout.debug("dst2 attributes now: " , dst2.data.attributes );
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
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		mout.obsolete("--> '--move' " );
		CmdMove::exec();
	};

};

/// Ensure correct ODIM types after setting attributes
/**
    The OPERA data information model (ODIM) defines conventions for weather radar data.
	Some important ODIM attributes can be added automatically with \c --completeODIM command, which sets \c nbins , \c nrays , \c xsize , and \c ysize
	equal to data dimensions, if already loaded as image.

	\b Synopsis
	\include completeODIM.hlp

	\see #rack::CmdSetODIM (code)
	\see CmdSetODIM

 */
class CmdCompleteODIM : public  drain::BasicCommand {

public:

	CmdCompleteODIM() : drain::BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	template <class OD>
	void complete(Hi5Tree & dstH5) const { // , OD & od)

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		typedef DstType<OD> DT;

		//DataSelector selector; // todo implement --select

		EncodingBag enc;
		enc.setValues(ctx.targetEncoding);
		// ? ctx.targetEncoding.clear();

		//mout.debug("start upd" );

		DataTools::updateInternalAttributes(dstH5); //, drain::FlexVariableMap());
		// DataTools::updateInternalAttributes(dstH5);

		// OD rootODIM(dstH5.data.dataSet.getProperties());
		// mout.debug("Root odim: " , rootODIM );

		//for (Hi5Tree::iterator it = dstH5.begin(); it != dstH5.end(); ++it){

		for (auto & entry: dstH5){

			mout.debug("considering: ", entry.first);

			if (!entry.first.is(ODIMPathElem::DATASET))  //{ // && selector.dataset.contains(it->first.getIndex())){
				continue;

			for (auto & subEntry: entry.second){

				if (!subEntry.first.belongsTo(ODIMPathElem::DATA|ODIMPathElem::QUALITY))
					continue;

				mout.debug(subEntry.first, " tree:\n", subEntry.second);
				PlainData<DT> dstData(subEntry.second);
				//PlainData<DT> & dstData = dit->second;

				//mout.warn("prop: " , dstData.data.properties );

				dstData.odim.updateFromMap(dstData.data.properties); // assume UpdateMetadata
				//mout.warn("ODIM: " , dstData.odim );
				dstData.odim.quantity = dstData.data.properties.get("what:quantity","");
				dstData.odim.type = dstData.data.properties.get("what:type", enc.type);

				if (!dstData.data.isEmpty()){
					const size_t w = dstData.data.getWidth();
					const size_t h = dstData.data.getHeight();

					const std::type_info & t = dstData.data.getType();

					dstData.odim.type = drain::Type::getTypeChar(t);

					;

					if (drain::Type::call<drain::typeIsInteger>(t) && !drain::Type::call<drain::isSigned>(t)){
						drain::Variable v;
						if (dstData.odim.nodata < 0){
							v.setType(t);
							v = dstData.odim.nodata;
							//mout.attention(dstData.odim.quantity, ":", EncodingODIM(dstData.odim), ", no data:",  dstData.odim.nodata);
							mout.note(dstData.odim.quantity, "[", dstData.odim.type, "]: adjusting nodata:", dstData.odim.nodata, " -> ", (double)v);
							dstData.odim.nodata = v;
						}
					}

					dstData.odim.setGeometry(w, h);
					//rootODIM.setGeometry(w, h);    // for Cartesian root-level xsize,ysize
					//dataSetODIM.setGeometry(w, h); // for Polar  dataset-level nbins, nrays
				}

				if (dstData.odim.quantity.empty()){
					//if (dit->first != "null")
					//	dstData.odim.quantity = dit->first;
					//else
					dstData.odim.quantity = enc.quantity;
					mout.special("quantity:" ,  dstData.odim.quantity );
				}
				if (dstData.odim.scaling.scale == 0){
					mout.info("setting quantity defaults [" , dstData.odim.quantity , ']' );
					getQuantityMap().setQuantityDefaults(dstData.odim); //, dstData.odim.quantity);
				}


				//mout.note("dstData.updateTree: " , dit->first );

				dstData.updateTree2();
				//rootODIM.updateLenient(dstData.odim);
				mout.debug("dstData.odim.time: " , dstData.odim.time );
				//mout.note("    rootODIM.time: " , rootODIM.time );
			}
			//}
		}

		/*
		if (rootODIM.date.empty())
			rootODIM.date = rootODIM.startdate;
		if (rootODIM.time.empty())
			rootODIM.time = rootODIM.starttime;
		 */
		/*
		mout.special("Final rootODIM.time: " , rootODIM.time );
		rootODIM.source = "radar.TEST";
		ODIM::copyToH5<ODIMPathElem::ROOT>(rootODIM, dstH5);
		mout.special(dstH5 );
		*/

		//mout.attention("DataTools::updateInternalAttributes");

		DataTools::updateInternalAttributes(dstH5);
		//hi5::Writer::writeFile("disto.h5", dstH5);

	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


		if (ctx.polarInputHi5[ODIMPathElem::WHAT].data.attributes["object"].toStr() == "COMP"){
			ctx.polarInputHi5.swap(ctx.cartesianHi5);
			ctx.currentHi5 = &ctx.cartesianHi5;
		}

		Hi5Tree & dst = *ctx.currentHi5;

		if (&dst == ctx.currentPolarHi5){
			complete<PolarODIM>(dst); // , odim
		}
		else if (&dst == &ctx.cartesianHi5){
			//
			complete<CartesianODIM>(dst); // , odim

			// hi5::Writer::writeFile("disto2.h5", dst);

			RootData<CartesianDst> root(dst);
			// mout.experimental(root.odim);
			//hi5::Writer::writeFile("disto3.h5", dst);

			CartesianODIM & rootODIM = root.odim;
			//root.odim.updateFromCastableMap(dst.data.dataSet.properties);
			mout.obsolete(root.odim);

			drain::VariableMap & where = dst[ODIMPathElem::WHERE].data.attributes; //root.getWhere();
			mout.debug2(where);
			GeoFrame frame(where.get("xsize", rootODIM.area.getWidth()), where.get("ysize", rootODIM.area.getHeight()));
			std::string projdef = where.get("projdef","");
			if (!projdef.empty())
				//projdef = "+proj=laea +lat_0=55.0 +lon_0=10.0 +x_0=1950000.0 +y_0=-2100000.0 +units=m +ellps=WGS84";
				frame.setProjection(projdef);
			frame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			// mout.experimental(frame);
			rootODIM.updateGeoInfo(frame);

			//hi5::Writer::writeFile("disto4.h5", dst);
			//dst.data.attributes.updateFromCastableMap(rootODIM);
			//mout.obsolete(dst.data.attributes);
			//ODIM::copyToH5<ODIMPathElem::ROOT>(rootODIM, dst);
		}
		else {

		}
		//hi5::Writer::writeFile("disto5.h5", dst);


	};

};


class CmdODIM : public drain::SimpleCommand<std::string> {

public:

	CmdODIM() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set ODIM version (experimental)",
			"version", "2.4", drain::sprinter(ODIM::versionFlagger.getDict().getKeys()).str()) {
	};

	void exec() const {
		/*
		std::string value_underscored;
		drain::StringTools::replace(value, ".", "_", value_underscored);
		ODIM::versionFlagger.set(value_underscored);
		*/
		// ... keep dictionary neat
		ODIM::versionFlagger.set(value);
	}

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
class CmdSetODIM : public drain::SimpleCommand<std::string> {  // consider rename to distingish from set ODIM version

public:

	CmdSetODIM() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		if (value.find_first_of("?*()^$") != std::string::npos){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.warn("RegExp support suppressed from this version" );
			return;
		}

		if (value.empty()){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.error("empty command");
			return;
		}

		/*
		if ((value.at(0) == '[') || (value.at(value.size()-1) == ']')){
			mout.error("parallel processing markers '[' and ']' must be given as separate args");
			return;
		}
		*/


		if (value.at(0) != '/'){
			ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
			mout.warn("Could not handle argument '" , value , "': no leading '/' " );
			mout.error("Dynamic command handler (" , getName() , ") failed " );
			//throw std::runtime_error();
			return;
		}


		mout.debug("value: ", value );

		Hi5Tree & src = *(ctx.currentHi5);

		mout.debug("try to track if argument is a path, an attribute, or a combination of both" );

		static
		const drain::RegExp re("^(.*(data|what|where|how)):([a-zA-Z0-9_]+(=.*)?)$");

		std::string assignment;
		DataSelector selector; //(resources.select);
		selector.consumeParameters(ctx.select);

		std::vector<std::string> result;
		if (re.execute(value, result)){
			selector.setPathMatcher(value);
			mout.debug("pathMatcher: " , selector.getPathMatcher());
		}
		else {
			selector.setPathMatcher(result[1]);
			mout.info("pathMatcher: " , selector.getPathMatcher());
			mout.debug("selector: " , selector );
			assignment = result[3];
			mout.debug2("assignment:  " , assignment    );
		}


		ODIMPathList paths;
		selector.getPaths(src, paths);
		if (paths.empty()){
			mout.debug("no paths found, so trying creating one:" , selector.getPathMatcher());
			mout.debug2("isLiteral:  " , selector.getPathMatcher().isLiteral() );
			ODIMPath path;
			selector.getPathMatcher().extractPath(path);
			paths.push_back(path);
		}

		for (ODIMPath path: paths) {
			Hi5Tree & d = src(path);
			if (assignment.empty()){
				mout.debug(path );
			}
			else {
				mout.debug("path: ", path, " assignment:'", assignment, "'");
				hi5::Hi5Base::assignAttribute(d, assignment);
				// mout.info("retrieved: '", d, "'");
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

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		std::ostream & ostr = std::cout;

		// const drain::CommandRegistry::map_t & map = drain::getRegistry().getMap();
		const drain::CommandBank & cmdBank = drain::getCommandBank();
		const drain::CommandBank::map_t & cmdMap = cmdBank.getMap();
		// drain::getCommandBank().re

		// Light strip (could use
		const std::string key = cmdBank.resolveFull(value);
		//const std::string key = value.substr(value.find_first_not_of('-'));

		if (key != value)
			mout.debug("resolved: " , value , " -> ", key );

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
			mout.fail("command " , value , " not found" );
		}

		exit(0);
	};




};

// static drain::CommandEntry<CmdHelpExample> cmdHelpExample("helpExample", 0);


/// Explain..
/**

	Text

	\b Synopsis
	\include JSON.hlp

	\see #::Command
 */
class CmdJSON : public drain::SimpleCommand<std::string> { // drain::BasicCommand {
public:

	CmdJSON() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump to JSON.", "property", "", ""){
	};

	virtual
	//void run(const std::string & params = "") {
	void exec() const {

		//RackResources & resources = getResources();
		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		//drain::CommandRegistry & reg = drain::getRegistry();
		drain::CommandBank & cmdBank = drain::getCommandBank();


		std::ostream & ostr = std::cout;

		if (value.empty()){
			//drain::Sprinter::sequenceToStream(ostr, cmdBank.getMap(), "{,}", drain::Sprinter::jsonLayout);
			// drain::Sprinter::mapToStream(ostr, cmdBank.getMap(), drain::Sprinter::jsonLayout, cmdBank.getKeys());
			mout.unimplemented("drain::JSONwriter");
			// JSON::treeToStream(ostr, cmdBank.getMap());
			// drain::JSONwriter::mapToStream(cmdBank.getMap(), ostr, 2);
			// reg.toJSON(ostr);
		}
		else {
			if (!cmdBank.has(value)){
				mout.error("no such key: " , value );
				return;
			}

			const drain::Command & command = cmdBank.get(value);
			const drain::ReferenceMap & m = command.getParameters();

			const drain::ReferenceMap::unitmap_t & u = m.getUnitMap();

			drain::JSONtree2 jsonRoot;

			drain::JSONtree2 & json = jsonRoot[value];
			json["title"] = command.getDescription();
			drain::JSONtree2 & variables = json["variables"];

			// O
			const drain::ReferenceMap::keylist_t & keys = m.getKeyList();

			for (const drain::ReferenceMap::key_t & key: keys){

				// const drain::Referencer & entry = m[key];

				drain::JSONtree2 & variable = variables[key];

				variable["value"] = m[key]; //entry;

				drain::ReferenceMap::unitmap_t::const_iterator uit = u.find(key);
				if (uit != u.end()){
					variable["unit"] = uit->second;
				}

			}

			drain::Sprinter::toStream(ostr, jsonRoot, drain::Sprinter::jsonLayout);
			// drain::TreeUtils::dump(jsonRoot, ostr, nullptr);

		}

		//std::cout << std::setw(10) << "Voila!" << std::endl;
	};

};


class CmdEcho : public drain::SimpleCommand<std::string> {

public:

	CmdEcho() : drain::SimpleCommand<std::string>(__FUNCTION__, "For testing. Dumps a formatted string to stdout.", "format","","std::string") {
	};

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

		//std::string format = value;
		// format = drain::StringTools::replace(format, "\\n", "\n");
		// format = drain::StringTools::replace(format, "\\t", "\t");
		// std::cout << format << std::endl;
		// std::cout << ctx.getStatus() << std::endl;
		//drain::StringMapper statusFormatter("[a-zA-Z0-9:_]+");
		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(value+'\n', true);

		// VariableFormatterODIM<drain::Variable> odimHandler;
		statusFormatter.toStream(std::cout, ctx.getStatusMap(), 0, RackContext::variableFormatter);

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

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = resources.mout;

		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(ctx.formatStr, true);

		mout.deprecating("Use  -o / --outputFile [file|'-'] instead of  ", getName());

		if (value == "log"){
			mout.unimplemented("Logging: future option" );
			//statusFormatter.toStream(ctx.log.getOstr, ctx.getStatus());
			//std::cout << statusFormatter;
		}
		else if (value == "image"){
			//resources.
			mout.deprecating("this command is unneed (--format is sufficient)" );
		}
		else {
			std::string outFileName;
			drain::VariableMap & statusMap = ctx.getStatusMap();
			if ((value == "")||(value == "-")){
				outFileName = "-";
			}
			else {
				drain::StringMapper filenameFormatter(RackContext::variableMapper);
				filenameFormatter.parse(ctx.outputPrefix + value, false);  //filename = mapper.toStr(ctx.getStatusMap());
				// Consider here or shared: VariableFormatterODIM<drain::Variable> odimHandler;
				outFileName = filenameFormatter.toStr(statusMap, -1, RackContext::variableFormatter);
				//outFileName = ctx.outputPrefix + value;
			}
			mout.info("writing " , outFileName );
			drain::Output ofstr(outFileName);
			//mout.warn(ctx.getStatus() );
			//std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			if (ofstr){
				// VariableFormatterODIM<drain::Variable> odimHandler;
				statusFormatter.toStream(ofstr, statusMap, 0, RackContext::variableFormatter); // odimHandler);
			}
			else
				mout.warn("write error: " , outFileName );
			//strm.toStream(ofstr, cmdStatus.statusMap.exportMap());
			//ofstr.close();
		}

		//mout.warn("after expansion: " , r.statusFormatter );
		//r.statusFormatter.debug(std::cerr, r.getStatusMap());

	};

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
			"encoding", "", EncodingBag().getKeys() // NOTE: latent-multiple-key case
	) {
		//assert( 1 ==0 );
		parameters.separator = 0;
	};


	virtual
	inline
	void exec() const { //(const std::string & params){

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());


		try {

			EncodingBag enc;

			/// Also check and warn of unknown parameters
			enc.setValues(value);  // sets type, perhaps, hence set type defaults and override them with user defs

			drain::Range<double> range;
			range.set(drain::Type::call<drain::typeMin, double>(enc.type), drain::Type::call<drain::typeMax, double>(enc.type));
			if (!range.contains(enc.undetect)){
				mout.warn("undetect=" , enc.undetect , " outside storage type range=" , range );
			}
			if (!range.contains(enc.nodata)){
				mout.warn("nodata=" , enc.nodata , " outside storage type range=" , range );
			}

			//mout.note("(user) params: " , params );
			//mout.note("parameters:    " , parameters );
			mout.debug("odim: " , enc );
			//odim.setTypeDefaults();

			/// Main action: store the value for later commands.
			ctx.targetEncoding = value;
			mout.debug("ctx.targetEncoding " , ctx.targetEncoding );

			// Reassign (to odim).
			//parameters.setValues(params);
			//mout.note("Re-assigned parameters: " , parameters );

		}
		catch (const std::runtime_error & e) {

			mout.warn("Could not set odim" );
			mout.note("pars: " , parameters );
			//mout.warn("odim: " , enc );
			mout.error(e.what() );

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

		mout.debug2("params: " , value );

		/// Syntax for recognising text files.
		static
		const drain::RegExp odimSyntax("^--?(/.+)$");

		if (value.empty()){
			mout.error("Empty parameters" );
		}
		else if (odimSyntax.execute(value) == 0) {
			//mout.warn("Recognised --/ hence running applyODIM" );
			mout.warn("assign: " , odimSyntax.result[1] );
			drain::getCommandBank().run("setODIM", odimSyntax.result[1], ctx);
			///drain::getRegistry().run("setODIM", odimSyntax.result[1]);

		}
		else if (value.at(0) == '-'){
				mout.error("Unknown parameter (or invalid filename): " , value );
		}
		else {
			try {
				mout.debug2("Assuming filename, trying to read." );
				drain::getCommandBank().run("inputFile", value, ctx);
				//drain::getRegistry().run("inputFile", params);
			} catch (std::exception & e) {
				mout.error("could not handle params='" , value , "'" );
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
		mout.debug2("L: " , line          );
		mout.debug2("V: " , nodeValidator );
		line.clear();
	}

	//RackResources & resources = getResources();

	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

	if (!src.empty()){

		ODIMPathList dataPaths;

		drain::TreeUtils::getPaths(src, dataPaths);
		// src.getPaths(dataPaths); // ALL

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
				mout.warn("REJECT path: " , path );
				return;
			}
			else {
				mout.debug2("RegExp: " , wit->pathRegExp.toStr() ); //.toStr()
				mout.debug("ACCEPT path: " , path );
			}

			const Hi5Tree & t = src(path); // (*ctx.currentHi5)(*it);
			if (t.data.image.isEmpty()){
				// std::cout << " GROUP" << '\n';
				// const drain::VariableMap & a = t.data.attributes;
				//for (drain::VariableMap::const_iterator ait=a.begin(); ait!=a.end(); ++ait){
				for (const auto & entry: t.data.attributes){
					std::string attributePath(path);
					attributePath.push_back(path.separator.character);
					attributePath.append(entry.first); //ait->first);
					ODIMValidator::const_iterator wit = validator.validate(attributePath, H5I_ATTR);
					if (wit == validator.end()){
						mout.warn("UNKNOWN attribute: " , attributePath );
						continue;
					}
					else {
						mout.debug2("ACCEPT attribute path: " , attributePath );
					}
					const std::type_info & rType = wit->basetype.getType();
					const std::type_info & aType = entry.second.getType();

					// Compose variable info: <path>:<key>=<value> <type>.
					std::stringstream sstr;
					sstr << path << ':' << entry.first << '=';
					entry.second.info(sstr);
					/*
					entry.second.valueToJSON(sstr);
					if (entry.second.isString())
						sstr << " string";
					else
						sstr << ' ' << drain::Type::call<drain::complexName>(aType);
					*/

					/// Type test
					if (aType == rType){
						mout.debug("COMPLIANT attribute type: " , sstr.str() );
					}
					else {
						sstr << ", should be " << drain::Type::call<drain::complexName>(rType);
						if ((drain::Type::call<drain::typeIsScalar>(aType) == drain::Type::call<drain::typeIsScalar>(rType)) ||
								(drain::Type::call<drain::typeIsInteger>(aType) == drain::Type::call<drain::typeIsInteger>(rType)) ||
								(drain::Type::call<drain::typeIsFloat>(aType) == drain::Type::call<drain::typeIsFloat>(rType))){
							mout.info("Slightly INCOMPLIANT attribute type: " , sstr.str() );
						}
						else if ((drain::Type::call<drain::typeIsScalar>(aType) && !drain::Type::call<drain::typeIsScalar>(rType))){
							mout.note("Moderately INCOMPLIANT attribute type: " , sstr.str() );
						}
						else {
							mout.warn("INCOMPLIANT attribute type: " , sstr.str() );
						}
					}

					/// Value test
					if (wit->valueRegExp.isSet()){
						sstr << " regExp='" <<  wit->valueRegExp.toStr() << "'";
						if (wit->valueRegExp.test(entry.second)){ // convert on the fly
							mout.debug("COMPLIANT attribute value: " , sstr.str() );
						}
						else {
							mout.warn("INCOMPLIANT attribute value: " , sstr.str() );
						}
					}
				}
			}
			else {
				mout.debug("ACCEPT data: " , path );
			}

		}
	}
	else
		mout.warn("no current H5 data" );

}




class CmdVersion : public drain::BasicCommand {

public:

	CmdVersion() : drain::BasicCommand(__FUNCTION__, "Displays software version and quits."){};

	void exec() const {

		std::cout << __RACK_VERSION__ << '\n';
		std::cout << __RACK__ << ' ' << __RACK_VERSION__;
#ifndef NDEBUG
		std::cout << "(debug)";
#endif
		std::cout << ' ' << __DATE__ << '\n';

		//std::cout << DRAIN_IMAGE << '\n';
		std::cout << "Proj " << drain::Proj6::getProjVersion() << '\n';


		unsigned int majnum=0, minnum=0, relnum=0;
		H5get_libversion(&majnum, &minnum, &relnum);  // error messages
		std::cout << "HDF5 " << majnum << '.' << minnum << '.' << relnum << '\n';

		//std::cout << "GeoTIFF support: ";
#ifndef  USE_GEOTIFF_NO
		std::cout << "GeoTIFF " << LIBGEOTIFF_VERSION <<  '\n';
#else
		std::cout << "GeoTIFF not supported" << '\n';
#endif
		std::cout << "experimental ODIM default: " << rack::ODIM::versionFlagger << '\n';
		std::cout << '\n';


	};

};


class OutputDataVerbosity : public drain::SimpleCommand<int> {

public:

	OutputDataVerbosity() : drain::SimpleCommand<int>(__FUNCTION__, "Request additional (debugging) outputs"){
	};

	virtual
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		mout.deprecating() << "use '--store " << value << "' instead";
		ctx.outputDataVerbosity = value;
	};

};

/*
class OutputDataVerbosity : public drain::BasicCommand {

public:

	OutputDataVerbosity() : drain::BasicCommand(__FUNCTION__, "Determines if also intermediate results (1) are saved. Replacing --aStore ?") {
		parameters.link("level", ProductBase::outputDataVerbosity = 0, "0=default,1=intermediate results|2=extra debug results");
	};

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		mout.deprecating() = "use '--store 2' instead";
	};

};
*/
// static drain::CommandEntry<OutputDataVerbosity> dataVebose("verboseData");



class CmdAppend : public drain::SimpleCommand<std::string> {  //public drain::BasicCommand {

public:

	CmdAppend() : drain::SimpleCommand<std::string>(__FUNCTION__, "Append inputs/products (empty=overwrite).", "path", "", "<amprty>|dataset[n]|data[n]"){
	}

	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		if (value.empty()){
			ctx.appendResults.set(ODIMPathElem::ROOT); // = overwrite, do not append
		}
		else if (value == "dataset")
			ctx.appendResults.set(ODIMPathElem::DATASET);
		else if (value == "data") // This is needed to distinguish between /data123 and /data
			ctx.appendResults.set(ODIMPathElem::DATA);
		else {
			ctx.appendResults.set(value); // possibly "data4" or "dataset7"
			//mout.warn("check path validity: ", value , "'" );
		}

		/*
		if (!ODIMPath::isIndexed(ctx.appendResults.getType()) && ! (ctx.appendResults != ODIMPathElem::NONE)){
			mout.warn("illegal path elem '", value , "'" );
		}
		 */

	}

	//std::string path;

};


// NOTE: order changed
class CmdStore : public drain::SimpleCommand<int> {

public:

	CmdStore() : drain::SimpleCommand<int>(__FUNCTION__, "Request additional (debugging) outputs", "level",
			0, "0=default,1=intermediate results|2=extra debug results"){
			//"Set how intermediate and final outputs are stored. See --append"){

		//parameters.link("intermediate", ProductBase::outputDataVerbosity = 0, "store intermediate images");
		//parameters.link("append",  ctx.appendResults = "", "|data|dataset");
		//parameters.link("append",  append, "|data|dataset (deprecated)");
	};

	/*
	CmdStore(const CmdStore & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	};
	*/

	//int outputDataVerbosity;
	//std::string append;

	virtual
	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		ctx.outputDataVerbosity = value;
		// DetectorOp::STORE = (ProductBase::outputDataVerbosity>0);
		//DetectorOp::STORE = ctx.outputDataVerbosity;
		/*
		if (!append.empty()){
			ctx.appendResults.set(append);
			mout.warn("option 'append' is deprecating, use --append <path> instead." );
		}
		*/
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

	CmdQuantityConf(const CmdQuantityConf & cmd) : drain::BasicCommand(cmd), zero(cmd.zero) {
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
				mout.fail("given encoding... 'quantity' parameter is obligatory" );
				return;
			}
			*/

			// todo: if (selector.consumeParameters(ctx.select)){
			if (ctx.select.empty()){
				// No quantity given, dump all the quantities
				std::cout << "Quantities:\n";
				drain::Sprinter::toStream(std::cout, m, drain::Sprinter::cmdLineLayout);
				//std::cout << m << std::endl;
				std::cout << std::endl;
			}
			else {
				// Select desired quantities. Note: on purpose, getResources().select not cleared by this operation
				DataSelector selector;
				selector.setParameters(ctx.select);
				const drain::RegExp regExp(selector.getQuantity());
				bool match = false;
				for (QuantityMap::const_iterator it = m.begin(); it != m.end(); ++it){
					if (regExp.test(it->first)){
						match = true;
						mout.warn(it->first , " matches " , regExp );
						editQuantityConf(it->first);
					}
				}
				if (!match)
					mout.warn("no matches with. " , regExp );
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
			mout.error("quantity empty");
			return;
		}

		mout.debug("[" , quantity , "]: " , encoding , ", zero=" , zero );

		QuantityMap & m = getQuantityMap();
		Quantity & q = m[quantity];

		//mout.warn("base: " , q );

		if (zero != std::numeric_limits<double>::max()){
			mout.warn("setting zero " , zero );
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
			mout.warn(odim);
			if (odim.type.length() != 1){
				mout.fail("illegal type: " , odim.type );
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

			mout.note("set default type for :" , quantity , '\n' , q );
		}
		else {  // No type given, dump quantity conf
			std::cout << quantity << '\n';
			// if (params != quantity)
			//	std::cout << " *\t" << q.get(q.defaultType) << '\n';
			// else
			// drain::Sprinter::toStream(std::cout, q, drain::Sprinter::cmdLineLayout);
			std::cout << q;
		}

	}

};

//
struct VerboseCmd : public drain::BasicCommand {

	VerboseCmd() : drain::BasicCommand(__FUNCTION__, "Set verbosity level") {
		parameters.link("level", level = "NOTE");
		parameters.link("imageLevel", imageLevel = "WARNING");
	};

	VerboseCmd(const VerboseCmd & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};

	inline
	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		ctx.log.setVerbosity(level); // NEW
		drain::getLog().setVerbosity(level);
		// drain::Logger mout(getLogH5(), __FILE__, __FUNCTION__);
		hi5::getLogH5().setVerbosity(level);
		drain::image::getImgLog().setVerbosity(imageLevel);
		/*
		std::cerr << "verbosity: " << level << '\n';
		std::cerr << "verbosity: " << ctx.log.getVerbosity() << '\n';
		std::cerr << "verbosity: " << getContext<drain::Context>().log.getVerbosity() << '\n';
		std::cerr << "verbosity: " << getContext<drain::SmartContext>().log.getVerbosity() << '\n';
		*/
	};

	std::string level;
	std::string imageLevel;
	//int level;
	//int imageLevel;

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

	install<CmdHelpExample>();

	drain::CmdScript script(cmdBank);
	installExternal(script);

	install<drain::CmdExecScript>();

	drain::CmdExecFile execFile(cmdBank);
	installExternal<drain::CmdExecFile>(execFile); // T param unneeded?

	drain::CmdLog log(cmdBank);
	installExternal(log); //  cmdLogFile; // consider abbr. -L ?


	// Independent commands
	install<CmdSelect>('s');
	install<CmdSelectQuantity>('Q');
	install<CmdSet>();
	install<drain::CmdStatus>();

	install<CmdEncoding>('e');
	install<CmdEncoding>("target");  // alias
	install<CmdFormatOut>();
	install<CmdEcho>();

	//installer.install<CmdSleep> cmdSleep;
	install<drain::CmdFormat>();
	install<drain::CmdFormatFile<RackContext> >();

	install<drain::CmdDebug>();
	install<VerboseCmd>('v');

	install<CmdExpandVariables2>();
	install<CmdErrorFlags>();

	install<UndetectWeight>();

	install<CmdCheckType>();
	install<CmdValidate>();

	install<CmdConvert>();
	install<CmdDelete>();
	install<CmdMove>();

	install<CmdDumpMap>();

	install<CmdJSON>();
	install<CmdKeep>();

	install<CmdODIM>("odim");
	install<CmdSetODIM>();
	install<CmdCompleteODIM>();
	install<CmdVersion>();

	install<CmdAppend>();
	install<CmdStore>();

	install<CmdQuantityConf>();
	install<CmdCreateDefaultQuality>();

}

class CmdInputFilter : public drain::SimpleCommand<std::string> {

public:

	CmdInputFilter() : drain::SimpleCommand<std::string>(__FUNCTION__, "Partial file read. You probably search for --inputSelect", "ATTRIBUTES", "3",
			sprinter(drain::EnumDict<hi5::Reader::Mode>::dict, "|").str()) {
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, getName());

		ctx.inputFilter.set(value);
	}
};

/*
class CmdEcho : public drain::SimpleCommand<std::string> {

public:

	CmdEcho() : drain::SimpleCommand<std::string>(__FUNCTION__, "Write to stdout", "string", "") {
	}

	void exec() const {

		drain::Logger mout(ctx.log, __FILE__, getName());
		// TODO: external Notication("echo", wau!)
		mout.attention<LOG_WARNING>(value);
	}

}
*/


class CmdPause : public drain::SimpleCommand<std::string> {

public:

	CmdPause() : drain::SimpleCommand<std::string>(__FUNCTION__, "Pause for n or random seconds", "seconds", "random", "[<number>|random]") {
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, getName());

		//double seconds = NAN;
		int seconds = 0;
		// drain::StringTools::convert(value, seconds);

		if (value == "random"){
			seconds = (::rand() & 7);
		}
		else if (value == "thread"){
			seconds = ctx.getId();
		}
		else {
			std::stringstream sstr(value);
			sstr >> seconds;
			/*
			if (std::isnan(seconds)){
				mout.error("could not interpret argument: ", value);
				return;
			}
			*/
		}

		if (seconds < 0){
			seconds = 0;
		}

		mout.info("Pausing for ", seconds, " s...");
		//sleep(static_cast<int>(seconds));
		sleep(seconds);

	}

};


class CmdTest2 : public drain::BasicCommand {

public:

	CmdTest2() : drain::BasicCommand(__FUNCTION__, "Debug") {

	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::CommandBank & cmdBank = drain::getCommandBank();
		cmdBank.addSection("TEST1");
		cmdBank.addSection("TEST2");
		// mout.experimental("Karttunen", drain::Static::get<CartesianSection>().index);

		// drain::CommandBank::iterator & data_type
		for (const auto & entry: cmdBank.getMap()){
			//mout.experimental(drain::StringBuilder(entry));
			mout.experimental(entry.first, ':', entry.second->getSource().section);
		}

		// drain::FlagResolver::dict_t & section
		// drain::getCommandBank().sections;
		for (const auto & entry: cmdBank.sections){
			mout.experimental(entry.first, ':', entry.second);
		}
	}

protected:

};

class CmdHdf5Test : public drain::SimpleCommand<std::string> {

public:

	CmdHdf5Test() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump H5 sources") {
		//parameters.link("level", level = 5);
	}



	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, getName());
		//drain::VariableMap m;
		//std::set<unsigned int> hset = {Hdf5Context::CURRENT};
		//ctx.getHi5(Hdf5Context::CURRENT);

		dumpStatus(ctx, "local");
		dumpStatus(ctx.getSharedContext(), "shared");

	}

	void dumpStatus(RackContext & ctx, const std::string & title) const {

		typedef DstType<ODIM> dst_t;

		const drain::Flags & h5roles = Hdf5Context::h5_role::getShared();
		const drain::Flags::dict_t & dict = h5roles.getDict();
		//mout.warn(dict);
		if (!this->value.empty()){
			Hi5Tree & dst = ctx.getMyHi5(dict.getValue(this->value));
			DataSet<dst_t> dstDataSet(dst("dataset1"));
			const ODIM & odim = dstDataSet.getFirstData().odim;
			//std::cout << "\t\t" << odim << '\n';
			std::cout  << ctx.getId() << ' ' << "\t" << odim.quantity << ':' << odim.area << '\n';
		}
		else {
			for (const auto & entry: dict){
				std::cout << title << '=' << ctx.getId() << ' ' << entry.first << ':' << entry.second << '\n';
				try {
					Hi5Tree & dst = ctx.getMyHi5(entry.second);
					DataSet<dst_t> dstDataSet(dst("dataset1"));
					//Data<dst_t> & data = dstDataSet.getFirstData();
					const ODIM & odim = dstDataSet.getFirstData().odim;
					std::cout << "\t\t" << odim.quantity << ':' << odim.area << '\n';
				}
				catch (const std::exception & e) {
					std::cout << "\t\t" << "---\n";
				}
			}
		}
	}

};


class CmdDumpXML : public drain::SimpleCommand<std::string> {

public:

	CmdDumpXML() : drain::SimpleCommand<std::string>(__FUNCTION__, "Dump XML track") {
		//parameters.link("level", level = 5);
	}



	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		mout.deprecating("use -o <file>.svg");
		// tODO: detect bounding box

		drain::Output ofstr(value);
		drain::image::NodeSVG::toStream(ofstr, ctx.svgTrack);
		//ofstr << ctx.xmlTrack << '\n';

	}




};


/*
class CmdTrigger : public drain::BasicCommand {

public:

	CmdTrigger() : drain::BasicCommand(__FUNCTION__, "Trigger script") {
	}

	void exec() const {
	}

};
*/

HiddenModule::HiddenModule(){ //

	// Bank-referencing commands first
	// drain::HelpCmd help(cmdBank);

	// install<CmdTrigger>();
	install<CmdPause>();

	// install<CmdEcho>(); consider CmdRemark

	install<CmdInputFilter>();

	// install<CmdSelectOld>();

	install<CmdTest2>("restart", 'R');

	install<CmdHdf5Test>("getMyH5");

	install<CmdDumpXML>();

}


} // namespace rack
