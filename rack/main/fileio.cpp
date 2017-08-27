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
#include <drain/util/Histogram.h>
#include <drain/util/RegExp.h>

#include <drain/prog/Command.h>

#include <drain/image/Image.h>
#include <drain/image/Sampler.h>

#include <drain/prog/CommandRegistry.h>
#include <drain/prog/Commands-ImageTools.h>

#include "andre/QualityCombinerOp.h"
#include "hi5/Hi5.h"
#include "hi5/Hi5Write.h"
#include "hi5/Hi5Read.h"
#include "data/ODIM.h"
#include "product/DataConversionOp.h"
#include "radar/FileGeoTIFF.h"
#include "radar/RadarDataPicker.h"


#include "commands.h"

#include "fileio.h"
#include "fileio-read.h"






namespace rack {

/// Syntax for recognising hdf5 files.
//  Edited 2017/07 such that also files without extension are considered h5 files.

const drain::RegExp h5FileExtension("^((.*\\.(h5|hdf5|hdf))|((.*/)?[\\w]+))$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising GeoTIFF files.
const drain::RegExp tiffFileExtension(".*\\.(tif|tiff)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising image files (currently, png supported).
const drain::RegExp pngFileExtension(".*\\.(png)$", REG_EXTENDED | REG_ICASE);

/// Syntax for recognising text files.
const drain::RegExp textFileExtension(".*\\.(txt)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for recognising numeric array files (in plain text format anyway).
const drain::RegExp arrayFileExtension(".*\\.(mat)$",  REG_EXTENDED | REG_ICASE);

/// Syntax for sparsely resampled data.
const drain::RegExp sampleFileExtension(".*\\.(dat)$",  REG_EXTENDED | REG_ICASE);


static DataSelector imageSelector(".*/data/?$","");   // Only for images. Not directly accessible.


class CmdInputSelect : public BasicCommand {

public:

	CmdInputSelect() : BasicCommand(__FUNCTION__, "Read ATTRIBUTES (1), DATA(2) or both (3)."){
		//, "value", hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS, "flag"){};
		parameters.reference("value", getResources().inputSelect = hi5::Reader::ATTRIBUTES|hi5::Reader::DATASETS, "flag");
	}

};
//static CommandEntry<CmdInputSelect> cmdInputSelect("inputSelect");


class CmdHistogram : public BasicCommand {

public:

	int count;
	mutable
	double minValue;
	mutable
	double maxValue;

	//	CmdHistogram() : SimpleCommand<int>(__FUNCTION__, "Histogram","slots", 256, "") {
	CmdHistogram() : BasicCommand(__FUNCTION__, "Histogram") {
		parameters.reference("count", count = 256);
		parameters.reference("min", minValue = std::numeric_limits<double>::min());
		parameters.reference("max", maxValue = std::numeric_limits<double>::max());
	};

	void exec() const {
		drain::MonitorSource mout(name, __FUNCTION__); // getResources().mout;

		RackResources & resources = getResources();

		std::list<std::string> paths;
		DataSelector selector;
		selector.setParameters(resources.select);

		DataSelector::getPaths(*resources.currentHi5, selector, paths);
		if (paths.empty()){
			mout.warn() << "no data " << mout.endl;
			return;
		}

		DataSetSrc<PolarSrc> srcDataSet((*resources.currentHi5)[*paths.begin()]);
		const Data<PolarSrc> &d = srcDataSet.getFirstData();
		drain::Histogram hist(count);

		if ((minValue == std::numeric_limits<double>::min()) || (maxValue == std::numeric_limits<double>::max())){
			const EncodingODIM & odim = getQuantityMap().get(d.odim.quantity).get('C');
			hist.setScale(odim.scaleForward(0), odim.scaleForward(0xff));
		}
		else {
			hist.setScale(minValue, maxValue);
		}

		mout.note() << "quantity: " << d.odim.quantity << mout.endl;

		mout.note() << hist << mout.endl;

		double f;
		int nodataCount = 0;
		int undetectCount = 0;
		for (drain::Image::const_iterator it = d.data.begin(); it != d.data.end(); ++it){
			f = *it;
			if (f == d.odim.undetect){
				++undetectCount;
			}
			else if (f == d.odim.nodata){
				++nodataCount;
			}
			else {
				f = d.odim.scaleForward(f);
				if (hist.withinLimits(f))
					hist.increment(f);
			}
		}
		mout.note() << hist << mout.endl;

		minValue = std::numeric_limits<double>::min();
		maxValue = std::numeric_limits<double>::max();

	}

};





class CmdGeoTiffTile : public BasicCommand {

public:

	//int width;
	//int height;
	CmdGeoTiffTile() : BasicCommand(__FUNCTION__, "GeoTIFF tile size") {
		parameters.reference("tilewidth", FileGeoTIFF::tileWidth=256);
		parameters.reference("tileheight", FileGeoTIFF::tileHeight=0);
	};


};
// static CommandEntry<CmdTiffTile> geoTIFF("geoTIFF");


// Cf. InputPrefix
class CmdOutputPrefix : public BasicCommand {

public:

	CmdOutputPrefix() : BasicCommand(__FUNCTION__, "Path prefix for output files."){
		parameters.reference("path", getResources().outputPrefix = "");
	};
};
extern CommandEntry< CmdOutputPrefix > cmdOutputPrefix;
//static CommandEntry<CmdOutputPrefix> cmdOutputPrefix( "outputPrefix", 0);


class CmdOutputFile : public SimpleCommand<std::string> {

public:

	CmdOutputFile() : SimpleCommand<>(__FUNCTION__, "Output current data to .hdf5, .png, .txt, .mat file. See also: --image, --outputRawImages.",
			"filename", "", "<filename>.[h5|png|txt|mat]|-") {
	};


	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__);

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
			mout.note() << "writing: " << value << mout.endl;

		// TODO: generalize select
		// TODO: generalize image pick (current or other) for png/tif

		if (h5FileExtension.test(value)){
			mout.info() << "File format: HDF5" << mout.endl;
			getResources().currentHi5->data.attributes["Conventions"] = "ODIM_H5/V2_2";
			/// getResources().currentHi5->data.attributes["version"] = "H5rad 2.2"; // is in ODIM /what/version
			const char c = hi5::Writer::tempPathSuffix;
			if (mout.isDebug(10))
				hi5::Writer::tempPathSuffix = 0; // save also paths with '~' suffix.
			hi5::Writer::writeFile(getResources().outputPrefix + value, *getResources().currentHi5);
			hi5::Writer::tempPathSuffix = c;
		}
		else if (pngFileExtension.test(value) || tiffFileExtension.test(value)) {


			// This is the simple version. See image commands (--iXxxxx)
			try {

				mout.info() << "File format: image" << mout.endl;

				// If there is already a product generated with --image, use that.
				//if (resources.select.empty() && ((resources.currentImage == &resources.grayImage) || (resources.currentImage == &resources.colorImage))){
				// && resources.targetEncoding.empty()
				if (resources.select.empty()  && (resources.currentImage != NULL)){
					mout.debug() << "Writing current image, no conversion " << mout.endl;
				}
				else {

					imageSelector.setParameters(resources.select);
					resources.select.clear();
					mout.debug(2) << imageSelector << mout.endl;

					std::list<std::string> l;
					DataSelector::getPaths(*resources.currentHi5, imageSelector, l); // todo getFirstData

					if (!l.empty()){

						const std::list<std::string>::const_iterator it = l.begin();
						mout.info() << "selected: " << *it << mout.endl;
						drain::image::Image & img = (*resources.currentHi5)(*it).data.dataSet;
						if (!img.isEmpty()){
							//RackOp::collectAttributes((*getResources().currentHi5),*it, img->properties);
							DataSelector::getAttributes(*resources.currentHi5, *it, img.properties); // may be unneeded
							//drain::image::File::write(img, value);
							resources.currentImage = &img;
						}
						else
							mout.warn() << "empty data in path: " << *it << mout.endl;
					}
					else {
						// if EXIT_ON_DATA_FAIL
						mout.warn() << "skipping, no image data found with selector " << imageSelector << mout.endl;
						return;
					}
				}

				if (!resources.currentImage->isEmpty()){
					if (pngFileExtension.test(value)){
						drain::image::File::write(*resources.currentImage, resources.outputPrefix + value);
					}
					else {
						// see FileGeoTiff::tileWidth
						FileGeoTIFF::write(resources.outputPrefix + value, *getResources().currentImage); //, geoTIFF.width, geoTIFF.height);
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
			std::list<std::string> paths;

			if (!resources.select.empty()){
				DataSelector selector;
				selector.setParameters(resources.select);
				DataSelector::getPaths(*resources.currentHi5, selector, paths);
				//DataSelector::getPaths(*getResources().currentHi5, DataSelector(resources.select), paths);
				resources.select.clear();
				// for (std::list<std::string>::const_iterator it = paths.begin(); it != paths.end(); ++it)
				//	mout.warn() << *it << mout.endl;
			}
			else {
				resources.currentHi5->getKeys(paths);
			}

			if (value == "-")
				hi5::Hi5Base::writeText(*resources.currentHi5, paths, std::cout);
			else {
				//ofstream ofstr((const std::string &)outputPrefix + value.c_str(), ios::out);
				std::string outFileName = resources.outputPrefix + value;
				std::ofstream ofstr(outFileName.c_str(), std::ios::out);
				//getResources().currentHi5->writeText(ofstr);
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

			const DataSetSrc<> product((*resources.currentHi5)["dataset1"], drain::RegExp(selector.quantity));
			//const DataSetSrc<> product((*resources.currentHi5), selector);

			std::string mainQuantity; // = product.getFirstData().odim.quantity;

			const drain::RegExp mainQuantityRegExp("HGHT");
			for (DataSetSrc<>::const_iterator it = product.begin(); it != product.end(); ++it){
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
			for (DataSetSrc<>::const_iterator it = product.begin(); it != product.end(); ++it){
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
				for (DataSetSrc<>::const_iterator it = product.begin(); it != product.end(); ++it){
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

			drain::StringMapper formatter("[a-zA-Z0-9_]+");
			std::string & format = cmdFormat.value;
			format = drain::String::replace(format, "\\n", "\n");
			format = drain::String::replace(format, "\\t", "\t");
			formatter.parse(format);

			DataSelector selector("dataset[0-9]/?$");
			selector.setParameters(resources.select);

			std::string path = "dataset1";
			//std::list<std::string> dataPaths;  // TODO: 3D sampling (3rd dim: elevations / altitudes)
			//DataSelector::getPath(*resources.currentHi5, selector, path);  // TODO (failed)

			const HI5TREE & src = (*resources.currentHi5)[path];

			Sampler & sampler = resources.sampler;

			if (resources.currentHi5 == resources.currentPolarHi5){

				mout.debug() << "sampling polar data" << mout.endl;
				const DataSetSrc<PolarSrc> dataset(src, drain::RegExp(selector.quantity));
				sampleData<PolarDataPicker>(dataset, sampler, format, ofstr);

			}
			else {

				mout.debug() << "sampling Cartesian data" << mout.endl;
				const DataSetSrc<CartesianSrc> dataset(src, drain::RegExp(selector.quantity));
				sampleData<CartesianDataPicker>(dataset, sampler, format, ofstr);

			}

			ofstr.close();

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
	//void sampleData(const std::string & quantity, Sampler & sampler, const std::string & format, ofstream &ofstr) const {
	void sampleData(const typename P::dataset_t & dataset, Sampler & sampler, const std::string & format, std::ofstream &ofstr) const {

		P picker(sampler.variableMap, dataset.getFirstData().odim);

		typename P::map_t dataMap;

		for (typename DataSetSrc<typename P::src_t>::const_iterator it = dataset.begin(); it != dataset.end(); ++it){
			dataMap.insert(typename P::map_t::value_type(it->first, it->second));
		}

		const typename P::data_t & q = dataset.getQualityData();
		if (!q.data.isEmpty()){
			dataMap.insert(typename P::map_t::value_type(q.odim.quantity, q));
		}

		sampler.sample(dataMap, picker, format, ofstr);

	}


};
// static CommandEntry<CmdOutputFile> cmdOutputFile("outputFile",'o');





class CmdOutputRawImages : public SimpleCommand<std::string> {

public:

	CmdOutputRawImages() : SimpleCommand<std::string>(__FUNCTION__, "Output datasets to png files named filename[NN].ext.",
			"filename", "", "std::string") {
	};


	void exec() const {

		drain::MonitorSource mout(name, __FUNCTION__); // = getResources().mout;
		mout.note() << "Writing multiple image files" << mout.endl;

		RackResources & resources = getResources();

		DataSelector iSelector("/data$");
		iSelector.setParameters(resources.select);
		resources.select.clear();

		std::list<std::string> l;
		//getResources().currentHi5->getKeys(l, options["data"]);
		DataSelector::getPaths(*getResources().currentHi5, iSelector, l);

		/// Split filename to basename+extension.
		static const drain::RegExp r("^(.*)(\\.[a-zA-Z0-9]+)$");
		r.execute(value);
		if (r.result.size() != 3){
			mout.error() << " Could not detect image format for " << value << mout.endl;
			return;
		}
		const std::string & basename  = r.result[1];
		const std::string & extension = r.result[2];

		// Overall index (prefix)
		int i=1;
		std::string filenameOut;
		/// Means for extracting initial letters and numbers from path components of type {dataset|data|quality}{0-9}
		// Notice: /dataset1/data2/quality2/what:quantity=CLASS
		//         /dataset1/quality2/what:quantity=CLASS
		drain::RegExp pathMatcher("^/?([dq])[^0-9]+([0-9]+)/([dq])[^0-9]+([0-9]+)(/([dq])[^0-9]+([0-9]+))?[^0-9]*$");
		//drain::RegExp pathMatcher("^.*[^0-9]+([0-9]+)/([dq])[^0-9]+([0-9]+)[^0-9]*$");


		for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); it++) {

			hi5::NodeHi5 & node = (*getResources().currentHi5)(*it).data;
			drain::image::Image & img = node.dataSet;

			mout.debug(2) << "testing: " << *it <<mout.endl;

			if (img.isEmpty())
				continue;

			DataSelector::getAttributes((*getResources().currentHi5), *it, img.properties); // may be unneeded

			std::stringstream sstr;
			sstr << basename;
			sstr.width(3);
			sstr.fill('0');
			sstr << i;
			if (pathMatcher.execute(*it) != REG_NOMATCH){
				//sstr.width(2);
				sstr << '-' << pathMatcher.result[1] << pathMatcher.result[2] << pathMatcher.result[3] << pathMatcher.result[4] << pathMatcher.result[6] << pathMatcher.result[7];
			}
			sstr << '_' << img.properties["what:quantity"];
			sstr << extension;
			filenameOut = sstr.str();

			mout.info() << "Writing image file: " << filenameOut << '\t' << *it << mout.endl;
			mout.debug() << "  data :" << *it << mout.endl;

			drain::image::File::write(img, filenameOut);
			//getResources().currentImage = & img; NEW 2017

			i++;
		}
		// options[key] = value;  // save last successful filename, see --writeText


	};

};
// static CommandEntry<CmdOutputRawImages> cmdOutputRawImages("outputRawImages",'O');

class CmdSample : public CmdSampler {
    public: //re 
	CmdSample() : CmdSampler(getResources().sampler, __FUNCTION__, "Sampling configuration. See --format and --outFile '*.dat'."){}; // __FUNCTION__) {};
};

FileModule::FileModule(const std::string & section, const std::string & prefix) : drain::CommandGroup(section, prefix) {

	static RackLetAdapter<CmdInputSelect> cmdInputSelect;
	static RackLetAdapter<CmdHistogram> hist;
	static RackLetAdapter<CmdInputFile> cmdInputFile('i');
	static RackLetAdapter<CmdSample> cmdSample;
	static RackLetAdapter<CmdGeoTiffTile> geoTiffTile;
	static RackLetAdapter<CmdInputPrefix> cmdInputPrefix;
	static RackLetAdapter<CmdOutputPrefix> cmdOutputPrefix;
	static RackLetAdapter<CmdOutputFile> cmdOutputFile('o');
	static RackLetAdapter<CmdOutputRawImages> cmdOutputRawImages('O');

}


//} // namespace (unnamed)


} // namespace rack

// Rack
