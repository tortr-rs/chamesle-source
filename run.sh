#!/bin/sh
# builds vi, the PID 1, and the initramfs, then boots them in qemu
#   ./run.sh           in a window
#   ./run.sh serial    in this terminal (ctrl-a x quits qemu)
#   ./run.sh build     only builds
#   ./run.sh disk      in a window, with out/disk.img (256 MB, made if missing) as /dev/vda, to install on
#   ./run.sh installed boots out/disk.img the way a PC does: UEFI firmware (FIRMWARE=, its edk2 code and vars)
# for now the kernel is the one Chamesle Linux ships; KERNEL= picks another, MODULES= the modules that go with it
set -eu
cd "$(dirname "$0")"
KERNEL=${KERNEL:-$(ls ../chamesle/work/kernel/build/k/boot/vmlinuz-* | head -n1)}
MODULES=${MODULES:-$(ls -d ../chamesle/work/kernel/build/k/usr/lib/modules/* | head -n1)}
FIRMWARE=${FIRMWARE:-/usr/share/qemu}
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

# the kernel and the FAT modules: to install with, for a disk that UEFI boots needs FAT.
# e1000 is qemu's network card, for wget
python3 mkinitramfs.py out/initramfs.cpio out/init usr/lib/ex1.1strings=out/ex1.1strings etc/ttycap=vi/ttycap \
    usr/include/c.h=c.h usr/src/fbttywm.c=fbttywm/fbttywm.c tmp/ boot/vmlinuz="$KERNEL" lib/modules/fat.ko.zst="$MODULES/kernel/fs/fat/fat.ko.zst" \
    lib/modules/vfat.ko.zst="$MODULES/kernel/fs/fat/vfat.ko.zst" \
    lib/modules/e1000.ko.zst="$MODULES/kernel/drivers/net/ethernet/intel/e1000/e1000.ko.zst"
[ "${1:-}" = build ] && exit 0
if [ "${1:-}" = disk ]; then
    [ -e out/disk.img ] || truncate -s 256M out/disk.img
    exec $QEMU -enable-kvm -m 2048 -kernel "$KERNEL" -initrd out/initramfs.cpio -append "quiet" \
        -drive file=out/disk.img,format=raw,if=virtio
fi
if [ "${1:-}" = installed ]; then
    [ -e out/vars.fd ] || cp "$FIRMWARE/edk2-i386-vars.fd" out/vars.fd
    exec $QEMU -enable-kvm -m 2048 -drive if=pflash,format=raw,readonly=on,file="$FIRMWARE/edk2-x86_64-code.fd" \
        -drive if=pflash,format=raw,file=out/vars.fd -drive file=out/disk.img,format=raw,if=virtio
fi
if [ "${1:-}" = serial ]; then
    exec $QEMU -enable-kvm -m 2048 -kernel "$KERNEL" -initrd out/initramfs.cpio -nographic -append "console=ttyS0 quiet"
fi
exec $QEMU -enable-kvm -m 2048 -kernel "$KERNEL" -initrd out/initramfs.cpio -append "quiet"
