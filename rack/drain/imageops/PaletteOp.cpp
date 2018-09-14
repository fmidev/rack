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
#include "PaletteOp.h"

namespace drain
{

namespace image
{

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */


/*
void Palette::skipLine(std::ifstream &ifstr) const{

}
*/

void Palette::load(const std::string &filename){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	mout.note() << " Reading file=" << filename << mout.endl;

	// TODO: drain::StringMapper formatter("[a-zA-Z0-9_]+");


	std::ifstream ifstr;
	ifstr.open(filename.c_str(), std::ios::in);
	if (!ifstr.good()){
		mout.error() << " opening file '" << filename << "' failed" << mout.endl;
	};

	_hasAlpha = false;
	char c;
	double d;
	_colors = 0;
	clear();

	std::vector<double> tmp;
	std::stringstream label;

	bool SPECIAL;
	while (!ifstr.eof()){

		c = ifstr.peek();
		while ((c==' ')||(c=='\t')||(c=='\n')){
			ifstr.get();
			c = ifstr.peek();
		}

		if (c=='#'){
			ifstr.get(); // swallow '#'
			label.str(""); // this is needed in two places
			//monitor.note(3) << " comment: " << monitor.endl;
			c = ifstr.peek();
			while (c==' '){
			   ifstr.get();
			   c = ifstr.peek();
			}
			while ((c = ifstr.get()) != '\n'){
				if (ifstr.eof())
					return;
				label << c;
			}
			if (title.empty())
				title = label.str();
		}
		else {

			SPECIAL = (ifstr.peek()=='@');

			// index
			if (!SPECIAL){
				ifstr >> d;
				mout.debug() << " index: " << d << '\t';
			}
			else {
				ifstr.get(); //
				mout.debug() << " special: " << label.str() << '\t';
			}

			if (ifstr.eof())
				return;

			//std::cout << "index:" << d << '\n';

			// Read line
			PaletteEntry &entry = SPECIAL ? specialCodes[label.str()] : (*this)[d]; // Create entry
			entry.label = label.str();  // Save last label
			label.str(""); // this is needed in two places
			//entry.resize(4);
			unsigned int i=0;
			do {
				//ifstr >> entry[i];
				ifstr >> d;
				if (ifstr.eof())
					return;

				if (_colors == 0){
					entry.resize(i+1);
				}
				else {
					entry.resize(_colors);
					if (i >= _colors)
						mout.error() << " increased color count? index=" << i << " #colors=" << _colors << mout.endl;
						//throw std::runtime_error(" Palette::read: ");
				};

				mout << d << ' ';

				entry[i] = d;
				++i;

				/// Read the trailing whitespace
				while ((ifstr.peek() == ' ') || (ifstr.peek() == '\t')){
					c = ifstr.get();
					//std::cout << c;
				}

				if (ifstr.eof())
					return;

			}
			while (ifstr.peek() != '\n');

			mout << mout.endl;

			// Now fixed.
			_colors = entry.size();

			// Read std::endline (because next
			//ifstr.get();
			// for (size_t i = 0; i < _colors; ++i)
			//	std::cout << entry[i] << ',';
			//std::cout << "NEXT\n";

		}

	};
	ifstr.close();

}


void Palette::refine(size_t n){

	Logger mout(getImgLog(), "Palette", __FUNCTION__);

	//const double n2 = static_cast<double>(size()-1) / static_cast<double>(n);

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
	const size_t s = it->second.size();

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
			pNew.resize(s);
			for (size_t j = 0; j < s; ++j) {
				pNew[j] = c*p[j] + (1.0f-c)*p0[j];
			}
		}
		it0 = it;
		++it;
	}

}

void Palette::getLegend(TreeSVG & svg, bool up) const {

	const int lineheight = 20;

	svg->setType(NodeSVG::SVG);
	// TODO font-size:  font-face:

	TreeSVG & header = svg["header"];
	header->setType(NodeSVG::TEXT);
	header->set("x", lineheight);
	header->set("y", 1.4*lineheight);
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
	int width = 100;
	int height = (1.5 + size() + specialCodes.size()) * lineheight;

	int y;
	std::stringstream name;
	std::stringstream style;
	std::stringstream threshold;

	Palette::const_iterator it = begin();
	std::map<std::string,PaletteEntry >::const_iterator itSpecial = specialCodes.begin();

	while ((it != end()) || (itSpecial != specialCodes.end())){
	//for (Palette::const_iterator it = begin(); it != end(); ++it){

		bool specialEntries = (itSpecial != specialCodes.end());
		//bool specialEntries = (it == end());

		const PaletteEntry & entry = specialEntries ? itSpecial->second : it->second;

		name.str("");
		name << "color";
		name.fill('0');
		name.width(2);
		name << index;
		TreeSVG & s = svg[name.str()];
		s->setType(NodeSVG::GROUP);

		y = up ? height - (index+1) * lineheight : index * lineheight;

		TreeSVG & rect = s["r"];
		rect->setType(NodeSVG::RECT);
		rect->set("x", 0);
		rect->set("y", y);
		rect->set("width", lineheight*1.7);
		rect->set("height", lineheight);
		//s["r"]->set("style", "fill:green");


		style.str("");
		switch (entry.size()){
		case 4:
			style << "fill-opacity:" << entry[3]/255.0 << ' ';
			// no break
		case 3:
			style << "fill:rgb(" << entry[0] << ',' << entry[1] << ',' << entry[2] << ");";
			break;
		case 1:
			style << "fill:rgb(" << entry[0] << ',' << entry[0] << ',' << entry[0] << ");";
			break;
		}
		rect->set("style", style.str());

		TreeSVG & thr = s["th"];
		thr->setType(NodeSVG::TEXT);
		threshold.str("");
		//minDBZ.precision(1);
		if (!specialEntries)
			threshold << it->first;
		//	minDBZ << itSpecial->first;
		//else

		thr->ctext = threshold.str();
		thr->set("text-anchor", "end");
		thr->set("x", lineheight*1.5);
		thr->set("y", y + lineheight-1);

		TreeSVG & text = s["t"];
		text->setType(NodeSVG::TEXT);
		text->ctext = entry.label;
		text->set("x", 2*lineheight);
		text->set("y", y + lineheight-1);

		//ostr << it->first << ':';

		++index;

		if (specialEntries)
			++itSpecial;
		else
			++it;
		//ostr << '\n';
	}

	background->set("width", width);
	background->set("height", height);


	svg->set("width", width);
	//svg->set("height", index * lineheight);
	svg->set("height", height);

}

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

void PaletteOp::setPalette(const Palette &palette) {
	this->palette = palette;
}


void PaletteOp::setSpecialCode(const std::string code, double f) {
	//if (this->palette == NULL)
	//	throw std::runtime_error("PaletteOp::setSpecialCode: palette not set (null)");
	//if (specialCode.hasKe)
	std::map<std::string,PaletteEntry >::const_iterator it = palette.specialCodes.find(code);
	if (it != palette.specialCodes.end())
		specialCodes[f] = it->second;
	else
	//	throw std::runtime_error(code + ": setSpecialCode: could not find entry");
		std::cerr << code <<  ": setSpecialCode: could not find entry\n";

}


/*
void PaletteOp::setPalette(const Image &palette) const {
	//const Geometry & gPal = palette.getGeometry();

	const unsigned int width  = palette.getWidth();
	const unsigned int height = palette.getHeight();
	const unsigned int channels = palette.getChannelCount();

	const unsigned int colors = 256;
	paletteImage.setGeometry(colors,1,palette.getImageChannelCount(),palette.getAlphaChannelCount());
	for (unsigned int i = 0; i < colors; ++i) {
		for (unsigned int k = 0; k < channels; ++k) {
			paletteImage.put(i,0,k, palette.get<int>((i*width)/colors,(i*height)/colors,k));
		}
	}
}
 */


void PaletteOp::makeCompatible(const ImageFrame &src,Image &dst) const {
	//const Geometry &gSrc = src.getGeometry();
	//const Geometry &gPal = paletteImage.getGeometry();
	//dst.setGeometry(gSrc.getWidth(),gSrc.getHeight(),gPal.getImageChannelCount(),gPal.getAlphaChannelCount());
	//const unsigned int alphaChannels = max(paletteImage.getAlphaChannelCount(),src.getAlphaChannelCount());
	const unsigned int alphaChannels = palette.hasAlpha() || (src.getAlphaChannelCount()>0) ? 1 : 0;
	dst.setGeometry(src.getWidth(), src.getHeight(), 3, alphaChannels);
}


void PaletteOp::help(std::ostream & ostr) const {
	ImageOp::help(ostr);
	for (std::map<double,PaletteEntry >::const_iterator cit = specialCodes.begin(); cit != specialCodes.end(); ++cit){
		ostr << cit->first << '=' << cit->second << '\n';
	}
	for (std::map<std::string,PaletteEntry >::const_iterator pit = palette.specialCodes.begin(); pit != palette.specialCodes.end(); ++pit){
		ostr << "'" << pit->first << "'=" << pit->second << '\n';
	}
}


void PaletteOp::process(const ImageFrame &src,Image &dst) const {

	Logger mout(getImgLog(), name, __FUNCTION__);

	initialize();


	makeCompatible(src,dst);

	const unsigned int width  = dst.getWidth();
	const unsigned int height = dst.getHeight();
	const unsigned int channels = palette.hasAlpha() ? 4 : 3;

	const bool hasSpecialCodes = !specialCodes.empty();  // for PolarODIM nodata & undetected



	double d;
	Palette::const_iterator it;      // lower bound
	Palette::const_iterator itLast;  // upped bound
	//std::map<double,std::vector<double> >::iterator
	unsigned int k;
	//int code;
	std::map<double,PaletteEntry >::const_iterator cit;
	for (unsigned int i = 0; i < width; ++i) {
		//std::cerr << "Palette: " << i << '\t' << '\n';
		for (unsigned int j = 0; j < height; ++j) {

			if (hasSpecialCodes){  // PolarODIM
				//code = src.get<double>(i,j);
				cit = specialCodes.find(src.get<double>(i,j));
				if (cit != specialCodes.end()){
					for (k = 0; k < channels; ++k)
						dst.put(i,j,k, cit->second[k]);
					continue;
				}
			}

			d = scale * src.get<double>(i,j) + offset;

			// TODO: stl::lower_bound
			itLast = palette.begin();
			for (it = palette.begin(); it != palette.end(); ++it){
				if (it->first > d)
					break;
				itLast = it;
			}

			for (k = 0; k < channels; ++k)
				dst.put(i,j,k, itLast->second[k]);

			/*
			if (i==j){
				//std::cout <<	"palettex " << src.get<double>(i,j) << "\t =>" << d << "\t => " << it->first << '=' << it->second << '\n';
				dst.put(i,j,0, i*123);
				dst.put(i,j,2, i*53);
			}


			if ((i<40)&&(j<40)){
				dst.put(i,j,0, 255);
				dst.put(i,j,1, i*6);
				dst.put(i,j,2, (40-j)*6);
			}
			dst.put(i,j,0, i+j);
			dst.put(i,j,1, 253*i+257*j);
			*/
		}
	}

	//File::write(dst,"palette-color.png");

	//for (unsigned int k = 0; k < channels; ++k) {
	//	dst.put(i,j,k, paletteImage.get<int>(src.get<int>(i,j)&255,0,k));  // % was not ok! for char?
	//}

	if (!palette.hasAlpha()){
		if ((src.getAlphaChannelCount()>0) && (dst.getAlphaChannelCount()>0)){
			mout.info() << "Copying original alpha channel" << mout.endl;
			CopyOp().traverseChannel(src.getAlphaChannel(), dst.getAlphaChannel());
		}
	}


	//return dst;
}



}

}

// Drain
