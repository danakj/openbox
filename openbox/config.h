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

void config_startup();
void config_shutdown();

gboolean config_set(char *name, ConfigValueType type, ConfigValue value);

void config_parse();

#endif
