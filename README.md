Chamesle Source Linux
=====================

A kernel that boots into vi. Nothing else.

vi is Bill Joy's ex 1.1 from the 1BSD tape (1977), the first vi, moved to
today's C. It runs as PID 1, with tcc built in: wq compiles the file and runs
it in vi's place. If it doesn't compile, you stay in vi and see why.

The libc is c.h, one file. There is no init, no shell and no tools. You
write them.

fbttywm comes with it, as source in /usr/src. Write a shell, the commands,
wget and tar, fetch a compiler with them, then build it:

	gcc -static -O2 /usr/src/fbttywm.c -o /fbttywm

The easy distro is Chamesle Linux: https://chamesle.org
Nobody should use this one for anything real.


Run it
------
Needs gcc, python3 and qemu.

	git clone https://github.com/TinyCC/tinycc third/tinycc
	git -C third/tinycc checkout e23dabed154a8495f3ac1ff3cc9d04fa1c207893
	./run.sh

The kernel comes from a Chamesle Linux build next to this one, or say which:

	KERNEL=/boot/vmlinuz-... MODULES=/usr/lib/modules/... ./run.sh

	./run.sh serial      in this terminal
	./run.sh disk        with a 256 MB disk to install on
	./run.sh installed   boots that disk through UEFI


Using vi 1.1
------------
It's older than the vi you know. Visual mode has no :, so q drops to ex's
: prompt, and there wq compiles and runs.

	i ... ESC    write
	q            to ex
	wq           build and run


Files
-----
	c.h              the libc
	vi/              ex 1.1, and sys.c, the glue to tcc and the kernel
	vi/port.py       turned the 1977 C into C compilers read today, once
	mkinitramfs.py   the initramfs, without cpio
	fbttywm/         windows, once you've earned gcc: /usr/src/fbttywm.c
	third/           ex 1.1 and libX as they are on the tape
	tests/           vi in a pty on the host, and c.h's tests
