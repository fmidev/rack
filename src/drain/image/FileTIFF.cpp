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

/*

   Based on the example code by Niles D. Ritter,
   http://svn.osgeo.org/metacrs/geotiff/trunk/libgeotiff/bin/makegeo.c

 */

#include "FileTIFF.h"


namespace drain {

namespace image {

const FileInfo FileTIFF::fileInfo("(tif|tiff)");
Frame2D<int> FileTIFF::defaultTile(256, 256);

//drain::EnumFlagger<>

#ifdef USE_GEOTIFF_NO
static const FileTIFF::dict_t FileTIFF::compressionDict = {};
/*
	const FileTIFF::dict_t & FileTIFF::getCompressionDict(){
		static FileTIFF::dict_t compressionDict;
		return compressionDict;
	}
 */
FileTIFF::dict_t::value_t FileTIFF::defaultCompression(1); // = NONE, but see below
#else

//static
const FileTIFF::dict_t FileTIFF::compressionDict = {
		{"NONE",     COMPRESSION_NONE},
		{"LZW",      COMPRESSION_LZW},
		{"DEFLATE",  COMPRESSION_DEFLATE},
		{"PACKBITS", COMPRESSION_PACKBITS}
};

	/*
	const FileTIFF::dict_t & FileTIFF::getCompressionDict(){

		static FileTIFF::dict_t compressionDict = {
			{"NONE",     COMPRESSION_NONE},
			{"LZW",      COMPRESSION_LZW},
			{"DEFLATE",  COMPRESSION_DEFLATE},
			{"PACKBITS", COMPRESSION_PACKBITS}
		};

		return compressionDict;
	}
	*/

	FileTIFF::dict_t::value_t FileTIFF::defaultCompression(COMPRESSION_LZW); // = tunable in fileio.cpp


void FileTIFF::setTime(const drain::Time & datetime){
	setField(TIFFTAG_DATETIME, datetime.str("%Y:%m:%d %H:%M:%S") );
}

void FileTIFF::setDefaults(){ //const drain::image::ImageConf & src){ // int tileWidth, int tileHeight){

	drain::Logger mout(__FILE__, __FUNCTION__);
	if (!isOpen()){
		mout.error("TIFF file not open");
	}

	setField(TIFFTAG_COMPRESSION, FileTIFF::defaultCompression);
	setField(TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	setField(TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

}


// Todo: file type select (different from src)
void FileTIFF::writeImageData(const drain::image::Image & src){

	drain::Logger mout(__FILE__, __FUNCTION__);
	if (!isOpen()){
		mout.error("TIFF file not open");
		//return 0;
	}

	const int width  = src.getWidth();
	const int height = src.getHeight();
	int bitspersample = 8;

	setField(TIFFTAG_IMAGEWIDTH,width);
	setField(TIFFTAG_IMAGELENGTH,height);

	const drain::Type type(src.getType());
	mout.debug(" bytes=", drain::Type::call<drain::sizeGetter>(type));
	// switch ((const char)t) {
	bool FLOAT_SRC = false;
	switch (drain::Type::getTypeChar(type)){
		case 'd':
		case 'f':
			// // Floating point format
			FLOAT_SRC = true;
			setField(TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		case 'C':
			// no break
		case 'S':
			bitspersample = 8*drain::Type::call<drain::sizeGetter>(type);
			// TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*drain::Type::call<drain::sizeGetter>(t));
			break;
		default:
			bitspersample = 8;
			//TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn("unsupported storage type=", drain::Type::getTypeChar(type), ", trying 8 bit mode");
	}

	setField(TIFFTAG_BITSPERSAMPLE,   bitspersample);

	/** The number of components per pixel.
	 *  SamplesPerPixel is usually 1 for bilevel, grayscale, and palette-color images.
	 *  SamplesPerPixel is usually 3 for RGB images.
	 *  https://www.awaresystems.be/imaging/tiff/tifftags/samplesperpixel.html
	 */
	setField(TIFFTAG_SAMPLESPERPIXEL, 1);

	//int tileWidth  = tile.getWidth();
	//int tileHeight = tile.getHeight();
	AreaGeometry tileSize(tile);
	if (tileSize.height <= 0){
		tile.height = tile.width;
	}


	if (tileSize.width > 0){

		if (setField(TIFFTAG_TILEWIDTH, tileSize.width)==0){
			mout.warn("invalid tile width=", tile, ", using 256");
			setField(TIFFTAG_TILEWIDTH, 256);
		}

		if (setField(TIFFTAG_TILELENGTH, tileSize.height)==0){
			mout.warn("invalid tile height=", tile, ", using 256");
			setField(TIFFTAG_TILELENGTH, 256);
		}

		/*  debug?
		TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &tileWidth);
		TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
		*/


		// Blocks
		const int W = (width/tileSize.width);
		const int H = (height/tileSize.height);

		const bool UCHAR8 = (bitspersample==8); // ? 1 : 2; // 8 or 16 bit data

		drain::image::Image tile;
		tile.setType(type); // same as src data
		tile.setGeometry(tileSize.width, tileSize.height);

		//mout.info("Using tiling: ", tileSize.width, 'x', tileSize.height);
		mout.info("tiled mode:"  , tile , ", bits=" , bitspersample );


		// const drain::ValueScaling & srcScaling = src.getScaling();
		// double f;
		if (FLOAT_SRC){
			mout.experimental("Tiled write for non-integer type: ", drain::Type::call<compactName>(type));
			mout.attention(tile);
		}

		if ((!FLOAT_SRC) && (!UCHAR8) && (width % tileSize.width)){
			mout.hint("16bit image, width != N*tileWidth (" , tileSize.width ,"), edge rendering errors in some applications (libgeotiff issue?)" );
		}


		/// current tile-widths
		int w;
		int h;
		const int wPartial = width  % tileSize.width;
		const int hPartial = height % tileSize.height;
		int iOffset;
		int jOffset;
		for (int l=0; l<=H; ++l){

			jOffset = l*tileSize.height;
			if (l<H)
				h = tileSize.height;
			else
				h = hPartial;

			for (int k=0; k<=W; ++k){

				iOffset = k*tileSize.width;
				if (k<W)
					w = tileSize.width;
				else // last tile is partial
					w = wPartial;


				// Copy image data to tile
				if ((w>0) && (h>0)){
					//if (!UCHAR8){tile.setGeometry(w, h);}
					mout.debug2("TILE:" , k , ',' ,  l , '\t' , w , 'x' , h );
					if (FLOAT_SRC){
						for (int j=0; j<h; ++j){
							for (int i=0; i<w; ++i){
								//buffer[j*tileWidth + i] = src.get<int>(iOffset+i, jOffset+j);
								//tile.put(i,j, srcScaling.fwd(src.get<double>(iOffset+i, jOffset+j)));
								/*
								f = src.get<double>(iOffset+i, jOffset+j);
								f = std::max(-10.0, f);
								f = std::min(f, 200.0);
								tile.put(i,j, f);
								*/
								tile.put(i,j, src.get<double>(iOffset+i, jOffset+j));
							}
						}
					}
					else {
						for (int j=0; j<h; ++j){
							for (int i=0; i<w; ++i){
								//buffer[j*tileWidth + i] = src.get<int>(iOffset+i, jOffset+j);
								tile.put(i,j, src.get<int>(iOffset+i, jOffset+j));
							}
						}
					}

					if(!TIFFWriteTile(tif, tile.getBuffer(), iOffset, jOffset, 0, 0)){
						TIFFError("WriteImage", "TIFFWriteTile failed \n");
					}

					//if (!UCHAR8){tile.setGeometry(tileHeight, tileHeight);}

				}
			}
		}

	}
	else {

		mout.info("direct mode (no tiling), writing row by row, bits=" , bitspersample);

		setField(TIFFTAG_ROWSPERSTRIP, 20L);

		unsigned char *buffer = nullptr;

		// const drain::Type t(src.getType());
		// if ((t == 'C') || (t=='S')){
		if (!FLOAT_SRC){
			/// Address each ŕow directly
			const int rowBytes = width * src.getConf().getElementSize();
			buffer = (unsigned char *)src.getBuffer();
			for (int j=0; j<height; ++j){
				if (!TIFFWriteScanline(tif, &(buffer[j * rowBytes]), j, 0))
					TIFFError(__FUNCTION__, "failure in direct WriteScanline\n");
			}
		}
		else {
			mout.unimplemented<LOG_ERR>("Direct (no tiling) write for non-integer type: ", drain::Type::call<compactName>(type));

			/// Copy each ŕow to buffer
			// buffer = new unsigned char[width*1]; // 8 bits

			drain::image::Image line;
			line.setType(type); // same as src data
			line.setGeometry(width, 1);
			buffer = (unsigned char *)line.getBuffer();

			for (int j=0; j<height; ++j){
				for (int i=0; i<width; ++i){
					line.put(i, src.get<int>(i,j));
					// buffer[i] = src.get<int>(i,j);
				}
					//buffer[i] = src.get<int>(i,j);
				if (!TIFFWriteScanline(tif, buffer, j, 0))
					TIFFError(__FUNCTION__, "failure in buffered WriteScanline\n");
			}
			delete buffer;
		}

	}


}

/** Writes drain::Image to a png image file applying G,GA, RGB or RGBA color model.
 *  Writes in 8 or 16 bits, according to template class.
 *  Floating point images will be scaled as 16 bit integral (unsigned short int).
 */
//void FileGeoTIFF::write(const std::string &filePath, const Hi5Tree & src, const ODIMPathList & paths){
void FileTIFF::write(const std::string &path, const drain::image::Image & src){

	// drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.note(src.properties );

	FileTIFF file(path, "w");
	file.setDefaults();
	file.writeImageData(src); //, tileSize, tileSitileWidthze/2);

	// std::string projstr = src.properties["where:projdef"];


}


#endif

} // image::

} // drain::



