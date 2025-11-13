""" Rack command line argument generator

Utility for collecting metadata simply using directories and text files.

    * property
    * property2

"""


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
        '-D', "--OUTDIR",
        type=str,
        metavar="<dir_syntax>",
        default='./statistics/{SITE}/{TIME|%M}min/dataset{DATASET}',
        help="String syntax for output directories. See --list-variables and --variables",
    )

    parser.add_argument(
        '-F', "--OUTFILE",
        type=str,
        metavar="<filename_syntax>",
        default='{SITECODE}_{POL}_{PRF}.txt',
        help="String syntax for output files. See --list-variables and --variables",
    )

    parser.add_argument(
        '-L', "--LINE",
        type=str,
        metavar="<syntax>",
        default='{TIME} {TIME_START|%H:%M:%S} {ELANGLE} # {QUANTITY}',
        help="Syntax for output lines. See --list-variables and --variables",
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
        "--gnuplot",
        type=str,
        metavar="<file>",
        default=None,
        help="Write applicable simple gnuplot script to a file",
    )

    parser.add_argument(
        "--gnuplot_columns",
        type=str,
        metavar="<col1,col2>",
        default=None,
        help="Columns to use for gnuplot (e.g., '2:3')",
    )


    rack.config.add_parameters(parser)
    rack.log.add_parameters(parser)

    return parser




#from collections import defaultdict
#
#def nested_dict():
#    return defaultdict(nested_dict)
# my_dict = nested_dict()

my_dict = dict()



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
"""
variables_fixed = {
    'SITECODE' : "${NOD}-${WMO}",
    'TASK':  '${what:date|%Y-%m-%d}T${what:time|%H:%M:%S}',
    'START': '${what:startdate|%Y-%m-%d}T${what:starttime|%H:%M:%S}',
    'END':   '${what:enddate|%Y-%m-%d}T${what:endtime|%H:%M:%S}',
    'FILE':  "${inputBasename}",
    'PRF'    : '${how:lowprf}-${how:highprf}',
    'QUANTITY':'${what:quantity}',
    #    'NOW':   "",
}

variables = {
    'SITE'   : '${NOD}',
    'TIMESTAMP': '${what:date|%Y-%m-%d}T${what:time|%H:%M:%S}',
    'ELANGLE': '${where:elangle|%05.2f}',
    'GEOM'   : '${where:nbins}x${where:nrays}x${where:rscale}',
    'POL' : 'AUTO', # UNSET
    'LDR' : 'AUTO', # UNSET
}
"""

variables_fixed = {
    "SITECODE" : {
        "desc": "Unique descriptor of the radar",
        "type": "string",
        "rack_expr": "${NOD}-${WMO}"
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
    "PRF" : {
        "desc": "Pulse repetition frequency",
        #"type": "",
        "rack_expr": "${how:lowprf}-${how:highprf}"
    },
    "QUANTITY": {
        "desc": "Quantity",
        "type": "list",
        "rack_expr": "${what:quantity}"
    },
    "POL" : {
        "desc": "",
        "type": "automatic",
        "rack_expr": "AUTO",
    }, 
    "LDR" : {
        "desc": "",
        "type": "automatic",
        "rack_expr": "AUTO",
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
        "rack_expr": "AUTO",
    }, 
    "LDR" : {
        "desc": "",
        "type": "automatic",
        "rack_expr": "AUTO",
    }
}

"""
    "TIMESTAMP": {
        "desc": "",
        "type": "",
        "rack_expr": "${what:date|%Y-%m-%d}T${what:time|%H:%M:%S}"
    },
"""




def extract_metadata(INFILES:list, variables:dict, metadata=dict()):

    log = logger.getChild("extract_metadata")
    #
    log.debug("start")

    global TIMEFORMAT
    SEPARATOR='_'

    # DEBUG
    #sfmt = SmartFormatter()

    var_map = [(k,v['rack_expr']) for (k,v) in variables.items()]
    #log.debug(var_map)
    
    (keys, values) = zip(*var_map)
    # log.debug(values)
    fmt = values 
    # fmt = list(variables.values())

    fmt = SEPARATOR.join(fmt)
    shared_cmd_args = f'--select data: --format {fmt}\n -o -'.split(' ')

    # Main loop 1: traverse HDF5 files
    for INFILE in INFILES:
        log.info(f'reading {INFILE}')
        
        # Todo: better cmd creator
        cmd = ['rack', INFILE ]
        cmd.extend(shared_cmd_args)
        logger.debug(" ".join(cmd))
        
        # Main loop 1: traverse HDF5 files
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        result = result.stdout.decode('utf-8')
        # Note: several lines, for each data<N> group!

        m = None
        for i in result.split(): # split by NEWLINE

            # Rejoin
            line = i.split(SEPARATOR)
            
            # quantity-wise info
            info = dict(zip(variables.keys(), i.split(SEPARATOR)))
            # logger.info(info)
            
            dataset_id = "{SITECODE}-{TIME_START}".format(**info)
            
            if dataset_id not in metadata:
                # Start new sweep
                log.info(f"dataset: {dataset_id}")
                if m:
                    logger.debug(m)
                m = metadata[dataset_id] = dict()
                m['QUANTITY'] = list()

            for i in ['TIME', 'TIME_START', 'TIME_END']:
                # Note: force UTC
                info[i] = dt.datetime.strptime(info[i], TIMEFORMAT).replace(tzinfo=dt.timezone.utc)


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
            
            log.debug(m)

    log.debug("end")
        

def write_metadata(metadata:dict, dir_syntax:str, file_syntax:str, line_syntax:str):
    """
    Write extracted metadata to file or standard output '-'.
    """
    
    log = logger.getChild("write_metadata")
    log.info(f"outdir_syntax:  {dir_syntax}")
    log.info(f"outfile_syntax: {file_syntax}")    
            
    # Results
    outdirs  = set()
    outfile_current=''
    outfile = sys.stdout
    STDOUT = (file_syntax == '-')

    line_tokens = rack.stringlet.parse_template(line_syntax)
    file_tokens = rack.stringlet.parse_template(file_syntax)
    dir_tokens  = rack.stringlet.parse_template(dir_syntax)

    
    def write_header(file_path):
        return f"# {line_syntax}"
    fileManager = rack.files.SmartFileManager(write_header)
    
    for dataset,info in my_dict.items():

        # Automatic additional attributes
        if 'LDR' in info['QUANTITY']:
            info['POL'] = 'LDR'
        elif set(info['QUANTITY']).intersection({'ZDR','RHOHV','KDP','PHIDP'}):
            info['POL'] = 'DUAL-POL'
        else:
            info['POL'] = 'SINGLE'

        # Reduce to single value, if both are same: str -> set -> str
        info['PRF'] = '-'.join(set(info['PRF'].split('-')))

        # list -> str
        info['QUANTITY'] = '-'.join(info['QUANTITY'])


        # line = line_syntax.format(**info).strip()
        # line = rack.formatter.smart_format(line_syntax, info).strip()
        line = rack.stringlet.tokens_tostring(line_tokens, info).strip() 
        
        if (STDOUT):
            print (line, file = outfile) # print adds newline
        else:
            #outdir = rack.formatter.smart_format(dir_syntax, info)
            outdir = rack.stringlet.tokens_tostring(dir_tokens, info)
            if outdir not in outdirs:
                log.info(f"outdir: {outdir}")
                os.makedirs(outdir,exist_ok=True) # mode=0o777
                outdirs.add(outdir)
                # RETHINK outfiles = set()
                # logger.info(f'outdir = {outdir}')

            # outfile = rack.formatter.smart_format(file_syntax, info)  
            outfile = rack.stringlet.tokens_tostring(file_tokens, info)
            # outfile = f'{outdir}/{outfile}'
            if outfile != outfile_current:
                log.info(f"outfile: {outfile}")

            fileManager.write(f'{outdir}/{outfile}', f'{line}\n')

            """
            file_path = pathlib.Path(f'{outdir}/{outfile}')
            is_new = not file_path.exists()                    
            with file_path.open("a", encoding="utf-8") as f:
                if is_new:
                    print (f'# {line_syntax}', file = f)
                    #f.write("# time  value  comment\n")  # header
                #f.write("2025-11-03T12:00  42.5  # sample data\n") 
                print (line, file = f)
            # outfile = open(f'{outdir}/{outfile}', 'a')
            # Todo: smarter file open/close
            """
            
        log.debug(info)
        
        #if (not STDOUT):
        #    outfile.close()

# Heavy but handy 
def derive_gnuplot_columns(args, conf: dict) -> tuple:
    
    log = logger.getChild("derive_gnuplot_columns")

    cols = (2,3)  # default
    
    if args.gnuplot_columns:
    
        cols = args.gnuplot_columns.split(',')
    
        if len(cols) != 2:
            log.error("Invalid --gnuplot_columns syntax, expected '<col1,col2>'")
            exit(1)

        if args.LINE: 

            # Split line syntax and get column indices
            tokens = rack.stringlet.parse_template(args.LINE)
            
            # Resolve to indices
            cols = [rack.stringlet.get_index(i, tokens, +1) for i in cols]
        
            var_tokens = rack.stringlet.get_vars(tokens)
            log.info(f"VARS: {var_tokens} ")
            #var_keys = rack.stringlet.get_var_keys(tokens)
            #log.info(f"VARS keys: {var_keys} ")
            log.warning(f"Using columns: {cols} ")
            # format_var = ['ydata', 'xdata'] # reversed, for pop
            format_var = 'x' # reversed, for pop
            for i in cols:
                var = var_tokens[i-1]  # 1-based
                if var.filters and (var.key in variables):
                    filter = var.filters[0]
                    log.warning(f"SUGGEST  conf format-{i} -> '{filter}' ")
                    var_conf = variables[var.key]
                    type_ = var_conf.get('type','string')
                    if type_ == 'datetime':
                        conf["timefmt"] = filter  # input format  (time)
                        conf[f"format_{format_var}"] = "%H:%M" # output format (x-axis)
                    #
                    #log.warning(f"SUGGEST  conf format-{i} -> datetime '{var.filters[0]}' ")
                        
                log.info(f"col[{i}]: '{var.key}'")
                format_var = 'y'  # next/remaining are y vars

            log.warning(f"experimental auto conf: '{conf}' ")
            

    return tuple(cols)




def create_gnuplot_script(files: list, settings=dict(), selector=(1,2)) -> str:

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
        #"using": '2:3',
    }
    conf.update(settings)

    #vars = get_vars(tokens)

    SEPARATOR='_'
    split_names = [f.replace('/',SEPARATOR).split(SEPARATOR) for f in files]
    distinct_indices = [i for i,values in enumerate(zip(*split_names)) if len(set(values)) > 1]
    shared_indices   = [i for i,values in enumerate(zip(*split_names)) if len(set(values)) == 1]


    # Take a "random" filename and pick common parts
    suffix = pathlib.Path(files[0]).suffix
    # log.debug(shared_indices)
    # split_name = pathlib.Path(files[0]).stem
    # split_name = pathlib.Path(files[0]).stem.replace('/', SEPARATOR).split(SEPARATOR)
    split_name = files[0].replace('/', SEPARATOR).split(SEPARATOR)
    title = " ".join([split_name[i] for i in shared_indices])
    conf['title'] = f'"{title}"'

    log.debug("add configuration")
    # prog = [f"set {k} {v}" for (k,v) in conf.items()]

    cmds = rack.gnuplot.GnuPlotCommandSequence()
    # Cmd = rack.gnuplot.GnuPlotCommand
    GP = rack.gnuplot.GnuPlot
    
    # cmds = GnuPlotCommandSequence()
    # cmds.add(rack.gnuplot.GnuPlot.set.terminal(rack.gnuplot.GnuPlot.Terminal.PNG, size=(800, 600)))
    # cmds.add(GnuPlot.set.output("plot_output.png"))

    for (k,v) in conf.items():
        # cmds.add(Cmd(k,v))
        func = getattr(GP.set, k)   # resolves GnuPlot.set.format_x
        cmds.add(func(v))


    plots = []
   
    log.debug("adding input files")
    files.reverse()
    # prog.append('plot \\')
    #
    selector = ":".join([str(i) for i in selector])
    #selector = ":".join(selector)
    while files:
        f = files.pop()
        #split_name = pathlib.Path(f).stem.split(SEPARATOR)
        split_name = f.replace('/',SEPARATOR).split(SEPARATOR)
        title =  " ".join([split_name[i] for i in distinct_indices])
        #plotline = "  '{infile}' using 2:3 with linespoints title '{title}'".format(infile=f, title=title)
        #if (files):
        #    plotline += ',\\'
        # prog.append(plotline)
        plots.append({"file": f, "using": selector, "with_": "lines", "title": title})

    cmds.add(GP.plot.plot( *plots ))
    #print("Generated GnuPlot script:\n")
    #print(cmds.to_string("\n"))
    #return prog # "\n".join(confs)+'\n'+",\n".join(plots)
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

        
    # Again?
    #if args.config:
    #    vars(args).update(rack.config.read(args.config))
            
    #rack.config.handle_parameters(args)
    if args.export_config:
        conf = vars(args)
        conf['variables'] = variables
        rack.config.write(args.export_config, conf)
        exit (0) # OK?
    
    if not args.INFILE:
        log.warning("No inputs?")
        # print help
        exit(0)

    if args.gnuplot:
    
        conf = dict()
        cols = derive_gnuplot_columns(args, conf)

        lines = create_gnuplot_script(args.INFILE, conf, selector=cols)
        log.info(f"wrote GnuPlot script: {args.gnuplot}")
        if args.gnuplot == 'exec':
            log.info("running?")
            print("\n".join(lines))
        elif args.gnuplot == '-':
            for i in lines:
                print(i)
        else:
            with open(args.gnuplot, 'w') as f:
                for i in lines:
                    f.write(i)
                    f.write('\n')
        exit(0)


    extract_metadata(args.INFILE, variables, my_dict)

    if args.write:
        write_metadata(my_dict, args.OUTDIR, args.OUTFILE, args.LINE)


    



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
