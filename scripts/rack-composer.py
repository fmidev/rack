#/usr/bin/python3
import os
import json
from argparse import ArgumentParser

parser = ArgumentParser(description = 'Create Rack command line and optionally, execute it.')

parser.add_argument(
    "-c", "--conf",
    type=str,
    default="cmd-dict.json",
    help="read product configuration", metavar="<file>.json")

parser.add_argument(
    "-g", "--geoconf",
    type=str,
    default="",
    help="read geographic configuration (BBOX, SIZE, PROJ)", metavar="<file>.json")

parser.add_argument(
    "-m", "--max", action='store_true',
    help="generate maximal command lines")

parser.add_argument(
    "-n", "--newline",
    type=str,
    default=" \\\n\t",
    help="separate cmd line args with this string")

parser.add_argument(
    "-e", "--exec", action='store_true',
    help="execute parsed command")

parser.add_argument(
    "-T", "--TIMESTAMP",
    type=str,
    default="202510011110,202510011110",
    help="loop variable (separate with commas)")

parser.add_argument(
    "-S", "--SITE",
    type=str,
    default="fikor,fianj",
    help="loop variable (separate with commas)")


args = parser.parse_args()

def load_json(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            data = json.load(file)
            return data
    except Exception as e:
        print(f"Error loading JSON file: {e}")
        return None



# Read main conf
cmd_conf = load_json(args.conf)

# Default geo conf
geo_conf = {
    "SIZE": [512,614],
    "PROJ": 3067,
    "BBOX": [-208000,6390000,1072000,7926000]
}
# Read optional geo conf
if args.geoconf:
    geo_conf.update(load_json(args.geoconf))

comp_conf = {
 "CMETHOD": "MAXIMUM"
}
comp_conf.update(geo_conf)


    

# Return basic types unformatted, containers as simple comma-separated values 
def arg2str(arg, separator=","):
    if (type(arg)==list) or (type(arg)==tuple):
        return separator.join([str(i) for i in arg])
    elif type(arg)==dict:
        return separator.join([f"{k}={v}" for (k,v) in arg.items()])
    else:
        return str(arg) #str(type(arg)) + 

    
def rack_args_composite(arg_list:list,
                        SIZE=[5120,6144],
                        PROJ=3067,
                        BBOX=[-208000,6390000,1072000,7926000],
                        CMETHOD="MAXIMUM"):
    args = {
        "SIZE": arg2str(SIZE),
        "PROJ": arg2str(PROJ),
        "BBOX": arg2str(BBOX),
        "CMETHOD": arg2str(CMETHOD)
    }
    #args.update(env)
    arg_list.extend("--cSize {SIZE}\\--cProj {PROJ}\\--cBBox {BBOX}\\--cMethod {CMETHOD}".format(**args).split('\\'))
    return arg_list

# General purpose arg composer.
# Adds --select "..." if any selection criterion (QUANTITY, DATASET, PRF, COUNT) supplied  
def rack_args(arg_list:list, QUANTITY="", DATASET=None, PRF="SINGLE", CMD="", CMD_PARAMS=None, COUNT=0, SELECT=None):

    if not SELECT:
        SELECT={}

    if (DATASET):
        SELECT["path"]  = "/dataset"+arg2str(DATASET,':')

    if (PRF):
        SELECT["prf"] = PRF

    if (QUANTITY):
        SELECT["quantity"] = arg2str(QUANTITY,':')
    
    if (SELECT):
        arg_list.append("--select " + arg2str(SELECT, ','))
    
    if (CMD):
        if (type(CMD_PARAMS) != None):
            arg_list.append(f"--{CMD} '{CMD_PARAMS}'")
        else:
            arg_list.append(f"--{CMD}")
            
    return arg_list


def rack_try_prefix(prefix_syntax:str, env: dict):
    try:
        return prefix_syntax.format(**env)
    except:
        return None

def rack_args_output(arg_list:list, output_basename:str, formats: list, output_prefix=""):
    # def rack_args_output(arg_list:list, output_filename:str, output_prefix=""):
    # arg_list.append(f"--echo {formats}")
    # return arg_list
    if output_prefix:
        arg_list.append(f"--outputPrefix {output_prefix}")

    #arg_list.append(f"--echo {formats}")
    if not formats:
        fmt = output_basename.split('.').pop()
        formats = [ fmt ]
        output_basename.removesuffix(fmt)
    
    #file_suffix = 
    fmts = [];
    fmts.extend(formats)
    if 'h5' in fmts:
        fmts.remove('h5')
        arg_list.append(f"--outputFile {output_basename}.h5")

    if 'tif' in fmts:
        fmts.remove('tif')
        arg_list.append(f"--outputConf tif:tile=512 -o {output_basename}.tif")

    if 'png' in fmts:
        fmts.remove('png')
        arg_list.append(f"--palette default -o {output_basename}.png")

    
    if (fmts):
        raise Exception('Unhandled formats:', fmts)
        
    return arg_list
            
    

args_composite = rack_args_composite([], **comp_conf)
# print ("\n  ".join(args_composite))

#conf = {}
#conf.update(conf_default)
#conf.update(geo_conf)

# print (conf)

infile_syntax  = cmd_conf['input']  # rename input  -> infile ?
outprefix_syntax = cmd_conf['output_prefix'] # rename output -> outfile?
outfile_syntax = cmd_conf['output_basename'] # rename output -> outfile?



for timestamp in args.TIMESTAMP.split(','):

    #conf['timestamp'] = timestamp
    for site in args.SITE.split(','):
        #conf['site'] = site
        #print (site)
        env = {"timestamp": timestamp, "site": site}
        infile  = infile_syntax.format(**env)
        #args_input = [infile]
        args_input = [f'--inputFile {infile}']
        
        rack_cmd = ['rack']
        rack_cmd.extend(args_composite)
        rack_cmd.extend(args_input)

        outprefix = rack_try_prefix(outprefix_syntax, env)
        print ("outprefix ", outprefix_syntax, ' -> ', outprefix)
        if outprefix:
            rack_cmd.append(f"--outputPrefix {outprefix}")
        
        
        for c in cmd_conf['products']:
            # if debug...
            # print (c)
            env.update(c) # risk: old values not removed?

            if not outprefix:
                outprefix = rack_try_prefix(outprefix_syntax, env)
                print ("outprefix ", outprefix_syntax, ' -> ', outprefix)
                if outprefix:
                    rack_cmd.append(f"--outputPrefix {outprefix}")
                    outprefix = None
                
        
            for parameter in c['parameters']:

                if args.max and not outprefix:
                    outprefix = rack_try_prefix(outprefix_syntax, env)
                    print ("outprefix ", outprefix_syntax, ' -> ', outprefix)
                    if outprefix:
                        rack_cmd.append(f"--outputPrefix {outprefix}")
                        outprefix = None

                env["parameter"] = parameter
                        
                args_prod=[]
                if (c["product_type"] == "ppi"):
                    args_prod = rack_args([], QUANTITY=c["quantity"], DATASET=int(parameter), COUNT=1)
                elif (c["product_type"] == "cappi"):
                    args_prod = rack_args([], CMD="pCappi", CMD_PARAMS=parameter)
                elif (c["product_type"] == "etop"):
                    args_prod = rack_args([], CMD="pEchoTop", CMD_PARAMS=parameter)
                else:
                    raise Exception('Unknow product_type:', c["product_type"])
                # print (args_prod)

                # continue
                if not args.max:
                    rack_cmd = ["rack"]
                    rack_cmd.extend(args_composite)
                    rack_cmd.extend(args_input)
                    outprefix = None

                rack_cmd.extend(args_prod)
                rack_cmd.append('--cCreateTile')
                    #rack_cmd.extend(args_prod)
                
                if not outprefix:
                    outprefix = rack_try_prefix(outprefix_syntax, env)
                    if outprefix:
                        rack_cmd.append(f"--outputPrefix {outprefix}")
                        

                file_format = c['file_format']
                # args_output = []
                outfile   = outfile_syntax.format(**env) #.removesuffix('.h5')
                args_output = rack_args_output([], output_basename=outfile, formats=file_format) #, output_prefix=outprefix)

                rack_cmd.extend(args_output)
                
                if not args.max:
                    print (args.newline.join(rack_cmd))
                    print ("")

                
        if args.max:
            #print (f"=== Full cmd for {timestamp}-{site} ===")
            print (args.newline.join(rack_cmd))                
            print ("\n\n")
