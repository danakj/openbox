#ifndef __config_h
#define __config_h

#include <glib.h>

typedef enum {
    Config_String,
    Config_Integer
} ConfigValueType;

typedef union {
    char *string;
    int integer;
} ConfigValue;

typedef struct {
    char *name;
    ConfigValueType type;
    ConfigValue value;
} ConfigEntry;

typedef struct {
    char *name;
    ConfigValueType type;
    /* if it is a string type optionally provide a list of valid strings */
    gboolean hasList;
    GSList *values;
} ConfigDefEntry;

void config_startup();
void config_shutdown();

/* Set a config variable's value. The variable must have already been defined
   with a call to config_def_set */
gboolean config_set(char *name, ConfigValueType type, ConfigValue value);

/* Get a config variable's value. Returns FALSE if the value has not been
   set. */
gboolean config_get(char *name, ConfigValueType type, ConfigValue *value);

/* Create a new config definition to add to the config system */
ConfigDefEntry *config_def_new(char *name, ConfigValueType type);

/* Add a value to a String type config definition */
gboolean config_def_add_value(ConfigDefEntry *entry, char *value);

/* Sets up the definition in the config system, Don't free or touch the entry
   after setting it with this. It is invalidated even if the function returns
   FALSE. */
gboolean config_def_set(ConfigDefEntry *entry);

void config_parse();

#endif
