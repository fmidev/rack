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

#include "FilePng.h"
#include "util/Time.h"
/*

*/

namespace drain
{
namespace image
{

// using namespace std;



/** Reads a png file to drain::Image.
 *  Converts indexed (palette) images to RGB or RGBA.
 *  Scales data to 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 *
 */
void FilePng::read(Image & image, const std::string & path, int png_transforms ) {

	drain::MonitorSource mout(drain::image::iMonitor, __FILE__, __FUNCTION__);

	mout.info() << "path='" << path << "'" << mout.endl;


	// Try to open the file
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		throw std::runtime_error(std::string("FilePng: could not open file: ") + path);

	// For checking magic code (signature)
	//const unsigned int PNG_BYTES_TO_CHECK=4;
	const size_t PNG_BYTES_TO_CHECK=4;
	png_byte buf[PNG_BYTES_TO_CHECK];

	/* Read in some of the signature bytes */
	if (fread((void *)buf, size_t(1), PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
		throw std::runtime_error(std::string("FilePng: suspicious size of file: ") + path);

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	   Return nonzero (true) if they match */
	if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0)
		throw std::runtime_error(std::string("FilePng: not a png file: ")+path);



	png_structp  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		throw std::runtime_error(std::string("FilePng: problem in allocating image memory for: ")+path);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
	    png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
		throw std::runtime_error(std::string("FilePng: problem in allocating info memory for: ")+path);
	}

	/*
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info){
	    png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);
	    throw std::runtime_error(std::string("FilePng: problem in allocating end_info memory for: ")+path);
	}
	*/

	// This may be unstable. According to the documentation, if one uses the high-level interface png_read_png()
	// one can only configure it with png_transforms flags (PNG_TRANSFORM_*)
	png_set_palette_to_rgb(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

	/// Main action
	//if (drain::Debug > 2)
	mout.debug(2) << "reading data" << mout.endl;

	png_read_png(png_ptr, info_ptr, png_transforms, NULL);





	/// Read comments
	mout.debug(2) << "reading image comments" << mout.endl;
	int num_text = 0;
	png_textp text_ptr = NULL;
	png_get_text(png_ptr, info_ptr,&text_ptr, &num_text);
	//mout.debug(2) << '\n';
	for (int i = 0; i < num_text; ++i) {
		mout << text_ptr[i].key << '=' << text_ptr[i].text << '\n';
		image.properties[text_ptr[i].key] = (const char *)text_ptr[i].text;
	}
	mout << mout.endl;

	/// Copy to drain::Image
	const unsigned int width  = png_get_image_width(png_ptr, info_ptr);
	const unsigned int height = png_get_image_height(png_ptr, info_ptr);
	const unsigned int channels = png_get_channels(png_ptr, info_ptr);
	const unsigned int bit_depth = png_get_bit_depth(png_ptr, info_ptr);


	switch (bit_depth) {
	case 16:
		//image.initialize<unsigned short>();
		image.setType<unsigned short>();
		break;
	case 8:
		image.setType<unsigned char>();
		break;
	default:
		fclose(fp);
		png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
		//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
		throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")+path);
		return;
	}

	// TODO: use png_get_pCal(.........)
#ifdef PNG_pCAL_SUPPORTED___DEFUNCT
	/// Read physical scaling
	if (info_ptr->pcal_X0 == info_ptr->pcal_X1){
		mout.info() << "physical scale supported, but no intensity range, pcalX0=" <<  info_ptr->pcal_X0 << ", pcalX1=" <<  info_ptr->pcal_X1  << mout.endl;
		image.setDefaultLimits();
	}
	else {
		image.setLimits(info_ptr->pcal_X0, info_ptr->pcal_X1);
		mout.note() << "setting physical scale: " << image << mout.endl;
	}

#endif


	mout.info() << "initialize, type " << image.getType().name() << mout.endl;

	// This test enables read into an alpha channel.
	if  ((channels!=image.getChannelCount())||(width!=image.getWidth())||(height!=image.getHeight())){
		switch (channels) {
		case 4:
			image.setGeometry(width,height,3,1);
			break;
		case 3:
			image.setGeometry(width,height,3);
			break;
		case 2:
			image.setGeometry(width,height,1,1);
			break;
		case 1:
			image.setGeometry(width,height,1);
			break;
		default:
			throw std::runtime_error(std::string("FilePng: invalid channel count in : ")+path);
		}
	}


	mout.debug(2) << "png geometry ok, ";
	mout << "png channels =" << channels << "\n";
	mout << "png bit_depth=" << bit_depth << "\n";
	mout << mout.endl;

	/*
	 if ((bit_depth!=8) && (bit_depth != 16)){
		fclose(fp);
		png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
		//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???
		throw std::runtime_error(std::string("FilePng: unsupported bit depth in : ")+path);
	}
	*/

	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	png_bytep p;
	int i0;
	for (unsigned int j = 0; j < height; ++j) {
		p = row_pointers[j];
		for (unsigned int i = 0; i < width; ++i) {
			for (unsigned int k = 0; k < channels; ++k) {
				i0 = channels*i + k;
				if (bit_depth == 8) {
					//image.at(i,j,k) = p[i0];
					image.put(i,j,k, p[i0]);
				}
				else {
					image.put(i,j,k, (p[i0*2]<<8) + (p[i0*2+1]<<0));
					//image.at(i,j,k) = p[i0*2] + (p[i0*2+1]<<8);
				}
			}
		}
	}

	fclose(fp);
	png_destroy_read_struct(&png_ptr,&info_ptr, (png_infopp)NULL);
	//png_free_data(png_ptr,info_ptr,PNG_FREE_ALL,-1);  // ???

	//png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
	//png_destroy_info_struct(png_ptr,&info_ptr);


}

/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
*/
void FilePng::write(const Image & image, const std::string & path){

	MonitorSource mout(iMonitor, "FilePng", __FUNCTION__);

	if (image.isEmpty()){
		mout.warn() << "empty image, skipping" << mout.endl;
		return;  // -1
	}

	FILE *fp = fopen(path.c_str(), "wb");
	if (!fp){
		//throw std::runtime_error(std::string("FilePng: could not open file : ")+path);
		mout.error() << "could not open file :" << path << mout.endl;
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
		break;
	case 0:
		mout.warn() << "zero-channel image" << mout.endl;
		fclose(fp);
		return;
	default:
		mout.error()  << "unsupported channel count: " << channels << mout.endl;
		//throw std::runtime_error(s.toStr());
	}


	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		fclose(fp);
		mout.error() << "could not allocate memory for file: " << path << mout.endl;
		//throw std::runtime_error(std::string("FilePng: could not allocate memory for: ")+path);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
	   png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
	   fclose(fp);
	   mout.error() << "could not allocate memory for file: " << path << mout.endl;
	   //throw std::runtime_error(std::string("FilePng: could not allocate info memory for: ")+path);
	}

	//info_ptr->pcal_

	// Required.
	png_init_io(png_ptr, fp);

	// Optional.
	png_set_filter(png_ptr, 0,PNG_FILTER_HEURISTIC_DEFAULT);

	// Optional
#ifdef PNG_1_2_X
	png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
#else
	png_set_compression_level(png_ptr, 5);
#endif


	const int byte_depth = image.getByteSize(); //sizeof(T);
	const int bit_depth  = byte_depth <= 2 ? byte_depth*8 : 16;

	mout.debug() << image.getGeometry() << ", png byte_depth=" << byte_depth << ", bit_depth=" << bit_depth << mout.endl;
	if (byte_depth > 2)
		mout.note() << "source image " << byte_depth << " byte data, converting to 2 bytes (16bit uInt).\n" << mout.endl;

	// Set header information
	png_set_IHDR(png_ptr, info_ptr, width, height,bit_depth, color_type,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // interlace_type, compression_type, filter_method

	// Optional
	// png_set_tIME(png_ptr, info_ptr, mod_time);
	// png_set_text(png_ptr, info_ptr, text_ptr, num_text);

	// Comment texts (optional)
	mout.debug(2) << "Adding comments " << mout.endl;
	std::map<std::string,std::string> comments;
	comments["Creation_time"] = drain::Time().str();
	comments["Software"] = "drain/image/FilePng Markus.Peura[c]fmi.fi";
	//for (std::map<std::string,Data>::const_iterator it = image.properties.begin(); it != image.properties.end(); it++){
	for (VariableMap::const_iterator it = image.properties.begin(); it != image.properties.end(); it++){
		comments[it->first] = it->second.toStr();
				//it->second.substr(0,79);
	}
	size_t i = 0;
	//png_text text_ptr[comments.size()];
	png_text *text_ptr = new png_text[comments.size()];
	for (std::map<std::string,std::string>::const_iterator it = comments.begin(); it != comments.end(); it++){
		// std::cout << "PngFile:" << it->first << ':' << it->second << '\n';
		text_ptr[i].key  = (char *)it->first.c_str();
		text_ptr[i].text = (char *)it->second.c_str();
		text_ptr[i].text_length = it->second.length();
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
		i++;
	}
	png_set_text(png_ptr, info_ptr, text_ptr, i);

	/// NEW:
	// TODO png_set_shift(png_ptr, &sig_bit);
	// png_write_info(png_ptr, info_ptr);


	// TODO: consider png_write_row and/or &at(row);

	// Create temporary image array.
	mout.debug(2) << "Create temporary image array " << mout.endl;
	png_byte **data;
	data = new png_byte*[height]; //[width*channels*byte_depth];
	for (int j = 0; j < height; ++j) {
		data[j] = new png_byte[width*channels*byte_depth];
	}

	// Copy data to png array.
	mout.debug(1) << "Copy data to png array." << mout.endl;
	mout.debug(2) << " width=" << width << ", height=" << height << " channels=" << channels << mout.endl;
	for (int k = 0; k < channels; ++k) {
		//const double coeff = image.get<png_byte>(i,j,k);
		const Image & channel = image.getChannel(k);
		const double coeff = static_cast<double>((byte_depth==1) ? 0xff : 0xffff) * channel.getScale();

		// 8 bit
		if (byte_depth == 1){
			// mout.debug(1) << "8 bits, height=" << height << " width=" << width << mout.endl;
			for (int j = 0; j < height; ++j) {
				//mout.debug(1) << " j=" << j << mout.endl;
				for (int i = 0; i < width; ++i) {
					//data[j][i*channels + k] = static_cast<png_byte>(image.at(i,j,k));
					data[j][i*channels + k] = image.get<png_byte>(i,j,k);
				}
			}
		}
		// 16 bit
		else if (byte_depth == 2){  // todo: short signed int, with bias 32768?
			int i0, value;
			for (int i = 0; i < width; ++i) {
				i0 = (i*channels + k)*2;
				for (int j = 0; j < height; ++j) {
					value = image.get<int>(i,j,k);
					data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
					data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
				}
			}
		}
		// More bytes...
		else {
			const Image & channel = image.getChannel(k);
			int i0, value;
			for (int i = 0; i < width; ++i) {
				i0 = (i*channels + k)*2;
				for (int j = 0; j < height; ++j) {
					value = static_cast<int>(coeff*channel.get<double>(i,j));
					data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
					data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
				}
			}
		}
	}
	mout.debug(2) << "Setting rows" << mout.endl;
	// png_byte row_pointers[height];
	//png_set_rows(png_ptr, info_ptr, row_pointers);
	png_set_rows(png_ptr, info_ptr, data);

	// Main operation
	mout.debug(1) << "Writing array" << mout.endl;
	int png_transforms = PNG_TRANSFORM_IDENTITY  || PNG_TRANSFORM_SHIFT;
	png_write_png(png_ptr, info_ptr, png_transforms, NULL);

	mout.debug(1) << "Closing file and freeing memory" << mout.endl;
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

}

}



// Drain
