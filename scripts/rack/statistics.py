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
import rack.config
import rack.log
from rack.log import logger
logger.name = pathlib.Path(__file__).name

def build_parser():
    """ Creates registry of supported options of this script
    """
    parser = argparse.ArgumentParser(description="Example app with JSON config support")

    parser.add_argument(
        "INFILE",
        nargs='*',
        help="Input files")

    
    parser.add_argument(
        '-D', "--OUTDIR",
        type=str,
        metavar="<dir_syntax>",
        default='./statistics/{SITE}/{MINUTE}/{DATASET}',
        help="String syntax for output directories.")

    parser.add_argument(
        '-F', "--OUTFILE",
        type=str,
        metavar="<filename_syntax>",
        default='{TIMESTAMP}_{ELANGLE}_{PRF}_{GEOM}',
        help="String syntax for output files."
    )

    parser.add_argument(
        '-L', "--LINE",
        type=str,
        metavar="<syntax>",
        default='{TIMESTAMP} {TIMESTAMP_START} {ELANGLE} # {QUANTITY}',
        help="Syntax for output lines."
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
        help="Mapping of program variables (SITE, TIMESTAMP,...) to syntax containing odim variables (${NOD}, ${what:date},...) "
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


import subprocess
from collections import defaultdict

def nested_dict():
    return defaultdict(nested_dict)

my_dict = nested_dict()




def run(args):

    
    variables = {
        'SITE'   : '${NOD}',
        'MINUTE' : '${what:time|%M}min',
        'DATASET': '${dataset|%02d}',
        'TIMESTAMP': '${what:date|%Y-%m-%d}T${what:time|%M:%S}',
        'TIMESTAMP_START'  : '${what:startdate|%Y-%m-%d}T${what:starttime|%M:%S}',
        'ELANGLE': '${where:elangle|%05.2f}',
        'PRF'    : '${how:lowprf}-${how:highprf}',
        'GEOM'   : '${where:nbins}x${where:nrays}x${where:rscale}',
        'QUANTITY':'${what:quantity}',
    }

    if args.variables:
        if type(args.variables) is str:
            print("loading ", args.variables)
            variables = rack.config.read(args.variables)

    if args.export_config:
        conf = vars(args)
        conf['variables'] = variables
        rack.config.write(args.export_config, conf)
        exit (0)
    
    SEPARATOR='_'
    
    if not args.INFILE:
        print("No inputs?")
        # print help
        exit(0)

    for INFILE in args.INFILE:
        logger.debug(f'reading {INFILE}')
        # fmt = '${NOD}/${what:time|%M}/${path}'
        # fmt = '${NOD}/${what:time|%M}min/dataset${dataset|%02d}'
        # fmt = '${path}_${what:date|%Y-%m-%d}T${what:time|%M:%S}_${what:startdate|%Y-%m-%d}T${what:starttime|%M:%S}_${where:elangle|%.2f}_${how:lowprf}/${how:highprf}_${where:nrays}x${where:nbins}x${whX.ere:rscale}'
        fmt = SEPARATOR.join(variables.values())
        cmd = f'rack {INFILE} --select data: --format {fmt}\n -o -'.split(' ')
        logger.debug(" ".join(cmd))


        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        # print(result.stdout.decode('utf-8'))
        metadata=result.stdout.decode('utf-8').split() # now by NEWLINE
        for i in metadata:
            # print (i)
            # print (i.split(SEPARATOR))
            data = dict(zip(variables.keys(), i.split(SEPARATOR)))
            data['PRF'] = ':'.join(set(data['PRF'].split('-')))
            # print (data)
            outdir  = args.OUTDIR.format(**data)
            outfile = args.OUTFILE.format(**data)
            # outdir  = '/'.join(extract_values(data, ['SITE', 'MINUTE', 'DATASET']))
            # outfile = '_'.join(copy_values(data, ['NOMINAL','ELANGLE','PRF', 'GEOM']))
            # key = "{NOMINAL}_{ELANGLE}_{PRF}_{GEOM}".format(**data)

            m = my_dict[outdir][outfile]

            # Special handling for quantities
            QUANTITY = data.pop('QUANTITY')
            if ('QUANTITY' not in m):
                m['QUANTITY'] = list()
            m['QUANTITY'].append(QUANTITY)
            m.update(data)


    file = sys.stderr
            
    # Results
    for outdir,val in my_dict.items():
        if (args.write):
            logger.info(outdir)
            os.makedirs(outdir,exist_ok=True) # mode=0o777
         
        for outfile,v in val.items():

            if (args.write):
                file = open(f'{outdir}/{outfile}', 'a')

            v = dict(v)
            if 'LDR' in v['QUANTITY']:
                v['LDR'] = True
            if set(v['QUANTITY']).intersection({'ZDR','RHOHV','KDP','PHIDP','LDR'}):
                v['DUALPOL'] = True
            v['QUANTITY'] = '-'.join(v['QUANTITY'])

            # write-back (as a std dict).. needed?
            val[outfile] = v

            line = args.LINE.format(**v).strip()
            # print (f'{outdir} / {outfile}.txt : ', line )
            print (line+'\n', file = file)
            
            if (args.write):
                file.close()


def main():

    parser = build_parser()
    args = parser.parse_args()
    #print(args)
    #print(vars(args))
    run(args)




if __name__ == "__main__":
    main()
