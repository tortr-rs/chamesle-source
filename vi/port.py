#!/usr/bin/env python3
# port.py: makes vi/ from Bill Joy's ex 1.1 in third/ex-1.1, converting what 1977 C wrote differently.
# only the mechanical part: everything else in vi/ is changed by hand, and git shows each change against this.
#   - "x =+ y" was "x += y" (and =- =* =/ =% =& =| =^ =<< =>>): in 1977 the operator went after the =.
#     a space in between ("x = -1") meant a plain assignment, so that stays
#   - initializers had no "=": "int lines 24;", "char name[] "text";", "struct x y[] { ... };"
#   - CTRL(x) put its argument inside a character constant, which only the old preprocessor did
#   - STATIC was an empty #define, "extern int x 1;" a definition with a value
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '../third/ex-1.1')


def code_spans(text):
    """yields (is_code, piece): strings, character constants and comments are not code"""
    i, start, n = 0, 0, len(text)
    while i < n:
        c = text[i]
        if c in '"\'' or text.startswith('/*', i):
            if start < i:
                yield True, text[start:i]
            if c in '"\'':
                j = i + 1
                while j < n and text[j] != c:
                    j += 2 if text[j] == '\\' else 1
                j += 1
            else:
                j = text.find('*/', i + 2)
                j = n if j < 0 else j + 2
            yield False, text[i:j]
            i = start = j
        else:
            i += 1
    if start < n:
        yield True, text[start:]


OLD_OP = re.compile(r'(?<![=!<>+\-*/%&|^])=(<<|>>|[+\-*/%&|^])(?!=)')


def compound(code):
    return OLD_OP.sub(lambda m: m.group(1) + '=', code)


CTRL = re.compile(r'\bCTRL\((\w)\)')
DECL_INIT = re.compile(r'^((?:extern\s+|STATIC\s+)?(?:static\s+)?(?:unsigned\s+)?(?:char|int|long|struct\s+\w+)\s+\**\w+(?:\[\w*\])*)'
                       r'(\s+)(-?\d|\'|"|\{|[A-Za-z_]\w*\s*[-+])', re.M)


def convert(text):
    out = []
    for is_code, piece in code_spans(text):
        if is_code:
            piece = compound(piece)
            piece = CTRL.sub(r"CTRL('\1')", piece)
        out.append(piece)
    text = ''.join(out)
    # initializers: only at the left margin, where the top-level declarations are
    text = DECL_INIT.sub(lambda m: m.group(1).replace('extern', '').strip() + ' = ' + m.group(3), text)
    text = text.replace("#define\tCTRL('c')\t('c' & 037)", '#define\tCTRL(c)\t((c) & 037)')
    # FS, control-\\: the old compiler read '\\\\' as two backslashes, and & 037 kept the low one
    text = text.replace('CTRL(\\\\\\\\)', "CTRL('\\\\')")
    return text


def main():
    for name in sorted(os.listdir(SRC)):
        if not name.endswith(('.c', '.h')):
            continue
        with open(os.path.join(SRC, name), encoding='latin1') as f:
            text = f.read()
        with open(os.path.join(HERE, name), 'w', encoding='latin1') as f:
            f.write(convert(text))
    print('converted', sys.argv[0])


main()
