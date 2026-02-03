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

#ifndef RACK_WIREFRAME_SVG
#define RACK_WIREFRAME_SVG

// #include <string>
#include <drain/prog/Command.h>

#include "hi5/Hi5.h"
#include "resources.h" // ctx

#include "graphics-radar.h" // ctx




namespace rack {

class CmdPolarBase {

protected:
	drain::SteppedRange<double> radiusMetres = {0.0, 0.0, 0.0};// (50000.0, 0.0, 250000.0);
	drain::SteppedRange<double> azimuthDegrees = {0.0, 0.0, 0.0}; // (30, 0, 360);
	bool MASK = false;

public:

	static
	const std::string DATA_ID; // "data-latest"

	/**
	 *
	 *  circle=max RADAR_CIRCLE
	 *  dot=0.0    RADAR_DOT
	 *  label=${NOD}  RADAR_LABEL
	 *
	 *
	 */

	enum GRAPHIC {
		DOT=8,
		CIRCLE=16,
	};

	/**
	 *  \param shared - if false, create private object ("layer") for each radar; else use common.
	 */
	static
	drain::image::TreeSVG & getOverlayGroup(RackContext & ctx, RadarSVG & radarSVG);

	static
	drain::image::TreeSVG & getOverlay(drain::image::TreeSVG & overlayGroup, const std::string & label="");

	static
	double ensureMetricRange(double maxRange, double range=1.0);

	// todo also min,max range check

	/// Given a distance or a distance range, ensure metric (if relative) and also validate generally.
	/**
	 *
	 */
	static
	void resolveDistance(const drain::SteppedRange<double> & ownDist, const drain::SteppedRange<double> & sharedDist, drain::SteppedRange<double> & dist, double maxDistance = 0.0);

	static
	void resolveAzimuthRange(const drain::SteppedRange<double> & ownAzm, const drain::SteppedRange<double> & sharedAzm, drain::SteppedRange<double> & azm);

};

/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */
class CmdRadarDot : public drain::BasicCommand, CmdPolarBase {

public:

	// Default: 10 kms.
	CmdRadarDot() : drain::BasicCommand(__FUNCTION__, "Draw circle describing the radar position") {
		getParameters().link("radius", radiusMetres.range.tuple(10000.0,10000.0), "radius of radar location disc [0.0..1.0|0..250..]").setFill(true);
		getParameters().link("MASK", MASK, "add mask");
	};

	// Copy constructor
	CmdRadarDot(const CmdRadarDot & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}


	virtual
	void exec() const override;

};
/**
 *
 *  See also: CmdGrid, CmdCartesianGrid
 */
class CmdRadarGrid : public drain::BasicCommand, CmdPolarBase {

	// future option
	bool AMZ_TEXT = true;
	bool DIST_TEXT = true;

public:

	inline
	CmdRadarGrid() : drain::BasicCommand(__FUNCTION__, "Draw polar sectors and rings. Styles: HIGHLIGHT") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
		getParameters().link("radius", radiusMetres.tuple(0.0, 0.0, 0.0), "step:start:end (metres)").fillArray = false;
		getParameters().link("azimuth", azimuthDegrees.tuple(30.0, 0.0, 360.0), "step:start:end (degrees)").fillArray = false;
		getParameters().link("MASK", MASK, "add a mask");
	};

	// Copy constructor
	inline
	CmdRadarGrid(const CmdRadarGrid & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	virtual
	void exec() const override;

};

class CmdRadarSector : public drain::BasicCommand, public CmdPolarBase {

public:

	CmdRadarSector() : drain::BasicCommand(__FUNCTION__, "Draw a sector, annulus or a disc. Styles: GRID,HIGHLIGHT,CmdPolarSector") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
		getParameters().link("radius", radiusMetres.range.tuple(0.0, 1.0), "start:end (metres)").fillArray = false;
		getParameters().link("azimuth", azimuthDegrees.range.tuple(0.0, 0.0), "start:end (degrees)").fillArray = false;
		getParameters().link("MASK", MASK, "add a mask");
	};

	// Copy constructor
	CmdRadarSector(const CmdRadarSector & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	virtual
	void exec() const override;

};


class CmdRadarRay : public drain::BasicCommand, public CmdPolarBase {

public:

	CmdRadarRay() : drain::BasicCommand(__FUNCTION__, "Draw a sector, annulus or a disc. Styles: GRID,HIGHLIGHT,CmdPolarSector") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
		getParameters().link("radius", radiusMetres.range.tuple(0.0, 1.0), "start:end (metres)").fillArray = true;
		getParameters().link("azimuth", azimuthDegrees.range.min, "(degrees)");
		getParameters().link("MASK", MASK, "add a mask");
	};

	// Copy constructor
	CmdRadarRay(const CmdRadarRay & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	virtual
	void exec() const override;

};


class CmdRadarLabel : public drain::BasicCommand, CmdPolarBase {

protected:

	// Mutable, to convert escaped chars. \see StringMapper::convertEscaped(
	// mutable
	std::string label = "${NOD}\n${PLC}";

public:

	CmdRadarLabel() : drain::BasicCommand(__FUNCTION__, "Draw circle describing the radar range. Styles: GRID,HIGHLIGHT,CmdPolarScope") {
		getParameters().link("label", label, "string, supporting variables like ${where:lon}, ${NOD}, ${PLC}");
		//getParameters().link("azimuth", azimuthDegrees.tuple(), "step:start:end (degrees)").fillArray = false;
	};

	// Copy constructor
	CmdRadarLabel(const CmdRadarLabel & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}


	virtual
	void exec() const override;

};

} // rack::


#endif

