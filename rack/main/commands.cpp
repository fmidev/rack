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

namespace rack {



/** General commands.
 *
 */


class CmdSelect : public BasicCommand {

public:

	CmdSelect() : BasicCommand(__FUNCTION__, "Data selection for the next operation."){
		parameters.reference("selector", getResources().select, DataSelector().getParameters().getKeys());
		parameters.separator = 0;
	};

	void exec() const {

		drain::Logger mout(getName(), __FUNCTION__);

		test.setParameters(getResources().select, '=', ',');
		mout.debug() << test << mout.endl;
		// test.getParameters()["elangle"].toJSON(std::cout, '\n');
		getResources().dataOk = true;

	}

	mutable DataSelector test;
};

class CmdSelectQuantity : public SimpleCommand<std::string> {

public:

	CmdSelectQuantity() : SimpleCommand<std::string>(__FUNCTION__, "Like --select quantity=... but with patterns (not regexps)", "quantities", "quantity[,quantity2,...]"){

	};

	inline
	void exec() const {

		drain::Logger mout(getName(), __FUNCTION__);
		const std::string v = StringTools::replace(StringTools::replace(StringTools::replace(value,",","|"), "*",".*"), "?", "[.]");
		mout.debug() << v << mout.endl;
		getResources().select = "quantity=^(" + v + ")$";
		//getRegistry().run("select", "quantity=^(" + vField + ")$");
	}

};


class CmdStatus : public BasicCommand {
public: //re
	CmdStatus() : BasicCommand(__FUNCTION__, "Dump information on current images.") {
		//parameters.reference("ls", value, "dw", "Accumulation layers to be extracted");
	};

	void exec() const {
		// map_t::iterator it = registry.find(name);
		std::ostream & ostr = std::cout; // for now...

		//drain::CommandRegistry & registry = getRegistry();

		/// Update latest file contents etc.
		//getResources().updateStatusMap();

		const drain::VariableMap & statusMap = getResources().getUpdatedStatusMap();
		for (drain::VariableMap::const_iterator it = statusMap.begin(); it != statusMap.end(); ++it){
			ostr << it->first << '=' << it->second << ' ';
			it->second.typeInfo(ostr);
			ostr << '\n';
		}
		// ostr << statusMap << std::endl;

	};

	//void updateStatusMap() const;

};
//extern CommandEntry<CmdStatus> cmdStatus;


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

		//getQuantityMap().setTypeDefaults(odim, "C");
		odim.setTypeDefaults("C");
		//odim.setTypeDefaults(typeid(unsigned char));
		//std::cerr << "CmdEncoding.odim:" << odim << std::endl;
		//std::cerr << "CmdEncoding.pars:" << parameters << std::endl;
	};


	virtual  //?
	inline
	void run(const std::string & params){

		Logger mout(getName(), __FUNCTION__);

		try {

			/// Main action: store it...
			getResources().targetEncoding = params;

			/// ...But check and warn of unknown parameters
			parameters.setValues(params);  // sets type, perhaps, hence set type defaults and override them with user defs

			// mout.note() << "pars: " << parameters << mout.endl;
			// mout.note() << "odim: " << odim << mout.endl;
			//getQuantityMap().setTypeDefaults(odim, odim.type);
			odim.setTypeDefaults();

			//mout.note() << "Set odim defaults: " << odim << mout.endl;
			parameters.setValues(params);

			//mout.note() << "Re-assigned parameters: " << parameters << mout.endl;

		} catch (std::runtime_error & e) {

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
		//options["RANGE"] = options.get("what:rscale",500) * options.get("@what:nbins",500) ;

		//cmdStatus.updateStatusMap();
		//resources.updateStatusMap();

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
 *   - \c --/dataset2/data2:
 *
 */
class CmdDefaultHandler : public BasicCommand {
public: //re
	//std::string value;
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


class CmdCreateDefaultQuality : public BasicCommand {

public:

	CmdCreateDefaultQuality() : BasicCommand(__FUNCTION__, "Creates default quality field. See --undetectQuality"){
		parameters.reference("quantitySpecific", quantitySpecific=false, "[0|1]");
	};

	bool quantitySpecific;

	template <class OD>
	void processStructure(HI5TREE & dst, const std::list<std::string> & paths, const drain::RegExp & quantityRegExp) const {

		drain::Logger mout(getName(), __FUNCTION__);

		typedef DstType<OD> DT; // ~ PolarDst, CartesianDst

		const QuantityMap & qmap = getQuantityMap();

		for (std::list<std::string>::const_iterator it = paths.begin(); it != paths.end(); ++it){
			mout.info() << *it  << mout.endl;
			HI5TREE & dstDataSetH5 = dst(*it);
			DataSet<DT> dstDataSet(dstDataSetH5, quantityRegExp);
			if (quantitySpecific){
				for (typename DataSet<DT>::iterator it2 = dstDataSet.begin(); it2!=dstDataSet.end(); ++it2){
					mout.warn() << '\t' << it2->first  << mout.endl;
					Data<DT> & dstData = it2->second;
					PlainData<DT> & dstQuality = dstData.getQualityData();
					if (!dstQuality.data.isEmpty())
						mout.warn() << "quality data exists already, overwriting" << mout.endl;
					//qmap.setQuantityDefaults(dstQuality, "QIND");dstQuality
					//dstQuality.setGeometry(dstData.data.getWidth(), dstData.data.getHeight());
					//dstData.createSimpleQualityData(dstQuality.data, 255.0, 0.0);
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
				//dstQuality.setGeometry(dstData.data.getWidth(), dstData.data.getHeight());
				//dstData.createSimpleQualityData(dstQuality.data, 255.0, 0.0);
				dstData.createSimpleQualityData(dstQuality, 1.0, 0.0, DataCoder::undetectQualityCoeff);
				qmap.setQuantityDefaults(dstQuality, "QIND");
				dstQuality.data.setScaling(dstQuality.odim.gain, dstQuality.odim.offset);
				//@ dstQuality.updateTree();
			}
			//@  DataTools::updateAttributes(dstDataSetH5);

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
		if (selector.path.empty())
			selector.path = "dataset[0-9]+$";

		std::list<std::string> paths;
		DataSelector::getPaths(*resources.currentHi5, selector, paths);


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

		DataTools::updateAttributes(*resources.currentHi5);
	};

};


class CmdCompleteODIM : public BasicCommand {

public:

	CmdCompleteODIM() : BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	template <class OD>  // const drain::VariableMap & rootProperties,
	void complete(HI5TREE & dstH5, OD & od) const {

		drain::Logger mout(getName(), __FUNCTION__);

		typedef DstType<OD> DT;
		//typedef DataSet<DT> DS;

		static const drain::RegExp reg("dataset[0-9]+");
		OD rootODIM;

		for (HI5TREE::iterator it = dstH5.begin(); it != dstH5.end(); ++it){
			if (reg.test(it->first)){
				mout.debug() << it->first << mout.endl;
				DataSet<DT> dstDataSet(it->second);
				for (typename DataSet<DT>::iterator dit = dstDataSet.begin(); dit != dstDataSet.end(); ++dit){
					mout.debug() << dit->first << " :" << dit->second << mout.endl;
					Data<DT> & dstData = dit->second;
					if (!dstData.data.isEmpty()){
						const size_t w = dstData.data.getWidth();
						const size_t h = dstData.data.getHeight();
						dstData.odim.type = (char)dstData.data.getType2();
						dstData.odim.setGeometry(w, h);
						//rootODIM.setGeometry(w, h);    // for Cartesian root-level xsize,ysize
						//dataSetODIM.setGeometry(w, h); // for Polar  dataset-level nbins, nrays
						if (dstData.odim.quantity.empty()){
							dstData.odim.quantity = dit->first;
						}
						if (dstData.odim.gain == 0){
							getQuantityMap().setQuantityDefaults(dstData.odim, dstData.odim.quantity);
						}
					}

					dstData.odim.updateFromMap(dstData.data.properties); // assume UpdateMetadata
					//rootODIM.updateFromMap(d.data.properties);
					rootODIM.update(dstData.odim);
					//odim.copyToData(dst);
					//dstData.odim.update(dstData.odim);
				}
				//dataSet.updateTrdtee3(rootODIM);

				//dataSet.updateTree(dataSetODIM);
			}
			//rootODIM.copyToRoot()
		}
		//rootODIM.

		/*
		if (rootODIM.date.empty())
			rootODIM.date = rootODIM.startdate;
		if (rootODIM.time.empty())
			rootODIM.time = rootODIM.starttime;
		*/
		ODIM::copyToH5<ODIM::ROOT>(rootODIM, dstH5);

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
		DataTools::updateAttributes(*resources.currentHi5);

	}

};



class CmdDelete : public SimpleCommand<std::string> {

public:

	CmdDelete() : SimpleCommand<std::string>(__FUNCTION__, "Deletes selected parts of h5 structure.", "selector", "", DataSelector().getParameters().getKeys()) {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		const bool DELETE_NOSAVE = value.empty();
		if (DELETE_NOSAVE){

		}

		std::list<std::string> l;
		DataSelector  s;
		s.setParameters(value);
		DataSelector::getPaths(*getResources().currentHi5, s, l);
		//DataSelector::getPaths
		//DataSelector::getPaths(*getResources().currentHi5, DataSelector(value), l);
		mout.info() << "deleting " << l.size() << " substructures" << mout.endl;
		for (std::list<std::string>::const_reverse_iterator it = l.rbegin(); it != l.rend(); it++){
			mout.debug() << "deleting: " << *it << mout.endl;
			if (DELETE_NOSAVE){
				//HI5TREE & dst = resources.currentHi5(*it);
				if ((*resources.currentHi5)(*it).data.noSave){
					resources.currentHi5->erase(*it);

				}
			}
			else {
				resources.currentHi5->erase(*it);
				mout.debug() << "deleting: " << *it << mout.endl;
			}
		}
	};

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

			ostr << "--" << key << ' ' << '\'';
			params.getValues(ostr);
			ostr << '\'' << '\n';
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

class CmdJSON : public BasicCommand {
public:

	CmdJSON() : BasicCommand(__FUNCTION__, "Dump to JSON."){};

	virtual
	void run(const std::string & params = "") {
		getRegistry().toJSON(std::cout);
	};

};



class CmdKeep : public SimpleCommand<std::string> {
public: //re
	//std::string value;

	CmdKeep() : SimpleCommand<std::string>(__FUNCTION__, "Keeps a part of the current h5 structure, deletes the rest. Path and quantity are regexps.",
			"selector", "", DataSelector().getParameters().getKeys()) {
		//parameters.separators.clear();
		//parameters.reference("value", value, "", DataSelector().getKeys());
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__); // = resources.mout;

		std::set<std::string> s;
		DataSelector::getPaths(*getResources().currentHi5, DataSelector(value), s);

		std::list<std::string> l;
		getResources().currentHi5->getKeys(l);

		for (std::list<std::string>::const_reverse_iterator it = l.rbegin(); it != l.rend(); it++){

			if (s.find(*it) != s.end()){
				mout.debug() << "keeping: " << *it << mout.endl;
			}
			else {
				mout.debug(1) << "deleting: " << *it << mout.endl;
				getResources().currentHi5->erase(*it);  // TODO check if nonexistent?
			}
		}

	};

};






class CmdRename : public SimpleCommand<std::string> {

public:

	CmdRename() : SimpleCommand<std::string>(__FUNCTION__, "Rename/swap path names (future ext: attribute)", "from,to", "", "<path>[:<key>],[<path2>|<key2>],") { // TODO
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		//drain::RegExp pathSplitter("^([^:]+)(:(.+))?,([^:]*)(:(.+))?$");
		const drain::RegExp pathSplitter("^([^:]+)(:(.+))?,(.+)?$");

		if (pathSplitter.execute(this->value) == 0){

			mout.debug() << "matches" << mout.endl;

			const std::string & path1 = pathSplitter.result[1];
			const std::string & attr1 = pathSplitter.result[3];
			const std::string & path2 = pathSplitter.result[4];
			//const std::string & attr2 = pathSplitter.result[6];

			//mout.warn() << path1 << '|' << attr1 << " => " << path2 << '|' << attr2 << mout.endl;
			if (attr1.empty())
				mout.note() << path1 << " => " << path2 << mout.endl;
			else
				mout.note() << path1 << '|' << attr1 << " => " << path2 << mout.endl;

			RackResources & resources = getResources();
			if (resources.currentHi5 == NULL){
				mout.error() << "current Hi5 == NULL" << mout.endl;
				return;
			}
			HI5TREE & dst = *resources.currentHi5;

			//if (attr1.empty() && attr2.empty()){
			if (attr1.empty()){
				mout.debug() << "renaming path" << path1 << " => " << path2 << mout.endl;
				//mout.warn() << dst << mout.endl;
				HI5TREE & d1 = dst(path1);
				HI5TREE & d2 = dst(path2);
				const bool noSave1 = d1.data.noSave;
				const bool noSave2 = d2.data.noSave;
				d2.swap(d1);
				d1.data.noSave = noSave2;
				d2.data.noSave = noSave1;
				d2.data.attributes.swap(d2.data.attributes);
				//dst.erase(path1);
				DataTools::updateAttributes(d1);
				DataTools::updateAttributes(d2);
				//mout.note() << dst << mout.endl;
			}
			else {//if (!attr1.empty() && path2.empty() && !attr2.empty()){
				mout.debug() << "renaming attribute (" << path1 << "):" << attr1 << " => " << path2 << mout.endl;
				HI5TREE & dst2 = dst[path1];
				if (dst2.data.attributes.hasKey(attr1)){
					dst2.data.attributes[path2] = dst2.data.attributes[attr1]; //???
					dst2.data.attributes.erase(attr1);
				}
				//dst.erase(path1);
			}
			/*
				else {
					mout.warn() << " illegal rename operation: " << path1 << ':' << attr1 << " => " << path2 << ':' << attr2 << mout.endl;
				}
			 */
			//if ()

		}
		else {
			mout.error() << "argument <from,to> syntax error; regexp: " << pathSplitter.toStr() << mout.endl;
		}

	}
};
// static CommandEntry<CmdRename> cmdRename("rename");


class CmdSetODIM : public SimpleCommand<> {

public:

	CmdSetODIM() : SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		drain::Logger mout(name, __FUNCTION__);

		// mout.warn() << "hello" << mout.endl;

		// List of paths in which assignments are repeated.
		// A single path, unless search key is a regexp, see below.
		std::list<std::string> pathList;

		const std::string & s = value;

		const size_t i = s.find(':');
		const size_t j = s.find('=');

		const std::string groupPath = s.substr(0, i);
		const std::string attributeKey = (i != std::string::npos) ? s.substr(i,j-i) : ""; // NOTE substr(i), not (i+1) => if non-empty, contains ':' as prefix
		const std::string value = s.substr(j+1);

		mout.debug(5) << "Group path:" << groupPath << ' ';
		mout << "Attribute:"  << attributeKey << ' ';
		mout << "Value:"      << value << mout.endl;

		HI5TREE & currentHi5 = *(getResources().currentHi5);

		if (groupPath.find_first_of(".?*[]") == std::string::npos){
			pathList.push_front(groupPath);
		}
		else {
			DataSelector::getPaths(*getResources().currentHi5, pathList, groupPath);
			// mout.warn()  << " --/ multiple, ok" << mout.endl;
		}

		static
		const drain::RegExp attributeGroup("^((.*)/)?(what|where|how)$");  // $1,$2: one step above "where","what","how"

		for (std::list<std::string>::const_iterator it = pathList.begin(); it != pathList.end(); ++it){
			// mout.warn()  << " multiple: key=" << *it << "|"  << attributeKey << mout.endl;
			hi5::Hi5Base::readTextLine(currentHi5, *it+attributeKey, value);

			if (attributeGroup.execute(*it) == 0){ // match
				// mout.warn() << "update attributes under: " <<  attributeGroup.result[2] << mout.endl;
				DataTools::updateAttributes(currentHi5(attributeGroup.result[2]));  // one step above where,what,how
			}
			else {
				//mout.warn() << "update attributes under: " <<  *it << mout.endl;
				DataTools::updateAttributes(currentHi5(*it));
			}
			// mout.warn() << "updateAttributes: " << attributeGroup.result[2] << '!' << attributeGroup.result[3] << mout.endl;
			// mout.warn() << "updateAttributes: " << *it << mout.endl;
		}

	};

};
// static CommandEntry<CmdSetODIM> cmdSetODIM("applyODIM");





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



class CmdAppend : public BasicCommand {

public:

	CmdAppend() : BasicCommand(__FUNCTION__, "Append inputs/outputs instead of overwriting."){
		parameters.reference("path", path, "|data<n>|dataset<n>");
	};

	virtual
	void exec() const {

		if (path == "dataset")
			ProductBase::appendResults.set(BaseODIM::DATASET);
		else if (path == "data")
			ProductBase::appendResults.set(BaseODIM::DATA);
		else
			ProductBase::appendResults.set(path); // possibly "data4" or "dataset7"

		if (!BaseODIM::isIndexed(ProductBase::appendResults.getType()) && ! (ProductBase::appendResults != BaseODIM::NONE)){
			drain::Logger mout(name, __FUNCTION__);
			mout.warn() << "illegal path elem '"<< path << "'" << mout.endl;
		}
		/*
		if (!ProductBase::appendResults.empty()){
			if ((ProductBase::appendResults != "dataset") && (ProductBase::appendResults != "data")){
				drain::Logger mout(name, __FUNCTION__);
				mout.warn() << "value '"<< ProductBase::appendResults << "'? (should be empty, 'data' or 'dataset')" << mout.endl;
			}

		}
		*/
	}

	std::string path;

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
		//ProductOp::appendResults = (append>0);
		DetectorOp::STORE = (ProductBase::outputDataVerbosity>0);
		//ProductOp::outputDataVerbosity = level;
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
			if (isnan(zero))        // TODO: convert toStr to double NaN
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



CommandModule::CommandModule(){ //
	//: scriptParser("script", 0), scriptExec( "exec", 'e', ScriptExec(scriptParser.script)), cmdSelect( "select", 's'),
	//cmdStatus("status"), cmdEncoding( "target", 't'), cmdFormatOut("formatOut"){

	CommandRegistry & registry = getRegistry();
	RackResources & resources = getResources();

	static RackLetAdapter<CmdDefaultHandler> cmdDefaultHandler(registry.DEFAULT_HANDLER);

	registry.add(resources.scriptParser, "script");

	static ScriptExec scriptExec(resources.scriptParser.script);
	registry.add(scriptExec, "exec");
	//static RackLetAdapter<drain::ScriptParser> scriptParser("script");
	//static RackLetAdapter<drain::ScriptExec> scriptExec( "exec", 'e', ScriptExec(scriptParser.script) );

	static RackLetAdapter<CmdSelect> cmdSelect("select", 's');
	static RackLetAdapter<CmdStatus> cmdStatus; // ("status");
	static RackLetAdapter<CmdEncoding> cmdTarget("target", 't');  // old
	static RackLetAdapter<CmdEncoding> cmdEncoding("encoding", 'e');  // new
	static RackLetAdapter<CmdFormatOut> cmdFormatOut;


	static RackLetAdapter<drain::CmdDebug> cmdDebug;
	static RackLetAdapter<CmdVerbose2> cmdVerbose("verbose",'v');
	static RackLetAdapter<drain::CommandLoader> commandLoader("execFile");
	static RackLetAdapter<drain::CmdExpandVariables> expandVariables;

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
	static RackLetAdapter<CmdHelpRack> help("help", 'h');
	static RackLetAdapter<CmdHelpExample> cmdHelpExample;
	static RackLetAdapter<CmdJSON> cmdJSON;
	static RackLetAdapter<CmdKeep> cmdKeep;
	static RackLetAdapter<CmdRename> cmdRename;
	static RackLetAdapter<CmdSetODIM> cmdSetODIM;
	static RackLetAdapter<CmdVersion> cmdVersion;
	static RackLetAdapter<OutputDataVerbosity> dataVebose("verboseData");

	static RackLetAdapter<CmdAppend>  cmdAppend;
	static RackLetAdapter<CmdStore>  cmdStore;

	static RackLetAdapter<CmdQuantityConf> cmdQuantity;


}


//void populateCommands(){}

} // namespace rack

// Rack
