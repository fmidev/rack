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


#include "drain/util/Time.h"
#include "FilePng.h"

namespace drain
{

namespace image
{


// http://www.libpng.org/
// zlib.h

/* compression levels */
/*
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
*/

//Default value for png_set_compression_level(png_ptr, ...);
#ifdef PNG_1_2_X
	short int FilePng::compressionLevel = Z_DEFAULT_COMPRESSION;
#else
	short int FilePng::compressionLevel = 5; // should be changed -1 = ffff?
#endif

/// Syntax for recognising image files
FileInfo FilePng::fileInfo("png");




void FilePng::readFile(const std::string & path, png_structp & png_ptr, png_infop & info_ptr, int png_transforms) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug("path='" , path , "'" );


	// Try to open the file
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == nullptr){
		throw std::runtime_error(std::string("FilePng: could not open file: ") + path);
	}

	// For checking magic code (signature)
	const size_t PNG_BYTES_TO_CHECK=4;
	png_byte buf[PNG_BYTES_TO_CHECK];

	/* Read in some of the signature bytes */
	if (fread((void *)buf, size_t(1), PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK){
		fclose(fp);
		throw std::runtime_error(std::string("FilePng: premature end, suspicious size of file: ") + path);
	}

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	   Return nonzero (true) if they match */
	if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0){
		fclose(fp);
		throw std::runtime_error(std::string("FilePng: not a png file: ") + path);
	}

	// Main'ish
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		fclose(fp);
		throw std::runtime_error(std::string("FilePng: problem in allocating image memory for: ")+path);
	}

	// Main'ish
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		fclose(fp);
	    png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
		throw std::runtime_error(std::string("FilePng: problem in allocating info memory for: ")+path);
	}

	// This may be unstable. According to the documentation, if one uses the high-level interface png_read_png()
	// one can only configure it with png_transforms flags (PNG_TRANSFORM_*)
	png_set_palette_to_rgb(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	/// Main action
	mout.debug("reading data" );

	png_read_png(png_ptr, info_ptr, png_transforms, NULL);

	// Should be safe - png struct is now complete.
	fclose(fp);


}

void FilePng::readComments(png_structp & png_ptr, png_infop & info_ptr, FlexVariableMap & properties) {

	/// Read comments
	// mout.debug("reading image comments");
	int num_text = 0;
	png_textp text_ptr = NULL;
	png_get_text(png_ptr, info_ptr,&text_ptr, &num_text);
	//mout.attention();
	for (int i = 0; i < num_text; ++i) {
		// mout.debug2(text_ptr[i].key, '=', text_ptr[i].text);
		// ValueReader::scanValue(text_ptr[i].text, image.properties[text_ptr[i].key]);
		JSON::readValue(text_ptr[i].text, properties[text_ptr[i].key]);
	}


}

void FilePng::readConfiguration(png_structp & png_ptr, png_infop & info_ptr, ImageConf & conf) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const unsigned int inputBitDepth = png_get_bit_depth(png_ptr, info_ptr);

	switch (inputBitDepth) {
	case 16:
		conf.setType<unsigned short>();
		break;
	case 8:
		conf.setType<unsigned char>();
		break;
	default:
	 //////////////	fclose(fp);
		png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
		//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
		throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")); // +path
		return;
	}

	mout.attention("type " , conf);

	//mout.debug2("initialize, type " , image.getType().name() );

	conf.setArea(png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr));

	// This test enables read into an alpha channel.
	switch (png_get_channels(png_ptr, info_ptr)) {
	case 4:
		conf.setChannelCount(3,1);
		break;
	case 3:
		conf.setChannelCount(3,0);
		break;
	case 2:
		conf.setChannelCount(1,1);
		break;
	case 1:
		conf.setChannelCount(1,0);
		break;
	default:
		throw std::runtime_error(std::string("FilePng: invalid channel count in : ")); // + conf);
	}


	// TODO: use png_get_pCal(.........)
	#ifdef PNG_pCAL_SUPPORTED___DEFUNCT
		/// Read physical scaling
		if (info_ptr->pcal_X0 == info_ptr->pcal_X1){
			mout("physical scale supported, but no intensity range, pcalX0=", info_ptr->pcal_X0, ", pcalX1=", info_ptr->pcal_X1);
			//image.setDefaultLimits();
			conf.set...(?);
		}
		else {
			//image.setLimits(info_ptr->pcal_X0, info_ptr->pcal_X1);
			conf.setPhysicalRange();
			mout.note("setting physical scale: " , image );
		}

	#endif


}

void FilePng::copyData(png_structp & png_ptr, png_infop & info_ptr, const ImageConf & pngConf, ImageFrame & image) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	const unsigned int inputBitDepth = 8*pngConf.getElementSize();
	mout.debug("png conf: ", pngConf);

	const unsigned int targetBitDepth = 8*image.getConf().getElementSize();

	const bool from8to8  = (inputBitDepth == 8) && (targetBitDepth ==  8);
	const bool from8to16 = (inputBitDepth == 8) && (targetBitDepth == 16);

	if (from8to8) {
		mout.debug("8-bit input, 8-bit target, easy..." );
		copyData8to8(png_ptr, info_ptr, image);
	}
	else if (from8to16) {
		mout.note("-bit input, 16-bit target, rescaling..." );
		copyData8to16(png_ptr, info_ptr, image);
	}
	else {
		if ((inputBitDepth == 16) && (targetBitDepth == 16)){
			mout.debug("16-bit input, 16-bit target, ok..." );
		}
		else {
			mout.warn(inputBitDepth , "-bit input, ", targetBitDepth , "-bit target, problems ahead?" );
			// png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
			///// png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
			// throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")+path);
		}
		copyData16(png_ptr, info_ptr, image);
	}

}

void FilePng::copyData8to8(png_structp & png_ptr, png_infop & info_ptr, ImageFrame & image) {

	const unsigned int width    = image.getWidth();
	const unsigned int height   = image.getHeight();
	const unsigned int channels = image.getChannelCount();

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	png_bytep p;
	int i0;
	for (unsigned int j = 0; j < height; ++j) {
		p = row_pointers[j];
		for (unsigned int i = 0; i < width; ++i) {
			for (unsigned int k = 0; k < channels; ++k) {
				i0 = channels*i + k;
				image.put(i,j,k, p[i0]);
				/*
				if (from8to8) {
					image.put(i,j,k, p[i0]);
				}
				else if (from8to16) {
					image.put(i,j,k, p[i0]<<8);
				}
				else {
					image.put(i,j,k, (p[i0*2]<<8) + (p[i0*2+1]<<0));
				}
				*/
			}
		}
	}

}

void FilePng::copyData8to16(png_structp & png_ptr, png_infop & info_ptr, ImageFrame & image) {

	const unsigned int width    = image.getWidth();
	const unsigned int height   = image.getHeight();
	const unsigned int channels = image.getChannelCount();

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	png_bytep p;
	int i0;
	for (unsigned int j = 0; j < height; ++j) {
		p = row_pointers[j];
		for (unsigned int i = 0; i < width; ++i) {
			for (unsigned int k = 0; k < channels; ++k) {
				i0 = channels*i + k;
				image.put(i,j,k, p[i0]<<8);
			}
		}
	}

}

void FilePng::copyData16(png_structp & png_ptr, png_infop & info_ptr, ImageFrame & image) {

	const unsigned int width    = image.getWidth();
	const unsigned int height   = image.getHeight();
	const unsigned int channels = image.getChannelCount();

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	png_bytep p;
	int i0;
	for (unsigned int j = 0; j < height; ++j) {
		p = row_pointers[j];
		for (unsigned int i = 0; i < width; ++i) {
			for (unsigned int k = 0; k < channels; ++k) {
				i0 = channels*i + k;
				image.put(i,j,k, (p[i0*2]<<8) + (p[i0*2+1]<<0));
			}
		}
	}

}


void FilePng::read(ImageFrame & image, const std::string & path, int png_transforms ) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug("path='" , path , "'" );

	png_structp  png_ptr = nullptr;
	png_infop info_ptr = nullptr;

	readFile(path, png_ptr, info_ptr, png_transforms);

	readComments(png_ptr, info_ptr, image.properties);

	ImageConf conf;
	readConfiguration(png_ptr, info_ptr, conf);

	image.suggestType(conf.getType());         // Image will always change type
	image.requireGeometry(conf.getGeometry()); // ImageFrame will throw exception

	copyData(png_ptr, info_ptr, conf, image);

	png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL); // confirmed by valgrind

}

/*
void FilePng::readImage(Image & image, const std::string & path, int png_transforms ) {

	drain::Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	mout.debug("path='" , path , "'" );

	png_structp  png_ptr = nullptr;
	png_infop info_ptr = nullptr;

	readFile(path, png_ptr, info_ptr, png_transforms);

	readComments(png_ptr, info_ptr, image.properties);

	ImageConf conf;
	readConfiguration(png_ptr, info_ptr, conf);


	image.initialize(conf.getType(), conf.getGeometry());

	copyData(png_ptr, info_ptr, conf, image);

	png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL); // confirmed by valgrind

}
*/


/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
*/
void FilePng::write(const ImageFrame & image, const std::string & path){

	Logger mout(getImgLog(), __FILE__, __FUNCTION__);

	if (image.isEmpty()){
		mout.warn("empty image, skipping" );
		return;  // -1
	}

	mout.debug("Path: ", path);
	//mout.warn("img: " , image );

	FILE *fp = fopen(path.c_str(), "wb");
	if (!fp){
		//throw std::runtime_error(std::string("FilePng: could not open file : ")+path);
		mout.error("could not open file: ", path);
		return;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL); // ? destroy null?
		fclose(fp);
		mout.error("could not allocate memory for file: " , path );
		//throw std::runtime_error(std::string("FilePng: could not allocate memory for: ")+path);
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
	   png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
	   fclose(fp);
	   mout.error("could not allocate memory for file: " , path );
	   //throw std::runtime_error(std::string("FilePng: could not allocate toOStr memory for: ")+path);
	   return;
	}


	const int width  = image.getWidth();
	const int height = image.getHeight();
	const int channels = image.getChannelCount();

	int color_type = PNG_COLOR_TYPE_GRAY;

	switch (channels) {
	case 4:
		color_type = PNG_COLOR_TYPE_RGBA;
		break;
	case 3:
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case 2:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 1:
		color_type = PNG_COLOR_TYPE_GRAY;
		// FUTURE extension
		if (false){ // image.hasPalette
			color_type = PNG_COLOR_TYPE_PALETTE;
			const int NUM_PALETTE = 256;
			png_color palette[NUM_PALETTE];
			for (int i = 0; i < NUM_PALETTE; ++i) {
				png_color & p = palette[i];
				p.red   = i;
				p.green = 128;
				p.blue  = 255-i;
			}
			//png_color_8 color;
			png_set_PLTE(png_ptr, info_ptr, palette, NUM_PALETTE);
		}
		break;
	case 0:
		mout.warn("zero-channel image" );
		fclose(fp);
		return;
	default:
		mout.error("unsupported channel count: " , channels );
		//throw std::runtime_error(s.toStr());
	}





	// Required.
	png_init_io(png_ptr, fp);

	// Optional.
	/// ? png_set_traverseFrame(png_ptr, 0,PNG_FILTER_HEURISTIC_DEFAULT);

	/*
#ifdef PNG_1_2_X
	png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
#else
	png_set_compression_level(png_ptr, 5);
#endif
	*/

	// Currently, only a global variable used.
	png_set_compression_level(png_ptr, FilePng::compressionLevel);


	const int byte_depth = image.getConf().getElementSize(); //sizeof(T);
	const int bit_depth  = (byte_depth == 1) ? 8 : 16;

	// mout.debug(image.getGeometry() , ", orig byte_depth=" , byte_depth , ", bit_depth=" , bit_depth );
	//mout.debug("orig storage type byte_depth=" , byte_depth , ", using bit_depth=" , bit_depth );
	/*
	if (byte_depth > 2)
		mout.info();
	else */
	mout.debug("source image " , byte_depth , "-byte data, writing " , (bit_depth/8) , "-byte (" , bit_depth , "bit) png" );
	// converting to 2 bytes (16bit uInt).\n" << mout.endl;

	// Set header information
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // interlace_type, compression_type, filter_method


	// Optional
	// png_set_tIME(png_ptr, info_ptr, mod_time);
	// png_set_text(png_ptr, info_ptr, text_ptr, num_text);
	//info_ptr->pcal_
	/*
	png_charp units = (png_charp)"mika";
	png_charp params[1] = {"sika"};
	png_set_pCAL(png_ptr, info_ptr, "mika", 0, 255, 5, 2, units, params);
	info_ptr->pcal_X0 = image.scaling.getMin<png_int_32>();
	info_ptr->pcal_X1 = image.scaling.getMax<png_int_32>();
	info_ptr->pcal_type = PNG_EQUATION_LINEAR;
	info_ptr->pcal_nparams = 0;
	info_ptr->pcal_params = {1.0, 1.0};
	static png_charp units = "meters";
	info_ptr->pcal_units = units;
	*/

	// png_uint_32 resolutionX = image.properties.get("resolutionX", png_uint_32(123) );
	// png_uint_32 resolutionY = image.properties.get("resolutionY", png_uint_32(456) );
	// png_set_pHYs(png_ptr, info_ptr, resolutionX, resolutionY, PNG_RESOLUTION_METER);
#ifdef PNG_sCAL_SUPPORTED
	// Experimental
	double resX = image.properties.get("resolutionX", 1.23 );
	double resY = image.properties.get("resolutionY", 4.56 );
	png_set_sCAL(png_ptr, info_ptr, PNG_RESOLUTION_METER, resX, resY);
#endif

	/*
	png_charp units = (png_charp)"test";
	png_charp params[1] = {"second"};
	png_set_pCAL(png_ptr, info_ptr, "test3", 0, 255, 5, 2, units, params);
	//png_set_pCAL(png_ptr, info_ptr, )
	*/


	// Comment texts (optional)
	mout.debug3("Adding comments " );
	std::map<std::string,std::string> comments;
	comments["Creation_time"] = drain::Time().str();
	// comments["Scaling"] = image.getScaling().toStr(); // FUTURE: only after known (below)
	comments["Software"] = "drain/rack Markus.Peura[c]fmi.fi";
	//for (std::map<std::string,Data>::const_iterator it = image.properties.begin(); it != image.properties.end(); it++){
	/// WARNING: for channels/views: getProperties instead?

	//
	for (const auto & entry: image.properties){
		mout.debug3("properties:", entry.first, '=', entry.second);
		//std::stringstream sstr;
		//drain::Sprinter::toStream(sstr, entry.second, drain::Sprinter::jsonLayout);
		// std::string s = drain::sprinter((const drain::Castable &)entry.second, drain::Sprinter::jsonLayout).str();
		std::string s = drain::sprinter((const drain::Castable &)entry.second, drain::Sprinter::jsonLayout).str();
		mout.debug3(s);
		comments[entry.first] = s; //it->second.toStr();
		//it->second.substr(0,79); MAX SIZE?
	}

	size_t i = 0;
	png_text *text_ptr = new png_text[comments.size()];
	for (const auto & entry: comments){

		if (entry.first.empty()){
			mout.note("Skipping comment with zero length keyword");
			continue;
		}
		text_ptr[i].key  = (char *)entry.first.c_str();
		text_ptr[i].text = (char *)entry.second.c_str();
		text_ptr[i].text_length = entry.second.length();
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
		i++;
	}
	png_set_text(png_ptr, info_ptr, text_ptr, i);

	/// NEW:
	// TODO png_set_shift(png_ptr, &sig_bit);
	// png_write_info(png_ptr, info_ptr);


	// TODO: consider png_write_row and/or &at(row);

	// Create temporary image array.
	mout.debug2("Create temporary image array: h w C bytes: ", height, 'x', width, 'x', channels, 'x', byte_depth);
	png_byte **data;
	data = new png_byte*[height]; //[width*channels*byte_depth];
	for (int j = 0; j < height; ++j) {
		data[j] = new png_byte[width*channels*byte_depth];
	}

	//std::numeric_limits<int>::is_signed();
	const int SHIFT_SIGNED = Type::call<drain::isSigned>(image.getType()) ? 1 : 0;
	if (SHIFT_SIGNED)
		mout.info("signed image data");

	// Copy data to png array.
	mout.debug2("Src: ", image);
	//mout.note("Image of type " , image.getType2() , ", scaling: " , image.getScaling() );
	mout.debug2("Copy data to png array, width=", width, ", height=", height, " channels=", channels);
	png_byte *row;
	for (int k = 0; k < channels; ++k) {

		const Channel & channel = image.getChannel(k);
		mout.debug3(" channel ", k, ": ", channel); //.getConf()

		// 8 bit
		if (byte_depth == 1){
			// mout.debug2("8 bits, height=" , height , " width=" , width );
			for (int j = 0; j < height; ++j) {
				//mout.debug2(" j=" , j );
				row = data[j];
				for (int i = 0; i < width; ++i) {
					//data[j]
					row[i*channels + k] = channel.get<png_byte>(i,j); // << SHIFT_SIGNED;
				}
			}
		}
		// 16 bit
		else if (byte_depth == 2){  // todo: short signed int, with bias 32768?
			int i0, value;
			for (int i = 0; i < width; ++i) {
				i0 = (i*channels + k)*2;
				for (int j = 0; j < height; ++j) {
					value = channel.get<int>(i,j); //  << SHIFT_SIGNED;
					data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
					data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
				}
			}
		}
		// More bytes...
		else {

			//mout.note("Image of type " , image.getType2() , ", scaling: " , image.getScaling() );
			//  << ", coeff=" << coeff; // << byte_depth << " bytes, "

			drain::ValueScaling scalingDst;
			scalingDst.setAbsoluteScale();
			scalingDst.adoptScaling(image.getScaling(), image.getType(), typeid(unsigned short));
			//scalingDst.setOptimalScale();

			drain::ValueScaling conv;
			conv.setConversionScale(image.getScaling(), scalingDst);
			mout.special("Converting [" , drain::Type::getTypeChar(image.getType()) , "] to 16b array, scaling: " , scalingDst );
			mout.debug("Direct conversion: " , conv );

			int i0, value;
			for (int i = 0; i < width; ++i) {
				i0 = (i*channels + k)*2;
				for (int j = 0; j < height; ++j) {
					// value = static_cast<int>(coeff*channel.get<double>(i,j));
					value = static_cast<int>(conv.inv(channel.get<double>(i,j)));
					data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
					data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
				}
			}
		}
	}
	mout.debug2("Setting rows");
	// png_byte row_pointers[height];
	//png_set_rows(png_ptr, info_ptr, row_pointers);
	png_set_rows(png_ptr, info_ptr, data);

	// Main operation
	mout.debug2("Writing array");
	int png_transforms = PNG_TRANSFORM_IDENTITY  || PNG_TRANSFORM_SHIFT;
	png_write_png(png_ptr, info_ptr, png_transforms, NULL);

	mout.debug2("Closing file and freeing memory"); // << mout.endl;
	fclose(fp);
	png_destroy_write_struct(&png_ptr,&info_ptr);
	//png_destroy_info ?? why not

	//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);
	//delete[] row_pointers;
	delete[] text_ptr;
	for (int j = 0; j < height; ++j) {
		delete[] data[j];
	}
	delete[] data;
}


} // image::

} // drain::



// Drain
