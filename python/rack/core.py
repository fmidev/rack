import rack.prog
class Rack(rack.prog.Register):
    def aAltitude(self ,
        functor:str='',
        aboveSeaLevel:bool=True,
        bias:float=0):
        """ Weights altitude

        --- Parameters ---
        functor:
        aboveSeaLevel:true
        bias:0
        """
        return self._make_cmd(locals())

    def aAttenuation(self ,
        reflHalfWidth:float=3,
        c:float=1.12e-07,
        p:float=0.62,
        c2:float=0,
        p2:float=0):
        """ Computes attenuation and converts it to probability

        --- Parameters ---
        reflHalfWidth:3
        c:1.12e-07
        p:0.62
        c2:0
        p2:0
        """
        return self._make_cmd(locals())

    def aBiomet(self ,
        reflMax:float=-10,
        maxAltitude:int=500,
        reflDev:float=5,
        devAltitude:int=1000):
        """ Detects birds and insects.

        --- Parameters ---
        reflMax:-10
        maxAltitude:500
        reflDev:5
        devAltitude:1000
        """
        return self._make_cmd(locals())

    def aBird(self ,
        dbzPeak:list=[-20,0],
        vradDevMin:float=1,
        rhoHVmax:list=[0.99,0.891],
        zdrAbsMin:float=1,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

        --- Parameters ---
        dbzPeak:-20:0
        vradDevMin:1
        rhoHVmax:0.99:0.891
        zdrAbsMin:1
        window:2500:5
        gamma:1
        """
        return self._make_cmd(locals())

    def aCCor(self ,
        threshold:list=[10,30],
        mask:bool=False):
        """ Detects clutter. Based on difference of DBZH and TH. At halfwidth PROB=0.5. Universal.

        --- Parameters ---
        threshold:10:30
        mask:false
        """
        return self._make_cmd(locals())

    def aChaff(self ,
        dbzPeak:list=[5,15],
        vradDevMax:float=5,
        rhoHVmax:list=[0.5,0.4],
        zdrAbsMin:float=0.5,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates chaff probability from DBZH, VRAD, RhoHV and ZDR.

        --- Parameters ---
        dbzPeak:5:15
        vradDevMax:5
        rhoHVmax:0.5:0.4
        zdrAbsMin:0.5
        window:2500:5
        gamma:1
        """
        return self._make_cmd(locals())

    def aClutter(self ,
        decay:float=0.5,
        gamma:float=1,
        quantity:str='CLUTTER',
        file:str='cluttermaps/cluttermap-${NOD}-${quantity}.h5'):
        """ Reads a ground clutter map and scales it to sweeps.

        --- Parameters ---
        decay:0.5
        gamma:1
        quantity:CLUTTER
        file:cluttermaps/cluttermap-${NOD}-${quantity}.h5
        """
        return self._make_cmd(locals())

    def aDamper(self ,
        threshold:float=0.5,
        undetectThreshold:float=0.1,
        minValue:float=None):
        """ Removes anomalies with fuzzy deletion operator.

        --- Parameters ---
        threshold:0.5
        undetectThreshold:0.1
        minValue:null
        """
        return self._make_cmd(locals())

    def aDefault(self ,
        probability:float=0.8):
        """ Marks the data values as unclassified, with high probability

        --- Parameters ---
        probability:0.8
        """
        return self._make_cmd(locals())

    def aDefaultQuality(self ,
        threshold:float=0.8):
        """ Quality index value below which also CLASS information will be updated.

        --- Parameters ---
        threshold:0.8
        """
        return self._make_cmd(locals())

    def aDopplerNoise(self ,
        speedDevThreshold:float=3,
        windowWidth:float=1500,
        windowHeight:float=3):
        """ Detects suspicious variance in Doppler speed (VRAD).

        --- Parameters ---
        speedDevThreshold:3
        windowWidth:1500
        windowHeight:3
        """
        return self._make_cmd(locals())

    def aEmitter(self ,
        lengthMin:float=5,
        thicknessMax:float=5,
        sensitivity:float=0.5):
        """ Detects electromagnetic interference segments by means of window medians.

        --- Parameters ---
        lengthMin:5
        thicknessMax:5
        sensitivity:0.5
        """
        return self._make_cmd(locals())

    def aGapFill(self ,
        width:int=1500,
        height:float=5,
        qualityThreshold:float=0.1):
        """ Removes low-quality data with gap fill based on distance transformation.

        --- Parameters ---
        width:1500
        height:5
        qualityThreshold:0.1
        """
        return self._make_cmd(locals())

    def aGapFillRec(self ,
        width:int=1500,
        height:float=5,
        loops:int=3):
        """ Recursive, 'splinic' gap filler.

        --- Parameters ---
        width:1500
        height:5
        loops:3
        """
        return self._make_cmd(locals())

    def aHydroClassBased(self ,
        nonMet:float=0.5,
        NOMET:int=1):
        """ Sets probabilities of non-meteorological echoes based on precomputed hydrometeor CLASS.

        --- Parameters ---
        nonMet:0.5
        NOMET:1
        """
        return self._make_cmd(locals())

    def aInsect(self ,
        dbzMax:list=[-5,5],
        vradDevMax:float=5,
        rhoHVmax:list=[0.99,0.792],
        zdrAbsMin:float=3,
        window:list=[2500,5],
        gamma:float=1):
        """ Probability of insects, based on DBZH, VRAD, RhoHV and ZDR.

        --- Parameters ---
        dbzMax:-5:5
        vradDevMax:5
        rhoHVmax:0.99:0.792
        zdrAbsMin:3
        window:2500:5
        gamma:1
        """
        return self._make_cmd(locals())

    def aJamming(self ,
        smoothnessThreshold:float=5,
        distanceMin:float=80,
        refit:bool=True):
        """ Detects broad lines caused by electromagnetic interference. Intensities should be smooth, increasing by distance.

        --- Parameters ---
        smoothnessThreshold:5
        distanceMin:80
        refit:true
        """
        return self._make_cmd(locals())

    def aNonMet(self ,
        threshold:list=[0.7,0.8],
        medianWindow:list=[0,0],
        medianPos:float=0):
        """ Detects clutter. Based on dual-pol parameter RhoHV . Optional post processing: morphological closing. Universal.

        --- Parameters ---
        threshold:0.7:0.8
        medianWindow:0:0
        medianPos:0
        """
        return self._make_cmd(locals())

    def aNonMet2(self ,
        threshold:list=[0.75,0.95],
        medianWindow:list=[0,0],
        medianPos:float=0):
        """ Estimates probability from DBZH, VRAD, RhoHV and ZDR.

        --- Parameters ---
        threshold:0.75:0.95
        medianWindow:0:0
        medianPos:0
        """
        return self._make_cmd(locals())

    def aPrecip(self ,
        probMax:float=0.5,
        dbz:float=20,
        dbzSpan:float=10):
        """ Detects precipitation...

        --- Parameters ---
        probMax:0.5
        dbz:20
        dbzSpan:10
        """
        return self._make_cmd(locals())

    def aQualityCombiner(self ):
        """ Combines detection probabilities to overall quality field QIND (and CLASS).

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def aRemover(self ,
        threshold:float=0.5,
        replace:str='nodata',
        clearQuality:bool=True):
        """ Simple anomaly removal operator.

        --- Parameters ---
        threshold:0.5
        replace:nodata
        clearQuality:true
        """
        return self._make_cmd(locals())

    def aShip(self ,
        reflMin:float=25,
        reflDev:float=15,
        windowWidth:int=1500,
        windowHeight:float=3):
        """ Detects ships based on their high absolute reflectivity and local reflectivity difference.

        --- Parameters ---
        reflMin:25
        reflDev:15
        windowWidth:1500
        windowHeight:3
        """
        return self._make_cmd(locals())

    def aSpeckle(self ,
        threshold:float=-20,
        area:int=16,
        invertPolar:bool=False):
        """ Detects speckle noise. Universal: uses DBZ data as input, applies to all data in a sweep group.

        --- Parameters ---
        threshold:-20
        area:16
        invertPolar:false
        """
        return self._make_cmd(locals())

    def aSun(self ,
        beamWidth:float=1,
        sensitivity:float=0.95):
        """ Draw the sun beam

        --- Parameters ---
        beamWidth:1
        sensitivity:0.95
        """
        return self._make_cmd(locals())

    def aTest(self ,
        value:float=0,
        prob:float=1,
        i:list=[200,230],
        j:list=[240,300]):
        """ Create rectangular fake anomaly (value) and 'detect' it with desired probability (prob).

        --- Parameters ---
        value:0
        prob:1
        i:200:230
        j:240:300
        """
        return self._make_cmd(locals())

    def aTime(self ,
        time:str='NOMINAL',
        decayPast:float=1,
        decayFuture:float=-1):
        """ Created quality field based on measurement time for each beam.

        --- Parameters ---
        time:NOMINAL
        decayPast:1
        decayFuture:-1
        """
        return self._make_cmd(locals())

    def aUniversal(self ,
        unversal:bool=True):
        """ Toggle the support for universal ie. Dataset-wide quality indices.

        --- Parameters ---
        unversal:true
        """
        return self._make_cmd(locals())

    def append(self ,
        path:str=''):
        """ Append inputs/products (empty=overwrite).

        --- Parameters ---
        path:
        """
        return self._make_cmd(locals())

    def cAdd(self ):
        """ Adds the current product to the composite.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cAddWeighted(self ,
        weight:float=1):
        """ Adds the current product to the composite applying weight.

        --- Parameters ---
        weight:1
        """
        return self._make_cmd(locals())

    def cBBox(self ,
        llLon:float=0,
        llLat:float=0,
        urLon:float=0,
        urLat:float=0):
        """ Bounding box of the Cartesian product.

        --- Parameters ---
        llLon:0
        llLat:0
        urLon:0
        urLat:0
        """
        return self._make_cmd(locals())

    def cBBoxReset(self ):
        """ Resets the bounding box (to be set again according to the next radar data).

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cBBoxTest(self ,
        mode:int=0):
        """ Tests whether the radar range is inside the composite.

        --- Parameters ---
        mode:0
        """
        return self._make_cmd(locals())

    def cBBoxTile(self ,
        llLon:float=0,
        llLat:float=0,
        urLon:float=0,
        urLat:float=0):
        """ Redefines bbox and compositing array size for several radars, applying original projection and resolution. See --cSize, --cBBox, --cProj.

        --- Parameters ---
        llLon:0
        llLat:0
        urLon:0
        urLat:0
        """
        return self._make_cmd(locals())

    def cCreate(self ):
        """ Maps the current polar product to a Cartesian product.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cCreateLookup(self ):
        """ Creates lookup objects

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cCreateSun(self ,
        timestamp:str='200507271845'):
        """ Cartesian sun shine field.

        --- Parameters ---
        timestamp:200507271845
        """
        return self._make_cmd(locals())

    def cCreateTile(self ):
        """ Maps the current polar product to a tile to be used in compositing.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cDecayTime(self ,
        time:int=0):
        """ Delay half-time in minutes. 0=no decay

        --- Parameters ---
        time:0
        """
        return self._make_cmd(locals())

    def cDefaultQuality(self ,
        weight:float=0.75):
        """ Set default quality (for data without quality field)

        --- Parameters ---
        weight:0.75
        """
        return self._make_cmd(locals())

    def cExtract(self ,
        channels:str='DATA,WEIGHT',
        bbox:str=''):
        """ Extract (export) data from the internal accumulation array

        --- Parameters ---
        channels:DATA,WEIGHT
        bbox:
        """
        return self._make_cmd(locals())

    def cGrid(self ,
        lonSpacing:int=1,
        latSpacing:int=1,
        width:float=1,
        intensity:float=0.5):
        """ Draw lat-lon grid onto a Cartesian product.

        --- Parameters ---
        lonSpacing:1
        latSpacing:1
        width:1
        intensity:0.5
        """
        return self._make_cmd(locals())

    def cInit(self ):
        """ Allocate memory to --cSize, applying --target and --select, if set.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cMethod(self ,
        method:str='MAXIMUM'):
        """ Method in accumulating values on a composite.

        --- Parameters ---
        method:MAXIMUM
        """
        return self._make_cmd(locals())

    def cOpticalFlow(self ,
        width:list=[5,5],
        resize:float=0,
        threshold:float=None,
        spread:bool=False,
        smooth:bool=False):
        """ Optical flow computed based on differential accumulation layers.

        --- Parameters ---
        width:5:5
        resize:0
        threshold:null
        spread:false
        smooth:false
        """
        return self._make_cmd(locals())

    def cPlot(self ,
        lon:float=0,
        lat:float=0,
        x:float=0,
        w:float=1):
        """ Add a single data point.

        --- Parameters ---
        lon:0
        lat:0
        x:0
        w:1
        """
        return self._make_cmd(locals())

    def cPlotFile(self ,
        file:str=''):
        """ Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.

        --- Parameters ---
        file:
        """
        return self._make_cmd(locals())

    def cProj(self ,
        projection:str=''):
        """ Set projection

        --- Parameters ---
        projection:
        """
        return self._make_cmd(locals())

    def cRange(self ,
        range:int=250000):
        """ Force a range for single-radar cartesian products (0=use-metadata).

        --- Parameters ---
        range:250000
        """
        return self._make_cmd(locals())

    def cReset(self ):
        """ Clears the current Cartesian product.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def cSize(self ,
        width:int=400,
        height:int=0):
        """ Set size of the compositing array. Does not allocate memory.

        --- Parameters ---
        width:400
        height:0
        """
        return self._make_cmd(locals())

    def cSpread(self ,
        horz:float=10,
        vert:float=0,
        loops:int=0):
        """ Set Spread of the compositing array. OBSOLETE. Use --iDistanceTransformFill(Exp) instead

        --- Parameters ---
        horz:10
        vert:0
        loops:0
        """
        return self._make_cmd(locals())

    def cTime(self ,
        time:str='201412091845'):
        """ Modify the time of the current composite. See --cTimeDecay 

        --- Parameters ---
        time:201412091845
        """
        return self._make_cmd(locals())

    def cTimeDecay(self ,
        weight:float=1):
        """ Weight (0.9...1.0) of delay, per minute. 1.0=no decay. See --cTime and --cDecayTime

        --- Parameters ---
        weight:1
        """
        return self._make_cmd(locals())

    def checkType(self ):
        """ Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def completeODIM(self ):
        """ Ensures ODIM types, for example after reading image data and setting attributes in command line std::strings.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def convert(self ):
        """ Convert --select:ed data to scaling and markers set by --encoding

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def createDefaultQuality(self ,
        data:float=None,
        undetect:float=None,
        nodata:float=None):
        """ Creates default quality field. See --undetectWeight and --aDefault

        --- Parameters ---
        data:null
        undetect:null
        nodata:null
        """
        return self._make_cmd(locals())

    def debug(self ):
        """ Set verbosity to LOG_DEBUG

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def delete(self ,
        selector:str='path,quantity,elangle,count,order,prf,timespan'):
        """ Deletes selected parts of h5 structure.

        --- Parameters ---
        selector:path,quantity,elangle,count,order,prf,timespan
        """
        return self._make_cmd(locals())

    def dumpMap(self ,
        filter:str='',
        filename:str=''):
        """ Dump variable map, filtered by keys, to std or file.

        --- Parameters ---
        filter:
        filename:
        """
        return self._make_cmd(locals())

    def echo(self ,
        format:str=''):
        """ For testing. Dumps a formatted string to stdout.

        --- Parameters ---
        format:
        """
        return self._make_cmd(locals())

    def encoding(self ,
        encoding:str=''):
        """ Sets encodings parameters for polar and Cartesian products, including composites.

        --- Parameters ---
        encoding:
        """
        return self._make_cmd(locals())

    def errorFlags(self ,
        flags:str=''):
        """ Status of last select.

        --- Parameters ---
        flags:
        """
        return self._make_cmd(locals())

    def execFile(self ,
        filename:str=''):
        """ Execute commands from a file.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def execScript(self ):
        """ Execute script.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def expandVariables(self ):
        """ Toggle variable expansions on/off

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def format(self ,
        syntax:str=''):
        """ Set format for data dumps (see --sample or --outputFile)

        --- Parameters ---
        syntax:
        """
        return self._make_cmd(locals())

    def formatFile(self ,
        filename:str=''):
        """ Read format for metadata dump from a file

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def formatOut(self ,
        filename:str=''):
        """ Dumps the formatted std::string to a file or stdout.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def freezingLevel(self ,
        height:float=None,
        thickness:float=0.1):
        """ Freezing level modelled simply as its height and thickness.

        --- Parameters ---
        height:null
        thickness:0.1
        """
        return self._make_cmd(locals())

    def gAlign(self ,
        position:str='',
        anchor:str='',
        anchorHorz:str='',
        anchorVert:str=''):
        """ Alignment of the next element

        --- Parameters ---
        position:
        anchor:
        anchorHorz:
        anchorVert:
        """
        return self._make_cmd(locals())

    def gAlignTest(self ,
        name:str='',
        panel:str='playGround1'):
        """ SVG test product

        --- Parameters ---
        name:
        panel:playGround1
        """
        return self._make_cmd(locals())

    def gDebug(self ,
        name:str='',
        panel:str='playGround1'):
        """ SVG test product

        --- Parameters ---
        name:
        panel:playGround1
        """
        return self._make_cmd(locals())

    def gFontSizes(self ,
        value:str=''):
        """ Adjust font sizes in CSS style section.

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def gGroupId(self ,
        syntax:str=''):
        """ Set grouping criterion based on variables

        --- Parameters ---
        syntax:
        """
        return self._make_cmd(locals())

    def gGroupTitle(self ,
        syntax:str=''):
        """ Set titles, supporting variables

        --- Parameters ---
        syntax:
        """
        return self._make_cmd(locals())

    def gInclude(self ,
        include:str='["NONE"|"PNG"|"SVG"|"TXT"|"ALL"|"NEXT"|"SKIP"|"ON"|"OFF"|"UNKNOWN"]'):
        """ Select images to include in SVG panel

        --- Parameters ---
        include:["NONE"|"PNG"|"SVG"|"TXT"|"ALL"|"NEXT"|"SKIP"|"ON"|"OFF"|"UNKNOWN"]
        """
        return self._make_cmd(locals())

    def gLayout(self ,
        orientation:str='HORZ',
        direction:str='INCR'):
        """ Set main panel alignment

        --- Parameters ---
        orientation:HORZ
        direction:INCR
        """
        return self._make_cmd(locals())

    def gLinkImage(self ,
        value:str=''):
        """ SVG test product

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def gPanel(self ,
        layout:str='TECH'):
        """ SVG test product

        --- Parameters ---
        layout:TECH
        """
        return self._make_cmd(locals())

    def gPanelFoo(self ,
        layout:str=''):
        """ SVG test product

        --- Parameters ---
        layout:
        """
        return self._make_cmd(locals())

    def gPanelTest(self ,
        layout:str=''):
        """ SVG test product

        --- Parameters ---
        layout:
        """
        return self._make_cmd(locals())

    def gPolarGrid(self ,
        distance:list=[1,0,0],
        azimuth:list=[1,0,0]):
        """ Draw polar sectors and rings

        --- Parameters ---
        distance:1:0:0
        azimuth:1:0:0
        """
        return self._make_cmd(locals())

    def gPolarScope(self ):
        """ Draw circle, typically transparent, on the radar range

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def gPolarSector(self ,
        value:str=''):
        """ Select (and draw) sector using natural coordinates or indices

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def gStyle(self ,
        value:str=''):
        """ Add or modify CSS entry

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def gTitle(self ,
        title:str='AUTO'):
        """ Set main title

        --- Parameters ---
        title:AUTO
        """
        return self._make_cmd(locals())

    def gTitleHeights(self ,
        value:str=''):
        """ Set title box heights and adjust font sizes. See --gFontSizes

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def geoTiff(self ,
        tile:list=[256,256],
        compression:str=''):
        """ GeoTIFF tile size. Deprecating?, use --outputConf tif:tile=<width>:<height>,compression=LZW

        --- Parameters ---
        tile:256:256
        compression:
        """
        return self._make_cmd(locals())

    def getMyH5(self ,
        value:str=''):
        """ Dump H5 sources

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def help(self ,
        key:str=''):
        """ Display help.

        --- Parameters ---
        key:
        """
        return self._make_cmd(locals())

    def helpExample(self ,
        keyword:str=''):
        """ Dump example of use and exit.

        --- Parameters ---
        keyword:
        """
        return self._make_cmd(locals())

    def histogram(self ,
        count:int=0,
        range:list=[0,0],
        filename:str='',
        attribute:str='histogram',
        commentChar:str='#'):
        """ Histogram. Optionally --format using keys index,range,range.min,range.max,count,label

        --- Parameters ---
        count:0
        range:0:0
        filename:
        attribute:histogram
        commentChar:#
        """
        return self._make_cmd(locals())

    def iAdd(self ,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Adds values

        --- Parameters ---
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iAverage(self ,
        width:list=[1,0]):
        """ Window averaging with optional weighting support.

        --- Parameters ---
        width:1:0
        """
        return self._make_cmd(locals())

    def iBlender(self ,
        width:list=[5,0],
        spreader:str='avg',
        mix:str='max',
        loops:int=1,
        expansionCoeff:float=1):
        """ Smoothes image repeatedly, mixing original image with the result at each round.

        --- Parameters ---
        width:5:0
        spreader:avg
        mix:max
        loops:1
        expansionCoeff:1
        """
        return self._make_cmd(locals())

    def iCatenate(self ,
        scale:float=1,
        offset:float=0):
        """ Catenates images

        --- Parameters ---
        scale:1
        offset:0
        """
        return self._make_cmd(locals())

    def iCopy(self ,
        dstView:str='',
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Copies current view to: f=full image, i=image channels, a=alpha channel(s), 0=1st, 1=2nd,...

        --- Parameters ---
        dstView:
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iCrop(self ,
        width:int=0,
        height:int=0,
        i:int=0,
        j:int=0):
        """ Crop image.

        --- Parameters ---
        width:0
        height:0
        i:0
        j:0
        """
        return self._make_cmd(locals())

    def iDistance(self ,
        functor:str=''):
        """ Computes the distance of pixel vectors. Post-scaling with desired functor.

        --- Parameters ---
        functor:
        """
        return self._make_cmd(locals())

    def iDistanceTransform(self ,
        width:list=[10,10],
        height:list=[None,None],
        topology:str='16-CONNECTED'):
        """ Linearly decreasing intensities - applies decrements.

        --- Parameters ---
        width:10:10
        height:null:null
        topology:16-CONNECTED
        """
        return self._make_cmd(locals())

    def iDistanceTransformExp(self ,
        width:list=[10,10],
        height:list=[None,None],
        topology:str='16-CONNECTED'):
        """ Exponentially decreasing intensities. Set half-decay radii.

        --- Parameters ---
        width:10:10
        height:null:null
        topology:16-CONNECTED
        """
        return self._make_cmd(locals())

    def iDistanceTransformFill(self ,
        width:list=[12.3,12.3],
        height:list=[None,None],
        topology:str='16-CONNECTED',
        alphaThreshold:list=[0,0]):
        """ Spreads intensities linearly up to distance defined by alpha channel.

        --- Parameters ---
        width:12.3:12.3
        height:null:null
        topology:16-CONNECTED
        alphaThreshold:0:0
        """
        return self._make_cmd(locals())

    def iDistanceTransformFillExp(self ,
        width:list=[12,12],
        height:list=[None,None],
        topology:str='16-CONNECTED',
        alphaThreshold:list=[0,0]):
        """ Spreads intensities exponentially up to distance defined by alpha intensities.

        --- Parameters ---
        width:12:12
        height:null:null
        topology:16-CONNECTED
        alphaThreshold:0:0
        """
        return self._make_cmd(locals())

    def iDiv(self ,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        --- Parameters ---
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFloodFill(self ,
        location:list=[0,0],
        intensity:list=[1,65536],
        value:float=1):
        """ Fills an area starting at (i,j) having intensity in [min,max], with a value.

        --- Parameters ---
        location:0:0
        intensity:1:65536
        value:1
        """
        return self._make_cmd(locals())

    def iFlowAverage(self ,
        width:list=[1,1]):
        """ Window average that preserves the magnitude

        --- Parameters ---
        width:1:1
        """
        return self._make_cmd(locals())

    def iFuzzyBell(self ,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy bell function.

        --- Parameters ---
        location:0
        width:1
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFuzzyBell2(self ,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy Gaussian-like bell function.

        --- Parameters ---
        location:0
        width:1
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFuzzyStep(self ,
        position:list=[-1,1],
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy step function.

        --- Parameters ---
        position:-1:1
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFuzzyStepsoid(self ,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy step function

        --- Parameters ---
        location:0
        width:1
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFuzzyTriangle(self ,
        position:list=[-1,1],
        peakPos:float=0,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy triangle function.

        --- Parameters ---
        position:-1:1
        peakPos:0
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iFuzzyTwinPeaks(self ,
        location:float=0,
        width:float=1,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Fuzzy function of two peaks.

        --- Parameters ---
        location:0
        width:1
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iGamma(self ,
        gamma:float=1,
        LIMIT:bool=False):
        """ Gamma correction for brightness.

        --- Parameters ---
        gamma:1
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iGaussianAverage(self ,
        width:list=[1,0],
        radius:float=0.5):
        """ Gaussian blur implemented as quick Wx1 and 1xH filtering.

        --- Parameters ---
        width:1:0
        radius:0.5
        """
        return self._make_cmd(locals())

    def iGradient(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Computes horizontal and vertical derivatives: df/di and df/dj.

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iGradientHorz(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Horizontal intensity difference

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iGradientVert(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Vertical intensity difference

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iGray(self ,
        coeff:str='1.0',
        normalize:bool=True):
        """ Convert multi-channel image to single. Post-scaling with desired functor.

        --- Parameters ---
        coeff:1.0
        normalize:true
        """
        return self._make_cmd(locals())

    def iHighBoost(self ,
        width:int=3,
        height:int=3,
        coeff:float=0.5):
        """ Mixture of original and high-pass filtered image

        --- Parameters ---
        width:3
        height:3
        coeff:0.5
        """
        return self._make_cmd(locals())

    def iHighPass(self ,
        width:int=1,
        height:int=1,
        scale:float=1,
        offset:float=0,
        LIMIT:bool=True):
        """ High-pass filter for recognizing details.

        --- Parameters ---
        width:1
        height:1
        scale:1
        offset:0
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iImpulseAvg(self ,
        decay:list=[0.75,0.75,0.75,0.75],
        extendHorz:int=0,
        extendVert:int=0,
        weightThreshold:float=0.05):
        """ Infinite-impulse response type spreading

        --- Parameters ---
        decay:0.75:0.75:0.75:0.75
        extendHorz:0
        extendVert:0
        weightThreshold:0.05
        """
        return self._make_cmd(locals())

    def iLaplace(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Second intensity derivatives, (df/di)^2 and (df/dj)^2

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iLaplaceHorz(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Second horizontal differential

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iLaplaceVert(self ,
        radius:int=1,
        LIMIT:bool=True):
        """ Second vertical differential

        --- Parameters ---
        radius:1
        LIMIT:true
        """
        return self._make_cmd(locals())

    def iMagnitude(self ,
        functor:str=''):
        """ Computes the magnitude of a pixel vector. Post-scaling with desired functor.

        --- Parameters ---
        functor:
        """
        return self._make_cmd(locals())

    def iMarginStat(self ,
        mode:str='horz',
        stat:str='asmdvNX',
        medianPos:float=0.5):
        """ Computes statistics on <horz> or <vert> lines: iAverage,sum,median,stdDev,variance,miN,maX

        --- Parameters ---
        mode:horz
        stat:asmdvNX
        medianPos:0.5
        """
        return self._make_cmd(locals())

    def iMax(self ):
        """ Maximum of two values.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def iMedian(self ,
        width:list=[1,1],
        percentage:float=0.5,
        bins:int=256):
        """ A pipeline implementation of window median.

        --- Parameters ---
        width:1:1
        percentage:0.5
        bins:256
        """
        return self._make_cmd(locals())

    def iMin(self ):
        """ Minimum of two values.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def iMix(self ,
        coeff:float=0.5,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        --- Parameters ---
        coeff:0.5
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iMul(self ,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        --- Parameters ---
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iMultiThreshold(self ,
        range:list=[0,1],
        min:float=0,
        max:float=1):
        """ Change values outside given range to target.min and target.max

        --- Parameters ---
        range:0:1
        min:0
        max:1
        """
        return self._make_cmd(locals())

    def iNegate(self ):
        """ Invert values (unsigned char or unsigned short int)

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def iPhysical(self ,
        physical:bool=False):
        """ Flag. Handle intensities as physical quantities instead of storage typed values.

        --- Parameters ---
        physical:false
        """
        return self._make_cmd(locals())

    def iProduct(self ,
        functor:str=''):
        """ Computes the dot product of pixel vectors. Post-scaling with desired functor.

        --- Parameters ---
        functor:
        """
        return self._make_cmd(locals())

    def iQualityMixer(self ,
        coeff:float=0.5):
        """ Weighted blending of two images.

        --- Parameters ---
        coeff:0.5
        """
        return self._make_cmd(locals())

    def iQualityOverride(self ,
        advantage:float=1,
        decay:float=1):
        """ Compares two images, preserving pixels having higher alpha value. Src alpha is pre-multiplied with advantage.

        --- Parameters ---
        advantage:1
        decay:1
        """
        return self._make_cmd(locals())

    def iQualityThreshold(self ,
        threshold:float=0,
        replace:float=None):
        """ Threshold data with quality

        --- Parameters ---
        threshold:0
        replace:null
        """
        return self._make_cmd(locals())

    def iQuantizator(self ,
        bits:int=4,
        LIMIT:bool=False):
        """ Quantize to n bits. (For integer images)

        --- Parameters ---
        bits:4
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iRemap(self ,
        fromValue:float=0,
        toValue:float=0,
        LIMIT:bool=False):
        """ Rescales intensities linerarly

        --- Parameters ---
        fromValue:0
        toValue:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iRescale(self ,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Rescales values linerarly: y = scale*x + bias

        --- Parameters ---
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iResize(self ,
        width:int=0,
        height:int=0,
        interpolation:str='n',
        scale:float=1):
        """ Resize geometry and scale intensities

        --- Parameters ---
        width:0
        height:0
        interpolation:n
        scale:1
        """
        return self._make_cmd(locals())

    def iRunLengthHorz(self ,
        threshold:float=0):
        """ Computes lengths of segments of intensity above threshold.

        --- Parameters ---
        threshold:0
        """
        return self._make_cmd(locals())

    def iRunLengthVert(self ,
        threshold:float=0):
        """ Computes lengths of segments of intensity above threshold.

        --- Parameters ---
        threshold:0
        """
        return self._make_cmd(locals())

    def iSegmentArea(self ,
        intensity:list=[1,1.79769e+308],
        functor:str=''):
        """ Computes segment sizes.

        --- Parameters ---
        intensity:1:1.79769e+308
        functor:
        """
        return self._make_cmd(locals())

    def iSegmentStats(self ,
        statistics:str='d',
        intensity:list=[1,255],
        functor:str=''):
        """ Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation

        --- Parameters ---
        statistics:d
        intensity:1:255
        functor:
        """
        return self._make_cmd(locals())

    def iSub(self ,
        scale:float=1,
        bias:float=0,
        LIMIT:bool=False):
        """ Subtracts values

        --- Parameters ---
        scale:1
        bias:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iSuperProber(self ,
        threshold:int=128,
        filename:str='',
        debug:int=0):
        """ Hierarchical segment prober

        --- Parameters ---
        threshold:128
        filename:
        debug:0
        """
        return self._make_cmd(locals())

    def iThreshold(self ,
        threshold:float=0.5,
        replace:float=0,
        LIMIT:bool=False):
        """ Resets values lower than a threshold

        --- Parameters ---
        threshold:0.5
        replace:0
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iThresholdBinary(self ,
        threshold:list=[0.5,0.5],
        replace:list=[0,1],
        LIMIT:bool=False):
        """ Resets values lower and higher than a threshold

        --- Parameters ---
        threshold:0.5:0.5
        replace:0:1
        LIMIT:false
        """
        return self._make_cmd(locals())

    def iTranspose(self ):
        """ Flips image matrix around its corner.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def iWindowHistogram(self ,
        width:list=[1,1],
        valueFunc:str='a',
        percentage:float=0.5,
        bins:int=256):
        """ A pipeline implementation of window histogram; valueFunc=[asmdvNX] (avg,sum,median,stddev,variance,miN,maX)

        --- Parameters ---
        width:1:1
        valueFunc:a
        percentage:0.5
        bins:256
        """
        return self._make_cmd(locals())

    def image(self ):
        """ Copies data to a separate image object. Encoding can be changed with --target .

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def imageAlpha(self ):
        """ Adds a transparency channel. Implies additional image, creates one if needed. See --encoding

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def imageBox(self ,
        i:list=[0,0],
        j:list=[0,0],
        value:str='0'):
        """ Set intensity at (i:i2,j:j2) to (f1,f2,f3,...).

        --- Parameters ---
        i:0:0
        j:0:0
        value:0
        """
        return self._make_cmd(locals())

    def imageFlatten(self ,
        bgcolor:str='0'):
        """ Removes a alpha (transparency) channel. Adds a new background of given color.

        --- Parameters ---
        bgcolor:0
        """
        return self._make_cmd(locals())

    def imagePhysical(self ,
        physical:bool=False):
        """ Flag. Handle intensities as physical quantities instead of storage typed values.

        --- Parameters ---
        physical:false
        """
        return self._make_cmd(locals())

    def imageQuality(self ,
        groups:str='dataset:data'):
        """ Applied parent groups for quality: dataset:data

        --- Parameters ---
        groups:dataset:data
        """
        return self._make_cmd(locals())

    def imageTransp(self ,
        ftor:str='',
        undetect:float=0,
        nodata:float=1):
        """ Adds a transparency channel. Uses copied image, creating one if needed.

        --- Parameters ---
        ftor:
        undetect:0
        nodata:1
        """
        return self._make_cmd(locals())

    def inputFile(self ,
        filename:str=''):
        """ Read HDF5, text or image file

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def inputFilter(self ,
        ATTRIBUTES:str='3'):
        """ Partial file read. You probably search for --inputSelect

        --- Parameters ---
        ATTRIBUTES:3
        """
        return self._make_cmd(locals())

    def inputPalette(self ,
        filename:str=''):
        """ Load palette.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def inputPrefix(self ,
        value:str=''):
        """ Path prefix for input files.

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def inputSelect(self ,
        value:str=''):
        """ Selector for input data.

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def jSON(self ,
        property:str=''):
        """ Export a command to JSON.

        --- Parameters ---
        property:
        """
        return self._make_cmd(locals())

    def keep(self ,
        selector:str='path,quantity,elangle,count,order,prf,timespan'):
        """ Keeps selected part of data structure, deletes rest.

        --- Parameters ---
        selector:path,quantity,elangle,count,order,prf,timespan
        """
        return self._make_cmd(locals())

    def legendOut(self ,
        filename:str=''):
        """ Save palette as TXT, JSON, SVG or SLD.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def log(self ,
        file:str='',
        level:str='',
        timing:bool=False,
        vt100:bool=True):
        """ Redirect log to file. Status variables like ${ID}, ${PID} and ${CTX} supported.

        --- Parameters ---
        file:
        level:
        timing:false
        vt100:true
        """
        return self._make_cmd(locals())

    def move(self ,
        src:str='',
        dst:str=''):
        """ Rename or move data groups and attributes.

        --- Parameters ---
        src:
        dst:
        """
        return self._make_cmd(locals())

    def odim(self ,
        version:str='2.4'):
        """ Set ODIM version (experimental)

        --- Parameters ---
        version:2.4
        """
        return self._make_cmd(locals())

    def outputConf(self ,
        value:str='<format>:<key>=value>,conf...'):
        """ Format (h5|tif|png|tre|dot) specific configurations

        --- Parameters ---
        value:<format>:<key>=value>,conf...
        """
        return self._make_cmd(locals())

    def outputFile(self ,
        filename:str=''):
        """ Output data to HDF5, text, image or GraphViz file. See also: --image, --outputRawImages.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def outputPalette(self ,
        filename:str=''):
        """ Save palette as TXT, JSON, SVG or SLD.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def outputPrefix(self ,
        value:str=''):
        """ Path prefix for output files.

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def outputRawImages(self ,
        filename:str=''):
        """ Output datasets to png files named filename[NN].ext.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def outputTree(self ,
        filename:str=''):
        """ Output data as simple tree structure.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def pAdd(self ):
        """ Add polar data to accumulation array.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def pAddWeighted(self ,
        weight:float=1):
        """ Adds the current product to the composite applying weight.

        --- Parameters ---
        weight:1
        """
        return self._make_cmd(locals())

    def pBeamAltitude(self ,
        aboveSeaLevel:bool=True):
        """ Computes the altitude at each bin

        --- Parameters ---
        aboveSeaLevel:true
        """
        return self._make_cmd(locals())

    def pBiomet(self ,
        reflMax:float=-10,
        maxAltitude:int=500,
        reflDev:float=5,
        devAltitude:int=1000):
        """ Detects birds and insects.

        --- Parameters ---
        reflMax:-10
        maxAltitude:500
        reflDev:5
        devAltitude:1000
        """
        return self._make_cmd(locals())

    def pBird(self ,
        dbzPeak:list=[-20,0],
        vradDevMin:float=1,
        rhoHVmax:list=[0.99,0.891],
        zdrAbsMin:float=1,
        window:list=[2500,5],
        gamma:float=1):
        """ Estimates bird probability from DBZH, VRAD, RhoHV and ZDR.

        --- Parameters ---
        dbzPeak:-20:0
        vradDevMin:1
        rhoHVmax:0.99:0.891
        zdrAbsMin:1
        window:2500:5
        gamma:1
        """
        return self._make_cmd(locals())

    def pCappi(self ,
        altitude:float=1000,
        aboveSeaLevel:bool=True,
        beamWidth:float=1,
        weightMin:float=-0.1,
        accumulationMethod:str='WAVG:1:8:-40',
        height:bool=False):
        """ Constant-altitude planar position indicator

        --- Parameters ---
        altitude:1000
        aboveSeaLevel:true
        beamWidth:1
        weightMin:-0.1
        accumulationMethod:WAVG:1:8:-40
        height:false
        """
        return self._make_cmd(locals())

    def pClutter(self ,
        decay:float=0.5,
        gamma:float=1,
        quantity:str='CLUTTER',
        file:str='cluttermaps/cluttermap-${NOD}-${quantity}.h5'):
        """ Reads a ground clutter map and scales it to sweeps.

        --- Parameters ---
        decay:0.5
        gamma:1
        quantity:CLUTTER
        file:cluttermaps/cluttermap-${NOD}-${quantity}.h5
        """
        return self._make_cmd(locals())

    def pConv(self ,
        maxEchoThreshold:float=25,
        cellDiameter:float=3,
        echoTopThreshold:float=2,
        echoTopDBZ:float=20,
        smoothAzm:float=0,
        smoothRad:float=0):
        """ Computes the probability of convection based on fuzzy cell intensity, area and height.

        --- Parameters ---
        maxEchoThreshold:25
        cellDiameter:3
        echoTopThreshold:2
        echoTopDBZ:20
        smoothAzm:0
        smoothRad:0
        """
        return self._make_cmd(locals())

    def pDopplerAvg(self ,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False,
        relativeScale:bool=False):
        """ Smoothens Doppler field, providing quality

        --- Parameters ---
        width:1500
        height:3
        threshold:0.5
        compensate:false
        relativeScale:false
        """
        return self._make_cmd(locals())

    def pDopplerAvgExp(self ,
        decay:list=[0.75,0.75,0.75,0.75],
        horzExtension:int=0,
        vertExtension:int=0):
        """ Doppler field smoother with exponential decay weighting

        --- Parameters ---
        decay:0.75:0.75:0.75:0.75
        horzExtension:0
        vertExtension:0
        """
        return self._make_cmd(locals())

    def pDopplerCrawler(self ,
        nyquist:float=100,
        threshold:float=0.9):
        """ Creates virtual 

        --- Parameters ---
        nyquist:100
        threshold:0.9
        """
        return self._make_cmd(locals())

    def pDopplerDev(self ,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False,
        relativeScale:bool=True):
        """ Computes std.dev.[m/s] of Doppler field

        --- Parameters ---
        width:1500
        height:3
        threshold:0.5
        compensate:false
        relativeScale:true
        """
        return self._make_cmd(locals())

    def pDopplerDiff(self ,
        dMax:float=0):
        """ Azimuthal difference of VRAD

        --- Parameters ---
        dMax:0
        """
        return self._make_cmd(locals())

    def pDopplerDiffPlotter(self ,
        azm:list=[0,0],
        range:list=[0,0],
        ray:list=[0,0],
        bin:list=[0,0]):
        """ Plots differences in VRAD data as fucntion of azimuth

        --- Parameters ---
        azm:0:0
        range:0:0
        ray:0:0
        bin:0:0
        """
        return self._make_cmd(locals())

    def pDopplerEccentricity(self ,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        compensate:bool=False):
        """ Magnitude of mean unit circle mapped Doppler speeds

        --- Parameters ---
        width:1500
        height:3
        threshold:0.5
        compensate:false
        """
        return self._make_cmd(locals())

    def pDopplerInversion(self ,
        width:int=500,
        height:float=3,
        altitudeWeight:str=''):
        """ Derives 2D wind (u,v) from aliased Doppler data.

        --- Parameters ---
        width:500
        height:3
        altitudeWeight:
        """
        return self._make_cmd(locals())

    def pDopplerReproject(self ,
        nyquist:float=100,
        match:int=0,
        quantity:str='VRAD'):
        """ Creates virtual 

        --- Parameters ---
        nyquist:100
        match:0
        quantity:VRAD
        """
        return self._make_cmd(locals())

    def pDopplerSampler(self ,
        azm:list=[0,0],
        range:list=[0,0],
        ray:list=[0,0],
        bin:list=[0,0]):
        """ Projects Doppler speeds to unit circle. Window corners as (azm,range) or (ray,bin)

        --- Parameters ---
        azm:0:0
        range:0:0
        ray:0:0
        bin:0:0
        """
        return self._make_cmd(locals())

    def pDrawing(self ,
        shape:str='direction',
        p1:float=0,
        p2:float=0,
        p3:float=0,
        p4:float=0,
        marker:int=255):
        """ Visualise a direction[azm,r,r2], range[r,r2], sector[azm,azm2,r,r2] or arc[azm,azm2,r].

        --- Parameters ---
        shape:direction
        p1:0
        p2:0
        p3:0
        p4:0
        marker:255
        """
        return self._make_cmd(locals())

    def pEchoTop(self ,
        threshold:float=20,
        reference:list=[-50,15000],
        undetectValue:float=-32,
        weights:list=[1,0.8,0.6,0.4,0.2],
        weightDecay:float=20,
        avgWindow:list=[0,0],
        EXTENDED:bool=True):
        """ Estimates maximum altitude of given reflectivity

        --- Parameters ---
        threshold:20
        reference:-50:15000
        undetectValue:-32
        weights:1:0.8:0.6:0.4:0.2
        weightDecay:20
        avgWindow:0:0
        EXTENDED:true
        """
        return self._make_cmd(locals())

    def pEchoTopOpOld(self ,
        minDBZ:float=20,
        dbzReference:float=-50,
        altitudeReference:float=15000,
        aboveSeaLevel:bool=True):
        """ Computes maximum (or minimum) altitude of echo.

        --- Parameters ---
        minDBZ:20
        dbzReference:-50
        altitudeReference:15000
        aboveSeaLevel:true
        """
        return self._make_cmd(locals())

    def pExtract(self ,
        channels:str='dw'):
        """ Extract polar-coordinate data that has been accumulated.

        --- Parameters ---
        channels:dw
        """
        return self._make_cmd(locals())

    def pFunctor(self ,
        ftor:str=''):
        """ Maps values using a function

        --- Parameters ---
        ftor:
        """
        return self._make_cmd(locals())

    def pMaxEcho(self ,
        altitude:list=[1000,9000],
        accumulationMethod:str='MAXIMUM'):
        """ Computes maximum dBZ inside altitude range

        --- Parameters ---
        altitude:1000:9000
        accumulationMethod:MAXIMUM
        """
        return self._make_cmd(locals())

    def pOutputQuantity(self ,
        productCmd:str=''):
        """ Return default outout quantity

        --- Parameters ---
        productCmd:
        """
        return self._make_cmd(locals())

    def pPlot(self ,
        lon:float=0,
        lat:float=0,
        value:float=0,
        weight:float=1):
        """ Add a single data point.

        --- Parameters ---
        lon:0
        lat:0
        value:0
        weight:1
        """
        return self._make_cmd(locals())

    def pPlotFile(self ,
        file:str=''):
        """ Plot file containing rows '<lat> <lon> <value> [weight] (skipped...)'.

        --- Parameters ---
        file:
        """
        return self._make_cmd(locals())

    def pPolarSlidingAvg(self ,
        width:float=1500,
        height:float=3,
        threshold:float=0.5,
        invertPolar:bool=False):
        """ Smoothen polar data

        --- Parameters ---
        width:1500
        height:3
        threshold:0.5
        invertPolar:false
        """
        return self._make_cmd(locals())

    def pPseudoRhi(self ,
        az_angle:float=0,
        xsize:int=500,
        ysize:int=250,
        range:list=[1,250],
        height:list=[0,10000],
        beamWidth:float=0.25,
        beamPowerThreshold:float=0.01):
        """ Computes vertical intersection in a volume in the beam direction.

        --- Parameters ---
        az_angle:0
        xsize:500
        ysize:250
        range:1:250
        height:0:10000
        beamWidth:0.25
        beamPowerThreshold:0.01
        """
        return self._make_cmd(locals())

    def pRainRate(self ):
        """ Estimates precipitation rate [mm/h] from reflectance [dBZ].

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def pRainRateDP(self ,
        dbz:list=[30,50],
        zdr:list=[0.15,0.25],
        rhohv:float=0.85,
        kdp:list=[0.25,0.35]):
        """ Precip. rate [mm/h] from dual-pol using fuzzy thresholds. Alg. by Brandon Hickman

        --- Parameters ---
        dbz:30:50
        zdr:0.15:0.25
        rhohv:0.85
        kdp:0.25:0.35
        """
        return self._make_cmd(locals())

    def pSite(self ,
        location:list=[25.2,60.1]):
        """ Set radar size location of the accumulated data. Also size etc., if --encoding set.

        --- Parameters ---
        location:25.2:60.1
        """
        return self._make_cmd(locals())

    def pSunShine(self ,
        timestamp:str='200527071845',
        quantity:str='SUNSHINE'):
        """ Computes the sun position

        --- Parameters ---
        timestamp:200527071845
        quantity:SUNSHINE
        """
        return self._make_cmd(locals())

    def pSweep(self ,
        quantity:str='DBZH',
        index:int=0):
        """ Return a single sweep

        --- Parameters ---
        quantity:DBZH
        index:0
        """
        return self._make_cmd(locals())

    def pVerticalProfile(self ,
        range:list=[10,100],
        height:list=[0,10000],
        levels:int=100,
        azm:list=[0,359.99],
        azSlots:int=1):
        """ Computes vertical dBZ distribution in within range [minRange,maxRange] km.

        --- Parameters ---
        range:10:100
        height:0:10000
        levels:100
        azm:0:359.99
        azSlots:1
        """
        return self._make_cmd(locals())

    def palette(self ,
        filename:str=''):
        """ Load and apply palette.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def paletteConf(self ,
        key:str='list',
        code:str='',
        lenient:bool=True):
        """ Check status of palette(s).

        --- Parameters ---
        key:list
        code:
        lenient:true
        """
        return self._make_cmd(locals())

    def paletteDefault(self ):
        """ Apply default palette matching the quantity of current data.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def paletteIn(self ,
        filename:str=''):
        """ Load palette.

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def paletteOut(self ,
        params:str=''):
        """ Deprecating command

        --- Parameters ---
        params:
        """
        return self._make_cmd(locals())

    def paletteRefine(self ,
        count:int=0):
        """ Refine colors

        --- Parameters ---
        count:0
        """
        return self._make_cmd(locals())

    def pause(self ,
        seconds:str='random'):
        """ Pause for n or random seconds

        --- Parameters ---
        seconds:random
        """
        return self._make_cmd(locals())

    def plot(self ,
        i,j:str='0,0'):
        """ Plot

        --- Parameters ---
        i,j:0,0
        """
        return self._make_cmd(locals())

    def precipKDP(self ,
        a:float=6.95326e-310,
        b:float=0):
        """ Precip rate from KDP

        --- Parameters ---
        a:6.95326e-310
        b:0
        """
        return self._make_cmd(locals())

    def precipKDPZDR(self ,
        a:float=6.95326e-310,
        b:float=0,
        c:float=6.10797e-310):
        """ Precipitation rate from KDP and ZDR

        --- Parameters ---
        a:6.95326e-310
        b:0
        c:6.10797e-310
        """
        return self._make_cmd(locals())

    def precipZZDR(self ,
        a:float=0.0122,
        b:float=0.82,
        c:float=-2.28):
        """ Precipitation rate from Z and ZDR

        --- Parameters ---
        a:0.0122
        b:0.82
        c:-2.28
        """
        return self._make_cmd(locals())

    def precipZrain(self ,
        a:float=200,
        b:float=1.6):
        """ Precipitation rate from Z (reflectivity)

        --- Parameters ---
        a:200
        b:1.6
        """
        return self._make_cmd(locals())

    def precipZsnow(self ,
        a:float=223,
        b:float=1.53):
        """ Precipitation rate from Z (reflectivity)

        --- Parameters ---
        a:223
        b:1.53
        """
        return self._make_cmd(locals())

    def python(self ,
        file:str='dump.py',
        count:int=100):
        """ Export command interfaces to Python. Compatible with rack.prog module.

        --- Parameters ---
        file:dump.py
        count:100
        """
        return self._make_cmd(locals())

    def quantityConf(self ,
        quantity:str='',
        encoding:str='',
        zero:str=''):
        """ 1) list quantities, 2) set default type for a quantity, 3) set default scaling for (quantity,type) pair

        --- Parameters ---
        quantity:
        encoding:
        zero:
        """
        return self._make_cmd(locals())

    def restart(self ):
        """ Debug

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def sample(self ,
        iStep:int=10,
        jStep:int=0,
        i:list=[-1,1],
        j:list=[-1,1],
        commentChar:str='#',
        skipVoid:bool=False,
        handleVoid:str='null'):
        """ Extract samples. See --format.

        --- Parameters ---
        iStep:10
        jStep:0
        i:-1:1
        j:-1:1
        commentChar:#
        skipVoid:false
        handleVoid:null
        """
        return self._make_cmd(locals())

    def script(self ,
        script:str=''):
        """ Define script.

        --- Parameters ---
        script:
        """
        return self._make_cmd(locals())

    def select(self ,
        selector:str='path,quantity,elangle,count,order,prf,timespan'):
        """ Data selector for the next computation

        --- Parameters ---
        selector:path,quantity,elangle,count,order,prf,timespan
        """
        return self._make_cmd(locals())

    def selectObject(self ,
        flags:str='INPUT|POLAR|CARTESIAN'):
        """ Select input object for the next operation

        --- Parameters ---
        flags:INPUT|POLAR|CARTESIAN
        """
        return self._make_cmd(locals())

    def selectQuantity(self ,
        quantities:str=''):
        """ Like --select quantity=... 

        --- Parameters ---
        quantities:
        """
        return self._make_cmd(locals())

    def set(self ,
        assignment:str=''):
        """ Set general-purpose variables

        --- Parameters ---
        assignment:
        """
        return self._make_cmd(locals())

    def setODIM(self ,
        assignment:str=''):
        """ Set data properties (ODIM). Works also directly: --/<path>:<key>[=<value>]. See --completeODIM

        --- Parameters ---
        assignment:
        """
        return self._make_cmd(locals())

    def status(self ):
        """ Dump information on current images.

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def stopOnError(self ,
        flags:str=''):
        """ Stop on given error condition(s).

        --- Parameters ---
        flags:
        """
        return self._make_cmd(locals())

    def store(self ,
        level:str='0'):
        """ Request additional (debugging) outputs

        --- Parameters ---
        level:0
        """
        return self._make_cmd(locals())

    def target(self ,
        encoding:str=''):
        """ Sets encodings parameters for polar and Cartesian products, including composites.

        --- Parameters ---
        encoding:
        """
        return self._make_cmd(locals())

    def testAreaSelector(self ,
        distance:list=[0,250],
        azimuth:list=[0,360]):
        """ Polar area (sector, annulus) selector

        --- Parameters ---
        distance:0:250
        azimuth:0:360
        """
        return self._make_cmd(locals())

    def testSelect(self ):
        """ Test...

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def testTree(self ,
        value:str=''):
        """ Dump XML track

        --- Parameters ---
        value:
        """
        return self._make_cmd(locals())

    def undetectWeight(self ,
        weight:float=0.75):
        """ Set the relative weight of data values assigned 'undetect'.

        --- Parameters ---
        weight:0.75
        """
        return self._make_cmd(locals())

    def updateVariables(self ):
        """ Force calling DataTools::updateInternalAttributes(ctx.getHi5(Hdf5Context::CURRENT))

        --- Parameters ---
        """
        return self._make_cmd(locals())

    def validate(self ,
        filename:str=''):
        """ Read CVS file 

        --- Parameters ---
        filename:
        """
        return self._make_cmd(locals())

    def verbose(self ,
        level:str='NOTE',
        imageLevel:str='WARNING'):
        """ Set verbosity level

        --- Parameters ---
        level:NOTE
        imageLevel:WARNING
        """
        return self._make_cmd(locals())

    def version(self ):
        """ Displays software version and quits.

        --- Parameters ---
        """
        return self._make_cmd(locals())

