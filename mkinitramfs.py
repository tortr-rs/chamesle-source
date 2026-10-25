#!/usr/bin/env python3
# mkinitramfs.py OUT.cpio INIT [NAME=FILE... DIR/...]: the initramfs the kernel unpacks as / (cpio "newc", no compression).
# NAME=FILE puts FILE in as NAME, making the directories it is in. DIR/ makes an empty directory.
# INIT becomes /init. /dev/console is in it because the kernel gives PID 1 its input and output through that node
import os
import stat
import sys


def entry(name, mode, data=b'', rdev=(0, 0)):
    name = name.encode() + b'\0'
    head = '070701' + ''.join('%08X' % v for v in (
        0, mode, 0, 0, 1, 0, len(data), 0, 0, rdev[0], rdev[1], len(name), 0))
    out = head.encode() + name
    out += b'\0' * (-len(out) % 4)
    out += data + b'\0' * (-len(data) % 4)
    return out


def main():
    out, init = sys.argv[1], sys.argv[2]
    parts = [entry('dev', stat.S_IFDIR | 0o755),
             entry('dev/console', stat.S_IFCHR | 0o600, rdev=(5, 1)),
             entry('init', stat.S_IFREG | 0o755, open(init, 'rb').read())]
    dirs = {'dev'}

    def directory(d):
        if d and d not in dirs:
            directory(os.path.dirname(d))
            dirs.add(d)
            parts.append(entry(d, stat.S_IFDIR | 0o755))

    for arg in sys.argv[3:]:
        if arg.endswith('/'):
            directory(arg.rstrip('/'))
            continue
        name, path = arg.split('=', 1)
        directory(os.path.dirname(name))
        mode = 0o755 if os.access(path, os.X_OK) else 0o644
        parts.append(entry(name, stat.S_IFREG | mode, open(path, 'rb').read()))
    parts.append(entry('TRAILER!!!', 0))
    with open(out, 'wb') as f:
        f.write(b''.join(parts))


main()
