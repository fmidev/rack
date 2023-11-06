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

ImageContext::ImageContext(): imagePhysical(true), qualityGroups(ODIMPathElem::NONE),
		currentImage(nullptr), currentGrayImage(nullptr) {
}

ImageContext::ImageContext(const ImageContext & ctx):
		imagePhysical(ctx.imagePhysical),
		qualityGroups(ctx.qualityGroups),
		currentImage(ctx.currentImage),
		currentGrayImage(ctx.currentGrayImage) {
}

// Note: for example Palette uses this to extract actual quantity
/**
 *  Note: for example Palette uses this to extract actual quantity
 *
 */
std::string ImageContext::outputQuantitySyntax("${what:quantity}|${cmdKey}(${cmdArgs})");


void ImageContext::getImageInfo(const drain::image::Image *ptr, Variable & entry) {
	std::stringstream sstr;
	if (ptr != nullptr){
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








Hi5Tree Hdf5Context::empty;


Hdf5Context::Hdf5Context():
	currentHi5(&polarInputHi5),
	currentPolarHi5(&polarInputHi5){
}

Hdf5Context::Hdf5Context(const Hdf5Context &ctx):
	currentHi5(ctx.currentHi5),
	currentPolarHi5(ctx.currentPolarHi5){
}

const Hdf5Context::h5_role::value_t Hdf5Context::CURRENT = h5_role::addEntry("CURRENT"); //,    **< Link also external targets *
const Hdf5Context::h5_role::value_t Hdf5Context::INPUT   = h5_role::addEntry("INPUT"); // ,      **< No not link, but add entry (void) *
const Hdf5Context::h5_role::value_t Hdf5Context::POLAR   = h5_role::addEntry("POLAR"); // =4,      **< No action *
const Hdf5Context::h5_role::value_t Hdf5Context::CARTESIAN=h5_role::addEntry("CARTESIAN"); // =8,  **< Throw exception *
const Hdf5Context::h5_role::value_t Hdf5Context::EMPTY   = h5_role::addEntry("EMPTY"); // =16,     **< Also accept empty  *
const Hdf5Context::h5_role::value_t Hdf5Context::PRIVATE = h5_role::addEntry("PRIVATE"); // =32,
const Hdf5Context::h5_role::value_t Hdf5Context::SHARED  = h5_role::addEntry("SHARED"); // =64     **< Try shared first  *

/* This is basically good (by design), but _used_ wrong... So often flags not used, esp. PRIVATE, SHARED, EMPTY.
 *
 */
Hi5Tree & Hdf5Context::getMyHi5(h5_role::value_t filter) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug("filter=", filter, h5_role::getShared().getKeys(filter, '|'));

	/*
	if (!(filter & (POLAR|CARTESIAN))){
		// = auto
		mout.debug("POLAR|CARTESIAN unset, accepting both",);
	}
	*/


	// Return Cartesian product if it is non empty, or empty Cartesian is accepted.
	if (filter & CARTESIAN){
		if ((filter & EMPTY) || !cartesianHi5.empty()){
			//mout.special("KARTTUNEN");
			return cartesianHi5;
		}
		//what about currentHi5 -> input? (if Cart) Was it converted (swapped)...
	}

	// Return polar data/product, if it is non empty, or empty polar is accepted.
	if (filter & POLAR){

		if (filter & CURRENT){ // = priorize current, "latest" (be it input volume or product

			if ((filter & EMPTY) || (currentPolarHi5 && !currentPolarHi5->empty())){
				// require local
				if (currentPolarHi5 == &polarProductHi5){
					return polarProductHi5;
				}
				else if (currentPolarHi5 == &polarInputHi5){
					return polarInputHi5;
				}
			}
		}

		if (filter & INPUT){
			if ((filter & EMPTY) || !polarInputHi5.empty())
				return polarInputHi5;
		}

		if ((filter & EMPTY) || !polarProductHi5.empty())
			return polarProductHi5;

		//if ((filter & EMPTY) || !inputHi5.empty())
		//	return inputHi5;
	}


	if (filter & CURRENT){
		if ((filter & EMPTY) || (currentHi5 && !currentHi5->empty()))
			return *currentHi5;
	}

	if ((filter & EMPTY)){
		mout.error() << "something went wrong, could not find even EMPTY H5 with filter=" << filter << mout.endl;
	}

	// debugging
	/*
	if ((filter==CARTESIAN) || (filter==POLAR) || (filter==CURRENT)) {
		mout.error("Requested strictly ", h5_role::getShared().getKeys(filter), ": not found");
	}
	*/

	// mout.note("Requested: ", filter, '=', h5_role::getShared().getKeys(filter), ": not found");

	return empty;
	//return inputHi5;


}


Hi5Tree & Hdf5Context::getHi5Defunct(h5_role::value_t filter) {

	drain::Logger mout( __FILE__, __FUNCTION__);


	bool emptyOk = (filter & EMPTY)>0;
	// mout.special("Accept empty:", emptyOk);

	Hi5Tree & dst = getMyHi5(filter);
	if (emptyOk || !dst.empty()){
		mout.debug("PRIVATE [", dst[ODIMPathElem::WHAT].data.attributes["object"], ']');
		return dst;
	}

	// mout.experimental("Changed/fixed CARTESIAN -> filter here");
	Hi5Tree & dstShared = drain::Static::get<Hdf5Context>().getMyHi5(filter); //CARTESIAN);
	if (emptyOk || !dstShared.empty()){
		// mout.note() = "shared";
		mout.debug( "SHARED [", dstShared[ODIMPathElem::WHAT].data.attributes["object"], ']');
		return dstShared;
	}

	/*
	const Hdf5Context & ctx = getStaticContext();
	mout.special("StaticContext: input empty=", ctx.inputHi5.empty() );
	mout.special("StaticContext: polar empty=", ctx.polarHi5.empty() );
	mout.special("StaticContext: cart  empty=", ctx.cartesianHi5.empty() );
	*/

	mout.info("PRIVATE, empty");
	return dst;

}

void Hdf5Context::updateHdf5Status(VariableMap & statusMap) {

	drain::Logger mout( __FILE__, __FUNCTION__);

	const Hi5Tree & src = getMyHi5(CURRENT);

	if (src.empty()){
		mout.debug("My CURRENT h5 empty, skipping status update...");
		// NOTE: private/shared selection moved to @RackContext
	}
	else {

		mout.debug("(Not empty)");

		DataSelector selector(ODIMPathElem::DATA);
		mout.debug2("selector orderFlags=", selector.order.str);
		//mout.special("selector orderFlags.value=", selector.orderFlags.value );
		//mout.special("selector orderFlags.own=",   selector.orderFlags.ownValue );

		// Do not consume (ie. leave value)
		selector.setParameters(select);
		selector.count = 1; // Because just one path wanted (below)

		mout.debug("status metadata selector: ", selector, " <- ", select, ", orderFlags=", selector.order.str);
		// mout.debug("status metadata selector: ", selector, " <- ", select, "orderFlags=", selector.orderFlags);

		ODIMPath path;
		selector.getPath(src, path);

		mout.debug() << path << mout.endl;


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

			const PolarODIM odim(statusMap);
			statusMap["how:NI"] = odim.getNyquist();

			statusMap["what:path"] = sprinter(path).str();
			// NEW 2021
			const drain::image::Image & img = src(path)[ODIMPathElem::ARRAY].data.dataSet;
			if (!img.isEmpty()){
				const std::type_info & t = img.getType();
				statusMap["what:type"] = std::string(1u, drain::Type::getTypeChar(t));
				//statusMap["what:type"] = drain::Type::getTypeChar(t);
				statusMap["how:bits"] = 8*drain::Type::call<drain::sizeGetter>(t);
				statusMap["how:fulltype"] = drain::Type::call<drain::compactName>(t);
				statusMap["how:complextype"] = drain::Type::call<drain::complexName>(t);
			}

		}

	}

}



} // rack::

