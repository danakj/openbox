#ifndef __actions_hh
#define __actions_hh
#include <list>

extern "C" {
#include <X11/X.h>
}
class Action {
public:
    // xOr: this is crap.
  enum ActionType {
    noaction = 0,
    execute,
    iconify,
    raiseWindow,
    lowerWindow,
    closeWindow,
    shade,
    moveWindowUp,
    moveWindowDown,
    moveWindowLeft,
    moveWindowRight,

    nextWindow,
    prevWindow,
    nextWindowOnAllDesktops,
    prevWindowOnAllDesktops,

    nextWindowOfClass,
    prevWindowOfClass,

    changeDesktop,
    nextDesktop,
    prevDesktop,

    // these are openbox extensions
    showRootMenu,
    showWorkspaceMenu,

    stringChain, 
    keyChain,
    numberChain,

    cancel,
  };

private:
  enum Action::ActionType _type;
  const KeyCode _keycode;
  const int _modifierMask;
  
public:
  inline enum ActionType type() const { return _type;}
  inline const KeyCode keycode() const { return _keycode; }
  inline const int modifierMask() const { return _modifierMask; }

  Action::Action(enum ActionType type, KeyCode keycode, int modifierMask);
};
  
typedef list<Action> ActionList;

#endif
