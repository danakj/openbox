#! /usr/bin/python

import sys

data = []

def out(str):
    sys.stderr.write(str)
    sys.stderr.flush()

def read_bool():
    while True:
        inp = sys.stdin.readline(1).strip()
        if inp == 'y' or inp == '': return True
        if inp == 'n': return False

def getkeyval(line):
    key = line[:line.find(':')].strip()
    value = line[line.find(':') + 1:].strip()
    if not (key and value):
        key = value = None
    return key, value

def simple_replace(data):
    for i in range(len(data)):
        l = data[i]
        key, value = getkeyval(l)
        if key and value:
            pairs = {}
            pairs['window.focus.font'] = 'window.label.focus.font'
            pairs['window.unfocus.font'] = 'window.label.unfocus.font'
            pairs['window.justify'] = 'window.label.justify'
            pairs['menu.frame.disableColor'] = 'menu.disabled.textColor'
            pairs['style.']  = 'info.'
            pairs['menu.frame'] = 'menu.items'
            pairs['menu.hilite'] = 'menu.selected'
            pairs['.picColor'] = '.imageColor'

            for k in pairs.keys():
                if key.find(k) != -1:
                    newl = l.replace(k, pairs[k])
                    out('Updating "' + key +
                        '" to "' + key.replace(k, pairs[k]) + '"\n')
                    data[i] = newl
                    break

def remove(data):
    i = 0
    n = len(data)
    while i < n:
        l = data[i]
        key, value = getkeyval(l)
        if key and value:
            invalid = []
            invalid.append('toolbar')
            invalid.append('rootCommand')
            invalid.append('menu.frame.justify')
            for inv in invalid:
                if key.find(inv) != -1:
                    out(key + ' is no longer supported.\nRemove (Y/n)? ')
                    if read_bool():
                        out('Removing "' + key + '"\n')
                        data.pop(i)
                        i -= 1
                        n -= 1
                    break
        i += 1

def pressed(data):
    i = 0
    n = len(data)
    while i < n:
        l = data[i]
        key, value = getkeyval(l)
        if key and value:
            if key == 'window.button.pressed':
                out('The window.button.pressed option has been replaced by ' +
                    'window.button.pressed.focus and ' +
                    'window.button.pressed.unfocus.\nUpdate (Y/n)? ')
                if read_bool():
                    out('Removing "window.button.pressed"\n')
                    data.pop(i)
                    out('Adding "window.button.pressed.unfocus"\n')
                    data.insert(i, l.replace('window.button.pressed',
                                             'window.button.pressed.unfocus'))
                    out('Adding "window.button.pressed.focus"\n')
                    data.insert(i, l.replace('window.button.pressed',
                                             'window.button.pressed.focus'))
                    i += 1
                    n += 1
                break
        i += 1

def fonts(data):
    for l in data:
        key, value = getkeyval(l)
        if key and value:
            if key == 'window.font':
                out('You appear to specify fonts using the old X fonts ' +
                    'syntax.\nShall I remove all fonts from the theme (Y/n)? ')
                if not read_bool():
                    return
    i = 0
    n = len(data)
    while i < n:
        l = data[i]
        key, value = getkeyval(l)
        if key and value:
            if key.find('font') != -1:
                out('Removing "' + key + '"\n')
                data.pop(i)
                i -= 1
                n -= 1
        i += 1

















def usage():
    print 'Usage: ' + sys.argv[0] + ' /path/to/themerc > newthemerc'
    print
    sys.exit()

try:
    file = open(sys.argv[1])
except IndexError:
    usage()
except IOError:
    print 'Unable to open file "' + sys.argv[1] + '"'
    print
    usage()

data = file.readlines()
for i in range(len(data)):
    data[i] = data[i].strip()

#simple_replace(data)
#remove(data)
#pressed(data)
fonts(data)

for l in data:
    print l
