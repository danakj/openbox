#ifndef __plugin_keyboard_action_h
#define __plugin_keyboard_action_h

#include "../../kernel/action.h"

typedef enum {
    DataType_Bool,
    DataType_Int,
    DataType_Uint,
    DataType_String
} KeyActionDataType;

typedef union {
    gboolean b;
    int i;
    guint u;
    char *s;
} KeyActionData;

typedef struct {
    Action action;
    KeyActionDataType type[2];
    KeyActionData data[2];
} KeyAction;

void keyaction_set_none(KeyAction *a, guint index);
void keyaction_set_bool(KeyAction *a, guint index, gboolean bool);
void keyaction_set_int(KeyAction *a, guint index, int i);
void keyaction_set_uint(KeyAction *a, guint index, guint uint);
void keyaction_set_string(KeyAction *a, guint index, char *string);

void keyaction_free(KeyAction *a);

void keyaction_do(KeyAction *a, Client *c);

#endif
