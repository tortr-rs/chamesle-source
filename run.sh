#!/bin/sh
# builds vi, the PID 1, and the initramfs, then boots them in qemu
#   ./run.sh           in a window
#   ./run.sh serial    in this terminal (ctrl-a x quits qemu)
#   ./run.sh build     only builds
# for now the kernel is the one Chamesle Linux ships; KERNEL= picks another
set -eu
cd "$(dirname "$0")"
KERNEL=${KERNEL:-$(ls ../chamesle/work/kernel/build/k/boot/vmlinuz-* | head -n1)}
QEMU=${QEMU:-qemu-system-x86_64}
CC=${CC:-gcc}
rm -rf out/vi
mkdir -p out/vi

# Bill Joy's ex 1.1 with its visual mode, the first vi. as in 1977, mkstr takes the error messages out first.
# it is K&R C for a PDP-11: built with what that needs, and nothing optimized, so it runs the way it was written
joy=$(ls vi/*.c | grep -vE '/(sys|v6|wq)\.c$')
python3 vi/mkstr.py out/ex1.1strings out/vi/ $joy
cp vi/*.h out/vi/
for f in out/vi/*.c; do
    $CC -std=gnu89 -Uunix -Ulinux -fcommon -fpermissive -w -O0 -fno-builtin -ffreestanding -nostdinc \
        -fno-stack-protector -fno-pic -fno-pie -include vi/v6.h -c "$f" -o "${f%.c}.o"
done
# c.h, tcc and the glue under ex
$CC -std=gnu17 -O2 -w -fno-builtin -ffreestanding -nostdinc -fno-stack-protector -fno-pic -fno-pie \
    -I. -Ithird/shim -Ithird/tinycc -DONE_SOURCE=1 -DTCC_TARGET_X86_64 -DCONFIG_TCC_STATIC -DCONFIG_TCC_SEMLOCK=0 \
    -DCONFIG_TCC_BACKTRACE=0 -DCONFIG_TCC_BCHECK=0 -c vi/sys.c -o out/vi/sys.o
ld -static -nostdlib -no-pie --no-warn-mismatch -o out/init out/vi/*.o 2>&1 | grep -v "size of symbol\|size discrepancies" || true

python3 mkinitramfs.py out/initramfs.cpio out/init usr/lib/ex1.1strings=out/ex1.1strings etc/ttycap=vi/ttycap \
    usr/include/c.h=c.h tmp/
[ "${1:-}" = build ] && exit 0
if [ "${1:-}" = serial ]; then
    exec $QEMU -enable-kvm -m 256 -kernel "$KERNEL" -initrd out/initramfs.cpio -nographic -append "console=ttyS0 quiet"
fi
exec $QEMU -enable-kvm -m 256 -kernel "$KERNEL" -initrd out/initramfs.cpio -append "quiet"
