/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy  of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "FilePng.h"
//#include "util/RegExp.h"
#include "util/Time.h"


namespace drain
{

namespace image
{

/// Syntax for recognising image files
const drain::RegExp FilePng::fileNameRegExp("^((.*/)?([^/]+))\\.(png)$", REG_EXTENDED | REG_ICASE);

template <>
void FilePng::initialize(Image & image, const std::type_info & t, const Geometry & g){
	image.initialize(t,g);
}

template <>
void FilePng::initialize(ImageFrame & image, const std::type_info & t, const Geometry & g){

	if ((image.getType() == t) && (image.getGeometry() == g))
		return;
	else
		std::runtime_error(std::string("FilePng::")+__FUNCTION__+": tried to change ImageFrame geometry");

}




/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
*/
void FilePng::write(const ImageFrame & image, const std::string & path){

	Logger mout(getImgLog(), "FilePng", __FUNCTION__);

	if (image.isEmpty()){
		mout.warn() << "empty image, skipping" << mout.endl;
		return;  // -1
	}

	mout.debug() << "Path: " << path << mout.endl;
	//mout.warn()  << "img: " << image << mout.endl;

	FILE *fp = fopen(path.c_str(), "wb");
	if (!fp){
		//throw std::runtime_error(std::string("FilePng: could not open file : ")+path);
		mout.error() << "could not open file: " << path << mout.endl;
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
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL); // ? destroy null?
		fclose(fp);
		mout.error() << "could not allocate memory for file: " << path << mout.endl;
		//throw std::runtime_error(std::string("FilePng: could not allocate memory for: ")+path);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
	   png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
	   fclose(fp);
	   mout.error() << "could not allocate memory for file: " << path << mout.endl;
	   //throw std::runtime_error(std::string("FilePng: could not allocate toOStr memory for: ")+path);
	}


	// Required.
	png_init_io(png_ptr, fp);

	// Optional.
	/// ? png_set_traverseFrame(png_ptr, 0,PNG_FILTER_HEURISTIC_DEFAULT);

	// Optional
#ifdef PNG_1_2_X
	png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
#else
	png_set_compression_level(png_ptr, 5);
#endif



	const int byte_depth = image.getByteSize(); //sizeof(T);
	const int bit_depth  = byte_depth <= 2 ? byte_depth*8 : 16;

	// mout.debug() << image.getGeometry() << ", orig byte_depth=" << byte_depth << ", bit_depth=" << bit_depth << mout.endl;
	//mout.debug() << "orig storage type byte_depth=" << byte_depth << ", using bit_depth=" << bit_depth << mout.endl;
	/*
	if (byte_depth > 2)
		mout.info();
	else */
	mout.debug();
	mout << "source image " << byte_depth << "byte data, writing " << (bit_depth/8) << "byte (" << bit_depth << "bit) png" << mout.endl;
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
	*/

	//png_set_pCAL(png_ptr, info_ptr, )

	// Comment texts (optional)
	mout.debug(2) << "Adding comments " << mout.endl;
	std::map<std::string,std::string> comments;
	comments["Creation_time"] = drain::Time().str();
	// comments["Scaling"] = image.getScaling().toStr(); // FUTURE: only after known (below)
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

	//std::numeric_limits<int>::is_signed();
	const int SHIFT_SIGNED = Type::call<drain::isSigned>(image.getType()) ? 1 : 0;
	if (SHIFT_SIGNED)
		mout.info() << "signed image data" << mout.endl;

	// Copy data to png array.
	mout.info() << "Src: " << image << mout.endl;
	//mout.note() << "Image of type " << image.getType2() << ", scaling: " << image.getScaling() << mout.endl;
	mout.debug(1) << "Copy data to png array, width=" << width << ", height=" << height << " channels=" << channels << mout.endl;
	for (int k = 0; k < channels; ++k) {
		//const double coeff = image.get<png_byte>(i,j,k);
		const Channel & channel = image.getChannel(k);
		//mout.note()  << "im: " << image   << mout.endl;
		//mout.note()  << "ch: " << channel << mout.endl;
		//const double coeff = static_cast<double>((byte_depth==1) ? 0xff : 0xffff) * channel.scaling.getScale();
		// mout.debug(1) << " channel " << k << ", coeff=" << coeff << " scaling=" << channel.scaling.toStr() << mout.endl;
		mout.debug(2) << " channel " << k << " scaling=" << channel.getScaling() << mout.endl;

		// 8 bit
		if (byte_depth == 1){
			// mout.debug(1) << "8 bits, height=" << height << " width=" << width << mout.endl;
			for (int j = 0; j < height; ++j) {
				//mout.debug(1) << " j=" << j << mout.endl;
				for (int i = 0; i < width; ++i) {
					//data[j][i*channels + k] = static_cast<png_byte>(image.at(i,j,k));
					//data[j][i*channels + k] = image.get<png_byte>(i,j,k);
					data[j][i*channels + k] = channel.get<png_byte>(i,j) << SHIFT_SIGNED;
				}
			}
		}
		// 16 bit
		else if (byte_depth == 2){  // todo: short signed int, with bias 32768?
			int i0, value;
			for (int i = 0; i < width; ++i) {
				i0 = (i*channels + k)*2;
				for (int j = 0; j < height; ++j) {
					//value = image.get<int>(i,j,k);
					value = channel.get<int>(i,j) << SHIFT_SIGNED;
					data[j][i0+1] = static_cast<png_byte>( value     & 0xff);
					data[j][i0  ] = static_cast<png_byte>((value>>8) & 0xff);
				}
			}
		}
		// More bytes...
		else {

			//mout.note() << "Image of type " << image.getType2() << ", scaling: " << image.getScaling() << mout.endl;
			//  << ", coeff=" << coeff; // << byte_depth << " bytes, "

			ImageScaling scalingDst;
			scalingDst.setAbsoluteScale();
			scalingDst.adoptScaling(image.getScaling(), image.getType(), typeid(unsigned short));
			//scalingDst.setOptimalScale();

			ImageScaling conv;
			conv.setConversionScale(image.getScaling(), scalingDst);
			mout.info() << "Converting [" << drain::Type::getTypeChar(image.getType()) << "] to 16b array, scaling: " << scalingDst << mout.endl;
			mout.debug() << "Direct conversion: " << conv << mout.endl;

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


