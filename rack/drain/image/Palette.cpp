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

#include <sstream>
#include <list>

#include "drain/util/FilePath.h"
#include "drain/util/StringMapper.h"
//
#include "drain/util/Output.h"

#include "Palette.h"

namespace drain
{

namespace image
{

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */


PaletteEntry::PaletteEntry(): BeanLike(__FUNCTION__){
	init();
}

PaletteEntry::PaletteEntry(const PaletteEntry & entry): BeanLike(__FUNCTION__){
	init();
	parameters.importMap(entry.getParameters());
	color = entry.color;
}


void PaletteEntry::init(){
	color.resize(1, 0);
	//color[3] = 255.0;
	parameters.link("value", value = 0.0);
	//parameters.link("color", color);
	parameters.link("alpha", alpha = 255.0);
	parameters.link("label", label);
	parameters.link("hidden", hidden=false);
	parameters.link("id", id);
}

void Palette::addEntry(double min, double red, double green, double blue, const std::string & id, const std::string & label){

	PaletteEntry entry = (*this)[min];
	entry.value = min;
	entry.color.resize(3);
	entry.color[0] = red;
	entry.color[1] = green;
	entry.color[2] = blue;
	entry.id = id;
	entry.label = label;

	dictionary.add(min, id);

}

// Alpha check
void PaletteEntry::checkAlpha(){

	const size_t s = color.size();

	switch (s){
		case 4:
		case 2:
			alpha = color[s-1];
			color.resize(s-1);
			break;
		default:
			break;
	}

}

void PaletteEntry::getHexColor(std::ostream & ostr) const {

	for (vect_t::const_iterator it = color.begin(); it!=color.end(); ++it){
		ostr.width(2);
		ostr.fill('0');
		ostr << std::hex << static_cast<int>(*it);
	}

}

std::ostream & PaletteEntry::toOStream(std::ostream &ostr, char separator, char separator2) const{

		if (!separator2)
			separator2 = separator;

		//ostr << "# " << label << "\n";
		if (std::isnan(value)){
			//ostr << "# " << id << "\n";
			ostr << '@' << separator;
		}
		else {
			ostr << value << separator;
		}

		// ostr << separator;
		for (vect_t::const_iterator it=color.begin(); it != color.end(); ++it){
			ostr << *it << separator2;
		}

		// alpha (optional)
		if (alpha < 255.0) // NOTE: can be 255 in alpha image also...
			ostr << alpha;
		//if (color.size()==4)
		//	if (color[3] != 255.0)
		//		ostr << color[3];

		return ostr;
}


void Palette::reset(){

	clear();
	specialCodes.clear();
	channels.setChannelCount(0, 0);

}

void Palette::update() const {

	Logger mout(__FUNCTION__, __FILE__);

	size_t i = 0;
	size_t a = 0;

	for (std::map<std::string,PaletteEntry >::const_iterator it = specialCodes.begin(); it != specialCodes.end(); ++it){

		const PaletteEntry & entry = it->second;

		if (entry.color.size() > i)
			i = entry.color.size();

		if (entry.alpha < 255.0)
			a = 1;

	}

	for (Palette::const_iterator it = begin(); it != end(); ++it){

		const PaletteEntry & entry = it->second;

		//mout.note() << " e" << it->first << '\t' << entry.color.size() << mout.endl;

		if (entry.color.size() > i)
			i = entry.color.size();

		if (entry.alpha < 255.0)
			a = 1;

	}

	channels.setChannelCount(i, a);

}


void Palette::load(const std::string & filename, bool flexible){

	//Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL __FUNCTION__, __FILE__);
	Logger mout(__FUNCTION__, __FILE__); //REPL __FUNCTION__, __FILE__);

	drain::FilePath filePath;

	static const drain::RegExp labelRE("^[A-Z]+[A-Z0-9_\\-]*$");

	if (labelRE.test(filename)){
		filePath.set(std::string("palette-") + filename + std::string(".json"));
		mout.info() << " extending path: " << filename << " => " << filePath << mout.endl;
	}
	else {
		mout.info() << " setting filepath: " << filename << mout.endl;
		filePath.set(filename);
	}

	mout.debug() << " Initial dir path: '" << filePath.dir << "'" << mout.endl;

	// If relative path, add explicit ./
	if (filePath.dir.empty())
		filePath.dir.push_front(".");

	mout.debug() << " Initial file path: " << filePath.str() << mout.endl;

	const std::string s = filePath.str();


	std::ifstream ifstr;
	ifstr.open(s.c_str(), std::ios::in);

	if (ifstr.good()){

		mout.note() << "reading: " << filePath.str() << mout.endl;

		if (filePath.extension == "txt"){
			loadTXT(ifstr);
		}
		else if (filePath.extension == "json"){
			loadJSON(ifstr);
		}
		else {
			ifstr.close();
			mout.error() << "unknown file type: " << filePath.extension << mout.endl;
			return;
		}

	}
	else {

		ifstr.close();

		if (!flexible){
			//ifstr.close();
			mout.error() << " opening file '" << filename << "' failed" << mout.endl;
			return;
		}
		else {

			// Prepare to search for  txt, json, ini files.
			std::list<std::string> extensions;
			extensions.push_back(filePath.extension);  // default

			if (filePath.extension == "txt"){
				extensions.push_back("json");
				// extensions.push_back("ini");
			}
			else if (filePath.extension == "json"){
				// extensions.push_back("ini");
				extensions.push_back("txt");
			}
			else if (filePath.extension == "ini"){
				extensions.push_back("json");
				extensions.push_back("txt");
			}
			else {
				mout.error() << " unsupported palette file type: '" << filePath.extension << "', filename='" << filename << "'" << mout.endl;
				return;
			}

			// Prepare to search additional palette/ dir
			drain::FilePath::path_t::list_t paths;
			paths.push_back(filePath.dir);
			if (filePath.dir.back() != "palette"){
				// Add /<dir>/palette
				paths.push_back(filePath.dir);
				paths.back().appendElem("palette");  // subdir
			}

			drain::FilePath finalFilePath;
			finalFilePath.basename = filePath.basename;
			for (drain::FilePath::path_t::list_t::const_iterator pit = paths.begin(); pit!=paths.end(); ++pit){
				for (std::list<std::string>::const_iterator eit = extensions.begin(); eit!=extensions.end(); ++eit){
					finalFilePath.dir = *pit;
					finalFilePath.extension = *eit;
					mout.info() << "trying... " << finalFilePath << mout.endl;
					mout.info() << "a.k.a.... " << finalFilePath.str().c_str() << mout.endl;
					ifstr.open(finalFilePath.str().c_str(), std::ios::in);
					//if (ifstr.good())
					if (ifstr.is_open())
						break;
				}
				if (ifstr.is_open())
					break;
			}

			if (!ifstr.good()){  // still not good
				ifstr.close();
				mout.error() << " opening file failed" << mout.endl;
				return;
			}

			mout.note() << "reading: " << finalFilePath.str() << mout.endl;

			reset();

			if (finalFilePath.extension == "txt"){
				loadTXT(ifstr);
			}
			else if (finalFilePath.extension == "json"){
				loadJSON(ifstr);
			}
			else {
				ifstr.close();
				mout.error() << "unknown file type: " << finalFilePath.extension << mout.endl;
			}

		}
	};

	ifstr.close();

	updateDictionary();


}


void Palette::loadTXT(std::ifstream & ifstr){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	/*
	if (!ifstr.good()){
		mout.error() << " reading file failed" << mout.endl;
	};
	*/

	reset();

	double d;

	bool SPECIAL;
	std::string line;
	std::string label;

	int index = 100;
	Variable id;
	id.setType(typeid(std::string));
	//id.setSeparator('-');

	while (std::getline(ifstr, line)){

		//mout.note() << "'" << line <<  "'" << std::endl;
		size_t i = line.find_first_not_of(" \t");

		// Skip empty lines
		if (i == std::string::npos)
			continue;

		char c = line.at(i);

		if (c == '#'){
			// Re-search
			++i;
			i = line.find_first_not_of(" \t", i);
			if (i == std::string::npos) // line contains only '#'
				continue;
			//mout.note() << "i=" << i << mout.endl;

			size_t j = line.find_last_not_of(" \t");
			if (j == std::string::npos) // when?
				continue;
			//mout.note() << "j=" << j << mout.endl;

			label = line.substr(i, j+1-i);

			/// First label in the file will be the title
			if (title.empty())
				title = label;

			continue;
		}

		if (c == '@'){
			SPECIAL = true;
			++i;
			mout.debug(3) << "reading special entry[" << label << "]" << mout.endl;
		}
		else {
			SPECIAL = false;
			i = 0;
		}

		//mout.note() << "remaining line '" << line.substr(i) << "'" << mout.endl;

		std::istringstream data(line.substr(i));
		if (SPECIAL){
			id = NAN;
		}
		else {
			if (!(data >> d)){
				mout.warn() << "suspicious line: " << line << mout.endl;
				continue;
			}
			mout.debug(4) << "reading  entry[" << d << "]" << mout.endl;
		}


		PaletteEntry & entry = SPECIAL ? specialCodes[label] : (*this)[d]; // Create entry?

		entry.value = d;
		if (!label.empty()){
			if (label.at(0) == '.'){
				entry.hidden = true;
				entry.label = label.substr(1);
			}
			else {
				entry.hidden = false;
				entry.label = label;
			}
		}

		id = "";
		id << ++index;
		id << entry.label;
		entry.id = id.toStr();

		Variable colours(typeid(PaletteEntry::value_t));

		while (data >> d) {
			colours << d;
		}
		colours.toSequence(entry.color);
		entry.checkAlpha();

		mout.note() << "got " << colours.getElementCount() << '/' << entry.color.size() << " colours" << mout.endl;

		// TODO!
		// if (entry.color.size() == 4)
		//  alpha=

		mout.debug3() << entry.label << '\t' << entry << mout.endl;

		label.clear();

	}

	ifstr.close(); // ?

}


void Palette::loadJSON(std::ifstream & ifstr){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	reset();

	drain::JSONtree::tree_t json;

	drain::JSONtree::readTree(json, ifstr);
	//drain::JSONtree::read(json, ifstr);

	const drain::JSONtree::node_t & metadata = json["metadata"].data;

	// Consider whole metadata as JSON tree?
	title = metadata["title"].toStr();

	mout.info() << "title: " << title << mout.endl;
	mout.debug() << "metadata: " << metadata << mout.endl;

	importJSON(json["entries"], 0);

}


void Palette::importJSON(const drain::JSONtree::tree_t & entries, int depth){

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	//const drain::JSONtree::tree_t & entries = json["entries"];

	for (drain::JSONtree::tree_t::const_iterator it = entries.begin(); it != entries.end(); ++it){

		//const std::string & id         = it->first;
		const drain::JSONtree::tree_t::key_t & id = it->first;
		const drain::JSONtree::tree_t & child     = it->second;

		const VariableMap & attr  = child.data;

		const Variable &value = attr["value"];

		// Special code: dynamically assigned value
		const bool SPECIAL = value.isString();
		const double d = value; // possibly NaN

		if (SPECIAL){
			mout.debug() << "special code: '" << value << "', id=" << id << mout.endl;
		}

		//PaletteEntry & entry = SPECIAL ? specialCodes[id] : (*this)[d]; // Create entry?
		PaletteEntry & entry = SPECIAL ? specialCodes[attr["value"]] : (*this)[d]; // Create entry?

		entry.id = id;
		// Deprecated, transitory:
		entry.value = attr["min"];
		entry.label = attr["en"].toStr();

		entry.getParameters().updateFromCastableMap(attr);

		attr["color"].toSequence(entry.color);
		entry.checkAlpha();

		if (SPECIAL){
			entry.value = NAN;
		}

		mout.debug() << "entry: " << id << ':' << entry.getParameters() << mout.endl;

		importJSON(child, depth + 1); // recursion not in use, currently
	}
}

void Palette::updateDictionary(){

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	this->dictionary.clear();

	for (Palette::const_iterator it = begin(); it != end(); ++it){
		mout.info() << "add " << it->first << ' ' << it->second.label << mout.endl;
		this->dictionary.add(it->first, it->second.id);
	}


}

void Palette::write(const std::string & filename){

	Logger mout(getImgLog(), __FUNCTION__, __FILE__); //REPL __FUNCTION__, __FILE__);

	drain::FilePath filepath(filename);

	if ((filepath.extension != "txt") && (filepath.extension != "json") &&
			(filepath.extension != "svg") && (filepath.extension != "pal")){
		mout.error() << "unknown file type: " << filepath.extension << mout.endl;
		return;
	}

	mout.info() << "kokeilu: " << *this << mout.endl;

	drain::Output ofstr(filename.c_str());

	/*
	std::ofstream ofstr(filename.c_str(), std::ios::out);
	if (!ofstr.good()){
		ofstr.close();
		mout.error() << "could not open file: " << filename << mout.endl;
		return;
	}
	*/


	if (filepath.extension == "svg"){
		mout.debug() << "writing SVG legend" << mout.endl;
		TreeSVG svg;
		exportSVGLegend(svg, true);
		ofstr << svg;
	}
	else if (filepath.extension == "json"){
		mout.debug() << "writing JSON palette/class file" << mout.endl;
		drain::JSONtree::tree_t json;
		exportJSON(json);
		drain::JSONwriter::toStream(json, ofstr);
		// = drain::JSONwriter::mapToStream(json, ofstr); explicit
		// = drain::JSONtree::writeJSON(json, ofstr);     old style
	}
	else if (filepath.extension == "txt"){
		mout.debug() << "writing plain txt palette file" << mout.endl;
		exportTXT(ofstr,'\t', '\t');
	}
	else if (filepath.extension == "pal"){
		mout.debug() << "writing formatted .pal file" << mout.endl;
		exportFMT(ofstr, "# ${label}\nset style line ${value} lt rgb '#${colorHex}' lw 5  \n");
		//exportFMT(ofstr, "#${label}\n set color ${color}\n");
	}
	else {
		//ofstr.close();
		mout.error() << "unknown file type: " << filepath.extension << mout.endl;
	}

	//ofstr.close();
}

void Palette::exportTXT(std::ostream & ostr, char separator, char separator2) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	if (!separator2)
		separator2 = separator;

	ostr << "# " << title << "\n";
	ostr << '\n';

	if (!specialCodes.empty()){
		for (std::map<std::string,PaletteEntry >::const_iterator it = specialCodes.begin(); it != specialCodes.end(); ++it){
			// ostr << it->second << '\n';

			ostr << '#' << it->second.label << '\n';
			ostr << '#' << it->first << '\n';
			it->second.toOStream(ostr, separator, separator2);
			ostr << '\n';
			ostr << '\n';
		}
		ostr << '\n';
		ostr << '\n';
	}

	for (Palette::const_iterator it = begin(); it != end(); ++it){
		//ostr << it->first << separator << it->second << '\n';
		ostr << '#';

		if (it->second.hidden)
			ostr << '.';

		if (!it->second.label.empty())
			ostr << it->second.label;
		else
			ostr << '?'; // << it->second.id;

		ostr << '\n';

		it->second.toOStream(ostr, separator, separator2);
		ostr << '\n';
		ostr << '\n';
	}

}


void Palette::exportJSON(drain::JSONtree::tree_t & json) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);


	VariableMap &metadata = json["metadata"].data;
	metadata["title"] = title;

	drain::JSONtree::tree_t & entries =  json["entries"];

	int i = 0;
	std::stringstream key;

	// Start with special codes (nodata, undetect)
	for (spec_t::const_iterator it = specialCodes.begin(); it!=specialCodes.end(); ++it){

		const PaletteEntry & entry = it->second;

		key.str("");
		key << "special";
		key.width(2);
		key.fill('0');
		key << ++i;

		drain::JSONtree::tree_t & js = entries[key.str()]; // entries[entry.id];
		js.data["color"] = entry.color;
		js.data.importCastableMap(entry.getParameters());
		Variable & value = js.data["value"];
		value.reset();
		value = it->first;
	}

	i = 0;
	for (Palette::const_iterator it = begin(); it!=end(); ++it){
		const PaletteEntry & entry = it->second;
		key.str("");
		key << "colour";
		key.width(3);
		key.fill('0');
		key << ++i;
		VariableMap & m = entries[key.str()].data; // entries[entry.id].data;
		m["color"] = entry.color;
		m.importCastableMap(entry.getParameters());
	}



}

/*
struct PalEntry : BeanLike {

	HistEntry() : BeanLike(__FUNCTION__), index(0), count(0){
		parameters.link("index", index);
		parameters.link("min", binRange.min);
		parameters.link("max", binRange.max);
		parameters.link("count", count);
		parameters.link("label", label);
	};

	drain::Histogram::vect_t::size_type index;
	Range<double> binRange;
	drain::Histogram::count_t count;
	std::string label;

};
*/

void Palette::exportFMT(std::ostream & ostr, const std::string & format) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	PaletteEntry entry;
	entry.id = "test";
	entry.color.resize(3);
	entry.getParameters().link("color", entry.color);

	std::string colorHex;
	entry.getParameters().link("colorHex", colorHex);
	/*
	drain::ReferenceMap entryWrapper;
	//entryWrapper
	entryWrapper.link("label", entry.label = "MIKA");
	entryWrapper.link("color", entry.color);
	entryWrapper.link("colorHex", colorHex);
	entryWrapper.link("value", entry.value = 123.566);
	entryWrapper["color"].setOutputSeparator(',');
	*/

	//mout.warn() << "color: " << entryWrapper["color"] << mout.endl;
	//entryWrapper["color"].toJSON();

	drain::StringMapper mapper;
	mapper.parse(format, true);

	/*
	if (!specialCodes.empty()){

		for (std::map<std::string,PaletteEntry >::const_iterator it = specialCodes.begin(); it != specialCodes.end(); ++it){
			entry = it->second;
			entry.label = it->first; // ?
			mapper.toStream(ostr, entryWrapper);
		}
		ostr << '\n';
		ostr << '\n';
	}
	*/

	for (Palette::const_iterator it = begin(); it != end(); ++it){
		if (!it->second.hidden){
			mout.warn() << it->second.id << mout.endl;
			//entry.map.append(it->second.map, true);
			entry = it->second; // if color.size() != 3 ??
			//entry.id = "koe";
			entry.getParameters().link("color", entry.color = it->second.color); // relocate
			entry.getParameters()["color"].setSeparator(',');
			//entry.color = it->second.color;
			entry.getHexColor(colorHex);
			mapper.toStream(ostr, entry.getParameters());
		}
	}
	ostr << '\n';
	ostr << '\n';

}



void Palette::refine(size_t n){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	if (n==0)
		n = refinement;

	if (n==0){
		mout.unimplemented("zero refinement; no defaults (like 256) defined") << mout.endl;
		return;
	}


	Palette::iterator it = begin();
	if (it == end()){
		mout.warn() << "empty palette" << mout.endl;
		return;
	}

	Palette::iterator it0 = it;
	++it;
	if (it == end()){
		mout.warn() << "single entry palette, skipping" << mout.endl;
		return;
	}

	if (n < size()){
		mout.warn() << "contains already" << size() << " elements " << mout.endl;
		return;
	}

	/// Number of colors.
	const size_t s = it->second.color.size();

	/// New number of entries.
	const size_t n2 = n / (size()-1);
	const float e = 1.0f / static_cast<float>(n2);
	float c;
	while (it != end()){

		const double & f  = it->first;
		PaletteEntry & p  = it->second;

		const double & f0 = it0->first;
		PaletteEntry & p0 = it0->second;

		//std::cout << '-' << p0 << "\n>" << p << std::endl;
		for (size_t i = 1; i < n2; ++i) {
			c = e * static_cast<float>(i);
			PaletteEntry & pNew = (*this)[0.01f * round(100.0*(f0 + c*(f-f0)))];
			pNew.color.resize(s);
			for (size_t j = 0; j < s; ++j) {
				pNew.color[j] = c*p.color[j] + (1.0f-c)*p0.color[j];
			}
		}
		it0 = it;
		++it;
	}

}

void Palette::exportSVGLegend(TreeSVG & svg, bool up) const {

	const int headerHeight = 30;
	const int lineheight = 20;

	svg->setType(NodeSVG::SVG);
	// TODO font-size:  font-face:

	TreeSVG & header = svg["header"];
	header->setType(NodeSVG::TEXT);
	header->set("x", lineheight/4);
	header->set("y", (headerHeight * 9) / 10);
	header->ctext = title;
	header->set("style","font-size:20");

	TreeSVG & background = svg["bg"];
	background->setType(NodeSVG::RECT);
	background->set("x", 0);
	background->set("y", 0);
	//background->set("style", "fill:white opacity:0.8"); // not supported by some SVG renderers
	background->set("fill", "white");
	background->set("opacity", 0.8);
	// width and height are set in the end (element slot reserved here,for rendering order)

	int index = 0;
	int width = 150;
	//int height = (1.5 + size() + specialCodes.size()) * lineheight;

	int y;
	std::stringstream name;
	std::stringstream style;
	std::stringstream threshold;

	Palette::const_iterator it = begin();
	std::map<std::string,PaletteEntry >::const_iterator itSpecial = specialCodes.begin();

	while ((it != end()) || (itSpecial != specialCodes.end())){

		bool specialEntries = (itSpecial != specialCodes.end());

		const PaletteEntry & entry = specialEntries ? itSpecial->second : it->second;

		if (!entry.hidden){

			name.str("");
			name << "color";
			name.fill('0');
			name.width(2);
			name << index;
			TreeSVG & child = svg[name.str()];
			child->setType(NodeSVG::GROUP);

			TreeSVG & t = child["title"];
			t->setType(NodeSVG::TITLE);
			// t->ctext =
			t->ctext = name.str(); // + threshold.str();

			//y = up ? height - (index+1) * lineheight : index * lineheight;
			y = headerHeight + index*lineheight;

			TreeSVG & rect = child["r"];
			rect->setType(NodeSVG::RECT);
			rect->set("x", 0);
			rect->set("y", y);
			rect->set("width", lineheight*1.7);
			rect->set("height", lineheight);
			//child["r"]->set("style", "fill:green");



			style.str("");
			const PaletteEntry::vect_t & color = entry.color;
			switch (color.size()){
			case 4:
				if (color[3] != 255.0)
					style << "fill-opacity:" << color[3]/255.0 << ';';
				// no break
			case 3:
				style << "fill:rgb(" << color[0] << ',' << color[1] << ',' << color[2] << ");";
				break;
			case 1:
				style << "fill:rgb(" << color[0] << ',' << color[0] << ',' << color[0] << ");";
				break;
			}
			rect->set("style", style.str());

			TreeSVG & thr = child["th"];
			thr->setType(NodeSVG::TEXT);
			threshold.str("");
			if (!specialEntries)
				threshold << it->first;

			thr->ctext = threshold.str();
			thr->set("text-anchor", "end");
			thr->set("x", lineheight*1.5);
			thr->set("y", y + lineheight-1);

			TreeSVG & text = child["t"];
			text->setType(NodeSVG::TEXT);
			text->ctext = entry.label;
			text->set("x", 2*lineheight);
			text->set("y", y + lineheight-1);

			t->ctext += ' ';
			t->ctext += threshold.str();
			t->ctext += ' ';
			t->ctext += entry.label;

			//ostr << it->first << ':';
			++index;

		}

		if (specialEntries)
			++itSpecial;
		else
			++it;
		//ostr << '\n';
	}

	const int height = headerHeight + index*lineheight;

	background->set("width", width);
	background->set("height", height);

	svg->set("width", width);
	svg->set("height", height);

}

/*
std::ostream & operator<<(std::ostream &ostr, const Palette & p){

	ostr << "Palette '" << p.title << "'\n";
	for (std::map<std::string,PaletteEntry >::const_iterator it = p.specialCodes.begin(); it != p.specialCodes.end(); ++it){
		// ostr << it->second << '\n';
		ostr << '#' << it->first << ':' << '\t' << it->second << '\n';
	}
	for (Palette::const_iterator it = p.begin(); it != p.end(); ++it){
		ostr << it->first << ':' << '\t' << it->second << '\n';
	}
	return ostr;
}
*/


/// Creates a gray palette ie. "identity mapping" from gray (x) to rgb (x,x,x).
// TODO T 256, T2 32768
/*
void PaletteOp::setGrayPalette(unsigned int iChannels,unsigned int aChannels,float brightness,float contrast) const {

	const unsigned int colors = 256;
	const unsigned int channels = iChannels+aChannels;

	paletteImage.setGeometry(colors,1,iChannels,aChannels);

	int g;
	//const float origin = (Intensity::min<int>()+Intensity::max<int>())/2.0;

	for (unsigned int i = 0; i < colors; ++i) {

		g = paletteImage.limit<int>( contrast*(static_cast<float>(i)) + brightness);
		for (unsigned int k = 0; k < iChannels; ++k)
			paletteImage.put(i,0,k, g);

		for (unsigned int k = iChannels; k < channels; ++k)
			paletteImage.put(i,0,k,255);  // TODO 16 bit?
	}
}
 */


}

}

// Drain
