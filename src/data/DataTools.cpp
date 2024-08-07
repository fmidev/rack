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

#include <drain/Type.h>
#include <set>
#include <drain/util/TextDecorator.h>
#include <drain/util/TreeUtils.h>
#include "hi5/Hi5.h"
#include "radar/Coordinates.h"

#include "DataTools.h"
#include "SourceODIM.h"

namespace rack {

using namespace hi5;

std::list<std::string> & DataTools::getMainAttributes(){

	static std::list<std::string> mainAttributes = {
			"what:product",
			"what:prodpars",
			"what:object",
			"what:date",
			"what:time",
			"what:quantity",
			"where:BBOX",
			"how:angles:0.3",
			"NOD",
			"where:lat",
			"where:lon",
			"how:highprf",
			"how:lowprf",
			"how:ACCnum",
	};

	return mainAttributes;
}

/// List of most important ODIM attributes (with style suggestion).
/**
 *
 */
drain::VariableMap & DataTools::getAttributeStyles(){

	static
	drain::VariableMap attributeStyles = {
		{"format", "vt100"}, // "txt", "html" ?
		{"image", "BLUE"},
		{"data", "BOLD"},
		{"object", "WHITE"},
		{"quantity", "BOLD:GREEN"},
		{"date", "RED:UNDERLINE"},
		{"time", "RED"},
		{"startdate", "RED:UNDERLINE"},
		{"starttime", "RED"},
		{"hiPRF", "YELLOW:DIM"},
		{"source", "YELLOW:DIM"},
		{"lon", "YELLOW:DIM"},
		{"lat", "YELLOW:DIM"},
		{"xsize", "YELLOW:DIM"},
		{"ysize", "YELLOW:DIM"},
		{"rscale", "YELLOW:DIM"},
		{"elangle", "ITALIC:YELLOW"},
		{"gain", "ITALIC:YELLOW"},
		{"offset", "ITALIC:YELLOW"},
		{"nodata", "DIM:YELLOW"},
		{"undetect", "DIM:YELLOW"},
		{"task_args", "CYAN"},
		{"ACCnum", "DIM:PURPLE"},
		{"legend", "PURPLE"},
	};

	return attributeStyles;

};


bool DataTools::treeToStream(const Hi5Tree::node_data_t & data, std::ostream &ostr){

	// Shared TextDecorator!
	// RackContext & ctx = getResources().getContext<RackContext>();

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	//mout.unimplemented("Future option... ");

	bool empty = true;

	drain::VariableMap & attrs = DataTools::getAttributeStyles();

	drain::TextDecorator noDeco;
	drain::TextDecoratorVt100 vt100Deco;

	drain::TextDecorator & decorator = attrs.get("format", "") == "vt100" ? vt100Deco : noDeco;
	decorator.setSeparator(":");

	if (data.exclude){
		ostr << "~";
		return false;
	}

	const drain::image::ImageFrame & img = data.image;
	if (!img.isEmpty()){
		// if (data.attributes.hasKey("image")){
		ostr << img.getWidth() << ',' << img.getHeight() << ' ';
		ostr << drain::Type::call<drain::compactName>(img.getType());
		ostr << '[' << (8*drain::Type::call<drain::sizeGetter>(img.getType())) << ']';
		//<< drain::Type::call<drain::complexName>(img.getType());
		ostr << ' ' << img.getCoordinatePolicy() << ' ';
		empty = false;
		//}
	}
	// else ...
	char sep = 0;

	for (const auto & entry: attrs){
		if (data.attributes.hasKey(entry.first)){
			if (sep)
				ostr << sep << ' ';
			else
				sep = ',';
			//decorator.set(entry.second);
			decorator.begin(ostr, entry.second);
			ostr << entry.first << '=' << data.attributes[entry.first];
			decorator.end(ostr);
			//decorator.reset();
			empty = false;
		}
	}

	return empty;

}


void DataTools::updateInternalAttributes(Hi5Tree & src){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::FlexVariableMap & properties = src.data.image.properties;
	properties.clearVariables();
	//properties.clear(); // should not remove references (linked variables)

	// NOTE: is essentially recursive, through linked variables.

	drain::VariableMap & what = src[ODIMPathElem::WHAT].data.attributes;

	std::string object = what.get("object", "");
	if ((object == "PVOL") || (object == "SCAN")){
		src.data.image.setCoordinatePolicy(polarLeftCoords);
	}
	else if (!object.empty()){
		// In image processing ops, data is "generalized" to ODIM (from PolarODIM and CartesianODIM)
		// So, explicit setting of coord policy should be avoided
		src.data.image.setCoordinatePolicy(limitCoords);
	}

	// Easier to do this only once?
	if (what.hasKey("source")){
		SourceODIM odim(what["source"].toStr());
		properties.importCastableMap(odim);
	}

	// drain::TreeUtils::dump(src, std::cout);
	// hi5::Hi5Base::writeText(src, std::cout);

	updateInternalAttributes(src, properties);
	//updateInternalAttributes(src, drain::FlexVariableMap());
	//updateInternalAttributes(src, drain::VariableMap());
}

void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::FlexVariableMap & parentAttributes){
//void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::VariableMap & attributes){

	drain::Logger mout(__FILE__, __FUNCTION__);

	/*
	mout.special();
	for (const auto & entry : src){
		mout << entry.first << ',';
	}
	mout << mout.endl;
	*/

	//std::string path = parentAttributes.get("how:path","");
	/*
	mout.special(// "path: ", parentAttributes.get("how:path",""),
			//" coordPolicy:", parentAttributes["coordPolicy"],
			" my coordPolicy( ", src.data.image.getCoordinatePolicy());
	*/

	/// Write to "hidden" variables of this node (src).
	drain::FlexVariableMap & localAttributes = src.data.image.properties;
	localAttributes.importMap(parentAttributes); // Init with upper level (parent) values

	// mout.special("my coordProp) ", attributes["coordPolicy"]);
	// attributes.updateFromCastableMap(parentAttributes);
	// std::cerr << "MAP now: " << a << "\n\n";
	// mout.special("my coordPolicy) ", src.data.image.getCoordinatePolicy());

	/// Step 1: collect local values of \c /what, \c /where, and \c /how groups, overwriting attributes initialised to parent values.
	std::stringstream sstr;
	for (const auto & entry : src){
		// mout.warn(" consider: ", entry.first);
		if (entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			// mout.warn(" ok, from: ", entry.first);
			const drain::VariableMap & groupAttributes = entry.second.data.attributes;
			for(const auto & e: groupAttributes){
				sstr.str("");
				sstr << entry.first << ':' << e.first; // group name + ':' + group attr name (like "where:lon")
				localAttributes[sstr.str()] = e.second; // = value
				// mout.warn("  ", sstr.str() , '=' , e.second , " ... [", drain::Type::getTypeChar(e.second.getType()), ']');
			}
		}
	}

	// Step 2: Traverse other children (recursion)
	for (auto & entry: src){

		if (entry.first.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (!entry.second.data.exclude){
				mout.debug3(entry.first , " => ensure '/data' groups  " );
				entry.second[ODIMPathElem::ARRAY].data.image;
			}
		}
		else if (entry.first.is(ODIMPathElem::ARRAY)){

			drain::image::Image & img = entry.second.data.image;
			if (img.typeIsSet()){
				// Non-standard
				localAttributes["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
				// Important: ensure Image has (scale,offset) as defined by above WHAT(gain,offset)
				drain::ValueScaling & s = img.getScaling();
				s.set(localAttributes.get("what:gain", s.scale), localAttributes.get("what:offset", s.offset));
				// Needed. As a side effect, empty data1.img may get a scaling...
				localAttributes["scale"]  = s.scale;
				localAttributes["offset"] = s.offset;
				//mout.warn(a.get("what:quantity", "?") , ", scaling " , img.getScaling() , ' ' , img );
			}

			if (img.getName().empty()){
				img.setName(localAttributes.get("name",""));
			}
			//mout.warn("scaling1 " , entry.second.data.image.getScaling() );
		}


		// mout.note("considering " , entry.first );
		if (entry.first.belongsTo(ODIMPathElem::DATA_GROUPS)){ // | ODIMPathElem::ARRAY)){  //
			// mout.warn(entry.first, " .... ");
			/*
			a["how:path"] = path;
			a["how:path"] << entry.first;
			mout.special("pathy -> ", a["how:path"]);
			*/
			updateInternalAttributes(entry.second,  localAttributes); // policy,
		}

		if (entry.first.is(ODIMPathElem::ARRAY)){
			if (!entry.second.data.image.isEmpty()){
				updateInternalAttributes(entry.second,  localAttributes); // policy,
				// mout.warn("  --image ", entry.second.data.image.getName(), " coordPolicy:", entry.second.data.image.getCoordinatePolicy());
				// mout.warn("  --image ", entry.second.data.image.getName(), " scaling: ",    entry.second.data.image.getScaling());
			}
		}

	}


	// if (src.hasChild(ODIMPathElem::ARRAY))
	//   mout.warn("scaling3 " , src[ODIMPathElem::ARRAY].data.image.getScaling() );
	// std::cerr << "### updateAttributes"
}



void DataTools::markExcluded(Hi5Tree &src, bool exclude){

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	for (auto & entry: src) {
		//if (it->first.isIndexed()){
		if (!entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			entry.second.data.exclude = exclude;
			markExcluded(entry.second, exclude);
		}
	}

}

// Marks all descendants of src for deletion
void DataTools::markExcluded(Hi5Tree &src, const Hi5Tree::path_t & path, bool exclude){
	//drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	Hi5Tree *ptr = &src; // Rare!
	for (const Hi5Tree::path_t::elem_t & elem: path){
		ptr->data.exclude = exclude;
		ptr = & (*ptr)[elem];
	}
	ptr->data.exclude = exclude;

}


void DataTools::updateCoordinatePolicy(Hi5Tree & src, const drain::image::CoordinatePolicy & policy){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// mout.deprecating("This may be better handled with updateInternalAttributes");
	//mout.warn("Removed: ", __FUNCTION__);
	mout.experimental<LOG_INFO>("Removed code: ", __FUNCTION__);

	return;

	drain::image::Image & data = src.data.image;

	if (!data.isEmpty()){
		data.setCoordinatePolicy(policy);
		//data.setName(data.properties["what:quantity"].toStr() + policy.toStr('_'));
		// mout.attention("image name=", data.getName(), " - ", data.getCoordinatePolicy());
	}

	for (auto & entry: src){
		if (! entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			// mout.special("next updating: ", entry.first);
			updateCoordinatePolicy(entry.second, policy);
		}
	}

	// data.setName(data.properties["what:quantity"].toStr());
	// mout.attention("image name=", data.getName(), " - ", data.getCoordinatePolicy());

}




}  // rack::
