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





#include <drain/image/File.h>

#include "hi5/Hi5.h"
#include "data/QuantityMap.h"
#include "product/DataConversionOp.h"

#include "cartesian.h"
#include "cartesian-motion.h" //



namespace rack {

void CartesianOpticalFlow::getSrcData(ImageTray<const Channel> & srcTray) const {

	Logger mout(__FUNCTION__, getName());

	RackResources & resources = getResources();
	//Hi5Tree & srcH5 = resources.cartesianHi5; //*resources.currentHi5;

	//ODIMPathList paths;
	ODIMPathList paths;
	DataSelector selector; //("/data[0-9]+/?$");
	selector.setParameters(resources.select);
	resources.select.clear();
	// selector.getPaths(resources.cartesianHi5, paths); // RE2
	selector.getPaths(resources.cartesianHi5, paths, ODIMPathElem::DATA);

	unsigned short count = 0;

	// TODO: this could be dataset<i> paths instead of dataset<i>/data<j> paths
	for (ODIMPathList::const_iterator it = paths.begin(); it != paths.end(); ++it){

		//const std::string & path = *it;
		const ODIMPath & path = *it;

		Hi5Tree & srcDataSetH5 = resources.cartesianHi5(path);

		//PlainData<CartesianDst> srcData(srcDataSetH5); // non-const "srcTray"
		Data<CartesianDst> srcData(srcDataSetH5); // non-const "srcTray"


		Quantity & quantity = getQuantityMap().get(srcData.odim.quantity);
		const EncodingODIM & encoding = quantity.get('C'); // check if exists?
		if (!EncodingODIM::haveSimilarEncoding(srcData.odim, encoding)){
			mout.warn() << "Rescaling input data\n";
			mout << "\t from: " << EncodingODIM(srcData.odim) << '\n';
			mout << "\t to:   " << encoding << '\n';
			mout << mout.endl;
			DataConversionOp<CartesianODIM> conversion;
			conversion.odim = encoding;
			conversion.processData(Data<CartesianSrc>(srcDataSetH5), srcData);
		}


		if (!srcData.data.isEmpty()){

			srcData.data.setScaling(srcData.odim.gain, srcData.odim.offset);
			//mout.note() << "Using " << *it << ':' << srcData.data.getGeometry() << mout.endl;
			mout.note() << "Using " << *it << ": " << path << ' ';
			mout <<  srcData.data.getGeometry() << ':' << Type::getTypeChar(srcData.data.getType()) << mout.endl;
			//<< srcData.data << mout.endl;
			mout.debug() << srcData.odim << mout.endl;

			areaGeometry.setArea(srcData.data.getGeometry());

			++count;
			switch (count){
			case 1:
				srcData.odim.getStartTime(t1);
				mout.debug() << t1.str() << mout.endl;
				break;
			case 2:
				srcData.odim.getStartTime(t2);
				mout.debug() << t2.str() << mout.endl;
				mout.debug() << "time difference: " << (t2.getTime() - t1.getTime())/60 << " mins" << mout.endl;
				break;
			default:
				mout.warn() << "file contains more than two datasets, discarding them" << mout.endl;
				return;
			}




			if (!srcData.hasQuality()){
				mout.note() << "no input quality; creating default field " << mout.endl;
				PlainData<CartesianDst> & srcQ = srcData.getQualityData();
				srcData.createSimpleQualityData(srcQ, 1.0, 0.0, 1.0); // undetect is often "true"
				// next lines better here?

				//srcQuality.setPhysicalRange(0.0, 1.0);
				//srcQuality.updateTree2();
				// = ODIM::copyToH5<ODIMPathElem::DATA>(srcQuality.odim, srcQuality.getTree());
				// = DataTools::updateInternalAttributes(srcQuality.getTree());
				srcQ.data.properties.updateFromMap(srcQ.odim); /// IMPORTANT: DataSet<CartesianDst> below will read properties.
			}
			else {
				mout.note() << "check gain: " <<  srcData.getQualityData().data.getScaling() << mout.endl;
			}

			PlainData<CartesianDst> & srcQuality = srcData.getQualityData();

			//mout.note() << "srcQuality scaling0: " << srcQuality.data.getScaling() << mout.endl;

			if (bean.optPreprocess()){

				// dataset path
				ODIMPath parent = path;
				parent.pop_back();


				//mout.warn() << "preparing to preprocess" << mout.endl;
				mout.note() << "preprocessing..." << mout.endl;

				const std::string quantityMod = srcData.odim.quantity + "_MOD";

				mout.info() << "creating new data array for '"<< quantityMod << "' under path=" << parent << mout.endl;

				// mout.note() << "srcQuality scalingA: " << srcQuality.data.getScaling() << mout.endl;

				/*
				QualityDataSupport<CartesianDst> test(resources.cartesianHi5(parent));
				mout.note() << "srcQuality scalingÅ: " << srcQuality.data.getScaling() << mout.endl;
				QualityDataSupport<CartesianDst> test2(test);
				mout.note() << "srcQuality scalingØ: " << srcQuality.data.getScaling() << mout.endl;
				*/
				//drain::Log & log = drain::getLog();
				//log.setVerbosity(20);

				DataSet<CartesianDst> dstDataSet(resources.cartesianHi5(parent));

				// mout.note() << "srcQuality scalingB: " << srcQuality.data.getScaling() << mout.endl;

				// mout.error() << "thats it" << mout.endl;

				Data<CartesianDst> & srcDataMod = dstDataSet.getData(quantityMod);
				srcDataMod.setNoSave(ProductBase::outputDataVerbosity==0);

				// mout.note() << "srcQuality scalingC: " << srcQuality.data.getScaling() << mout.endl;

				srcDataMod.odim.updateFromMap(srcData.odim);
				srcDataMod.odim.quantity = quantityMod;
				srcDataMod.odim.product = "oflowPreprocess"; //
				//if (bean.optResize())
				srcDataMod.odim.prodpar = getParameters().toStr();
				srcDataMod.data.adoptScaling(srcData.data);
				//mout.warn() << "srcDataMod scaling: " << srcDataMod.data.getScaling() << mout.endl;


				//smoother.process(srcData.data, srcDataMod.data);
				PlainData<CartesianDst> & srcQualityMod = srcDataMod.getQualityData();
				getQuantityMap().setQuantityDefaults(srcQualityMod, "QIND");
				//srcQualityMod.copyEncoding(srcQuality);
				//srcQualityMod.data.adoptScaling(srcQuality.data);
				//srcQualityMod.setPhysicalRange(0.0, 0.999);
				// mout.note() << "srcQuality scaling1: " << srcQuality.data.getScaling() << mout.endl;
				//srcQualityMod.updateTree2();
				//mout.warn() << "srcQualityMod scaling2: " << srcQualityMod.data.getScaling() << mout.endl;
				//srcQualityMod.copyEncoding(srcQuality);

				bean.preprocess(srcData.data, srcQuality.data, srcDataMod.data, srcQualityMod.data);
				/*
				mout.note() << "srcQualityMod scaling3: " << srcQualityMod.data.getScaling() << mout.endl;
				drain::image::File::write(srcQuality.data, "dataQ.png");
				drain::image::File::write(srcDataMod.data, "datamod.png");
				drain::image::File::write(srcQualityMod.data, "datamodQ.png");
				*/
				srcTray.appendImage(srcDataMod.data);
				srcTray.appendAlpha(srcQualityMod.data);
			}
			else {
				//srcTray.setChannels(srcData.data, quality);
				srcTray.appendImage(srcData.data);
				srcTray.appendAlpha(srcQuality.data);
			}

			/*
			if (srcData.hasQuality()){
				mout.info() << "appending alpha channel of " << path << mout.endl;
				srcTray.appendAlpha(srcData.getQualityData().data[0]);
			}
			*/


		}
		else {
			mout.note() << "empty data in " << *it << "/data" << mout.endl;
		}

	}

	// mout.note() << "input odim: " << odim << mout.endl;

	switch (srcTray.size()) {
	case 2:
		// OK
		return;
		break;
	case 1:
		mout.warn() << "found " << srcTray << mout.endl;
		mout.error() << "only one data array found with selector: " << selector << mout.endl;
		break;
	case 0:
		mout.error() << "no data found with selector: " << selector << mout.endl;
		break;
	default:
		mout.warn() << "found more than 2 input fields: " << selector << mout.endl;
	}


}

/// The result is stored in this channel pack.
void CartesianOpticalFlow::getDiff(size_t width, size_t height, double max, ImageTray<Channel> & channels) const {

	Logger mout(__FUNCTION__, getName());

	//mout.info() << "start" << mout.endl;

	RackResources & resources = getResources();

	channels.clear();

	ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	DataSelector::getNextChild(resources.cartesianHi5, parent);

	mout.info() << "storing diff channels to: " << parent << mout.endl;

	Hi5Tree & dstH5 = (resources.cartesianHi5)[parent];
	dstH5.data.noSave = (ProductBase::outputDataVerbosity==0);

	DataSet<CartesianDst> dstDataSet(dstH5);


	for (std::size_t i = 0; i < bean.getDiffChannelCount(); ++i) {
		std::stringstream sstr;
		sstr << "DIFF" << i;
		Data<CartesianDst> & data = dstDataSet.getData(sstr.str());
		data.initialize(typeid(double), width, height);
		Image & dx = data.data;
		dx.setPhysicalScale(-max, max);// if saved as image
		channels.appendImage(dx.getChannel(0));
	}


	PlainData<CartesianDst> & wData = dstDataSet.getQualityData("QIND");
	wData.initialize(typeid(double), width, height);
	Image & w = wData.data;
	w.setPhysicalScale(0, 255.0); // if saved
	channels.setAlpha(w.getChannel(0));  // Quality

	//@ wData.updateTree();

	//@ dstDataSet.updateTree(dxData.odim);

	//debugChannels(channels, 200, 100);
	//resources.currentImage = ;
	//DataTools::updateInternalAttributes(dstH5);

}


void CartesianOpticalFlow::getMotion(size_t width, size_t height, ImageTray<Channel> & channels) const {

	Logger mout(__FUNCTION__, getName());

	RackResources & resources = getResources();

	//mout.warn() << *resources.currentHi5 << mout.endl;

	/// Derive the timestep needed further below for [m/s] scaling
	/// of the vectors (algorithm produces raw results in pixel scale).
	//  Time difference in seconds
	time_t timestep = (t2.getTime() - t1.getTime());
	mout.note() << "timestep: " << timestep << "s (" << static_cast<int>(timestep/60) << " mins)" << mout.endl;
	if (timestep != 0){
		if (timestep < 0){
			mout.warn() << "negative time difference, inverting it..." << mout.endl;
			timestep = -timestep;
		}
	}
	else {
		mout.warn() << "cannot derive vector scaling, no time difference in inputs: start=" << t1.str() << ", end=" << t2.str() << mout.endl;
		timestep = 60*15; // TODO: parameter?
	}

	// A new dataset<N> should be allocated, otherwise /dataset1/quality1/ may be overwritten or become ambiguous
	// std::string path="dataset1";
	// DataSelector::getNextOrdinalPath(resources.cartesianHi5, "dataset[0-9]/?$", path);
	ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	DataSelector::getNextChild(resources.cartesianHi5, parent);

	mout.debug() << "appending " << parent << mout.endl;

	/// Copy metadata from dataset1 (src1 ~ src2)
	CartesianODIM odim;
	odim.updateFromMap(resources.cartesianHi5.data.dataSet.properties); // projdef
	odim.updateFromMap(resources.cartesianHi5["dataset1"].data.dataSet.properties); // needed?
	//mout.debug() << "prop " << resources.cartesianHi5.data.dataSet.properties << mout.endl;
	mout.debug() << "odim: " << odim << mout.endl;

	//Hi5Tree & dstH5 = (*resources.currentHi5)(path);
	Hi5Tree & dstH5 = resources.cartesianHi5[parent];

	if (true){ // SCOPE


		DataSet<CartesianDst> dstDataSet(dstH5);

		const QuantityMap & qmap = getQuantityMap();

		/// Create array for "horizontal" motion (uField component)
		PlainData<CartesianDst> & amvu = dstDataSet.getData("AMVU");
		amvu.initialize(typeid(double), width, height);
		amvu.setPhysicalRange(-100.0, +100.0);
		amvu.data.setName("AMVU"); // for debugging
		if (odim.xscale){
			amvu.odim.gain = odim.xscale / timestep;
		}
		else {
			mout.warn() << "no xscale, cannot set  [m/s] scaling for horz (u) component" << mout.endl;
		}

		//
		//amvu.updateTree2();
		channels.set(amvu.data.getChannel(0), 0);


		/// Create array for "vertical" motion (vField component)
		PlainData<CartesianDst> & amvv = dstDataSet.getData("AMVV");
		amvv.initialize(typeid(double), width, height);
		amvv.setPhysicalRange(-100.0, +100.0);
		amvv.data.setName("AMVV"); // for debugging
		if (odim.yscale){
			amvv.odim.gain = odim.yscale / timestep;
		}
		else {
			mout.warn() << "no yscale, cannot set  [m/s] scaling for vert (v) component" << mout.endl;
		}
		//@
		//amvv.updateTree2();
		channels.set(amvv.data.getChannel(0), 1);

		mout.info() << "motion scale: (" << amvu.odim.gain << "," << amvv.odim.gain << ") m/s" << mout.endl;

		PlainData<CartesianDst> & qind = dstDataSet.getQualityData();
		qmap.setQuantityDefaults(qind, "QIND");
		qind.setGeometry(width, height);
		qind.data.setName("AMVQ");
		//@
		//qind.updateTree2();
		channels.alpha.set(qind.data.getChannel(0));
		// mout.note() << "mote: " << qind << " m/s" << mout.endl;

		//dstDataSet.updateTree(amvu.odim);
		odim.product = "AMV";
		odim.prodpar = bean.getParameters().toStr(':');
		odim.startdate = t1.str(ODIM::dateformat);
		odim.starttime = t1.str(ODIM::timeformat);
		odim.enddate   = t2.str(ODIM::dateformat);
		odim.endtime   = t2.str(ODIM::timeformat);
		//dstDataSet.updateTree3(odim); // does not update non-dataset props in images!

		//mout.warn() << "amvu props" << odim << mout.endl;
		//mout.note() << "amvu props" << amvu.data.getProperties() << mout.endl;

		resources.currentHi5 = & resources.cartesianHi5;
		// Not very useful
		resources.currentImage = & amvu.data;
		resources.currentGrayImage = & amvu.data;


	}

	// NOTE: this is neededn because  resources.cartesianHi5("dataset4") above does not propagate information.
	DataTools::updateInternalAttributes(resources.cartesianHi5);

	// hi5::Hi5Base::deleteNoSave(resources.cartesianHi5);

	//mout.note() << "input odim: " << odim << mout.endl;

}



}  // namespace rack::



// Rack
 // REP // REP
 // REP // REP // REP
