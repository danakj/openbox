#include "actions.hh"

Action::Action(enum ActionType type, KeyCode keycode, int modifierMask):
  _type(type), _keycode(keycode), _modifierMask(modifierMask)
{ }
