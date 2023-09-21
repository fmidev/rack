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

#include "drain/util/JSON.h"
#include "drain/util/Output.h"
#include "drain/image/Palette.h"

#include "EchoClass.h"

namespace rack {


drain::image::Palette & getClassPalette(){

	static drain::image::Palette palette;

	if (palette.empty()){
		palette =
		#include "palette/palette-ANDRE-CLASS.inc"
		;
		/*
		palette.addEntry(10, 240,240,240, "tech.err.time", "Timing problem");
		palette.addEntry(13, 144,144,144, "tech.class.unclass", "Unclassified");
		palette.addEntry(64, 80,208,80, "precip", "Precipitation");
		palette.addEntry(72, 32,248,96, "precip.widespread", "Rain");
		palette.addEntry(80, 0,208,255, "precip.snow", "Snow");
		palette.addEntry(104, 176,0,255, "precip.conv.graupel", "Graupel");
		palette.addEntry(112, 224,128,255, "precip.conv.hail", "Hail");
		palette.addEntry(128, 255,192,0, "nonmet", "Non-meteorological");
		palette.addEntry(144, 180,96,0, "nonmet.clutter", "Clutter");
		palette.addEntry(148, 160,92,64, "nonmet.clutter.ground", "Ground clutter");
		palette.addEntry(152, 128,128,160, "nonmet.clutter.sea", "Sea clutter");
		palette.addEntry(166, 170,208,255, "nonmet.artef.vessel.ship", "Ship");
		palette.addEntry(176, 255,64,64, "nonmet.biol", "Organic target");
		palette.addEntry(180, 255,248,0, "nonmet.biol.insect", "Insects");
		palette.addEntry(184, 255,0,128, "nonmet.biol.bird", "Birds");
		palette.addEntry(196, 255,224,224, "dist.attn.rain", "Attenuation in rain");
		palette.addEntry(232, 240,240,240, "signal.noise", "Receiver noise");
		palette.addEntry(240, 240,240,128, "signal.emitter", "External emitter");
		palette.addEntry(242, 240,208,224, "signal.emitter.line", "Emitter line or segment");
		palette.addEntry(246, 240,224,208, "signal.emitter.jamming", "Jamming");
		palette.addEntry(248, 255,255,192, "signal.sun", "Sun");
		*/

		/*
		palette.addEntry(13, 144,144,144, "tech.class.reject", "Unclassified");
		palette.addEntry(64, 80,208,80, "precip", "Precipitation");
		palette.addEntry(72, 32,248,96, "precip.widespread", "Rain");
		palette.addEntry(80, 0,208,255, "precip.snow", "Snow");
		palette.addEntry(104, 176,0,255, "precip.conv.graupel", "Graupel");
		palette.addEntry(112, 224,128,255, "precip.conv.hail", "Hail");
		palette.addEntry(128, 255,192,0, "nonmet", "Non-meteorological");
		palette.addEntry(148, 160,92,64, "nonmet.clutter.ground", "Ground clutter");
		palette.addEntry(152, 128,128,160, "nonmet.clutter.sea", "Sea clutter");
		palette.addEntry(166, 170,208,255, "nonmet.artef.vessel.ship", "Ship");
		palette.addEntry(176, 255,64,64, "nonmet.biol", "Organic target");
		palette.addEntry(180, 255,248,0, "nonmet.biol.insect", "Insects");
		palette.addEntry(184, 255,0,128, "nonmet.biol.bird", "Birds");
		palette.addEntry(232, 240,240,240, "signal.noise", "Receiver noise");
		palette.addEntry(240, 240,240,128, "signal.emitter", "External emitter");
		palette.addEntry(246, 240,224,208, "signal.emitter.jamming", "Jamming");
		palette.addEntry(248, 255,255,128, "signal.sun", "Sun");
		 */
		//drain::JSONwriter::toStream(palette);
		//palette.write("EchoClass.txt");

		/*
		for (const auto & s: {"inc", "json","svg", "txt"} ){
			palette.write(std::string("pal-echoClass.") + s);

		}
		*/

		/*
		drain::Output output("pal-echoClass.inc");
		drain::Sprinter::toStream(output, palette, drain::Sprinter::cppLayout);
		*/

	}

	return palette;

}


int getClassCode(const std::string & id){

	drain::Logger mout(__FUNCTION__, __FILE__);

	static drain::image::Palette & palette = getClassPalette();

	return palette.getValueByCode(id);

	/*
	static int counter = 0;

	typedef drain::image::Palette::dict_t dict_t;

	dict_t & dict = palette.dictionary;

	if (!dict.hasValue(id)){
		double d;
		while(counter < 256){
			d = static_cast<double>(counter);
			if (!dict.hasKey(d)){
				dict.add(d, id);
				mout.warn() << "key '" << id << "' was not found in class (palette) dictionary, added it with index=" << counter << mout.endl;
				return counter;
			}
			++counter;
		}
		mout.error() << "could not add entry for '" << id << "', all the indices [0,255] in use" << mout.endl;
	}

	return static_cast<int>(palette.dictionary.getKey(id));
	*/
}


/*
classdict_t & getClassDict(){

	static classdict_t dict;
	if (dict.empty()){
		// fill!
		getClassTree();
	}
	return dict;
}

void addClass(drain::JSONtree::tree_t & tree, const std::string & pathStr, const drain::Variable & code, const std::string & label = "", const std::string & color = ""){

	drain::Logger mout("EchoClass", __FUNCTION__);

	mout.debug2() << "class: '" << pathStr << "'" << mout.endl;


	// NEW (temporary, before legend)
	classdict_t & dict = getClassDict();
	dict.add(code, pathStr);

	// OLD
	//const drain::JSONtree::tree_t::path_t path(pathStr, '.'); // ORIG
	drain::JSONtree::tree_t & entries = tree["entries"];

	drain::VariableMap & attr = entries[pathStr].data;
	attr["value"] = code;
	attr["label"] = label;
	attr["color"].setType(typeid(int));

	// Check hierarchical depth
	drain::JSONtree::tree_t::path_t p(pathStr, '.'); // Note: now split to elements
	attr["hidden"].setType(typeid(bool));
	attr["hidden"] = (p.size() > 2);

	if (!color.empty()){
		attr["color"] = color;
	}
	else {

		if (pathStr.empty()){
			attr["color"] = "128,128,255";
			return;
		}

		attr["color"] = "128,255,128";

		// drain::JSONtree::tree_t::path_t p(path); // ORIG
		//drain::JSONtree::tree_t::path_t p(pathStr, '.'); // Note: now split to elements
		if (!p.empty())
			p.pop_back();

		while (!p.empty()){
			mout.debug3() << p << mout.endl;
			//const drain::Variable & parentColor = tree(p).data["color"];
			const drain::Variable & parentColor = entries[p].data["color"];
			if (!parentColor.isEmpty()){
				attr["color"] = parentColor;
				break;
			}
			p.pop_back();
		}

	}

}
*/

/*
classtree_t & getClassTree(){

	//static classtree_t tree('.');
	static classtree_t tree('/'); // Dot used in names, but hierarchy kept flat for easier portability

	if (tree.isEmpty()){

		tree["metadata"].data["title"] = "Echo class";

		//drain::JSONtree::tree_t & entries = tree["entries"];

		// Technical information 0-15

		addClass(tree, "tech", 			1, "Technical", "240,240,240");

		addClass(tree, "tech.undetect",	"undetect", "Data not detected", "0,0,0");
		addClass(tree, "tech.nodata",   "nodata",   "No data", "255,255,255");
		// addClass(tree, "undetect",	"undetect", "Data not detected", "0,0,0");
		// addClass(tree, "nodata",    "nodata",   "No data", "255,255,255");


		// addClass(tree, "tech.notarget", 2, "No target"); // Empty echo
		addClass(tree, "tech.quality", 	4, "Quality problem");
		addClass(tree, "tech.err", 		8, "Data error"); //
		addClass(tree, "tech.err.time", 10, "");     // Delayed data
		addClass(tree, "tech.class",	12, "Classifier issue");
		addClass(tree, "tech.class.unclass", 13, "Unclassified", "128,128,128"); //  "reject"?
		//addClass(tree, "tech.class.unclass", 16, "Unclassified");
		addClass(tree, "tech.class.ambig", 14, "Multiple class");
		addClass(tree, "tech.class.err", 15, "Classifier fatal error");


		// User defined 32-63
		addClass(tree, "user", 32, "User defined", "64,64,64");


		// Meteorological echoes

		addClass(tree, "precip", 64, "Precipitation", "64,192,64");
		addClass(tree, "precip.widespread", 72, "Rain", "0,255,85");
		addClass(tree, "precip.widespread.rain", 76, "Rain", "48,255,128");
		addClass(tree, "precip.widespread.rain.super", 78, "Supercooled rain", "64,255,164");
		addClass(tree, "precip.snow", 80, "Snow", "0,176,255");
		addClass(tree, "precip.snow.dry", 82, "Snow", "48,128,176");
		addClass(tree, "precip.snow.wet", 84, "Wet snow", "48,160,160");
		addClass(tree, "precip.conv", 96, "Convective", "176,0,255");
		addClass(tree, "precip.conv.graupel", 104, "Graupel");
		addClass(tree, "precip.conv.graupel.dry", 106, "Graupel, dry");
		addClass(tree, "precip.conv.graupel.wet", 108, "Graupel, wet");
		addClass(tree, "precip.conv.hail", 112, "Hail", "224,128,255");
		addClass(tree, "precip.conv.hail.dry", 114, "Hail, dry");
		addClass(tree, "precip.conv.hail.wet", 116, "Hail, wet");


		/// Non-meteorological echoes

		// Mostly natural
		addClass(tree, "nonmet", 128, "Non-meteorological", "255,110,0");
		addClass(tree, "nonmet.debris", 130, "Debris", "240,170,64");
		addClass(tree, "nonmet.chaff", 132, "Chaff", "240,64,120");
		addClass(tree, "nonmet.clutter", 144, "Clutter", "144,128,128");
		addClass(tree, "nonmet.clutter.ground", 148, "Ground clutter", "144,128,64");
		addClass(tree, "nonmet.clutter.ground.tree", 150, "Tree crowns", "128,144,128");
		addClass(tree, "nonmet.clutter.sea", 152, "Sea clutter", "128,128,160");

		/// Artefacts (human created)
		addClass(tree, "nonmet.artef", 160, "Artefact", "255,192,0");
		addClass(tree, "nonmet.artef.vessel", 164, "Vessel");
		addClass(tree, "nonmet.artef.vessel.aircraft", 165, "Aircraft");
		addClass(tree, "nonmet.artef.vessel.ship", 166, "Ship");
		addClass(tree, "nonmet.artef.constr", 168, "Construction", "160,160,160");
		addClass(tree, "nonmet.artef.constr.mast", 169, "Mast");
		addClass(tree, "nonmet.artef.constr.pylon", 170, "Pylon");
		addClass(tree, "nonmet.artef.constr.crane", 171, "Chimney");
		addClass(tree, "nonmet.artef.constr.tower", 172, "Tower");
		addClass(tree, "nonmet.artef.constr.chimney", 173, "Chimney");
		addClass(tree, "nonmet.artef.constr.building", 174, "Building");
		addClass(tree, "nonmet.artef.constr.wind", 175, "Wind turbine");

		addClass(tree, "nonmet.biol", 176, "Organic target", "255,128,248");
		addClass(tree, "nonmet.biol.bat", 178, "Bat");
		addClass(tree, "nonmet.biol.insect", 180, "Insect", "255,0,240");
		addClass(tree, "nonmet.biol.bird", 184, "Birds", "255,0,240");
		addClass(tree, "nonmet.biol.bird.small", 189, "Small birds");
		addClass(tree, "nonmet.biol.bird.large", 190, "Large birds");
		addClass(tree, "nonmet.biol.bird.flock", 191, "Flock of birds");

		/// Distortions: 192 - 223
		addClass(tree, "dist", 192, "Distortions", "255,128,128");
		addClass(tree, "dist.attn", 193, "Attenuation", "255,160,160");
		addClass(tree, "dist.attn.rain", 196, "");
		addClass(tree, "dist.attn.radome", 200, "Radome attenuation");
		addClass(tree, "dist.attn.radome.wet", 204, "Wet radome attenuation");
		addClass(tree, "dist.anaprop", 208, "Anaprop");
		addClass(tree, "dist.refl", 212, "Reflection");
		addClass(tree, "dist.second", 216, "Second trip");

		/// Signals: 224 - 255
		addClass(tree, "signal", 224, "External signal", "180,180,180");
		addClass(tree, "signal.noise", 232, "Receiver noise", "240,255,128");
		addClass(tree, "signal.emitter", 240, "External emitter", "240,240,160");
		addClass(tree, "signal.emitter.line", 242, "Emitter line or segment", "240,240,192");
		addClass(tree, "signal.emitter.jamming", 246, "Overall contamination", "255,240,192");
		addClass(tree, "signal.sun", 248, "Sun", "255,255,192");
		//unsigned short int i = t["met"]["rain"];

	}

	//t.dump();

	return tree;

}
*/

/*
int getClassCode(const std::string & key){

	drain::Logger mout(__FUNCTION__, __FILE__);

	classtree_t &t = getClassTree()["entries"];

	classtree_t::path_t path(key, t.getSeparator());
	mout.debug() << "path(" << path.separator << ") "<< drain::StringTools::join(path, path.separator) << mout.endl;

	return getClassCode(t, path.begin(), path.end());

}


int getClassCode(classtree_t & tr, classtree_t::path_t::const_iterator it, classtree_t::path_t::const_iterator eit){


	drain::Logger mout(__FUNCTION__, __FILE__);

	if (it == eit){ // "empty path"
		if (!tr.data.hasKey("value")){
			mout.note() << "missing 'value' attribute (index) of existing class " << tr.data << mout.endl; // ddificult to locate, try tr.dump()
		}
		return tr.data["value"];
	}

	const classtree_t::path_t::value_type & key = *it;
	//mout.note() << "entered " << key << mout.endl;

	if (!tr.hasChild(key)){
		static unsigned short counter(32);
		mout.note() << "creating class code: *." << *it << ' ' << counter << mout.endl;
		//tr.getPaths()
		tr[key].data["value"] = counter;
		++counter;
	}
	else {
		mout.info() << "existing class code: *." << *it << '(' << tr[key].data.getValues() << ')' << mout.endl;
	}

	//mout.note() << "descending to " << *it << mout.endl;
	return getClassCode(tr[key], ++it, eit);

}
*/
/*
#define ECHO_CLASS_PRECIP	4
#define ECHO_CLASS_HAIL	8
#define ECHO_CLASS_WET_HAIL	10
#define ECHO_CLASS_GRAUPEL	12
#define ECHO_CLASS_SNOW	16
#define ECHO_CLASS_WET_SNOW	18
#define ECHO_CLASS_RAIN	32
#define ECHO_CLASS_SUPERCOOLED_RAIN	36
#define ECHO_CLASS_DUST	56
#define ECHO_CLASS_CHAFF	58
#define ECHO_CLASS_INSECT	60
#define ECHO_CLASS_CLUTTER	64
#define ECHO_CLASS_MAST	84
#define ECHO_CLASS_VEHICLE	92
#define ECHO_CLASS_BIRD	96
#define ECHO_CLASS_BAT	104
#define ECHO_CLASS_ATTENUATED	192
#define ECHO_CLASS_SPECULAR	200
#define ECHO_CLASS_FLARE_ECHO	208
#define ECHO_CLASS_SECOND_TRIP	216
#define ECHO_CLASS_SUN	224
#define ECHO_CLASS_EMITTER	240
#define ECHO_CLASS_JAMMING	242
#define ECHO_CLASS_NOISE	244
#define ECHO_CLASS_DELAY	248
*/


}  // namespace rack



// Rack
