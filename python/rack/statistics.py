""" Rack command line argument generator

Utility for collecting metadata simply using directories and text files.

    * property
    * property2

"""


import re
import sys
import os # mkdirs()
import pathlib #Path
import argparse
import json
import subprocess
import datetime as dt

import rack.base
import rack.log
import rack.config
import rack.stringlet
import rack.files  # SmartFileManager


# from rack.formatter import smart_format
import rack.gnuplot  # GnuPlotCommand, GnuPlotCommandSequence


# Here we go!
#logger.name = pathlib.Path(__file__).name
logger = rack.base.logger.getChild(pathlib.Path(__file__).stem)

def build_parser():

    log = logger.getChild("parser")
    log.debug("parsing")
    
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Example app with JSON config support")

    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files",
    )

    
    parser.add_argument(
        '-D', "--outdir-syntax",
        type=str,
        metavar="<syntax>",
        #default='./statistics/{SITE}/{TIME|%M}min/dataset{DATASET}',
        default='./statistics/{SITE}/{TIME|%M}min',
        help="String syntax for output directories. See --export-variables and --variables",
    )

    parser.add_argument(
        '-F', "--outfile-syntax",
        type=str,
        metavar="<syntax>",
        default='{SITECODE}_{POL}_{PRF}.txt',
        help="String syntax for output files. See --export-variables and --variables",
    )

    parser.add_argument(
        '-L', "--line-syntax",
        type=str,
        metavar="<syntax>",
        default='{TIME} {TIME_START|%H:%M:%S} {ELANGLE} # {QUANTITY}',
        help="Syntax for output lines. See --export-variables and --variables",
    )


    parser.add_argument(
        "--variables",
        type=str,
        metavar="<file>.json",
        default=None,
        help="Read declaration of (additional) global variables, display with: --export-variables",
    )

    parser.add_argument(
        "--export-variables",
        #dest='list_variables',
        action='store_true',
        help="List available global variables"
        #(SITE, TIMESTAMP,...) to syntax containing odim variables (${NOD}, ${what:date},...) ""
    )


    parser.add_argument(
        "--write",
        type=bool,
        #metavar="<file>.json",
        default=True,
        help="Write to text files. If false, dry-run - just extract metadata."
    )

    
    parser.add_argument(
        "--gnuplot-script",
        type=str,
        metavar="<file>.gnu",
        default=None,
        help="Write applicable simple gnuplot script to a file",
    )

    parser.add_argument(
        "--gnuplot-output",
        type=str,
        metavar="<file>[.png|.svg]",
        default=None,
        help="Set gnuplot output file name (overrides default 'out.png')",
    )

    parser.add_argument(
        "--gnuplot",
        type=str,
        metavar="<file>",
        default=None,
        help="Create and run gnuplot script, creating a graphics/image file",
    )


    parser.add_argument(
        "--gnuplot-columns",
        type=str,
        metavar="<col1,col2>",
        default=None,
        help="Columns to plot (e.g., '2:3' or 'TIME:ELANGLE')",
    )


    rack.config.add_parameters(parser)
    rack.log.add_parameters(parser)

    return parser




#from collections import defaultdict
#
#def nested_dict():
#    return defaultdict(nested_dict)
# my_dict = nested_dict()

my_stats = dict()



"""
t = dt.datetime.now()
dt.datetime.strftime(t, "%m %Y")
dt.datetime.strptime("2025-10-09","%Y-%m-%d")
"""


"""
    TASK
    START
    END
    FILE
    NOW
"""

TIMEFORMAT='%Y-%m-%dT%H:%M:%S'

# This are used and handled by the system

variables_fixed = {
    "SITECODE" : {
        "desc": "Unique descriptor of the radar",
        "type": "string",
        "rack_expr": "${NOD}-${WMO}"
    },
    "SITE" : {
        "desc": "Radar identifier NOD (5-letter)",
        "type": "string",
        "rack_expr": "${NOD}"
    },
    "COUNTRY" : {
        "desc": "Country prefix (2-letter) of NOD code",
        "type": "string",
        "rack_expr": "${NOD|0:2}"
    },
    "SITE3" : {
        "desc": "National radar identifier (3-letter, omitting 2-letter COUNTRY prefix)",
        "type": "string",
        "rack_expr": "${NOD|2:3}"
    },
    "TIME": {
        "desc": "Nominal time of measurement",
        "type": "datetime",
        "rack_expr": "${what:date|%Y-%m-%d}T${what:time|%H:%M:%S}"
    },
    "TIME_START": {
        "desc": "Start time of scan",
        "type": "datetime",
        "rack_expr": "${what:startdate|%Y-%m-%d}T${what:starttime|%H:%M:%S}"
    },
    "TIME_END": {
        "desc": "End time of scan",
        "type": "datetime",
        "rack_expr": "${what:enddate|%Y-%m-%d}T${what:endtime|%H:%M:%S}"
    },
    "TIME_START_REL": {
        "desc": "Difference between start time and nominal time",
        "type": "datetime",
        "rack_expr": "AUTOMATIC",
    },
    "TIME_END_REL": {
        "desc": "Difference between end time and nominal time",
        "type": "datetime",
        "rack_expr": "AUTOMATIC",
    },
    "FILE": {
        "desc": "Filename without directory and extension",
        "type": "",
        "rack_expr": "${inputBasename}"
    },
    "QUANTITY": {
        "desc": "Quantity",
        "type": "list",
        "rack_expr": "${what:quantity}"
    },
    "POL" : {
        "desc": "Polarization mode ('SINGLE'|'DUAL-POL|'LDR')",
        "type": "automatic",
        "rack_expr": "AUTOMATIC",
    }, 
    "PRF" : {
        "desc": "Pulse repetition mode (1PRF or 2PRF)",
    },
    "PRF_LO" : {
        "desc": "Pulse repetition frequency",
        "rack_expr": "${how:lowprf}"
    },
    "PRF_HI" : {
        "desc": "Pulse repetition frequency (higher)",
        "rack_expr": "${how:highprf}"
    },
    "PRFS" : {
        "desc": "All the pulse repetition frequencies",
    },
    "DATASET": {
        "desc": "index in 'dataset<i>' group ",
        "type": "string",
        "rack_expr": "${dataset}"
    },
    "DATA": {
        "desc": "index in 'data<i>' group ",
        "type": "string",
        "rack_expr": "${data}"
    }
}

variables = {
    "SITE"   : {
        "desc": "Unique descriptor of the radar",
        "rack_expr": "${NOD}"
    },
    "ELANGLE": {
        "desc": "Elevation angle",
        "type": "",
        "rack_expr": "${where:elangle|%05.2f}"
    },
    "GEOM": {
        "desc": "Geometry",
        "type": "",
        "rack_expr": "${where:nbins}x${where:nrays}x${where:rscale}"
    },
    "POL" : {
        "desc": "",
        "type": "automatic",
    }, 
}





def extract_metadata(INFILES:list, variables:dict, metadata=dict()):

    log = logger.getChild("extract_metadata")
    #
    log.info("start")

    global TIMEFORMAT
    SEPARATOR='_'

    # DEBUG
    #sfmt = SmartFormatter()

    var_map = [(k,v.get('rack_expr','AUTO')) for (k,v) in variables.items()]
    #log.debug(var_map)
    log.info(var_map)

    (keys, values) = zip(*var_map)
    # log.debug(values)
    fmt = values
    # fmt = list(variables.values())

    fmt = SEPARATOR.join(fmt)
    log.info(f"fmt = {fmt}")

    shared_cmd_args = f'--select data: --format {fmt}\n -o -'.split(' ')

    # Main loop 1: traverse HDF5 files
    for INFILE in INFILES:
        log.debug(f'reading {INFILE}') # rack echoes it, so skip here
        
        # Todo: better cmd creator
        cmd = ['rack', INFILE ]
        cmd.extend(shared_cmd_args)
        logger.debug(" ".join(cmd))
        
        # Main loop 1: traverse HDF5 files
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        result = result.stdout.decode('utf-8')
        # Note: several lines, for each data<N> group!

        m = None
        for i in result.split(): # split by NEWline_syntax

            # Rejoin
            line = i.split(SEPARATOR)
            
            # quantity-wise info
            info = dict(zip(variables.keys(), i.split(SEPARATOR)))
            # logger.info(info)
            
            dataset_id = "{SITECODE}-{TIME_START}".format(**info)
            
            if dataset_id not in metadata:
                # Start new sweep
                log.debug(f"dataset: {dataset_id}")
                if m:
                    logger.debug(m)
                m = metadata[dataset_id] = dict()
                m['QUANTITY'] = list()

            # Parse datetime fields. Sensitive - if one fails, skip entire entry
            try:
                for i in ['TIME', 'TIME_START', 'TIME_END']:
                    # Note:  force UTC
                    info[i] = dt.datetime.strptime(info[i], TIMEFORMAT).replace(tzinfo=dt.timezone.utc)
            except Exception as e:
                log.error(f"Skipping {INFILE} – failed to parse datetime for {i}: '{info[i]}'")
                # remove incomplete entry
                metadata.pop(dataset_id)
                m = None
                continue
                #raise e 


            # Better: dt.datetime.fromtimestamp(21020102, dt.UTC)
            time = info['TIME'].timestamp()
            time_start = info['TIME_START'].timestamp()
            time_end   = info['TIME_END'].timestamp()
            #print (time_start - time)
            #print (time_end - time)
            info['TIME_START_REL'] = dt.datetime.fromtimestamp(time_start - time, dt.timezone.utc) #.replace(tzinfo=dt.timezone.utc)
            info['TIME_END_REL']   = dt.datetime.fromtimestamp(time_end   - time, dt.timezone.utc) #.replace(tzinfo=dt.timezone.utc)

            
            #print(sfmt.format("Time: {TASK|%Y-%m-%d %H %M}\n", **info))
                    
            # Special handling for some properties
            QUANTITY = info.pop('QUANTITY')
            m.update(info)            
            m['QUANTITY'].append(QUANTITY)

            # Reduce to single value, if both are same: str -> set -> str
            prfs = set()
            for i in ["PRF_LO", "PRF_HI"]:
                prf = m.get(i, "")
                if (prf):
                    prfs.add(int(prf))

            prfs = list(prfs)
            prfs.sort()
            m['PRF'] = f"{len(prfs)}PRF"            
            m['PRFS'] = prfs
            log.debug(m)

    log.debug("end")
        

def write_metadata(metadata:dict, dir_syntax:str, file_syntax:str, line_syntax:str):
    """
    Write extracted metadata to file or standard output '-'.
    """
    
    log = logger.getChild("write_metadata")

    log.debug("start")
    log.info(f"outdir_syntax:  {dir_syntax}")
    log.info(f"outfile_syntax: {file_syntax}")    
            
    # Results
    outdirs  = set()
    # outfile_current=''
    outfile = sys.stdout
    STDOUT = (file_syntax == '-')

    line_tokens = rack.stringlet.Stringlet(line_syntax)
    file_tokens = rack.stringlet.Stringlet(file_syntax)
    dir_tokens  = rack.stringlet.Stringlet(dir_syntax)

    log.warning(f"line_tokens: {line_tokens}")

    
    def write_header(file_path):
        # Write metadata variable keys (and not file path) 
        return f"# {line_syntax}"
    fileManager = rack.files.SmartFileManager(write_header)
    
    for info in my_stats.values():
        # for dataset,info in my_dict.items():

        # Automatic additional attributes
        if 'LDR' in info['QUANTITY']:
            info['POL'] = 'LDR'
        elif set(info['QUANTITY']).intersection({'ZDR','RHOHV','KDP','PHIDP'}):
            info['POL'] = '2POL'
        else:
            info['POL'] = '1POL'
            
        # list -> str
        # info['QUANTITY'] = '-'.join(info['QUANTITY'])

        line = line_tokens.string(info).strip()
        # line = rack.stringlet.string(line_tokens, info).strip() 
        
        if (STDOUT):
            print (line, file = outfile) # print adds newline
        else:
            outdir = dir_tokens.string(info) #rack.stringlet.string(dir_tokens, info)
            if outdir not in outdirs:
                log.debug(f"ensuring outdir: {outdir}")
                os.makedirs(outdir,exist_ok=True) # mode=0o777
                outdirs.add(outdir)

            # outfile = rack.stringlet.string(file_tokens, info)
            outfile = file_tokens.string(info) #rack.stringlet.string(file_tokens, info) 
            fileManager.write(f'{outdir}/{outfile}', f'{line}\n')
            
        log.debug(info)


    open_files = fileManager.get_filenames()        
    if (open_files):
        n = len(open_files)
        log.info(f"Wrote to {n} files") # : {open_files[0]} ... {open_files[n-1]}")
        log.info(f"First file: {open_files[0]}")
        log.info(f"Last file:  {open_files[n-1]}")
        if log.getEffectiveLevel() <= rack.log.logging.DEBUG:
            for f in open_files:
                log.debug(f" - {f}")    
        fileManager.close() # Actually redundant here, but for clarity


# Heavy but handy 
def derive_gnuplot_columns(col_indices:str, line_syntax: str, conf: dict) -> tuple:
    
    log = logger.getChild("derive_gnuplot_columns")

    #cols_result = {2:"TIME",3:"VALUE"}  # default
    #cols = (2,3)  # default

    if not col_indices:

        # log.info("No --gnuplot_columns given, trying to derive from --line_syntax syntax")
        # Default columns: assume second col is some kiond of time, third is value (ELANGLE)
        # return [(2,"TIME"), (3,"VALUE")] 
        return [(2,3), ("TIME","VALUE")] 
    
    else:
    
        cols_result = [] #dict() # {2:"TIME",3:"VALUE"} 
        cols = col_indices.split(',')
    
        if len(cols) != 2:
            log.error("Invalid --gnuplot_columns syntax, expected '<col1,col2>'")
            exit(1)

        if line_syntax: 

            # Split line syntax and get column indices
            tokens = rack.stringlet.Stringlet(line_syntax)
            
            # Resolve to indices
            #cols = [tokens.get_index(i, +1) for i in cols]
            cols = [tokens.get_index(i)+1 for i in cols]
        
            var_tokens = tokens.get_vars()
            log.debug(f"Variable stringlet: {var_tokens} ")
            # var_keys = rack.stringlet.get_var_keys(tokens)
            # log.info(f"VARS keys: {var_keys} ")
            log.debug(f"Using columns: {cols} ")
            # format_var = ['ydata', 'xdata'] # reversed, for pop
            format_var = 'x' # reversed, for pop
            for i in cols:
                var = var_tokens[i-1]  # 1-based
                cols_result.append((i, var.key))
                if var.filters and (var.key in variables):
                    filter = var.filters[0]
                    # log.warning(f"SUGGEST  conf format-{i} -> '{filter}' ")
                    var_conf = variables[var.key]
                    type_ = var_conf.get('type','string')
                    if type_ == 'datetime':
                        conf["timefmt"] = filter  # input format  (time)
                        conf[f"format_{format_var}"] = "%H:%M" # output format (x-axis)
                    #
                    #log.warning(f"SUGGEST  conf format-{i} -> datetime '{var.filters[0]}' ")
                    log.info(f"{format_var}: column {i}: '{var.key}'")
                else:        
                    log.info(f"{format_var}: column {i}: '{var.key}'-> filter='{filter}'")
                format_var = 'y'  # next/remaining are y vars

            log.debug(f"experimental auto conf: '{conf}' ")
            
    return zip(*cols_result)
    

class TitleMagic:
    """ Automagically derive distinct and shared parts of filenames for titles.
    Split file paths by standard directory separator '/' and filename segment separator SEPARATOR
    A list of lists: each sublist contains parts of one filename
    """
    distinct_indices = []
    shared_indices   = []
    shared_keys = []
    SEPARATOR='_'

    def __init__(self, files: list, separator: str = '_'):

        global logger
        log = logger.getChild("TitleMagic")
        # Automagically derive distinct and shared parts of filenames for titles.
        # Split file paths by standard directory separator '/' and filename segment separator SEPARATOR
        # A list of lists: each sublist contains parts of one filename
        self.SEPARATOR=separator
        split_names = [self.split_filepath(f) for f in files]   
        # x contains now e.g. [['data-acc', '201703061200', 'radar.polar.fiuta.h5'], [...], ...]
        # x = list(zip(*split_names))  
        # log.warning(x)
        for i,values in enumerate(zip(*split_names)):
            # log.warning(f"i={i} values={values} set={set(values)} len={len(set(values))}")
            if len(set(values)) == 1:  # all same
                self.shared_indices.append(i)
                self.shared_keys.append(values[0])
            else:
                self.distinct_indices.append(i)

        log.info(f"distinct indices: {self.distinct_indices}")
        log.info(f"shared indices:   {self.shared_indices}")
        log.info(f"shared keys:      {self.shared_keys}")   

    def split_filepath(self, filepath: str) -> list:
        """ Split filepath into parts by '/' and SEPARATOR """
        # re.split(f'[/|{self.SEPARATOR}]', f)
        return filepath.replace('/',self.SEPARATOR).split(self.SEPARATOR)

    def get_title(self) -> str:
        return " ".join(self.shared_keys)

    def get_distinct_keys(self, filepath_keys: list) -> list:
        
        if not type(filepath_keys) is list:
            filepath_keys = self.split_filepath(str(filepath_keys))
        
        #keys = self.split_filepath(filepath) 
        return [filepath_keys[i] for i in self.distinct_indices]
        


    def get_plot_title(self, filepath_keys: list) -> str:
        
        if not type(filepath_keys) is list:
            filepath_keys = self.split_filepath(str(filepath_keys))
        
        return " ".join(filepath_keys) #.removesuffix(suffix)
        

    def get_line_style(self, filepath_keys:list, default: str = "lines") -> str:
        
        if not type(filepath_keys) is list:
            filepath_keys = self.split_filepath(str(filepath_keys))

        linestyle_dict = {
            #'dashed': 'dashed',
            'DUAL-POL': 'linespoints',
            #'points': 'points',
            'SINGLE': 'lines lw 4',
        }

        for k in filepath_keys:
            if k in linestyle_dict:
                return linestyle_dict[k]    

        return default

    # site_syntax = re.compile(r'fiika|fikor|fiuta')
    site_syntax = re.compile(r'^(de|dk|ee|fi|no|se)([a-z]{3})$')

    def get_color(self, filepath_keys:list, default: str = "") -> str:
        
        log = logger.getChild("TitleMagic.get_color")

        if not type(filepath_keys) is list:
            filepath_keys = self.split_filepath(str(filepath_keys))


        color_dict = {
            #'dashed': 'dashed',
            'fiika': 'lt rgb "red"',
            'fikor': 'lt rgb "brown"',        
            'fiuta': 'lt rgb "blue"'
            }

        color_dict_nms = {
            #'dashed': 'dashed',
            'dk': 'lt rgb "brown"',
            'ee': 'lt rgb "black"',
            'fi': 'lt rgb "blue"',
            'no': 'lt rgb "red"',        
            'se': 'lt rgb "green"',
        }

        for k in filepath_keys:
            m =  self.site_syntax.match(k)
            if m:
                log.info(f"matched site code: {m.group(0)}")        
                nms_code  = m.group(1)
                if nms_code in color_dict_nms:
                    return color_dict_nms[nms_code] 
                site_code = m.group(0)
                if site_code in color_dict:
                    return color_dict[site_code]    


        for k in filepath_keys:
            if k in color_dict:
                return color_dict[k]    

        return default




def create_gnuplot_script(files: list, settings=dict(), columns=(1,2)) -> str:

    log = logger.getChild("create_gnuplot_script")
    
    conf = {
        "terminal": "png size 800,600",
        "output": '"out.png"',
        "datafile": "separator whitespace",
        "xdata": "time",
#        "timefmt": '"%Y-%m-%dT%H:%M:%S"', # must match with above TIMESTAMP
        "timefmt": '"%s"', # must match with above TIMESTAMP
#        "format": 'x "%s"',
        #"format": 'x "%H:%M"',
        "format_x": '%H:%M',
        # "format": ["y", '%H:%M'],
        "grid": "",
        "title": '"Measured data (actual timestamp)"',
        "xlabel": '"Time"',
        "ylabel": '"Value"',
        "key": "inside left top",
        #"using": '2:3',
    }
    conf.update(settings)


    SEPARATOR='_'
   
    suffix = pathlib.Path(files[0]).suffix
   
    # TODO: use outdir_syntax and outfile_syntax to derive title, line styles, colors, ...
    tm = TitleMagic(files, separator=SEPARATOR)
    conf['title'] = tm.get_title()
    
    log.debug("add configuration")

    cmds = rack.gnuplot.GnuPlotCommandSequence()
    
    for (k,v) in conf.items():
        # cmds.add(Cmd(k,v))
        func = getattr(rack.gnuplot.GnuPlot.set, k)   # resolves GnuPlot.set.format_x
        cmds.add(func(v))

    linetype_keys = []
    linecolor_keys = []

   
    columns = ":".join([str(i) for i in columns])
    log.debug(f"using columns: {columns}")
    plots = []
    log.debug("add plot command for each input file")
    for f in files:    
        filepath_keys = tm.split_filepath(f)
        distinct_keys = tm.get_distinct_keys(filepath_keys) 
   
        linetype = "with lines"
        if len(distinct_keys) >= 2:
            k = distinct_keys[1]
            if k not in linetype_keys:
                linetype_keys.append(k)
                log.debug(f"added linetype key: {k}")   
            linetype = "with linespoints linetype " + str(linetype_keys.index(k)+1)


        linecolor = ""
        if len (distinct_keys) >= 1:
            k = distinct_keys[0]
            if k not in linecolor_keys:
                linecolor_keys.append(k)
                log.debug(f"added linecolor key: {k}")
            linecolor = "linecolor " + str(linecolor_keys.index(k)+1)

        plot_title = tm.get_plot_title(distinct_keys).removesuffix(suffix)  # only after 3.9: removesuffix(suffix)
        #plot_style = tm.get_line_style(distinct_keys, default="lines")
        #plot_style = plot_style + " " + tm.get_color(distinct_keys, default="")
        #plot_style = f"{tm.get_line_style(distinct_keys, default='lines')} {linetype} {linecolor}".strip()
        plot_style = f"{linetype} lw 3 {linecolor}".strip()
        # log.debug(f"file: {f} title: '{plot_title}' style: '{plot_style}' columns: {columns}")
        #plots.append({"file": f, "using": columns, "with_": plot_style, "title": plot_title})
        plots.append({"file": f, "using": columns, "style": plot_style, "title": plot_title})

    cmds.add(rack.gnuplot.GnuPlot.plot.plot(*plots))
    # print(cmds.to_string("\n"))
    return cmds.to_list()

        
def run(args):

    global variables_fixed
    global variables
    global logger
    
    log = logger.getChild("run")
 
    # Handle --log_level <level>, --debug, --verbose
    rack.log.handle_parameters(args)

    # This happens only through API call of run(args)
    if args.config:
        log.debug("reading config file {args.config}")
        vars(args).update(rack.config.read(args.config))

    if args.variables:
        if type(args.variables) is str:
            print("loading ", args.variables)
            variables = rack.config.read(args.variables)

    # Override...
    variables.update(variables_fixed)
    #logger.warning(variables)
            
    if args.export_variables:
        json.dump(variables, sys.stdout, indent=4)
        print()
        keys=list(variables_fixed.keys())
        log.info(f"Note: reserved (automatic) variables: {keys}")
        exit (0) # OK?
        # for (k,v) in variables.items():
        #    print ('\t{"'+k+'":"'+v+'"}')

    if args.export_config:
        conf = vars(args)
        # conf['variables'] = variables
        rack.config.write(args.export_config, conf)
        exit (0) # OK?
    
    if not args.INFILE:
        log.warning("No inputs?")
        # print help
        exit(0)

        # debug mode: Check if files exist? (esp. for gnuplot)

    if args.line_syntax:
        args.line_syntax.replace(r'\t','\t')  #  


    if args.gnuplot:
        args.gnuplot_output = args.gnuplot # _output or args.gnuplot.replace('.gnu', '.png')
        # args.gnuplot_execute = True ?

    if args.gnuplot or args.gnuplot_script:    
        
        # More defaults here!
        conf = {
            "output": f'"{args.gnuplot_output or "out.png"}"',
        }
        
        col_indices, col_labels = derive_gnuplot_columns(args.gnuplot_columns, args.line_syntax, conf)
        # = zip(*cols)
        log.info(f"Using gnuplot columns: {col_indices} with labels {col_labels}")
        conf["xlabel"] = col_labels[0].replace('_',' ')
        conf["ylabel"] = col_labels[1].replace('_',' ')
        #cols = col_indices

        lines = create_gnuplot_script(args.INFILE, conf, columns=col_indices)

        if not args.gnuplot_script:
            pass
        elif args.gnuplot_script == '-':
            for i in lines:
                print(i)
        else:
            log.info(f"writing GnuPlot script: {args.gnuplot_script}")
            with open(args.gnuplot_script, 'w') as f:
                for i in lines:
                    print(i, file=f)

        if args.gnuplot:
            script=";\n  ".join(lines)
            log.info(f"Executing 'gnuplot', output: {args.gnuplot_output}")
            log.debug(f"GnuPlot script:\n{script}")
            result = subprocess.run(["gnuplot"], input=script, text=True, capture_output=True)
            #result = subprocess.run(["gnuplot"], input=script)

            if result.returncode != 0:
                log.error("execution of 'gnuplot' failed")
                print("STDOUT:", result.stdout)
                print("STDERR:", result.stderr)
                exit(1)
            #log.info(f"GnuPlot output: {args.gnuplot_output}")

        exit(0)

        

    extract_metadata(args.INFILE, variables, my_stats)

    if args.write:
        write_metadata(my_stats, args.outdir_syntax, args.outfile_syntax, args.line_syntax)


    



def main():

    parser = build_parser()

    # Detects --config
    rack.config.read_defaults(parser)

    args = parser.parse_args()

    # Better... (dangerous?)
    # if args.config:
    #    vars(args).update(rack.config.read(args.config))
        
    run(args)




if __name__ == "__main__":
    main()
