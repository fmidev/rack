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

#include "hi5/Hi5.h"

#include "resources.h"
#include "data/SourceODIM.h"

using namespace drain;
using namespace drain::image;

namespace rack {


// const drain::StatusFlags::value_t RackContext::BBOX_STATUS =   StatusFlags::add("BBOX");

ImageContext::ImageContext(): imagePhysical(true), currentImage(nullptr), currentGrayImage(nullptr) {
}

ImageContext::ImageContext(const ImageContext & ctx):
		imagePhysical(ctx.imagePhysical),
		currentImage(ctx.currentImage),
		currentGrayImage(ctx.currentGrayImage) {
}

std::string ImageContext::outputQuantitySyntax("${what:quantity}_${command}");


void ImageContext::getImageInfo(const drain::image::Image *ptr, Variable & entry) {
	std::stringstream sstr;
	if (ptr){
		ptr->toOStr(sstr);
	}
	else {
		sstr << "NULL";
	}
	entry = sstr.str();
}

void ImageContext::updateImageStatus(drain::VariableMap & statusMap) const {

	getImageInfo(&colorImage, statusMap["img:colorImage"]);
	getImageInfo(&grayImage, statusMap["img:grayImage"]);
	getImageInfo(currentImage, statusMap["img:currentImage"]);
	getImageInfo(currentGrayImage, statusMap["img:currentGrayImage"]);

}









Hdf5Context::Hdf5Context():
	currentHi5(&inputHi5),
	currentPolarHi5(&inputHi5){
}

Hdf5Context::Hdf5Context(const Hdf5Context &ctx):
	currentHi5(ctx.currentHi5),
	currentPolarHi5(ctx.currentPolarHi5){
}

const Hdf5Context::h5_role::value_t Hdf5Context::CURRENT = h5_role::add("CURRENT"); //,    **< Link also external targets *
const Hdf5Context::h5_role::value_t Hdf5Context::INPUT   = h5_role::add("INPUT"); // ,      **< No not link, but add entry (void) *
const Hdf5Context::h5_role::value_t Hdf5Context::POLAR   = h5_role::add("POLAR"); // =4,      **< No action *
const Hdf5Context::h5_role::value_t Hdf5Context::CARTESIAN=h5_role::add("CARTESIAN"); // =8,  **< Throw exception *
const Hdf5Context::h5_role::value_t Hdf5Context::EMPTY   = h5_role::add("EMPTY"); // =16,     **< Also accept empty  *
const Hdf5Context::h5_role::value_t Hdf5Context::PRIVATE = h5_role::add("PRIVATE"); // =32,
const Hdf5Context::h5_role::value_t Hdf5Context::SHARED  = h5_role::add("SHARED"); // =64     **< Try shared first  *


//   (CARTESIAN|POLAR|INPUT|CURRENT)
Hi5Tree & Hdf5Context::getMyHi5(h5_role::value_t filter) {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const bool emptyOK = (filter & EMPTY);

	mout.debug() << "filter=" << filter << h5_role::getShared().getKeys(filter, '|') << mout.endl;
	// const drain::Variable & inputObject = inputH5[ODIMPathElem::WHAT].data.attributes["object"];

	if (filter & CARTESIAN){
		if (emptyOK || !cartesianHi5.isEmpty())
			return cartesianHi5;
	}

	if (filter & POLAR){

		if (filter & CURRENT){ // = priorize current (be it input volume or product

			if (emptyOK || (currentPolarHi5 && !currentPolarHi5->isEmpty())){
				// require local
				if (currentPolarHi5 == &polarHi5){
					return polarHi5;
				}
				else if (currentPolarHi5 == &inputHi5){
					return inputHi5;
				}
			}
		}

		if (filter & INPUT){
			if (emptyOK || !inputHi5.isEmpty())
				return inputHi5;
		}

		if (emptyOK || !polarHi5.isEmpty())
			return polarHi5;

		if (emptyOK || !inputHi5.isEmpty())
			return inputHi5;
	}


	if (filter & CURRENT){
		if (emptyOK || (currentHi5 && !currentHi5->isEmpty()))
			return *currentHi5;
	}

	if (emptyOK){
		mout.error() << "something went wrong, could not find even EMPTY H5 with filter=" << filter << mout.endl;
	}

	return inputHi5;


}


Hi5Tree & Hdf5Context::getHi5(h5_role::value_t filter) {

	drain::Logger mout( __FUNCTION__, __FILE__);

	Hi5Tree & dst = getMyHi5(filter);
	if ((filter & EMPTY) || !dst.isEmpty()){
		mout.debug() << "local [" << dst[ODIMPathElem::WHAT].data.attributes["object"] << ']' << mout.endl;
		return dst;
	}

	Hi5Tree & dstShared = getStaticContext().getMyHi5(CARTESIAN);
	if ((filter & EMPTY) || !dstShared.isEmpty()){
		// mout.note() = "shared";
		mout.debug() << "shared [" << dstShared[ODIMPathElem::WHAT].data.attributes["object"] << ']' << mout.endl;
		return dstShared;
	}

	mout.note() = "local, empty";
	return dst;

}

void Hdf5Context::updateHdf5Status(VariableMap & statusMap) {

	drain::Logger mout( __FUNCTION__, __FILE__);

	const Hi5Tree & src = getHi5(CURRENT);

	if (!src.isEmpty()){

		mout.debug() << "(Not empty)" << mout.endl;

		DataSelector selector(ODIMPathElem::DATA);

		selector.setParameters(select);

		mout.debug() << "status metadata selector: " << selector << mout.endl;
		ODIMPath path;
		selector.getPath3(src, path);

		if (path.empty()){
			//mout.special() << src << mout.endl;
			mout.note() << "data exists, but no data groups found with selector '" << selector << "'" << mout.endl;
		}
		else {
			mout.debug() << "using path=" << path << mout.endl;
			DataTools::getAttributes(src, path, statusMap);

			/// Split what:source to separate fields
			const SourceODIM sourceODIM(statusMap["what:source"].toStr());

			statusMap.importCastableMap(sourceODIM); // NOD, PLC, WIGOS, ...

			//mout.warn() << "PolarODIM" << mout.endl;
			const PolarODIM odim(statusMap);
			//mout.warn() << odim << mout.endl;
			statusMap["how:NI"] = odim.getNyquist();
		}

	}

}



} // rack::

