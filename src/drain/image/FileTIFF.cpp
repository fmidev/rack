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

	// drain::Logger mout("FileGeoTIFF", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!isOpen()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("TIFF file not open");
		//return 0;
	}

	setField(TIFFTAG_COMPRESSION, FileTIFF::defaultCompression);
	setField(TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK);
	setField(TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);


	/*
	setField(TIFFTAG_IMAGEWIDTH,width);
	setField(TIFFTAG_IMAGELENGTH,height);
	const drain::Type t(src.getType());
	mout.debug(" bytes=" , drain::Type::call<drain::sizeGetter>(t) );
	switch ((const char)t) {
		case 'C':
			// no break
		case 'S':
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*drain::Type::call<drain::sizeGetter>(t));
			break;
		default:
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn("unsupported storage type=" ,  drain::Type::getTypeChar(t) , ", trying 8 bit mode");
	}
	*/

}



void FileTIFF::writeImageData(const drain::image::Image & src) //, int tileWidth = 0, int tileHeight = 0)
{

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!isOpen()){
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.error("TIFF file not open");
		//return 0;
	}

	const int width  = src.getWidth();
	const int height = src.getHeight();
	int bitspersample = 8;

	setField(TIFFTAG_IMAGEWIDTH,width);
	setField(TIFFTAG_IMAGELENGTH,height);

	const drain::Type t(src.getType());
	mout.debug(" bytes=", drain::Type::call<drain::sizeGetter>(t));
	switch ((const char)t) {
		case 'C':
			// no break
		case 'S':
			bitspersample = 8*drain::Type::call<drain::sizeGetter>(t);
			// TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*drain::Type::call<drain::sizeGetter>(t));
			break;
		default:
			bitspersample = 8;
			//TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn("unsupported storage type=", drain::Type::getTypeChar(t), ", trying 8 bit mode");
	}

	setField(TIFFTAG_BITSPERSAMPLE,   bitspersample);

	/** The number of components per pixel.
	 *  SamplesPerPixel is usually 1 for bilevel, grayscale, and palette-color images.
	 *  SamplesPerPixel is usually 3 for RGB images.
	 *  https://www.awaresystems.be/imaging/tiff/tifftags/samplesperpixel.html
	 */
	setField(TIFFTAG_SAMPLESPERPIXEL, 1);

	int tileWidth  = tile.getWidth();
	int tileHeight = tile.getHeight();


	if (tileWidth > 0){

		if (setField(TIFFTAG_TILEWIDTH, tileWidth)==0){
			mout.warn("invalid tile width=", tile, ", using 256");
			setField(TIFFTAG_TILEWIDTH, 256);
		}

		if (tileHeight <= 0){
			tileHeight = tileWidth;
		}

		if (setField(TIFFTAG_TILELENGTH, tileHeight)==0){
			mout.warn("invalid tile height=", tile, ", using 256");
			setField(TIFFTAG_TILELENGTH, 256);
		}
		mout.info("Using tiling: ", tileWidth, 'x', tileHeight);
	}
	else {
		mout.info("No tiling, writing row by row");
		setField(TIFFTAG_ROWSPERSTRIP, 20L);
	}


	unsigned char *buffer = nullptr;
	//unsigned short int *buffer16b = NULL;



	/*
	TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &tileWidth);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	*/

	if (tileWidth > 0){

		if (tileHeight == 0)
			tileHeight = tileWidth;

		const int W = (width/tileWidth);
		const int H = (height/tileHeight);

		const bool UCHAR8 = (bitspersample==8); // ? 1 : 2; // 8 or 16 bit data

		drain::image::Image tile;
		if (UCHAR8)
			tile.setType<unsigned char>();
		else
			tile.setType<unsigned short int>();
		tile.setGeometry(tileWidth, tileHeight);

		mout.info("tiled mode:"  , tile , ", bits=" , bitspersample );

		if ((!UCHAR8) && (width % tileWidth)){
			mout.hint("16bit image, width != N*tileWidth (" , tileWidth ,"), edge rendering errors in some applications (libgeotiff issue?)" );
		}

		/// current tile-widths
		int w;
		int h;
		const int wPartial = width  % tileWidth;
		const int hPartial = height % tileHeight;
		int iOffset;
		int jOffset;
		for (int l=0; l<=H; ++l){

			jOffset = l*tileHeight;
			if (l<H)
				h = tileHeight;
			else
				h = hPartial;

			for (int k=0; k<=W; ++k){

				iOffset = k*tileWidth;
				if (k<W)
					w = tileWidth;
				else // last tile is partial
					w = wPartial;


				// Copy image data to tile
				if ((w>0) && (h>0)){
					//if (!UCHAR8){tile.setGeometry(w, h);}
					mout.debug2("TILE:" , k , ',' ,  l , '\t' , w , 'x' , h );
					for (int j=0; j<h; ++j){
						for (int i=0; i<w; ++i){
							//buffer[j*tileWidth + i] = src.get<int>(iOffset+i, jOffset+j);
							tile.put(i,j, src.get<int>(iOffset+i, jOffset+j));
						}
					}
					/*
					if ((k==W) || (l==H)){
						std::stringstream s;
						s << "tile" << l << k << ".png";
						drain::image::File::write(tile, s.str());
					}
					*/
					if(!TIFFWriteTile(tif, tile.getBuffer(), iOffset, jOffset, 0, 0)){
						TIFFError("WriteImage", "TIFFWriteTile failed \n");
					}

					//if (!UCHAR8){tile.setGeometry(tileHeight, tileHeight);}

				}
			}
		}

	}
	else {

		const drain::Type t(src.getType());
		if ((t == 'C') || (t=='S')){
			/// Address each ŕow directly
			const int rowBytes = width * src.getConf().getElementSize();
			buffer = (unsigned char *)src.getBuffer();
			for (int j=0; j<height; ++j){
				if (!TIFFWriteScanline(tif, &(buffer[j * rowBytes]), j, 0))
					TIFFError(__FUNCTION__, "failure in direct WriteScanline\n");
			}
		}
		else {
			/// Copy each ŕow to buffer
			buffer = new unsigned char[width*1]; // 8 bits
			for (int j=0; j<height; ++j){
				for (int i=0; i<width; ++i)
					buffer[i] = src.get<int>(i,j);
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



