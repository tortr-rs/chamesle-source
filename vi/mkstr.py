#!/usr/bin/env python3
# mkstr.py MESGFILE PREFIX FILE...: Bill Joy's mkstr (1977), the step of ex's build that takes the error messages out
# of the program. every error("...") in each FILE becomes error(N), N where the message starts in MESGFILE, and the
# file is written again as PREFIX + its name. ex's error() reads the message back from /usr/lib/ex1.1strings.
# in MESGFILE each message ends with a NUL and a newline, as in the ex1.1strings on the 1BSD tape
import os
import sys

ESCAPES = {'b': '\b', 't': '\t', 'r': '\r', 'n': '\n', 'f': '\f', '\\': '\\', '"': '"', "'": "'"}


def message(text, i):
    """the string starting after the opening quote at text[i]: (its characters, where it ends)"""
    out = []
    while text[i] != '"':
        c = text[i]
        if c == '\\':
            i += 1
            c = text[i]
            if c == '\n':
                i += 1
                continue
            if c in '01234567':
                j = i
                while j < i + 3 and text[j] in '01234567':
                    j += 1
                out.append(chr(int(text[i:j], 8)))
                i = j
                continue
            c = ESCAPES.get(c, c)
        out.append(c)
        i += 1
    return ''.join(out), i + 1


def main():
    mesgfile, prefix, files = sys.argv[1], sys.argv[2], sys.argv[3:]
    strings = bytearray()
    for path in files:
        with open(path, encoding='latin1') as f:
            text = f.read()
        out, i = [], 0
        while True:
            j = text.find('error("', i)
            if j < 0:
                out.append(text[i:])
                break
            out.append(text[i:j] + 'error(')
            msg, i = message(text, j + len('error("'))
            out.append(str(len(strings)))
            strings += msg.encode('latin1') + b'\0\n'
        with open(prefix + os.path.basename(path), 'w', encoding='latin1') as f:
            f.write(''.join(out))
    with open(mesgfile, 'wb') as f:
        f.write(strings)


main()
