
def typical(obj, dst_type:type=None, separator:str=","):
    """ Convert obj to dst_type, if possible. If dst_type is None, convert to 
        type (str, list, int, float) based on the type of obj. 
        If separator is provided and obj is a string, it can be split into a list of strings. 
        If dst_type is a list or tuple and obj is a string, it can be split into a list or tuple of strings. 
        If dst_type is int or float, obj can be converted to that type. Otherwise, return obj as is. 
        This function can be used for parsing command line arguments or configuration values into appropriate types.
    """

    if isinstance(obj, dst_type):   
        return obj
    elif dst_type == str:
        if type(obj) == str:
            return obj  
        elif separator and isinstance(obj, (list,tuple)):
            # Ensure string, for join()
            obj = [str(v) for v in obj]
            return separator.join(obj)
        else:
            return str(obj)
    elif dst_type in (list,tuple):
        if separator and type(obj) == str:
            return dst_type(obj.split(separator))
        else:
            return dst_type(obj)
    elif dst_type in (int,float):
        return dst_type(obj)
    else:
        # warning or error?
        return obj
