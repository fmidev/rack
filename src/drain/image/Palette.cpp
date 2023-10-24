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

// #include "drain/util/FilePath.h"
#include "drain/util/Input.h"
#include "drain/util/StringMapper.h"
//
#include "drain/util/Output.h"

#include "Palette.h"

namespace drain
{

namespace image
{

const SprinterLayout Palette::cppLayout2("{,}", "{,}", "{,}", "\"\"", "");

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */


// important!
Palette::Palette(std::initializer_list<std::pair<Variable, PaletteEntry> > inits): ImageCodeMap<PaletteEntry>(), refinement(0){

	for (const auto & entry: inits){
		if (entry.first.isString()){
			if (entry.first.toStr() == "title"){
				title = entry.second.label;
			}
			else {
				specialCodes[entry.first] = entry.second;
			}
			// std::cout << "special/";
		}
		else {
			(*this)[entry.first] = entry.second;
		}
		// std::cout << entry.first << '|' << entry.first.getType().name();
		// std::cout << '=' << sprinter(entry.second, Sprinter::cppLayout) << '\n';
	}

	comment = "Built-in";  // = Initialized in code

	/*
	std::cout << "cppLayout: --- "<< '\n';
	Sprinter::toStream(std::cout, *this, Sprinter::cppLayout);

	static const SprinterLayout cppLayout2("{,}", "{,}", "{,}", "\"\"", "");
	std::cout << "cppLayout2: --- "<< '\n';
	Sprinter::toStream(std::cout, *this, cppLayout2);
	*/
};

void Palette::addEntry(double min, double red, double green, double blue, const std::string & id, const std::string & label){

	Logger mout(__FUNCTION__, __FILE__);

	//mout.warn("probably faulty code");

	// 2023/01 Fixed to reference:
	PaletteEntry & entry = (*this)[min];
	// entry.value = min; NEW
	// NOVECT entry.color.resize(3);
	entry.color[0] = red;
	entry.color[1] = green;
	entry.color[2] = blue;
	entry.code = id;
	entry.label = label;

	dictionary.add(min, id);

}

Palette::value_type & Palette::getEntryByCode(const std::string & code, bool lenient){

	Logger mout(__FUNCTION__, __FILE__);

	for (auto & entry: *this){
		if (entry.second.code == code){
			return entry;
		}
	}

	if (!lenient){
		mout.advice("Use lenient=true if partial string sufficient");
		mout.error("could not find code: '", code, "' (with exact match)");
		//throw std::runtime_error(drain::StringBuilder(__FILE__, __FUNCTION__, ": could not find code: ", code));
		//return NAN;
	}

	std::string code_lc(code);
	for (char & c: code_lc){
		if ((c>='A') && (c<='Z'))
			c = (c-'A') + 'a';
	}

	if (code_lc != code){
		mout.debug("matching failed with '", code, "', trying with lowercase: '", code_lc, "'");
		for (auto & entry: *this){
			if (entry.second.code == code_lc){
				mout.experimental("matched with lowercase: '", code_lc, "' <-> '", code, "'");
				return entry;
			}
		}
		mout.debug("continuing substring matching with (lowercase) '", code_lc, "'");
	}


	for (auto & entry: *this){
		// mout.experimental("starts with '", code_lc, "' ?");
		if (entry.second.code.find(code_lc) == 0){
			mout.experimental("'", entry.second.code, "' starts with '", code_lc, "'");
			return entry;
		}
	}

	size_t length = code_lc.length();
	for (auto & entry: *this){
		size_t i = entry.second.code.rfind(code_lc);
		mout.warn(entry.second.code, " <=> ", code_lc);
		if ((i != std::string::npos) && (i != (entry.second.code.length()-length))){
			mout.experimental("'", entry.second.code, "' ends with '", code_lc, "', i=", i);
			return entry;
		}
	}

	for (auto & entry: *this){
		size_t i = std::string::npos;
		if ((i=entry.second.code.find(code_lc)) != std::string::npos){
			mout.experimental("'", entry.second.code, "' contains '", code_lc, "' starting at pos(", i, ")");
			return entry;
		}
	}

	mout.error("could not find code '", code_lc, "' with flexible (substring) matching");

	/*
	// if extra lenient...
	static int counter = 0;
	key_type min = static_cast<double>(++counter);
	mout.warn("code (key) '", code, "' was not found in CLASS palette, added it with index=", counter);
	(*this)[min].code = code;

	return min;
	*/
	static
	value_type dummy;

	return dummy;
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

	//Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	Logger mout(__FUNCTION__, __FILE__);

	drain::FilePath filePath;

	// static
	const drain::RegExp labelRE("^[A-Z]+[A-Z0-9_\\-]*$");

	if (labelRE.test(filename)){
		//filePath.set(std::string("palette-") + filename + std::string(".json"));
		filePath.set(std::string("palette-") + filename + std::string(".txt")); // primary default type..
		mout.info(" extending path: ", filename, " => ", filePath);
	}
	else {
		mout.info(" setting filepath: ", filename);
		filePath.set(filename);
	}

	mout.debug(" Initial dir path: '", filePath.dir, "'");

	// If relative path, add explicit ./
	if (filePath.dir.empty())
		filePath.dir.push_front(".");

	mout.debug(" Initial file path: ", filePath.str());

	const std::string s = filePath.str();

	drain::Input ifstr(s);

	if (ifstr){

		comment = filePath.str();
		mout.note("reading: ", filePath.str());

		if (filePath.extension == "txt"){
			loadTXT(ifstr);
		}
		else if (filePath.extension == "json"){
			loadJSON(ifstr);
		}
		else {
			mout.error() << "unknown file type: " << filePath.extension << mout.endl;
			return;
		}

	}
	else {

		if (!flexible){
			mout.error(" opening file '", filename, "' failed");
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
				mout.error(" unsupported palette file type: '", filePath.extension, "', filename='", filename, "'");
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
					mout.info("trying... ", finalFilePath);
					mout.info("a.k.a.... ", finalFilePath.str().c_str());
					//ifstr.open(finalFilePath.str().c_str(), std::ios::in);
					ifstr.open(finalFilePath.str());
					//if (ifstr.good())
					//if (ifstr.is_open())
					if (ifstr)
						break;
				}
				if (ifstr)
					break;
				// if (ifstr.is_open())
				//	break;
			}

			if (!ifstr){  // still not good
				// if (!ifstr.good()){  // still not good
				///// ifstr.close();
				mout.error(" opening file '", finalFilePath.str(), "' failed");
				return;
			}

			mout.note("reading: ", finalFilePath.str());

			reset();

			if (finalFilePath.extension == "txt"){
				loadTXT(ifstr);
			}
			else if (finalFilePath.extension == "json"){
				loadJSON(ifstr);
			}
			else {
				///// ifstr.close();
				mout.error("unknown file type: ", finalFilePath.extension);
			}

		}
	};

	///// ifstr.close();
	mout.special("Title: ", this->title);

	updateDictionary();


}


void Palette::loadTXT(std::istream & ifstr){

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
	std::string labelPrev;


	// int index = 100;
	// Variable id;
	// id.setType(typeid(std::string));
	// id.setSeparator('-');

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
			// labelPrev = label;
			labelPrev = label;

			label = line.substr(i, j+1-i);

			/// First label in the file will be the title
			if (title.empty()){
				title = label;
				label.clear();
				labelPrev.clear();
			}

			continue;
		}

		if (c == '@'){
			SPECIAL = true;
			++i;
			mout .debug3("reading special entry[", label, "]");
		}
		else {
			SPECIAL = false;
			i = 0;
		}

		//mout.note() << "remaining line '" << line.substr(i) << "'" << mout.endl;

		std::istringstream data(line.substr(i));
		if (SPECIAL){
			//id = NAN;
		}
		else {
			if (!(data >> d)){
				mout.warn("suspicious line: ", line);
				continue;
			}
			mout .debug3("reading  entry[", d, "]");
		}


		PaletteEntry & entry = SPECIAL ? specialCodes[label] : (*this)[d]; // Create entry?

		// entry.value = d; NEW
		if (!label.empty()){
			if (label.at(0) == '.'){
				entry.hidden = true;
				entry.label = label.substr(1);
			}
			else {
				entry.hidden = false;
				entry.label = label;
			}
			entry.code = labelPrev;

			label.clear();
			labelPrev.clear();
		}

		// mout.warn(line, " <- ", label);

		/*
		id = "";
		id << ++index;
		id << entry.label;
		// entry.code = id.toStr();   NEW 2023/09
		*/

		//Variable colours(typeid(PaletteEntry::value_t)); // NOVECT
		PaletteEntry::color_t::iterator cit = entry.color.begin();
		while (data >> d) {
			//colours << d;
			if (cit == entry.color.end()){
				mout.error("Overflow of color elements(", entry.color.size(), "). last value: ", d);
			}
			else {
				*cit = d;
			}
			++cit;
		}
		//colours.toSequence(entry.color);


		entry.checkAlpha();

		//mout.note() << "got " << colours.getElementCount() << '/' << entry.color.size() << " colours" << mout.endl;

		// TODO!
		// if (entry.color.size() == 4)
		//  alpha=

		mout.debug3(entry.label, '\t', entry);


		// label.clear();

	}

	///// ifstr.close(); // ?

}


void Palette::loadJSON(std::istream & ifstr){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	reset();

	drain::JSONtree2 json;

	drain::JSON::readTree(json, ifstr);
	//drain::JSONtree::read(json, ifstr);

	//const drain::JSONtree2::node_data_t & metadata = json["metadata"].data;
	// Consider whole metadata as JSON tree?
	//title = metadata["title"]; // .toStr();
	const drain::JSONtree2 & metadata = json["metadata"];
	title = metadata["title"].data.toStr();

	mout.info("title: ", title);
	//mout.isLevel(LOG_INFO)
	mout.debug(); // << "metadata: ";
	drain::TreeUtils::dump(json, mout); //true);
	//drain::TreeUtils::dump(json, std::cout, nullptr); //true);
	mout << mout.endl;
	importJSON(json["entries"]);

}


void Palette::importJSON(const drain::JSONtree2 & entries){ //, int depth){

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	//const drain::JSONtree::tree_t & entries = json["entries"];

	//for (drain::JSONtree::tree_t::const_iterator it = entries.begin(); it != entries.end(); ++it){
	for (const auto & data: entries){

		//const std::string & id         = it->first;
		const drain::JSONtree2::key_t & id = data.first;
		const drain::JSONtree2 & child     = data.second;

		//const VariableMap & attr  = child.data;

		const Variable &value = child["value"].data;

		// Special code: dynamically assigned value
		const bool SPECIAL = value.isString();
		const double d = value; // possibly NaN

		if (SPECIAL){
			mout.debug("special code: '", value, "', id=", id);
		}

		//PaletteEntry & entry = SPECIAL ? specialCodes[id] : (*this)[d]; // Create entry?
		//PaletteEntry & entry = SPECIAL ? specialCodes[attr["value"]] : (*this)[d]; // Create entry?
		PaletteEntry & entry = SPECIAL ? specialCodes[value] : (*this)[d]; // Create entry?

		entry.code = id;
		// Deprecated, transitory:
		// entry.value = child["min"]; // NEW 2023
		entry.label = child["label"].data.toStr(); // child["en"].data.toStr();
		//entry.label = child["label"].data.toStr();

		mout.debug3("child (id=", id, '\n');
		// drain::TreeUtils::dump(child, mout);
		// mout << mout.endl;

		// mout.unimplemented("entry.getParameters()");
		// entry.getParameters().updateFromCastableMap(attr); ??

		// std::cerr << "color" << attr["color"] << std::endl;
		// NOVECT attr["color"].toSequence(entry.color);
		std::list<double> l;
		child["color"].data.toSequence(l);
		switch (l.size()) {
			case 4:
				entry.alpha = l.back();
				l.pop_back();
			case 3:
				entry.color.assignSequence(l);
				break;
			default:
				mout.fail("Unsupported number of colours: ", l.size());
		}

		// NOVECT entry.color.fromSequence(attr["color"]);
		// NOVECT entry.checkAlpha();

		if (SPECIAL){
			// entry.value = NAN;   NEW2023
		}

		mout.debug2("entry: ", id, ':', entry.getParameters());
		//importJSON(child, depth  + 1); // recursion not in use, currently
	}
}

void Palette::updateDictionary(){

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	this->dictionary.clear();

	for (Palette::const_iterator it = begin(); it != end(); ++it){
		mout.info() << "add " << it->first << ' ' << it->second.label << mout.endl;
		this->dictionary.add(it->first, it->second.code);
	}


}

void Palette::write(const std::string & filename) const {

	//Logger mout(getImgLog(), __FUNCTION__, __FILE__);
	Logger mout(__FUNCTION__, __FILE__);

	drain::FilePath filepath(filename);

	mout.info("writing: ", *this);
	mout.special("palette comment: ", comment);

	drain::Output ofstr(filename);

	if (NodeSVG::fileinfo.checkExtension(filepath.extension)){ // .svg
		mout.debug("writing SVG legend");
		TreeSVG svg;
		exportSVGLegend(svg, true);
		ofstr << svg;
	}
	else if (drain::JSON::fileInfo.checkExtension(filepath.extension)){ // .json
		mout.debug("exporting JSON palette");
		drain::JSONtree2 json;
		exportJSON(json);
		mout.debug("writing JSON palette/class file");
		drain::JSON::treeToStream(ofstr, json);
	}
	else if (filepath.extension == "inc"){
		// mout.debug("writing C++ palette (flat)");
		// exportCPP(ofstr, true);
		mout.info("Writing in 'flat' C++ format: list of values.");
		mout.debug("This format is compatible with PaletteEntry constructors.");
		//static const SprinterLayout Palette::cppLayout2("{,}", "{,}", "{,}", "\"\"", "");
		Sprinter::toStream(ofstr, *this, Palette::cppLayout2);
	}
	else if (filepath.extension == "ipp"){
		mout.note("Writing in structured C++ format: list of (nested) {key,value} pairs.");
		mout.debug("This format is compatible with std::map (ReferenceMap applied by BeanLike).");
		Sprinter::toStream(ofstr, *this, Sprinter::cppLayout);
		/*
		drain::JSONtree2 json;
		exportJSON(json);
		static const drain::SprinterLayout myCpp("[,]", "{:}", "(=)", "\"\"");
		drain::Sprinter::toStream(ofstr, json, myCpp); // drain::Sprinter::pythonLayout);
		*/
	}
	else if (filepath.extension == "py"){
		mout.note("Writing in flat Python format, preferring list to arrays.");
		static const SprinterLayout pythonLayout2("[,]", "{,}",  "(,)", "''", "\"\"");
		Sprinter::toStream(ofstr, *this, Sprinter::pythonLayout);
	}
	else if (filepath.extension == "PY"){
		mout.note("Writing in structured Python format: list of (nested) {key,value} pairs.");
		static const SprinterLayout pythonLayout2("[,]", "{,}",  "(,)", "''", "\"\"");
		Sprinter::toStream(ofstr, *this, pythonLayout2);
	}
	else if (filepath.extension == "JSON"){
		mout.note("Writing in JSON format: list of (nested) {key,value} pairs.");
		// mout.debug("This format is compatible with std::map (ReferenceMap applied by BeanLike).");
		Sprinter::toStream(ofstr, *this, Sprinter::jsonLayout);
	}
	else if (filepath.extension == "txt"){
		mout.debug("writing plain txt palette file");
		exportTXT(ofstr,'\t', '\t');
	}
	else if (filepath.extension == "pal"){
		mout.debug("writing formatted .pal file");
		exportFMT(ofstr, "# ${label}\nset style line ${value} lt rgb '#${colorHex}' lw 5  \n");
		//exportFMT(ofstr, "#${label}\n set color ${color}\n");
	}
	else {
		//
		mout.error("unknown file type, extension: ", filepath.extension);
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
		//for (std::map<std::string,PaletteEntry >::const_iterator it = specialCodes.begin(); it != specialCodes.end(); ++it){
		for (const auto & entry: specialCodes){
			// ostr << it->second << '\n';

			// ostr << '#' << entry.second.label << '\n';
			ostr << '#' << entry.first << '\n';
			ostr << "@ ";
			entry.second.toStream(ostr, separator); // , separator2);
			ostr << '\n';
			ostr << '\n';
		}
		ostr << '\n';
		ostr << '\n';
	}

	for (const auto & entry: *this){

		const double & min      = entry.first;
		const PaletteEntry & colour = entry.second;

		if (!colour.code.empty()){
			ostr << '#' << ' ' << colour.code << '\n'; // New, risky...
		}

		ostr << '#';

		if (colour.hidden)
			ostr << '.';

		if (!colour.label.empty()){
			ostr << ' ' << colour.label;
		}
		else
			ostr << '?'; // << it->second.id;

		ostr << '\n';

		ostr << min << separator;
		colour.toStream(ostr, separator); //, separator2);
		ostr << '\n';
		ostr << '\n';
	}

}

/*
void Palette::exportCPP(std::ostream & ostr, bool flat) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	//std::cout << "cppLayout: --- "<< '\n';
	if (flat){
		mout.info("Writing in 'flat' C++ format: list of values.");
		mout.debug("This format relies on PaletteEntry constructors.");
		//static const SprinterLayout Palette::cppLayout2("{,}", "{,}", "{,}", "\"\"", "");
		Sprinter::toStream(ostr, *this, Palette::cppLayout2);
	}
	else {
		mout.note("Writing in structured C++ format: list of (nested) {key,value} pairs.");
		mout.debug("This format relies on ReferenceMap reader applied by BeanLike.");
		Sprinter::toStream(ostr, *this, Sprinter::cppLayout);
	}

}

*/


	/*
	ostr << "{";
	if (title.empty()){
		ostr << " // " << title;
	}
	ostr << '\n';

	char sep = 0;

	if (!specialCodes.empty()){
		ostr << "  // special codes //\n";
		for (const auto & entry: specialCodes){
			if (sep)
				ostr << sep << '\n';
			else
				sep = ',';
			ostr << "    {" << entry.first << ", {";
			entry.second.toStream(ostr, ',');
			ostr << "} }\n";
		}
	}

	if (sep)
		ostr << sep << '\n';

	ostr << "  // colours  \n";
	for (const auto & entry: *this){
		if (sep)
			ostr << sep << '\n';
		else
			sep = ',';
		ostr << "    {" << entry.first << ", ";
		if ((!entry.second.id.empty()) && (entry.second.id != entry.second.label)){
			ostr << '"' << entry.second.id << '"' << ", ";
		}
		ostr << '"' << entry.second.label << '"' << ", {";
		entry.second.toStream(ostr, ',');
		//ostr << "} }";
		ostr << "}";
		if (entry.second.alpha < 1.0){
			ostr << ", " << entry.second.alpha;
		}
		ostr << " }";
	}

	ostr << "\n}\n";
	*/


//void Palette::exportJSON(drain::JSONtree::tree_t & json) const {
void Palette::exportJSON(drain::JSONtree2 & json) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	mout.warn("JSON syntax may change in future, ensure palettes using other formats.");

	drain::JSONtree2 & metadata = json["metadata"];
	metadata["title"] = title;

	drain::JSONtree2 & entries =  json["entries"];

	int i = 0;
	std::stringstream key;

	// Start with special codes (nodata, undetect)
	// for (spec_t::const_iterator it = specialCodes.begin(); it!=specialCodes.end(); ++it){
	// const PaletteEntry & entry = it->second;

	for (const auto & entry: specialCodes){

		key.str("");
		key << "special";
		key.width(2);
		key.fill('0');
		key << ++i;

		//drain::JSONtree::tree_t & js = entries[key.str()]; // entries[entry.id];
		drain::JSONtree2 & js = entries[key.str()];

		js["color"] = entry.second.color;
		//js.data.importCastableMap(entry.second.getParameters()); // color repeated?
		for (const auto & param: entry.second.getParameters()){
			js[param.first] = param.second;
		}
		js["value"] = entry.first;
		//mout.warn("value", entry.first);
	}

	i = 0;
	for (const auto & entry: *this){
		//for (Palette::const_iterator it = begin(); it!=end(); ++it){
		//const PaletteEntry & entry = it->second;
		key.str("");
		key << "c";
		key.width(4);
		key.fill('0');
		key << ++i;
		drain::JSONtree2 & m = entries[key.str()];
		const drain::image::PaletteEntry & col = entry.second;
		// m["color"] = col.color;
		// mout.warn("id: ", col.id, " = ", col.getParameter<std::string>("id"));
		for (const auto & param: col.getParameters()){
			m[param.first] = param.second;
		}
		// m.importCastableMap(entry.second.getParameters());
	}

	/*

	VariableMap & metadata = json["metadata"].data;
	metadata["title"] = title;

	drain::JSONtree::tree_t & entries =  json["entries"];

	int i = 0;
	std::stringstream key;

	// Start with special codes (nodata, undetect)
	// for (spec_t::const_iterator it = specialCodes.begin(); it!=specialCodes.end(); ++it){
	// const PaletteEntry & entry = it->second;

	for (const auto & entry: specialCodes){
		key.str("");
		key << "special";
		key.width(2);
		key.fill('0');
		key << ++i;

		drain::JSONtree::tree_t & js = entries[key.str()]; // entries[entry.id];

		js.data["color"] = entry.second.color;
		js.data.importCastableMap(entry.second.getParameters()); // color repeated?
		Variable & value = js.data["value"];
		value.reset();
		value = entry.first;
		//mout.warn("value", entry.first);
	}

	i = 0;
	for (const auto & entry: *this){
		//for (Palette::const_iterator it = begin(); it!=end(); ++it){
		//const PaletteEntry & entry = it->second;
		key.str("");
		key << "colour";
		key.width(3);
		key.fill('0');
		key << ++i;
		VariableMap & m = entries[key.str()].data; // entries[entry.id].data;
		m["color"] = entry.second.color;
		m.importCastableMap(entry.second.getParameters());
	}
	 */

}


void Palette::exportFMT(std::ostream & ostr, const std::string & format) const {

	Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	PaletteEntry entry;

	// entry.id = "test";

	// entry.color.resize(3); // NOVECT
	//entry.getParameters().link("color", entry.color);
	entry.getParameters().link("color", entry.color.tuple());

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
			mout.warn() << it->second.code << mout.endl;
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
			// pNew.color.resize(s);
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
	// svg->set("id", title);
	// TODO font-size:  font-face:

	/*  Moved below, and "header" -> "title"
	TreeSVG & header = svg["header"];
	header->setType(NodeSVG::TEXT);
	header->set("x", lineheight/4);
	header->set("y", (headerHeight * 9) / 10);
	header->ctext = title;
	header->set("style","font-size:20");
	*/

	TreeSVG & background = svg["bg"];
	background->setType(NodeSVG::RECT);
	background->set("x", 0);
	background->set("y", 0);
	//background->set("style", "fill:white opacity:0.8"); // not supported by some SVG renderers
	background->set("fill", "white");
	background->set("opacity", 0.8);
	// width and height are set in the end (element slot reserved here,for rendering order)


	TreeSVG & header = svg["title"];
	header->setType(NodeSVG::TEXT);
	header->set("x", lineheight/4);
	header->set("y", (headerHeight * 9) / 10);
	header->ctext = title;
	header->set("style","font-size:20");


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
			const PaletteEntry::color_t & color = entry.color;
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


} // image::

} // drain::

template <>
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const drain::image::Palette & palette, const drain::SprinterLayout & layout){

	char sep = 0;
	ostr << layout.arrayChars.prefix << '\n';

	if (!palette.title.empty()){
		ostr << "  " << layout.pairChars.prefix;
		// ostr << layout.keyChars.prefix << "title" << layout.keyChars.suffix;
		drain::Sprinter::toStream(ostr, "title", layout);
		ostr << layout.pairChars.separator << ' ';
		drain::Sprinter::toStream(ostr, palette.title, layout);
		ostr << layout.pairChars.suffix;
		ostr << layout.arrayChars.separator << '\n';
	}

	if (!palette.specialCodes.empty()){
		for (const auto & entry: palette.specialCodes){

			if (sep)
				ostr << sep << '\n'; // <-- yes, additional newline
			else
				sep = layout.mapChars.separator;

			ostr << "  " << layout.pairChars.prefix;
			drain::Sprinter::toStream(ostr, entry.first, layout);
			// ostr << layout.keyChars.prefix << entry.first << layout.mapChars.separator << ' ';
			ostr << layout.pairChars.separator << ' ';
			drain::Sprinter::toStream(ostr, entry.second, layout);
			ostr << ' ' << layout.pairChars.suffix;
		}
		// ostr << layout.mapChars.separator;
	}


	for (const auto & entry: palette){

		if (sep)
			ostr << sep << '\n'; // <-- yes, additional newline
		else
			sep = layout.mapChars.separator;

		ostr << "  " << layout.pairChars.prefix;
		ostr.fill(' ');
		ostr.width(10);
		ostr << entry.first << layout.pairChars.separator << ' ';
		drain::Sprinter::toStream(ostr, entry.second, layout);
		//drain::Sprinter::mapToStream(ostr, entry.second.getParameters().getMap(), layout, keys);
		ostr << ' ' << layout.pairChars.suffix;
	}
	ostr << '\n' << layout.arrayChars.suffix << '\n';

	return ostr;
}

// Drain
