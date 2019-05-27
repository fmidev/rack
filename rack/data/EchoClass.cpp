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


void addClass(drain::JSON::tree_t & tree, const std::string & path, int index, const std::string & en = "", const std::string & color = "0,128,255"){
	tree(path).data["index"] = index;
	tree(path).data["en"] = en;
	tree(path).data["color"].setType(typeid(int));
	tree(path).data["color"] = color;
}

classtree_t & getClassTree(){

	static classtree_t tree('.');

	if (tree.isEmpty()){

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

		addClass(tree, "met", 64, "Hydrometeor", "0,128,255");
		addClass(tree, "met.rain", 68, "Rain");
		addClass(tree, "met.rain.super", 70, "Supercooled rain");
		addClass(tree, "met.snow", 80, "Snow");
		addClass(tree, "met.snow.wet", 84, "Wet snow");
		addClass(tree, "met.graupel", 96, "Graupel");
		addClass(tree, "met.hail", 112, "Hail");
		addClass(tree, "met.hail.wet", 116, "Wet hail");


		/// Non-meteorological echoes

		// Mostly natural
		addClass(tree, "nonmet", 128, "Non-meteorological");
		addClass(tree, "nonmet.debris", 130, "Debris (sand, dust)");
		addClass(tree, "nonmet.chaff", 132, "Chaff (military)");
		addClass(tree, "nonmet.clutter", 144, "Clutter");
		addClass(tree, "nonmet.clutter.ground", 148, "Ground clutter");
		addClass(tree, "nonmet.clutter.ground.tree", 150, "Tree crowns");
		addClass(tree, "nonmet.clutter.sea", 152, "Sea clutter");

		/// Artefacts (human created)
		//  addClass(tree, "nonmet.artef", 160, "Construction, unspecified");
		addClass(tree, "nonmet.craft", 164, "");
		addClass(tree, "nonmet.craft.airplane", 165, "");
		addClass(tree, "nonmet.craff.ship", 166, "");
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
		addClass(tree, "dist", 192, "Distortions");
		addClass(tree, "dist.attn", 193, "Attenuation");
		addClass(tree, "dist.attn.rain", 196, "");
		addClass(tree, "dist.attn.radome", 200, "Radome attenuation");
		addClass(tree, "dist.attn.radome.wet", 204, "Wet radome attenuation");
		addClass(tree, "dist.anaprop", 208, "Anaprop");
		addClass(tree, "dist.refl", 212, "Reflection");
		addClass(tree, "dist.second", 216, "Second trip");

		/// Signals: 224 - 255
		addClass(tree, "signal", 224, "External signal");
		addClass(tree, "signal.noise", 232, "Receiver noise");
		addClass(tree, "signal.emitter", 240, "External emitter");
		addClass(tree, "signal.emitter.line", 244, "Emitter line or segment");
		addClass(tree, "signal.emitter.jamming", 246, "Overall contamination");
		addClass(tree, "signal.sun", 248, "Sun");
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
