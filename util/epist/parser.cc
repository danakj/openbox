extern "C" {
#include <stdio.h>
}

#include "parser.hh"

#include <string>

using std::string;


parser::parser(keytree *kt)
    : _kt(kt), _mask(0), _action(Action::noaction), _key(""), _arg("") 
{
}

parser::~parser()
{
    // nothing to see here. move along.
}

void parser::parse(string rc_file)
{
    extern int yyparse(void *);
    extern FILE *yyin;

    yyin = fopen(rc_file.c_str(), "r");

    yyparse(this);

    fclose(yyin);
    _kt->reset();
}

void parser::setAction(string act)
{
    struct {
        string str;
        Action::ActionType act;
    }
    actions[] = {
        { "noaction", Action::noaction },
        { "execute", Action::execute },
        { "iconify", Action::iconify },
        { "raise", Action::raise },
        { "lower", Action::lower },
        { "close", Action::close },
        { "toggleshade", Action::toggleshade },
        { "toggleomnipresent", Action::toggleomnipresent },
        { "moveWindowUp", Action::moveWindowUp },
        { "moveWindowDown", Action::moveWindowDown },
        { "moveWindowLeft", Action::moveWindowLeft },
        { "moveWindowRight", Action::moveWindowRight },
        { "resizeWindowWidth", Action::resizeWindowWidth },
        { "resizeWindowHeight", Action::resizeWindowHeight },
        { "toggleMaximizeFull", Action::toggleMaximizeFull },
        { "toggleMaximizeVertical", Action::toggleMaximizeVertical },
        { "toggleMaximizeHorizontal", Action::toggleMaximizeHorizontal },
        { "sendToWorkspace", Action::sendToWorkspace },
        { "nextWindow", Action::nextWindow },
        { "prevWindow", Action::prevWindow },
        { "nextWindowOnAllWorkspaces", Action::nextWindowOnAllWorkspaces },
        { "prevWindowOnAllWorkspaces", Action::prevWindowOnAllWorkspaces },
        { "nextWindowOnAllScreens", Action::nextWindowOnAllScreens },
        { "prevWindowOnAllScreens", Action::prevWindowOnAllScreens },
        { "nextWindowOfClass", Action::nextWindowOfClass },
        { "prevWindowOfClass", Action::prevWindowOfClass },
        { "nextWindowOfClassOnAllWorkspaces", Action::nextWindowOfClassOnAllWorkspaces },
        { "prevWindowOfClassOnAllWorkspaces", Action::prevWindowOfClassOnAllWorkspaces },
        { "changeWorkspace", Action::changeWorkspace },
        { "nextWorkspace", Action::nextWorkspace },
        { "prevWorkspace", Action::prevWorkspace },
        { "nextScreen", Action::nextScreen },
        { "prevScreen", Action::prevScreen },
        { "showRootMenu", Action::showRootMenu },
        { "showWorkspaceMenu", Action::showWorkspaceMenu },
        { "stringChain", Action::stringChain },
        { "keyChain", Action::keyChain },
        { "numberChain", Action::numberChain },
        { "cancel", Action::cancel },
        { "", Action::noaction }
    };

    bool found = false;

    for (int i = 0; actions[i].str != ""; ++i) {
        if (actions[i].str == act) {
            _action = actions[i].act;
            found = true;
        }
    }

    if (!found)
        _action = Action::noaction;
}

void parser::addModifier(string mod)
{
    struct {
        string str;
        unsigned int mask;
    }
    modifiers[] = {
        { "Mod1", Mod1Mask },
        { "Mod2", Mod2Mask },
        { "Mod3", Mod3Mask },
        { "Control", ControlMask },
        { "Shift", ShiftMask },
        { "", 0 }
    };

    for (int i = 0; modifiers[i].str != ""; ++i) {
        if (modifiers[i].str == mod)
            _mask |= modifiers[i].mask;
    }
}

void parser::endAction()
{
    _kt->addAction(_action, _mask, _key, _arg);
    reset();
}

void parser::startChain()
{
    _kt->advanceOnNewNode();
    setChainBinding();
    reset();
}

void parser::endChain()
{
    _kt->retract();
    reset();
}

void parser::setChainBinding()
{
    if (_mask != 0 && _key != "") {
        _kt->setCurrentNodeProps(Action::noaction, _mask, _key, "");
        reset();
    }
}

void parser::reset()
{
    _mask = 0;
    _action = Action::noaction;
    _key = "";
    _arg = "";
}
