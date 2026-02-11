import rack.core


def build_parser(parser):
    """
    Selection: todo: move to separate function, and also support for multiple SELECT-like parameters (e.g. quantity and dataset)
    """
    parser.add_argument(
        "--SELECT",
        default=None,
        help="") 

    parser.add_argument(
        "--QUANTITY",
        default=None,
        #default='DBZH',
        metavar="<code>",
        help="Same as --SELECT quantity=<code> , where code is DBZH, VRAD, HGHT") 

    parser.add_argument(
        "--DATASET",
        default='', 
        metavar="<index>[:<index2>]",
        help="Same as --SELECT path=/dataset<index>") 

    parser.add_argument(
        "--PRF",
        default=None, 
        metavar="SINGLE|DOUBLE|ANY",
        help="Same as --SELECT prf=<prf>") 
    
    


def handle_select(args, progBuilder: rack.core.Rack):
 
    value = []
    if args.SELECT:
        value.append(args.SELECT)

    if args.DATASET:
        value.append(f"path=/dataset{args.DATASET}")

    if args.QUANTITY:
        value.append(f"quantity={args.QUANTITY}")

    if args.PRF:
        value.append(f"prf={args.PRF}")

    args = ",".join(value)
    if args:
        progBuilder.select(",".join(value))
