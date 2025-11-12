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
#import rack.log
import rack.config

# from rack.log import logger
# from rack.formatter import SmartFormatter
from rack.formatter import smart_format
#import rack.gnuplot as gp # GnuPlotCommand, GnuPlotCommandSequence
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
        help="Mapping of program variables. To see it, issue --list-variables",
    )

    parser.add_argument(
        "--list-variables",
        dest='list_variables',
        action='store_true',
        help="List available variable mappings"
        #(SITE, TIMESTAMP,...) to syntax containing odim variables (${NOD}, ${what:date},...) ""
    )


    parser.add_argument(
        "--write",
        type=bool,
        #metavar="<file>.json",
        default=True,
        help="Write to text files"
    )

    
    parser.add_argument(
        "--gnuplot",
        type=str,
        metavar="<file>",
        default=None,
        help="Write applicable simple gnuplot script to a file",
    )


    rack.config.add_parameters(parser)
    rack.base.add_parameters_logging(parser)

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
    fmt = values # list(variables.values())
    #fmt = list(variables.values())



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

            # Kludge (conversion through string) and fragile (format not fixed)?
            # 
            # info['TIME_START_REL'] = dt.datetime.strptime(str(info['TIME_START'] - info['TIME']), "%H:%M:%S")
            # info['TIME_END_REL']   = dt.datetime.strptime(str(info['TIME_END']   - info['TIME']), "%H:%M:%S")

            # Better: dt.datetime.fromtimestamp(21020102, dt.UTC)
            time = info['TIME'].timestamp()
            time_start = info['TIME_START'].timestamp()
            time_end   = info['TIME_END'].timestamp()
            #print (time_start - time)
            #print (time_end - time)
            info['TIME_START_REL'] = dt.datetime.fromtimestamp(time_start - time, dt.timezone.utc) #.replace(tzinfo=dt.timezone.utc)
            info['TIME_END_REL']   = dt.datetime.fromtimestamp(time_end   - time, dt.timezone.utc) #.replace(tzinfo=dt.timezone.utc)

            """
            time = dt.datetime.fromtimestamp(0)
            print ('normi', time, time.timestamp(), time.strftime('%s'))
            time = dt.datetime.fromtimestamp(0, dt.timezone.utc)
            print ('hormi', time, time.timestamp(), time.strftime('%s'))
            time = dt.datetime.strptime('1970-01-01 00:00 UTC', "%Y-%m-%d %H:%M %Z")
            print ('normi', time, time.timestamp(), time.strftime('%s'))
            time = dt.datetime.fromtimestamp(0, dt.timezone.utc)
            print (time, time.timestamp(), time.strftime('%s'))
            """
            
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

        if (not STDOUT):
            #outdir = dir_syntax.format(**info)
            outdir = rack.formatter.smart_format(dir_syntax, info)
            if outdir not in outdirs:
                log.info(f"outdir: {outdir}")
                os.makedirs(outdir,exist_ok=True) # mode=0o777
                outdirs.add(outdir)
                # RETHINK outfiles = set()
                # logger.info(f'outdir = {outdir}')
            outfile = rack.formatter.smart_format(file_syntax, info) #file_syntax.format(**info)
            if outfile != outfile_current:
                log.info(f"outfile: {outfile}")
            #if outfile not in outfiles:
                # to stg
                # RETHINK reset
            #    outfile.add(outfile)
                # Note: reseted for each new dir (but not when revisiting dir... hmm...)
            outfile = open(f'{outdir}/{outfile}', 'a')
            # Todo: smarter file open/close
            
        log.debug(info)
        
        # line = line_syntax.format(**info).strip()
        # line = rack.formatter.smart_format(line_syntax, info).strip()
        line = rack.stringlet.tokens_tostring(line_tokens, info).strip() 
        print (line, file = outfile) # print adds newline

        if (not STDOUT):
            outfile.close()



def create_gnuplot_script(files: list, settings=dict()) -> str:

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
    while files:
        f = files.pop()
        #split_name = pathlib.Path(f).stem.split(SEPARATOR)
        split_name = f.replace('/',SEPARATOR).split(SEPARATOR)
        title =  " ".join([split_name[i] for i in distinct_indices])
        plotline = "  '{infile}' using 2:3 with linespoints title '{title}'".format(infile=f, title=title)
        if (files):
            plotline += ',\\'
        # prog.append(plotline)
        plots.append({"file": f, "using": "2:3", "with_": "lines", "title": title})

    # print(plots)
    cmds.add(GP.plot.plot( *plots ))
    #{"expr": "sin(x)", "title": "Sine", "with_": "lines"},
    #{"expr": "cos(x)", "title": "Cosine", "with_": "lines"}
    # cmds.to_script("example_plot.plt")
    print("Generated GnuPlot script:\n")
    print(cmds.to_string("\n"))
    #return prog # "\n".join(confs)+'\n'+",\n".join(plots)
    return cmds.to_list()

        
def run(args):

    global variables_fixed
    global variables
    global logger
    
    log = logger.getChild("run")
 
    # Handle --log_level <level>, --debug, --verbose
    rack.base.handle_parameters_logging(args)

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
            
    if args.list_variables:
        json.dump(variables, sys.stdout, indent=4)
        print()
        keys=list(variables_fixed.keys())
        log.info(f"Note: reserved (automatic) variables: {keys}")
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
        lines = create_gnuplot_script(args.INFILE)
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
