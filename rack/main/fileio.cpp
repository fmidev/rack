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

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
#include <regex.h>
#include <stddef.h>

#include <util/Log.h>
#include <util/FilePath.h>
#include <util/Output.h>
#include <util/StringMapper.h>
#include <util/Tree.h>
#include <util/Variable.h>

#include <prog/Command.h>
#include <prog/CommandAdapter.h>
#include <prog/CommandPack.h>

#include <data/Data.h>
#include <data/DataOutput.h>
#include <data/DataSelector.h>
#include <data/DataTools.h>
#include <data/ODIMPath.h>
#include <data/PolarODIM.h>
#include <hi5/Hi5.h>
//#include <hi5/Hi5Read.h>
#include <hi5/Hi5Write.h>
#include <image/File.h>
#include <image/Image.h>
#include <image/Sampler.h>
#include <imageops/ImageModifierPack.h>
#include <main/fileio.h>
#include <main/fileio-read.h>
#include <main/images.h> // for calling --image on the fly
#include <main/resources.h>
//#include <product/HistogramOp.h>
#include <product/ProductOp.h>
#include <radar/FileGeoTIFF.h>
#include <radar/RadarDataPicker.h>






namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.

const drain::RegExp h5FileExtension("^((.*\\.(h5|hdf5|hdf))|((.*/)?[\\w]+))$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising GeoTIFF files.
const drain::RegExp tiffFileExtension(".*\\.(tif|tiff)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
const drain::RegExp pngFileExtension(".*\\.(png)$", REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
const drain::RegExp pnmFileExtension(".*\\.(p[nbgp]m)$", REG_EXTENDED | REG_ICASE); //[nbgp]

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

struct HistEntry : BeanLike {

	HistEntry() : BeanLike(__FUNCTION__), index(0), count(0){
		parameters.reference("index", index);
		parameters.reference("min", binRange.min);
		parameters.reference("max", binRange.max);
		parameters.reference("count", count);
		parameters.reference("label", label);
	};

	drain::Histogram::vect_t::size_type index;
	Range<double> binRange;
	drain::Histogram::count_t count;
	std::string label;

};
static HistEntry histEntryHelper;

/// TODO: generalize to array outfile
class CmdHistogram : public BasicCommand {

public:

	int count;

	Range<double> range;

	std::string store;
	std::string filename;

	//	CmdHistogram() : SimpleCommand<int>(__FUNCTION__, "Histogram","slots", 256, "") {
	CmdHistogram() : BasicCommand(__FUNCTION__, std::string("Histogram. Optionally --format using keys ") + histEntryHelper.getParameters().getKeys()) {
		parameters.reference("count", count = 256);
		parameters.reference("range", range.vect);
		//parameters.reference("max", maxValue = +std::numeric_limits<double>::max());
		parameters.reference("filename", filename="", "<filename>.txt|-");
		parameters.reference("store", store="histogram", "<attribute_key>");
	};

	// virtual	inline const std::string & getDescription() const { return description; };


	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__); // getResources().mout;

		RackResources & resources = getResources();

		Hi5Tree & currentHi5 = *resources.currentHi5;

		DataSelector selector(ODIMPathElemMatcher::DATA);
		//selector.pathMatcher.clear();
		//selector.pathMatcher << ODIMPathElemMatcher(ODIMPathElemMatcher::DATA);
		selector.setParameters(resources.select);

		ODIMPath path;
		selector.getPath3(currentHi5, path);
		resources.select.clear();



		PlainData<BasicDst> dstData(currentHi5(path));

		mout.note() << "path: " << path << " [" << dstData.odim.quantity << ']' << mout.endl;

		// NO resources.setCurrentImage(selector);
		// drain::image::Image & img = *resources.currentImage;
		mout.warn() << "img " << dstData.data << mout.endl;

		drain::Histogram histogram(256);
		histogram.setScale(dstData.data.getScaling());
		histogram.compute(dstData.data, dstData.data.getType());

		if (!filename.empty()){

			drain::Output out((filename == "-") ? filename : getResources().outputPrefix + filename);

			std::ostream & ostr = out;

			drain::StringMapper mapper;
			if (!cmdFormat.value.empty()){
				//std::string format(cmdFormat.value);
				//format = drain::StringTools::replace(format, "\\t", "\t");
				//format = drain::StringTools::replace(format, "\\n", "\n");
				mapper.parse(cmdFormat.value, true);
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





class CmdGeoTiffTile : public BasicCommand {

public:

	CmdGeoTiffTile() : BasicCommand(__FUNCTION__, "GeoTIFF tile size") {
		parameters.reference("tilewidth", FileGeoTIFF::tileWidth=256);
		parameters.reference("tileheight", FileGeoTIFF::tileHeight=0);
	};


};



// Cf. InputPrefix
class CmdOutputPrefix : public BasicCommand {

public:

	CmdOutputPrefix() : BasicCommand(__FUNCTION__, "Path prefix for output files."){
		parameters.reference("path", getResources().outputPrefix = "");
	};
};
extern CommandEntry<CmdOutputPrefix> cmdOutputPrefix;


class CmdOutputFile : public SimpleCommand<std::string> {

public:

	CmdOutputFile() : SimpleCommand<>(__FUNCTION__, "Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.",
			"filename", "", "<filename>.[h5|hdf5|png|pgm|txt|mat|dot]|-") {
	};


	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		RackResources & resources = getResources();

		if (value.empty()){
			mout.error() << "File name missing. (Use '-' for stdout.)" << mout.endl;
			return;
		}

		if (!resources.inputOk){
			mout.warn() << "input failed, skipping" << mout.endl;
			return;
		}

		if (!resources.dataOk){
			mout.warn() << "data error, skipping" << mout.endl;
			return;
		}

		if (value != "-")
			mout.note() << "writing: '" << value << "'" << mout.endl;

		// TODO: generalize select
		// TODO: generalize image pick (current or str) for png/tif

		const bool IMAGE_PNG = pngFileExtension.test(value);
		const bool IMAGE_PNM = drain::image::FilePnm::fileNameRegExp.test(value);
		const bool IMAGE_TIF = tiffFileExtension.test(value);

		if (h5FileExtension.test(value)){
			mout.info() << "File format: HDF5" << mout.endl;

			getResources().currentHi5->data.attributes["Conventions"] = "ODIM_H5/V2_2";


			/// getResources().currentHi5->data.attributes["version"] = "H5rad 2.2"; // is in ODIM /what/version
			//const char c = hi5::Writer::tempPathSuffix;
			//if (mout.isDebug(10))
			//	hi5::Writer::tempPathSuffix = 0; // save also paths with '~' suffix.
			hi5::Writer::writeFile(getResources().outputPrefix + value, *getResources().currentHi5);
			//hi5::Writer::tempPathSuffix = c;
		}
		else if (IMAGE_PNG || IMAGE_PNM || IMAGE_TIF) {


			// This is the simple version. See image commands (--iXxxxx)
			try {

				mout.info() << "File format: image" << mout.endl;


				const bool CONVERT = !resources.targetEncoding.empty();

				// If there is already a product generated with --image, use that.
				// if (resources.select.empty() && ((resources.currentImage == &resources.grayImage) || (resources.currentImage == &resources.colorImage))){
				//   && resources.targetEncoding.empty() && resources.targetEncoding.empty()
				if (resources.select.empty() && (resources.currentImage != NULL) && !CONVERT){
					mout.debug() << "Writing current image, no conversion " << mout.endl;
				}
				else if (CONVERT){
					mout.note() << "encoding requested, calling "<< cmdImage.getName() << " implicitly" << mout.endl;
					cmdImage.exec();
				}
				else { // pointer (resources.currentImage) needs update
					DataSelector imageSelector;  //
					imageSelector.pathMatcher.setElems(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
					//imageSelector.pathMatcher << ODIMPathElemMatcher(ODIMPathElem::DATASET,1) << ODIMPathElemMatcher(ODIMPathElem::DATA,1);
					imageSelector.setParameters(resources.select);
					resources.select.clear();
					mout.debug() << imageSelector << mout.endl;
					if (resources.setCurrentImage(imageSelector)){
						// OK
						if (!(resources.currentImage->getScaling().isPhysical() || drain::Type::call<drain::typeIsSmallInt>(resources.currentImage->getType()))){
							mout.warn() << "no physical scaling, consider --encoding C or --encoding S" << mout.endl;
						}
					}
					else {
						mout.warn() << "data not found or empty data with selector: " << imageSelector << mout.endl;
						return;
					}

				}

				if (!resources.currentImage->isEmpty()){

					if (IMAGE_PNG || IMAGE_PNM){
						mout.debug() << "PNG or PGM format" << mout.endl;
						drain::image::File::write(*resources.currentImage, resources.outputPrefix + value);
					}
					else if (IMAGE_TIF) {
						// see FileGeoTiff::tileWidth
						FileGeoTIFF::write(resources.outputPrefix + value, *getResources().currentImage); //, geoTIFF.width, geoTIFF.height);
					}
					else {
						mout.error() << "unknown file name extension" << mout.endl;
					}
				}
				else {
					mout.warn() << "empty data, skipped" << mout.endl;
				}

			} catch (std::exception &e) {
				mout.warn() << "failed in writing " << value << ": " << e.what() << mout.endl;
			} catch (...) {
				mout.warn() << "failed in writing " << value << mout.endl;
			}
		}
		else if (textFileExtension.test(value) || (value == "-")){

			mout.info() << "File format: TXT" << mout.endl;
			ODIMPathList paths;

			if (!resources.select.empty()){

				DataSelector selector;
				//selector.deriveParameters(resources.select, true);
				selector.setParameters(resources.select);
				mout.debug() << selector << mout.endl;
				selector.getPaths3(*resources.currentHi5, paths);
				/* OLD
				selector.groups = ODIMPathElem::ALL_GROUPS; //ODIMPathElem::DATA_GROUPS;
				selector.deriveParameters(resources.select, false); //, ODIMPathElem::ALL_GROUPS);
				// ODIMPathElem::group_t groups = selector.quantity.empty() ? ODIMPathElem::ALL_GROUPS : ODIMPathElem::DATA_GROUPS;
				// selector.getPaths(*getResources().currentHi5, paths, groups);
				// selector.groups = ODIMPathElem::ALL_GROUPS;
				selector.getPaths(*getResources().currentHi5, paths);
				*/
				resources.select.clear();
			}
			else {
				resources.currentHi5->getPaths(paths);
			}

			if (value == "-")
				hi5::Hi5Base::writeText(*resources.currentHi5, paths, std::cout);
			else {
				std::string outFileName = resources.outputPrefix + value;
				std::ofstream ofstr(outFileName.c_str(), std::ios::out);
				hi5::Hi5Base::writeText(*resources.currentHi5, paths, ofstr);
				ofstr.close();
			}
		}
		else if (arrayFileExtension.test(value)){

			/// Currently designed only for vertical profiles produced by VerticalProfileOp (\c --pVerticalProfile )
			/// TODO: modify DataSet such that the quantities appear in desired order.

			mout.info() << "File format: .mat (text dump of data)" << mout.endl;

			DataSelector selector;
			selector.setParameters(resources.select);
			resources.select.clear();
			//selector.path = "/data$";
			mout.debug(1) << selector << mout.endl;
			const drain::RegExp quantityRegExp(selector.quantity);
			const bool USE_COUNT = quantityRegExp.test("COUNT");
			// mout.debug(2) << "use count" << static_cast<int>(USE_COUNT) << mout.endl;

			const DataSet<PolarSrc> product((*resources.currentHi5)["dataset1"], drain::RegExp(selector.quantity));
			//const DataSet<> product((*resources.currentHi5), selector);

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
					mout.debug(2) << "picked main quantity: " << mainQuantity << mout.endl;
					break;
				}
			}

			mout.debug(1) << "main quantity: " << mainQuantity << mout.endl;


			const Data<PolarSrc> & srcMainData = product.getData(mainQuantity); // intervals//product.getData("HGHT"); // intervals
			if (srcMainData.data.isEmpty()){
				mout.warn() << "zero dimension data (for " << mainQuantity << "), giving up." << mout.endl;
				return;
			}

			std::string outFileName = resources.outputPrefix + value;
			std::ofstream ofstr(outFileName.c_str(), std::ios::out);

			/// Step 1: create header
			//  ofstr << "## " << (*resources.currentHi5)["where"].data.attributes << '\n';
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

			ofstr.close();

		}
		else if (sampleFileExtension.test(value)){

			mout.info() << "Sample file (.dat)" << mout.endl;

			std::string outFileName = resources.outputPrefix + value;
			std::ofstream ofstr(outFileName.c_str(), std::ios::out);

			DataSelector selector;
			selector.pathMatcher.setElems(ODIMPathElem::DATASET);
			selector.setParameters(resources.select);
			//selector.convertRegExpToRanges();
			selector.count = 1;
			//selector.data.max = 0;
			mout.debug() << "selector: " << selector << mout.endl;

			ODIMPath path;
			selector.getPath3(*resources.currentHi5, path);

			mout.info() << "Sampling path: " << path << mout.endl;

			const Hi5Tree & src = (*resources.currentHi5)(path);

			const Sampler & sampler = resources.sampler.getSampler();

			if (resources.currentHi5 == resources.currentPolarHi5){

				mout.debug() << "sampling polar data" << mout.endl;
				const DataSet<PolarSrc> dataset(src, drain::RegExp(selector.quantity));
				sampleData<PolarDataPicker>(dataset, sampler, cmdFormat.value, ofstr);

			}
			else {

				mout.debug() << "sampling Cartesian data: " << mout.endl;
				const DataSet<CartesianSrc> dataset(src, drain::RegExp(selector.quantity));
				mout.info() << "data:" << dataset << mout.endl;
				/*
				for (DataSet<CartesianSrc>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
					mout.warn() << "data:" << it->first << mout.endl;
				}
				*/

				sampleData<CartesianDataPicker>(dataset, sampler, cmdFormat.value, ofstr);

			}

			ofstr.close();

		}
		else if (dotFileExtension.test(value)) {

			mout.info() << "Dot/Graphviz file (.dot)" << mout.endl;

			DataSelector selector;
			//selector.groups.value = ODIMPathElem::ALL_GROUPS;
			selector.setParameters(resources.select);
			//selector.convertRegExpToRanges();
			if (!resources.select.empty()){
				mout.warn() << "lacking --select support, use --delete and --keep instead" << mout.endl;
			}
			resources.select.clear();

			Output out(resources.outputPrefix + value);
			DataOutput::writeToDot(out, *resources.currentHi5, ODIMPathElem::ALL_GROUPS);// selector.groups);


		}
		else {
			// or warn?
			mout.error() << "Unrecognized file type, writing png image file " << value  << mout.endl;
		}

	};

protected:

	/**
	 *  \tparam P - Picker class (PolarDataPicker or CartesianDataPicker)
	 */
	template <class P>
	void sampleData(const typename P::dataset_t & dataset, const Sampler & sampler, const std::string & format, std::ofstream &ofstr) const {

		drain::Logger mout(__FUNCTION__, __FILE__);

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
			mout.note() << "no quality data" << mout.endl;
		}

		sampler.sample(dataMap, picker, format, ofstr);

	}

};
// static CommandEntry<CmdOutputFile> cmdOutputFile("outputFile",'o');





class CmdOutputRawImages : public SimpleCommand<std::string> {

public:

	CmdOutputRawImages() : SimpleCommand<std::string>(__FUNCTION__, "Output datasets to png files named filename[NN].ext.",
			"filename", "", "string") {
	};


	void exec() const {

		drain::Logger mout(__FUNCTION__, __FILE__);
		//mout.note() << "Writing multiple image files" << mout.endl;

		RackResources & resources = getResources();

		DataSelector iSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY); //("/data$");
		iSelector.setParameters(resources.select);
		resources.select.clear();
		mout.debug() << iSelector << mout.endl;

		ODIMPathList paths;
		iSelector.getPaths3(*getResources().currentHi5, paths); //, ODIMPathElem::DATA | ODIMPathElem::QUALITY); // RE2

		/// Split filename to dir+basename+extension.
		drain::FilePath fp(resources.outputPrefix + value);

		mout.note() << "Writing multiple image files: " << fp.dir << ' ' << fp.basename << "???_*." << fp.extension << mout.endl;

		std::string filenameOut;
		int i=0; // Overall index (prefix)

		for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); it++) {

			const ODIMPath & path = (*it); // modified below
			hi5::NodeHi5 & node = (*getResources().currentHi5)(path)["data"].data;
			drain::image::Image & img = node.dataSet;

			mout.debug() << "testing: " << path << " => /data" <<mout.endl;

			if (img.isEmpty()){
				mout.warn() << "empty data array: " << path << " => /data" <<mout.endl;
				continue;
			}

			DataTools::getAttributes((*getResources().currentHi5), path, img.properties); // may be unneeded

			//if (path.front().isRoot()) // typically is, string started with slash '/'
			//path.pop_back(); // strip /data
			mout.debug(2) << "constructing filename for : " << path <<mout.endl;

			std::stringstream sstr;
			sstr << fp.dir << fp.basename;
			sstr.width(3);
			sstr.fill('0');
			sstr << ++i << '_';
			for (ODIMPath::const_iterator pit=path.begin(); pit!=path.end(); ++pit){
				sstr << pit->getCharCode();
				if (pit->isIndexed()){
					sstr << pit->getIndex();
				}
				sstr << '-';
			}
			sstr << img.properties["what:quantity"];
			sstr << '.' << fp.extension;
			filenameOut = sstr.str();

			mout.info() << "Writing image file: " << filenameOut << '\t' << *it << mout.endl;
			//mout.debug() << "  data :" << *it << mout.endl;

			drain::image::File::write(img, filenameOut);
			//getResources().currentImage = & img; NEW 2017

		}
		// options[key] = value;  // save last successful filename, see --writeText


	};

};
// static CommandEntry<CmdOutputRawImages> cmdOutputRawImages("outputRawImages",'O');

//class CmdSample : public drain::CmdSampler {
class CmdSample : public drain::Command {
public: //re

	//CmdSample() : CmdSampler(getResources().sampler, __FUNCTION__, "Sampling configuration. See --format and --outFile '*.dat'."){}; // __FUNCTION__) {};

	inline
	const std::string & getName() const {
		return getResources().sampler.getName();
	};

	inline
	const std::string & getDescription() const { return getResources().sampler.getDescription(); };

	virtual
	inline
	const drain::ReferenceMap & getParameters() const { return getResources().sampler.getParameters(); };

	void run(const std::string & parameters){
		getResources().sampler.setParameters(parameters);
		// mod.setParameters(parameters);
		// exec();
	}


};



FileModule::FileModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix) {

	static RackLetAdapter<CmdHistogram> hist;
	static RackLetAdapter<CmdInputFile> cmdInputFile('i');
	static RackLetAdapter<CmdSample> cmdSample("sample");
	static RackLetAdapter<CmdGeoTiffTile> geoTiffTile;
	static RackLetAdapter<CmdInputPrefix> cmdInputPrefix;
	static RackLetAdapter<CmdOutputPrefix> cmdOutputPrefix;
	static RackLetAdapter<CmdOutputFile> cmdOutputFile('o');
	static RackLetAdapter<CmdOutputRawImages> cmdOutputRawImages('O');

}


} // namespace rack
