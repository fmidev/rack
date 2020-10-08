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

/*
void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::FlexVariableMap & attributes){

        drain::Logger mout(__FILE__, __FUNCTION__);

        /// Write to "hidden" variables of this node (src).
        drain::FlexVariableMap & a = src.data.dataSet.properties;
        a.importMap(attributes);
        //std::cerr << "MAP now: " << a << "\n\n";

        /// Collect values of succeeding /what, /where, and /how groups .
        const std::set<ODIMPathElem> & g = EncodingODIM::attributeGroups;
        std::stringstream sstr;
        for (std::set<ODIMPathElem>::const_iterator git = g.begin(); git != g.end(); ++git){
                if (src.hasChild(*git)){
                        const drain::VariableMap  & groupAttributes = src[*git].data.attributes;
                        for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
                                sstr.str("");
                                sstr << *git << ':' << it->first;
                                a[sstr.str()] = it->second;
                                // if (it->first == "quantity") mout.warn() << "quantity=" << it->second << mout.endl;
                        }
                }
        }

        const bool HAS_DATA = src.hasChild("data");

        if (HAS_DATA){  // move down?
                const drain::image::Image & img = src["data"].data.dataSet;
                //img.setCoordinatePolicy(policy);
                if (img.typeIsSet())
                        a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
        }
        if (HAS_DATA){
                drain::image::Image & img = src["data"].data.dataSet;
                if (img.typeIsSet()){
                        const drain::ValueScaling & s = img.getScaling();
                        img.setScaling(a.get("what:gain", s.scale), a.get("what:offset", s.offset));
                }
        }



        // Traverse children (recursion)

        for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){
                if (! it->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS))
                        //if (g.find(it->first) == g.end())
                        updateInternalAttributes(it->second,  a); // policy,
        }


        // std::cerr << "### updateAttributes"
}
*/

void DataTools::updateInternalAttributes(Hi5Tree & src,  const drain::FlexVariableMap & attributes){

	drain::Logger mout(__FUNCTION__, __FILE__);

	/// Write to "hidden" variables of this node (src).
	drain::FlexVariableMap & a = src.data.dataSet.properties;
	// Init with upper level state
	a.importMap(attributes, false, LOG_DEBUG+2);
	//std::cerr << "MAP now: " << a << "\n\n";

	/// Step 1: collect local values of \c /what, \c /where, and \c /how groups, overwriting previous (upper-level) values.
	std::stringstream sstr;
	for (std::set<ODIMPathElem>::const_iterator git = EncodingODIM::attributeGroups.begin(); git != EncodingODIM::attributeGroups.end(); ++git){

		if (src.hasChild(*git)){

			const drain::VariableMap & groupAttributes = src[*git].data.attributes;
			for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
				sstr.str("");
				sstr << *git << ':' << it->first;
				a[sstr.str()] = it->second;
				//mout.warn() << sstr.str() << '=' << it->second << " ... " << a[sstr.str()] << drain::Type::getTypeChar(it->second.getType()) << mout.endl;
			}

		}

	}

	/*
	if (src.hasChild(ODIMPathElem::ARRAY)){  // move down?

		drain::image::Image & img = src[ODIMPathElem::ARRAY].data.dataSet;
		if (img.typeIsSet()){
			a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
			const drain::ValueScaling & s = img.getScaling();
			img.setScaling(a.get("what:gain", s.scale), a.get("what:offset", s.offset));
			mout.warn() << a.get("what:quantity", "?") << ", scaling " << img.getScaling() << mout.endl;
		}
		mout.warn() << "scaling1 " << src[ODIMPathElem::ARRAY].data.dataSet.getScaling() << mout.endl;
	}
	 */


	// Traverse children (recursion)
	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){

		if (it->first.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (!it->second.data.noSave){
				mout.debug(2) << it->first << " => ensure '/data' groups  " << mout.endl;
				it->second[ODIMPathElem::ARRAY].data.dataSet;
			}
		}
		else if (it->first.is(ODIMPathElem::ARRAY)){

			drain::image::Image & img = it->second.data.dataSet;
			if (img.typeIsSet()){
				a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
				drain::ValueScaling & s = img.getScaling();
				s.setScale(a.get("what:gain", s.scale), a.get("what:offset", s.offset));
				//s.setScale(a["what:gain"], a["what:offset"]);
				// Needed. As a side effect, empty data1.img will get a scaling...
				a["scale"]  = s.scale;
				a["offset"] = s.offset;
				//mout.warn() << a.get("what:quantity", "?") << ", scaling " << img.getScaling() << ' ' << img << mout.endl;
			}
			//mout.warn() << "scaling1 " << it->second.data.dataSet.getScaling() << mout.endl;
		}


		// mout.note() << "considering " << it->first << mout.endl;
		if (it->first.belongsTo(ODIMPathElem::DATA_GROUPS | ODIMPathElem::ARRAY)){  // ){//
			//mout.warn() << "descending to... " << it->first << mout.endl;
			updateInternalAttributes(it->second,  a); // policy,
		}

		if (it->first.is(ODIMPathElem::ARRAY)){
			//mout.warn() << "scaling2 " << it->second.data.dataSet.getScaling() << mout.endl;
		}

	}

	//if (src.hasChild(ODIMPathElem::ARRAY))
		//mout.warn() << "scaling3 " << src[ODIMPathElem::ARRAY].data.dataSet.getScaling() << mout.endl;

	// std::cerr << "### updateAttributes"
}


bool DataTools::removeIfNoSave(Hi5Tree & dst){
	if (dst.data.noSave){
		drain::Logger mout("DataTools", __FUNCTION__);
		mout.note() << "// about to resetting noSave struct: " << dst.data << mout.endl;
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

	drain::Logger mout(__FUNCTION__, __FILE__);

	drain::image::Image & data = src.data.dataSet;
	if (!data.isEmpty()){
		data.setCoordinatePolicy(policy);
		//data.setName(path + ':' + data.properties["what:quantity"].toStr());
		data.setName(data.properties["what:quantity"].toStr());
		//mout.warn() << "qty=" << data.getName() << " - " << data.getCoordinatePolicy() << '\n';
	}

	for (Hi5Tree::iterator it = src.begin(); it != src.end(); ++it){

		if (!it->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			//mout.warn() << "updating " << it->first << '\n';
			updateCoordinatePolicy(it->second, policy);
		}
		else {
		//	mout.warn() << "not updating " << it->first << '\n';
		}
		//if (g.find(key) == g.end()) updateCoordinatePolicy(it->second, policy);
	}
}

}  // rack::
