# Openbox's config system. Please use the defined functions instead of
# accessing the internal data structures directly, for the sake of us all.

def add(modulename, name, friendlyname, description, type, default,
        **keywords):
    """Add a variable to the configuration system.

    Add a variable to the configuration system for a module.
    modulename - The name of the module, e.g. 'focus'
    name - The name of the variable, e.g. 'my_variable'
    friendlyname - The user-friendly name of the variable, e.g. 'My Variable'
    description - The detailed destription of the variable, e.g. 'Does Things'
    type - The type of the variable, one of:
             - 'boolean'
             - 'enum'
             - 'integer'
             - 'string'
             - 'file'
             - 'function'
             - 'object'
    default - The default value for the variable, e.g. 300
    keywords - Extra keyword=value pairs to further define the variable. These
               can be:
                 - For 'enum' types:
                     - options : A list of possible options for the variable.
                                 This *must* be set for all enum variables.
                 - For 'integer' types:
                     - min : The minimum value for the variable.
                     - max : The maximum value for the variable.
    """
    modulename = str(modulename).lower()
    name = str(name).lower()
    friendlyname = str(friendlyname)
    description = str(description)
    type = str(type).lower()

    # make sure the sub-dicts exist
    try:
        _settings[modulename]
        try:
            _settings[modulename][name]
        except KeyError:
            _settings[modulename][name] = {}
    except KeyError:
        _settings[modulename] = {}
        _settings[modulename][name] = {}

    # add the keywords first as they are used for the tests in set()
    for key,value in zip(keywords.keys(), keywords.values()):
        _settings[modulename][name][key] = value

    _settings[modulename][name]['name'] = friendlyname
    _settings[modulename][name]['description'] = description
    _settings[modulename][name]['type'] = type
    _settings[modulename][name]['default'] = default

    # put it through the tests
    try:
        set(modulename, name, default)
    except:
        del _settings[modulename][name]
        import sys
        raise sys.exc_info()[0], sys.exc_info()[1] # re-raise it

def set(modulename, name, value):
    """Set a variable's value.

    Sets the value for a variable of the specified module.
    modulename - The name of the module, e.g. 'focus'
    name - The name of the variable, e.g. 'my_variable'
    value - The new value for the variable.
    """
    modulename = str(modulename).lower()
    name = str(name).lower()

    # proper value checking for 'boolean's
    if _settings[modulename][name]['type'] == 'boolean':
        if not (value == 0 or value == 1):
            raise ValueError, 'Attempted to set ' + name + ' to a value of '+\
                  str(value) + ' but boolean variables can only contain 0 or'+\
                  ' 1.'

    # proper value checking for 'enum's
    elif _settings[modulename][name]['type'] == 'enum':
        options = _settings[modulename][name]['options']
        if not value in options:
            raise ValueError, 'Attempted to set ' + name + ' to a value of '+\
                  str(value) + ' but this is not one of the possible values '+\
                  'for this enum variable. Possible values are: ' +\
                  str(options) + "."

    # min/max checking for 'integer's
    elif _settings[modulename][name]['type'] == 'integer':
        try:
            min = _settings[modulename][name]['min']
            if value < min:
                raise ValueError, 'Attempted to set ' + name + ' to a value '+\
                      ' of ' + str(value) + ' but it has a minimum value ' +\
                      ' of ' + str(min) + '.'
        except KeyError: pass
        try:
            max = _settings[modulename][name]['max']
            if value > max:
                raise ValueError, 'Attempted to set ' + name + ' to a value '+\
                      ' of ' + str(value) + ' but it has a maximum value ' +\
                      ' of ' + str(min) + '.'
        except KeyError: pass
    
    _settings[modulename][name]['value'] = value

def reset(modulename, name):
    """Reset a variable to its default value.

    Resets the value for a variable in the specified module back to its
    original (default) value.
    modulename - The name of the module, e.g. 'focus'
    name - The name of the variable, e.g. 'my_variable'
    """
    modulename = str(modulename).lower()
    name = str(name).lower()
    _settings[modulename][name]['value'] = \
                                 _settings[modulename][name]['default']

def get(modulename, name):
    """Returns the value of a variable.

    Returns the current value for a variable in the specified module.
    modulename - The name of the module, e.g. 'focus'
    name - The name of the variable, e.g. 'my variable'
    """
    modulename = str(modulename).lower()
    name = str(name).lower()
    return _settings[modulename][name]['value']

#---------------------------- Internals ---------------------------

"""The main configuration dictionary, which holds sub-dictionaries for each
   module.

   The format for entries in here like this (for a string):
   _settings['modulename']['varname']['name'] = 'Text Label'
   _settings['modulename']['varname']['description'] = 'Does this'
   _settings['modulename']['varname']['type'] = 'string'
   _settings['modulename']['varname']['default'] = 'Foo'
   _settings['modulename']['varname']['value'] = 'Foo'
                             # 'value' should always be initialized to the same
                             # value as the 'default' field!

   Here's an example of an enum:
   _settings['modulename']['varname']['name'] = 'My Enum Variable'
   _settings['modulename']['varname']['description'] = 'Does Enum-like things.'
   _settings['modulename']['varname']['type'] = 'enum'
   _settings['modulename']['varname']['default'] = \
     _settings['modulename']['varname']['value'] = [ 'Blue', 'Green', 'Pink' ]

   And Here's an example of an integer with bounds:
   _settings['modulename']['varname']['name'] = 'A Bounded Integer'
   _settings['modulename']['varname']['description'] = 'A fierce party animal!'
   _settings['modulename']['varname']['type'] = 'integer'
   _settings['modulename']['varname']['default'] = \
     _settings['modulename']['varname']['value'] = 0
   _settings['modulename']['varname']['min'] = 0
   _settings['modulename']['varname']['max'] = 49

   Hopefully you get the idea.
   """
_settings = {}

"""Valid values for a variable's type."""
_types = [ 'boolean', # Boolean types can only hold a value of 0 or 1.
           
           'enum',    # Enum types hold a value from a list of possible values.
                      # An 'options' field *must* be provided for enums,
                      # containing a list of possible values for the variable.

           'integer', # Integer types hold a single number, as well as a 'min'
                      # and 'max' property.
                      # If the 'min' or 'max' is ignore then bounds checking
                      # will not be performed in that direction.

           'string',  # String types hold a text string.

           'file',    # File types hold a file object.

           'function',# Function types hold any callable object.

           'object'   # Object types can hold any python object.
           ];

print "Loaded config.py"
