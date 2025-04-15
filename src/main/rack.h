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
#define __RACK_VERSION__ "12.1"

/**
 *  Version 12.0
 *  - Revised QuantityMap, allowing retrieve("DBZXP")
 *  Release 12.1
 *  - StringTools: extractPrefix (common leading part)
 *  - Image memory area fix prefixed hex string
 *  - Fix: TreeSVG translate (extra parentheses)
 *  - cMethod: added MINMAX
 *
 *  Version 11.0
 *  - Help supports "see also", linking related commands.
 *  - Initial SLD palette
 *  Release 11.1
 *  - Recycling ctext as a multi-purpose string attribute, requires redefining isSingular()
 *  - SLD details (type="")
 *  Release 11.2
 *  - TreeUtilsHTML, isSingular(), isExplicit() adjusted
 *  Release 11.3
 *  - Fixed/revised support of transparency in palette values
 *  - GeoTiff: float/double type; explicit --encoding recommended with nodata= and undetect
 *  Release 11.4
 *  - EchoTop dataSelector: prf=ANY
 *  - Clear --select after --outputFile (?)
 *  Release 11.6
 *  - Fixed composite tile storage type bug (S = uint16)
 *  Release 11.7
 *  - Fixed accumulation encoding (automatic undetect/nodata)
 *  Release 11.8
 *  - Fixed accumulation empty selection skipping
 *
 *  Version 10.7
 *   - gPanel TECH
 *   - SVG transformations (translate, scale, skew, matrix)
 *  Release 10.7.1
 *   - cEchoTop ReferenceMap short key bug (type=C)
 *
 *  Version 10.6
 *   - gFontSize, gBoxSizes, gInclude,
 *  Release 10.6.1
 *   - Fixed projstr EPSG bug
 *
 *  Version 10.5
 *   - completed SVG titles
 *
 *  Version 10.4
 *   - SVG titles IMAGETITLE, GROUPTITLE, MAINTITLE
 *  Release 10.4.1
 *   - hasChild(const T & key) -> getKey(...) experiments without success
 *
 *  Version 10.3
 *   - XML refactoring, separated utilities
 *
 *  Version 10.2.2
 *   - Graphics/XML: XML de-templating

 *  Version 10.2.1
 *   - Graphics/XML-SVG: CSS style refactoring
 *
 *  Version 10.2
 *   - Graphics/SVG-alignment: FILL - stretching element.
 *
 *   Version 10.1.2
 *   - Graphics/SVG-alignment: FILL - stretching element.
 *
 *   Version 10.1.1
 *   - Graphics/SVG-alignment: const axis HorzAlign and VertAlign
 *
 *   Version 10.1
 *   - Graphics/SVG-alignment: streamlined fixed/flipping HORZ/VERT anchor
 *   - Graphics: --gLayout HORZ,INCR , --gPanelTest <TITLE> (hidden)
 *
 *   Version 10.0.1
 *   - Graphics: --gLinkImage, ( --gRadarSectors )
 *
 *   Version 10.0
 *   - what:prodpar pruned: only explicit argument(s), from cmd line
 *   - experimental graphics section: --gLinkImage, --gRadarSectors
 *
 *   Version 9.3.1
 *   - odim (version command): added "KILOMETERS"
 *   - added Proj4 version check with #if for proj_context_clone()
 *
 *   Version 9.3
 *   - pCappi: Pseudo CAPPI as default (weightMin==-1 < 0 )
 *
 *   Version 9.2.3
 *   - VariableFormatterODIM: bypassed formatPlace() to support numeric formatting like "%04.1f"
 *
 *   Version 9.2.2
 *   - TextStyle: VT100 separately
 *
 *   Version 9.2.1
 *   - CmdOutputFile: revised metadata statusMap: combines global and local
 *
 *   Version 9.2
 *   - Compositing: exact (native) input BBOX
 *
 *   Version 9.1
 *   - Compositing: revised count handling for QIND
 *   - RackContext::findImage(): keep ctx.selector intact
 *
 *   Version 9.0
 *   - DataSelector: quantity selection streamlined DBZH/QIND, /QIND
 *   - EchoTop: new version, renamed EchoTopOld -> EchoTopOld
 *   - Metadata: BBOX_data and SIZE_data changed to BBOX_input and SIZE_input
 *
 *   Version 8.9.6
 *   - EchoTop2 sliding window
 *   - Fixed bugs in --cExtract crop parameter, and changed DATA to INPUT (OVERLAP prevailing)
 *   - Revised --select quantity=... quality handling (QIND for dataset, and /QIND for quantity-specific)
 *
 *   Version 8.9.5
 *   - Added KDP, PHIDP and HCLASS to QuantityMap (--quantityConf)
 *   - Contained bugs in --cExtract crop parameter, appearing (only) with some --cMethod options.
 *
 *   Version 8.9.4
 *   - select: added (highPRF == 0) for prf==SINGLE testing
 *
 *   Version 8.9.3
 *   - pEchoTop2: zero weights switch interpolation to extrapolation
 *
 *   Version 8.9.2
 *   - Changed syntax: --store intermediate=...  to --store INTERMEDIATE etc.
 *
 *   Version 8.9.1
 *   - Fixed --cExtract crop=bbox parameter, also added cropping options DATA and OVERLAP
 *
 *   Version 8.9
 *   - Revised --cExtract syntax: 'dwcs' syntax completed with alternative 'DATA:WEIGHT:COUNT:DEVIATION' syntax, also sequential --encoding supported
 *
 *   Version 8.8
 *   - Revised, simplified Command class hierarchy.
 *
 *   Version 8.7.6
 *   - Fixed date & time formatting for GDAL variables, eg.  --format '{"nominal-time", "${what:date|%Y/%m/%d} ${what:time|%H:%M} UTC"}'
 *
 *   Version 8.7.5
 *   - Added support for GDAL variables, issued like --/what:GDAL:<key>=<value> or --format '{"key", 12.345,"key2", "value"}'
 *
 *   Version 8.7.4
 *   - Fix: Added support of (double) hyphen matching in scripts (--palette "" and --format "\"something\"")
 *
 *   Version 8.7.3
 *   - Fix: Composite extract (internal variables)
 *
 *   Version 8.7.2
 *   - Fix: SourceODIM
 *   - Fix: VariableT<>::operator==(const char *s)
 *
 *   Version 8.7.1
 *   - Variable comparisons
 *
 *   Version 8.7
 *   - Extensive documentation for drain::VariableT - drain::Variable, drain::FlexibleVariable and drain::Reference .
 *   - Fixed (Flexible)Variable assignment bug
 *   - DRAIN_TYPENAME(type) macro
 *
 *   Version 8.6.1
 *   - Added special arg \c 'empty' for \c --delete , as other \c --delete commands may leave empty data groups
 *   - Fixed basedir creation of html output
 *
 *   Version 8.6
 *   - pEchoTop2
 *
 *   Version 8.5
 *   - Redesigned (separated) VariableBase and VariableInitialiser
 *   - Logger ctr with variadic parameters
 *
 *   Version 8.4.1
 *   - Drain core classes directly at /drain
 *   - Refactored Variable, Reference and FlexibleVariable
 *   - #ifndef NDEBUG applied (from <cassert> )
 *
 *   Version 8.4
 *   - Revised Variable, Reference and FlexibleVariable
 *   - Tentative ODIM version switch support
 *   - EchoTop2
 *
 *   Version 8.3.1
 *   --outputConf tif:  <- default compliancy=EPSG:STRICT
 *
 *   Version 8.3.1
 *   Version 8.3
 *   -- SVG title formatting, like ${what:starttime|:2:4}
 *
 *   Version 8.2.2
 *   -- CodeCheck: (self-assignment) cert-oop54-cpp
 *
 *   Version 8.2.1
 *   -- SVG title formatting, like ${what:starttime|:2:4}
 *   -- pCappi back to PRF any
 *
 *   Version 8.2
 *   -- output variable formatting: 2.7182818... -> ${e|%4.2f} ${e|%06.2f} ${e|:6:2}
 *   -- HTML output
 *
 *   Version 8.17
 *   -- enhanced SVG output
 *
 *   Version 8.16.1
 *   -- Fixed /what:EPSG -> /where:EPSG
 *
 *   Version 8.16
 *   -- enhanced SVG output
 *   -- nonstandard /how:EPSG -> /where:EPSG
 *
 *   Version 8.15
 *   -- SVG output by extension (.svg).
 *
 *   Version 8.14
 *   -- QuantityMatcher: period '.' added to regExp-recognising-regExp:  "^.?*[]()$"
 *
 *   Version 8.13
 *   -- Streamlined XML/SVG/HTML classes
 *
 *   Version 8.13
 *   -- Fixed SegmentProber POLAR bug
 *
 *   Version 8.12
 *   -- SVG output panel: --outputConf svg[:<params>] --outputFile <basename>.svg
 *
 *   Version 8.11
 *   -- Fixed SVG GROUP bug
 *
 *   Version 8.1
 *   -- Program output tracking
 *   -- SVG, XML development
 *
 *   Version 8.0
 *   -- Revised quantity selection: list of strings/regExps.
 *
 *   Version 7.9
 *   -- Palette aliases, like DBZH->DBZ, DBHV->DBZ, TH->DBZ, TV-> ...
 *   -- Fixed PGM 16bit read/write bugs
 *   -- DataSelector supports list and regexp: --selectQuantity 'DBZH,DBZV,TH,^DBZH.*$'
 *
 *   Version 7.83
 *   -- Revised ODIMElement, ODIMElementMatcher, DataSelector, QuantitySelector
 *
 *   Version 7.81
 *   -- Further revised/simplified DataSelector
 *
 *   Version 7.8
 *   -- Revised DataSelector: #selectPaths()
 *   -- New functionality: selective read --inputSelect
 *   -- Log: adjustable sublevels: accept<LOG_DEBUG>(...) , fail<LOG_INFO>(...)
 *   -- Revised definitions: Variable, ReferenceVariable, FlexibleVariable
 *   -- Image processing: keys instead of numeric indices
 *   -- Check rscale, nbins, nrays in compositing
 *
 *   Version 7.79
 *   -- DistanceTransform: literal keywords: "4-CONNECTED", "8-CONNECTED", "16-CONNECTED",

 *   Version 7.78
 *   -- Logger: added op(), enabling log(level)(args...)
 *   -- Palette: fixed file search
 *   -- Input: adjustable error level
 *
 *   Version 7.77
 *   -- Logger arg order
 *   -- Fixed conversion bug
 *
 *   Version 7.76
 *   -- Bug fix: QualityCombiner
 *
 *   Version 7.75
 *   -- Revised AnDRe class code assignment
 *
 *   Version 7.74
 *   Version 7.73
 *   -- Palette tuning
 *
 *   Version 7.73
 *   -- Revised drain::Path design (variadic templates)
 *
 *   Version 7.71
 *   -- Revised DotGraph output
 *   -- Revised & fixed combining of input quality fields
 *
 *
 *   Version 7.7
 *   -- Revised & fixed combining of input quality fields
 *
 *   Version 7.62
 *   -- Default palettes for the most common quantities (DBZH, VRAD, ...) are included directly as C++ code, from ./palette
 *
 *   Version 7.61
 *   -- Bug fix: qualityCombiner
 *
 *   Version 7.6
 *   -- Palette supports initializer_list
 *
 *   Version 7.53
 *   -- cBoxTest uses AEQD as default projection
 *
 *   Version 7.52
 *   -- Added Cropping BBOX parameter to --cExtract
 *   -- Fixed cCreateTile (clear composite)
 *
 *   Version 7.51
 *   -- More support for EPSG in saving composites in GeoTIFF.
 *
 *   Version 7.5
 *   -- Revised build.sh and make.sh
 *
 *   Version 7.41
 *   -- Image processing result quantity: ${what:quantity}|${cmdKey}(${cmdArgs} (esp. for iResize & palette)
 *   -- Proj 7 compatibility
 *
 *   Version 7.4
 *   -- Proj 6.3.1
 *
 *   Version 7.32
 *   -- Extended EPSG handling for selected codes (now 3067)
 *
 *   Version 7.31
 *   -- Revised EPSG vs Proj.4 projstr handling
 *   -- Revised TIFFTAG_IMAGEDESCRIPTION, example: COMP:DBZH:PCAPPI(1500,true,1,0,WAVG:1:8:-40):elangles(0.3,0.7,1.5,3,5,9,2,7,11,15,25,45,0.4)
 *
 *   Version 7.3
 *   -- GeoTIFF EPSG support: plainEPSG output option
 *   -- GeoTIFF EPSG:3067 support
 *
 *   Version 7.22
 *   -- Changed --iResize adjusts ODIM (rscale, startaz, ...)
 *
 *   Version 7.21
 *   -- Changed --iResize encoding quantity syntax
 *   -- Fixed --iResize path selector
 *
 *   Version 7.20
 *   -- Fixed scripted volume append bug
 *
 *   Version 7.2
 *   -- Fixed --convert
 *
 *   Version 7.1
 *   -- Separate JSONtree class removed
 *   -- execFile JSON fix
 *
 *   Version 7.0
 *   -- Revised JSON classes
 *   -- execFile JSON support
 *
 *   Version 6.83
 *   -- Revised Flagger/Flags design with  enum support
 *   -- dualPRF selector redesigned
 *
 *
 *   Version 6.82
 *   -- Checks unknown single char instructions, ie. other than [, /, or ] .
 *
 *   Version 6.81
 *   Version 6.8
 *   -- DataSelector: added order: MIN|MAX:DATA|ELANGLE|TIME
 *
 *   Version 6.7
 *   -- Bug fix: cReset
 *
 *   Version 6.6
 *   -- Thread support for / small / commands /
 *
 *   Version 6.51
 *   -- Strip single and double quotes in scripting
 *   -- Added inputFileBasename to status
 *
 *   Version 6.5
 *   -- More GeoTiff resiliency in metadata errors
 *   -- Caster support for std::vector suppressed (resizing may change pointer address)
 *
 *   Version 6.49
 *   -- Bug fix: H5Aread / variable-length strings
 *
 *   Version 6.48
 *   -- aGapFill to use low-quality instead of 'nodata' (which required aRemover )
 *   -- aGapFillRec fix
 *   -- Bug fix: Frame2D template parameter
 *
 *   Version 6.47
 *   -- Bug fix: handling empty geoTiff compression parameter
 *
 *   Version 6.46
 *   -- Bug fix: recognition of Metric coords (ie. large values) based on any coord exceeding.
 *
 *   Version 6.45
 *   -- Added "pseudo" region to pCappi
 *   -- Bug fix: command section bug affecting --help
 *   -- Bug fix: Histogram resize/rescale
 *
 *   Version 6.44
 *   -- aCCor also writes TH-DBHZ as quantity "TH_DBZH"
 *
 *   Version 6.43
 *   -- Bug fix: PGM 16 bit write
 *   -- Developed further: file extension support
 *
 *   Version 6.42
 *   -- TIFF & GeoTIFF re-organized
 *
 *   Version 6.41
 *   -- Logger field width, precision
 *   -- StringBuilder, StreamBuilder
 *   -- Bug fix: PaletteOp precision
 *
 *   Version 6.4
 *   -- Data Selector: dualPRF
 *
 *   Version 6.35
 *   -- CAPPI: from polynomial to Gaussian beam modelling
 *
 *   Version 6.34
 *   -- Bug fix: Hi5 read string
 *
 *   Version 6.33
 *   -- Bug fix: ctx.statusFlags DATA_ERROR cleared upon new file read
 *
 *   Version 6.32
 *   -- select elangleMin,elangleMax removed
 *
 *
 *   Version 6.31
 *   -- Status formatting
 *
 *   Version 6.30
 *   -- Redesign: Logger calls with variadic templates
 *   -- Accumulator detached from AccumulationMethod
 *
 *
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
