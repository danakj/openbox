#include "actions.hh"
#include "keytree.hh"

#include <string>

class parser {
public:
    parser(keytree *);
    ~parser();

    void parse(std::string);

    void setKey(std::string key)
    {  _key = key; }

    void setArgumentNum(std::string arg)
    { _arg = arg; }

    void setArgumentNegNum(std::string arg)
    { _arg = "-" + arg; }

    void setArgumentStr(std::string arg)
    { _arg = arg.substr(1, arg.size() - 2); }

    void setAction(std::string);
    void addModifier(std::string);
    void endAction();
    void startChain();
    void setChainBinding();
    void endChain();

private:
    void reset();

    keytree *_kt;
    unsigned int _mask;
    Action::ActionType _action;
    std::string _key;
    std::string _arg;
};
