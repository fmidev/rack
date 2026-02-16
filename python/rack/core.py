import rack.prog

class Rack(rack.prog.Register):
    """ Rack - a radar data processing program

    Automatic command set export
    """

    # defaultCmdKey=inputFile
    # execFileCmd=CmdExecFile
    # commandBank.scriptCmd=CmdScript

    def aAltitude(self,
        functor:str='',
        aboveSeaLevel:bool=True,
        bias:float=0):
        """ Weights altitude

        Parameters
        ----------
        functor:str
           Functor
        aboveSeaLevel:bool
          Reference is sea level, not radar site
        bias:float
          offset added to altitude

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aAttenuation(self,
        reflHalfWidth:float=3,
        c:float=1.12e-07,
        p:float=0.62,
        c2:float=0,
        p2:float=0):
        """ Computes attenuation and converts it to probability

        Parameters
        ----------
        reflHalfWidth:float
          dBZ limit of 50% quality
        c:float
          coeff
        p:float
          coeff
        c2:float
          coeff
        p2:float
          coeff

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aBiomet(self,
        reflMax:float=-10,
        maxAltitude:int=500,
        reflDev:float=5,
        devAltitude:int=1000):
        """ Detects birds and insects.

        Parameters
        ----------
        reflMax:float
          dBZ
        maxAltitude:int
          m
        reflDev:float
          dBZ
        devAltitude:int
          m

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aBird(self,
        dbzPeak:list=[-20,0],
        vradDevMin:float=1,
        rhoHVmax:list=[0.99,0.891],
        zdrAbsMin:float=1,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

        Parameters
        ----------
        dbzPeak:list
          Typical reflectivity (dBZ)
        vradDevMin:float
          Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)
        rhoHVmax:list
          Fuzzy threshold of maximum rhoHV value
        zdrAbsMin:float
          Fuzzy threshold of absolute ZDR
        window:list
          beam-directional(m), azimuthal(deg)
        gamma:float
          Contrast adjustment, dark=0.0 < 1.0 < brighter 

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aCCor(self,
        threshold:list=[10,30],
        mask:bool=False):
        """ Detects clutter. Based on difference of DBZH and TH. At halfwidth PROB=0.5. Universal.

        Parameters
        ----------
        threshold:list
          dBZ range
        mask:bool
          Doppler-cleared only

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aChaff(self,
        dbzPeak:list=[5,15],
        vradDevMax:float=5,
        rhoHVmax:list=[0.5,0.4],
        zdrAbsMin:float=0.5,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates chaff probability from DBZH, VRAD, RhoHV and ZDR.

        Parameters
        ----------
        dbzPeak:list
          Typical reflectivity (DBZH)
        vradDevMax:float
          Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)
        rhoHVmax:list
          Fuzzy threshold of maximum rhoHV value
        zdrAbsMin:float
          Fuzzy threshold of absolute ZDR
        window:list
          beam-directional(m), azimuthal(deg)
        gamma:float
          Contrast adjustment, dark=0.0 < 1.0 < brighter 

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aClutter(self,
        decay:float=0.5,
        gamma:float=1,
        quantity:str='CLUTTER',
        file:str='cluttermaps/cluttermap-${NOD}-${quantity}.h5'):
        """ Reads a ground clutter map and scales it to sweeps.

        Parameters
        ----------
        decay:float
          per 1000m
        gamma:float
          brightness
        quantity:str
          CLUTTER|OBSTACLE|...
        file:str
          path syntax

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aDamper(self,
        threshold:float=0.5,
        undetectThreshold:float=0.1,
        minValue:float=None):
        """ Removes anomalies with fuzzy deletion operator.

        Parameters
        ----------
        threshold:float
          
        undetectThreshold:float
          
        minValue:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aDefault(self,
        probability:float=0.8):
        """ Marks the data values as unclassified, with high probability

        Parameters
        ----------
        probability:float
          'resulting' probability

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aDefaultQuality(self,
        threshold:float=0.8):
        """ Quality index value below which also CLASS information will be updated.

        Parameters
        ----------
        threshold:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aDopplerNoise(self,
        speedDevThreshold:float=3,
        windowWidth:float=1500,
        windowHeight:float=3):
        """ Detects suspicious variance in Doppler speed (VRAD).

        Parameters
        ----------
        speedDevThreshold:float
          Minimum of bin-to-bin Doppler speed (VRAD) deviation (m/s)
        windowWidth:float
          window width, beam-directional (m)
        windowHeight:float
          window height, azimuthal (deg)

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aEmitter(self,
        lengthMin:float=5,
        thicknessMax:float=5,
        sensitivity:float=0.5):
        """ Detects electromagnetic interference segments by means of window medians.

        Parameters
        ----------
        lengthMin:float
          km
        thicknessMax:float
          deg
        sensitivity:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aGapFill(self,
        width:int=1500,
        height:float=5,
        qualityThreshold:float=0.1):
        """ Removes low-quality data with gap fill based on distance transformation.

        Parameters
        ----------
        width:int
          meters
        height:float
          degrees
        qualityThreshold:float
          0.0...1.0

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aGapFillRec(self,
        width:int=1500,
        height:float=5,
        loops:int=3):
        """ Recursive, 'splinic' gap filler.

        Parameters
        ----------
        width:int
          meters
        height:float
          degrees
        loops:int
          N

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aHydroClassBased(self,
        nonMet:float=0.5,
        NOMET:int=1):
        """ Sets probabilities of non-meteorological echoes based on precomputed hydrometeor CLASS.

        Parameters
        ----------
        nonMet:float
          
        NOMET:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aInsect(self,
        dbzMax:list=[-5,5],
        vradDevMax:float=5,
        rhoHVmax:list=[0.99,0.792],
        zdrAbsMin:float=3,
        window:list=[2500,5],
        gamma:float=1):
        """ Probability of insects, based on DBZH, VRAD, RhoHV and ZDR.

        Parameters
        ----------
        dbzMax:list
          Max reflectivity
        vradDevMax:float
          Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)
        rhoHVmax:list
          Fuzzy threshold of maximum rhoHV value
        zdrAbsMin:float
          Fuzzy threshold of absolute ZDR
        window:list
          beam-directional(m), azimuthal(deg)
        gamma:float
          Contrast adjustment, dark=0.0 < 1.0 < brighter 

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aJamming(self,
        smoothnessThreshold:float=5,
        distanceMin:float=80,
        refit:bool=True):
        """ Detects broad lines caused by electromagnetic interference. Intensities should be smooth, increasing by distance.

        Parameters
        ----------
        smoothnessThreshold:float
          dBZ
        distanceMin:float
          km
        refit:bool
          true|false

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aNonMet(self,
        threshold:list=[0.7,0.8],
        medianWindow:list=[0,0],
        medianPos:float=0):
        """ Detects clutter. Based on dual-pol parameter RhoHV . Optional post processing: morphological closing. Universal.

        Parameters
        ----------
        threshold:list
          0...1[:0...1]
        medianWindow:list
          metres,degrees
        medianPos:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aNonMet2(self,
        threshold:list=[0.75,0.95],
        medianWindow:list=[0,0],
        medianPos:float=0):
        """ Estimates probability from DBZH, VRAD, RhoHV and ZDR.

        Parameters
        ----------
        threshold:list
          0...1[:0...1]
        medianWindow:list
          metres,degrees
        medianPos:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aPrecip(self,
        probMax:float=0.5,
        dbz:float=20,
        dbzSpan:float=10):
        """ Detects precipitation...

        Parameters
        ----------
        probMax:float
          probability
        dbz:float
          dBZ
        dbzSpan:float
          dBZ

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aQualityCombiner(self):
        """ Combines detection probabilities to overall quality field QIND (and CLASS).


        """

        cmd = self.make_cmd(locals())
        return cmd


    def aRemover(self,
        threshold:float=0.5,
        replace:str='nodata',
        clearQuality:bool=True):
        """ Simple anomaly removal operator.

        Parameters
        ----------
        threshold:float
          probability
        replace:str
          nodata|undetect|<physical_value>
        clearQuality:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aShip(self,
        reflMin:float=25,
        reflDev:float=15,
        windowWidth:int=1500,
        windowHeight:float=3):
        """ Detects ships based on their high absolute reflectivity and local reflectivity difference.

        Parameters
        ----------
        reflMin:float
          dBZ
        reflDev:float
          dBZ
        windowWidth:int
          m
        windowHeight:float
          deg

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aSpeckle(self,
        threshold:float=-20,
        area:int=16,
        invertPolar:bool=False):
        """ Detects speckle noise. Universal: uses DBZ data as input, applies to all data in a sweep group.

        Parameters
        ----------
        threshold:float
          dBZ
        area:int
          bins
        invertPolar:bool
          bins

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aSun(self,
        beamWidth:float=1,
        sensitivity:float=0.95):
        """ Draw the sun beam

        Parameters
        ----------
        beamWidth:float
          deg
        sensitivity:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aTest(self,
        value:float=0,
        prob:float=1,
        i:list=[200,230],
        j:list=[240,300]):
        """ Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob).

        Parameters
        ----------
        value:float
          value
        prob:float
          prob
        i:list
          coord range
        j:list
          coord range

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aTime(self,
        time:str='NOMINAL',
        decayPast:float=1,
        decayFuture:float=-1):
        """ Created quality field based on measurement time for each beam.

        Parameters
        ----------
        time:str
          NOMINAL,NOW,<YYYYmmddMMHH>
        decayPast:float
          mins
        decayFuture:float
          mins

        """

        cmd = self.make_cmd(locals())
        return cmd


    def aUniversal(self,
        unversal:bool=True):
        """ Toggle the support for universal ie. Dataset-wide quality indices.

        Parameters
        ----------
        unversal:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def append(self,
        path:str=''):
        """ Append inputs/products (empty=overwrite).

        Parameters
        ----------
        path:str
          <empty>|dataset[n]|data[n]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cAdd(self):
        """ Adds the current product to the composite.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cAddWeighted(self,
        weight:float=1):
        """ Adds the current product to the composite applying weight.

        Parameters
        ----------
        weight:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cBBox(self,
        llLon:float=0,
        llLat:float=0,
        urLon:float=0,
        urLat:float=0):
        """ Bounding box of the Cartesian product.

        Parameters
        ----------
        llLon:float
          deg|m
        llLat:float
          deg|m
        urLon:float
          deg|m
        urLat:float
          deg|m

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cBBoxReset(self):
        """ Resets the bounding box (to be set again according to the next radar data).


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cBBoxTest(self,
        mode:int=0):
        """ Tests whether the radar range is inside the composite.

        Parameters
        ----------
        mode:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cBBoxTile(self,
        llLon:float=0,
        llLat:float=0,
        urLon:float=0,
        urLat:float=0):
        """ Redefines bbox and compositing array size for several radars, applying original projection and resolution. See --cSize, --cBBox, --cProj.

        Parameters
        ----------
        llLon:float
          deg
        llLat:float
          deg
        urLon:float
          deg
        urLat:float
          deg

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cCreate(self):
        """ Maps the current polar product to a Cartesian product.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cCreateLookup(self):
        """ Creates lookup objects


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cCreateSun(self,
        timestamp:str='200507271845'):
        """ Cartesian sun shine field.

        Parameters
        ----------
        timestamp:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cCreateTile(self):
        """ Maps the current polar product to a tile to be used in compositing.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cDecayTime(self,
        time:int=0):
        """ Delay half-time in minutes. 0=no decay

        Parameters
        ----------
        time:int
          minutes

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cDefaultQuality(self,
        weight:float=0.75):
        """ Set default quality (for data without quality field)

        Parameters
        ----------
        weight:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cExtract(self,
        channels:str='DATA,WEIGHT',
        bbox:str=''):
        """ Extract (export) data from the internal accumulation array

        Parameters
        ----------
        channels:str
          DATA,WEIGHT,COUNT,DEVIATION
        bbox:str
          Optional cropping: ...:<LLx>,<LLy>,<URx>,<URy> or INPUT or OVERLAP

        """

        cmd = self.make_cmd(locals())
        # note: separator ':'
        cmd.set_separators(':', ',')
        return cmd


    def cGrid(self,
        lonSpacing:int=1,
        latSpacing:int=1,
        width:float=1,
        intensity:float=0.5):
        """ Draw lat-lon grid onto a Cartesian product.

        Parameters
        ----------
        lonSpacing:int
          deg
        latSpacing:int
          deg
        width:float
          pix
        intensity:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cInit(self):
        """ Allocate memory to --cSize, applying --target and --select, if set.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cMethod(self,
        method:str='MAXIMUM'):
        """ Method in accumulating values on a composite.

        Parameters
        ----------
        method:str
          LATEST|MAXIMUM|MAXW|AVERAGE|WAVG,p,r,bias

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cOpticalFlow(self,
        width:list=[5,5],
        resize:float=0,
        threshold:float=None,
        spread:bool=False,
        smooth:bool=False):
        """ Optical flow computed based on differential accumulation layers.

        Parameters
        ----------
        width:list
          
        resize:float
          0.0..1.0|pix
        threshold:float
          value
        spread:bool
          0|1
        smooth:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cPlot(self,
        lon:float=0,
        lat:float=0,
        x:float=0,
        w:float=1):
        """ Add a single data point.

        Parameters
        ----------
        lon:float
          longitude
        lat:float
          latitude
        x:float
          value
        w:float
          weight

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cPlotFile(self,
        file:str=''):
        """ Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.

        Parameters
        ----------
        file:str
          filename

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cProj(self,
        projection:str=''):
        """ Set projection

        Parameters
        ----------
        projection:str
          <EPSG-code>|<projstr> Proj.4 syntax

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cRange(self,
        range:int=250000):
        """ Force a range for single-radar cartesian products (0=use-metadata).

        Parameters
        ----------
        range:int
          km

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cReset(self):
        """ Clears the current Cartesian product.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def cSize(self,
        width:int=400,
        height:int=0):
        """ Set size of the compositing array. Does not allocate memory.

        Parameters
        ----------
        width:int
          pixels
        height:int
          pixels

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cSpread(self,
        horz:float=10,
        vert:float=0,
        loops:int=0):
        """ Set Spread of the compositing array. OBSOLETE. Use --iDistanceTransformFill(Exp) instead

        Parameters
        ----------
        horz:float
          pixels
        vert:float
          pixels
        loops:int
          N

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cTime(self,
        time:str='201412091845'):
        """ Modify the time of the current composite. See --cTimeDecay 

        Parameters
        ----------
        time:str
          YYYYmmddHHMMSS

        """

        cmd = self.make_cmd(locals())
        return cmd


    def cTimeDecay(self,
        weight:float=1):
        """ Weight (0.9...1.0) of delay, per minute. 1.0=no decay. See --cTime and --cDecayTime

        Parameters
        ----------
        weight:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def checkType(self):
        """ Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def completeODIM(self):
        """ Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def convert(self):
        """ Convert --select:ed data to scaling and markers set by --encoding


        """

        cmd = self.make_cmd(locals())
        return cmd


    def createDefaultQuality(self,
        data:float=None,
        undetect:float=None,
        nodata:float=None):
        """ Creates default quality field. See --undetectWeight and --aDefault

        Parameters
        ----------
        data:float
          0..1
        undetect:float
          0..1
        nodata:float
          0..1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def debug(self):
        """ Set verbosity to LOG_DEBUG


        """

        cmd = self.make_cmd(locals())
        return cmd


    def delete(self,
        path:str='',
        quantity:str='',
        elangle:list=[-90,90],
        count:int=4095,
        order:str='DATA:MIN',
        prf:str='ANY',
        timespan:list=[0,0]):
        """ Deletes selected parts of h5 structure.

        Parameters
        ----------
        path:str
          [/]dataset<i>[/data<j>|/quality<j>]
        quantity:str
          DBZH|VRAD|RHOHV|...
        elangle:list
          min[:max]
        count:int
          
        order:str
          ["DATA","ELANGLE","TIME"]:["MIN","MAX"]
        prf:str
          ["ANY","SINGLE","DOUBLE"]
        timespan:list
          range from nominal time [seconds]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def dumpMap(self,
        filter:str='',
        filename:str=''):
        """ Dump variable map, filtered by keys, to std or file.

        Parameters
        ----------
        filter:str
          regexp
        filename:str
          std::string

        """

        cmd = self.make_cmd(locals())
        # note: separator ':'
        cmd.set_separators(':', ',')
        return cmd


    def echo(self,
        format:str=''):
        """ For testing. Dumps a formatted string to stdout.

        Parameters
        ----------
        format:str
          std::string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def encoding(self,
        type:str='C',
        gain:float=0,
        offset:float=0,
        undetect:float=0,
        nodata:float=0,
        rscale:float=0,
        nrays:int=0,
        nbins:int=0,
        quantity:str=''):
        """ Sets encodings parameters for polar and Cartesian products, including composites.

        Parameters
        ----------
        type:str
          storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)
        gain:float
          scaling coefficient
        offset:float
          bias
        undetect:float
          marker
        nodata:float
          marker
        rscale:float
          metres
        nrays:int
          count
        nbins:int
          count
        quantity:str
          string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def errorFlags(self,
        flags:str=''):
        """ Status of last select.

        Parameters
        ----------
        flags:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    # NOTE: key == commandBank.execFileCmd TODO...
    def execFile(self,
        filename:str=''):
        """ Execute commands from a file.

        Parameters
        ----------
        filename:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def execScript(self):
        """ Execute script.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def expandVariables(self):
        """ Toggle variable expansions on/off


        """

        cmd = self.make_cmd(locals())
        return cmd


    def format(self,
        syntax:str=''):
        """ Set format for data dumps (see --sample or --outputFile)

        Parameters
        ----------
        syntax:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def formatFile(self,
        filename:str=''):
        """ Read format for metadata dump from a file

        Parameters
        ----------
        filename:str
          std::string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def formatOut(self,
        filename:str=''):
        """ Dumps the formatted std::string to a file or stdout.

        Parameters
        ----------
        filename:str
          std::string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def freezingLevel(self,
        height:float=None,
        thickness:float=0.1):
        """ Freezing level modelled simply as its height and thickness.

        Parameters
        ----------
        height:float
          km
        thickness:float
          km

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gAlign(self,
        position:str='',
        anchor:str='',
        anchorHorz:str='',
        anchorVert:str=''):
        """ Alignment of the next element

        Parameters
        ----------
        position:str
          INSIDE|OUTSIDE:LEFT|CENTER|RIGHT|HORZ_FILL|UNDEFINED_HORZ,INSIDE|OUTSIDE:TOP|MIDDLE|BOTTOM|VERT_FILL|UNDEFINED_VERT
        anchor:str
          <name>|<empty>|@NONE|@PREVIOUS|@NEXT|@COLLECTIVE_CURRENT|@COLLECTIVE_FINAL
        anchorHorz:str
          ...
        anchorVert:str
          ...

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gAlignTest(self,
        name:str='',
        panel:str='playGround1'):
        """ SVG test product

        Parameters
        ----------
        name:str
          label
        panel:str
          label

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gCoords(self):
        """ SVG test product. CSS classes: GRID,GRID


        """

        cmd = self.make_cmd(locals())
        return cmd


    def gDebug(self,
        name:str='',
        panel:str='playGround1'):
        """ SVG test product

        Parameters
        ----------
        name:str
          label
        panel:str
          label

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gDot(self,
        lonlat:list=[25,60],
        radius:list=[0,25000],
        id:str='',
        style:str=''):
        """ Draw a marker circle. CSS classes: GRID,LABEL

        Parameters
        ----------
        lonlat:list
          Coordinate (lon,lat) in degrees(decimal) or metres.
        radius:list
          metres or relative
        id:str
          XML element id
        style:str
          XML element CSS style

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gFontSizes(self,
        value:str=''):
        """ Adjust font sizes in CSS style section.

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gGroupId(self,
        syntax:str=''):
        """ Set grouping criterion based on variables

        Parameters
        ----------
        syntax:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gGroupTitle(self,
        syntax:str=''):
        """ Set titles, supporting variables

        Parameters
        ----------
        syntax:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gInclude(self,
        include:str='["NONE"|"PNG"|"SVG"|"TXT"|"ALL"|"NEXT"|"SKIP"|"ON"|"OFF"|"UNKNOWN"]'):
        """ Select images to include in SVG panel

        Parameters
        ----------
        include:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gLayout(self,
        orientation:str='HORZ',
        direction:str='INCR'):
        """ Set main panel alignment

        Parameters
        ----------
        orientation:str
          HORZ|VERT
        direction:str
          INCR|DECR

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gLinkImage(self,
        value:str=''):
        """ Link arbitrary external image (PNG).

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gPanel(self,
        layout:str='TECH'):
        """ SVG test product

        Parameters
        ----------
        layout:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gPanelFoo(self,
        layout:str=''):
        """ SVG test product

        Parameters
        ----------
        layout:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gPanelTest(self,
        layout:str=''):
        """ SVG test product

        Parameters
        ----------
        layout:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gRadarDot(self,
        radius:list=[10000,10000],
        MASK:bool=False):
        """ Mark the radar position with a circle. CSS classes: GRID,DOT

        Parameters
        ----------
        radius:list
          metres or relative
        MASK:bool
          add mask

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gRadarGrid(self,
        radius:list=[0,0,0],
        azimuth:list=[30,0,360],
        MASK:bool=False):
        """ Draw polar sectors and rings. CSS classes: GRID,GRID

        Parameters
        ----------
        radius:list
          step:start:end (metres or relative)
        azimuth:list
          step:start:end (degrees)
        MASK:bool
          add a mask

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gRadarLabel(self,
        label:str='${NOD}\n${PLC}'):
        """ Draw circle describing the radar range.. CSS classes: GRID,LABEL

        Parameters
        ----------
        label:str
          string, supporting variables like ${where:lon}, ${NOD}, ${PLC}

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gRadarRay(self,
        radius:list=[0,1],
        azimuth:float=0,
        MASK:bool=False):
        """ Draw a sector, annulus or a disc. Styles: GRID,HIGHLIGHT,CmdPolarSector. CSS classes: GRID,RAY

        Parameters
        ----------
        radius:list
          start:end (metres)
        azimuth:float
          (degrees)
        MASK:bool
          add a mask

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gRadarSector(self,
        radius:list=[0,1],
        azimuth:list=[0,0],
        MASK:bool=False):
        """ Draw a sector, annulus or a disc.. CSS classes: GRID,SECTOR

        Parameters
        ----------
        radius:list
          start:end (metres or relative)
        azimuth:list
          start:end (degrees)
        MASK:bool
          add a mask

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gStyle(self,
        value:str=''):
        """ Add or modify CSS entry

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gTitle(self,
        title:str='AUTO'):
        """ Set main title

        Parameters
        ----------
        title:str
          <empty>|<string>|'auto'

        """

        cmd = self.make_cmd(locals())
        return cmd


    def gTitleHeights(self,
        value:str=''):
        """ Set title box heights and adjust font sizes. See --gFontSizes

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def geoTiff(self,
        tile:list=[256,256],
        compression:str=''):
        """ GeoTIFF tile size. Deprecating?, use --outputConf tif:tile=<width>:<height>,compression=LZW

        Parameters
        ----------
        tile:list
          
        compression:str
          NONE=1,LZW=5,DEFLATE=32946,PACKBITS=32773

        """

        cmd = self.make_cmd(locals())
        return cmd


    def getMyH5(self,
        value:str=''):
        """ Dump H5 sources

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def help(self,
        key:str=''):
        """ Display help.

        Parameters
        ----------
        key:str
          command|sections

        """

        cmd = self.make_cmd(locals())
        return cmd


    def helpExample(self,
        keyword:str=''):
        """ Dump example of use and exit.

        Parameters
        ----------
        keyword:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def histogram(self,
        count:int=0,
        range:list=[0,0],
        filename:str='',
        attribute:str='histogram',
        commentChar:str='#'):
        """ Histogram. Optionally --format using keys index,range,range.min,range.max,count,label

        Parameters
        ----------
        count:int
          
        range:list
          
        filename:str
          <filename>.txt|-
        attribute:str
          <attribute_key>
        commentChar:str
          Prefix for header and postfix for labels

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iAdd(self,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Adds values

        Parameters
        ----------
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iAverage(self,
        width:list=[1,0]):
        """ Window averaging with optional weighting support.

        Parameters
        ----------
        width:list
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iBlender(self,
        width:list=[5,0],
        spreader:str='avg',
        mix:str='max',
        loops:int=1,
        expansionCoeff:float=1):
        """ Smoothes image repeatedly, mixing original image with the result at each round.

        Parameters
        ----------
        width:list
          
        spreader:str
          "avg"="average""avgFlow"="flowAverage""avgGauss"="gaussianAverage""dist"="distanceTransform""distExp"="distanceTransformExp"
        mix:str
          max|<coeff>: (quality) max, (quality) blend
        loops:int
          number of repetitions
        expansionCoeff:float
          window enlargement

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iCatenate(self,
        scale:float=1,
        offset:float=0):
        """ Catenates images

        Parameters
        ----------
        scale:float
          
        offset:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iCopy(self,
        dstView:str='',
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...

        Parameters
        ----------
        dstView:str
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iCrop(self,
        width:int=0,
        height:int=0,
        i:int=0,
        j:int=0):
        """ Crop image.

        Parameters
        ----------
        width:int
          
        height:int
          
        i:int
          
        j:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDistance(self,
        functor:str=''):
        """ Computes the distance of pixel vectors. Post-scaling with desired functor.

        Parameters
        ----------
        functor:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDistanceTransform(self,
        width:list=[10,10],
        height:list=[None,None],
        topology:str='16-CONNECTED'):
        """ Linearly decreasing intensities - applies decrements.

        Parameters
        ----------
        width:list
          pix
        height:list
          pix
        topology:str
          ["CONN_UNSET","4-CONNECTED","8-CONNECTED","16-CONNECTED"]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDistanceTransformExp(self,
        width:list=[10,10],
        height:list=[None,None],
        topology:str='16-CONNECTED'):
        """ Exponentially decreasing intensities. Set half-decay radii.

        Parameters
        ----------
        width:list
          pix
        height:list
          pix
        topology:str
          ["CONN_UNSET","4-CONNECTED","8-CONNECTED","16-CONNECTED"]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDistanceTransformFill(self,
        width:list=[12.3,12.3],
        height:list=[None,None],
        topology:str='16-CONNECTED',
        alphaThreshold:list=[0,0]):
        """ Spreads intensities linearly up to distance defined by alpha channel.

        Parameters
        ----------
        width:list
          pix
        height:list
          pix
        topology:str
          ["CONN_UNSET","4-CONNECTED","8-CONNECTED","16-CONNECTED"]
        alphaThreshold:list
          0..1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDistanceTransformFillExp(self,
        width:list=[12,12],
        height:list=[None,None],
        topology:str='16-CONNECTED',
        alphaThreshold:list=[0,0]):
        """ Spreads intensities exponentially up to distance defined by alpha intensities.

        Parameters
        ----------
        width:list
          pix
        height:list
          pix
        topology:str
          ["CONN_UNSET","4-CONNECTED","8-CONNECTED","16-CONNECTED"]
        alphaThreshold:list
          0..1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iDiv(self,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        Parameters
        ----------
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFloodFill(self,
        location:list=[0,0],
        intensity:list=[1,65536],
        value:float=1):
        """ Fills an area starting at (i,j) having intensity in [min,max], with a value.

        Parameters
        ----------
        location:list
          i:j
        intensity:list
          min:max
        value:float
          intensity

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFlowAverage(self,
        width:list=[1,1]):
        """ Window average that preserves the magnitude

        Parameters
        ----------
        width:list
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyBell(self,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy bell function.

        Parameters
        ----------
        location:float
          
        width:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyBell2(self,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy Gaussian-like bell function.

        Parameters
        ----------
        location:float
          
        width:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyStep(self,
        position:list=[-1,1],
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy step function.

        Parameters
        ----------
        position:list
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyStepsoid(self,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy step function

        Parameters
        ----------
        location:float
          
        width:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyTriangle(self,
        position:list=[-1,1],
        peakPos:float=0,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy triangle function.

        Parameters
        ----------
        position:list
          
        peakPos:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iFuzzyTwinPeaks(self,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy function of two peaks.

        Parameters
        ----------
        location:float
          
        width:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGamma(self,
        gamma:float=1,
        LIMIT:bool=False):
        """ Gamma correction for brightness.

        Parameters
        ----------
        gamma:float
          0.0...
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGaussianAverage(self,
        width:list=[1,0],
        radius:float=0.5):
        """ Gaussian blur implemented as quick Wx1 and 1xH filtering.

        Parameters
        ----------
        width:list
          
        radius:float
          distance, relative to width and height, where gaussian kernel obtains value 0.5.

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGradient(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Computes horizontal and vertical derivatives: df/di and df/dj.

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGradientHorz(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Horizontal intensity difference

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGradientVert(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Vertical intensity difference

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iGray(self,
        coeff:str='1.0',
        normalize:bool=True):
        """ Convert multi-channel image to single. Post-scaling with desired functor.

        Parameters
        ----------
        coeff:str
          c[:c2:c3:...]
        normalize:bool
          true|false

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iHighBoost(self,
        width:int=3,
        height:int=3,
        coeff:float=0.5):
        """ Mixture of original and high-pass filtered image

        Parameters
        ----------
        width:int
          
        height:int
          
        coeff:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iHighPass(self,
        width:int=1,
        height:int=1,
        scale:float=1,
        offset:float=0,
        LIMIT:bool=True):
        """ High-pass filter for recognizing details.

        Parameters
        ----------
        width:int
          
        height:int
          
        scale:float
          
        offset:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iImpulseAvg(self,
        decay:list=[0.75,0.75,0.75,0.75],
        extendHorz:int=0,
        extendVert:int=0,
        weightThreshold:float=0.05):
        """ Infinite-impulse response type spreading

        Parameters
        ----------
        decay:list
          
        extendHorz:int
          pix
        extendVert:int
          pix
        weightThreshold:float
          [0..1.0]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iLaplace(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Second intensity derivatives, (df/di)^2 and (df/dj)^2

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iLaplaceHorz(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Second horizontal differential

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iLaplaceVert(self,
        radius:int=1,
        LIMIT:bool=True):
        """ Second vertical differential

        Parameters
        ----------
        radius:int
          pix
        LIMIT:bool
          0|1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMagnitude(self,
        functor:str=''):
        """ Computes the magnitude of a pixel vector. Post-scaling with desired functor.

        Parameters
        ----------
        functor:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMarginStat(self,
        mode:str='horz',
        stat:str='asmdvNX',
        medianPos:float=0.5):
        """ Computes statistics on <horz> or <vert> lines: iAverage,sum,median,stdDev,variance,miN,maX

        Parameters
        ----------
        mode:str
          
        stat:str
          
        medianPos:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMax(self):
        """ Maximum of two values.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMedian(self,
        width:list=[1,1],
        percentage:float=0.5,
        bins:int=256):
        """ A pipeline implementation of window median.

        Parameters
        ----------
        width:list
          
        percentage:float
          
        bins:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMin(self):
        """ Minimum of two values.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMix(self,
        coeff:float=0.5,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        Parameters
        ----------
        coeff:float
          
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMul(self,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        Parameters
        ----------
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iMultiThreshold(self,
        range:list=[0,1],
        min:float=0,
        max:float=1):
        """ Change values outside given range to target.min and target.max

        Parameters
        ----------
        range:list
          accepted values [min:max]
        min:float
          value set below range.min
        max:float
          value set above range.max

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iNegate(self):
        """ Invert values (unsigned char or unsigned short int)


        """

        cmd = self.make_cmd(locals())
        return cmd


    def iPhysical(self,
        physical:bool=False):
        """ Flag. Handle intensities as physical quantities instead of storage typed values.

        Parameters
        ----------
        physical:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iProduct(self,
        functor:str=''):
        """ Computes the dot product of pixel vectors. Post-scaling with desired functor.

        Parameters
        ----------
        functor:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iQualityMixer(self,
        coeff:float=0.5):
        """ Weighted blending of two images.

        Parameters
        ----------
        coeff:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iQualityOverride(self,
        advantage:float=1,
        decay:float=1):
        """ Compares two images, preserving pixels having higher alpha value. Src alpha is pre-multiplied with advantage.

        Parameters
        ----------
        advantage:float
          0.8..1.2
        decay:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iQualityThreshold(self,
        threshold:float=0,
        replace:float=None):
        """ Threshold data with quality

        Parameters
        ----------
        threshold:float
          0.0...1.0
        replace:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iQuantizator(self,
        bits:int=4,
        LIMIT:bool=False):
        """ Quantize to n bits. (For integer images)

        Parameters
        ----------
        bits:int
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iRemap(self,
        fromValue:float=0,
        toValue:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        Parameters
        ----------
        fromValue:float
          
        toValue:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iRescale(self,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales values linerarly: y = scale*x + bias

        Parameters
        ----------
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iResize(self,
        width:int=0,
        height:int=0,
        interpolation:str='n',
        scale:float=1):
        """ Resize geometry and scale intensities

        Parameters
        ----------
        width:int
          pix
        height:int
          pix
        interpolation:str
          n=nearest,b=bilinear
        scale:float
          rescaling factor

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iRunLengthHorz(self,
        threshold:float=0):
        """ Computes lengths of segments of intensity above threshold.

        Parameters
        ----------
        threshold:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iRunLengthVert(self,
        threshold:float=0):
        """ Computes lengths of segments of intensity above threshold.

        Parameters
        ----------
        threshold:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iSegmentArea(self,
        intensity:list=[1,1.79769e+308],
        functor:str=''):
        """ Computes segment sizes.

        Parameters
        ----------
        intensity:list
          min:max
        functor:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iSegmentStats(self,
        statistics:str='d',
        intensity:list=[1,255],
        functor:str=''):
        """ Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation

        Parameters
        ----------
        statistics:str
          aAxXyYsSlhve
        intensity:list
          min:max
        functor:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iSub(self,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Subtracts values

        Parameters
        ----------
        scale:float
          
        bias:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iSuperProber(self,
        threshold:int=128,
        filename:str='',
        debug:int=0):
        """ Hierarchical segment prober

        Parameters
        ----------
        threshold:int
          min intensity
        filename:str
          
        debug:int
          Dump intermediate control images etc

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iThreshold(self,
        threshold:float=0.5,
        replace:float=0,
        LIMIT:bool=False):
        """ Resets values lower than a threshold

        Parameters
        ----------
        threshold:float
          
        replace:float
          
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iThresholdBinary(self,
        threshold:list=[0.5,0.5],
        replace:list=[0,1],
        LIMIT:bool=False):
        """ Resets values lower and higher than a threshold

        Parameters
        ----------
        threshold:list
          min[:max]
        replace:list
          min[:max]
        LIMIT:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def iTranspose(self):
        """ Flips image matrix around its corner.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def iWindowHistogram(self,
        width:list=[1,1],
        valueFunc:str='a',
        percentage:float=0.5,
        bins:int=256):
        """ A pipeline implementation of window histogram; valueFunc=[asmdvNX] (avg,sum,median,stddev,variance,miN,maX)

        Parameters
        ----------
        width:list
          
        valueFunc:str
          asmdvXN
        percentage:float
          
        bins:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def image(self):
        """ Copies data to a separate image object. Encoding can be changed with --target .


        """

        cmd = self.make_cmd(locals())
        return cmd


    def imageAlpha(self):
        """ Adds a transparency channel. Implies additional image, creates one if needed. See --encoding


        """

        cmd = self.make_cmd(locals())
        return cmd


    def imageBox(self,
        i:list=[0,0],
        j:list=[0,0],
        value:str='0'):
        """ Set intensity at (i:i2,j:j2) to (f1,f2,f3,...).

        Parameters
        ----------
        i:list
          i:i2
        j:list
          j:j2
        value:str
          <f1>[:f2:f3:alpha]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def imageFlatten(self,
        bgcolor:str='0'):
        """ Removes a alpha (transparency) channel. Adds a new background of given color.

        Parameters
        ----------
        bgcolor:str
          <gray>|<red>,<green>,<blue>

        """

        cmd = self.make_cmd(locals())
        return cmd


    def imagePhysical(self,
        physical:bool=False):
        """ Flag. Handle intensities as physical quantities instead of storage typed values.

        Parameters
        ----------
        physical:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def imageQuality(self,
        groups:str='dataset:data'):
        """ Applied parent groups for quality: dataset:data

        Parameters
        ----------
        groups:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def imageTransp(self,
        ftor:str='',
        undetect:float=0,
        nodata:float=1):
        """ Adds a transparency channel. Uses copied image, creating one if needed.

        Parameters
        ----------
        ftor:str
          min:max or Functor_p1_p2_p3_...
        undetect:float
          opacity of 'undetect' pixels
        nodata:float
          opacity of 'nodata' pixels

        """

        cmd = self.make_cmd(locals())
        return cmd


    # TODO: key == commandBank.defaultCmdKey...
    def inputFile(self,
        filename:str=''):
        """ Read HDF5, text or image file

        Parameters
        ----------
        filename:str
          <filename>.[h5|hdf5|png|pgm|ppm|txt]

        """

        cmd = self.make_cmd(locals())
        cmd.set_implicit()
        return cmd


    def inputFilter(self,
        ATTRIBUTES:str='3'):
        """ Partial file read. You probably search for --inputSelect

        Parameters
        ----------
        ATTRIBUTES:str
          ATTRIBUTES=1,DATASETS=2,MARKED=4

        """

        cmd = self.make_cmd(locals())
        return cmd


    def inputPalette(self,
        filename:str=''):
        """ Load palette.

        Parameters
        ----------
        filename:str
          <filename>.[txt|json]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def inputPrefix(self,
        value:str=''):
        """ Path prefix for input files.

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def inputSelect(self,
        value:str=''):
        """ Selector for input data.

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def jSON(self,
        property:str=''):
        """ Export a command to JSON.

        Parameters
        ----------
        property:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def keep(self,
        path:str='',
        quantity:str='',
        elangle:list=[-90,90],
        count:int=4095,
        order:str='DATA:MIN',
        prf:str='ANY',
        timespan:list=[0,0]):
        """ Keeps selected part of data structure, deletes rest.

        Parameters
        ----------
        path:str
          [/]dataset<i>[/data<j>|/quality<j>]
        quantity:str
          DBZH|VRAD|RHOHV|...
        elangle:list
          min[:max]
        count:int
          
        order:str
          ["DATA","ELANGLE","TIME"]:["MIN","MAX"]
        prf:str
          ["ANY","SINGLE","DOUBLE"]
        timespan:list
          range from nominal time [seconds]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def legendOut(self,
        params:str=''):
        """ Deprecating command

        Parameters
        ----------
        params:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def log(self,
        file:str='',
        level:str='',
        timing:bool=False,
        vt100:bool=True):
        """ Redirect log to file. Status variables like ${ID}, ${PID} and ${CTX} supported.

        Parameters
        ----------
        file:str
          
        level:str
          
        timing:bool
          
        vt100:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def move(self,
        src:str='',
        dst:str=''):
        """ Rename or move data groups and attributes.

        Parameters
        ----------
        src:str
          /group/group2[:attr]
        dst:str
          /group/group2[:attr]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def odim(self,
        version:str='2.4'):
        """ Set ODIM version (experimental)

        Parameters
        ----------
        version:str
          ["2.2","2.3","2.4","RACK","KILOMETRES"]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputConf(self,
        value:str='<format>:<key>=value>,conf...'):
        """ Format (h5|tif|png|tre|dot) specific configurations

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputFile(self,
        filename:str=''):
        """ Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.

        Parameters
        ----------
        filename:str
          <filename>.[h5|hdf5|png|pgm|txt|dat|mat|dot]|-

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputPalette(self,
        filename:str=''):
        """ Save palette as TXT, JSON, SVG or SLD.

        Parameters
        ----------
        filename:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputPrefix(self,
        value:str=''):
        """ Path prefix for output files.

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputRawImages(self,
        filename:str=''):
        """ Output datasets to png files named filename[NN].ext.

        Parameters
        ----------
        filename:str
          string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def outputTree(self,
        filename:str=''):
        """ Output data as simple tree structure.

        Parameters
        ----------
        filename:str
          <filename>|-

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pAdd(self):
        """ Add polar data to accumulation array.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def pAddWeighted(self,
        weight:float=1):
        """ Adds the current product to the composite applying weight.

        Parameters
        ----------
        weight:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pBeamAltitude(self,
        aboveSeaLevel:bool=True):
        """ Computes the altitude at each bin

        Parameters
        ----------
        aboveSeaLevel:bool
          false=radar site|true=sea level

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pBiomet(self,
        reflMax:float=-10,
        maxAltitude:int=500,
        reflDev:float=5,
        devAltitude:int=1000):
        """ Detects birds and insects.

        Parameters
        ----------
        reflMax:float
          dBZ
        maxAltitude:int
          m
        reflDev:float
          dBZ
        devAltitude:int
          m

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pBird(self,
        dbzPeak:list=[-20,0],
        vradDevMin:float=1,
        rhoHVmax:list=[0.99,0.891],
        zdrAbsMin:float=1,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

        Parameters
        ----------
        dbzPeak:list
          Typical reflectivity (dBZ)
        vradDevMin:float
          Fuzzy threshold of Doppler speed (VRAD) deviation (m/s)
        rhoHVmax:list
          Fuzzy threshold of maximum rhoHV value
        zdrAbsMin:float
          Fuzzy threshold of absolute ZDR
        window:list
          beam-directional(m), azimuthal(deg)
        gamma:float
          Contrast adjustment, dark=0.0 < 1.0 < brighter 

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pCappi(self,
        altitude:float=1000,
        aboveSeaLevel:bool=True,
        beamWidth:float=1,
        weightMin:float=-0.1,
        accumulationMethod:str='WAVG:1:8:-40',
        height:bool=False):
        """ Constant-altitude planar position indicator

        Parameters
        ----------
        altitude:float
          metres
        aboveSeaLevel:bool
          
        beamWidth:float
          deg
        weightMin:float
          -0.1|0...1
        accumulationMethod:str
          string
        height:bool
          true|false

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pClutter(self,
        decay:float=0.5,
        gamma:float=1,
        quantity:str='CLUTTER',
        file:str='cluttermaps/cluttermap-${NOD}-${quantity}.h5'):
        """ Reads a ground clutter map and scales it to sweeps.

        Parameters
        ----------
        decay:float
          per 1000m
        gamma:float
          brightness
        quantity:str
          CLUTTER|OBSTACLE|...
        file:str
          path syntax

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pConv(self,
        maxEchoThreshold:float=25,
        cellDiameter:float=3,
        echoTopThreshold:float=2,
        echoTopDBZ:float=20,
        smoothAzm:float=0,
        smoothRad:float=0):
        """ Computes the probability of convection based on fuzzy cell intensity, area and height.

        Parameters
        ----------
        maxEchoThreshold:float
          dBZ
        cellDiameter:float
          km
        echoTopThreshold:float
          km
        echoTopDBZ:float
          dBZ
        smoothAzm:float
          deg
        smoothRad:float
          km

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerAvg(self,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False,
        relativeScale:bool=False):
        """ Smoothens Doppler field, providing quality

        Parameters
        ----------
        width:float
          metres
        height:float
          deg
        threshold:float
          percentage
        compensate:bool
          cart/polar[0|1]
        relativeScale:bool
          false|true

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerAvgExp(self,
        decay:list=[0.75,0.75,0.75,0.75],
        horzExtension:int=0,
        vertExtension:int=0):
        """ Doppler field smoother with exponential decay weighting

        Parameters
        ----------
        decay:list
          
        horzExtension:int
          pix
        vertExtension:int
          pix

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerCrawler(self,
        nyquist:float=100,
        threshold:float=0.9):
        """ Creates virtual 

        Parameters
        ----------
        nyquist:float
          max-unamb-velocity
        threshold:float
          relative speed

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerDev(self,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False,
        relativeScale:bool=True):
        """ Computes std.dev.[m/s] of Doppler field

        Parameters
        ----------
        width:float
          metres
        height:float
          deg
        threshold:float
          percentage
        compensate:bool
          cart/polar[0|1]
        relativeScale:bool
          true|false

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerDiff(self,
        dMax:float=0):
        """ Azimuthal difference of VRAD

        Parameters
        ----------
        dMax:float
          m/s

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerDiffPlotter(self,
        azm:list=[0,0],
        range:list=[0,0],
        ray:list=[0,0],
        bin:list=[0,0]):
        """ Plots differences in VRAD data as fucntion of azimuth

        Parameters
        ----------
        azm:list
          deg
        range:list
          km
        ray:list
          index
        bin:list
          index

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerEccentricity(self,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False):
        """ Magnitude of mean unit circle mapped Doppler speeds

        Parameters
        ----------
        width:float
          metres
        height:float
          deg
        threshold:float
          percentage
        compensate:bool
          cart/polar[0|1]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerInversion(self,
        width:int=500,
        height:float=3,
        altitudeWeight:str=''):
        """ Derives 2D wind (u,v) from aliased Doppler data.

        Parameters
        ----------
        width:int
          metres
        height:float
          degrees
        altitudeWeight:str
          Functor:a:b:c...

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerReproject(self,
        nyquist:float=100,
        match:int=0,
        quantity:str='VRAD'):
        """ Creates virtual 

        Parameters
        ----------
        nyquist:float
          max-unamb-velocity
        match:int
          flag(aliased=1,nodata=2)
        quantity:str
          output-quantity

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDopplerSampler(self,
        azm:list=[0,0],
        range:list=[0,0],
        ray:list=[0,0],
        bin:list=[0,0]):
        """ Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)

        Parameters
        ----------
        azm:list
          deg
        range:list
          km
        ray:list
          index
        bin:list
          index

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pDrawing(self,
        shape:str='direction',
        p1:float=0,
        p2:float=0,
        p3:float=0,
        p4:float=0,
        marker:int=255):
        """ Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].

        Parameters
        ----------
        shape:str
          [direction|range|sector|arc]
        p1:float
          
        p2:float
          
        p3:float
          
        p4:float
          
        marker:int
          intensity

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pEchoTop(self,
        threshold:float=20,
        reference:list=[-50,15000],
        undetectValue:float=-32,
        weights:list=[1,0.8,0.6,0.4,0.2],
        weightDecay:float=20,
        avgWindow:list=[0,0],
        EXTENDED:bool=True):
        """ Estimates maximum altitude of given reflectivity

        Parameters
        ----------
        threshold:float
          reflectivity limit (dB)
        reference:list
          'dry point' of low reflectivity and high altitude [dBZ:m]
        undetectValue:float
          reflectivity replacing 'undetect' [dBZ]
        weights:list
          weights for INTERPOLATION, INTERP_UNDET, EXTRAP_UP, EXTRAP_DOWN, CLEAR
        weightDecay:float
          radius from threshold in decreasing weight [dBZ]
        avgWindow:list
          optional reference window [metres,degrees]
        EXTENDED:bool
          store also DBZ_SLOPE and CLASS

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pEchoTopOpOld(self,
        minDBZ:float=20,
        dbzReference:float=-50,
        altitudeReference:float=15000,
        aboveSeaLevel:bool=True):
        """ Computes maximum (or minimum) altitude of echo.

        Parameters
        ----------
        minDBZ:float
          dBZ
        dbzReference:float
          dBZ
        altitudeReference:float
          metres
        aboveSeaLevel:bool
          false=radar site|true=sea level

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pExtract(self,
        channels:str='dw'):
        """ Extract polar-coordinate data that has been accumulated.

        Parameters
        ----------
        channels:str
          Layers: data,count,weight,std.deviation

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pFunctor(self,
        ftor:str=''):
        """ Maps values using a function

        Parameters
        ----------
        ftor:str
          Functor:a:b:c...

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pMaxEcho(self,
        altitude:list=[1000,9000],
        accumulationMethod:str='MAXIMUM'):
        """ Computes maximum dBZ inside altitude range

        Parameters
        ----------
        altitude:list
          metres
        accumulationMethod:str
          MAXIMUM|AVERAGE|WAVG:2:2|MAXW

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pOutputQuantity(self,
        productCmd:str=''):
        """ Return default outout quantity

        Parameters
        ----------
        productCmd:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pPlot(self,
        lon:float=0,
        lat:float=0,
        value:float=0,
        weight:float=1):
        """ Add a single data point.

        Parameters
        ----------
        lon:float
          longitude
        lat:float
          latitude
        value:float
          value
        weight:float
          weight

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pPlotFile(self,
        file:str=''):
        """ Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.

        Parameters
        ----------
        file:str
          filename

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pPolarSlidingAvg(self,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        invertPolar:bool=False):
        """ Smoothen polar data

        Parameters
        ----------
        width:float
          metres
        height:float
          deg
        threshold:float
          percentage
        invertPolar:bool
          cart/polar

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pPseudoRhi(self,
        az_angle:float=0,
        range:list=[0,0],
        height:list=[0,10000],
        size:list=[500,250],
        beamWidth:float=0.25,
        beamThreshold:float=0.01):
        """ Computes vertical intersection in a volume in the beam direction.

        Parameters
        ----------
        az_angle:float
          deg
        range:list
          m
        height:list
          m
        size:list
          pix
        beamWidth:float
          deg
        beamThreshold:float
          0..1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pRainRate(self):
        """ Estimates precipitation rate [mm/h] from reflectance [dBZ].


        """

        cmd = self.make_cmd(locals())
        return cmd


    def pRainRateDP(self,
        dbz:list=[30,50],
        zdr:list=[0.15,0.25],
        rhohv:float=0.85,
        kdp:list=[0.25,0.35]):
        """ Precip. rate [mm/h] from dual-pol using fuzzy thresholds. Alg. by Brandon Hickman

        Parameters
        ----------
        dbz:list
          heavy:hail
        zdr:list
          heavy
        rhohv:float
          met
        kdp:list
          heavy

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pSite(self,
        location:list=[25.2,60.1]):
        """ Set radar size location of the accumulated data. Also size etc., if --encoding set.

        Parameters
        ----------
        location:list
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pSunShine(self,
        timestamp:str='200527071845',
        quantity:str='SUNSHINE'):
        """ Computes the sun position

        Parameters
        ----------
        timestamp:str
          
        quantity:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pSweep(self,
        quantity:str='DBZH',
        index:int=0):
        """ Return a single sweep

        Parameters
        ----------
        quantity:str
          
        index:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pVerticalProfile(self,
        range:list=[10,100],
        height:list=[0,10000],
        levels:int=100,
        azm:list=[0,359.99],
        azSlots:int=1):
        """ Computes vertical dBZ distribution in within range [minRange,maxRange] km.

        Parameters
        ----------
        range:list
          km
        height:list
          m
        levels:int
          
        azm:list
          deg
        azSlots:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def palette(self,
        filename:str=''):
        """ Load and apply palette.

        Parameters
        ----------
        filename:str
          <filename>.[txt|json]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def paletteConf(self,
        key:str='list',
        code:str='',
        lenient:bool=True):
        """ Check status of palette(s).

        Parameters
        ----------
        key:str
          
        code:str
          
        lenient:bool
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def paletteDefault(self):
        """ Apply default palette matching the quantity of current data.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def paletteIn(self,
        filename:str=''):
        """ Load palette.

        Parameters
        ----------
        filename:str
          <filename>.[txt|json]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def paletteOut(self,
        params:str=''):
        """ Deprecating command

        Parameters
        ----------
        params:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def paletteRefine(self,
        count:int=0):
        """ Refine colors

        Parameters
        ----------
        count:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def pause(self,
        seconds:str='random'):
        """ Pause for n or random seconds

        Parameters
        ----------
        seconds:str
          [<number>|random]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def plot(self,
        i,j:str='0,0'):
        """ Plot

        Parameters
        ----------
        i,j:str
          <i>,<j>,...

        """

        cmd = self.make_cmd(locals())
        return cmd


    def polarSelect(self,
        azimuth:list=[0,0],
        azimuthStep:float=0,
        radius:list=[0,0],
        radiusStep:float=0):
        """ Data selector for the next computation

        Parameters
        ----------
        azimuth:list
          
        azimuthStep:float
          
        radius:list
          
        radiusStep:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def precipKDP(self,
        a:float=6.95258e-310,
        b:float=0):
        """ Precip rate from KDP

        Parameters
        ----------
        a:float
          
        b:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def precipKDPZDR(self,
        a:float=6.95258e-310,
        b:float=0,
        c:float=6.08801e-310):
        """ Precipitation rate from KDP and ZDR

        Parameters
        ----------
        a:float
          
        b:float
          
        c:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def precipZZDR(self,
        a:float=0.0122,
        b:float=0.82,
        c:float=-2.28):
        """ Precipitation rate from Z and ZDR

        Parameters
        ----------
        a:float
          
        b:float
          
        c:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def precipZrain(self,
        a:float=200,
        b:float=1.6):
        """ Precipitation rate from Z (reflectivity)

        Parameters
        ----------
        a:float
          
        b:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def precipZsnow(self,
        a:float=223,
        b:float=1.53):
        """ Precipitation rate from Z (reflectivity)

        Parameters
        ----------
        a:float
          
        b:float
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def python(self,
        file:str='dump.py',
        count:int=0):
        """ Export command interfaces to Python. Compatible with rack.prog module.

        Parameters
        ----------
        file:str
          
        count:int
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def quantityConf(self,
        quantity:str='',
        encoding:str='',
        zero:str=''):
        """ 1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair

        Parameters
        ----------
        quantity:str
          quantity (DBZH,VRAD,...)
        encoding:str
          C,0,0,0,0
        zero:str
          number or NaN

        """

        cmd = self.make_cmd(locals())
        # note: separator ':'
        cmd.set_separators(':', ',')
        return cmd


    def restart(self):
        """ Debug


        """

        cmd = self.make_cmd(locals())
        return cmd


    def sample(self,
        iStep:int=10,
        jStep:int=0,
        i:list=[-1,1],
        j:list=[-1,1],
        commentChar:str='#',
        skipVoid:bool=False,
        handleVoid:str='null'):
        """ Extract samples. See --format.

        Parameters
        ----------
        iStep:int
          horz coord step
        jStep:int
          vert coord step
        i:list
          horz index or range
        j:list
          vert index or range
        commentChar:str
          comment prefix (char or bytevalue)
        skipVoid:bool
          skip lines with invalid/missing values
        handleVoid:str
          skip or mark invalid values [skip|null|<number>]

        """

        cmd = self.make_cmd(locals())
        return cmd


    # NOTE: key == commandBank.scriptCmd  TODO SCRIPT QUOTE check...

    def script(self,
        script:str=''):
        """ Define script.

        Parameters
        ----------
        script:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def select(self,
        path:str='',
        quantity:str='',
        elangle:list=[-90,90],
        count:int=4095,
        order:str='DATA:MIN',
        prf:str='ANY',
        timespan:list=[0,0]):
        """ Data selector for the next computation

        Parameters
        ----------
        path:str
          [/]dataset<i>[/data<j>|/quality<j>]
        quantity:str
          DBZH|VRAD|RHOHV|...
        elangle:list
          min[:max]
        count:int
          
        order:str
          ["DATA","ELANGLE","TIME"]:["MIN","MAX"]
        prf:str
          ["ANY","SINGLE","DOUBLE"]
        timespan:list
          range from nominal time [seconds]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def selectObject(self,
        flags:str='INPUT|POLAR|CARTESIAN'):
        """ Select input object for the next operation

        Parameters
        ----------
        flags:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def selectQuantity(self,
        quantities:str=''):
        """ Like --select quantity=... 

        Parameters
        ----------
        quantities:str
          quantity[,quantity2,...]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def set(self,
        assignment:str=''):
        """ Set general-purpose variables

        Parameters
        ----------
        assignment:str
          key=value

        """

        cmd = self.make_cmd(locals())
        return cmd


    def setODIM(self,
        assignment:str=''):
        """ Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM

        Parameters
        ----------
        assignment:str
          /<path>:<key>[=<value>]

        """

        cmd = self.make_cmd(locals())
        return cmd


    def status(self):
        """ Dump information on current images.


        """

        cmd = self.make_cmd(locals())
        return cmd


    def stopOnError(self,
        flags:str=''):
        """ Stop on given error condition(s).

        Parameters
        ----------
        flags:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def store(self,
        level:str='0'):
        """ Request additional (debugging) outputs

        Parameters
        ----------
        level:str
          DEFAULT=0,INTERMEDIATE=1,QUALITY=4,DEBUG=2

        """

        cmd = self.make_cmd(locals())
        return cmd


    def target(self,
        type:str='C',
        gain:float=0,
        offset:float=0,
        undetect:float=0,
        nodata:float=0,
        rscale:float=0,
        nrays:int=0,
        nbins:int=0,
        quantity:str=''):
        """ Sets encodings parameters for polar and Cartesian products, including composites.

        Parameters
        ----------
        type:str
          storage type (C=unsigned char, S=unsigned short, d=double precision float, f=float,...)
        gain:float
          scaling coefficient
        offset:float
          bias
        undetect:float
          marker
        nodata:float
          marker
        rscale:float
          metres
        nrays:int
          count
        nbins:int
          count
        quantity:str
          string

        """

        cmd = self.make_cmd(locals())
        return cmd


    def testSelect(self):
        """ Test...


        """

        cmd = self.make_cmd(locals())
        return cmd


    def testTree(self,
        value:str=''):
        """ Dump XML track

        Parameters
        ----------
        value:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def undetectWeight(self,
        weight:float=0.75):
        """ Set the relative weight of data values assigned 'undetect'.

        Parameters
        ----------
        weight:float
          0...1

        """

        cmd = self.make_cmd(locals())
        return cmd


    def updateVariables(self):
        """ Force calling DataTools::updateInternalAttributes(ctx.getHi5(Hdf5Context::CURRENT))


        """

        cmd = self.make_cmd(locals())
        return cmd


    def validate(self,
        filename:str=''):
        """ Read CVS file 

        Parameters
        ----------
        filename:str
          <filename>.cvs

        """

        cmd = self.make_cmd(locals())
        return cmd


    def verbose(self,
        level:str='NOTE',
        imageLevel:str='WARNING'):
        """ Set verbosity level

        Parameters
        ----------
        level:str
          
        imageLevel:str
          

        """

        cmd = self.make_cmd(locals())
        return cmd


    def version(self):
        """ Displays software version and quits.


        """

        cmd = self.make_cmd(locals())
        return cmd


