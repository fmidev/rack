
import re

def diagnose(self, x):
        print(f"diagnosing: *{x}*")
        print(type(x))
        print(x)
        print(x.__repr__())
        print(x.__str__())

def typical(obj, dst_type:type=None, separator:str=","):
    """ Convert obj to dst_type, if possible. If dst_type is None, convert to 
        type (str, list, int, float) based on the type of obj. 
        If separator is provided and obj is a string, it can be split into a list of strings. 
        If dst_type is a list or tuple and obj is a string, it can be split into a list or tuple of strings. 
        If dst_type is int or float, obj can be converted to that type. Otherwise, return obj as is. 
        This function can be used for parsing command line arguments or configuration values into appropriate types.
    """

    # Todo: alternative splitters, or regexp?

    if dst_type is None:
        return obj
    
    compound_types = (list, tuple, set)
    
    # Derive element type if dst_type is a list, tuple, or set with a single element type specified.
    elem_type = None #str #None
    if type(dst_type) != type:
        if isinstance(dst_type, compound_types):
            if len(dst_type) == 1:
                elem_type = dst_type[0]
                dst_type  = type(dst_type)
                #print(f"Ok, type: {dst_type} of {elem_type}")
            else:
                raise ValueError(f"dst_type {dst_type} is of compound type, but does not have exactly one element type specified")
        else:
            raise ValueError(f"dst_type {dst_type} is not a valid type or a single-element list/tuple/set of types")

    if isinstance(obj, dst_type) and not elem_type:
        # No conversion
        return obj
    elif dst_type == str:
        #if type(obj) == str: HANDLED ABOVE
        #    return obj  
        #el
        if separator and isinstance(obj, compound_types):
            # Ensure string, for join()
            obj = [str(v) for v in obj]
            return separator.join(obj)
        else:
            return str(obj)
    elif dst_type in compound_types:

        if type(obj) == str:
            obj = obj.strip()
            if separator:
                if type(separator) == str:
                    if len(separator) == 1:
                        obj = obj.split(separator)
                    else:
                        obj = re.split(re.compile(separator), obj)
                else:
                    obj = re.split(separator, obj)
                # obj =  obj.split(separator)
            else:
                return dst_type(obj)  # eg. ["Hello, world!"]

        if isinstance(obj, compound_types):
            if elem_type:
                # Convert elements
                return dst_type([elem_type(v) for v in obj])
            else:
                # Copy elements directly 
                return dst_type(obj)

        # Finally, try direct type cast  
        return dst_type(obj)
    elif dst_type in (int,float):
        return dst_type(obj)
    else:
        # TODO bool, dict, set, etc. or custom types? Or just return obj as is for unsupported dst_type?
        # warning or error?
        return obj
