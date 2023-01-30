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
#include "product/DataConversionOp.h"

#include "rack.h"
#include "resources.h"

// using namespace drain;
// using namespace drain::image;

namespace rack {


RackContext::RackContext() : drain::SmartContext(__FUNCTION__){
	statusMap["version"] = __RACK_VERSION__;
	statusMap["rackContext"] = "BASE";
}

RackContext::RackContext(const RackContext & ctx): drain::SmartContext(ctx), ImageContext(ctx), Hdf5Context(ctx){
	statusMap["version"] = __RACK_VERSION__;
	statusMap["rackContext"] = "CLONED";
	inputPrefix = ctx.inputPrefix;
	outputPrefix = ctx.outputPrefix;
}

/**  Colon (:) is included due to support group prefixed ODIM variable names, e.g, "where:lon"
 *
 */
const drain::StringMapper RackContext::variableMapper("", "[a-zA-Z0-9:_]+");

// sstr << "^(.*)\\$\\{(" << chars << ")\\}(.*)$";
// const drain::RegExp RackContext::variableMapperSyntax("^(.*)\\$\\{[a-zA-Z0-9:_]+)\\}(.*)$", REG_EXTENDED);

/*
Hi5Tree & RackContext::getHi5Full(h5_role::value_t filter, h5_role::value_t... filters){

	Hi5Tree & dst = getHi5(filter);
	if (filter != 0)
		return dst;
	else
		return getHi5(filters);
};
*/

/*
Hi5Tree & RackContext::getHi5Full(h5_role::value_t & filter) {
	filter = 0;
	return empty;
}
*/

Hi5Tree & RackContext::getHi5Full(h5_role::value_t & filter) {
//Hi5Tree & RackContext::getHi5(h5_role::value_t filter) {

	drain::Logger mout( __FUNCTION__, __FILE__);
	// bool emptyOk = (filter & EMPTY)>0;
	// mout.special("Accept empty:", emptyOk);
	if ((filter & (PRIVATE|SHARED)) == 0){
		filter = (filter|PRIVATE|SHARED);
		mout.debug("Implicit PRIVATE|SHARED, accepting both, filter=", h5_role::getShared().getKeys(filter, '|'));
	}

	if (filter & PRIVATE){
		Hi5Tree & dst = getMyHi5(filter);
		if ((filter & EMPTY) || !dst.empty()){
			const drain::VariableMap & attr = dst[ODIMPathElem::WHAT].data.attributes;
			mout.info("Using PRIVATE (", h5_role::getShared().getKeys(filter, '|'),") object=", attr["object"], ", product=", attr["product"], ']');
			return dst;
		}
	}

	if (filter & SHARED){
		Hi5Tree & dst = getResources().baseCtx().getMyHi5(filter);
		if ((filter & EMPTY) || !dst.empty()){
			const drain::VariableMap & attr = dst[ODIMPathElem::WHAT].data.attributes;
			mout.info("Using SHARED (", h5_role::getShared().getKeys(filter, '|'),") object=", attr["object"], ", product=", attr["product"], ']');
			return dst;
		}
	}


	filter = 0;
	return empty;

}


Composite & RackContext::getComposite(h5_role::value_t filter){

	//RackContext & ctx  = this->template getContext<RackContext>();
	RackContext & baseCtx = getResources().baseCtx();

	drain::Logger mout(log, __FUNCTION__, __FILE__);

	if ((filter & (PRIVATE|SHARED)) == 0){
		filter = (filter|PRIVATE|SHARED);
		mout.warn("Unset PRIVATE|SHARED selection, accepting both, filter=", h5_role::getShared().getKeys(filter, '|'));
	}

	if (filter & SHARED){
		if (baseCtx.composite.isDefined()){ // raw or product
			mout.debug() << "shared composite" << mout.endl;
			return baseCtx.composite;
		}
	}

	// Else PRIVATE
	if (filter & PRIVATE){
		if (composite.isDefined()){ // raw or product
			mout.debug() << "private composite" << mout.endl;
			return composite;
		}
	}

	// Undefined, but go on...

	if (filter & SHARED){
		return baseCtx.composite;
	}

	return composite;

};


const drain::image::Image &  RackContext::getCurrentGrayImage(){ // RackContext & ctx){

	RackContext & ctx = *this;

	if ((ctx.currentGrayImage == NULL) || !select.empty()){
		findImage();
		// RackContext::findImage(ctx);
	}

	if (ctx.currentGrayImage == NULL){
		// drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		//mout.fail() << "no gray image data available, returning default image, maybe empty" << mout.endl;
		return ctx.grayImage;
	}

	return *ctx.currentGrayImage;
}

const drain::image::Image &  RackContext::getCurrentImage(){ // RackContext & ctx){

	//RackContext & ctx = *this;

	if (currentImage == NULL){
		findImage(); //RackContext::findImage(ctx);
	}

	if (currentImage == NULL){
		// drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
		//mout.fail() << "no gray image data available, returning default image, maybe empty" << mout.endl;
		return grayImage;
	}

	return *currentImage;
}

ODIMPath RackContext::findImage(){ //RackContext & ctx){

	drain::Logger mout(this->log, __FUNCTION__, __FILE__);

	DataSelector imageSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY); // TODO: modify PathMatcher output to "data|quality" instead of "other".
	// mout.accept("Image selector", imageSelector);

	imageSelector.consumeParameters(this->select); // ctx.findImage
	if (imageSelector.count > 1){
		drain::Logger mout(this->log, __FUNCTION__, __FILE__);
		mout.debug("Adjusting image selector.count=", imageSelector.count, " to 1");
		imageSelector.count = 1;
	}
	// mout.special("Image selector", imageSelector);

	imageSelector.ensureDataGroup();
	// mout.accept("Image selector", imageSelector);
	return findImage(imageSelector); //ctx,
}



ODIMPath RackContext::findImage(const DataSelector & imageSelector){ // RackContext & ctx,

	RackContext & ctx = *this;
	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	// NOTE  ODIMPathElem::ARRAY ie. "/data" cannot be searched, so it is added under DATA or QUALITY path.

	Hi5Tree & src = ctx.getHi5(RackContext::CURRENT|RackContext::PRIVATE, RackContext::CURRENT|RackContext::SHARED);


	ODIMPath path;

	if (src.empty()){
		mout.debug() << "No HDF5 data" << mout.endl;
		return path;
	}

	if (imageSelector.getPath(src, path)){

		mout.ok() << "using path: '" << path << "'" << mout.endl;

		drain::image::Image & img = src(path)[ODIMPathElem::ARRAY].data.dataSet;
		mout.info() << img.getProperties().get("what:quantity", "?") << ", scaling: " << img.getScaling() << "  " << img << mout.endl;

		if (!img.isEmpty()){
			// WHY TWICE?
			DataTools::getAttributes(src, path, img.properties); // for image processing ops?
			if (!(img.getScaling().isPhysical() || drain::Type::call<drain::typeIsSmallInt>(img.getType()))){ // CHECK LOGIC!
				mout.warn() << "no physical scaling, consider --encoding C or --encoding S" << mout.endl;
			}
			img.properties["path"] = drain::sprinter(path,"/").str();
		}
		else {
			mout.warn() << "data not found or empty data with selector: " << imageSelector << mout.endl;
			mout.debug() << "empty image: " << img.properties << mout.endl;
			mout.warn()  << "empty image: " << img << mout.endl;
			ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR); // resources.dataOk = false;
		}

		// Hence, image may also be empty.
		ctx.setCurrentImages(img);

	}
	else {
		path.clear();
		// if (path.empty()){
		mout.warn("no paths found with ", imageSelector, " (skipping?)");
		ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR);
		//return false;
	}

	return path;

}

// PREPARE current
const drain::image::Image & RackContext::updateCurrentImage(){ //RackContext & ctx,

	RackContext & ctx = *this;
	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);

	ODIMPath path;

	// if ctx.select ..
	if (!ctx.select.empty()){
		mout.info("selector (", ctx.select, ")");
		path = findImage();
		mout.info("selected new image ->  ", path);
	}

	if (!ctx.targetEncoding.empty()){
		if (ctx.currentImage != NULL){
			if (ctx.currentGrayImage == &ctx.grayImage){
				mout.warn() << "re-converting gray image..." << mout.endl;
			}
			RackContext::convertGrayImage(*currentGrayImage); // ctx, *ctx.
		}
	}

	if (ctx.currentImage == NULL){
		path = findImage();
		if (!path.empty())
			mout.info() << "found image at: " << path << mout.endl;
	}

	if (ctx.currentImage == NULL){
		mout.warn() << "image data not found"  << mout.endl;
		//mout.warn() << "data not found or empty data with selector: " << imageSelector << mout.endl;
		ctx.statusFlags.set(drain::StatusFlags::DATA_ERROR); // resources.dataOk = false;
		return ctx.grayImage;
	}

	// Security risk?
	//ctx.currentImage->properties["path"] = path;

	return *ctx.currentImage;


}


//void CmdImage::convertImage(const Hi5Tree & src, const DataSelector & selector, const std::string & encoding, drain::image::Image &dst){ //  const{
//bool CmdImage::convertGrayImage(RackContext & ctx){ //  const{
void RackContext::convertGrayImage(const drain::image::Image & srcImage){ // RackContext & ctx  // const{

	RackContext & ctx = *this;

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
	ODIM srcOdim(srcImage);
	if (srcOdim.scaling.scale == 0){
		mout.note() << "src image: " << srcImage << mout.endl;
		mout.warn() << "no scaling:" << (const EncodingODIM &)srcOdim  << mout.endl;
	}

	mout.debug() << "srcEncoding: " << EncodingODIM(srcOdim) << mout.endl;


	DataConversionOp<ODIM> op;
	//op.odim.copyFrom(srcImage);

	op.odim.updateFromCastableMap(srcOdim); // quantity etc?
	ProductBase::completeEncoding(op.odim, ctx.targetEncoding);
	mout.debug() << "target: '" << ctx.targetEncoding << "' -> "<< EncodingODIM(op.odim) << mout.endl;
	ctx.targetEncoding.clear();


	ctx.grayImage.properties.importCastableMap(op.odim); // Optional
	op.processImage(srcOdim, srcImage, op.odim, ctx.grayImage);
	// ctx.grayImage.setScaling(op.odim.scaling.scale, op.odim.scaling.offset);
	mout.debug() << "result: " << ctx.grayImage << mout.endl;
	ctx.setCurrentImages(ctx.grayImage);

}



drain::image::Image &  RackContext::getModifiableImage(){  // RackContext & ctx

	RackContext & ctx = *this;

	drain::Logger mout(ctx.log, __FUNCTION__, __FILE__);
	mout.note() << " getModifiableImage start" << mout.endl;

	/// Ensure
	//RackContext::updateCurrentImage(ctx);
	//RackContext::findImage(ctx);
	getCurrentImage(); // ctx

	if (ctx.currentImage == NULL){
		mout.warn() << " could not find image, yt" << mout.endl;
		return ctx.grayImage;
	}
	else if (ctx.currentImage == &ctx.colorImage){
		mout.debug() << " using already existing colour image" << mout.endl;
		return ctx.colorImage;
	}
	else if (ctx.currentImage == &ctx.grayImage){
		mout.debug() << " using already existing (additional) gray image" << mout.endl;
		if (ctx.imagePhysical){
			mout.warn() << " check/ensure physical scale(?)" << mout.endl;
		}
		return ctx.grayImage;
	}
	else { // ctx.currentImage != &ctx.grayImage
		//mout.debug() << " currentImage not modifiable, creating a copy to grayImage" << mout.endl;
		//convertGrayImage(*currentImage);
		const drain::FlexVariable & quantity = ctx.currentGrayImage->properties["what:quantity"];
		mout.warn() << "Experimental! Using h5-stored image, quantity=" << quantity << ", " << *ctx.currentImage << mout;
		return *((drain::image::Image *)ctx.currentImage); // force...
	}

	return ctx.grayImage;
}


const CoordinatePolicy RackResources::polarLeft(CoordinatePolicy::POLAR, CoordinatePolicy::WRAP, CoordinatePolicy::LIMIT, CoordinatePolicy::WRAP);

const CoordinatePolicy RackResources::limit(CoordinatePolicy::LIMIT, CoordinatePolicy::LIMIT, CoordinatePolicy::LIMIT,CoordinatePolicy::LIMIT);

/* TODO: more status flags
 * INCOMPLETE_PRODUCT ?
 */




bool RackContext::guessDatasetGroup(const Hi5Tree & src, ODIMPathElem & pathElem) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	pathElem.set(ODIMPathElem::DATASET, 1);
	//ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	if (appendResults.is(ODIMPathElem::DATASET)){
		DataSelector::getNextChild(src, pathElem);
		/*
		if (pathElem == currentPath.front()){
			mout.note() << "this path could have been set automatically: " << currentPath << mout.endl;
		}
		*/
		return true;
	}
	else if (appendResults.is(ODIMPathElem::DATA)){
		DataSelector::getLastChild(src, pathElem);
		if (pathElem.index == 0){
			pathElem.index = 1;
		}
		//path << parent;
		/*
		if (pathElem == currentPath.front()){
			mout.note() << "this path could have been set automatically: " << currentPath << mout.endl;
		}
		*/
		return true;
	}
	else {
		//path = currentPath;
		return false;
	}
}


void RackContext::updateStatus(){

	updateImageStatus(statusMap);
	updateHdf5Status(statusMap);

	/// Miscellaneous
	statusMap["select"]      = select;
	statusMap["andreSelect"] = andreSelect;
	statusMap["composite"]   = "segFault..."; //drain::sprinter(composite).str();
	// statusMap["accumulator"] = acc.toStr();

}







//static drain::Flagger::value_t RackResources::BBOX_STATUS = getResources().getContext().get().statusFlags.add("BBOX");


//drain::Logger RackResources::mout("racklet"); inputOk(true), dataOk(true),
RackResources::RackResources() { // : scriptExec(scriptParser.script) { //currentImage(NULL), currentGrayImage(NULL), inputSelect(0),, errorFlags(errorFlagValue, errorFlagDict, ',')
	polarAccumulator.setMethod("WAVG");
	//andreSelect = "dataset1,count=1";

}

/*
void RackResources::setSource(Hi5Tree & dst, const drain::Command & cmd){

	drain::Logger mout("RackResources", __FUNCTION__);

	typedef std::map<void *, const drain::Command *> sourceMap;
	static sourceMap m;

	if (m[&dst] != &cmd){
		mout.debug() << "Cleared dst for " << cmd.getName() << mout.endl;
		dst.clear();
	}

	m[&dst] = &cmd;

}
*/




/// Default instance
RackResources & getResources() {
	static RackResources resources;
	return resources;
}


} // rack::

