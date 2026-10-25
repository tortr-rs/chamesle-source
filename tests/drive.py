#!/usr/bin/env python3
# drive.py KEYS...: runs the built vi (out/init) here, in a pseudo-terminal, types each KEYS argument (waiting a
# little after each), and prints the screen. a test without qemu: a private mount namespace gives it /etc/ttycap,
# argv[0] "aex" makes it read its messages from ./ex1.1strings, as ex did when it was called that.
# Screen below is a small terminal, just enough for the screen to be read back
import os, pty, sys, time, select


class Screen:
    """just what vi's ttycap entry uses: ESC [ H, ESC [ J, ESC [ K, ESC [ l ; c H, ESC [ C, ESC [ A, CR, LF, BS"""
    def __init__(self, cols, lines):
        self.cols, self.lines = cols, lines
        self.rows = [[' '] * cols for _ in range(lines)]
        self.y = self.x = 0
        self.esc = None

    def scroll(self):
        self.rows.pop(0)
        self.rows.append([' '] * self.cols)
        self.y = self.lines - 1

    def feed(self, data):
        for b in data:
            c = chr(b)
            if self.esc is not None:
                self.esc += c
                if self.esc == '[' or (self.esc.startswith('[') and not c.isalpha()):
                    continue
                self.csi(self.esc)
                self.esc = None
            elif c == '\x1b':
                self.esc = ''
            elif c == '\r':
                self.x = 0
            elif c == '\n':
                self.y += 1
                if self.y >= self.lines:
                    self.scroll()
            elif c == '\b':
                self.x = max(0, self.x - 1)
            elif c == '\t':
                self.x = min(self.cols - 1, (self.x + 8) & ~7)
            elif 32 <= b < 127:
                if self.x >= self.cols:
                    self.x = 0
                    self.y += 1
                    if self.y >= self.lines:
                        self.scroll()
                self.rows[self.y][self.x] = c
                self.x += 1

    def csi(self, seq):
        if not seq.startswith('['):
            return
        cmd, args = seq[-1], seq[1:-1]
        n = [int(a) if a else 0 for a in args.split(';')] if args else []
        if cmd == 'H':
            self.y = (n[0] - 1 if n else 0)
            self.x = (n[1] - 1 if len(n) > 1 else 0)
        elif cmd == 'J':
            for r in range(self.y, self.lines):
                for col in range(self.x if r == self.y else 0, self.cols):
                    self.rows[r][col] = ' '
        elif cmd == 'K':
            for col in range(self.x, self.cols):
                self.rows[self.y][col] = ' '
        elif cmd == 'C':
            self.x = min(self.cols - 1, self.x + (n[0] if n else 1))
        elif cmd == 'A':
            self.y = max(0, self.y - (n[0] if n else 1))

    @property
    def display(self):
        return [''.join(r) for r in self.rows]

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
work = os.path.join(ROOT, 'out/drive')
os.makedirs(work, exist_ok=True)
os.system('cp %s/out/init %s/out/ex1.1strings %s/' % (ROOT, ROOT, work))
script = ('mount -t tmpfs t /etc && cp %s/vi/ttycap /etc/ttycap && mount -t tmpfs t /tmp && '
          'mount -t tmpfs t /usr/include && cp %s/c.h /usr/include/ && cd %s && exec -a aex ./init' % (ROOT, ROOT, work))
screen = Screen(80, 25)
stream = screen
pid, fd = pty.fork()
if pid == 0:
    os.environ.clear()
    os.environ.update({'TERM': 'linux', 'PATH': '/usr/bin:/bin'})
    os.execvp('unshare', ['unshare', '-rm', 'bash', '-c', script])


def pump(t):
    end = time.time() + t
    while time.time() < end:
        r, _, _ = select.select([fd], [], [], 0.05)
        if r:
            try:
                stream.feed(os.read(fd, 4096))
            except OSError:
                return


pump(1.5)
for keys in sys.argv[1:]:
    # \n in KEYS is the Enter key, which sends a carriage return, as on a real terminal
    for ch in keys.encode().decode('unicode_escape').replace('\n', '\r').encode('latin1'):
        os.write(fd, bytes([ch]))
        pump(0.02)
    pump(0.6)
pump(0.5)
for line in screen.display:
    print(line.rstrip())
done, status = os.waitpid(pid, os.WNOHANG)
if done:
    print('[exited: status %d]' % status)
