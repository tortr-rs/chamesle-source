#!/bin/sh
# builds PID 1 and the initramfs, then boots them in qemu
#   ./run.sh           in a window
#   ./run.sh serial    in this terminal (ctrl-a x quits qemu)
# for now the kernel is the one Chamesle Linux ships; KERNEL= picks another
set -eu
cd "$(dirname "$0")"
KERNEL=${KERNEL:-$(ls ../chamesle/work/kernel/build/k/boot/vmlinuz-* | head -n1)}
QEMU=${QEMU:-qemu-system-x86_64}
mkdir -p out
cc -static -nostdlib -ffreestanding -fno-stack-protector -fno-builtin -no-pie -O2 -Wall -o out/init init.c
python3 mkinitramfs.py out/initramfs.cpio out/init
if [ "${1:-}" = serial ]; then
    exec $QEMU -enable-kvm -m 256 -kernel "$KERNEL" -initrd out/initramfs.cpio -nographic -append "console=ttyS0 quiet"
fi
exec $QEMU -enable-kvm -m 256 -kernel "$KERNEL" -initrd out/initramfs.cpio -append "quiet"
