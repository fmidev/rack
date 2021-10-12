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

#ifndef __RACK_API__
#define __RACK_API__
#define __RACK__ "Rack_fmi.fi"
#define __RACK_VERSION__ "6.29"

/**
 *   Version 6.29
 *   -- paletteRefine can (and should) be given before applying \c --palette.
 *
 *   Version 6.26
 *   -- Added VRAD-VRADH flexibility to Doppler products
 *
 *   Version 6.25
 *   -- Image geometry accepts (area, channels, alphachannels)
 *   -- Palette cmd preserves alpha channel (by copying it)
 *
 *   Version 6.24
 *   -- Restored --execScript (yet not often needed)
 *   -- Bug fix in CommandInstaller, cmd section assignment -> logical OR

 *   Version 6.23
 *   -- Bug fix in where:lat, where:lon (originated from SmartMapTools)
 *
 *   Version 6.22
 *   -- Bug fix in --cBBox and --cProj order problem
 *
 *   Version 6.21
 *   -- how:angles listed in Cartesian products (ODIM 2.3), also in polar products.
 *   -- fixed VRAD/VRADH problem in pDopplerAvg
 *
 *   Version 6.2
 *   -- Revised logic in selecting (quality) data for image processing operators( -h imageOps )
 *
 *   Version 6.0
 *   -- Revised program core (drain::CommandBank, drain::Program, drain::Script)
 *   -- Parallel processing by OpenMP, introducing '[' and ']' on command line
 *   -- Revised anomaly detection and removal
 *   -- Migration to C++11 standard started
 *
 *   Version 5.63
 *   - --setODIM: support indexed search, like: --/dataset1:5/data3/what:quanity=DBZH
 *
 *   Version 5.62
 *   - Sample command: iRange,jRange =>  i,j to support sampling a single position
 *   - PGM comment override option
 *
 *   Version 5.61
 *   - More support for ODIM 2.3: what:source WIGOS
 *
 *   Version 5.6
 *   - Initial tests for parallel computation (openmp)
 *   - ReferenceMap supports copying references, if a common struct applied
 *
 *   Version 5.5
 *   - IDE update (Eclipse Embedded C/C++ plugin)
 *   - $CCFLAGS and $LDFLAGS redesigned in configure.sh and build.sh
 *
 *   Version 5.45
 *   - Output format confs generalized: --outputConf (replaced --outputTiffConf)
 *   - Plain numeric EPSG codes recognized in --cProj <value> , some (4236, 3067) also predefined for portability
 *   - GeoFrame refactored (Metric => Native)
 *
 *   Version 5.44
 *   - GeoTIFF: Compression added, with new --outputTiffConf  <tilewidth>,<tileheight>,<compression>
 *   - Recognition of metric bbox in --cBBox
 *
 *   Version 5.43
 *   - Append sweeps: accept --inputPrefix
 *
 *   Version 5.42
 *   - ODIM Validator
 *
 *   Version 5.4
 *   - geometry.{width,height} replaces {nbins,nrays} in PolarODIM and {xsize,ysize} in CartesianODIM
 *   - EncodingODIM uses ValueScaling as base class
 *
 *   Version 5.31
 *   - Source NOD in tiled compositing
 *   - BBOX vector output (where::BBOX was through string conv)
 *   - GeoTIFF output: GDAL metadata
 *
 *   Version 5.29
 *   - Distance transforms using dynamic DistanceElement's
 *   - FillArray bug
 *
 *   Version 5.28
 *   - Sweep indexing also by timestamp
 *
 *   Version 5.08
 *   - Palette read (JSON and TXT)
 *
 *   Version 5.07
 *   - Log msg of NOTE and INFO simplified
 *   - Image selection fixed
 *
 *   Version 5.06
 *   - JSON palette support
 *   - bool Variable read and write for true/false as well (not only zero/nonzero)
 *
 *   Version 5.05
 *  - Data Selector path RegExp deprecation
 *  - JSON outputs
 *
 *
 *  Todo:
 *  - clear proj def in  accumula
 */

namespace rack {

/*!  Main program - renamed to facilitate calls from outside, esp. JNI calls.
 *   The standard main() will call this directly.
 */
int process(int argc, const char **argv);

} // rack::



#endif

// Rack
