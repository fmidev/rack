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


#include "EchoClass.h"

namespace rack {


void addClass(drain::JSON::tree_t & tree, const std::string & pathStr, int index, const std::string & en = "", const std::string & color = ""){

	drain::Logger mout("EchoClass", __FUNCTION__);

	mout.debug(1) << "class: '" << pathStr << "'" << mout.endl;

	//const drain::JSON::tree_t::path_t path(pathStr, '.'); // ORIG
	drain::JSON::tree_t & entries = tree["entries"];

	// drain::VariableMap & attr = tree(path).data; // ORIG
	drain::VariableMap & attr = entries[pathStr].data; // NEW (flat)   //entries[pathStr].data; // NEW (flat)
	attr["min"] = index;
	attr["en"] = en;
	attr["color"].setType(typeid(int));

	// Check hierarchical depth
	drain::JSON::tree_t::path_t p(pathStr, '.'); // Note: now split to elements
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

		// drain::JSON::tree_t::path_t p(path); // ORIG
		//drain::JSON::tree_t::path_t p(pathStr, '.'); // Note: now split to elements
		if (!p.empty())
			p.pop_back();

		while (!p.empty()){
			mout.debug(2) << p << mout.endl;
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

classtree_t & getClassTree(){

	//static classtree_t tree('.');
	static classtree_t tree('/'); // Dot used in names, but hierarchy kept flat for easier portability

	if (tree.isEmpty()){

		tree["metadata"].data["title"] = "Echo class";

		// Technical information 0-15

		addClass(tree, "tech", 0, "Unclassified", "0,0,0");
		addClass(tree, "tech.nodata", 1, "No data");  // "0,0,0,255" ?
		addClass(tree, "tech.notarget", 2, "No target"); // Empty echo
		addClass(tree, "tech.quality", 4, "Quality problem");
		addClass(tree, "tech.err", 8, "Data error"); //
		addClass(tree, "tech.err.time", 10, "");     // Delayed data
		addClass(tree, "tech.class", 12, "Classifier issue");
		addClass(tree, "tech.class.reject", 13, "Classifier uncertainty");
		addClass(tree, "tech.class.ambig", 14, "Multiple class");
		addClass(tree, "tech.class.err", 15, "Classifier fatal error");

		// User defined 32-63

		addClass(tree, "user", 32, "User defined");


		// Meteorological echoes

		addClass(tree, "precip", 64, "Hydrometeor", "48,192,96");
		addClass(tree, "precip.rain", 68, "Rain", "64,255,128");
		addClass(tree, "precip.rain.super", 70, "Supercooled rain");
		addClass(tree, "precip.snow", 80, "Snow", "48,96,192");
		addClass(tree, "precip.snow.wet", 84, "Wet snow", "48,160,160");
		addClass(tree, "precip.graupel", 96, "Graupel", "128,255,128");
		addClass(tree, "precip.hail", 112, "Hail", "192,255,128");
		addClass(tree, "precip.hail.wet", 116, "Wet hail");


		/// Non-meteorological echoes

		// Mostly natural
		addClass(tree, "nonmet", 128, "Non-meteorological", "255,192,0");
		addClass(tree, "nonmet.debris", 130, "Debris (sand, dust)");
		addClass(tree, "nonmet.chaff", 132, "Chaff (military)", "240,64,240");
		addClass(tree, "nonmet.clutter", 144, "Clutter", "128,128,128");
		addClass(tree, "nonmet.clutter.ground", 148, "Ground clutter", "208,192,192");
		addClass(tree, "nonmet.clutter.ground.tree", 150, "Tree crowns", "192,208,192");
		addClass(tree, "nonmet.clutter.sea", 152, "Sea clutter", "128,128,224");

		/// Artefacts (human created)
		//  addClass(tree, "nonmet.artef", 160, "Construction, unspecified");
		addClass(tree, "nonmet.craft", 164, "");
		addClass(tree, "nonmet.craft.airplane", 165, "");
		addClass(tree, "nonmet.craft.ship", 166, "");
		addClass(tree, "nonmet.constr", 168, "Mast, unspecied");
		addClass(tree, "nonmet.constr.mast", 169, "");
		addClass(tree, "nonmet.constr.pylon", 170, "");
		addClass(tree, "nonmet.constr.crane", 171, "Chimney");
		addClass(tree, "nonmet.constr.tower", 172, "Pylon");
		addClass(tree, "nonmet.constr.chimney", 173, "Radio mast");
		addClass(tree, "nonmet.constr.building", 174, "");
		addClass(tree, "nonmet.constr.wind", 175, "Wind turbine");
		addClass(tree, "nonmet.biol", 176, "Organic matter");  // plants?
		addClass(tree, "nonmet.biol.bat", 178, "Bat");
		addClass(tree, "nonmet.biol.insect", 180, "Insect");
		// addClass(tree, "nonmet.biol.insect.X XX", 18, "");
		addClass(tree, "nonmet.biol.bird", 184, "Birds");
		addClass(tree, "nonmet.biol.bird.small", 189, "Small birds");
		addClass(tree, "nonmet.biol.bird.large", 190, "Large birds");
		addClass(tree, "nonmet.biol.bird.flock", 191, "Flock of birds");

		/// Distortions: 192 - 223
		addClass(tree, "dist", 192, "Distortions", "255,128,128");
		addClass(tree, "dist.attn", 193, "Attenuation");
		addClass(tree, "dist.attn.rain", 196, "");
		addClass(tree, "dist.attn.radome", 200, "Radome attenuation");
		addClass(tree, "dist.attn.radome.wet", 204, "Wet radome attenuation");
		addClass(tree, "dist.anaprop", 208, "Anaprop");
		addClass(tree, "dist.refl", 212, "Reflection");
		addClass(tree, "dist.second", 216, "Second trip");

		/// Signals: 224 - 255
		addClass(tree, "signal", 224, "External signal", "144,144,144");
		addClass(tree, "signal.noise", 232, "Receiver noise", "192,192,192");
		addClass(tree, "signal.emitter", 240, "External emitter");
		addClass(tree, "signal.emitter.line", 242, "Emitter line or segment");
		addClass(tree, "signal.emitter.sun", 244, "Sun");
		addClass(tree, "signal.emitter.jamming", 246, "Overall contamination");
		/*
		t["met"]         = 64;
		t["met"]["rain"] = 68;
		t["met"]["snow"].data = 72;
		t["met"]["graupel"] = 76;
		t["met"]["hail"] = 82;
		*/
		//unsigned short int i = t["met"]["rain"];
	}

	//t.dump();

	return tree;

}

}  // namespace rack



// Rack
