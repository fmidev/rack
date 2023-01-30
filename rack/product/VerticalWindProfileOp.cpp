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
 * VerticalProfileOp.cpp
 *
 *  Created on: Mar2, 2021
 *      Author: mpeura
 */

#include "data/QuantityMap.h"
#include "product/VerticalWindProfileOp.h"
#include "radar/Constants.h"
#include "radar/Geometry.h"
#include "drain/util/Log.h"
#include "drain/util/RegExp.h"
#include "drain/image/ImageT.h"
#include <map>
#include <set>
#include <utility>

namespace rack {

// TODO: implement azSlots

void VerticalWindProfileOp::computeSingleProduct(const DataSetMap<PolarSrc> & srcSweeps, DataSet<VprDst> & dstProduct) const {
	//void VerticalProfileOp::filterGroups(const Hi5Tree &src, const ODIMPathList & paths, Hi5Tree &dst) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	mout.debug() << *this << mout.endl;

	/// Copy top level metadata, part I: product settings  (including undetectValue timestamps - so this must be before user's attributes)

	if (srcSweeps.empty()){
		mout.warn() << "no input data, skipping" << mout.endl;
		return;
	}

	/// Trick. Mutable member referenced by odim.
	interval = (odim.altitudeRange.max - odim.altitudeRange.min) / static_cast<double>(odim.levels);
	//mout.warn() << "interval" << interval << mout.endl;
	//mout.warn() << "odim " << odim << mout.endl;


	// Create (as first data?)
	Data<VprDst> & dstHeight = dstProduct.getData("HGHT"); //.odim.quantity = "HGHT";
	getQuantityMap().setQuantityDefaults(dstHeight, "HGHT", "S");
	//dstHeight.setQuantityDefaults("HGHT","S");
	dstHeight.odim.scaling.offset = 0.0;
	dstHeight.odim.scaling.scale = 0.25;
	//dstHeight.data.setType<unsigned short>();
	dstHeight.data.setGeometry(1, odim.levels);
	for (int k = 0; k < odim.levels; ++k) // inverse vertical coordinate (image convention)
		dstHeight.data.put(k, dstHeight.odim.scaleInverse(odim.altitudeRange.min + static_cast<double>(odim.levels-1 - k) * interval));
	//@ dstHeight.updateTree();

	//setGeometry(dstHeight);

	// const VerticalProfileODIM & odimFinal = odim;
	/*
	VerticalProfileODIM odimFinal; //(odim);
	odimFinal.link("interval", interval);
	odimFinal.importMap(odim);
	*/
	//@? dstProduct.updateTree(odim);  // check

	/// Common geometry for all the quantities.
	const drain::image::Geometry geometry(odim.azSlots, odim.levels);

	mout .debug3() << geometry << mout.endl;

	const drain::RegExp decibels("(T|DBZ)(H|V)");


	mout.debug3() << "Step 1: initialize accumulation arrays." << mout.endl;

	// Temporary storage for eacg quantity
	std::map<std::string,drain::image::ImageT<double> > sumDataMap; //quantityGroup;
	std::map<std::string,drain::image::ImageT<double> > sumQualityMap; //quantityGroup;
	std::map<std::string,drain::image::ImageT<unsigned int> > countMap; //quantityGroup;

	/*  FUTURE EXTENSION
	RadarProj4 proj;
	bool COLLECT_LATLON = true;
	proj.setProjectionDst("+proj=longlat +datum=WGS84 +no_defs");
	 */

	mout.debug3() << "Step 2: main loop - collect profile data." << mout.endl;

	std::set<double> elangles;

	/// Traverse elevation angles
	for (DataSetMap<PolarSrc>::const_iterator it = srcSweeps.begin(); it != srcSweeps.end(); ++it) {

		mout.debug3() << "Sweep:" << it->first << mout.endl;

		const DataSet<PolarSrc> & sweep = it->second;

		/// Traverse quantities of this elevation angle
		for (DataSet<PolarSrc>::const_iterator dit = sweep.begin(); dit != sweep.end(); ++dit) {

			const std::string & quantity = dit->first;
			const Data<PolarSrc> & srcData = dit->second;

			mout.note() << "  quantity:" << quantity << mout.endl;

			if (srcData.data.isEmpty()){
				mout.warn() << " empty quantity:" << quantity << mout.endl;
				continue;  // TODO warn?
			}

			drain::image::ImageT<double> & sumData     =    sumDataMap[quantity];
			drain::image::ImageT<double> & sumQuality  = sumQualityMap[quantity];
			drain::image::ImageT<unsigned int> & count =      countMap[quantity];


			const bool DB_SCALE = decibels.test(quantity);
			mout .debug3() << "quantity: "  << quantity << " db scale: " << (int)DB_SCALE << mout.endl;

			Data<VprDst> & dstData = dstProduct.getData(quantity);
			if (dstData.data.isEmpty()) {
				mout.debug() << "init quantity: "  << quantity << " db scale: " << (int)DB_SCALE << mout.endl;
				dstData.odim.quantity = quantity; // causes copying metadata from src
				setEncoding(srcData.odim, dstData);
				mout.debug() << EncodingODIM(dstData.odim) << mout.endl;
				dstData.data.setType(dstData.odim.type.at(0));
				//dstData.initialize(dstData.odim.type, geometry.getWidth(), geometry.getHeight());
				//dstData.setEncoding(dstData.odim.type);
				dstData.setGeometry(geometry.getWidth(), geometry.getHeight());
				// mout.warn() << "init: "  << dstData << mout.endl;

				sumData.setGeometry(geometry);  // .Map[quantity]
				sumQuality.setGeometry(geometry);
				count.setGeometry(geometry);

				// odimFinal.prodpar = parameters.toStr();
				// if (COLLECT_LATLON)
				//	proj.setLocation(srcData.odim.lon, srcData.odim.lat);

				//const int azm = static_cast<int>(odim.stopaz+360.0 - odim.startaz) % 360;
				/*
				if ((static_cast<int>(odim.stopaz+360.0 - odim.startaz) % 360) < 180){
					RadarProj4 proj(srcData.odim.lon, srcData.odim.lat);
					proj.setProjectionDst("+proj=latlong +ellps=WGS84 +datum=WGS84");
					//proj.setLocation(srcData.odim.lon, srcData.odim.lat);
					//double lon, lat;
					//dstData.odim.lat =
					const double meanRange = 1000.0*(odim.firstRange + odim.range)/2.0;
					const double meanAzm   = static_cast<double>(static_cast<int>(odim.startaz + odim.stopaz) % 360);

					//proj.projectFwd(meanRange, meanAzm, odimFinal.lon, odimFinal.lat);
					//odimFinal.lon *= RAD2DEG;
					//odimFinal.lat *= RAD2DEG;
					//mout.warn() << dstData.odim.lon << ',' << dstData.odim.lat << '\t' << dstData.odim.prodpar << mout.endl;
					//mout.warn() << odimFinal.lon << ',' << odimFinal.lat << '\t' << odimFinal.prodpar << mout.endl;
					//mout.warn() << EncodingODIM(dstData.odim) << mout.endl;

				}
				*/
			}



			const PlainData<PolarSrc> & srcQuality = srcData.getQualityData();

			const bool USE_QUALITY = ! srcQuality.data.isEmpty();

			/// Temporary arrays.
			drain::image::ImageT<double> & profile            = sumDataMap[quantity]; // rename
			drain::image::ImageT<double> & profileQuality     = sumQualityMap[quantity];
			drain::image::ImageT<unsigned int> & profileCount = countMap[quantity];


			/// Elevation (in radians).
			const double eta = srcData.odim.getElangleR();
			if (elangles.find(srcData.odim.elangle) == elangles.end()){
				//dst["where"].data.attributes["elangles"] << srcData.odim.elangle;  // todo renew
				elangles.insert(srcData.odim.elangle);
			}

			/// Distance to a bin center (in metres).
			double beam;

			/// Distance (in metres) to the first measurement bin center.
			const double beamOffset = srcData.odim.rstart + srcData.odim.rscale/2.0;

			/// Distance (in metres) to the first measurement requested by the user.
			const double beamMin = Geometry::beamFromEtaGround(eta, odim.distanceRange.min * 1000.0);
			if (beamMin < beamOffset)
				mout.info() << "requested minimum distance " << beamMin << " smaller than measured distance " << beamOffset << mout.endl;

			/// Distance (in metres) to the last measurement requested by the user.
			const double beamMax = Geometry::beamFromEtaGround(eta, odim.distanceRange.max * 1000.0);
			const double beamMaxMeasured = srcData.odim.area.width*srcData.odim.rscale + beamOffset;
			if (beamMax > beamMaxMeasured)
				mout.info() << "requested maximum distance " << beamMax << " greater than measured distance " << beamMaxMeasured << mout.endl;


			const int startRay = srcData.odim.area.height * (odim.azmRange.min / 360.0);  //
			const int stopRay  = srcData.odim.area.height * (odim.azmRange.max / 360.0) + (odim.azmRange.max > odim.azmRange.min ? 0 : srcData.odim.area.height);  // Sector goes over 360 deg
			mout .debug3() << "rays: "  << startRay << "..." << stopRay << mout.endl;


			/// Bin altitude in metres.
			double altitude;

			unsigned int binStart = static_cast<int>( (beamMin-srcData.odim.rstart) / srcData.odim.rscale);
			if (binStart < 0)
				binStart = 0;

			unsigned int binEnd   = static_cast<unsigned int>( (beamMax-srcData.odim.rstart) / srcData.odim.rscale) ;
			if (binEnd > srcData.odim.area.width)
				binEnd = srcData.odim.area.width;

			// (Altitude check is within the loop)

			mout.debug() << "Bins:    "  << binStart << '-' << binEnd << mout.endl;
			mout.debug() << "Beam[m]: "  << beamMin << '-' << beamMax << mout.endl;
			mout.debug2() << srcData.odim << mout.endl;

			//if (beamStart )
			int k = 0; // level index (vertical coordinate)

			/// Raw (unscaled) input value
			double x;

			double q = 1.0;
			double qFinal;

			int j; // ray index (azimuthal coordinate of the ray)
			int j2; // profile column index (azimuthal coordinate of the profile bin)

			/// for location on ground
			/*  //COLLECT_LATLO
			double range;
			double azm;
			double lon, lat;
			*/

			//for (int i = 0; i<srcData.odim.geometry.width; i++){
			for (unsigned int i = binStart; i<binEnd; i++){
				beam     = beamOffset + static_cast<double>(i)*srcData.odim.rscale;
				altitude = Geometry::heightFromEtaBeam(eta, beam);
				// if (COLLECT_LATLON)
				//	range = Geometry::groundFromEtaBeam(eta, beam);

				k = odim.levels-1 - static_cast<int>((altitude - odim.altitudeRange.min) / interval); // inverse vertical coordinate (image convention)
				// Check altitude
				if ((k >= 0) && (k < odim.levels)){

					//for (size_t j = 0; j<data.getHeight(); j++){
					for (int j0 = startRay; j0<stopRay; j0++){
						j = ((j0+srcData.odim.area.height) % srcData.odim.area.height);
						x = srcData.data.get<double>(i,j);
						if (x != srcData.odim.nodata){
							if (x != srcData.odim.undetect){ // TODO: HANDLE?
								if (USE_QUALITY)
									q = srcQuality.odim.scaleForward( srcQuality.data.get<double>(i,j) );
								qFinal = (x != srcData.odim.undetect) ? q : q*0.5; // TODO: map undetect?
								if (DB_SCALE)
									x = dbzToZ(srcData.odim.scaleForward(x));
								else
									x = srcData.odim.scaleForward(x);
								j2 = (odim.azSlots * j)/srcData.odim.area.height;
								profile.at(j2, k)        += x * qFinal;
								profileQuality.at(j2, k) += qFinal;
								profileCount.at(j2, k)   += 1;
							}
							/*
							if (COLLECT_LATLON){
								azm = static_cast<double>(j)/static_cast<double>(srcData.odim.geometry.height)*360.0 * DEG2RAD;
								proj.projectFwd(range*sin(azm), range*cos(azm), lon, lat);
								lon *= RAD2DEG;
								lat *= RAD2DEG;
								std::cout << " pos=" << lon << ',' << lat << '\n'; // << range << '\n';
							}
							*/
						}
					}
				}  // end azimuthal iteration
				// COLLECT_LATLON = false; debug
			} // end range iteration

		} // end quantities

	} // end elevations


	mout.debug3() << "Step 3: copy profiles to structure" << mout.endl;


	/// Second loop: copy the profile(s), each quantity at a time.

	for (DataSet<VprDst>::iterator it = dstProduct.begin(); it != dstProduct.end(); ++it) {

		const std::string & quantity = it->first;

		Data<VprDst> & dstData = it->second;

		mout.debug3() << "quantity:" << quantity << mout.endl;

		//Hi5Tree & group = dstDataSet[groupName];
		//drain::VariableMap & what = group["what"].data.attributes;
		//what["quantity"] = quantity;

		if (quantity == "HGHT"){
			// SKIP
			//dstData.setQuantityDefaults("HGHT","S");
			//dstData.data.setGeometry(1, odim.levels);
			/*
			for (int k = 0; k < odim.levels; ++k) // inverse vertical coordinate (image convention)
				//dstData.data.put(k, odim.firstheight + static_cast<double>(odim.levels-1 - k) * interval);
				dstData.data.put(k, dstData.odim.scaleInverse(odim.firstheight + static_cast<double>(odim.levels-1 - k) * interval));

			dstData.updateTree();
			*/
		}
		else {

			const bool DB_SCALE = decibels.test(quantity);

			//mout.warn() << EncodingODIM(dstData.odim) << " DB_SCALE=" << (int)DB_SCALE << mout.endl;

			// TODO check cf namings above
			ImageT<double> & profile            = sumDataMap[quantity]; // rename
			ImageT<double> & profileQuality     = sumQualityMap[quantity];
			ImageT<unsigned int> & profileCount = countMap[quantity];

			const QuantityMap & qm = getQuantityMap();

			PlainData<VprDst> & dstQualityData = dstData.getQualityData("QIND");
			// dstQualityData.setQuantityDefaults("QIND");
			qm.setQuantityDefaults(dstQualityData, "QIND");
			dstQualityData.data.setGeometry(geometry); // why not geom?

			PlainData<VprDst> & dstCountData   = dstData.getQualityData("COUNT"); // could be anything (except QIND)
			qm.setQuantityDefaults(dstCountData, "COUNT","I");
			//dstCountData.odim.scaling.offset = 0.0;
			//dstCountData.odim.scaling.scale   = 1.0;
			//dstCountData.odim.quantity = quantity+"_COUNT";  // TODO OPERA
			dstCountData.data.setGeometry(geometry);  // why not geom?

			long int n = 0;
			long int c;// = 0;
			double w;
			for (int j = 0; j < odim.azSlots; ++j) {
				for (int k = 0; k < odim.levels; ++k) {

					if ((w = profileQuality.get<double>(j,k)) > 0.0){
						//if (k%15 == 0)	std::cerr << quantity << ' ' << profile.get<double>(j,k)/w << '\t' << odim.scaleInverse( zToDbz(profile.get<double>(j,k)/w) ) << '\n';
						if (DB_SCALE)
							dstData.data.put(j, k, dstData.odim.scaleInverse( zToDbz(profile.get<double>(j,k)/w) ) );
						else
							dstData.data.put(j, k, dstData.odim.scaleInverse(        profile.get<double>(j,k)/w)  );
						c = profileCount.get<long int>(j,k);
						dstQualityData.data.put(j, k, dstQualityData.odim.scaleInverse(w/static_cast<double>(c)) );
						dstCountData.data.put(j, k, c);
						n += c;
					}
				}
			}

			//@ dstData.updateTree();
			//@ dstQualityData.updateTree();
			//@ dstCountData.updateTree();
			// group.data.attributes["n"] = n;  // Number of samples

		}

		//dstProduct.updateTree(odimFinal);

	}

	// @? dstProduct.updateTree(odimFinal);
	//mout.warn() << odimFinal << mout.endl;


}

} // rack::

// Rack
 // REP
