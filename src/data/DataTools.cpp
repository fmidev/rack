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

#include "drain/util/Type.h"

#include "hi5/Hi5.h"
#include "DataTools.h"

namespace rack {

using namespace hi5;

void DataTools::updateInternalAttributes(Hi5Tree & src){

	//src.data.dataSet.properties.clear();
	drain::FlexVariableMap & properties = src.data.dataSet.properties;
	//properties.clear(); // TODO: should not remove linked variables!

	//drain::Variable & object = src[ODIMPathElem::WHAT].data.attributes["object"];
	std::string object = src[ODIMPathElem::WHAT].data.attributes.get("object", "");
	if (object == "PVOL"){
		src.data.dataSet.setCoordinatePolicy(
				drain::image::CoordinatePolicy::POLAR,
				drain::image::CoordinatePolicy::WRAP,
				drain::image::CoordinatePolicy::LIMIT,
				drain::image::CoordinatePolicy::WRAP);
	}
	else {
		src.data.dataSet.setCoordinatePolicy(
				drain::image::CoordinatePolicy::LIMIT,
				drain::image::CoordinatePolicy::LIMIT,
				drain::image::CoordinatePolicy::LIMIT,
				drain::image::CoordinatePolicy::LIMIT);
	}

	updateInternalAttributes(src, properties);
	//updateInternalAttributes(src, drain::FlexVariableMap());
	//updateInternalAttributes(src, drain::VariableMap());
}

void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::FlexVariableMap & parentAttributes){
//void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::VariableMap & attributes){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//std::string path = parentAttributes.get("how:path","");
	/*
	mout.special(// "path: ", parentAttributes.get("how:path",""),
			//" coordPolicy:", parentAttributes["coordPolicy"],
			" my coordPolicy( ", src.data.dataSet.getCoordinatePolicy());
	*/

	/// Write to "hidden" variables of this node (src).
	drain::FlexVariableMap & attributes = src.data.dataSet.properties;
	// mout.special("my coordProp) ", attributes["coordPolicy"]);

	// Init with upper level state
	attributes.importMap(parentAttributes); //, LOG_DEBUG+2);
	// attributes.updateFromCastableMap(parentAttributes);
	// std::cerr << "MAP now: " << a << "\n\n";
	// mout.special("my coordPolicy) ", src.data.dataSet.getCoordinatePolicy());

	/// Step 1: collect local values of \c /what, \c /where, and \c /how groups, overwriting attributes initialised to parent values.
	std::stringstream sstr;
	for (const auto & entry : src){
		if (entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			const drain::VariableMap & a = entry.second.data.attributes;
			for(const auto & e: a){
				sstr.str("");
				sstr << entry.first << ':' << e.first;
				attributes[sstr.str()] = e.second;
				//mout.warn(sstr.str() , '=' , it->second , " ... " , a[sstr.str()] , drain::Type::getTypeChar(it->second.getType()) );
			}
		}
	}

	// Step 2: Traverse other children (recursion)
	for (auto & entry: src){

		if (entry.first.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (!entry.second.data.noSave){
				mout.debug3(entry.first , " => ensure '/data' groups  " );
				entry.second[ODIMPathElem::ARRAY].data.dataSet;
			}
		}
		else if (entry.first.is(ODIMPathElem::ARRAY)){

			drain::image::Image & img = entry.second.data.dataSet;
			if (img.typeIsSet()){
				// Non-standard
				attributes["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
				// Important: ensure Image has (scale,offset) as defined by above WHAT(gain,offset)
				drain::ValueScaling & s = img.getScaling();
				s.set(attributes.get("what:gain", s.scale), attributes.get("what:offset", s.offset));
				// Needed. As a side effect, empty data1.img may get a scaling...
				attributes["scale"]  = s.scale;
				attributes["offset"] = s.offset;
				//mout.warn(a.get("what:quantity", "?") , ", scaling " , img.getScaling() , ' ' , img );
			}

			if (img.getName().empty()){
				img.setName(attributes.get("name",""));
			}
			//mout.warn("scaling1 " , entry.second.data.dataSet.getScaling() );
		}


		// mout.note("considering " , entry.first );
		if (entry.first.belongsTo(ODIMPathElem::DATA_GROUPS | ODIMPathElem::ARRAY)){  // ){//
			//mout.warn("descending to... " , entry.first );
			/*
			a["how:path"] = path;
			a["how:path"] << entry.first;
			mout.special("pathy -> ", a["how:path"]);
			*/
			updateInternalAttributes(entry.second,  attributes); // policy,
		}

		if (entry.first.is(ODIMPathElem::ARRAY)){
			// mout.warn("image ", entry.second.data.dataSet.getName(), " coordPolicy:", entry.second.data.dataSet.getCoordinatePolicy());
			// mout.warn("image ", entry.second.data.dataSet.getName(), " scaling: ",    entry.second.data.dataSet.getScaling());
		}

	}


	// if (src.hasChild(ODIMPathElem::ARRAY))
	//   mout.warn("scaling3 " , src[ODIMPathElem::ARRAY].data.dataSet.getScaling() );
	// std::cerr << "### updateAttributes"
}


void DataTools::markNoSave(Hi5Tree &src, bool noSave){

	// drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	for (auto & entry: src) {
		//if (it->first.isIndexed()){
		if (!entry.first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			entry.second.data.noSave = noSave;
			markNoSave(entry.second, noSave);
		}
	}

}


bool DataTools::removeIfNoSave(Hi5Tree & dst){
	if (dst.data.noSave){
		drain::Logger mout("DataTools", __FUNCTION__);
		mout.note("// about to resetting noSave struct: " , dst.data );
		/*
		dst.data.attributes.clear();
		dst.data.dataSet.resetGeometry();
		dst.getChildren().clear();
		 */
		return true;
	}
	else
		return false;
}

void DataTools::updateCoordinatePolicy(Hi5Tree & src, const drain::image::CoordinatePolicy & policy){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//mout.deprecating("This may be better handled with updateInternalAttributes");

	drain::image::Image & data = src.data.dataSet;

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

/// For drain::TreeUtils dumper()
/**
 *  \return – true, if data "empty", ie. no attributes or data array.
bool DataTools::dataToStream(const Hi5Tree::node_data_t & data, std::ostream &ostr){

	bool empty = true;

	const drain::image::ImageFrame & img = data.dataSet;
	if (!img.isEmpty()){
		ostr << img.getWidth() << ',' << img.getHeight() << ' ';
		ostr << drain::Type::call<drain::compactName>(img.getType());
		ostr << '[' << (8*drain::Type::call<drain::sizeGetter>(img.getType())) << ']';
		//<< drain::Type::call<drain::complexName>(img.getType());
		empty = false;
	}
	// else ...
	char sep = 0;
	for (const auto & key: {"quantity", "date", "time", "src", "elangle", "task_args", "legend"}){
		if (data.attributes.hasKey(key)){
			if (sep)
				ostr << sep << ' ';
			else
				sep = ',';
			ostr << key << '=' << data.attributes[key];
			empty = false;
		}
	}

	return empty;

}
 */




}  // rack::
