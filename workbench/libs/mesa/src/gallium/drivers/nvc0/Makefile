TOP = ../../../..
include $(TOP)/configs/current

LIBNAME = nvc0

C_SOURCES = \
	nvc0_context.c \
	nvc0_draw.c \
	nvc0_formats.c \
	nvc0_miptree.c \
	nvc0_resource.c \
	nvc0_screen.c \
	nvc0_state.c \
	nvc0_state_validate.c \
	nvc0_surface.c \
	nvc0_tex.c \
	nvc0_transfer.c \
	nvc0_vbo.c \
	nvc0_program.c \
	nvc0_shader_state.c \
	nvc0_pc.c \
	nvc0_pc_print.c \
	nvc0_pc_emit.c \
	nvc0_tgsi_to_nc.c \
	nvc0_pc_optimize.c \
	nvc0_pc_regalloc.c \
	nvc0_push.c \
	nvc0_push2.c \
	nvc0_query.c

LIBRARY_INCLUDES = \
	$(LIBDRM_CFLAGS)

include ../../Makefile.template
