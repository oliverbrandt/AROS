# -*- makefile -*-

COMMON_ASFLAGS	= -m32 -nostdinc -fno-builtin
COMMON_CFLAGS	= -ffreestanding -mrtd -mregparm=3
COMMON_LDFLAGS	= -nostdlib -static -lgcc

# Used by various components.  These rules need to precede them.
normal/execute.c_DEPENDENCIES = grub_script.tab.h
normal/command.c_DEPENDENCIES = grub_script.tab.h
normal/function.c_DEPENDENCIES = grub_script.tab.h
normal/lexer.c_DEPENDENCIES = grub_script.tab.h

# Images.
pkglib_PROGRAMS = kernel.elf

# For kernel.elf.
kernel_elf_SOURCES = kern/i386/ieee1275/startup.S kern/i386/ieee1275/init.c \
	kern/powerpc/ieee1275/init.c \
	kern/powerpc/ieee1275/cmain.c kern/powerpc/ieee1275/openfw.c \
	kern/main.c kern/device.c \
	kern/disk.c kern/dl.c kern/file.c kern/fs.c kern/err.c \
	kern/misc.c kern/mm.c kern/loader.c kern/rescue.c kern/term.c \
	kern/i386/dl.c kern/parser.c kern/partition.c \
	kern/env.c \
	kern/ieee1275/ieee1275.c \
	term/ieee1275/ofconsole.c term/i386/pc/at_keyboard.c \
	disk/ieee1275/ofdisk.c \
	symlist.c
CLEANFILES += kernel.elf kernel_elf-kern_i386_ieee1275_startup.o kernel_elf-kern_i386_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_cmain.o kernel_elf-kern_powerpc_ieee1275_openfw.o kernel_elf-kern_main.o kernel_elf-kern_device.o kernel_elf-kern_disk.o kernel_elf-kern_dl.o kernel_elf-kern_file.o kernel_elf-kern_fs.o kernel_elf-kern_err.o kernel_elf-kern_misc.o kernel_elf-kern_mm.o kernel_elf-kern_loader.o kernel_elf-kern_rescue.o kernel_elf-kern_term.o kernel_elf-kern_i386_dl.o kernel_elf-kern_parser.o kernel_elf-kern_partition.o kernel_elf-kern_env.o kernel_elf-kern_ieee1275_ieee1275.o kernel_elf-term_ieee1275_ofconsole.o kernel_elf-term_i386_pc_at_keyboard.o kernel_elf-disk_ieee1275_ofdisk.o kernel_elf-symlist.o
MOSTLYCLEANFILES += kernel_elf-kern_i386_ieee1275_startup.d kernel_elf-kern_i386_ieee1275_init.d kernel_elf-kern_powerpc_ieee1275_init.d kernel_elf-kern_powerpc_ieee1275_cmain.d kernel_elf-kern_powerpc_ieee1275_openfw.d kernel_elf-kern_main.d kernel_elf-kern_device.d kernel_elf-kern_disk.d kernel_elf-kern_dl.d kernel_elf-kern_file.d kernel_elf-kern_fs.d kernel_elf-kern_err.d kernel_elf-kern_misc.d kernel_elf-kern_mm.d kernel_elf-kern_loader.d kernel_elf-kern_rescue.d kernel_elf-kern_term.d kernel_elf-kern_i386_dl.d kernel_elf-kern_parser.d kernel_elf-kern_partition.d kernel_elf-kern_env.d kernel_elf-kern_ieee1275_ieee1275.d kernel_elf-term_ieee1275_ofconsole.d kernel_elf-term_i386_pc_at_keyboard.d kernel_elf-disk_ieee1275_ofdisk.d kernel_elf-symlist.d

kernel.elf: $(kernel_elf_DEPENDENCIES) kernel_elf-kern_i386_ieee1275_startup.o kernel_elf-kern_i386_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_cmain.o kernel_elf-kern_powerpc_ieee1275_openfw.o kernel_elf-kern_main.o kernel_elf-kern_device.o kernel_elf-kern_disk.o kernel_elf-kern_dl.o kernel_elf-kern_file.o kernel_elf-kern_fs.o kernel_elf-kern_err.o kernel_elf-kern_misc.o kernel_elf-kern_mm.o kernel_elf-kern_loader.o kernel_elf-kern_rescue.o kernel_elf-kern_term.o kernel_elf-kern_i386_dl.o kernel_elf-kern_parser.o kernel_elf-kern_partition.o kernel_elf-kern_env.o kernel_elf-kern_ieee1275_ieee1275.o kernel_elf-term_ieee1275_ofconsole.o kernel_elf-term_i386_pc_at_keyboard.o kernel_elf-disk_ieee1275_ofdisk.o kernel_elf-symlist.o
	$(TARGET_CC) -o $@ kernel_elf-kern_i386_ieee1275_startup.o kernel_elf-kern_i386_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_init.o kernel_elf-kern_powerpc_ieee1275_cmain.o kernel_elf-kern_powerpc_ieee1275_openfw.o kernel_elf-kern_main.o kernel_elf-kern_device.o kernel_elf-kern_disk.o kernel_elf-kern_dl.o kernel_elf-kern_file.o kernel_elf-kern_fs.o kernel_elf-kern_err.o kernel_elf-kern_misc.o kernel_elf-kern_mm.o kernel_elf-kern_loader.o kernel_elf-kern_rescue.o kernel_elf-kern_term.o kernel_elf-kern_i386_dl.o kernel_elf-kern_parser.o kernel_elf-kern_partition.o kernel_elf-kern_env.o kernel_elf-kern_ieee1275_ieee1275.o kernel_elf-term_ieee1275_ofconsole.o kernel_elf-term_i386_pc_at_keyboard.o kernel_elf-disk_ieee1275_ofdisk.o kernel_elf-symlist.o $(TARGET_LDFLAGS) $(kernel_elf_LDFLAGS)

kernel_elf-kern_i386_ieee1275_startup.o: kern/i386/ieee1275/startup.S $(kern/i386/ieee1275/startup.S_DEPENDENCIES)
	$(TARGET_CC) -Ikern/i386/ieee1275 -I$(srcdir)/kern/i386/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_i386_ieee1275_startup.d

kernel_elf-kern_i386_ieee1275_init.o: kern/i386/ieee1275/init.c $(kern/i386/ieee1275/init.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/i386/ieee1275 -I$(srcdir)/kern/i386/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_i386_ieee1275_init.d

kernel_elf-kern_powerpc_ieee1275_init.o: kern/powerpc/ieee1275/init.c $(kern/powerpc/ieee1275/init.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/powerpc/ieee1275 -I$(srcdir)/kern/powerpc/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_powerpc_ieee1275_init.d

kernel_elf-kern_powerpc_ieee1275_cmain.o: kern/powerpc/ieee1275/cmain.c $(kern/powerpc/ieee1275/cmain.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/powerpc/ieee1275 -I$(srcdir)/kern/powerpc/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_powerpc_ieee1275_cmain.d

kernel_elf-kern_powerpc_ieee1275_openfw.o: kern/powerpc/ieee1275/openfw.c $(kern/powerpc/ieee1275/openfw.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/powerpc/ieee1275 -I$(srcdir)/kern/powerpc/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_powerpc_ieee1275_openfw.d

kernel_elf-kern_main.o: kern/main.c $(kern/main.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_main.d

kernel_elf-kern_device.o: kern/device.c $(kern/device.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_device.d

kernel_elf-kern_disk.o: kern/disk.c $(kern/disk.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_disk.d

kernel_elf-kern_dl.o: kern/dl.c $(kern/dl.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_dl.d

kernel_elf-kern_file.o: kern/file.c $(kern/file.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_file.d

kernel_elf-kern_fs.o: kern/fs.c $(kern/fs.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_fs.d

kernel_elf-kern_err.o: kern/err.c $(kern/err.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_err.d

kernel_elf-kern_misc.o: kern/misc.c $(kern/misc.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_misc.d

kernel_elf-kern_mm.o: kern/mm.c $(kern/mm.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_mm.d

kernel_elf-kern_loader.o: kern/loader.c $(kern/loader.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_loader.d

kernel_elf-kern_rescue.o: kern/rescue.c $(kern/rescue.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_rescue.d

kernel_elf-kern_term.o: kern/term.c $(kern/term.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_term.d

kernel_elf-kern_i386_dl.o: kern/i386/dl.c $(kern/i386/dl.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/i386 -I$(srcdir)/kern/i386 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_i386_dl.d

kernel_elf-kern_parser.o: kern/parser.c $(kern/parser.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_parser.d

kernel_elf-kern_partition.o: kern/partition.c $(kern/partition.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_partition.d

kernel_elf-kern_env.o: kern/env.c $(kern/env.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern -I$(srcdir)/kern $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_env.d

kernel_elf-kern_ieee1275_ieee1275.o: kern/ieee1275/ieee1275.c $(kern/ieee1275/ieee1275.c_DEPENDENCIES)
	$(TARGET_CC) -Ikern/ieee1275 -I$(srcdir)/kern/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-kern_ieee1275_ieee1275.d

kernel_elf-term_ieee1275_ofconsole.o: term/ieee1275/ofconsole.c $(term/ieee1275/ofconsole.c_DEPENDENCIES)
	$(TARGET_CC) -Iterm/ieee1275 -I$(srcdir)/term/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-term_ieee1275_ofconsole.d

kernel_elf-term_i386_pc_at_keyboard.o: term/i386/pc/at_keyboard.c $(term/i386/pc/at_keyboard.c_DEPENDENCIES)
	$(TARGET_CC) -Iterm/i386/pc -I$(srcdir)/term/i386/pc $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-term_i386_pc_at_keyboard.d

kernel_elf-disk_ieee1275_ofdisk.o: disk/ieee1275/ofdisk.c $(disk/ieee1275/ofdisk.c_DEPENDENCIES)
	$(TARGET_CC) -Idisk/ieee1275 -I$(srcdir)/disk/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-disk_ieee1275_ofdisk.d

kernel_elf-symlist.o: symlist.c $(symlist.c_DEPENDENCIES)
	$(TARGET_CC) -I. -I$(srcdir)/. $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(kernel_elf_CFLAGS) -MD -c -o $@ $<
-include kernel_elf-symlist.d

kernel_elf_HEADERS = arg.h cache.h device.h disk.h dl.h elf.h elfload.h \
	env.h err.h file.h fs.h kernel.h loader.h misc.h mm.h net.h parser.h \
	partition.h pc_partition.h rescue.h symbol.h term.h time.h types.h \
	ieee1275/ieee1275.h machine/kernel.h machine/loader.h machine/memory.h
kernel_elf_CFLAGS = $(COMMON_CFLAGS)
kernel_elf_LDFLAGS = $(COMMON_LDFLAGS) -Wl,-N,-S,-Ttext,0x10000,-Bstatic

MOSTLYCLEANFILES += symlist.c kernel_syms.lst
DEFSYMFILES += kernel_syms.lst

symlist.c: $(addprefix include/grub/,$(kernel_elf_HEADERS)) config.h gensymlist.sh
	/bin/sh gensymlist.sh $(filter %.h,$^) > $@ || (rm -f $@; exit 1)

kernel_syms.lst: $(addprefix include/grub/,$(kernel_elf_HEADERS)) config.h genkernsyms.sh
	/bin/sh genkernsyms.sh $(filter %.h,$^) > $@ || (rm -f $@; exit 1)

# Utilities.
bin_UTILITIES = grub-mkimage
sbin_UTILITIES = grub-mkdevicemap
ifeq ($(enable_grub_emu), yes)
sbin_UTILITIES += grub-emu
endif

# For grub-mkimage.
grub_mkimage_SOURCES = util/elf/grub-mkimage.c util/misc.c \
	util/resolve.c
CLEANFILES += grub-mkimage grub_mkimage-util_elf_grub_mkimage.o grub_mkimage-util_misc.o grub_mkimage-util_resolve.o
MOSTLYCLEANFILES += grub_mkimage-util_elf_grub_mkimage.d grub_mkimage-util_misc.d grub_mkimage-util_resolve.d

grub-mkimage: $(grub_mkimage_DEPENDENCIES) grub_mkimage-util_elf_grub_mkimage.o grub_mkimage-util_misc.o grub_mkimage-util_resolve.o
	$(CC) -o $@ grub_mkimage-util_elf_grub_mkimage.o grub_mkimage-util_misc.o grub_mkimage-util_resolve.o $(LDFLAGS) $(grub_mkimage_LDFLAGS)

grub_mkimage-util_elf_grub_mkimage.o: util/elf/grub-mkimage.c $(util/elf/grub-mkimage.c_DEPENDENCIES)
	$(CC) -Iutil/elf -I$(srcdir)/util/elf $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkimage_CFLAGS) -MD -c -o $@ $<
-include grub_mkimage-util_elf_grub_mkimage.d

grub_mkimage-util_misc.o: util/misc.c $(util/misc.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkimage_CFLAGS) -MD -c -o $@ $<
-include grub_mkimage-util_misc.d

grub_mkimage-util_resolve.o: util/resolve.c $(util/resolve.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkimage_CFLAGS) -MD -c -o $@ $<
-include grub_mkimage-util_resolve.d

grub_mkimage_LDFLAGS = $(LIBLZO)

# For grub-mkdevicemap.
grub_mkdevicemap_SOURCES = util/grub-mkdevicemap.c util/misc.c		\
	util/i386/get_disk_name.c
CLEANFILES += grub-mkdevicemap grub_mkdevicemap-util_grub_mkdevicemap.o grub_mkdevicemap-util_misc.o grub_mkdevicemap-util_i386_get_disk_name.o
MOSTLYCLEANFILES += grub_mkdevicemap-util_grub_mkdevicemap.d grub_mkdevicemap-util_misc.d grub_mkdevicemap-util_i386_get_disk_name.d

grub-mkdevicemap: $(grub_mkdevicemap_DEPENDENCIES) grub_mkdevicemap-util_grub_mkdevicemap.o grub_mkdevicemap-util_misc.o grub_mkdevicemap-util_i386_get_disk_name.o
	$(CC) -o $@ grub_mkdevicemap-util_grub_mkdevicemap.o grub_mkdevicemap-util_misc.o grub_mkdevicemap-util_i386_get_disk_name.o $(LDFLAGS) $(grub_mkdevicemap_LDFLAGS)

grub_mkdevicemap-util_grub_mkdevicemap.o: util/grub-mkdevicemap.c $(util/grub-mkdevicemap.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkdevicemap_CFLAGS) -MD -c -o $@ $<
-include grub_mkdevicemap-util_grub_mkdevicemap.d

grub_mkdevicemap-util_misc.o: util/misc.c $(util/misc.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkdevicemap_CFLAGS) -MD -c -o $@ $<
-include grub_mkdevicemap-util_misc.d

grub_mkdevicemap-util_i386_get_disk_name.o: util/i386/get_disk_name.c $(util/i386/get_disk_name.c_DEPENDENCIES)
	$(CC) -Iutil/i386 -I$(srcdir)/util/i386 $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_mkdevicemap_CFLAGS) -MD -c -o $@ $<
-include grub_mkdevicemap-util_i386_get_disk_name.d


# For grub-emu.
util/grub-emu.c_DEPENDENCIES = grub_emu_init.h
grub_emu_SOURCES = commands/boot.c commands/cat.c commands/cmp.c	\
	commands/configfile.c commands/echo.c commands/help.c		\
	commands/terminal.c commands/ls.c commands/test.c 		\
	commands/search.c commands/blocklist.c commands/hexdump.c	\
	commands/halt.c commands/reboot.c				\
	commands/i386/cpuid.c						\
	disk/host.c disk/loopback.c					\
	\
	fs/affs.c fs/cpio.c fs/ext2.c fs/fat.c fs/hfs.c			\
	fs/hfsplus.c fs/iso9660.c fs/udf.c fs/jfs.c fs/minix.c		\
	fs/ntfs.c fs/ntfscomp.c fs/reiserfs.c fs/sfs.c			\
	fs/ufs.c fs/xfs.c						\
	\
	fs/fshelp.c							\
	io/gzio.c							\
	kern/device.c kern/disk.c kern/dl.c kern/elf.c kern/env.c	\
	kern/err.c							\
	normal/execute.c kern/file.c kern/fs.c normal/lexer.c 		\
	kern/loader.c kern/main.c kern/misc.c kern/parser.c		\
	grub_script.tab.c kern/partition.c kern/rescue.c kern/term.c	\
	normal/arg.c normal/cmdline.c normal/command.c normal/function.c\
	normal/completion.c normal/main.c				\
	normal/menu.c normal/menu_entry.c normal/misc.c normal/script.c	\
	normal/color.c							\
	partmap/amiga.c	partmap/apple.c partmap/pc.c partmap/sun.c	\
	partmap/acorn.c partmap/gpt.c					\
	util/console.c util/hostfs.c util/grub-emu.c util/misc.c	\
	util/biosdisk.c util/getroot.c					\
	util/i386/pc/misc.c						\
	\
	disk/raid.c disk/lvm.c						\
	grub_emu_init.c
CLEANFILES += grub-emu grub_emu-commands_boot.o grub_emu-commands_cat.o grub_emu-commands_cmp.o grub_emu-commands_configfile.o grub_emu-commands_echo.o grub_emu-commands_help.o grub_emu-commands_terminal.o grub_emu-commands_ls.o grub_emu-commands_test.o grub_emu-commands_search.o grub_emu-commands_blocklist.o grub_emu-commands_hexdump.o grub_emu-commands_halt.o grub_emu-commands_reboot.o grub_emu-commands_i386_cpuid.o grub_emu-disk_host.o grub_emu-disk_loopback.o grub_emu-fs_affs.o grub_emu-fs_cpio.o grub_emu-fs_ext2.o grub_emu-fs_fat.o grub_emu-fs_hfs.o grub_emu-fs_hfsplus.o grub_emu-fs_iso9660.o grub_emu-fs_udf.o grub_emu-fs_jfs.o grub_emu-fs_minix.o grub_emu-fs_ntfs.o grub_emu-fs_ntfscomp.o grub_emu-fs_reiserfs.o grub_emu-fs_sfs.o grub_emu-fs_ufs.o grub_emu-fs_xfs.o grub_emu-fs_fshelp.o grub_emu-io_gzio.o grub_emu-kern_device.o grub_emu-kern_disk.o grub_emu-kern_dl.o grub_emu-kern_elf.o grub_emu-kern_env.o grub_emu-kern_err.o grub_emu-normal_execute.o grub_emu-kern_file.o grub_emu-kern_fs.o grub_emu-normal_lexer.o grub_emu-kern_loader.o grub_emu-kern_main.o grub_emu-kern_misc.o grub_emu-kern_parser.o grub_emu-grub_script_tab.o grub_emu-kern_partition.o grub_emu-kern_rescue.o grub_emu-kern_term.o grub_emu-normal_arg.o grub_emu-normal_cmdline.o grub_emu-normal_command.o grub_emu-normal_function.o grub_emu-normal_completion.o grub_emu-normal_main.o grub_emu-normal_menu.o grub_emu-normal_menu_entry.o grub_emu-normal_misc.o grub_emu-normal_script.o grub_emu-normal_color.o grub_emu-partmap_amiga.o grub_emu-partmap_apple.o grub_emu-partmap_pc.o grub_emu-partmap_sun.o grub_emu-partmap_acorn.o grub_emu-partmap_gpt.o grub_emu-util_console.o grub_emu-util_hostfs.o grub_emu-util_grub_emu.o grub_emu-util_misc.o grub_emu-util_biosdisk.o grub_emu-util_getroot.o grub_emu-util_i386_pc_misc.o grub_emu-disk_raid.o grub_emu-disk_lvm.o grub_emu-grub_emu_init.o
MOSTLYCLEANFILES += grub_emu-commands_boot.d grub_emu-commands_cat.d grub_emu-commands_cmp.d grub_emu-commands_configfile.d grub_emu-commands_echo.d grub_emu-commands_help.d grub_emu-commands_terminal.d grub_emu-commands_ls.d grub_emu-commands_test.d grub_emu-commands_search.d grub_emu-commands_blocklist.d grub_emu-commands_hexdump.d grub_emu-commands_halt.d grub_emu-commands_reboot.d grub_emu-commands_i386_cpuid.d grub_emu-disk_host.d grub_emu-disk_loopback.d grub_emu-fs_affs.d grub_emu-fs_cpio.d grub_emu-fs_ext2.d grub_emu-fs_fat.d grub_emu-fs_hfs.d grub_emu-fs_hfsplus.d grub_emu-fs_iso9660.d grub_emu-fs_udf.d grub_emu-fs_jfs.d grub_emu-fs_minix.d grub_emu-fs_ntfs.d grub_emu-fs_ntfscomp.d grub_emu-fs_reiserfs.d grub_emu-fs_sfs.d grub_emu-fs_ufs.d grub_emu-fs_xfs.d grub_emu-fs_fshelp.d grub_emu-io_gzio.d grub_emu-kern_device.d grub_emu-kern_disk.d grub_emu-kern_dl.d grub_emu-kern_elf.d grub_emu-kern_env.d grub_emu-kern_err.d grub_emu-normal_execute.d grub_emu-kern_file.d grub_emu-kern_fs.d grub_emu-normal_lexer.d grub_emu-kern_loader.d grub_emu-kern_main.d grub_emu-kern_misc.d grub_emu-kern_parser.d grub_emu-grub_script_tab.d grub_emu-kern_partition.d grub_emu-kern_rescue.d grub_emu-kern_term.d grub_emu-normal_arg.d grub_emu-normal_cmdline.d grub_emu-normal_command.d grub_emu-normal_function.d grub_emu-normal_completion.d grub_emu-normal_main.d grub_emu-normal_menu.d grub_emu-normal_menu_entry.d grub_emu-normal_misc.d grub_emu-normal_script.d grub_emu-normal_color.d grub_emu-partmap_amiga.d grub_emu-partmap_apple.d grub_emu-partmap_pc.d grub_emu-partmap_sun.d grub_emu-partmap_acorn.d grub_emu-partmap_gpt.d grub_emu-util_console.d grub_emu-util_hostfs.d grub_emu-util_grub_emu.d grub_emu-util_misc.d grub_emu-util_biosdisk.d grub_emu-util_getroot.d grub_emu-util_i386_pc_misc.d grub_emu-disk_raid.d grub_emu-disk_lvm.d grub_emu-grub_emu_init.d

grub-emu: $(grub_emu_DEPENDENCIES) grub_emu-commands_boot.o grub_emu-commands_cat.o grub_emu-commands_cmp.o grub_emu-commands_configfile.o grub_emu-commands_echo.o grub_emu-commands_help.o grub_emu-commands_terminal.o grub_emu-commands_ls.o grub_emu-commands_test.o grub_emu-commands_search.o grub_emu-commands_blocklist.o grub_emu-commands_hexdump.o grub_emu-commands_halt.o grub_emu-commands_reboot.o grub_emu-commands_i386_cpuid.o grub_emu-disk_host.o grub_emu-disk_loopback.o grub_emu-fs_affs.o grub_emu-fs_cpio.o grub_emu-fs_ext2.o grub_emu-fs_fat.o grub_emu-fs_hfs.o grub_emu-fs_hfsplus.o grub_emu-fs_iso9660.o grub_emu-fs_udf.o grub_emu-fs_jfs.o grub_emu-fs_minix.o grub_emu-fs_ntfs.o grub_emu-fs_ntfscomp.o grub_emu-fs_reiserfs.o grub_emu-fs_sfs.o grub_emu-fs_ufs.o grub_emu-fs_xfs.o grub_emu-fs_fshelp.o grub_emu-io_gzio.o grub_emu-kern_device.o grub_emu-kern_disk.o grub_emu-kern_dl.o grub_emu-kern_elf.o grub_emu-kern_env.o grub_emu-kern_err.o grub_emu-normal_execute.o grub_emu-kern_file.o grub_emu-kern_fs.o grub_emu-normal_lexer.o grub_emu-kern_loader.o grub_emu-kern_main.o grub_emu-kern_misc.o grub_emu-kern_parser.o grub_emu-grub_script_tab.o grub_emu-kern_partition.o grub_emu-kern_rescue.o grub_emu-kern_term.o grub_emu-normal_arg.o grub_emu-normal_cmdline.o grub_emu-normal_command.o grub_emu-normal_function.o grub_emu-normal_completion.o grub_emu-normal_main.o grub_emu-normal_menu.o grub_emu-normal_menu_entry.o grub_emu-normal_misc.o grub_emu-normal_script.o grub_emu-normal_color.o grub_emu-partmap_amiga.o grub_emu-partmap_apple.o grub_emu-partmap_pc.o grub_emu-partmap_sun.o grub_emu-partmap_acorn.o grub_emu-partmap_gpt.o grub_emu-util_console.o grub_emu-util_hostfs.o grub_emu-util_grub_emu.o grub_emu-util_misc.o grub_emu-util_biosdisk.o grub_emu-util_getroot.o grub_emu-util_i386_pc_misc.o grub_emu-disk_raid.o grub_emu-disk_lvm.o grub_emu-grub_emu_init.o
	$(CC) -o $@ grub_emu-commands_boot.o grub_emu-commands_cat.o grub_emu-commands_cmp.o grub_emu-commands_configfile.o grub_emu-commands_echo.o grub_emu-commands_help.o grub_emu-commands_terminal.o grub_emu-commands_ls.o grub_emu-commands_test.o grub_emu-commands_search.o grub_emu-commands_blocklist.o grub_emu-commands_hexdump.o grub_emu-commands_halt.o grub_emu-commands_reboot.o grub_emu-commands_i386_cpuid.o grub_emu-disk_host.o grub_emu-disk_loopback.o grub_emu-fs_affs.o grub_emu-fs_cpio.o grub_emu-fs_ext2.o grub_emu-fs_fat.o grub_emu-fs_hfs.o grub_emu-fs_hfsplus.o grub_emu-fs_iso9660.o grub_emu-fs_udf.o grub_emu-fs_jfs.o grub_emu-fs_minix.o grub_emu-fs_ntfs.o grub_emu-fs_ntfscomp.o grub_emu-fs_reiserfs.o grub_emu-fs_sfs.o grub_emu-fs_ufs.o grub_emu-fs_xfs.o grub_emu-fs_fshelp.o grub_emu-io_gzio.o grub_emu-kern_device.o grub_emu-kern_disk.o grub_emu-kern_dl.o grub_emu-kern_elf.o grub_emu-kern_env.o grub_emu-kern_err.o grub_emu-normal_execute.o grub_emu-kern_file.o grub_emu-kern_fs.o grub_emu-normal_lexer.o grub_emu-kern_loader.o grub_emu-kern_main.o grub_emu-kern_misc.o grub_emu-kern_parser.o grub_emu-grub_script_tab.o grub_emu-kern_partition.o grub_emu-kern_rescue.o grub_emu-kern_term.o grub_emu-normal_arg.o grub_emu-normal_cmdline.o grub_emu-normal_command.o grub_emu-normal_function.o grub_emu-normal_completion.o grub_emu-normal_main.o grub_emu-normal_menu.o grub_emu-normal_menu_entry.o grub_emu-normal_misc.o grub_emu-normal_script.o grub_emu-normal_color.o grub_emu-partmap_amiga.o grub_emu-partmap_apple.o grub_emu-partmap_pc.o grub_emu-partmap_sun.o grub_emu-partmap_acorn.o grub_emu-partmap_gpt.o grub_emu-util_console.o grub_emu-util_hostfs.o grub_emu-util_grub_emu.o grub_emu-util_misc.o grub_emu-util_biosdisk.o grub_emu-util_getroot.o grub_emu-util_i386_pc_misc.o grub_emu-disk_raid.o grub_emu-disk_lvm.o grub_emu-grub_emu_init.o $(LDFLAGS) $(grub_emu_LDFLAGS)

grub_emu-commands_boot.o: commands/boot.c $(commands/boot.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_boot.d

grub_emu-commands_cat.o: commands/cat.c $(commands/cat.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_cat.d

grub_emu-commands_cmp.o: commands/cmp.c $(commands/cmp.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_cmp.d

grub_emu-commands_configfile.o: commands/configfile.c $(commands/configfile.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_configfile.d

grub_emu-commands_echo.o: commands/echo.c $(commands/echo.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_echo.d

grub_emu-commands_help.o: commands/help.c $(commands/help.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_help.d

grub_emu-commands_terminal.o: commands/terminal.c $(commands/terminal.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_terminal.d

grub_emu-commands_ls.o: commands/ls.c $(commands/ls.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_ls.d

grub_emu-commands_test.o: commands/test.c $(commands/test.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_test.d

grub_emu-commands_search.o: commands/search.c $(commands/search.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_search.d

grub_emu-commands_blocklist.o: commands/blocklist.c $(commands/blocklist.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_blocklist.d

grub_emu-commands_hexdump.o: commands/hexdump.c $(commands/hexdump.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_hexdump.d

grub_emu-commands_halt.o: commands/halt.c $(commands/halt.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_halt.d

grub_emu-commands_reboot.o: commands/reboot.c $(commands/reboot.c_DEPENDENCIES)
	$(CC) -Icommands -I$(srcdir)/commands $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_reboot.d

grub_emu-commands_i386_cpuid.o: commands/i386/cpuid.c $(commands/i386/cpuid.c_DEPENDENCIES)
	$(CC) -Icommands/i386 -I$(srcdir)/commands/i386 $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-commands_i386_cpuid.d

grub_emu-disk_host.o: disk/host.c $(disk/host.c_DEPENDENCIES)
	$(CC) -Idisk -I$(srcdir)/disk $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-disk_host.d

grub_emu-disk_loopback.o: disk/loopback.c $(disk/loopback.c_DEPENDENCIES)
	$(CC) -Idisk -I$(srcdir)/disk $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-disk_loopback.d

grub_emu-fs_affs.o: fs/affs.c $(fs/affs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_affs.d

grub_emu-fs_cpio.o: fs/cpio.c $(fs/cpio.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_cpio.d

grub_emu-fs_ext2.o: fs/ext2.c $(fs/ext2.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_ext2.d

grub_emu-fs_fat.o: fs/fat.c $(fs/fat.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_fat.d

grub_emu-fs_hfs.o: fs/hfs.c $(fs/hfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_hfs.d

grub_emu-fs_hfsplus.o: fs/hfsplus.c $(fs/hfsplus.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_hfsplus.d

grub_emu-fs_iso9660.o: fs/iso9660.c $(fs/iso9660.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_iso9660.d

grub_emu-fs_udf.o: fs/udf.c $(fs/udf.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_udf.d

grub_emu-fs_jfs.o: fs/jfs.c $(fs/jfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_jfs.d

grub_emu-fs_minix.o: fs/minix.c $(fs/minix.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_minix.d

grub_emu-fs_ntfs.o: fs/ntfs.c $(fs/ntfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_ntfs.d

grub_emu-fs_ntfscomp.o: fs/ntfscomp.c $(fs/ntfscomp.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_ntfscomp.d

grub_emu-fs_reiserfs.o: fs/reiserfs.c $(fs/reiserfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_reiserfs.d

grub_emu-fs_sfs.o: fs/sfs.c $(fs/sfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_sfs.d

grub_emu-fs_ufs.o: fs/ufs.c $(fs/ufs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_ufs.d

grub_emu-fs_xfs.o: fs/xfs.c $(fs/xfs.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_xfs.d

grub_emu-fs_fshelp.o: fs/fshelp.c $(fs/fshelp.c_DEPENDENCIES)
	$(CC) -Ifs -I$(srcdir)/fs $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-fs_fshelp.d

grub_emu-io_gzio.o: io/gzio.c $(io/gzio.c_DEPENDENCIES)
	$(CC) -Iio -I$(srcdir)/io $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-io_gzio.d

grub_emu-kern_device.o: kern/device.c $(kern/device.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_device.d

grub_emu-kern_disk.o: kern/disk.c $(kern/disk.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_disk.d

grub_emu-kern_dl.o: kern/dl.c $(kern/dl.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_dl.d

grub_emu-kern_elf.o: kern/elf.c $(kern/elf.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_elf.d

grub_emu-kern_env.o: kern/env.c $(kern/env.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_env.d

grub_emu-kern_err.o: kern/err.c $(kern/err.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_err.d

grub_emu-normal_execute.o: normal/execute.c $(normal/execute.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_execute.d

grub_emu-kern_file.o: kern/file.c $(kern/file.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_file.d

grub_emu-kern_fs.o: kern/fs.c $(kern/fs.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_fs.d

grub_emu-normal_lexer.o: normal/lexer.c $(normal/lexer.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_lexer.d

grub_emu-kern_loader.o: kern/loader.c $(kern/loader.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_loader.d

grub_emu-kern_main.o: kern/main.c $(kern/main.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_main.d

grub_emu-kern_misc.o: kern/misc.c $(kern/misc.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_misc.d

grub_emu-kern_parser.o: kern/parser.c $(kern/parser.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_parser.d

grub_emu-grub_script_tab.o: grub_script.tab.c $(grub_script.tab.c_DEPENDENCIES)
	$(CC) -I. -I$(srcdir)/. $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-grub_script_tab.d

grub_emu-kern_partition.o: kern/partition.c $(kern/partition.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_partition.d

grub_emu-kern_rescue.o: kern/rescue.c $(kern/rescue.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_rescue.d

grub_emu-kern_term.o: kern/term.c $(kern/term.c_DEPENDENCIES)
	$(CC) -Ikern -I$(srcdir)/kern $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-kern_term.d

grub_emu-normal_arg.o: normal/arg.c $(normal/arg.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_arg.d

grub_emu-normal_cmdline.o: normal/cmdline.c $(normal/cmdline.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_cmdline.d

grub_emu-normal_command.o: normal/command.c $(normal/command.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_command.d

grub_emu-normal_function.o: normal/function.c $(normal/function.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_function.d

grub_emu-normal_completion.o: normal/completion.c $(normal/completion.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_completion.d

grub_emu-normal_main.o: normal/main.c $(normal/main.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_main.d

grub_emu-normal_menu.o: normal/menu.c $(normal/menu.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_menu.d

grub_emu-normal_menu_entry.o: normal/menu_entry.c $(normal/menu_entry.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_menu_entry.d

grub_emu-normal_misc.o: normal/misc.c $(normal/misc.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_misc.d

grub_emu-normal_script.o: normal/script.c $(normal/script.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_script.d

grub_emu-normal_color.o: normal/color.c $(normal/color.c_DEPENDENCIES)
	$(CC) -Inormal -I$(srcdir)/normal $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-normal_color.d

grub_emu-partmap_amiga.o: partmap/amiga.c $(partmap/amiga.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_amiga.d

grub_emu-partmap_apple.o: partmap/apple.c $(partmap/apple.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_apple.d

grub_emu-partmap_pc.o: partmap/pc.c $(partmap/pc.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_pc.d

grub_emu-partmap_sun.o: partmap/sun.c $(partmap/sun.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_sun.d

grub_emu-partmap_acorn.o: partmap/acorn.c $(partmap/acorn.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_acorn.d

grub_emu-partmap_gpt.o: partmap/gpt.c $(partmap/gpt.c_DEPENDENCIES)
	$(CC) -Ipartmap -I$(srcdir)/partmap $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-partmap_gpt.d

grub_emu-util_console.o: util/console.c $(util/console.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_console.d

grub_emu-util_hostfs.o: util/hostfs.c $(util/hostfs.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_hostfs.d

grub_emu-util_grub_emu.o: util/grub-emu.c $(util/grub-emu.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_grub_emu.d

grub_emu-util_misc.o: util/misc.c $(util/misc.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_misc.d

grub_emu-util_biosdisk.o: util/biosdisk.c $(util/biosdisk.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_biosdisk.d

grub_emu-util_getroot.o: util/getroot.c $(util/getroot.c_DEPENDENCIES)
	$(CC) -Iutil -I$(srcdir)/util $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_getroot.d

grub_emu-util_i386_pc_misc.o: util/i386/pc/misc.c $(util/i386/pc/misc.c_DEPENDENCIES)
	$(CC) -Iutil/i386/pc -I$(srcdir)/util/i386/pc $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-util_i386_pc_misc.d

grub_emu-disk_raid.o: disk/raid.c $(disk/raid.c_DEPENDENCIES)
	$(CC) -Idisk -I$(srcdir)/disk $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-disk_raid.d

grub_emu-disk_lvm.o: disk/lvm.c $(disk/lvm.c_DEPENDENCIES)
	$(CC) -Idisk -I$(srcdir)/disk $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-disk_lvm.d

grub_emu-grub_emu_init.o: grub_emu_init.c $(grub_emu_init.c_DEPENDENCIES)
	$(CC) -I. -I$(srcdir)/. $(CPPFLAGS) $(CFLAGS) -DGRUB_UTIL=1 $(grub_emu_CFLAGS) -MD -c -o $@ $<
-include grub_emu-grub_emu_init.d


grub_emu_LDFLAGS = $(LIBCURSES)

# Modules.
pkglib_MODULES = normal.mod halt.mod reboot.mod suspend.mod cpuid.mod	\
	multiboot.mod _multiboot.mod serial.mod

# For normal.mod.
normal_mod_SOURCES = normal/arg.c normal/cmdline.c normal/command.c	\
	normal/completion.c normal/execute.c		 		\
	normal/function.c normal/lexer.c normal/main.c normal/menu.c	\
	normal/menu_entry.c normal/misc.c grub_script.tab.c 		\
	normal/script.c normal/i386/setjmp.S normal/color.c
CLEANFILES += normal.mod mod-normal.o mod-normal.c pre-normal.o normal_mod-normal_arg.o normal_mod-normal_cmdline.o normal_mod-normal_command.o normal_mod-normal_completion.o normal_mod-normal_execute.o normal_mod-normal_function.o normal_mod-normal_lexer.o normal_mod-normal_main.o normal_mod-normal_menu.o normal_mod-normal_menu_entry.o normal_mod-normal_misc.o normal_mod-grub_script_tab.o normal_mod-normal_script.o normal_mod-normal_i386_setjmp.o normal_mod-normal_color.o und-normal.lst
ifneq ($(normal_mod_EXPORTS),no)
CLEANFILES += def-normal.lst
DEFSYMFILES += def-normal.lst
endif
MOSTLYCLEANFILES += normal_mod-normal_arg.d normal_mod-normal_cmdline.d normal_mod-normal_command.d normal_mod-normal_completion.d normal_mod-normal_execute.d normal_mod-normal_function.d normal_mod-normal_lexer.d normal_mod-normal_main.d normal_mod-normal_menu.d normal_mod-normal_menu_entry.d normal_mod-normal_misc.d normal_mod-grub_script_tab.d normal_mod-normal_script.d normal_mod-normal_i386_setjmp.d normal_mod-normal_color.d
UNDSYMFILES += und-normal.lst

normal.mod: pre-normal.o mod-normal.o
	-rm -f $@
	$(TARGET_CC) $(normal_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-normal.o: $(normal_mod_DEPENDENCIES) normal_mod-normal_arg.o normal_mod-normal_cmdline.o normal_mod-normal_command.o normal_mod-normal_completion.o normal_mod-normal_execute.o normal_mod-normal_function.o normal_mod-normal_lexer.o normal_mod-normal_main.o normal_mod-normal_menu.o normal_mod-normal_menu_entry.o normal_mod-normal_misc.o normal_mod-grub_script_tab.o normal_mod-normal_script.o normal_mod-normal_i386_setjmp.o normal_mod-normal_color.o
	-rm -f $@
	$(TARGET_CC) $(normal_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ normal_mod-normal_arg.o normal_mod-normal_cmdline.o normal_mod-normal_command.o normal_mod-normal_completion.o normal_mod-normal_execute.o normal_mod-normal_function.o normal_mod-normal_lexer.o normal_mod-normal_main.o normal_mod-normal_menu.o normal_mod-normal_menu_entry.o normal_mod-normal_misc.o normal_mod-grub_script_tab.o normal_mod-normal_script.o normal_mod-normal_i386_setjmp.o normal_mod-normal_color.o

mod-normal.o: mod-normal.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -c -o $@ $<

mod-normal.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'normal' $< > $@ || (rm -f $@; exit 1)

ifneq ($(normal_mod_EXPORTS),no)
def-normal.lst: pre-normal.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 normal/' > $@
endif

und-normal.lst: pre-normal.o
	echo 'normal' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

normal_mod-normal_arg.o: normal/arg.c $(normal/arg.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_arg.d

CLEANFILES += cmd-normal_mod-normal_arg.lst fs-normal_mod-normal_arg.lst
COMMANDFILES += cmd-normal_mod-normal_arg.lst
FSFILES += fs-normal_mod-normal_arg.lst

cmd-normal_mod-normal_arg.lst: normal/arg.c $(normal/arg.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_arg.lst: normal/arg.c $(normal/arg.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_cmdline.o: normal/cmdline.c $(normal/cmdline.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_cmdline.d

CLEANFILES += cmd-normal_mod-normal_cmdline.lst fs-normal_mod-normal_cmdline.lst
COMMANDFILES += cmd-normal_mod-normal_cmdline.lst
FSFILES += fs-normal_mod-normal_cmdline.lst

cmd-normal_mod-normal_cmdline.lst: normal/cmdline.c $(normal/cmdline.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_cmdline.lst: normal/cmdline.c $(normal/cmdline.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_command.o: normal/command.c $(normal/command.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_command.d

CLEANFILES += cmd-normal_mod-normal_command.lst fs-normal_mod-normal_command.lst
COMMANDFILES += cmd-normal_mod-normal_command.lst
FSFILES += fs-normal_mod-normal_command.lst

cmd-normal_mod-normal_command.lst: normal/command.c $(normal/command.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_command.lst: normal/command.c $(normal/command.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_completion.o: normal/completion.c $(normal/completion.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_completion.d

CLEANFILES += cmd-normal_mod-normal_completion.lst fs-normal_mod-normal_completion.lst
COMMANDFILES += cmd-normal_mod-normal_completion.lst
FSFILES += fs-normal_mod-normal_completion.lst

cmd-normal_mod-normal_completion.lst: normal/completion.c $(normal/completion.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_completion.lst: normal/completion.c $(normal/completion.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_execute.o: normal/execute.c $(normal/execute.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_execute.d

CLEANFILES += cmd-normal_mod-normal_execute.lst fs-normal_mod-normal_execute.lst
COMMANDFILES += cmd-normal_mod-normal_execute.lst
FSFILES += fs-normal_mod-normal_execute.lst

cmd-normal_mod-normal_execute.lst: normal/execute.c $(normal/execute.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_execute.lst: normal/execute.c $(normal/execute.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_function.o: normal/function.c $(normal/function.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_function.d

CLEANFILES += cmd-normal_mod-normal_function.lst fs-normal_mod-normal_function.lst
COMMANDFILES += cmd-normal_mod-normal_function.lst
FSFILES += fs-normal_mod-normal_function.lst

cmd-normal_mod-normal_function.lst: normal/function.c $(normal/function.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_function.lst: normal/function.c $(normal/function.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_lexer.o: normal/lexer.c $(normal/lexer.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_lexer.d

CLEANFILES += cmd-normal_mod-normal_lexer.lst fs-normal_mod-normal_lexer.lst
COMMANDFILES += cmd-normal_mod-normal_lexer.lst
FSFILES += fs-normal_mod-normal_lexer.lst

cmd-normal_mod-normal_lexer.lst: normal/lexer.c $(normal/lexer.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_lexer.lst: normal/lexer.c $(normal/lexer.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_main.o: normal/main.c $(normal/main.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_main.d

CLEANFILES += cmd-normal_mod-normal_main.lst fs-normal_mod-normal_main.lst
COMMANDFILES += cmd-normal_mod-normal_main.lst
FSFILES += fs-normal_mod-normal_main.lst

cmd-normal_mod-normal_main.lst: normal/main.c $(normal/main.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_main.lst: normal/main.c $(normal/main.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_menu.o: normal/menu.c $(normal/menu.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_menu.d

CLEANFILES += cmd-normal_mod-normal_menu.lst fs-normal_mod-normal_menu.lst
COMMANDFILES += cmd-normal_mod-normal_menu.lst
FSFILES += fs-normal_mod-normal_menu.lst

cmd-normal_mod-normal_menu.lst: normal/menu.c $(normal/menu.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_menu.lst: normal/menu.c $(normal/menu.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_menu_entry.o: normal/menu_entry.c $(normal/menu_entry.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_menu_entry.d

CLEANFILES += cmd-normal_mod-normal_menu_entry.lst fs-normal_mod-normal_menu_entry.lst
COMMANDFILES += cmd-normal_mod-normal_menu_entry.lst
FSFILES += fs-normal_mod-normal_menu_entry.lst

cmd-normal_mod-normal_menu_entry.lst: normal/menu_entry.c $(normal/menu_entry.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_menu_entry.lst: normal/menu_entry.c $(normal/menu_entry.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_misc.o: normal/misc.c $(normal/misc.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_misc.d

CLEANFILES += cmd-normal_mod-normal_misc.lst fs-normal_mod-normal_misc.lst
COMMANDFILES += cmd-normal_mod-normal_misc.lst
FSFILES += fs-normal_mod-normal_misc.lst

cmd-normal_mod-normal_misc.lst: normal/misc.c $(normal/misc.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_misc.lst: normal/misc.c $(normal/misc.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-grub_script_tab.o: grub_script.tab.c $(grub_script.tab.c_DEPENDENCIES)
	$(TARGET_CC) -I. -I$(srcdir)/. $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-grub_script_tab.d

CLEANFILES += cmd-normal_mod-grub_script_tab.lst fs-normal_mod-grub_script_tab.lst
COMMANDFILES += cmd-normal_mod-grub_script_tab.lst
FSFILES += fs-normal_mod-grub_script_tab.lst

cmd-normal_mod-grub_script_tab.lst: grub_script.tab.c $(grub_script.tab.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -I. -I$(srcdir)/. $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-grub_script_tab.lst: grub_script.tab.c $(grub_script.tab.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -I. -I$(srcdir)/. $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_script.o: normal/script.c $(normal/script.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_script.d

CLEANFILES += cmd-normal_mod-normal_script.lst fs-normal_mod-normal_script.lst
COMMANDFILES += cmd-normal_mod-normal_script.lst
FSFILES += fs-normal_mod-normal_script.lst

cmd-normal_mod-normal_script.lst: normal/script.c $(normal/script.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_script.lst: normal/script.c $(normal/script.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_i386_setjmp.o: normal/i386/setjmp.S $(normal/i386/setjmp.S_DEPENDENCIES)
	$(TARGET_CC) -Inormal/i386 -I$(srcdir)/normal/i386 $(TARGET_CPPFLAGS) -DASM_FILE=1 $(TARGET_ASFLAGS) $(normal_mod_ASFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_i386_setjmp.d

CLEANFILES += cmd-normal_mod-normal_i386_setjmp.lst fs-normal_mod-normal_i386_setjmp.lst
COMMANDFILES += cmd-normal_mod-normal_i386_setjmp.lst
FSFILES += fs-normal_mod-normal_i386_setjmp.lst

cmd-normal_mod-normal_i386_setjmp.lst: normal/i386/setjmp.S $(normal/i386/setjmp.S_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal/i386 -I$(srcdir)/normal/i386 $(TARGET_CPPFLAGS) $(TARGET_ASFLAGS) $(normal_mod_ASFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_i386_setjmp.lst: normal/i386/setjmp.S $(normal/i386/setjmp.S_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal/i386 -I$(srcdir)/normal/i386 $(TARGET_CPPFLAGS) $(TARGET_ASFLAGS) $(normal_mod_ASFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod-normal_color.o: normal/color.c $(normal/color.c_DEPENDENCIES)
	$(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -MD -c -o $@ $<
-include normal_mod-normal_color.d

CLEANFILES += cmd-normal_mod-normal_color.lst fs-normal_mod-normal_color.lst
COMMANDFILES += cmd-normal_mod-normal_color.lst
FSFILES += fs-normal_mod-normal_color.lst

cmd-normal_mod-normal_color.lst: normal/color.c $(normal/color.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh normal > $@ || (rm -f $@; exit 1)

fs-normal_mod-normal_color.lst: normal/color.c $(normal/color.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Inormal -I$(srcdir)/normal $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(normal_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh normal > $@ || (rm -f $@; exit 1)


normal_mod_CFLAGS = $(COMMON_CFLAGS)
normal_mod_ASFLAGS = $(COMMON_ASFLAGS)
normal_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For _multiboot.mod.
_multiboot_mod_SOURCES = loader/powerpc/ieee1275/multiboot2.c \
			 loader/multiboot2.c \
			 loader/multiboot_loader.c
CLEANFILES += _multiboot.mod mod-_multiboot.o mod-_multiboot.c pre-_multiboot.o _multiboot_mod-loader_powerpc_ieee1275_multiboot2.o _multiboot_mod-loader_multiboot2.o _multiboot_mod-loader_multiboot_loader.o und-_multiboot.lst
ifneq ($(_multiboot_mod_EXPORTS),no)
CLEANFILES += def-_multiboot.lst
DEFSYMFILES += def-_multiboot.lst
endif
MOSTLYCLEANFILES += _multiboot_mod-loader_powerpc_ieee1275_multiboot2.d _multiboot_mod-loader_multiboot2.d _multiboot_mod-loader_multiboot_loader.d
UNDSYMFILES += und-_multiboot.lst

_multiboot.mod: pre-_multiboot.o mod-_multiboot.o
	-rm -f $@
	$(TARGET_CC) $(_multiboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-_multiboot.o: $(_multiboot_mod_DEPENDENCIES) _multiboot_mod-loader_powerpc_ieee1275_multiboot2.o _multiboot_mod-loader_multiboot2.o _multiboot_mod-loader_multiboot_loader.o
	-rm -f $@
	$(TARGET_CC) $(_multiboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ _multiboot_mod-loader_powerpc_ieee1275_multiboot2.o _multiboot_mod-loader_multiboot2.o _multiboot_mod-loader_multiboot_loader.o

mod-_multiboot.o: mod-_multiboot.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -c -o $@ $<

mod-_multiboot.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh '_multiboot' $< > $@ || (rm -f $@; exit 1)

ifneq ($(_multiboot_mod_EXPORTS),no)
def-_multiboot.lst: pre-_multiboot.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 _multiboot/' > $@
endif

und-_multiboot.lst: pre-_multiboot.o
	echo '_multiboot' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

_multiboot_mod-loader_powerpc_ieee1275_multiboot2.o: loader/powerpc/ieee1275/multiboot2.c $(loader/powerpc/ieee1275/multiboot2.c_DEPENDENCIES)
	$(TARGET_CC) -Iloader/powerpc/ieee1275 -I$(srcdir)/loader/powerpc/ieee1275 $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -MD -c -o $@ $<
-include _multiboot_mod-loader_powerpc_ieee1275_multiboot2.d

CLEANFILES += cmd-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst fs-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst
COMMANDFILES += cmd-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst
FSFILES += fs-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst

cmd-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst: loader/powerpc/ieee1275/multiboot2.c $(loader/powerpc/ieee1275/multiboot2.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Iloader/powerpc/ieee1275 -I$(srcdir)/loader/powerpc/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh _multiboot > $@ || (rm -f $@; exit 1)

fs-_multiboot_mod-loader_powerpc_ieee1275_multiboot2.lst: loader/powerpc/ieee1275/multiboot2.c $(loader/powerpc/ieee1275/multiboot2.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Iloader/powerpc/ieee1275 -I$(srcdir)/loader/powerpc/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh _multiboot > $@ || (rm -f $@; exit 1)


_multiboot_mod-loader_multiboot2.o: loader/multiboot2.c $(loader/multiboot2.c_DEPENDENCIES)
	$(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -MD -c -o $@ $<
-include _multiboot_mod-loader_multiboot2.d

CLEANFILES += cmd-_multiboot_mod-loader_multiboot2.lst fs-_multiboot_mod-loader_multiboot2.lst
COMMANDFILES += cmd-_multiboot_mod-loader_multiboot2.lst
FSFILES += fs-_multiboot_mod-loader_multiboot2.lst

cmd-_multiboot_mod-loader_multiboot2.lst: loader/multiboot2.c $(loader/multiboot2.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh _multiboot > $@ || (rm -f $@; exit 1)

fs-_multiboot_mod-loader_multiboot2.lst: loader/multiboot2.c $(loader/multiboot2.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh _multiboot > $@ || (rm -f $@; exit 1)


_multiboot_mod-loader_multiboot_loader.o: loader/multiboot_loader.c $(loader/multiboot_loader.c_DEPENDENCIES)
	$(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -MD -c -o $@ $<
-include _multiboot_mod-loader_multiboot_loader.d

CLEANFILES += cmd-_multiboot_mod-loader_multiboot_loader.lst fs-_multiboot_mod-loader_multiboot_loader.lst
COMMANDFILES += cmd-_multiboot_mod-loader_multiboot_loader.lst
FSFILES += fs-_multiboot_mod-loader_multiboot_loader.lst

cmd-_multiboot_mod-loader_multiboot_loader.lst: loader/multiboot_loader.c $(loader/multiboot_loader.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh _multiboot > $@ || (rm -f $@; exit 1)

fs-_multiboot_mod-loader_multiboot_loader.lst: loader/multiboot_loader.c $(loader/multiboot_loader.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(_multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh _multiboot > $@ || (rm -f $@; exit 1)


_multiboot_mod_CFLAGS = $(COMMON_CFLAGS)
_multiboot_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For multiboot.mod.
multiboot_mod_SOURCES = loader/multiboot_loader_normal.c 
CLEANFILES += multiboot.mod mod-multiboot.o mod-multiboot.c pre-multiboot.o multiboot_mod-loader_multiboot_loader_normal.o und-multiboot.lst
ifneq ($(multiboot_mod_EXPORTS),no)
CLEANFILES += def-multiboot.lst
DEFSYMFILES += def-multiboot.lst
endif
MOSTLYCLEANFILES += multiboot_mod-loader_multiboot_loader_normal.d
UNDSYMFILES += und-multiboot.lst

multiboot.mod: pre-multiboot.o mod-multiboot.o
	-rm -f $@
	$(TARGET_CC) $(multiboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-multiboot.o: $(multiboot_mod_DEPENDENCIES) multiboot_mod-loader_multiboot_loader_normal.o
	-rm -f $@
	$(TARGET_CC) $(multiboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ multiboot_mod-loader_multiboot_loader_normal.o

mod-multiboot.o: mod-multiboot.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(multiboot_mod_CFLAGS) -c -o $@ $<

mod-multiboot.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'multiboot' $< > $@ || (rm -f $@; exit 1)

ifneq ($(multiboot_mod_EXPORTS),no)
def-multiboot.lst: pre-multiboot.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 multiboot/' > $@
endif

und-multiboot.lst: pre-multiboot.o
	echo 'multiboot' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

multiboot_mod-loader_multiboot_loader_normal.o: loader/multiboot_loader_normal.c $(loader/multiboot_loader_normal.c_DEPENDENCIES)
	$(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(multiboot_mod_CFLAGS) -MD -c -o $@ $<
-include multiboot_mod-loader_multiboot_loader_normal.d

CLEANFILES += cmd-multiboot_mod-loader_multiboot_loader_normal.lst fs-multiboot_mod-loader_multiboot_loader_normal.lst
COMMANDFILES += cmd-multiboot_mod-loader_multiboot_loader_normal.lst
FSFILES += fs-multiboot_mod-loader_multiboot_loader_normal.lst

cmd-multiboot_mod-loader_multiboot_loader_normal.lst: loader/multiboot_loader_normal.c $(loader/multiboot_loader_normal.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh multiboot > $@ || (rm -f $@; exit 1)

fs-multiboot_mod-loader_multiboot_loader_normal.lst: loader/multiboot_loader_normal.c $(loader/multiboot_loader_normal.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Iloader -I$(srcdir)/loader $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(multiboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh multiboot > $@ || (rm -f $@; exit 1)


multiboot_mod_CFLAGS = $(COMMON_CFLAGS)
multiboot_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For suspend.mod
suspend_mod_SOURCES = commands/ieee1275/suspend.c
CLEANFILES += suspend.mod mod-suspend.o mod-suspend.c pre-suspend.o suspend_mod-commands_ieee1275_suspend.o und-suspend.lst
ifneq ($(suspend_mod_EXPORTS),no)
CLEANFILES += def-suspend.lst
DEFSYMFILES += def-suspend.lst
endif
MOSTLYCLEANFILES += suspend_mod-commands_ieee1275_suspend.d
UNDSYMFILES += und-suspend.lst

suspend.mod: pre-suspend.o mod-suspend.o
	-rm -f $@
	$(TARGET_CC) $(suspend_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-suspend.o: $(suspend_mod_DEPENDENCIES) suspend_mod-commands_ieee1275_suspend.o
	-rm -f $@
	$(TARGET_CC) $(suspend_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ suspend_mod-commands_ieee1275_suspend.o

mod-suspend.o: mod-suspend.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(suspend_mod_CFLAGS) -c -o $@ $<

mod-suspend.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'suspend' $< > $@ || (rm -f $@; exit 1)

ifneq ($(suspend_mod_EXPORTS),no)
def-suspend.lst: pre-suspend.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 suspend/' > $@
endif

und-suspend.lst: pre-suspend.o
	echo 'suspend' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

suspend_mod-commands_ieee1275_suspend.o: commands/ieee1275/suspend.c $(commands/ieee1275/suspend.c_DEPENDENCIES)
	$(TARGET_CC) -Icommands/ieee1275 -I$(srcdir)/commands/ieee1275 $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(suspend_mod_CFLAGS) -MD -c -o $@ $<
-include suspend_mod-commands_ieee1275_suspend.d

CLEANFILES += cmd-suspend_mod-commands_ieee1275_suspend.lst fs-suspend_mod-commands_ieee1275_suspend.lst
COMMANDFILES += cmd-suspend_mod-commands_ieee1275_suspend.lst
FSFILES += fs-suspend_mod-commands_ieee1275_suspend.lst

cmd-suspend_mod-commands_ieee1275_suspend.lst: commands/ieee1275/suspend.c $(commands/ieee1275/suspend.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Icommands/ieee1275 -I$(srcdir)/commands/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(suspend_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh suspend > $@ || (rm -f $@; exit 1)

fs-suspend_mod-commands_ieee1275_suspend.lst: commands/ieee1275/suspend.c $(commands/ieee1275/suspend.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Icommands/ieee1275 -I$(srcdir)/commands/ieee1275 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(suspend_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh suspend > $@ || (rm -f $@; exit 1)


suspend_mod_CFLAGS = $(COMMON_CFLAGS)
suspend_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For reboot.mod
reboot_mod_SOURCES = commands/reboot.c
CLEANFILES += reboot.mod mod-reboot.o mod-reboot.c pre-reboot.o reboot_mod-commands_reboot.o und-reboot.lst
ifneq ($(reboot_mod_EXPORTS),no)
CLEANFILES += def-reboot.lst
DEFSYMFILES += def-reboot.lst
endif
MOSTLYCLEANFILES += reboot_mod-commands_reboot.d
UNDSYMFILES += und-reboot.lst

reboot.mod: pre-reboot.o mod-reboot.o
	-rm -f $@
	$(TARGET_CC) $(reboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-reboot.o: $(reboot_mod_DEPENDENCIES) reboot_mod-commands_reboot.o
	-rm -f $@
	$(TARGET_CC) $(reboot_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ reboot_mod-commands_reboot.o

mod-reboot.o: mod-reboot.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(reboot_mod_CFLAGS) -c -o $@ $<

mod-reboot.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'reboot' $< > $@ || (rm -f $@; exit 1)

ifneq ($(reboot_mod_EXPORTS),no)
def-reboot.lst: pre-reboot.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 reboot/' > $@
endif

und-reboot.lst: pre-reboot.o
	echo 'reboot' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

reboot_mod-commands_reboot.o: commands/reboot.c $(commands/reboot.c_DEPENDENCIES)
	$(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(reboot_mod_CFLAGS) -MD -c -o $@ $<
-include reboot_mod-commands_reboot.d

CLEANFILES += cmd-reboot_mod-commands_reboot.lst fs-reboot_mod-commands_reboot.lst
COMMANDFILES += cmd-reboot_mod-commands_reboot.lst
FSFILES += fs-reboot_mod-commands_reboot.lst

cmd-reboot_mod-commands_reboot.lst: commands/reboot.c $(commands/reboot.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(reboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh reboot > $@ || (rm -f $@; exit 1)

fs-reboot_mod-commands_reboot.lst: commands/reboot.c $(commands/reboot.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(reboot_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh reboot > $@ || (rm -f $@; exit 1)


reboot_mod_CFLAGS = $(COMMON_CFLAGS)
reboot_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For halt.mod
halt_mod_SOURCES = commands/halt.c
CLEANFILES += halt.mod mod-halt.o mod-halt.c pre-halt.o halt_mod-commands_halt.o und-halt.lst
ifneq ($(halt_mod_EXPORTS),no)
CLEANFILES += def-halt.lst
DEFSYMFILES += def-halt.lst
endif
MOSTLYCLEANFILES += halt_mod-commands_halt.d
UNDSYMFILES += und-halt.lst

halt.mod: pre-halt.o mod-halt.o
	-rm -f $@
	$(TARGET_CC) $(halt_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-halt.o: $(halt_mod_DEPENDENCIES) halt_mod-commands_halt.o
	-rm -f $@
	$(TARGET_CC) $(halt_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ halt_mod-commands_halt.o

mod-halt.o: mod-halt.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(halt_mod_CFLAGS) -c -o $@ $<

mod-halt.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'halt' $< > $@ || (rm -f $@; exit 1)

ifneq ($(halt_mod_EXPORTS),no)
def-halt.lst: pre-halt.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 halt/' > $@
endif

und-halt.lst: pre-halt.o
	echo 'halt' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

halt_mod-commands_halt.o: commands/halt.c $(commands/halt.c_DEPENDENCIES)
	$(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(halt_mod_CFLAGS) -MD -c -o $@ $<
-include halt_mod-commands_halt.d

CLEANFILES += cmd-halt_mod-commands_halt.lst fs-halt_mod-commands_halt.lst
COMMANDFILES += cmd-halt_mod-commands_halt.lst
FSFILES += fs-halt_mod-commands_halt.lst

cmd-halt_mod-commands_halt.lst: commands/halt.c $(commands/halt.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(halt_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh halt > $@ || (rm -f $@; exit 1)

fs-halt_mod-commands_halt.lst: commands/halt.c $(commands/halt.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Icommands -I$(srcdir)/commands $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(halt_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh halt > $@ || (rm -f $@; exit 1)


halt_mod_CFLAGS = $(COMMON_CFLAGS)
halt_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For cpuid.mod.
cpuid_mod_SOURCES = commands/i386/cpuid.c
CLEANFILES += cpuid.mod mod-cpuid.o mod-cpuid.c pre-cpuid.o cpuid_mod-commands_i386_cpuid.o und-cpuid.lst
ifneq ($(cpuid_mod_EXPORTS),no)
CLEANFILES += def-cpuid.lst
DEFSYMFILES += def-cpuid.lst
endif
MOSTLYCLEANFILES += cpuid_mod-commands_i386_cpuid.d
UNDSYMFILES += und-cpuid.lst

cpuid.mod: pre-cpuid.o mod-cpuid.o
	-rm -f $@
	$(TARGET_CC) $(cpuid_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-cpuid.o: $(cpuid_mod_DEPENDENCIES) cpuid_mod-commands_i386_cpuid.o
	-rm -f $@
	$(TARGET_CC) $(cpuid_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ cpuid_mod-commands_i386_cpuid.o

mod-cpuid.o: mod-cpuid.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(cpuid_mod_CFLAGS) -c -o $@ $<

mod-cpuid.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'cpuid' $< > $@ || (rm -f $@; exit 1)

ifneq ($(cpuid_mod_EXPORTS),no)
def-cpuid.lst: pre-cpuid.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 cpuid/' > $@
endif

und-cpuid.lst: pre-cpuid.o
	echo 'cpuid' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

cpuid_mod-commands_i386_cpuid.o: commands/i386/cpuid.c $(commands/i386/cpuid.c_DEPENDENCIES)
	$(TARGET_CC) -Icommands/i386 -I$(srcdir)/commands/i386 $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(cpuid_mod_CFLAGS) -MD -c -o $@ $<
-include cpuid_mod-commands_i386_cpuid.d

CLEANFILES += cmd-cpuid_mod-commands_i386_cpuid.lst fs-cpuid_mod-commands_i386_cpuid.lst
COMMANDFILES += cmd-cpuid_mod-commands_i386_cpuid.lst
FSFILES += fs-cpuid_mod-commands_i386_cpuid.lst

cmd-cpuid_mod-commands_i386_cpuid.lst: commands/i386/cpuid.c $(commands/i386/cpuid.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Icommands/i386 -I$(srcdir)/commands/i386 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(cpuid_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh cpuid > $@ || (rm -f $@; exit 1)

fs-cpuid_mod-commands_i386_cpuid.lst: commands/i386/cpuid.c $(commands/i386/cpuid.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Icommands/i386 -I$(srcdir)/commands/i386 $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(cpuid_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh cpuid > $@ || (rm -f $@; exit 1)


cpuid_mod_CFLAGS = $(COMMON_CFLAGS)
cpuid_mod_LDFLAGS = $(COMMON_LDFLAGS)

# For serial.mod.
serial_mod_SOURCES = term/i386/pc/serial.c
CLEANFILES += serial.mod mod-serial.o mod-serial.c pre-serial.o serial_mod-term_i386_pc_serial.o und-serial.lst
ifneq ($(serial_mod_EXPORTS),no)
CLEANFILES += def-serial.lst
DEFSYMFILES += def-serial.lst
endif
MOSTLYCLEANFILES += serial_mod-term_i386_pc_serial.d
UNDSYMFILES += und-serial.lst

serial.mod: pre-serial.o mod-serial.o
	-rm -f $@
	$(TARGET_CC) $(serial_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ $^
	$(STRIP) --strip-unneeded -K grub_mod_init -K grub_mod_fini -R .note -R .comment $@

pre-serial.o: $(serial_mod_DEPENDENCIES) serial_mod-term_i386_pc_serial.o
	-rm -f $@
	$(TARGET_CC) $(serial_mod_LDFLAGS) $(TARGET_LDFLAGS) -Wl,-r,-d -o $@ serial_mod-term_i386_pc_serial.o

mod-serial.o: mod-serial.c
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(serial_mod_CFLAGS) -c -o $@ $<

mod-serial.c: moddep.lst genmodsrc.sh
	sh $(srcdir)/genmodsrc.sh 'serial' $< > $@ || (rm -f $@; exit 1)

ifneq ($(serial_mod_EXPORTS),no)
def-serial.lst: pre-serial.o
	$(NM) -g --defined-only -P -p $< | sed 's/^\([^ ]*\).*/\1 serial/' > $@
endif

und-serial.lst: pre-serial.o
	echo 'serial' > $@
	$(NM) -u -P -p $< | cut -f1 -d' ' >> $@

serial_mod-term_i386_pc_serial.o: term/i386/pc/serial.c $(term/i386/pc/serial.c_DEPENDENCIES)
	$(TARGET_CC) -Iterm/i386/pc -I$(srcdir)/term/i386/pc $(TARGET_CPPFLAGS)  $(TARGET_CFLAGS) $(serial_mod_CFLAGS) -MD -c -o $@ $<
-include serial_mod-term_i386_pc_serial.d

CLEANFILES += cmd-serial_mod-term_i386_pc_serial.lst fs-serial_mod-term_i386_pc_serial.lst
COMMANDFILES += cmd-serial_mod-term_i386_pc_serial.lst
FSFILES += fs-serial_mod-term_i386_pc_serial.lst

cmd-serial_mod-term_i386_pc_serial.lst: term/i386/pc/serial.c $(term/i386/pc/serial.c_DEPENDENCIES) gencmdlist.sh
	set -e; 	  $(TARGET_CC) -Iterm/i386/pc -I$(srcdir)/term/i386/pc $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(serial_mod_CFLAGS) -E $< 	  | sh $(srcdir)/gencmdlist.sh serial > $@ || (rm -f $@; exit 1)

fs-serial_mod-term_i386_pc_serial.lst: term/i386/pc/serial.c $(term/i386/pc/serial.c_DEPENDENCIES) genfslist.sh
	set -e; 	  $(TARGET_CC) -Iterm/i386/pc -I$(srcdir)/term/i386/pc $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(serial_mod_CFLAGS) -E $< 	  | sh $(srcdir)/genfslist.sh serial > $@ || (rm -f $@; exit 1)


serial_mod_CFLAGS = $(COMMON_CFLAGS)
serial_mod_LDFLAGS = $(COMMON_LDFLAGS)

include $(srcdir)/conf/common.mk
