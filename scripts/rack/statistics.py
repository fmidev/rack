""" Rack command line argument generator

Utility for collecting metadata simply using directories and text files.

    * property
    * property2

"""
import argparse



import sys
#from pathlib import Path
import os # mkdirs()


#import rack.base
import pathlib #Path
import json
import subprocess

import rack.config
import rack.log
from rack.log import logger

# Here we go!
logger.name = pathlib.Path(__file__).name

def build_parser():
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
        default='./statistics/{SITE}/{MINUTE}min/dataset{DATASET}',
        help="String syntax for output directories. See --list-variables and --variables",
    )

    parser.add_argument(
        '-F', "--OUTFILE",
        type=str,
        metavar="<filename_syntax>",
        default='{TIMESTAMP}_{ELANGLE}_{PRF}_{GEOM}',
        help="String syntax for output files. See --list-variables and --variables",
    )

    parser.add_argument(
        '-L', "--LINE",
        type=str,
        metavar="<syntax>",
        default='{TIMESTAMP} {TIMESTAMP_START} {ELANGLE} # {QUANTITY}',
        help="Syntax for output lines. See --list-variables and --variables",
    )

    """
    parser.add_argument(
        "--format_datetime",
        type=str,
        metavar="<format>",
        default='%Y-%m-%dT%H:%M:%S',
        help="Syntax for output variables of type date/time"
    )
    """

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

    rack.config.add_parameters(parser)
    rack.log.add_parameters(parser)

    return parser




#from collections import defaultdict
#
#def nested_dict():
#    return defaultdict(nested_dict)
# my_dict = nested_dict()

my_dict = dict()

variables = {
    'SITE'   : '${NOD}',
    'YEAR'   : '${what:date|%Y}',
    'MONTH'  : '${what:date|%m}',
    'DAY'    : '${what:date|%d}',
    'HOUR'   : '${what:date|%H}',
    'MINUTE' : '${what:time|%M}',
    'SECOND' : '${what:time|%M}',
    'DATASET': '${dataset|%02d}',
    'TIMESTAMP': '${what:date|%Y-%m-%d}T${what:time|%M:%S}',
    'TIMESTAMP_START' : '${what:startdate|%Y-%m-%d}T${what:starttime|%M:%S}',
    'ELANGLE': '${where:elangle|%05.2f}',
    'PRF'    : '${how:lowprf}-${how:highprf}',
    'GEOM'   : '${where:nbins}x${where:nrays}x${where:rscale}',
    'QUANTITY':'${what:quantity}',
    'POL' : 'AUTO', # UNSET
    'LDR' : 'AUTO', # UNSET
}



def run(args):

    global variables

    rack.log.handle_parameters(args)

    # This happens only through API call of run(args)
    if args.config:
        vars(args).update(rack.config.read(args.config))

    if args.variables:
        if type(args.variables) is str:
            print("loading ", args.variables)
            variables = rack.config.read(args.variables)

    if args.list_variables:
        json.dump(variables, sys.stdout, indent=4)
        print()
        #for (k,v) in variables.items():
        #    print ('\t{"'+k+'":"'+v+'"}')
        

    if args.config:
        vars(args).update(rack.config.read(args.config))
            
    #rack.config.handle_parameters(args)
    if args.export_config:
        conf = vars(args)
        conf['variables'] = variables
        rack.config.write(args.export_config, conf)
        exit (0) # OK?

    
    SEPARATOR='_'
    
    if not args.INFILE:
        print("No inputs?")
        # print help
        exit(0)

    # "Invisible" main keys
    # fmt_variables = ['${dataset}', '${data}'].append(variables.values())
    # fmt_variables.update(variables.values())
    fmt = list(variables.values())
    fmt.append('${what:startdate}T${what:starttime}-${NOD}-${WMO}')
    #fmt.append('${data}')
    fmt = SEPARATOR.join(fmt)
    shared_cmd_args = f'--select data: --format {fmt}\n -o -'.split(' ')
    
    # Main loop 1: traverse HDF5 files
    for INFILE in args.INFILE:
        logger.debug(f'reading {INFILE}')
        # fmt = '${NOD}/${what:time|%M}/${path}'
        # fmt = '${NOD}/${what:time|%M}min/dataset${dataset|%02d}'
        # fmt = '${path}_${what:date|%Y-%m-%d}T${what:time|%M:%S}_${what:startdate|%Y-%m-%d}T${what:starttime|%M:%S}_
        #         ${where:elangle|%.2f}_${how:lowprf}/${how:highprf}_${where:nrays}x${where:nbins}x${whX.ere:rscale}'
        # fmt = ['dataset','data']

        # Todo: better cmd creator
        cmd = ['rack', INFILE ]
        cmd.extend(shared_cmd_args)
        logger.debug(" ".join(cmd))
        #
        
        # Main loop 1: traverse HDF5 files
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        metadata=result.stdout.decode('utf-8')
        # Note: several lines, for each data<N> group!

        m = None
        for i in metadata.split(): # split by NEWLINE
            # print (i.split(SEPARATOR))
            # Rejoin
            line = i.split(SEPARATOR)
            #data1   = line.pop()
            # Main KEY
            dataset = line.pop()

            # full quantity-wise info
            info = dict(zip(variables.keys(), i.split(SEPARATOR)))
            info['PRF'] = '-'.join(set(info['PRF'].split('-')))
            # print (data)
            # outdir  = args.OUTDIR.format(**info)
            #outfile = args.OUTFILE.format(**info)
            # outdir  = '/'.join(extract_values(data, ['SITE', 'MINUTE', 'DATASET']))
            # outfile = '_'.join(copy_values(data, ['NOMINAL','ELANGLE','PRF', 'GEOM']))
            # key = "{NOMINAL}_{ELANGLE}_{PRF}_{GEOM}".format(**data)

            if dataset not in my_dict:
                if m:
                    logger.debug(m)
                my_dict[dataset] = dict()
                
            m = my_dict[dataset]

            # Special handling for quantities
            QUANTITY = info.pop('QUANTITY')
            if ('QUANTITY' not in m):
                m['QUANTITY'] = list()
            m['QUANTITY'].append(QUANTITY)
            m.update(info)
            #print (m)
            logger.debug(m)


    file = sys.stderr
            
    # Results
    if (args.write):
        outdirs = set()
        outfile_current=''
        for dataset,info in my_dict.items():

            if 'LDR' in info['QUANTITY']:
                info['POL'] = 'DUAL-POL-LDR'
            elif set(info['QUANTITY']).intersection({'ZDR','RHOHV','KDP','PHIDP','LDR'}):
                info['POL'] = 'DUAL-POL'
            else:
                info['POL'] = 'SINGLE'
                
            info['QUANTITY'] = '-'.join(info['QUANTITY'])


            outdir = args.OUTDIR.format(**info)
            if outdir not in outdirs:
                logger.info(f'outdir = {outdir}')
                # Or local bookkeeping?
                os.makedirs(outdir,exist_ok=True) # mode=0o777

            outfile = args.OUTFILE.format(**info)
            #if (outfile != outfile_current):
            # YES open all the time
            file = open(f'{outdir}/{outfile}', 'a')

            #val[outfile] = v
            logger.debug(info)

            line = args.LINE.format(**info).strip()
            # print (f'{outdir} / {outfile}.txt : ', line )
            print (line, file = file) # print adds newline
            
            #if (args.write):
            file.close()


def main():

    parser = build_parser()

    # Detects --config
    rack.config.read_defaults(parser)

    args = parser.parse_args()

    # Better... (dangerous?)
    # if args.config:
    #    vars(args).update(rack.config.read(args.config))
        

    #print(args)
    #print(vars(args))
    run(args)




if __name__ == "__main__":
    main()
