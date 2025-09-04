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


namespace rack {

RackContext::RackContext() : drain::SmartContext(__FUNCTION__) {
	statusMap["version"] = __RACK_VERSION__;
	statusMap["rackContext"] = "BASE";
	svgTrack->setType(drain::image::svg::SVG); //, xmlTrack(drain::image::BaseSVG::SVG)
	svgTrack->set("width", 64);
	svgTrack->set("height", 48);
}

RackContext::RackContext(const RackContext & ctx): drain::SmartContext(ctx), ImageContext(ctx), Hdf5Context(ctx){
	drain::Logger mout( __FILE__, __FUNCTION__);
	mout.special("copying ", __FUNCTION__);

	// , xmlTrack(drain::image::BaseSVG::SVG)
	svgTrack->setType(drain::image::svg::SVG);

	statusMap["version"] = __RACK_VERSION__;
	statusMap["rackContext"] = "CLONED";
	inputPrefix = ctx.inputPrefix;
	outputPrefix = ctx.outputPrefix;
}

void RackContext::resolveFilePath(const std::string & prefix, const std::string & filePath, std::string & finalFilePath){

	drain::Logger mout(this->log, __FILE__, __FUNCTION__);

	if (filePath.empty()){
		mout.error("empty filename: '", filePath, "'");
	}
	else if (prefix.empty()){
		finalFilePath = filePath;
	}
	else if (filePath.at(0) == '/'){
		mout.revised<LOG_NOTICE>("file path starts with '/' – omitting prefix '", prefix, "'");
		mout.advice("append '/' to prefix if you meant '", prefix, filePath, "'");
		/*
		size_t l = prefix.length();
		if (prefix.at(l-1) == '/'){
			//mout.discouraged("inputPrefix ends with '/'");
			mout.advice("remove '/' from '", filePath,"' and append it to prefix if you meant '", prefix.substr(0,l-1), filePath, "'");
		}
		*/
		finalFilePath = filePath;
	}
	else if (filePath.substr(0,2) == "./"){
		mout.revised<LOG_NOTICE>("file path starts with './' – omitting prefix '", prefix, "'");
		finalFilePath = filePath;
	}
	else {
		finalFilePath = prefix + filePath;
	}

	/*
	if (filePath.at(0) != '/'){
		finalFilePath = prefix + filePath;
		// return finalFilePath;
	}
	else if (prefix.empty()){
		finalFilePath = filePath;
		// return filePath;
	}
	else {
		finalFilePath = filePath;
		// return filePath;
	}
	*/


}

// const std::string RackContext::validVariableChars("[a-zA-Z0-9][a-zA-Z0-9_:]*");

/**
 *
 *   Special chars accepted:
 *   - colon (:) for ODIM variable names prefixed with "where:", "where:", and "how:"
 *   - pipe  (|) for separating optional string processing
 *   - colon (%) for time formatting (after pipe '|')
 *
 */
//const drain::StringMapper RackContext::variableMapper("", "[a-zA-Z0-9][a-zA-Z0-9_:]*(\\|[^}]*)?");
const drain::StringMapper RackContext::variableMapper("", "[a-zA-Z0-9][a-zA-Z0-9_:]*"); // NEW: post processing separated

const VariableFormatterODIM<drain::Variable> RackContext::variableFormatter;

const VariableFormatterODIM<drain::FlexibleVariable> RackContext::flexVariableFormatter;

std::string RackContext::getFormattedStatus(const std::string & format) const {
	drain::StringMapper smapper(RackContext::variableMapper); // XXX
	smapper.parse(format);
	return smapper.toStr(getStatusMap(), '_', RackContext::variableFormatter);
}
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

//Hi5Tree & RackContext::getHi5Full(h5_role::ivalue_t & filter) {
Hi5Tree & RackContext::getHi5Full(Hi5RoleFlagger::ivalue_t filter) {
//Hi5Tree & RackContext::getHi5(h5_role::value_t filter) {

	drain::Logger mout(this->log, __FILE__, __FUNCTION__);

	const drain::EnumDict<Hdf5Context::Hi5Role>::dict_t & dict = drain::EnumDict<Hdf5Context::Hi5Role>::dict;

	// bool emptyOk = (filter & EMPTY)>0;
	// mout.special("Accept empty:", emptyOk);
	if ((filter & (PRIVATE|SHARED)) == 0){
		filter = (filter|PRIVATE|SHARED);
		// FlagResolver::getKeys(dict, this->value, this->separator)
		// mout.debug("Implicit PRIVATE|SHARED, accepting both, filter=", h5_role::getKeysNEW2(filter, '|'));
		mout.debug("Implicit PRIVATE|SHARED, accepting both, filter=", drain::FlagResolver::getKeys(dict, filter, '|'));
	}

	if (filter & PRIVATE){
		Hi5Tree & dst = getMyHi5(filter);
		if ((filter & EMPTY) || !dst.empty()){
			const drain::VariableMap & attr = dst[ODIMPathElem::WHAT].data.attributes;
			mout.info("Using PRIVATE (", drain::FlagResolver::getKeys(dict, filter, '|') ,") object=", attr["object"], ", product=", attr["product"], ']');
			return dst;
		}
	}

	if (filter & SHARED){
		Hi5Tree & dst = getResources().baseCtx().getMyHi5(filter);
		if ((filter & EMPTY) || !dst.empty()){
			const drain::VariableMap & attr = dst[ODIMPathElem::WHAT].data.attributes;
			mout.info("Using SHARED (", drain::FlagResolver::getKeys(dict, filter, '|'), ") object=", attr["object"], ", product=", attr["product"], ']');
			return dst;
		}
	}


	filter = 0; // currently not referenced
	return empty;

}


Composite & RackContext::getComposite(Hi5RoleFlagger::ivalue_t filter){

	//RackContext & ctx  = this->template getContext<RackContext>();
	RackContext & baseCtx = getResources().baseCtx();

	drain::Logger mout(log, __FILE__, __FUNCTION__);

	if ((filter & (PRIVATE|SHARED)) == 0){
		filter = (filter|PRIVATE|SHARED);
		const drain::EnumDict<Hdf5Context::Hi5Role>::dict_t & dict = drain::EnumDict<Hdf5Context::Hi5Role>::dict;
		mout.warn("Unset PRIVATE|SHARED selection, accepting both, filter=", drain::FlagResolver::getKeys(dict, filter, '|')); // h5_role::getKeysNEW2(filter, '|'));
	}

	if (filter & SHARED){
		if (baseCtx.composite.isDefined()){ // raw or product
			mout.debug("shared composite" );
			return baseCtx.composite;
		}
	}

	// Else PRIVATE
	if (filter & PRIVATE){
		if (composite.isDefined()){ // raw or product
			mout.debug("private composite" );
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
		// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		//mout.fail("no gray image data available, returning default image, maybe empty" );
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
		// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		//mout.fail("no gray image data available, returning default image, maybe empty" );
		return grayImage;
	}

	return *currentImage;
}

ODIMPath RackContext::findImage(){ //RackContext & ctx){

	drain::Logger mout(this->log, __FILE__, __FUNCTION__);

	//DataSelector imageSelector(ODIMPathElem::DATA|ODIMPathElem::QUALITY); // TODO: modify PathMatcher output to "data|quality" instead of "other".
	DataSelector imageSelector;
	// mout.accept("Image selector", imageSelector);
	// mout.attention(__FUNCTION__, ':', __LINE__, " quantity: ", this->getStatusMap().get("what:quantity","??"));
	if (!this->select.empty()){
		mout.revised<LOG_INFO>("keeping, not clearing selector: ", this->select);
	}
	// imageSelector.consumeParameters(this->select);

	imageSelector.setParameters(this->select);
	if (imageSelector.getMaxCount() > 1){
		mout.debug("Adjusting image selector.count=", imageSelector.getMaxCount(), " to 1");
		imageSelector.setMaxCount(1);
	}

	// mout.attention(__FUNCTION__, ':', __LINE__, " quantity: ", this->getStatusMap().get("what:quantity","??"));
	// mout.accept("Image selector1: ", imageSelector);

	imageSelector.ensureDataGroup();
	// mout.accept("Image selector2: ", imageSelector);

	return findImage(imageSelector); //ctx,
}



ODIMPath RackContext::findImage(const DataSelector & imageSelector){ // RackContext & ctx,

	//RackContext & ctx = *this;
	drain::Logger mout(this->log, __FILE__, __FUNCTION__);

	// NOTE  ODIMPathElem::ARRAY ie. "/data" cannot be searched, so it is added under DATA or QUALITY path.

	mout.experimental<LOG_DEBUG>("findImage / imageSelector: ", imageSelector);


	Hi5Tree & src = this->getHi5(RackContext::CURRENT|RackContext::PRIVATE, RackContext::CURRENT|RackContext::SHARED);


	ODIMPath path;

	if (src.empty()){
		mout.debug("No HDF5 data" );
		return path;
	}

	if (imageSelector.getPath(src, path)){

		mout.ok("using path: '" , path , "'" );

		drain::image::Image & img = src(path)[ODIMPathElem::ARRAY].data.image;
		mout.info("quantity [", img.getProperties().get("what:quantity", "?") , "], scaling: " , img.getScaling() , "  " , img );
		// mout.attention("coordPolicy: " , img.getCoordinatePolicy());
		// mout.attention("quantity [", img.getProperties().get("what:quantity", "?") , "], scaling: " , img.getScaling() , "  " , img );

		if (!img.isEmpty()){
			// WHY TWICE?
			mout.ok("TEST-start");
			DataTools::getAttributes(src, path, img.properties); // for image processing ops?
			mout.ok("TEST-end");
			const std::type_info & t = img.getType();
			if (!(img.getScaling().isPhysical() || drain::Type::call<drain::typeIsSmallInt>(t))){ // CHECK LOGIC!
				if (targetEncoding.empty()){
					mout.warn("storage type '", drain::Type::call<drain::simpleName>(t),  "', no physical scaling, consider --encoding C or --encoding S" );
				}
			}
			img.properties["path"] = drain::sprinter(path,"/").str();
			// mout.attention("quantity [", img.getProperties().get("what:quantity", "?") , "], scaling: " , img.getScaling() , "  " , img );
		}
		else {
			mout.warn("data not found or empty data with selector: " , imageSelector );
			mout.debug("empty image: " , img.properties );
			mout.warn("empty image: ", img);
			this->statusFlags.set(drain::Status::DATA_ERROR); // resources.dataOk = false;
		}

		mout.attention<LOG_DEBUG>("found: ", img);

		// Hence, image may also be empty.
		this->setCurrentImages(img);

	}
	else {
		path.clear();
		// if (path.empty()){
		mout.warn("no paths found with ", imageSelector, " (skipping?)");
		for (const drain::StringMatcher & matcher: imageSelector.getQuantitySelector()){
			mout.warn("\t quantity matcher: ", matcher.value, " [", matcher.getType(), ']');
		}

		this->statusFlags.set(drain::Status::DATA_ERROR);
		//return false;
	}

	return path;

}

// PREPARE current
const drain::image::Image & RackContext::updateCurrentImage(){ //RackContext & ctx,

	RackContext & ctx = *this;
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	ODIMPath path;

	// mout.attention(__FUNCTION__, ':', __LINE__, " quantity: ", ctx.getStatusMap().get("what:quantity","??"));

	// if ctx.select ..
	if (!ctx.select.empty()){
		mout.info("selector: ", ctx.select);
		path = findImage();
		mout.info("selected image at: ", path);
	}

	// mout.attention(__FUNCTION__, ':', __LINE__, " quantity: ", ctx.getStatusMap().get("what:quantity","??"));

	if (!ctx.targetEncoding.empty()){
		if (ctx.currentImage != NULL){
			if (ctx.currentGrayImage == &ctx.grayImage){
				mout.warn("Re-converting gray image...");
			}
			else {
				mout.hint<LOG_INFO>("Use --convert to change encoding in HDF5 struct");
				mout.info("Converting encoding...");
			}
			mout.info("Converting GrayImage: ", path, " coord policy: ", ctx.currentGrayImage->getCoordinatePolicy());
			RackContext::convertGrayImage(*currentGrayImage); // ctx, *ctx.
		}
	}


	if (ctx.currentImage == NULL){
		mout.attention("Still: ctx.currentImage == NULL");
		path = findImage();
		if (!path.empty())
			mout.info("found image at: ", path);
	}

	// mout.attention(__FUNCTION__, ':', __LINE__, " quantity: ", ctx.getStatusMap().get("what:quantity","??"));

	if (ctx.currentImage == NULL){
		mout.warn("image data not found");
		//mout.warn("data not found or empty data with selector: " , imageSelector );
		ctx.statusFlags.set(drain::Status::DATA_ERROR); // resources.dataOk = false;
		return ctx.grayImage;
	}
	else {
		mout.info("Image: ", path, ": ", ctx.currentImage->getCoordinatePolicy());
		//ctx.currentImage->properties["path"] = path;
		return *ctx.currentImage;
	}


}


//void CmdImage::convertImage(const Hi5Tree & src, const DataSelector & selector, const std::string & encoding, drain::image::Image &dst){ //  const{
//bool CmdImage::convertGrayImage(RackContext & ctx){ //  const{
void RackContext::convertGrayImage(const drain::image::Image & srcImage){ // RackContext & ctx  // const{

	RackContext & ctx = *this;

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	ODIM srcOdim(srcImage);
	if (srcOdim.scaling.scale == 0){
		mout.note("src image: ", srcImage);
		mout.error("no scaling:", (const EncodingODIM &)srcOdim);
	}

	mout.special<LOG_DEBUG>("srcEncoding: ", EncodingODIM(srcOdim));
	mout.special<LOG_DEBUG>("srcCoords: ", srcImage.getCoordinatePolicy());

	/// TODO: wrap this and --convert to same code
	DataConversionOp<ODIM> op;
	op.setEncodingRequest(ctx.targetEncoding);
	ctx.targetEncoding.clear();

	op.processImage2023(srcOdim, srcImage, ctx.grayImage);
	/*
	op.odim.updateFromCastableMap(srcOdim); // quantity etc?
	op.odim.completeEncoding( ctx.targetEncoding);
	mout.debug("target: '", ctx.targetEncoding, "' -> ", EncodingODIM(op.odim));
	// ctx.grayImage.properties.importCastableMap(op.odim); // Optional
	op.processImage(srcOdim, srcImage, op.odim, ctx.grayImage);
	*/
	// ctx.grayImage.setScaling(op.odim.scaling.scale, op.odim.scaling.offset);
	// ctx.grayImage.setCoordinatePolicy(srcImage.getCoordinatePolicy());
	mout.debug("result: ", ctx.grayImage);
	ctx.setCurrentImages(ctx.grayImage);

}



drain::image::Image &  RackContext::getModifiableImage(){  // RackContext & ctx

	RackContext & ctx = *this;

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	// mout.note(" getModifiableImage start" );

	/// Ensure
	//RackContext::updateCurrentImage(ctx);
	//RackContext::findImage(ctx);
	getCurrentImage(); // ctx

	if (ctx.currentImage == NULL){
		mout.warn(" could not find image, yt" );
		return ctx.grayImage;
	}
	else if (ctx.currentImage == &ctx.colorImage){
		mout.debug(" using already existing colour image" );
		return ctx.colorImage;
	}
	else if (ctx.currentImage == &ctx.grayImage){
		mout.debug(" using already existing (additional) gray image" );
		if (ctx.imagePhysical){
			mout.warn(" check/ensure physical scale(?)" );
		}
		return ctx.grayImage;
	}
	else { // ctx.currentImage != &ctx.grayImage
		//mout.debug(" currentImage not modifiable, creating a copy to grayImage" );
		//convertGrayImage(*currentImage);
		const drain::FlexibleVariable & quantity = ctx.currentGrayImage->properties["what:quantity"];
		mout.experimental<LOG_INFO>("Using h5-stored image, quantity=[" , quantity , "], " , *ctx.currentImage );
		return *((drain::image::Image *)ctx.currentImage); // force...
	}

	return ctx.grayImage;
}



/* TODO: more status flags
 * INCOMPLETE_PRODUCT ?
 */




bool RackContext::guessDatasetGroup(const Hi5Tree & src, ODIMPathElem & pathElem) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	pathElem.set(ODIMPathElem::DATASET, 1);
	//ODIMPathElem parent(ODIMPathElem::DATASET, 1);
	if (appendResults.is(ODIMPathElem::DATASET)){
		ODIMPathTools::getNextChild(src, pathElem);
		/*
		if (pathElem == currentPath.front()){
			mout.note("this path could have been set automatically: " , currentPath );
		}
		*/
		return true;
	}
	else if (appendResults.is(ODIMPathElem::DATA)){
		ODIMPathTools::getLastChild(src, pathElem);
		if (pathElem.index == 0){
			pathElem.index = 1;
		}
		//path << parent;
		/*
		if (pathElem == currentPath.front()){
			mout.note("this path could have been set automatically: " , currentPath );
		}
		*/
		return true;
	}
	else {
		//path = currentPath;
		return false;
	}
}


void RackContext::updateStatus() const {

	Context::updateStatus(); // option: add true <- kludge

	updateImageStatus(statusMap);
	updateHdf5Status(statusMap);

	/// Miscellaneous
	statusMap["select"]      = select;
	statusMap["andreSelect"] = andreSelect;
	statusMap["composite"]   = "under_constr..."; //drain::sprinter(composite).str();
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
		mout.debug("Cleared dst for " , cmd.getName() );
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

