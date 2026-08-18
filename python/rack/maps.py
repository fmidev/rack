import argparse
import pathlib
import os
import re
import sys
import urllib.request

import rack.config
import rack.log


logger = rack.log.logger.getChild(pathlib.Path(__file__).stem)

# maps.wms_BBOX=-15000,6508000,625000,6892000_MAPCONF=geoserver-natural_PROJ=3067_SIZE=1280,768.png
# http://map.fmi.fi/geoserver/wms?service=WMS&version=1.3.0&request=GetMap&format=image/png&layers=world-map-combo-en&srs=EPSG:3067&bbox=-15000,6508000,625000,6892000&width=1280&height=768    
# https://ows.terrestris.de/osm/service?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=OSM-WMS&STYLES=&CRS=EPSG:3857&BBOX=2610000,7500000,2620000,7510000&WIDTH=800&HEIGHT=600&FORMAT=image/png
server_conf = {
        "fmi": {
            "endpoint": "http://map.fmi.fi/geoserver/wms",
            "version": "1.3.0",
            "layers": ["world-map-combo-en"], # (standard map)
            "epsg": [3067, 4326] # (EPSG:3067 is default, but also support 4326)
        },
        "terrestris": {
            "endpoint": "https://ows.terrestris.de/osm/service",
            "version": "1.3.0",
            "layers": ["OSM-WMS"], # default, but shoud be more?
        },
        "mundialis": {
            "endpoint": "https://ows.mundialis.de/osm/service",
            "layers": [
                "OSM-WMS", # (standard map)
                "TOPO-WMS", # (terrain colored)
                "TOPO-OSM-WMS" # (overlay combo)
            ]
        },
        "nasa-neo": {
            "endpoint": "https://neo.sci.gsfc.nasa.gov/wms/wms", 
            "version": "1.3.0",
            "layers": ["MODIS_Terra_CorrectedReflectance_TrueColor"]    
        }
        
    }

GEO_CONF_PATH_SYNTAX  = "mapconf/geo-{key}" # (default path syntax for geoconf files)
SERVER_CONF_PATH_SYNTAX  = "mapconf/server-{key}" # (default path syntax for geoconf files)
MAP_CACHE_PATH_SYNTAX = "./mapcache/{mapServer}/{CRS}/BBOX={BBOX}_LAYERS={layers}_SIZE={WIDTH},{HEIGHT}_STYLES={styles}.png" # (default path syntax for cached map files)

def build_parser() -> argparse.ArgumentParser:

    parser = argparse.ArgumentParser(
        description="Example app with JSON config support")

    add_arguments(parser)

    return parser


def print_argument_help(parser: argparse.ArgumentParser, name: str) -> bool:
    """Print help (incl. default value) for a single named argument, e.g. 'cache' or '--cache'.

    Returns True if a matching argument was found and printed.
    """

    key = name.lstrip('-').upper()
    for action in parser._actions:
        candidates = [opt.lstrip('-').upper() for opt in action.option_strings]
        if action.dest:
            candidates.append(action.dest.upper())
        if key in candidates:
            # Show the default here only, not in the general -h listing.
            formatter = argparse.ArgumentDefaultsHelpFormatter(prog=parser.prog)
            formatter.add_argument(action)
            print(formatter.format_help().strip())
            return True

    return False


def add_basic_arguments(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    
    parser.add_argument(
        "--BBOX",
        #default='6,51.3,49,70.2',
        metavar="<lonLL,latLL,lonUR,latUR>",
        help="Bounding box [cBBox]")  # FMI Scandinavia

    parser.add_argument(
        "--PROJ",
        #"--EPSG",
        default=3067,
        metavar="<epsg>",
        help="EPSG code of the target projection (CRS), e.g. 3067 or 4326")

    parser.add_argument(
        "--SIZE",
        #default=None,
        default="800,800",
        metavar="<width>[,<height>]",
        help="Requested map image size in pixels")

    parser.add_argument(
        "--GEOCONF",
        metavar="<KEY>|<filepath>-<KEY>.json>",
        help="Read BBOX, PROJ, SIZE from file, default: map/geo-<KEY>.json or .cnf")



def add_arguments(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    
    add_basic_arguments(parser)

    parser.add_argument(
        "--mapLayers",
        type=str,
        default="",
        help="Layers to request, comma-separated, e.g. 'OSM-WMS,TOPO-WMS'") 

    parser.add_argument(
        "--mapFile",
        type=str,
        default="",
        metavar="<filename>",
        help="Filename of the map file to save, e.g. 'my-map.png'") 

    parser.add_argument(
        "--mapStyles",
        type=str,
        default="",
        #metavar="",
        help="WMS style name(s) to request, comma-separated")

    parser.add_argument(
        "--mapServer",
        type=str,
        default=None,
        metavar=list(server_conf.keys()),
        help="Name of a preconfigured WMS server to use")

    # TODO LINK GEOCONF
    parser.add_argument(
        "--mapCache",
        default=MAP_CACHE_PATH_SYNTAX,
        metavar="<syntax>",
        help="Path pattern for the cached map file")

    parser.add_argument(
        "--mapForce",
        action='store_true',
        help="bypass cache and force retrieval of map from server") 

    parser.add_argument(
        "--mapLink",
        type=str,
        default=None,
        metavar="path/filename.png",
        help="Link cached file at this path")


    return parser
    



# EPSG codes registered with (lat, lon) axis order rather than (lon, lat)/(x, y).
# WMS 1.3.0 (CRS=) requires BBOX in the CRS's registered axis order, so these need
# swapping; WMS < 1.3.0 (SRS=) always used (lon,lat)-like order, regardless of CRS.
# Extend this set if other geographic (lat,lon) CRSs are used, e.g. 4258, 4269.
LATLON_AXIS_EPSG = {4326}

def bbox_needs_swap(version, epsg) -> bool:
    """Whether BBOX=lonLL,latLL,lonUR,latUR needs swapping to latLL,lonLL,latUR,lonUR
    for this WMS `version` and `epsg`, per the WMS 1.3.0 axis-order rule.
    """

    try:
        epsg = int(epsg)
    except (TypeError, ValueError):
        return False

    if epsg not in LATLON_AXIS_EPSG:
        return False

    try:
        major, minor = str(version).split('.')[:2]
        version_tuple = (int(major), int(minor))
    except (ValueError, AttributeError):
        return False

    return version_tuple >= (1, 3)


def construct_http_get(defaults: dict, **args) -> dict:

    get_params = {} #defaults.copy()
    # keys = args2.keys()

    for (k,v) in defaults.items():
        if k in ["endpoint"]:
            continue
        if k in args and args[k] != None:
            v = args[k]
        get_params[k] = v

    if 'PROJ' in args:
        v = args['PROJ']
        if not v: # ???
            v = 3067
        get_params["CRS"] = f"EPSG:{v}"

    if 'SIZE' in args:
        (w,h) = rack.typical(args['SIZE'], tuple, separator=',')
        get_params["WIDTH"]  = w
        get_params["HEIGHT"] = h

    if 'BBOX' in args and args['BBOX']:
        # CLI/API convention: always lonLL,latLL,lonUR,latUR.
        bbox = rack.typical(args['BBOX'], tuple, separator=',')
        if bbox_needs_swap(defaults.get('VERSION'), args.get('PROJ')):
            lonLL, latLL, lonUR, latUR = bbox
            bbox = (latLL, lonLL, latUR, lonUR)
            logger.info(f"WMS {defaults.get('VERSION')} + EPSG:{args.get('PROJ')}: swapping BBOX axis order -> {bbox}")
        get_params["BBOX"] = rack.typical(bbox, str, separator=',')
        #args['BBOX'].strip() # Remove possible quotes, e.g. from JSON string.

    # logger.info(get_params)
    # print(get_params)
    return get_params

def get_cache_path(cache_syntax:str, **kw_args):
    try:
        return cache_syntax.format(**kw_args)
    except KeyError as e:
        logger.warning(f"Args: {kw_args}")
        logger.error(f"Missing key in cache syntax= {cache_syntax}: {e}")
        raise

import urllib.request
import urllib.error
import xml.etree.ElementTree as ET

def extract_wms_error(text):
    """Try to extract a readable WMS ServiceException / ExceptionText."""
    try:
        root = ET.fromstring(text)

        # Common WMS exception formats
        for elem in root.iter():
            tag = elem.tag.lower()
            if tag.endswith("serviceexception") or tag.endswith("exceptiontext"):
                if elem.text and elem.text.strip():
                    return elem.text.strip()

        # Fallback: return whole XML/text
        return text.strip()

    except ET.ParseError:
        return text.strip()

def suggest_server_conf(server: str): # -> dict:
    """No server conf found for `server`, neither built in nor as a file.

    Create a template file at mapconf/server-<server>.json by copying the
    first entry of rack.maps.server_conf, so there's a starting point to edit.
    """

    filepath = pathlib.Path(f'mapconf/server-{server}.json')
    template_name, template = next(iter(server_conf.items()))
    conf = dict(template)

    logger.warning(
        f"No server conf found for '{server}' (in code or file). "
        f"Creating a template at {filepath}, copied from '{template_name}' - please edit it."
    )

    os.makedirs(filepath.parent, exist_ok=True)
    rack.config.write(str(filepath), conf)
    #return conf


def get_server_conf(server:str="", layers:str="", epsg:int=None) -> dict:
    """ Todo

        Tries to find a server conf with given name.
        If found, checks if desired layers and projection (epsg) exists.
    """

    logger.info(f"First, looking for built-in config for server='{server}', layer='{layers}'")
    if layers:
        layers = rack.typical(layers, [str])
    else:
        layers = []


    conf = dict(server_conf)

    if server:
        if server in conf:
            v = conf[server]
        else:
            # Not a built-in server: look for a file-based conf next.
            # (Built directly, not via path_syntax: resolve_path only auto-applies
            # path_syntax for ALL-CAPS-style keys, and server names are lowercase.)
            filename = f'mapconf/server-{server}'
            v = rack.config.read(filename, formats=['.json', '.cnf'], lenient=True)
            if v:
                logger.info(f"Found file-based server conf '{filename}' for '{server}'.")
            else:
                suggest_server_conf(server)
                sys.exit(1)

        supported_layers = set(v.get("layers", []))
        if not supported_layers.intersection(set(layers)):
            #logger.info(f"Found server '{server}' providing some of the layers {layers} in its configuration: {supported_layers}.")
            #v["layers"] = list(supported_layers.intersection(set(layers))) # Note: order mat change?
            logger.warning(f"Server '{server}' has no layer '{layers}' but {supported_layers}.")
        if epsg is not None:
            supported_epsg = v.get("epsg", [])
            if epsg in supported_epsg:
                logger.info(f"Server '{server}' supports EPSG:{epsg}.")
                v["CRS"] = f"EPSG:{epsg}"
            else:
                logger.warning(f"Server '{server}' does not support EPSG:{epsg} in its configuration: {supported_epsg}. Using default CRS.")
        else:
            #raise ValueError(f"Server '{server}' does not have layer '{layers}' in its configuration.")
            #raise KeyError(f"EPSG' {epsg}' not found in configuration.")
            pass
        logger.info(f"Server '{server}' :{v}.")
        return v
    
    # Try exact match first, then look for layer in any server config.
    for (k,v) in conf.items():
        supported_layers = set(v.get("layers", []))
        if supported_layers.issuperset(set(layers)):
            logger.info(f"Found server '{k}' providing all the layers {layers} in its configuration.")
            v["layers"] = layers # Override with requested layers, if server provides all of them.
            return v
        else:
            logger.info(f"Server '{k}' does not have layer '{layers}' in its configuration: {supported_layers}.")

    # Try exact match first, then look for layer in any server config.
    for (k,v) in conf.items():
        supported_layers = set(v.get("layers", []))
        if supported_layers.intersection(set(layers)):
            logger.info(f"Found server '{k}' providing some of the layers {layers} in its configuration: {supported_layers}.")
            v["layers"] = list(supported_layers.intersection(set(layers))) # Note: order mat change?
            return v
        else:
            logger.info(f"Server '{k}' does not have layer '{layers}' in its configuration: {supported_layers}.")

    raise ValueError(f"No layer '{layers}' configurations.")    
    
    

def link_map(cache_path: pathlib.Path, mapLink: str = None):
    """Link `mapLink` to the retrieved/cached file at `cache_path`.

    If `mapLink` does not exist, it is created as a symlink to `cache_path`.
    If it already exists, it is considered correct only if it is a symlink
    pointing at this exact `cache_path` - i.e. the current mapCache syntax
    (which identifies the file's contents) would produce the same file.
    Otherwise, raise rather than silently overwrite or ignore the mismatch.
    """

    if not mapLink:
        return

    link_path = pathlib.Path(mapLink)
    target = cache_path.resolve()

    if link_path.is_symlink() or link_path.exists(): 
        # symlink may point to a non-existing file, but we want to check if it points to the correct target.
        #    if link_path.is_symlink() and link_path.resolve() == target:
        #if link_path.exists():
        if link_path.resolve() == target:
            logger.info(f"Link already exists and is correct: {link_path} -> {cache_path}")
            return
        else:
            logger.warning(f"Dst :{target}")
            logger.warning(f"New :{link_path.resolve()}")
        raise FileExistsError(
            f"'{link_path}' already exists but is not a link to the expected cache file "
            f"'{cache_path}' (as produced by the current mapCache syntax). Refusing to overwrite."
        )

    logger.info(f"Linking {link_path} -> {cache_path}")
    if link_path.parent != pathlib.Path('.'):
        logger.info(f"Creating parent directory for link: {link_path.parent}")
        os.makedirs(link_path.parent, exist_ok=True)
        #os.makedirs(link_path.parent, exist_ok=True)
    link_path.symlink_to(target)


def get(mapCache:str, mapServer:str="mundialis", mapLayers=None, mapForce=False, mapLink:str=None, **kw_args) -> pathlib.Path:
    """ Retrieve map, if not already in cache.

    Parameters:
    - mapCache: cache path syntax, e.g. "./mapcache/{layers}/BBOX={BBOX}_CRS={CRS}_SIZE={WIDTH},{HEIGHT}_STYLES={styles}.png"
    - mapServer: server name, e.g. "mundialis", "terrestris", "fmi", "nasa-neo"
    - mapLayers: list of layers to request, e.g. ["OSM-WMS", "TOPO-WMS"]
    - mapLink: if given, link this path to the retrieved/cached file
    - kw_args: additional parameters, e.g. BBOX, PROJ, SIZE,
        which will be used to construct the HTTP GET request and the cache path.
    Returns:
    - pathlib.Path to the cached map file, if successful.
    - None, if retrieval failed.
    """


    # Currently, built-in defaults...
    # terrestris
    # https://ows.terrestris.de/osm/service?
    # https://ows.terrestris.de/osm/service?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=OSM-WMS&STYLES=&CRS=EPSG:3035&BBOX=2500000,1500000,6500000,5500000&WIDTH=800&HEIGHT=600&FORMAT=image/png
    params = {
        "endpoint": "https://ows.terrestris.de/osm/service",
        "SERVICE": "WMS",
        "VERSION": "1.3.0",
        "REQUEST": "GetMap",
        #"layers": ["OSM-WMS"],
        "layers": [],
        "styles": "",
        "CRS": "EPSG:3067",
        "BBOX": "-80000,6390000,944000,7926000",
        #"CRS": "EPSG:3857",
        #"BBOX": "2610000,7500000,2620000,7510000",
        "WIDTH": "800",
        "HEIGHT": "600",
        "FORMAT": "image/png"
    }
    #PROJ=3067
    #BBOX=-80000,6390000,944000,7926000
    #SIZE=1024,1536 
    mapLayers = rack.typical(mapLayers, [str])

    server_conf = get_server_conf(server=mapServer, layers=mapLayers)
    if server_conf:
        for i in ["endpoint", "VERSION", "layers"]: # "REQUEST", "SERVICE",  
            if i in server_conf:
                params[i] = server_conf[i]
        #params["SERVICE"] = server_conf.get("SERVICE", params["SERVICE"])
        #params["VERSION"] = server_conf.get("VERSION", params["VERSION"])
        #params["layers"]  = ",".join(server_conf.get("layers",  params["layers"])) # "".join(',')

    # Change coordinate order for BBOX if WMS version is 1.3.0 or higher.
    if params["VERSION"] >= "1.3.0": # and "PROJ" in kw_args and bbox_needs_swap(server_conf["VERSION"], kw_args["PROJ"]):
        if "BBOX" in kw_args:
            bbox = rack.typical(kw_args["BBOX"], tuple, separator=',')
            lonLL, latLL, lonUR, latUR = bbox
            bbox = (latLL, lonLL, latUR, lonUR)
            kw_args["BBOX"] = rack.typical(bbox, str, separator=',')
            logger.info(f"WMS {params['VERSION']} : swapping BBOX axis order -> {bbox}")    
            #logger.info(f"WMS {server_conf['VERSION']} + EPSG:{kw_args['PROJ']}: swapping BBOX axis order -> {kw_args['BBOX']}")    


    params["layers"]  = ",".join(params["layers"])

    logger.info(f"defaults: {params}")

    get_params = construct_http_get(params, **kw_args)
    logger.info(f"get_param_str: {get_params}")

    cache_path = pathlib.Path(get_cache_path(cache_syntax=mapCache,
                                              mapServer=mapServer,
                                              **get_params))
    logger.info(f"Cache path: {cache_path}")
    
    if not (cache_path.exists() or mapForce):
        #logger.info(f"Does not exist, retrieving: {cache_path}")
        logger.info(f"Retrieving: {cache_path}")
        logger.info(f"Ensure dir: {cache_path.parent}")
        os.makedirs(cache_path.parent ,exist_ok=True) 
        # TODO: lock and/or tmpfile

        get_param_str = "&".join([f"{k}={v}" for (k,v) in get_params.items()])
        #url = "https://ows.terrestris.de/osm/service?" + get_param_str
        url = params["endpoint"] + '?' + get_param_str
        logger.info(f'url: {url}')

        # Assume myURL is already defined and valid
        #output_file = "wms_image.png"

        try:
            with urllib.request.urlopen(url) as response:
                content_type = response.headers.get("Content-Type", "")
                data = response.read()

                if "image/png" not in content_type.lower():
                    text = data.decode("utf-8", errors="replace")
                    msg = extract_wms_error(text)
                    raise RuntimeError(
                        f"WMS error: server did not return PNG.\n"
                        f"Content-Type: {content_type}\n"
                        f"Message: {msg}"
                    )

            with open(cache_path, "wb") as f:
                f.write(data)

            logger.info(f"Saved WMS image to {cache_path}")
            # link_map(cache_path, mapLink)
            # return cache_path
        
        except urllib.error.HTTPError as e:
            logger.error(f"HTTP error: {e.code} {e.reason}")

            try:
                text = e.read().decode("utf-8", errors="replace")
                msg = extract_wms_error(text)
                logger.error(f"WMS/server message: {msg}")
                raise e
            except Exception:
                #pass
                return None
            #return None

        except urllib.error.URLError as e:
            logger.error(f"Connection error: {e.reason}")
            raise e
        except Exception as e:
            logger.error(e)
            raise e

    logger.info(f"EXIST: {cache_path}")
    if mapLink:
        logger.info(f"Linking: {cache_path} to {mapLink}")
        link_map(cache_path, mapLink)

    return cache_path
    #    else:

    #logger.error(f"Did NOT save {cache_path}")

    """
    with urllib.request.urlopen(url) as response:
        image_data = response.read()  # bytes
        with open(cache_path, "wb") as f:
            f.write(image_data)
    """
    #print(f"Saved WMS image to {cache_path}")

    return None


"""
    urllib.request.urlopen("mika")
    with urllib.request.urlopen(myURL) as response:
        image_data = response.read()  # bytes

        with open(output_file, "wb") as f:
            f.write(image_data)

    print(f"Saved WMS image to {output_file}")
"""



def main():

    parser = build_parser()

    argv = sys.argv[1:]
    if len(argv) >= 2 and argv[0] in ('-h', '--help'):
        if print_argument_help(parser, argv[1]):
            return
        print(f"Unknown argument: {argv[1]}\n")
        parser.print_help()
        sys.exit(2)

    # Apply --GEOCONF as new parser defaults *before* the real parse, so that
    # CLI-given values (which always beat argparse defaults) still win over
    # the geoconf file, and the geoconf file still wins over the built-in
    # defaults. Same pattern as --config in rack.composite / rack.vertical / rack.args.
    known_args, _ = parser.parse_known_args(argv)
    geoconf_key = None
    if known_args.GEOCONF:
        geoconf_key, geoconf_path = rack.config.resolve_path(known_args.GEOCONF, "mapconf/geo-{key}")
        conf = rack.config.read(geoconf_path)
        logger.info(f"Setting parser defaults: {conf}")
        parser.set_defaults(**conf)

    args = parser.parse_args(argv)
    if geoconf_key:
        args.GEOCONF = geoconf_key
    #print(args)

    #get(cache=args.cache, server=args.server, layers=args.layers, **vars(args))
    get(**vars(args))


    #
    #if isinstance(args, dict):
    #    args = argparse.Namespace(**args)


if __name__ == "__main__":
    main()
