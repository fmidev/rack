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
/**
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
 */

#include "FilePnm.h"

#include "drain/util/Input.h"
#include "drain/util/JSON.h"
// #include "drain/util/JSONtree.h"
#include "drain/util/Output.h"
#include "drain/util/TextReader.h"
#include "drain/util/Time.h"


namespace drain
{

namespace image
{

/// Syntax for recognising PNM image files
//const drain::RegExp FilePnm::fileNameRegExp("^((.*/)?([^/]+))\\.(p([bgpn])m)$", REG_EXTENDED | REG_ICASE);
const drain::FileInfo FilePnm::fileInfo("p([bgpn])m");


// drain::Type & t, drain::image::Geometry & geometry
FilePnm::FileType FilePnm::readHeader(drain::image::ImageConf & conf, drain::FlexVariableMap & properties, std::istream & infile) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (infile.get() != 'P'){
		mout.warn("file does not start with  'P' (magic code)" );
		mout.error("not an PNM file" );
		return UNDEFINED;
	}

	FileType fileType = UNDEFINED;
	// infile >> fileType;

	int width;
	int height;
	int channels = 1;
	int maxValue;

	int c = infile.get();

	switch (c){
	case '1':
		fileType = PBM_ASC;
		channels = 1;
		break;
	case '2':
		fileType = PGM_ASC;
		channels = 1;
		break;
	case '3':
		fileType = PPM_ASC;
		channels = 3;
		break;
	case '4':
		fileType = PBM_RAW;
		channels = 1;
		break;
	case '5':
		fileType = PGM_RAW;
		channels = 1;
		break;
	case '6':
		fileType = PPM_RAW;
		channels = 3;
		break;
	default:
		mout.error("unrecognized PPM type" );
		return fileType;
	}

	mout.info("PNM type: P" ,  (char)c , " (" , channels  , " channels)" );

	std::string key;
	std::stringstream sstr;

	// Jump to end of line.,
	drain::TextReader::skipWhiteSpace(infile);
	//drain::TextReader::scanSegment(infile, "\n", sstr);  // NEW
	//mout.warn("Remains: '", sstr.str(), "' peek=", (char)infile.peek());
	//sstr.str("");
	//infile.get();

	// Read comment lines
	while (infile.peek() == '#'){
		infile.get(); // swallow '#'
		while ((c=infile.get()) !='\n' ){

			if (c == '='){
				key = drain::StringTools::trim(sstr.str());
				//mout.warn("key: ", key);
				sstr.str("");
			}
			else
				sstr.put(c);

			if (infile.eof()){
				mout.error("Premature end of file: ");
			}

		}
		if (!key.empty()){
			mout.debug2("Comment: " , key , ": " ,  sstr.str() );
			//ValueReader::scanValue(sstr.str(), properties[key]);
			JSON::readValue(sstr.str(), properties[key]);
		}
		else {
			mout.note("Comment:" ,  sstr.str() );
		}
		// mout.attention(sstr.str());
		sstr.str("");
	}
	// mout.note("Done" );
	infile >> width;
	infile >> height;
	// mout.attention("size: ", width, 'x', height);

	if ((fileType != PBM_ASC) && (fileType != PBM_RAW))
		infile >> maxValue;

	if (maxValue < 100){
		mout.warn("suspicious max value:" ,  maxValue );
	}

	if (maxValue > 0xff){
		if (maxValue > 0xffff){
			mout.warn("suspiciously large max value:" ,  maxValue );
		}
		mout.note("max value (" ,  maxValue , ") over 255, using 16 bits (unsigned)");
		conf.setType(typeid(unsigned short));
	}

	// drain::TextReader::scanSegment(infile, "\n");
	drain::TextReader::skipWhiteSpace(infile);

	// Under constr
	// conf.encoding.setType();
	// conf.encoding.scaling.setPhysicalMax(maxValue);
	if (properties.hasKey("coordinatePolicy")){
		std::vector<int> policy;
		properties["coordinatePolicy"].toSequence(policy);
		conf.coordinatePolicy.assignSequence(policy);
	}

	conf.setGeometry(width, height, channels);

	return fileType;
}


// , const CommentReader & commentReader
void FilePnm::read(Image & image, const std::string & path) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug2("path='" , path , "'" );

	Input infile(path);
	// std::ifstream infile;
	// infile.open(path.c_str(), std::ios::in);

	if (!infile){
		mout.warn("opening file '" , path , "' failed" );
		return;
	}

	ImageConf conf;
	FileType fileType = readHeader(conf, image.properties, infile);
	mout.debug("conf: ", conf );
	//mout.debug("prop: " , image.properties );

	// Resize
	image.setConf(conf);

	mout.debug2(image );
	readFrame(image, infile, fileType);

}

// , const CommentReader & commentReader
void FilePnm::readFrame(ImageFrame & image, const std::string & path) {

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info("reading image: " , image , ", file=" , path );

	Input infile(path);

	ImageConf conf;

	FileType fileType = readHeader(conf, image.properties, infile);
	mout.debug("conf: " , conf );
	mout.debug("prop: " , image.properties );

	readFrame(image, infile, fileType);

	//infile.close();



}


/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::readFrame(ImageFrame & image, std::istream & infile, FileType fileType){ // , FileType t

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info("reading to frame: " , image );
	// const size_t channels = image.getChannelCount();

	bool ARRAY_FULL = false;

	const int BYTES = Type::call<drain::sizeGetter>(image.getType());
	// drain::Type::call<drain::typeMax, int>(image.getType());
	// image.getType();
	//const bool  = (bytes==2);

	const ImageFrame::iterator eit = image.end();

	if (fileType == PGM_ASC){
		int i=0;
		ImageFrame::iterator it = image.begin();
		while ((!infile.eof()) && (it != eit)){
			infile >> i;
			*it = i;
			++it;
		}
		drain::TextReader::scanSegment(infile, " \t\n\r");

		// mout.debug("read " , i , " bytes" );
		ARRAY_FULL = (it==eit);

		if (it != eit){
			mout.warn("premature end of file: " , image );
		}
	}
	else if (fileType == PGM_RAW){
		// ONLY FOR int8 and uint16 dst img!
		ImageFrame::iterator it = image.begin();
		if (BYTES == 1){
			while ((!infile.eof()) && (it != eit)){
				*it = infile.get();
				++it;
			}
		}
		else {
			int data;
			while ((!infile.eof()) && (it != eit)){
				data =  (infile.get()<<8);
				data |= infile.get();
				*it = data;
				++it;
			}
		}
		// mout.debug("read " , i , " bytes" );
		ARRAY_FULL = (it==eit);
	}
	else if (fileType == PPM_ASC){
		int r=0, g=0, b=0;
		ImageFrame::iterator  rit = image.getChannel(0).begin();
		ImageFrame::iterator  git = image.getChannel(1).begin();
		ImageFrame::iterator  bit = image.getChannel(2).begin();
		while (!infile.eof()){
			// how to ensure numeric char?
			infile >> r >> g >> b;
			*rit = r;
			*git = g;
			*bit = b;
			++rit, ++git; ++bit;
			// TODO CHECK
		}


		drain::TextReader::scanSegment(infile, " \t\n\r");
		ARRAY_FULL = (bit==eit);

		if (bit != eit){
			mout.warn("premature end of file: " , image );
		}
	}
	else if (fileType == PPM_RAW){
		ImageFrame::iterator  rit = image.getChannel(0).begin();
		ImageFrame::iterator  git = image.getChannel(1).begin();
		ImageFrame::iterator  bit = image.getChannel(2).begin();
		while (!infile.eof()){
			*rit = infile.get();
			++rit;
			*git = infile.get();
			++git;
			*bit = infile.get();
			++bit;
			// TODO CHECK
		}

		ARRAY_FULL = (bit==eit);
		if (bit != eit){
			mout.warn("premature end of file: " , image );
		}

	}
	else {
		mout.unimplemented(image.getConf() );
		mout.unimplemented("PBM file type: " , fileType );
		mout.error("Unsupported file type" );
		//mout.error("Sorry, PNM image with " , channels , " channels not implemented" );
	}


	if (infile.peek() != EOF)
		mout.warn(" spurious bytes at end of file?" );

	if (!ARRAY_FULL){
		mout.warn("premature end of file: " , image );
	}


	/*
	if (!infile.eof()){
		int i=0; // , c;
		while (!infile.eof()){
			// c =
			infile.get();
			++i;
			//mout.warn(++i , "\t'" , (char)c , "'" );
		}
		mout.warn(i , " spurious bytes in end of file" );
	}
	*/

	//infile.close();

}

/*
void FilePnm::readFrameASCII(ImageFrame & image, std::istream & infile){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.info("reading to frame: " , image );

	const size_t channels = image.getChannelCount();

	const ImageFrame::iterator eit = image.end();

	if (channels == 1){
		int i=0;
		ImageFrame::iterator        it = image.begin();
		while ((!infile.eof()) && (it != eit)){
			infile >> i;
			*it = i;
			++it;			// ++i;
		}
		// mout.debug("read " , i , " bytes" );
		if (it != eit){
			mout.warn("premature end of file: " , image );
		}
	}
	else if (channels == 3){
		int r=0, g=0, b=0;
		ImageFrame::iterator  rit = image.getChannel(0).begin();
		ImageFrame::iterator  git = image.getChannel(1).begin();
		ImageFrame::iterator  bit = image.getChannel(2).begin();
		while (!infile.eof()){
			// how to ensure numeric char?
			infile >> r >> g >> b;
			*rit = r;
			*git = g;
			*bit = b;
			++rit, ++git; ++bit;
			// TODO CHECK
		}

		if (bit != eit){
			mout.warn("premature end of file: " , image );
		}

	}
	else {
		mout.error("Sorry, PNM image with " , channels , " channels not implemented" );
	}


	if (infile.peek() != EOF)
		mout.warn(" spurious bytes at end of file?" );


}
*/





/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
void FilePnm::write(const ImageFrame & image, const std::string & path){

	Logger mout(getImgLog(), "FilePnm", __FUNCTION__);

	if (image.isEmpty()){
		mout.warn("empty image, skipping" );
		return;  // -1
	}

	mout.debug("Path: " , path );


	const int width    = image.getWidth();
	const int height   = image.getHeight();
	const int channels = image.getChannelCount();
	const std::type_info & type = image.getType();
	const int maxValue = drain::Type::call<drain::typeMax, int>(type);
	const bool SINGLE_BYTE = (maxValue <= 0xff);

	FileType storage_type = UNDEFINED;

	// Debugging. Consider also using for checking file extension vs channel count.
	char colorTypeChar = 0;
	drain::RegExp::result_t result;
	drain::FilePath filepath(path);
	mout.experimental("Extension check, regexp=", fileInfo.extensionRegexp);
	if (!fileInfo.extensionRegexp.execute(filepath.extension, result)){
		colorTypeChar = result[1].at(0);
		//mout.experimental("file char: " , result[1] );
	}
	// mout.warn("Result: " ,	sprinter(result) );
	mout.experimental("Result: ", sprinter(result), " color type char: ",  colorTypeChar);

	switch (channels) {
	case 4:
		mout.warn() << "four-channel image; writing channels 0,1,2 only" << mout.endl;
		// no break
	case 3:
		storage_type = PPM_RAW;
		if (colorTypeChar != 'p'){
			mout.warn("Odd color type char '", colorTypeChar, "' for rgb image data");
		}
		break;
	case 2:
		mout.warn("two-channel image, writing channel 0" );
		// no break
	case 1:
		storage_type = PGM_RAW;
		if (colorTypeChar != 'g'){
			mout.warn("Odd color type char '", colorTypeChar, "' for gray image data");
		}
		break;
	case 0:
		mout.warn("zero-channel image" );
		//fclose(fp);
		return;
	default:
		mout.error("unsupported channel count: " , channels );
		// throw std::runtime_error(s.toStr());
	}

	drain::Output output(path);
	std::ofstream & ofstr = output;

	/// FILE HEADER
	mout.debug("magic code: P" , storage_type );
	ofstr << 'P' << storage_type << '\n';

	mout.debug("writing comments (metadata)");
	const FlexVariableMap & vmap = image.getProperties();
	if (vmap.hasKey("")){
		mout.note("Comment override mode detected (comment with empty key)");
		ofstr << '#' << ' ' << drain::StringTools::replace( vmap.get("", ""), "\n", "\n# ");
		// ofstr << vmap.get("", "");
		ofstr << '\n';
	}
	else {
		for (const auto & entry: vmap) {
			ofstr << '#' << ' ' << entry.first << '=';
			//it->second.valueToJSON(ofstr);
			Sprinter::toStream(ofstr, entry.second, Sprinter::jsonLayout);
			ofstr << '\n';
		}
	}

	// GEOMETRY
	ofstr << width << ' ' << height << '\n';
	if (drain::Type::call<drain::typeIsSmallInt>(type))
		ofstr << maxValue << '\n';
	else {
		mout.error("unimplemented: double type image (needs scaling) " );
		ofstr << 255 << '\n';
	}

	int i;
	int j;
	int value;

	switch (storage_type) {
	case PGM_RAW:
		if (SINGLE_BYTE){
			mout.note("PGM_RAW, 8 bits" );
			for (j = 0; j < height; ++j) {
				for (i = 0; i < width; ++i) {
					ofstr.put(image.get<unsigned char>(i,j));
					//ofstr.
				}
			}
		}
		else {
			mout.note("PGM_RAW, 16 bits" );
			if (maxValue > 0xffff){
				mout.warn("storage type over 16 bits (max value > 0xffff) unsupported" );
			}
			// TODO : scalar iter
			for (j = 0; j < height; ++j) {
				for (i = 0; i < width; ++i) {
					value = image.get<unsigned short>(i,j);
					ofstr.put((value>>8) & 0xff);  // check order!
					ofstr.put(value & 0xff);
					//valueHi = (value >> 8);
				}
			}
		}
		break;
	case PGM_ASC:
		mout.note("PGM_ASC" );
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				ofstr << image.get<int>(i,j) << ' ';
			}
			ofstr << '\n';
		}
		break;
	default:
		mout.error("PBM/binary types: unimplemented code");
		break;
	}

	mout.debug2("Closing file" );
	ofstr.close();

}




} // image::

} // drain::



// Drain
