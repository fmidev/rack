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


#include <drain/util/Frame.h>

#include "FileTIFF.h"




#include <drain/util/Log.h>
#include <drain/util/StringBuilder.h>
#include <drain/util/Time.h>
#include "drain/util/TreeXML.h"
#include <drain/image/ImageFile.h>
#include "drain/image/AccumulatorGeo.h"
#include "main/rack.h"


#include <proj_api.h>

namespace drain::image {

	const FileInfo FileTIFF::fileInfo("(tif|tiff)");
	Frame2D<int> FileTIFF::defaultTile(256, 256);

	const drain::Dictionary2<int, std::string> & FileTIFF::getCompressionDict(){

		static drain::Dictionary2<int, std::string> compressionDict;

#ifdef USE_GEOTIFF_YES

		if (compressionDict.empty()){
			// Populate
			compressionDict.add(COMPRESSION_NONE,     "NONE");
			compressionDict.add(COMPRESSION_LZW,      "LZW");
			compressionDict.add(COMPRESSION_DEFLATE,  "DEFLATE");
			compressionDict.add(COMPRESSION_PACKBITS, "PACKBITS");
		}
#endif

		return compressionDict;
	}

#ifndef USE_GEOTIFF_YES

	int FileTIFF::defaultCompression(1); // = NONE, but see below

#else

	int FileTIFF::defaultCompression(COMPRESSION_LZW); // = tunable in fileio.cpp



//drain::Variable FileGeoTIFF::ties(typeid(double));
void FileTIFF::setTileSize(int tileWidth, int tileHeight){

	drain::Logger mout(__FILE__, __FUNCTION__);

	// write as tiles
	if (tileWidth > 0){

		if (tileHeight <= 0)
			tileHeight = tileWidth;

		if (setField(TIFFTAG_TILEWIDTH,tileWidth)==0){
			mout.warn("invalid tileWidth=", tileWidth, ", using 256");
			setField(TIFFTAG_TILEWIDTH,256);
		}

		if (setField(TIFFTAG_TILELENGTH,tileHeight)==0){
			mout.warn("invalid tileWidth=", tileHeight, ", using 256");
			setField(TIFFTAG_TILELENGTH,256);
		}

	}
	else {
		setField(TIFFTAG_ROWSPERSTRIP,20L);
	}

}

void FileTIFF::setTime(const drain::Time & datetime){
	setField(TIFFTAG_DATETIME,datetime.str("%Y:%m:%d %H:%M:%S") );
}

void FileTIFF::setUpTIFFDirectory(const drain::image::ImageConf & src){ // int tileWidth, int tileHeight){

	// drain::Logger mout("FileGeoTIFF", __FUNCTION__);
	drain::Logger mout(__FILE__, __FUNCTION__);

	//const drain::FlexVariableMap & prop = src.properties;

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();

	setField(TIFFTAG_IMAGEWIDTH,width);
	//TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,    width);
	setField(TIFFTAG_IMAGELENGTH,height);
	//TIFFSetField(tif,TIFFTAG_COMPRESSION,   COMPRESSION_NONE);
	setField(TIFFTAG_COMPRESSION,FileTIFF::defaultCompression);
	setField(TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK);
	setField(TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);


	const drain::Type t(src.getType());
	mout.debug() << " bytes=" << drain::Type::call<drain::sizeGetter>(t) << mout.endl;
	switch ((const char)t) {
		case 'C':
			// no break
		case 'S':
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8*drain::Type::call<drain::sizeGetter>(t));
			break;
		default:
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
			mout.warn() << "unsupported storage type=" <<  drain::Type::getTypeChar(t) << ", trying 8 bit mode"<< mout.endl;
	}

	//setTileSize(tif, tileWidth, tileHeight);
	/*
	// write as tiles
	if (tileWidth > 0){

		if (tileHeight <= 0)
			tileHeight = tileWidth;

		if (TIFFSetField(tif, TIFFTAG_TILEWIDTH,  tileWidth)==0){
			mout.warn() << "invalid tileWidth=" << tileWidth << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILEWIDTH, 256);
		}

		if (TIFFSetField(tif,TIFFTAG_TILELENGTH,  tileHeight)==0){
			mout.warn() << "invalid tileWidth=" << tileHeight << ", using 256"<< mout.endl;
			TIFFSetField(tif, TIFFTAG_TILELENGTH, 256);
		}

	}
	else {
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,  20L);
	}
	*/
}



void FileTIFF::writeImageData(const drain::image::Image & src) //, int tileWidth = 0, int tileHeight = 0)
{

	drain::Logger mout(__FILE__, __FUNCTION__);

	const int width  = src.getWidth();
	const int height = src.getHeight();

	unsigned char *buffer = NULL;
	//unsigned short int *buffer16b = NULL;

	int tileWidth = 0;
	int tileHeight = 0;
	int bitspersample = 8;

	TIFFGetField(tif, TIFFTAG_TILEWIDTH,  &tileWidth);
	TIFFGetField(tif, TIFFTAG_TILELENGTH, &tileHeight);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

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

		mout.info() << "tiled mode:"  << tile << ", bits=" << bitspersample << mout.endl;

		if ((!UCHAR8) && (width % tileWidth)){
			mout.warn() << "16bit image, width != N*tileWidth (" << tileWidth <<"), errors may occur (libgeotiff problem?)" << mout.endl;
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
					mout.debug2() << "TILE:" << k << ',' <<  l << '\t' << w << 'x' << h << mout.endl;
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

#endif



}



