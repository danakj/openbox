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

  NUM_ACTIONS
};

