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
#include "data/SourceODIM.h"

#include "resources.h"


namespace drain {

/*
template <>
const drain::EnumDict<rack::GraphicsContext::ElemClass>::dict_t  drain::EnumDict<rack::GraphicsContext::ElemClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::GraphicsContext::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};


template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::GraphicsContext::ElemClass & key) const {
	return hasChild(EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}

/// Automatic conversion of elem classes to strings.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & value) const {
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass & key){
	return (*this)[EnumDict<rack::GraphicsContext::ElemClass>::dict.getKey(key, false)];
}
*/

}



using namespace drain;
using namespace drain::image;

namespace rack {




Hi5Tree Hdf5Context::empty;

Hdf5Context::Hdf5Context():
	currentHi5(&polarInputHi5),
	currentPolarHi5(&polarInputHi5){
}

Hdf5Context::Hdf5Context(const Hdf5Context &ctx):
	currentHi5(ctx.currentHi5),
	currentPolarHi5(ctx.currentPolarHi5){
}

template <>
const drain::EnumDict<Hdf5Context::Hi5Role>::dict_t drain::EnumDict<Hdf5Context::Hi5Role>::dict = {
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, CURRENT),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, INPUT),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, POLAR),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, CARTESIAN),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, PRODUCT),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, EMPTY),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, PRIVATE),
		DRAIN_ENUM_ENTRY(rack::Hdf5Context, SHARED),
};


/* This is basically good (by design), but _used_ wrong... So often flags not used, esp. PRIVATE, SHARED, EMPTY.
 *
 */
Hi5Tree & Hdf5Context::getMyHi5(Hi5RoleFlagger::ivalue_t filter){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.debug("filter=", h5_role::getShared().getKeys(filter, '|'), " (", filter, ')');

	mout.debug("filter=", FlagResolver::getKeys( drain::EnumDict<Hdf5Context::Hi5Role>::dict, filter, '|')); // h5_role::getShared().getKeys(filter, '|'), " (", filter, ')');
	/*
	if (!(filter & (POLAR|CARTESIAN))){
		// = auto
		mout.debug("POLAR|CARTESIAN unset, accepting both",);
	}
	*/
	//const Hi5Tree & cart = cartesianHi5;
	//if ((filter & POLAR) || (filter & PRODUCT)){
	if (filter & PRODUCT){
		if (!polarProductHi5.empty()){
			mout.experimental("Returning polar product");
			return polarProductHi5;
		}
		else {
			mout.warn("Polar product was requested, but not found");
		}
	}


	// Return Cartesian product if it is non empty, or empty Cartesian is accepted.
	if (filter & CARTESIAN){
		if ((filter & EMPTY) || !cartesianHi5.empty()){
			// mout.special("KARTTUNEN");
			// Hi5Tree *ptr = & (const Hi5Tree &)cartesianHi5;
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
		mout.error("something went wrong, could not find even EMPTY H5 with filter=" , filter );
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


Hi5Tree & Hdf5Context::getHi5Defunct(Hi5RoleFlagger::ivalue_t filter) {

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

void Hdf5Context::updateHdf5Status(VariableMap & statusMap) const {

	drain::Logger mout( __FILE__, __FUNCTION__);

	// mout.attention("start");

	if (!currentHi5)
		return;

	// const Hi5Tree & src = getMyHi5(CURRENT);
	const Hi5Tree & src = *this->currentHi5;

	if (src.empty()){
		mout.debug("My CURRENT h5 empty, skipping status update...");
		// NOTE: private/shared selection moved to @RackContext
	}
	else {

		mout.debug("(Not empty)");

		//DataSelector selector(ODIMPathElem::DATA);
		// DataSelector selector(ODIMPathElem::DATA, ODIMPathElem::ARRAY);
		DataSelector selector(ODIMPathElem::DATA|ODIMPathElem::QUALITY, ODIMPathElem::ARRAY);
		// mout.attention("status metadat0: ", select, " -> '", selector, "'"); //, ", orderFlags=", selector.order.str);
		mout.debug2("selector orderFlags=", selector.getOrder());
		//mout.special("selector orderFlags.value=", selector.orderFlags.value );
		//mout.special("selector orderFlags.own=",   selector.orderFlags.ownValue );

		// Do not consume (ie. leave value)
		selector.setParameters(select);
		selector.setMaxCount(1); // Because just one path wanted (below)

		// mout.attention("status metadata: ", select, " -> '", selector, "'"); //, ", orderFlags=", selector.order.str);
		// mout.debug("status metadata selector: ", selector, " <- ", select, "orderFlags=", selector.orderFlags);

		ODIMPath path;
		selector.getPath(src, path);

		mout.debug(path);


		if (path.empty()){
			// mout.special(selector); // ,"'"
			mout.note("h5 data exists, but no data groups found with selector: ", selector); // ,"'"
		}
		else {
			mout.debug("using path=" , path );
			DataTools::getAttributes(src, path, statusMap);

			/// Split what:source to separate fields
			const SourceODIM sourceODIM(statusMap["what:source"].toStr());
			statusMap.importCastableMap(sourceODIM); // NOD, PLC, WIGOS, ...

			const PolarODIM odim(statusMap);
			statusMap["how:NI"] = odim.getNyquist();

			statusMap["what:path"] = sprinter(path).str();
			// NEW 2021
			const drain::image::Image & img = src(path)[ODIMPathElem::ARRAY].data.image;
			if (!img.isEmpty()){
				const std::type_info & t = img.getType();
				statusMap["what:type"] = std::string(1u, drain::Type::getTypeChar(t));
				//statusMap["what:type"] = drain::Type::getTypeChar(t);
				statusMap["how:bits"] = 8*drain::Type::call<drain::sizeGetter>(t);
				statusMap["how:fulltype"] = drain::Type::call<drain::compactName>(t);
				statusMap["how:complextype"] = drain::Type::call<drain::complexName>(t);
			}

			const std::string quantity = statusMap.get("what:quantity", "");
			//mout.attention("quantity: '", quantity, "'");
			if (!quantity.empty()){
				const Quantity & q = getQuantityMap().get(quantity); // NOTE: doubled search
				// mout.attention("quantity: ", q.name, " keys:", q.keySelector);
				const FM301KeyMatcher & m = q.keySelector.retrieve(quantity);
				if (m.value.empty()){
					mout.warn("no cFM301 quantity declared for: ", quantity);
				}
				else if (m.value != quantity){
					mout.special("quantity: ", m.value, " mapped to : ", quantity);
				}
				statusMap["what:quantityName"] = m.getStandardName();
				statusMap["what:quantityDescription"] = m.getLongName();
			}
			else {
				statusMap.erase("what:quantityName");
				statusMap.erase("what:quantityDescription");
				// statusMap["what:quantityDescription"] = "TEST";
				// statusMap["what:quantityDescription"] = "TEST";
			}

		}

	}

}



} // rack::

