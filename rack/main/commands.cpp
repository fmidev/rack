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



#include <drain/util/Debug.h>
#include <drain/util/RegExp.h>

#include <drain/image/Image.h>
#include <drain/image/TreeSVG.h>

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/Commands-ImageTools.h>

#include "rack.h"
#include "product/DataConversionOp.h"
#include "data/ODIM.h"
#include "hi5/Hi5.h"
#include "commands.h"
#include "data/DataCoder.h"

namespace rack {




// static CommandEntry<drain::CmdDebug> cmdDebug("debug");
// static CommandEntry<drain::CmdVerbose> cmdVerbose("verbose",'v');
// static CommandEntry<drain::CommandLoader> commandLoader( "execFile", 0);
// static CommandEntry<drain::CmdExpandVariables> expandVariables("expandVariables");

/** General commands.
 *
 */

//extern CommandEntry<drain::ScriptParser> scriptParser;
//extern CommandEntry<drain::ScriptExec> scriptExec;
//extern CommandEntry<drain::CmdExpandVariables> expandVariables;



class CmdSelect : public BasicCommand {
    public: //re 
	CmdSelect() : BasicCommand(__FUNCTION__, "Data selection for the next operation."){
		parameters.reference("selector", getResources().select, DataSelector().getParameters().getKeys());
		//s.getP
		parameters.separator = 0;
	};

	void exec() const {
		test.setParameters(getResources().select, '=', ',');
		getResources().dataOk = true;
	}

	mutable DataSelector test;
};
//extern CommandEntry<CmdSelect> cmdSelect;


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
		parameters.reference("type", odim.type = "C", "storage type (C,S,d,f,...)");
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

		MonitorSource mout(getName(), __FUNCTION__);

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
    public: //re 
	//std::string filename;

	CmdFormatOut() : SimpleCommand<std::string>(__FUNCTION__, "Dumps the formatted std::string to a file or stdout.", "filename","","std::string") {
		//parameters.separators.clear();
		//parameters.reference("filename", filename, "");
	};

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		std::string & format = cmdFormat.value;
		format = drain::String::replace(format, "\\n", "\n");
		format = drain::String::replace(format, "\\t", "\t");
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
			mout.info() << "writing " << outFileName.c_str() << mout.endl;
			std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			ofstr << reg.statusFormatter;
			//strm.toStream(ofstr, cmdStatus.statusMap.exportMap());
			ofstr.close();
		}

		//mout.warn() << "after expansion: " << r.statusFormatter << mout.endl;
		//r.statusFormatter.debug(std::cerr, r.getStatusMap());

	};

};





class CmdAutoExec : public BasicCommand {
    public: //re 
	CmdAutoExec() : BasicCommand(__FUNCTION__, "Execute script automatically after each input. See --script") {
		parameters.reference("exec", getResources().scriptParser.autoExec = -1, "0=false, 1=true, -1=set to true by --script");
	}

};
//static CommandEntry<CmdAutoExec> cmdAutoExec("autoExec");

class CmdDataOk : public BasicCommand {

public:

	CmdDataOk() : BasicCommand(__FUNCTION__, "Status of last select."){
		parameters.reference("flag", getResources().dataOk = true);
	};
};
// static CommandEntry<CmdDataOk> dataOk("dataOk", -1);

class UndetectWeight : public BasicCommand {  // TODO: move to general commands, leave warning here

public:

	UndetectWeight() : BasicCommand(__FUNCTION__, "Set the relative weight of data values assigned 'undetect'."){
		//parameters.reference("value", getResources().composite.undetectValue = 0.0, "quantity");
		//parameters.reference("weight", getResources().composite.undetectQualityCoeff = 0.0, "0...1");
		parameters.reference("weight", DataCoder::undetectQualityCoeff, "0...1");
	};


};
// static CommandEntry<UndetectWeight> undetectWeight("undetectWeight");

/// Default handler for requests without a handler. Handles options that are recognized as 1) files to be read or 2) ODIM properties to be assigned in current H5 structure.
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

		MonitorSource mout(getName());

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
//static CommandEntry<CmdDefaultHandler> cmdDefaultHandler(getRegistry().DEFAULT_HANDLER);
//extern CommandEntry<CmdDefaultHandler> cmdHandler;


class CmdSelectQuantity : public SimpleCommand<std::string> {
    public: //re 
	CmdSelectQuantity() : SimpleCommand<std::string>(__FUNCTION__, "Same as --select quantity.", "quantity"){};

	inline
	void exec() const {
		getRegistry().run("select", "quantity=" + value);
		//cmdSelect.run("quantity=" + value);
	}

};
// static CommandEntry<CmdSelectQuantity> cmdSelectQuantity("quantity",'Q');



class CmdAppend : public BasicCommand {
    public: //re 
	CmdAppend() : BasicCommand(__FUNCTION__, "Append outputs instead of overwriting. Deprecated, see --store"){};

	virtual
	void run(const std::string & params = "") {

		ProductOp::appendResults = !ProductOp::appendResults;
	};

};
//static CommandEntry<CmdAppend> cmdAppend("append");



class CmdCheckType : public BasicCommand {
    public: //re 
	CmdCheckType() : BasicCommand(__FUNCTION__, "Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings."){};

	void exec() const {
		PolarODIM::checkType(*getResources().currentHi5);
	};

};
//static CommandEntry<CmdCheckType> cmdCheckType("checkType");


class CmdConvert : public BasicCommand {
    public: //re 
	//std::string properties;

	CmdConvert() : BasicCommand(__FUNCTION__, "Convert --select'ed data to scaling and markers set by --target") {
		// parameters.separators.clear();
		//parameters.reference("properties", properties, "", cmdEncoding.odim.getKeys());
	};


	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout; = resources.mout;

		RackResources & resources = getResources();

		DataConversionOp op;
		op.setEncodingRequest(resources.targetEncoding);
		resources.targetEncoding.clear();

		//mout.info() << op.encodingRequest << mout.endl;

		//mout.warn() << op << mout.endl;

		op.dataSelector.setParameters(resources.select); //
		resources.select.clear();

		RackResources & r = getResources();
		op.processVolume(*r.currentHi5, *r.currentHi5);

	};

protected:

	//mutable



};
// static CommandEntry<CmdConvert> cmdConvert("convert");



class CmdDelete : public SimpleCommand<std::string> {
    public: //re 

	CmdDelete() : SimpleCommand<std::string>(__FUNCTION__, "Deletes selected parts of h5 structure.", "selector", "", DataSelector().getParameters().getKeys()) {
	};


	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		RackResources & resources = getResources();

		std::list<std::string> l;
		//DataSelector::getPaths(*getResources().currentHi5, DataSelector().setParameters(value, true), l);
		DataSelector::getPaths(*getResources().currentHi5, DataSelector(value), l);
		mout.info() << "deleting " << l.size() << " substructures" << mout.endl;
		//for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); it++){
		for (std::list<std::string>::const_reverse_iterator it = l.rbegin(); it != l.rend(); it++){
			mout.debug() << "deleting: " << *it << mout.endl;
			resources.currentHi5->erase(*it);
		}
	};

};
// static CommandEntry<CmdDelete> cmdDelete("delete");


class CmdDumpMap : public BasicCommand {
    public: //re 
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
// static CommandEntry<CmdDumpMap> cmdDumpMap("dumpMap"); // obsolete?







class CmdHelpRack : public CmdHelp {
public:
	CmdHelpRack() : CmdHelp(std::string(__RACK__) + " - a radar data processing program", "Usage: rack <input> [commands...] -o <outputFile>") {};
};
// static CommandEntry<CmdHelpRack> help("help", 'h');


class CmdHelpExample : public SimpleCommand<std::string> {
    public: //re 
	//std::string keyword;

	CmdHelpExample() : SimpleCommand<std::string>(__FUNCTION__, "Dump example of use and exit.", "keyword", "") {
		//parameters.reference("keyword", keyword, "");
	};


	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__);

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




class CmdKeep : public SimpleCommand<std::string> {
    public: //re 
	//std::string value;

	CmdKeep() : SimpleCommand<std::string>(__FUNCTION__, "Keeps a part of the current h5 structure, deletes the rest. Path and quantity are regexps.",
			"selector", "", DataSelector().getParameters().getKeys()) {
		//parameters.separators.clear();
		//parameters.reference("value", value, "", DataSelector().getKeys());
	};

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = resources.mout;

		std::set<std::string> s;
		DataSelector::getPaths(*getResources().currentHi5, DataSelector(value), s);

		std::list<std::string> l;
		getResources().currentHi5->getKeys(l);

		for (std::list<std::string>::const_reverse_iterator it = l.rbegin(); it != l.rend(); it++){
			// for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); it++){
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
// static CommandEntry<CmdKeep> cmdKeep("keep");





class CmdRenameODIM : public SimpleCommand<std::string> {
    public: //re 

	CmdRenameODIM() : SimpleCommand<std::string>(__FUNCTION__, "Rename path (future ext: attribute)", "from,to", "", "<path>[:<key>],[<path2>|<key2>],") { // TODO
		//parameters.separators.clear();
		//parameters.reference("assignment", assignment, "", "path:key=value");
	};

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__);

		//drain::RegExp pathSplitter("^([^:]+)(:(.+))?,([^:]*)(:(.+))?$");
		drain::RegExp pathSplitter("^([^:]+)(:(.+))?,(.+)?$");
		if (pathSplitter.execute(this->value) == 0){
			mout.warn() << "matches" << mout.endl;



			const std::string & path1 = pathSplitter.result[1];
			const std::string & attr1 = pathSplitter.result[3];
			const std::string & path2 = pathSplitter.result[4];
			//const std::string & attr2 = pathSplitter.result[6];

			//mout.warn() << path1 << '|' << attr1 << " => " << path2 << '|' << attr2 << mout.endl;
			mout.warn() << path1 << '|' << attr1 << " => " << path2 << mout.endl;

			RackResources & resources = getResources();
			if (resources.currentPolarHi5 == NULL){
				mout.error() << "currentPolarHi5 == NULL" << mout.endl;
				return;
			}
			HI5TREE & dst = *resources.currentPolarHi5;

			//if (attr1.empty() && attr2.empty()){
			if (attr1.empty()){
				mout.debug() << "renaming path" << path1 << " => " << path2 << mout.endl;
				dst[path2].swap(dst[path1]);
				dst[path2].data.attributes.swap(dst[path1].data.attributes);
				//dst.erase(path1);
				DataSelector::updateAttributes(dst[path2]);
			}
			else {//if (!attr1.empty() && path2.empty() && !attr2.empty()){
				mout.debug() << "renaming attribute (" << path1 << "):" << attr1 << " => " << path2 << mout.endl;
				HI5TREE & dst2 = dst[path1];
				if (dst2.data.attributes.hasKey(attr1)){
					dst2.data.attributes[path2] = dst2.data.attributes[attr1];
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
// static CommandEntry<CmdRenameODIM> cmdRename("rename");


class CmdSetODIM : public SimpleCommand<> {
    public: //re 
	CmdSetODIM() : SimpleCommand<std::string>(__FUNCTION__, "Set data properties (ODIM).",
			"assignment", "", "/<path>:<key>[=<value>]") {
	};

	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__);

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
				DataSelector::updateAttributes(currentHi5(attributeGroup.result[2]));  // one step above where,what,how
			}
			else {
				//mout.warn() << "update attributes under: " <<  *it << mout.endl;
				DataSelector::updateAttributes(currentHi5(*it));
			}
			// mout.warn() << "updateAttributes: " << attributeGroup.result[2] << '!' << attributeGroup.result[3] << mout.endl;
			// mout.warn() << "updateAttributes: " << *it << mout.endl;
		}

	};

};
// static CommandEntry<CmdSetODIM> cmdSetODIM("applyODIM");





class CmdVersion : public BasicCommand {
    public: //re 
	CmdVersion() : BasicCommand(__FUNCTION__, "Displays software version and quits."){};

	void exec() const {

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
// static CommandEntry<CmdVersion> cmdVersion("version");


class OutputDataVerbosity : public BasicCommand {
    public: //re 
	OutputDataVerbosity() : BasicCommand(__FUNCTION__, "Determines if also intermediate results (1) are saved. Replacing --aStore ?") {
		parameters.reference("level", ProductOp::outputDataVerbosity = 0, "0=default,1=intermediate results|2=extra debug results");
	};

	void exec() const {
		drain::MonitorSource mout(name, __FUNCTION__);
		mout.warn() << "deprecating option, use '--store intermediate=2' instead" << mout.endl;
	};

};
// static CommandEntry<OutputDataVerbosity> dataVebose("verboseData");





class CmdStore : public BasicCommand {
    public: //re 
	CmdStore() : BasicCommand(__FUNCTION__, "Set how intermediate and final outputs are stored."){
		parameters.reference("append",  append = 0, "append products in /datasets, don't overwrite");
		parameters.reference("intermediate", intermediate = 0, "store intermediate images");
	};

	int append;
	int intermediate;

	virtual
	void exec() const {
		//drain::MonitorSource mout(name, __FUNCTION__);
		ProductOp::appendResults = (append>0);
		DetectorOp::STORE = (intermediate>0);
		ProductOp::outputDataVerbosity = intermediate;
	};

};
// extern CommandEntry<CmdStore> cmdStore;
// static CommandEntry<CmdStore>  cmdStore("store");

class CmdQuantityConf : public BasicCommand {
    public: //re 

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

		MonitorSource mout(getName(), __FUNCTION__);

		setParameters(params);
		const size_t i = quantityType.find(':');
		const std::string quantity = quantityType.substr(0, i);
		const std::string type     = (i != std::string::npos) ? quantityType.substr(i+1) : std::string("");

		QuantityMap & m = getQuantityMap();
		mout.debug() << quantity << '(' << type << ')' << mout.endl;

		if (!quantity.empty()){

			Quantity & q = m[quantity];

			if (zero != std::numeric_limits<double>::min()){
				if (isnan(zero))  // TODO: convert toStr to double NaN
					q.unsetZero();
				else
					q.setZero(zero);
			}

			//std::cout << q;
			if (type.length() == 1){

				mout.debug(1) << "setting default type " << type << mout.endl;
				const char typecode = type.at(0);
				q.defaultType = typecode;
				q.set(typecode).setScaling(1.0, 0).updateValues(params);
				//q[typecode].updateValues(params);
				//q[typecode].type = type; // set by set() above
				//std::cout << "setting default type " << type << '\t';

				mout.debug() << "setting default type: " << quantity << '\n' << q << mout.endl;
			}
			else {  // No type given, dump quantity conf
				std::cout << quantity << '\n';
				if (params != quantity)
					std::cout << " *\t" << q.get(q.defaultType) << '\n';
				else
					std::cout << q;
			}
		}
		else { // No quantity given, dump all the quantities
			std::cout << "Quantities:\n";
			std::cout << m << '\n';
		}

		quantityType.clear();
		zero = std::numeric_limits<double>::min();  // what about max?
		//odim.type.clear();
		// odim.gain = 0.0;
	}

protected:
	std::string quantityType;
	double zero;
	EncodingODIM odim;

};
//extern CommandEntry<CmdQuantityConf> cmdQuantity;
// static CommandEntry<CmdQuantityConf> cmdQuantity( "quantityConf");

//



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
	static RackLetAdapter<drain::CmdVerbose> cmdVerbose("verbose",'v');
	static RackLetAdapter<drain::CommandLoader> commandLoader("execFile");
	static RackLetAdapter<drain::CmdExpandVariables> expandVariables;

	static RackLetAdapter<CmdAutoExec> cmdAutoExec;
	static RackLetAdapter<CmdDataOk> dataOk("dataOk", -1);
	static RackLetAdapter<UndetectWeight> undetectWeight("undetectWeight");

	static RackLetAdapter<CmdSelectQuantity> cmdSelectQuantity("quantity",'Q');
	static RackLetAdapter<CmdAppend> cmdAppend;
	static RackLetAdapter<CmdCheckType> cmdCheckType;
	static RackLetAdapter<CmdConvert> cmdConvert;
	static RackLetAdapter<CmdDelete> cmdDelete;
	static RackLetAdapter<CmdDumpMap> cmdDumpMap; // obsolete?
	static RackLetAdapter<CmdHelpRack> help("help", 'h');
	static RackLetAdapter<CmdHelpExample> cmdHelpExample;
	static RackLetAdapter<CmdKeep> cmdKeep;
	static RackLetAdapter<CmdRenameODIM> cmdRename;
	static RackLetAdapter<CmdSetODIM> cmdSetODIM;
	static RackLetAdapter<CmdVersion> cmdVersion;
	static RackLetAdapter<OutputDataVerbosity> dataVebose("verboseData");
	static RackLetAdapter<CmdStore>  cmdStore;
	static RackLetAdapter<CmdQuantityConf> cmdQuantity;


}


//void populateCommands(){}

} // namespace rack

// Rack
