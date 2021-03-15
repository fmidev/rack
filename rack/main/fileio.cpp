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

//#include <exception>
#include <fstream>
#include <iostream>
/*
#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
*/
//#include <regex.h>
// #include <stddef.h>

#include "drain/util/Log.h"
#include "drain/util/FilePath.h"
#include "drain/util/Output.h"
#include "drain/util/StringMapper.h"
#include "drain/util/Tree.h"
#include "drain/util/Variable.h"
#include "drain/image/File.h"
#include "drain/image/Image.h"
#include "drain/image/Sampler.h"
#include "drain/imageops/ImageModifierPack.h"

#include "drain/prog/Command.h"
#include "drain/prog/CommandBankUtils.h"
#include "drain/prog/CommandInstaller.h"

#include "data/Data.h"
#include "data/DataOutput.h"
#include "data/DataSelector.h"
#include "data/DataTools.h"
#include "data/ODIMPath.h"
#include "data/PolarODIM.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "product/ProductOp.h"
#include "radar/FileGeoTIFF.h"
#include "radar/RadarDataPicker.h"

#include "fileio.h"
#include "fileio-read.h"
#include "resources.h"






namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.

const drain::RegExp h5FileExtension("^((.*\\.(h5|hdf5|hdf))|((.*/)?[\\w]+))$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising GeoTIFF files.
const drain::RegExp tiffFileExtension(".*\\.(tif|tiff)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
//const drain::RegExp pngFileExtension(".*\\.(png)$", REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
//const drain::RegExp pnmFileExtension(".*\\.(p[nbgp]m)$", REG_EXTENDED | REG_ICASE); //[nbgp]

/// Syntax for recognising text files.
const drain::RegExp textFileExtension(".*\\.(txt)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising numeric array files (in plain text format anyway).
const drain::RegExp arrayFileExtension(".*\\.(mat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for sparsely resampled data.
const drain::RegExp sampleFileExtension(".*\\.(dat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising Graphviz DOT files.
const drain::RegExp dotFileExtension(".*\\.(dot)$",  REG_EXTENDED | REG_ICASE);



//static DataSelector imageSelector(".*/data/?$","");   // Only for images. Not directly accessible.
//static DataSelector imageSelector;  // Only images. Not directly accessible. Consider that of images.h


struct HistEntry : drain::BeanLike {

	HistEntry() : drain::BeanLike(__FUNCTION__), index(0), count(0){
		parameters.link("index", index);
		parameters.link("min", binRange.min);
		parameters.link("max", binRange.max);
		parameters.link("count", count);
		parameters.link("label", label);
	};

	drain::Histogram::vect_t::size_type index;
	drain::Range<double> binRange;
	drain::Histogram::count_t count;
	std::string label;

};
static HistEntry histEntryHelper;


/// TODO: generalize to array outfile
class CmdHistogram : public drain::BasicCommand {

public:

	int count;

	drain::Range<double> range;

	std::string store;
	std::string filename;

	//	CmdHistogram() : drain::SimpleCommand<int>(__FUNCTION__, "Histogram","slots", 256, "") {
	CmdHistogram() : drain::BasicCommand(__FUNCTION__, std::string("Histogram. Optionally --format using keys ") + histEntryHelper.getParameters().getKeys()) {
		parameters.link("count", count = 256);
		parameters.link("range", range.tuple());
		//parameters.link("max", maxValue = +std::numeric_limits<double>::max());
		parameters.link("filename", filename="", "<filename>.txt|-");
		parameters.link("store", store="histogram", "<attribute_key>");
	};

	CmdHistogram(const CmdHistogram & cmd): drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	};
	// virtual	inline const std::string & getDescription() const { return description; };


	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__); // getResources().mout;

		Hi5Tree & currentHi5 = *ctx.currentHi5;

		DataSelector selector(ODIMPathElemMatcher::DATA);
		//selector.pathMatcher.clear();
		//selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElemMatcher::DATA);
		selector.setParameters(ctx.select);

		ODIMPath path;
		selector.getPath3(currentHi5, path);
		ctx.select.clear();



		PlainData<BasicDst> dstData(currentHi5(path));

		mout.note() << "path: " << path << " [" << dstData.odim.quantity << ']' << mout.endl;

		// NO resources.setCurrentImage(selector);
		// drain::image::Image & img = *ctx.currentImage;
		mout.warn() << "img " << dstData.data << mout.endl;

		drain::Histogram histogram(256);
		histogram.setScale(dstData.data.getScaling());
		histogram.compute(dstData.data, dstData.data.getType());

		if (!filename.empty()){

			drain::Output out((filename == "-") ? filename : ctx.outputPrefix + filename);

			std::ostream & ostr = out;

			drain::StringMapper mapper;
			if (! ctx.formatStr.empty()){
				//std::string format(cmdFormat.value);
				//format = drain::StringTools::replace(format, "\\t", "\t");
				//format = drain::StringTools::replace(format, "\\n", "\n");
				mapper.parse(ctx.formatStr, true);
			}
			else
				mapper.parse("${count} # '${label}' (${index}) [${min}, ${max}] \n", false); // here \n IS newline...

			// Header
			ostr << "# [0," << histogram.getSize() << "] ";
			if (histogram.scaling.isPhysical())
				ostr << '[' << histogram.scaling.physRange << ']';
			ostr << '\n';

			HistEntry entry;
			const drain::Histogram::vect_t & v = histogram.getVector();

			drain::VariableMap & dstWhat = dstData.getWhat();
			if (dstWhat.hasKey("legend")){
				mout.note() << "Using legend" <<  dstWhat["legend"] << mout.endl;
				typedef std::map<int, std::string> legend;
				legend leg;
				dstWhat["legend"].toMap(leg, ',', ':'); // TODO: StringTools::toMap

				for (legend::const_iterator it=leg.begin(); it!=leg.end(); ++it){
					ostr << "# " << it->first << '=' << it->second << '\n';
				}
				for (legend::const_iterator it=leg.begin(); it!=leg.end(); ++it){
					entry.index = it->first;
					entry.count = v[it->first];
					entry.binRange.min = histogram.scaling.fwd(it->first);
					entry.binRange.max = histogram.scaling.fwd(it->first + 1);
					entry.label = it->second; // or parameters.reference?
					mapper.toStream(ostr, entry.getParameters());
				}
				ostr << '\n';
			}
			else {
				mout.note() << "No legend found, writing all elements" << mout.endl;
				for (std::size_t i=0; i<v.size(); ++i){

					entry.index = i;
					entry.count = v[i];
					entry.binRange.min = histogram.scaling.fwd(i);
					entry.binRange.max = histogram.scaling.fwd(i+1);

					if (i == dstData.odim.nodata)
						entry.label = "nodata";
					else if (i == dstData.odim.undetect)
						entry.label = "undetect";
					else
						entry.label.clear();

					mapper.toStream(ostr, entry.getParameters());
				}
				ostr << '\n';
			}

			// histogram.dump(out);
		}

		if (!store.empty()){
			dstData.getHow()[store] = histogram.getVector();
			//dstData.updateTree2();
		}

	}

};





class CmdGeoTiffTile : public drain::BasicCommand {

public:

	CmdGeoTiffTile() : drain::BasicCommand(__FUNCTION__, "GeoTIFF tile size. Deprecating, use --outputConf tif:<width>,<height>") {
		parameters.link("tilewidth", FileGeoTIFF::tileWidth=256);
		parameters.link("tileheight", FileGeoTIFF::tileHeight=0);
	};

	CmdGeoTiffTile(const CmdGeoTiffTile & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK); // static targets
	}

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		mout.deprecating() = "In future versions, use --outputConf tif,<params>";
	}
};


class CmdOutputConf : public drain::BasicCommand {

public:

	CmdOutputConf() : drain::BasicCommand(__FUNCTION__, "Format specific configurations") {

		parameters.separator = ':';
		parameters.link("format", format, "h5|png|tif");
		parameters.link("params", params, "<key>=<value>[,<key2>=<value2>,...]");

		gtiffConf.link("tilewidth", FileGeoTIFF::tileWidth=256);
		gtiffConf.link("tileheight", FileGeoTIFF::tileHeight=0);
		gtiffConf.link("compression", FileGeoTIFF::compression, FileGeoTIFF::getCompressionDict().toStr('|'));

	};

	CmdOutputConf(const CmdOutputConf & cmd) : drain::BasicCommand(cmd) {
		parameters.copyStruct(cmd.getParameters(), cmd, *this);
	}

	std::string format;
	std::string params;

	/*
	virtual inline
	void set Parameters(const std::string & params, char assignmentSymbol=0) {
		 drain::BasicCommand::setParameters(params, 0); // sep = ':'
	}
	*/

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		mout.warn() << "Redesigned (setParameters dropped). Check results." << mout.endl;
		// mout.warn() << format << mout.endl;
		// mout.note() << params << mout.endl;
		// todo: shared resource for output conf:  refMap of refMaps...
		// todo recognize tif,TIFF
		if (format == "h5"){
			mout.unimplemented() << "(future extension)" << mout.endl;
		}
		else if (format == "png"){
			mout.unimplemented() << "(future extension)" << mout.endl;
		}
		else if (format == "tif"){
			// mout.note() <<  gtiffConf.getKeys() << mout.endl;
			if (!params.empty())
				gtiffConf.setValues(params);
			else
				mout.info() << gtiffConf << mout;
		}
		else {
			mout.warn() << "format '" << format << "' not recognized" << mout.endl;
		}

	}

	mutable
	drain::ReferenceMap gtiffConf;

};

/*
class CmdOutputTiffConf : public drain::BasicCommand {

public:

	CmdOutputTiffConf() : drain::BasicCommand(__FUNCTION__, "GeoTIFF configuration") {
		parameters.link("tilewidth", FileGeoTIFF::tileWidth=256);
		parameters.link("tileheight", FileGeoTIFF::tileHeight=0);
		parameters.link("compression", FileGeoTIFF::compression, FileGeoTIFF::getCompressionDict().toStr('|'));
	};


};
*/


// Cf. InputPrefix
class CmdOutputPrefix : public drain::SimpleCommand<std::string> {
//class CmdOutputPrefix : public drain::BasicCommand {

public:

	CmdOutputPrefix() : drain::SimpleCommand<std::string> (__FUNCTION__, "Path prefix for output files."){
	};

	inline
	void exec() const {
		getContext<RackContext>().outputPrefix = value;
	}
};
//extern drain::CommandEntry<CmdOutputPrefix> cmdOutputPrefix;


class CmdOutputFile : public drain::SimpleCommand<std::string> {

public:

	CmdOutputFile() : drain::SimpleCommand<>(__FUNCTION__, "Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.",
			"filename", "", "<filename>.[h5|hdf5|png|pgm|txt|dat|mat|dot]|-") {
		//execRoutine = false;
	};


	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		if (value.empty()){
			mout.error() << "File name missing. (Use '-' for stdout.)" << mout.endl;
			return;
		}

		if (ctx.statusFlags.isSet(drain::StatusFlags::INPUT_ERROR)){
			mout.warn() << "input failed, skipping" << mout.endl;
			return;
		}

		if (ctx.statusFlags.isSet(drain::StatusFlags::DATA_ERROR)){
			mout.warn() << "data error, skipping" << mout.endl;
			return;
		}

		std::string filename;

		if (value != "-"){
			//mout.warn() << RackContext::variableMapper << mout.endl;
			drain::StringMapper mapper(RackContext::variableMapper);
			mapper.parse(ctx.outputPrefix + value);
			filename = mapper.toStr(ctx.getStatusMap());
			mout.note() << "writing: '" << filename << "'" << mout.endl;
		}
		else {
			filename = "-";
		}

		// mout.note() << "filename: " << filename << mout.endl;

		// TODO: generalize select
		// TODO: generalize image pick (current or str) for png/tif

		const bool IMAGE_PNG = drain::image::FilePng::fileNameRegExp.test(value);  //pngFileExtension.test(value);
		const bool IMAGE_PNM = drain::image::FilePnm::fileNameRegExp.test(value);
		const bool IMAGE_TIF = tiffFileExtension.test(value);

		Hi5Tree & src = ctx.getHi5(RackContext::CURRENT); // mostly shared (unneeded in image output, but fast anyway)

		if (h5FileExtension.test(value)){

			mout.info() << "File format: HDF5" << mout.endl;
			src.data.attributes["Conventions"] = "ODIM_H5/V2_2"; // CHECK
			hi5::Writer::writeFile(filename, src); //*ctx.currentHi5);
			// ctx.outputPrefix + value

		}
		else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {

			// This is the simple version. See image commands (--iXxxxx)

			mout.info() << "File format: image" << mout.endl;

			/*
			if (ctx.select.empty()){
				ctx.findImage();
			}
			*/
			ctx.updateCurrentImage();

			//const drain::image::Image & src = ctx.getCurrentImage(); // ImageKit::getCurrentImage(ctx);

			// Use ctx.select and/or ctx.targetEncoding, if defined.
			const drain::image::Image & src = ctx.updateCurrentImage();

			if (!src.isEmpty()){

				if (!ctx.formatStr.empty()){
					mout.special() << "formatting comments" << mout.endl;
					drain::StringMapper statusFormatter(RackContext::variableMapper);
					//drain::StringMapper statusFormatter(ctx.formatStr, "[a-zA-Z0-9:_]+");
					statusFormatter.parse(ctx.formatStr, true);
					drain::image::Image &dst = (drain::image::Image &)src; // violence...
					dst.properties[""] = statusFormatter.toStr(ctx.getStatusMap(), 0);
					ctx.formatStr.clear(); // OK?
				}

				if (IMAGE_PNG || IMAGE_PNM){
					mout.debug() << "PNG or PGM format" << mout.endl;
					drain::image::File::write(src, filename);
				}
				else if (IMAGE_TIF) {
					// see FileGeoTiff::tileWidth
					FileGeoTIFF::write(filename, src); //, geoTIFF.width, geoTIFF.height);
				}
				else {
					ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR || drain::StatusFlags::OUTPUT_ERROR);
					mout.error() << "unknown file name extension" << mout.endl;
				}
			}
			else {
				ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
				mout.warn() << "empty data, skipped" << mout.endl;
			}
			/*
			} catch (std::exception &e) {
				mout.warn() << "failed in writing " << value << ": " << e.what() << mout.endl;
			} catch (...) {
				mout.warn() << "failed in writing " << value << mout.endl;
			}*/
		}
		else if (arrayFileExtension.test(filename)){

			/// Currently designed only for vertical profiles produced by VerticalProfileOp (\c --pVerticalProfile )
			/// TODO: modify DataSet such that the quantities appear in desired order.

			mout.info() << "File format: .mat (text dump of data)" << mout.endl;

			DataSelector selector;
			selector.setParameters(ctx.select);
			ctx.select.clear();
			//selector.path = "/data$";
			mout.debug2() << selector << mout.endl;
			const drain::RegExp quantityRegExp(selector.quantity);
			const bool USE_COUNT = quantityRegExp.test("COUNT");
			// mout.debug3() << "use count" << static_cast<int>(USE_COUNT) << mout.endl;

			const DataSet<PolarSrc> product(src["dataset1"], drain::RegExp(selector.quantity));
			//const DataSet<> product((*ctx.currentHi5), selector);

			std::string mainQuantity; // = product.getFirstData().odim.quantity;

			const drain::RegExp mainQuantityRegExp("HGHT");
			for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
				if ((it->second.data.getWidth()!=1) && (it->second.data.getHeight()!=1)){
					mout.warn() << "skipping non-1D data, quantity: " << it->first << mout.endl;
					continue;
				}
				if (mainQuantity.empty()){
					mainQuantity = it->first;
				}
				else if (mainQuantityRegExp.test(it->first)){
					mainQuantity = it->first;
					mout.debug3() << "picked main quantity: " << mainQuantity << mout.endl;
					break;
				}
			}

			mout.debug2() << "main quantity: " << mainQuantity << mout.endl;


			const Data<PolarSrc> & srcMainData = product.getData(mainQuantity); // intervals//product.getData("HGHT"); // intervals
			if (srcMainData.data.isEmpty()){
				ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
				mout.warn() << "zero dimension data (for " << mainQuantity << "), giving up." << mout.endl;
				return;
			}

			drain::Output output(filename);
			//std::string outFileName = ctx.outputPrefix + value;
			//std::ofstream ofstr(outFileName.c_str(), std::ios::out);
			std::ofstream & ofstr = output;

			/// Step 1: create header
			//  ofstr << "## " << (*ctx.currentHi5)["where"].data.attributes << '\n';
			//  ofstr << "## " << product.tree["where"].data.attributes << '\n';
			ofstr << "# " << mainQuantity << '\t';
			for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
				const std::string & quantity = it->first;
				const Data<PolarSrc> & srcData = it->second;
				if ((srcData.data.getWidth()!=1) && (srcData.data.getHeight()!=1))
					mout.warn() << "Skipping" << mout.endl;
				//mout.warn() << "Skipping non 1-dimensional data, un supported for .mat format. Geometry=" << srcData.data.getGeometry() << " ." << mout.endl;
				else if (quantity != mainQuantity){
					ofstr << it->first << '\t'; // TO
					if (USE_COUNT){
						if (srcData.hasQuality("COUNT"))
							ofstr << "COUNT\t"; // it->first <<
					}
				}
			}
			ofstr << '\n';


			const size_t n = std::max(srcMainData.data.getWidth(), srcMainData.data.getHeight());
			if (n == 0){
				mout.warn() << "zero dimension data, giving up." << mout.endl;
				return;
			}

			/// Step 2: Main loop
			//  Notice: here turned downside up, because the images (~profile arrays) are upside down.
			for (int i = n-1; i >= 0; --i) {
				/// Data rows. (This loop is somewhat slow due to [*it] .)
				/// HGHT
				ofstr << srcMainData.odim.scaleForward(srcMainData.data.get<double>(i)) << '\t';  ///
				/// others
				for (DataSet<PolarSrc>::const_iterator it = product.begin(); it != product.end(); ++it){
					if (it->first != mainQuantity){
						const Data<PolarSrc> & srcData = it->second;
						if ((srcData.data.getWidth()==1) || (srcData.data.getHeight()==1)){

							ofstr << srcData.odim.scaleForward(srcData.data.get<double>(i)) << '\t';  ///
							//const std::string countStr(it->first+"_COUNT");
							if (USE_COUNT){
								const PlainData<PolarSrc> & count = srcData.getQualityData("COUNT");
								if (!count.data.isEmpty()){
									ofstr <<  count.odim.scaleForward(count.data.get<unsigned int>(i)) << '\t'; // TO
								}
							}
						}
					}
				}
				ofstr << '\n';
				//std::cerr << "i=" << i << std::endl;
			}

			//ofstr.close();

		}
		else if (sampleFileExtension.test(value)){

			mout.info() << "Sample file (.dat)" << mout.endl;

			//std::string outFileName = ct.outputxPrefix + value;
			//std::ofstream ofstr(outFileName.c_str(), std::ios::out);

			drain::Output ofstr(filename);

			DataSelector selector;
			selector.pathMatcher.setElems(ODIMPathElem::DATASET);
			selector.setParameters(ctx.select);
			//selector.convertRegExpToRanges();
			selector.count = 1;
			//selector.data.second = 0;
			mout.special() << "selector: " << selector << mout.endl;

			ODIMPath path;
			selector.getPath3(src, path);

			mout.info() << "Sampling path: " << path << mout.endl;

			const Hi5Tree & srcDataSet = src(path);

			/// Sampling parameters have been set by --sample (CmdSample)
			const Sampler & sampler = ctx.imageSampler.getSampler();

			if (ctx.currentHi5 == ctx.currentPolarHi5){

				mout.debug() << "sampling polar data" << mout.endl;
				const DataSet<PolarSrc> dataset(srcDataSet, drain::RegExp(selector.quantity));
				mout.info() << "data: " << dataset << mout.endl;

				sampleData<PolarDataPicker>(dataset, sampler, ctx.formatStr, ofstr);

			}
			else {

				mout.debug() << "sampling Cartesian data: " << mout.endl;
				const DataSet<CartesianSrc> dataset(srcDataSet, drain::RegExp(selector.quantity));
				mout.info() << "data: " << dataset << mout.endl;
				/*
				for (DataSet<CartesianSrc>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
					mout.warn() << "data:" << it->first << mout.endl;
				}
				*/

				sampleData<CartesianDataPicker>(dataset, sampler, ctx.formatStr, ofstr);

			}

			//ofstr.close();

		}
		else if (dotFileExtension.test(value)) {

			mout.info() << "Dot/Graphviz file (.dot)" << mout.endl;

			DataSelector selector;
			if (selector.consumeParameters(ctx.select)){
				mout.warn() << "no --select support for this command, use --delete and --keep instead" << mout.endl;
			}

			//Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

			drain::Output output(filename);
			DataOutput::writeToDot(output, src, ODIMPathElem::ALL_GROUPS);// selector.groups);


		}
		else {


			mout.info() << "File format: text" << mout.endl;

			drain::Output output(filename);

			if (ctx.formatStr.empty()){

				if (textFileExtension.test(filename) || (value == "-")){
					mout.info() << "Dumping HDF5 structure" << mout.endl;
				}
				else {
					mout.error() << "Text formatting --format unset, and unknown file format: " << value << mout.endl;
					return;
				}

				ODIMPathList paths;

				if (!ctx.select.empty()){
					DataSelector selector;
					selector.consumeParameters(ctx.select);
					// mout.warn() << selector << mout.endl;
					selector.getPaths(src, paths);
				}
				else {
					ctx.getHi5(RackContext::CURRENT).getPaths(paths);
					//ctx.currentHi5->getPaths(paths); // ALL
				}

				hi5::Hi5Base::writeText(src, paths, output);
			}
			else {
				mout.debug() << "formatting text output" << mout.endl;
				drain::StringMapper statusFormatter(RackContext::variableMapper);
				statusFormatter.parse(ctx.formatStr, true);
				statusFormatter.toStream(output, ctx.getStatusMap());
				// ctx.formatStr.clear(); // ?
			}

			/*
					if (value == "-")
						hi5::Hi5Base::writeText(src, paths, std::cout);
					else {
						//std::string outFileName = ctx.outputPrefix + value;
						drain::Output output(filename);
						hi5::Hi5Base::writeText(src, paths, output);
						//std::ofstream ofstr(filename.c_str(), std::ios::out);
						//ofstr.close();
					}
			 */
		}

	};

protected:

	/**
	 *  \tparam P - Picker class (PolarDataPicker or CartesianDataPicker)
	 */
	template <class P>
	//void sampleData(const typename P::dataset_t & dataset, const Sampler & sampler, const std::string & format, std::ofstream &ofstr) const {
	void sampleData(const typename P::dataset_t & dataset, const Sampler & sampler, const std::string & format, std::ostream &ostr) const {

		RackContext & ctx  = this->template getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

		//mout.warn() << dataset.getFirstData() << mout.endl;

		P picker(sampler.variableMap, dataset.getFirstData().odim);

		//mout.note() << picker << mout.endl;

		typename P::map_t dataMap;

		for (typename DataSet<typename P::src_t>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
			dataMap.insert(typename P::map_t::value_type(it->first, it->second));
		}

		const typename P::data_t & q = dataset.getQualityData();
		if (!q.data.isEmpty()){
			mout.note() << "using quality data, quantity=" << q.odim.quantity << mout.endl;
			if (q.odim.quantity.empty()){
				mout.warn() << " empty data, properties: \n " <<  q.data.properties  << mout.endl;
			}
			dataMap.insert(typename P::map_t::value_type(q.odim.quantity, q));
		}
		else {
			mout.info() << "no quality data" << mout.endl;
		}

		sampler.sample(dataMap, picker, format, ostr);

	}

};
// static drain::CommandEntry<CmdOutputFile> cmdOutputFile("outputFile",'o');





class CmdOutputRawImages : public drain::SimpleCommand<std::string> {

public:

	CmdOutputRawImages() : drain::SimpleCommand<std::string>(__FUNCTION__, "Output datasets to png files named filename[NN].ext.",
			"filename", "", "string") {
	};


	void exec() const {

		RackContext & ctx  = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		//mout.note() << "Writing multiple image files" << mout.endl;

		//DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); //("/data$");
		DataSelector imageSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); // ImageS elector imageS elector;
		imageSelector.consumeParameters(ctx.select);
		imageSelector.ensureDataGroup();
		mout.debug() << imageSelector << mout.endl;


		/// Split filename to dir+basename+extension.
		drain::FilePath fp(ctx.outputPrefix + value);
		mout.note() << "outputPrefix='" << ctx.outputPrefix << "'" << mout;
		mout.note() << "Writing multiple image files: " << fp.dir << ' ' << fp.basename << "???_*." << fp.extension << mout.endl;

		std::string filenameOut;
		int i=0; // Overall index (prefix)

		Hi5Tree & src = ctx.getHi5(RackContext::CURRENT);

		ODIMPathList paths;
		imageSelector.getPaths(src, paths); //, ODIMPathElem::DATA | ODIMPathElem::QUALITY); // RE2

		//for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); it++) {
		for (const ODIMPath & path: paths) {

			//const ODIMPath & path = (*it); // modified below
			hi5::NodeHi5 & node = src(path)[ODIMPathElem::ARRAY].data;
			drain::image::Image & img = node.dataSet;

			mout.debug() << "testing: " << path << " => /data" <<mout.endl;

			if (img.isEmpty()){
				mout.warn() << "empty data array: " << path << " => /data" <<mout.endl;
				continue;
			}

			DataTools::getAttributes(src, path, img.properties); // may be unneeded

			//if (path.front().isRoot()) // typically is, string started with slash '/'
			//path.pop_back(); // strip /data
			mout.debug3() << "constructing filename for : " << path <<mout.endl;

			std::stringstream sstr;
			sstr << fp.dir << fp.basename;
			sstr.width(3);
			sstr.fill('0');
			sstr << ++i << '_';
			//for (ODIMPath::const_iterator pit=path.begin(); pit!=path.end(); ++pit){
			for (const ODIMPathElem & elem: path){
				if (elem.isRoot())
					continue;
				sstr << elem.getCharCode();
				if (elem.isIndexed()){
					sstr << elem.getIndex();
				}
				sstr << '-';
			}
			sstr << img.properties["what:quantity"];
			sstr << '.' << fp.extension;
			filenameOut = sstr.str();

			mout.info() << "Writing image file: " << filenameOut << '\t' << path << mout.endl;
			//mout.debug() << "  data :" << *it << mout.endl;

			drain::image::File::write(img, filenameOut);
			//getResources().currentImage = & img; NEW 2017

		}
		// options[key] = value;  // save last successful filename, see --writeText


	};

};



// BeanCommand<drain::image::ImageSampler>
// BeanerCommand<drain::image::ImageSampler>
//class CmdSample : public drain::SimpleCommand<> {
//class CmdImageSampler : public drain::BeanCommand<drain::image::ImageSampler> {
class CmdImageSampler : public drain::BeanerCommand<drain::image::ImageSampler> {

public:

	// Main
	virtual
	const bean_t & getBean() const {
		RackContext & ctx  = getContext<RackContext>();
		return ctx.imageSampler;
		//return getResources().sampler;
	};


	// Main
	virtual
	bean_t & getBean(){
		RackContext & ctx  = getContext<RackContext>();
		return ctx.imageSampler;
		//return getResources().sampler;
	};

};



FileModule::FileModule(drain::CommandBank & bank) : module_t(bank) { // :(){ // : drain::CommandSection("general") {

	//drain::CommandBank & cmdBank = drain::getCommandBank();
	const drain::Flagger::value_t TRIGGER = drain::Static::get<drain::TriggerSection>().index;

	//drain::CommandInstaller<> installer(drain::getCommandBank());

	install<CmdInputFile>('i').addSection(TRIGGER);
	install<CmdOutputFile>('o'); //  cmdOutputFile('o');

	install<CmdInputPrefix>(); //  cmdInputPrefix;
	install<CmdOutputPrefix>(); //  cmdOutputPrefix;
	install<CmdOutputRawImages>('O'); //  cmdOutputRawImages('O');
	install<CmdOutputConf>(); // cmdOutputConf;
	install<CmdGeoTiffTile>(); //  geoTiffTile;
	install<CmdImageSampler>("sample"); //cmdSample("sample");
	install<CmdHistogram>(); // hist;

	//installer<CmdInputValidatorFile> cmdInputValidatorFile;

}


} // namespace rack
