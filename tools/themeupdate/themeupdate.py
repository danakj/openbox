#! /usr/bin/python

import sys

data = []
valid = True

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

def find_key(data, keysubstr, exact = False):
    i = 0
    n = len(data)
    while i < n:
        l = data[i]
        key, value = getkeyval(l)
        if key and value:
            if (exact and key == keysubstr) or \
               (not exact and key.find(keysubstr) != -1):
                return i, key, value
        i += 1
    return -1, None, None

def simple_replace(data):
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
        while 1:
            i, key, nul = find_key(data, k);
            if i >= 0:
                newl = data[i].replace(k, pairs[k])
                out('Updating "' + key +
                    '" to "' + key.replace(k, pairs[k]) + '"\n')
                data[i] = newl
            else:
                break

def remove(data):
    invalid = []
    invalid.append('toolbar')
    invalid.append('rootCommand')
    invalid.append('menu.frame.justify')
    for inv in invalid:
        while 1:
            i, key, nul = find_key(data, inv)
            if i >= 0:
                out(key + ' is no longer supported.\nRemove (Y/n)? ')
                if read_bool():
                    out('Removing "' + key + '"\n')
                    data.pop(i)
            else:
                break

def pressed(data):
    i, nul, nul = find_key(data, 'window.button.pressed', True)
    if i >= 0:
        out('The window.button.pressed option has been replaced by ' +
            'window.button.pressed.focus and ' +
            'window.button.pressed.unfocus.\nUpdate (Y/n)? ')
        if read_bool():
            l = data[i]
            out('Removing "window.button.pressed"\n')
            data.pop(i)
            out('Adding "window.button.pressed.unfocus"\n')
            data.insert(i, l.replace('window.button.pressed',
                                             'window.button.pressed.unfocus'))
            out('Adding "window.button.pressed.focus"\n')
            data.insert(i, l.replace('window.button.pressed',
                                     'window.button.pressed.focus'))

def x_fonts(data):
    i, nul, nul = find_key(data, 'window.font')
    if i >= 0:
        out('You appear to specify fonts using the old X fonts ' +
            'syntax.\nShall I remove all fonts from the theme (Y/n)? ')
        if not read_bool():
            return
    else: return
    while 1:
        i, key = key_find(data, '.font')
        if i < 0:
            break
        out('Removing "' + key + '"\n')
        data.pop(i)

def xft_fonts(data):
    i, nul, nul = find_key(data, '.xft.')
    if i >= 0:
        out('You appear to specify fonts using the old Xft fonts ' +
            'syntax.\nShall I update these to the new syntax (Y/n)? ')
        if not read_bool():
            return
    else: return
    fonts = {}
    fonts['window'] = 'window.label.focus.font'
    fonts['menu.items'] = 'menu.items.font'
    fonts['menu.title'] = 'menu.title.font'
    for f in fonts.keys():
        li, nul, flags = find_key(data, f + '.xft.flags')
        if li < 0:
            li, nul, flags = find_key(data, '*.xft.flags')
        else:
            out('Removing ' + f + '.xft.flags\n')
            data.pop(li)
        oi, nul, offset = find_key(data, f + '.xft.shadow.offset')
        if oi < 0:
            oi, nul, offset = find_key(data, '*.xft.shadow.offset')
        else:
            out('Removing ' + f + '.xft.shadow.offset\n')
            data.pop(oi)
        ti, nul, tint = find_key(data, f + '.xft.shadow.tint')
        if ti < 0:
            ti, nul, tint = find_key(data, '*.xft.shadow.tint')
        else:
            out('Removing ' + f + '.xft.shadow.tint\n')
            data.pop(ti)
        fi, nul, face = find_key(data, f + '.xft.font')
        if fi < 0:
            fi, nul, face = find_key(data, '*.xft.font')
            if fi >= 0: fi = len(data) - 1
        else:
            out('Removing ' + f + '.xft.font\n')
            data.pop(fi)

        if fi >= 0:
            s = face
            if li >= 0:
                if flags.find('bold'):
                    s = s + ':bold'
                if flags.find('shadow'):
                    s = s + ':shadow=y'
            if oi >= 0:
                s = s + ':shadowoffset=' + offset
            if ti >= 0:
                s = s + ':shadowtint=' + tint
        out('Adding ' + fonts[f] + '\n')
        data.insert(fi, fonts[f] + ': ' + s)

    for stars in ('*.xft.flags', '*.xft.shadow.offset' ,
                  '*.xft.shadow.tint', '*.xft.font'):
        i, key, nul = find_key(data, stars)
        if i >= 0:
            out('Removing ' + key + '\n')
            data.pop(i)

def warn_missing(data):
    need = ('window.button.hover.focus',  'window.button.hover.unfocus',
            'menuOverlap')
    for n in need:
        i, nul, nul = find_key(data, n)
        if i < 0:
            out('The ' + n + ' value was not found in the theme, but it '
                'can optionally be set.\n')

def err_missing(data):
    need = ('window.button.disabled.focus',  'window.button.disabled.unfocus',
            'window.frame.focusColor', 'window.frame.unfocusColor')
    for n in need:
        i, nul, nul = find_key(data, n)
        if i < 0:
            out('*** ERROR *** The ' + n + ' value was not found in the '
                'theme, but it is required to be set.\n')
            global valid
            valid = False


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

simple_replace(data)
remove(data)
pressed(data)
x_fonts(data)
xft_fonts(data)
warn_missing(data)
err_missing(data)

for l in data:
    print l

sys.exit(not valid)
