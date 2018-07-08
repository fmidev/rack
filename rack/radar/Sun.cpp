/*

    Copyright 2011
    Author: Harri Hohti, C++ wrapper by Markus Peura (First.Last@fmi.fi)

    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <drain/util/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <limits.h>

#include "Geometry.h"
#include "Sun.h"


namespace rack {



/* 2001 (C) Harri.Hohti@fmi.fi */



void Sun::getSunPos(const std::string & datestr, double lon, double lat, double & azm, double & elev){


	const double JD = JDate(datestr.c_str());

	/* Tähtiaika radiaaneina auringon tuntikulman laskentaa varten */
	const double Sd0 = Sidereal_GW(JD) * 15.0 * drain::DEG2RAD;

	//std::cerr << "TIMESTAMP:" << datestr << std::endl;

	/* Lasketaan auringon ekvatoriaalikoordinaatit ajan funktiona */
	Equatorial equ;
	Solar_coords(JD, equ);
	//std::cerr << "Egu: RA" << Equ.RA << " Equ.Dec" << Equ.Dec << std::endl;

	/* Lasketaan auringon horisontaalikoordinaatit paikan ja ajan funktiona */
	Geographical geo;
	geo.lat = lat;
	geo.lon = lon;
	//std::cerr << "Geo: lat=" << Geo.lat << " Geo.lon=" << Geo.lon << std::endl;

	Horizontal hor;
	Solar_pos(Sd0, geo, equ, hor);

	azm = hor.azm;
	elev = hor.elev;

}


double Sun::Sidereal_GW(double JD)
{
	double JD0,d,T,SDR,SD_0GW,SD_GW;

	JD0=(double)((long int)(JD-0.5));
	d=(JD-0.5-JD0)*24.0;
	T=(JD0+0.5-2415020.0)/36525.0;

	/*   printf("%f %.16f\n",JD0+0.5,T); */

	/* Lasketaan t�htiaika 0-meridiaanilla 0 UTC */
	SDR=0.276919398+100.0021359*T+0.000001075*T*T;
	SD_0GW=(SDR-(double)((long int)SDR))*24.0;

	/* ja lis�t��n siihen todellinen aika t�htiajaksi muunnettuna,
      jotta saadaan t�htiaika halutulle ajanhetkelle */
	SD_GW=SD_0GW+d*1.002737908;

	return(SD_GW);
}


double Sun::JDate(const char *datestr)
{
	double JD=0.0;
	int YY,MM,DD,hh,mm,A,B,y,m;
	double d,fy,fm;

	sscanf(datestr,"%4d%2d%2d%2d%2d",&YY,&MM,&DD,&hh,&mm);

	y=YY;
	m=MM;
	if(MM<3){
		y = YY-1;
		m = MM+12;
	}

	A = y/100;

	B = 2-A+A/4;

	fy = (double)y;

	fm = (double)m;

	d=(double)DD+(double)hh/24.0+(double)mm/1440.0;
	JD=(double)((long int)(fy*365.25))+(double)((long int)(30.6001*(fm+1)))
    		  +d+1720994.5+(double)B;

	return(JD);
}


void Sun::Solar_coords(double JD, Equatorial & equ)
{

	double T,TT,L,M,OL,C,W,y,x,OA,ep;
	// double e;

	//Equatorial Equ;

	T = (JD-2415020.0)/36525.0;
	TT = T*T;

	/* Auringon keskilongitudi */
	L = 279.69668+36000.76892*T+0.0003025*TT;

	/* Auringon keskianomalia */
	M = 358.47583+35999.04975*T-0.00015*TT-0.0000033*T*TT;

	/* Maan radan eksentrisyys */
	// e = 0.01675104-0.0000418*T-0.000000126*TT;

	/* printf("M=%.12f\nL=%.12f\ne=%.12f\n",M,L,e); */

	M *= drain::DEG2RAD;

	/* Auringon "equation of the center" */
	C  = (1.919460-0.004789*T-0.000014*TT) * sin(M);
	C += (0.020094-0.0001*T) * sin(2*M);
	C +=  0.000293 * sin(3*M);

	/* Auringon longitudi */
	OL=L+C;
	/*   printf("OL=%.12f\nC=%.12f\n",OL,C); */

	/* Maan nousevan solmun longitudi */
	W = (259.18-1934.142*T) * drain::DEG2RAD;

	/* Auringon longitudi korjattuna todelliseen epookkiin */
	OA = OL-0.00569-0.00479*sin(W);
	/* printf("Oapp=%.12f\n",OA); */

	OA *= drain::DEG2RAD;

	/* ekliptikan kaltevuus */
	ep = 23.452294-0.0130125*T-0.00000164*TT+0.000000503*T*TT+0.00256*cos(W);
	ep *= drain::DEG2RAD;

	/* Muunnos auringon ekliptikaalisista koordinaateista
      (longitudi lambda, latitudi beta (auringolle < 1.2"))
      ekvatoriaalisiin, rektaskensioon ja deklinaatioon */

	y=cos(ep)*sin(OA);
	x=cos(OA);

	equ.dec=asin(sin(ep)*sin(OA));
	equ.RA=atan2(y,x);

	/* printf("RA=%.12f\nDec=%.12f\n",Equ.RA*RAD_TO_DEG/15.0,Equ.Dec*RAD_TO_DEG); */

	//return(Equ);
}


void Sun::Solar_pos(double Sd0, const Geographical & geo, const Equatorial & equ, Horizontal & hor)
{

	double H,x,y;
	//Horizontal Hor;

	/* Tuntikulma H on 0-meridiaanin t�htiaika + longitudi - rektaskensio */
	H = Sd0 + geo.lon - equ.RA;

	/* Muunnos ekvatoriaalikoordinaateista halutun latitudin ja
      longitudin horisontaalisiin koordinaatteihin */

	y = sin(H);
	x = cos(H)*sin(geo.lat) - tan(equ.dec)*cos(geo.lat);

	/* Lis�t��n 180, jotta atsimuutti alkaisi pohjoisesta */
	/*
	Hor.A = atan2(y,x) * RAD2DEG + 180.0;
	if (Hor.A < 0.0)
		Hor.A += 360.0;
		*/
	hor.azm = atan2(y,x) + M_PI;
	if (hor.azm < 0.0)
		hor.azm += 2.0 * M_PI;

	//Hor.elev = (asin(sin(Geo.lat)*sin(Equ.Dec) + cos(Geo.lat)*cos(Equ.Dec)*cos(H))) * RAD2DEG;
	hor.elev = (asin(sin(geo.lat)*sin(equ.dec) + cos(geo.lat)*cos(equ.dec)*cos(H)));

	// return (Hor);
}


}
