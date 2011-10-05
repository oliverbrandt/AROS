/*
 * Copyright (C) 2005-2007  Brian Paul   All Rights Reserved.
 * Copyright (C) 2008  VMware, Inc.   All Rights Reserved.
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file ir_to_mesa.cpp
 *
 * Translate GLSL IR to Mesa's gl_program representation.
 */

#include <stdio.h>
#include "main/compiler.h"
#include "ir.h"
#include "ir_visitor.h"
#include "ir_print_visitor.h"
#include "ir_expression_flattening.h"
#include "glsl_types.h"
#include "glsl_parser_extras.h"
#include "../glsl/program.h"
#include "ir_optimization.h"
#include "ast.h"

extern "C" {
#include "main/mtypes.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "program/hash_table.h"
#include "program/prog_instruction.h"
#include "program/prog_optimize.h"
#include "program/prog_print.h"
#include "program/program.h"
#include "program/prog_uniform.h"
#include "program/prog_parameter.h"
#include "program/sampler.h"
}

class src_reg;
class dst_reg;

static int swizzle_for_size(int size);

/**
 * This struct is a corresponding struct to Mesa prog_src_register, with
 * wider fields.
 */
class src_reg {
public:
   src_reg(gl_register_file file, int index, const glsl_type *type)
   {
      this->file = file;
      this->index = index;
      if (type && (type->is_scalar() || type->is_vector() || type->is_matrix()))
	 this->swizzle = swizzle_for_size(type->vector_elements);
      else
	 this->swizzle = SWIZZLE_XYZW;
      this->negate = 0;
      this->reladdr = NULL;
   }

   src_reg()
   {
      this->file = PROGRAM_UNDEFINED;
      this->index = 0;
      this->swizzle = 0;
      this->negate = 0;
      this->reladdr = NULL;
   }

   explicit src_reg(dst_reg reg);

   gl_register_file file; /**< PROGRAM_* from Mesa */
   int index; /**< temporary index, VERT_ATTRIB_*, FRAG_ATTRIB_*, etc. */
   GLuint swizzle; /**< SWIZZLE_XYZWONEZERO swizzles from Mesa. */
   int negate; /**< NEGATE_XYZW mask from mesa */
   /** Register index should be offset by the integer in this reg. */
   src_reg *reladdr;
};

class dst_reg {
public:
   dst_reg(gl_register_file file, int writemask)
   {
      this->file = file;
      this->index = 0;
      this->writemask = writemask;
      this->cond_mask = COND_TR;
      this->reladdr = NULL;
   }

   dst_reg()
   {
      this->file = PROGRAM_UNDEFINED;
      this->index = 0;
      this->writemask = 0;
      this->cond_mask = COND_TR;
      this->reladdr = NULL;
   }

   explicit dst_reg(src_reg reg);

   gl_register_file file; /**< PROGRAM_* from Mesa */
   int index; /**< temporary index, VERT_ATTRIB_*, FRAG_ATTRIB_*, etc. */
   int writemask; /**< Bitfield of WRITEMASK_[XYZW] */
   GLuint cond_mask:4;
   /** Register index should be offset by the integer in this reg. */
   src_reg *reladdr;
};

src_reg::src_reg(dst_reg reg)
{
   this->file = reg.file;
   this->index = reg.index;
   this->swizzle = SWIZZLE_XYZW;
   this->negate = 0;
   this->reladdr = reg.reladdr;
}

dst_reg::dst_reg(src_reg reg)
{
   this->file = reg.file;
   this->index = reg.index;
   this->writemask = WRITEMASK_XYZW;
   this->cond_mask = COND_TR;
   this->reladdr = reg.reladdr;
}

class ir_to_mesa_instruction : public exec_node {
public:
   /* Callers of this ralloc-based new need not call delete. It's
    * easier to just ralloc_free 'ctx' (or any of its ancestors). */
   static void* operator new(size_t size, void *ctx)
   {
      void *node;

      node = rzalloc_size(ctx, size);
      assert(node != NULL);

      return node;
   }

   enum prog_opcode op;
   dst_reg dst;
   src_reg src[3];
   /** Pointer to the ir source this tree came from for debugging */
   ir_instruction *ir;
   GLboolean cond_update;
   bool saturate;
   int sampler; /**< sampler index */
   int tex_target; /**< One of TEXTURE_*_INDEX */
   GLboolean tex_shadow;

   class function_entry *function; /* Set on OPCODE_CAL or OPCODE_BGNSUB */
};

class variable_storage : public exec_node {
public:
   variable_storage(ir_variable *var, gl_register_file file, int index)
      : file(file), index(index), var(var)
   {
      /* empty */
   }

   gl_register_file file;
   int index;
   ir_variable *var; /* variable that maps to this, if any */
};

class function_entry : public exec_node {
public:
   ir_function_signature *sig;

   /**
    * identifier of this function signature used by the program.
    *
    * At the point that Mesa instructions for function calls are
    * generated, we don't know the address of the first instruction of
    * the function body.  So we make the BranchTarget that is called a
    * small integer and rewrite them during set_branchtargets().
    */
   int sig_id;

   /**
    * Pointer to first instruction of the function body.
    *
    * Set during function body emits after main() is processed.
    */
   ir_to_mesa_instruction *bgn_inst;

   /**
    * Index of the first instruction of the function body in actual
    * Mesa IR.
    *
    * Set after convertion from ir_to_mesa_instruction to prog_instruction.
    */
   int inst;

   /** Storage for the return value. */
   src_reg return_reg;
};

class ir_to_mesa_visitor : public ir_visitor {
public:
   ir_to_mesa_visitor();
   ~ir_to_mesa_visitor();

   function_entry *current_function;

   struct gl_context *ctx;
   struct gl_program *prog;
   struct gl_shader_program *shader_program;
   struct gl_shader_compiler_options *options;

   int next_temp;

   variable_storage *find_variable_storage(ir_variable *var);

   function_entry *get_function_signature(ir_function_signature *sig);

   src_reg get_temp(const glsl_type *type);
   void reladdr_to_temp(ir_instruction *ir, src_reg *reg, int *num_reladdr);

   src_reg src_reg_for_float(float val);

   /**
    * \name Visit methods
    *
    * As typical for the visitor pattern, there must be one \c visit method for
    * each concrete subclass of \c ir_instruction.  Virtual base classes within
    * the hierarchy should not have \c visit methods.
    */
   /*@{*/
   virtual void visit(ir_variable *);
   virtual void visit(ir_loop *);
   virtual void visit(ir_loop_jump *);
   virtual void visit(ir_function_signature *);
   virtual void visit(ir_function *);
   virtual void visit(ir_expression *);
   virtual void visit(ir_swizzle *);
   virtual void visit(ir_dereference_variable  *);
   virtual void visit(ir_dereference_array *);
   virtual void visit(ir_dereference_record *);
   virtual void visit(ir_assignment *);
   virtual void visit(ir_constant *);
   virtual void visit(ir_call *);
   virtual void visit(ir_return *);
   virtual void visit(ir_discard *);
   virtual void visit(ir_texture *);
   virtual void visit(ir_if *);
   /*@}*/

   src_reg result;

   /** List of variable_storage */
   exec_list variables;

   /** List of function_entry */
   exec_list function_signatures;
   int next_signature_id;

   /** List of ir_to_mesa_instruction */
   exec_list instructions;

   ir_to_mesa_instruction *emit(ir_instruction *ir, enum prog_opcode op);

   ir_to_mesa_instruction *emit(ir_instruction *ir, enum prog_opcode op,
			        dst_reg dst, src_reg src0);

   ir_to_mesa_instruction *emit(ir_instruction *ir, enum prog_opcode op,
			        dst_reg dst, src_reg src0, src_reg src1);

   ir_to_mesa_instruction *emit(ir_instruction *ir, enum prog_opcode op,
			        dst_reg dst,
			        src_reg src0, src_reg src1, src_reg src2);

   /**
    * Emit the correct dot-product instruction for the type of arguments
    */
   void emit_dp(ir_instruction *ir,
	        dst_reg dst,
	        src_reg src0,
	        src_reg src1,
	        unsigned elements);

   void emit_scalar(ir_instruction *ir, enum prog_opcode op,
		    dst_reg dst, src_reg src0);

   void emit_scalar(ir_instruction *ir, enum prog_opcode op,
		    dst_reg dst, src_reg src0, src_reg src1);

   void emit_scs(ir_instruction *ir, enum prog_opcode op,
		 dst_reg dst, const src_reg &src);

   GLboolean try_emit_mad(ir_expression *ir,
			  int mul_operand);
   GLboolean try_emit_sat(ir_expression *ir);

   void emit_swz(ir_expression *ir);

   bool process_move_condition(ir_rvalue *ir);

   void copy_propagate(void);

   void *mem_ctx;
};

src_reg undef_src = src_reg(PROGRAM_UNDEFINED, 0, NULL);

dst_reg undef_dst = dst_reg(PROGRAM_UNDEFINED, SWIZZLE_NOOP);

dst_reg address_reg = dst_reg(PROGRAM_ADDRESS, WRITEMASK_X);

static void
fail_link(struct gl_shader_program *prog, const char *fmt, ...) PRINTFLIKE(2, 3);

static void
fail_link(struct gl_shader_program *prog, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   ralloc_vasprintf_append(&prog->InfoLog, fmt, args);
   va_end(args);

   prog->LinkStatus = GL_FALSE;
}

static int
swizzle_for_size(int size)
{
   int size_swizzles[4] = {
      MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X),
      MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Y, SWIZZLE_Y),
      MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_Z),
      MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_W),
   };

   assert((size >= 1) && (size <= 4));
   return size_swizzles[size - 1];
}

ir_to_mesa_instruction *
ir_to_mesa_visitor::emit(ir_instruction *ir, enum prog_opcode op,
			 dst_reg dst,
			 src_reg src0, src_reg src1, src_reg src2)
{
   ir_to_mesa_instruction *inst = new(mem_ctx) ir_to_mesa_instruction();
   int num_reladdr = 0;

   /* If we have to do relative addressing, we want to load the ARL
    * reg directly for one of the regs, and preload the other reladdr
    * sources into temps.
    */
   num_reladdr += dst.reladdr != NULL;
   num_reladdr += src0.reladdr != NULL;
   num_reladdr += src1.reladdr != NULL;
   num_reladdr += src2.reladdr != NULL;

   reladdr_to_temp(ir, &src2, &num_reladdr);
   reladdr_to_temp(ir, &src1, &num_reladdr);
   reladdr_to_temp(ir, &src0, &num_reladdr);

   if (dst.reladdr) {
      emit(ir, OPCODE_ARL, address_reg, *dst.reladdr);
      num_reladdr--;
   }
   assert(num_reladdr == 0);

   inst->op = op;
   inst->dst = dst;
   inst->src[0] = src0;
   inst->src[1] = src1;
   inst->src[2] = src2;
   inst->ir = ir;

   inst->function = NULL;

   this->instructions.push_tail(inst);

   return inst;
}


ir_to_mesa_instruction *
ir_to_mesa_visitor::emit(ir_instruction *ir, enum prog_opcode op,
			 dst_reg dst, src_reg src0, src_reg src1)
{
   return emit(ir, op, dst, src0, src1, undef_src);
}

ir_to_mesa_instruction *
ir_to_mesa_visitor::emit(ir_instruction *ir, enum prog_opcode op,
			 dst_reg dst, src_reg src0)
{
   assert(dst.writemask != 0);
   return emit(ir, op, dst, src0, undef_src, undef_src);
}

ir_to_mesa_instruction *
ir_to_mesa_visitor::emit(ir_instruction *ir, enum prog_opcode op)
{
   return emit(ir, op, undef_dst, undef_src, undef_src, undef_src);
}

void
ir_to_mesa_visitor::emit_dp(ir_instruction *ir,
			    dst_reg dst, src_reg src0, src_reg src1,
			    unsigned elements)
{
   static const gl_inst_opcode dot_opcodes[] = {
      OPCODE_DP2, OPCODE_DP3, OPCODE_DP4
   };

   emit(ir, dot_opcodes[elements - 2], dst, src0, src1);
}

/**
 * Emits Mesa scalar opcodes to produce unique answers across channels.
 *
 * Some Mesa opcodes are scalar-only, like ARB_fp/vp.  The src X
 * channel determines the result across all channels.  So to do a vec4
 * of this operation, we want to emit a scalar per source channel used
 * to produce dest channels.
 */
void
ir_to_mesa_visitor::emit_scalar(ir_instruction *ir, enum prog_opcode op,
			        dst_reg dst,
				src_reg orig_src0, src_reg orig_src1)
{
   int i, j;
   int done_mask = ~dst.writemask;

   /* Mesa RCP is a scalar operation splatting results to all channels,
    * like ARB_fp/vp.  So emit as many RCPs as necessary to cover our
    * dst channels.
    */
   for (i = 0; i < 4; i++) {
      GLuint this_mask = (1 << i);
      ir_to_mesa_instruction *inst;
      src_reg src0 = orig_src0;
      src_reg src1 = orig_src1;

      if (done_mask & this_mask)
	 continue;

      GLuint src0_swiz = GET_SWZ(src0.swizzle, i);
      GLuint src1_swiz = GET_SWZ(src1.swizzle, i);
      for (j = i + 1; j < 4; j++) {
	 /* If there is another enabled component in the destination that is
	  * derived from the same inputs, generate its value on this pass as
	  * well.
	  */
	 if (!(done_mask & (1 << j)) &&
	     GET_SWZ(src0.swizzle, j) == src0_swiz &&
	     GET_SWZ(src1.swizzle, j) == src1_swiz) {
	    this_mask |= (1 << j);
	 }
      }
      src0.swizzle = MAKE_SWIZZLE4(src0_swiz, src0_swiz,
				   src0_swiz, src0_swiz);
      src1.swizzle = MAKE_SWIZZLE4(src1_swiz, src1_swiz,
				  src1_swiz, src1_swiz);

      inst = emit(ir, op, dst, src0, src1);
      inst->dst.writemask = this_mask;
      done_mask |= this_mask;
   }
}

void
ir_to_mesa_visitor::emit_scalar(ir_instruction *ir, enum prog_opcode op,
			        dst_reg dst, src_reg src0)
{
   src_reg undef = undef_src;

   undef.swizzle = SWIZZLE_XXXX;

   emit_scalar(ir, op, dst, src0, undef);
}

/**
 * Emit an OPCODE_SCS instruction
 *
 * The \c SCS opcode functions a bit differently than the other Mesa (or
 * ARB_fragment_program) opcodes.  Instead of splatting its result across all
 * four components of the destination, it writes one value to the \c x
 * component and another value to the \c y component.
 *
 * \param ir        IR instruction being processed
 * \param op        Either \c OPCODE_SIN or \c OPCODE_COS depending on which
 *                  value is desired.
 * \param dst       Destination register
 * \param src       Source register
 */
void
ir_to_mesa_visitor::emit_scs(ir_instruction *ir, enum prog_opcode op,
			     dst_reg dst,
			     const src_reg &src)
{
   /* Vertex programs cannot use the SCS opcode.
    */
   if (this->prog->Target == GL_VERTEX_PROGRAM_ARB) {
      emit_scalar(ir, op, dst, src);
      return;
   }

   const unsigned component = (op == OPCODE_SIN) ? 0 : 1;
   const unsigned scs_mask = (1U << component);
   int done_mask = ~dst.writemask;
   src_reg tmp;

   assert(op == OPCODE_SIN || op == OPCODE_COS);

   /* If there are compnents in the destination that differ from the component
    * that will be written by the SCS instrution, we'll need a temporary.
    */
   if (scs_mask != unsigned(dst.writemask)) {
      tmp = get_temp(glsl_type::vec4_type);
   }

   for (unsigned i = 0; i < 4; i++) {
      unsigned this_mask = (1U << i);
      src_reg src0 = src;

      if ((done_mask & this_mask) != 0)
	 continue;

      /* The source swizzle specified which component of the source generates
       * sine / cosine for the current component in the destination.  The SCS
       * instruction requires that this value be swizzle to the X component.
       * Replace the current swizzle with a swizzle that puts the source in
       * the X component.
       */
      unsigned src0_swiz = GET_SWZ(src.swizzle, i);

      src0.swizzle = MAKE_SWIZZLE4(src0_swiz, src0_swiz,
				   src0_swiz, src0_swiz);
      for (unsigned j = i + 1; j < 4; j++) {
	 /* If there is another enabled component in the destination that is
	  * derived from the same inputs, generate its value on this pass as
	  * well.
	  */
	 if (!(done_mask & (1 << j)) &&
	     GET_SWZ(src0.swizzle, j) == src0_swiz) {
	    this_mask |= (1 << j);
	 }
      }

      if (this_mask != scs_mask) {
	 ir_to_mesa_instruction *inst;
	 dst_reg tmp_dst = dst_reg(tmp);

	 /* Emit the SCS instruction.
	  */
	 inst = emit(ir, OPCODE_SCS, tmp_dst, src0);
	 inst->dst.writemask = scs_mask;

	 /* Move the result of the SCS instruction to the desired location in
	  * the destination.
	  */
	 tmp.swizzle = MAKE_SWIZZLE4(component, component,
				     component, component);
	 inst = emit(ir, OPCODE_SCS, dst, tmp);
	 inst->dst.writemask = this_mask;
      } else {
	 /* Emit the SCS instruction to write directly to the destination.
	  */
	 ir_to_mesa_instruction *inst = emit(ir, OPCODE_SCS, dst, src0);
	 inst->dst.writemask = scs_mask;
      }

      done_mask |= this_mask;
   }
}

struct src_reg
ir_to_mesa_visitor::src_reg_for_float(float val)
{
   src_reg src(PROGRAM_CONSTANT, -1, NULL);

   src.index = _mesa_add_unnamed_constant(this->prog->Parameters,
					  &val, 1, &src.swizzle);

   return src;
}

static int
type_size(const struct glsl_type *type)
{
   unsigned int i;
   int size;

   switch (type->base_type) {
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_FLOAT:
   case GLSL_TYPE_BOOL:
      if (type->is_matrix()) {
	 return type->matrix_columns;
      } else {
	 /* Regardless of size of vector, it gets a vec4. This is bad
	  * packing for things like floats, but otherwise arrays become a
	  * mess.  Hopefully a later pass over the code can pack scalars
	  * down if appropriate.
	  */
	 return 1;
      }
   case GLSL_TYPE_ARRAY:
      assert(type->length > 0);
      return type_size(type->fields.array) * type->length;
   case GLSL_TYPE_STRUCT:
      size = 0;
      for (i = 0; i < type->length; i++) {
	 size += type_size(type->fields.structure[i].type);
      }
      return size;
   case GLSL_TYPE_SAMPLER:
      /* Samplers take up one slot in UNIFORMS[], but they're baked in
       * at link time.
       */
      return 1;
   default:
      assert(0);
      return 0;
   }
}

/**
 * In the initial pass of codegen, we assign temporary numbers to
 * intermediate results.  (not SSA -- variable assignments will reuse
 * storage).  Actual register allocation for the Mesa VM occurs in a
 * pass over the Mesa IR later.
 */
src_reg
ir_to_mesa_visitor::get_temp(const glsl_type *type)
{
   src_reg src;
   int swizzle[4];
   int i;

   src.file = PROGRAM_TEMPORARY;
   src.index = next_temp;
   src.reladdr = NULL;
   next_temp += type_size(type);

   if (type->is_array() || type->is_record()) {
      src.swizzle = SWIZZLE_NOOP;
   } else {
      for (i = 0; i < type->vector_elements; i++)
	 swizzle[i] = i;
      for (; i < 4; i++)
	 swizzle[i] = type->vector_elements - 1;
      src.swizzle = MAKE_SWIZZLE4(swizzle[0], swizzle[1],
				  swizzle[2], swizzle[3]);
   }
   src.negate = 0;

   return src;
}

variable_storage *
ir_to_mesa_visitor::find_variable_storage(ir_variable *var)
{
   
   variable_storage *entry;

   foreach_iter(exec_list_iterator, iter, this->variables) {
      entry = (variable_storage *)iter.get();

      if (entry->var == var)
	 return entry;
   }

   return NULL;
}

void
ir_to_mesa_visitor::visit(ir_variable *ir)
{
   if (strcmp(ir->name, "gl_FragCoord") == 0) {
      struct gl_fragment_program *fp = (struct gl_fragment_program *)this->prog;

      fp->OriginUpperLeft = ir->origin_upper_left;
      fp->PixelCenterInteger = ir->pixel_center_integer;

   } else if (strcmp(ir->name, "gl_FragDepth") == 0) {
      struct gl_fragment_program *fp = (struct gl_fragment_program *)this->prog;
      switch (ir->depth_layout) {
      case ir_depth_layout_none:
	 fp->FragDepthLayout = FRAG_DEPTH_LAYOUT_NONE;
	 break;
      case ir_depth_layout_any:
	 fp->FragDepthLayout = FRAG_DEPTH_LAYOUT_ANY;
	 break;
      case ir_depth_layout_greater:
	 fp->FragDepthLayout = FRAG_DEPTH_LAYOUT_GREATER;
	 break;
      case ir_depth_layout_less:
	 fp->FragDepthLayout = FRAG_DEPTH_LAYOUT_LESS;
	 break;
      case ir_depth_layout_unchanged:
	 fp->FragDepthLayout = FRAG_DEPTH_LAYOUT_UNCHANGED;
	 break;
      default:
	 assert(0);
	 break;
      }
   }

   if (ir->mode == ir_var_uniform && strncmp(ir->name, "gl_", 3) == 0) {
      unsigned int i;
      const ir_state_slot *const slots = ir->state_slots;
      assert(ir->state_slots != NULL);

      /* Check if this statevar's setup in the STATE file exactly
       * matches how we'll want to reference it as a
       * struct/array/whatever.  If not, then we need to move it into
       * temporary storage and hope that it'll get copy-propagated
       * out.
       */
      for (i = 0; i < ir->num_state_slots; i++) {
	 if (slots[i].swizzle != SWIZZLE_XYZW) {
	    break;
	 }
      }

      struct variable_storage *storage;
      dst_reg dst;
      if (i == ir->num_state_slots) {
	 /* We'll set the index later. */
	 storage = new(mem_ctx) variable_storage(ir, PROGRAM_STATE_VAR, -1);
	 this->variables.push_tail(storage);

	 dst = undef_dst;
      } else {
	 /* The variable_storage constructor allocates slots based on the size
	  * of the type.  However, this had better match the number of state
	  * elements that we're going to copy into the new temporary.
	  */
	 assert((int) ir->num_state_slots == type_size(ir->type));

	 storage = new(mem_ctx) variable_storage(ir, PROGRAM_TEMPORARY,
						 this->next_temp);
	 this->variables.push_tail(storage);
	 this->next_temp += type_size(ir->type);

	 dst = dst_reg(src_reg(PROGRAM_TEMPORARY, storage->index, NULL));
      }


      for (unsigned int i = 0; i < ir->num_state_slots; i++) {
	 int index = _mesa_add_state_reference(this->prog->Parameters,
					       (gl_state_index *)slots[i].tokens);

	 if (storage->file == PROGRAM_STATE_VAR) {
	    if (storage->index == -1) {
	       storage->index = index;
	    } else {
	       assert(index == storage->index + (int)i);
	    }
	 } else {
	    src_reg src(PROGRAM_STATE_VAR, index, NULL);
	    src.swizzle = slots[i].swizzle;
	    emit(ir, OPCODE_MOV, dst, src);
	    /* even a float takes up a whole vec4 reg in a struct/array. */
	    dst.index++;
	 }
      }

      if (storage->file == PROGRAM_TEMPORARY &&
	  dst.index != storage->index + (int) ir->num_state_slots) {
	 fail_link(this->shader_program,
		   "failed to load builtin uniform `%s'  (%d/%d regs loaded)\n",
		   ir->name, dst.index - storage->index,
		   type_size(ir->type));
      }
   }
}

void
ir_to_mesa_visitor::visit(ir_loop *ir)
{
   ir_dereference_variable *counter = NULL;

   if (ir->counter != NULL)
      counter = new(mem_ctx) ir_dereference_variable(ir->counter);

   if (ir->from != NULL) {
      assert(ir->counter != NULL);

      ir_assignment *a =
	new(mem_ctx) ir_assignment(counter, ir->from, NULL);

      a->accept(this);
   }

   emit(NULL, OPCODE_BGNLOOP);

   if (ir->to) {
      ir_expression *e =
	 new(mem_ctx) ir_expression(ir->cmp, glsl_type::bool_type,
					  counter, ir->to);
      ir_if *if_stmt =  new(mem_ctx) ir_if(e);

      ir_loop_jump *brk =
	new(mem_ctx) ir_loop_jump(ir_loop_jump::jump_break);

      if_stmt->then_instructions.push_tail(brk);

      if_stmt->accept(this);
   }

   visit_exec_list(&ir->body_instructions, this);

   if (ir->increment) {
      ir_expression *e =
	 new(mem_ctx) ir_expression(ir_binop_add, counter->type,
					  counter, ir->increment);

      ir_assignment *a =
	new(mem_ctx) ir_assignment(counter, e, NULL);

      a->accept(this);
   }

   emit(NULL, OPCODE_ENDLOOP);
}

void
ir_to_mesa_visitor::visit(ir_loop_jump *ir)
{
   switch (ir->mode) {
   case ir_loop_jump::jump_break:
      emit(NULL, OPCODE_BRK);
      break;
   case ir_loop_jump::jump_continue:
      emit(NULL, OPCODE_CONT);
      break;
   }
}


void
ir_to_mesa_visitor::visit(ir_function_signature *ir)
{
   assert(0);
   (void)ir;
}

void
ir_to_mesa_visitor::visit(ir_function *ir)
{
   /* Ignore function bodies other than main() -- we shouldn't see calls to
    * them since they should all be inlined before we get to ir_to_mesa.
    */
   if (strcmp(ir->name, "main") == 0) {
      const ir_function_signature *sig;
      exec_list empty;

      sig = ir->matching_signature(&empty);

      assert(sig);

      foreach_iter(exec_list_iterator, iter, sig->body) {
	 ir_instruction *ir = (ir_instruction *)iter.get();

	 ir->accept(this);
      }
   }
}

GLboolean
ir_to_mesa_visitor::try_emit_mad(ir_expression *ir, int mul_operand)
{
   int nonmul_operand = 1 - mul_operand;
   src_reg a, b, c;

   ir_expression *expr = ir->operands[mul_operand]->as_expression();
   if (!expr || expr->operation != ir_binop_mul)
      return false;

   expr->operands[0]->accept(this);
   a = this->result;
   expr->operands[1]->accept(this);
   b = this->result;
   ir->operands[nonmul_operand]->accept(this);
   c = this->result;

   this->result = get_temp(ir->type);
   emit(ir, OPCODE_MAD, dst_reg(this->result), a, b, c);

   return true;
}

GLboolean
ir_to_mesa_visitor::try_emit_sat(ir_expression *ir)
{
   /* Saturates were only introduced to vertex programs in
    * NV_vertex_program3, so don't give them to drivers in the VP.
    */
   if (this->prog->Target == GL_VERTEX_PROGRAM_ARB)
      return false;

   ir_rvalue *sat_src = ir->as_rvalue_to_saturate();
   if (!sat_src)
      return false;

   sat_src->accept(this);
   src_reg src = this->result;

   this->result = get_temp(ir->type);
   ir_to_mesa_instruction *inst;
   inst = emit(ir, OPCODE_MOV, dst_reg(this->result), src);
   inst->saturate = true;

   return true;
}

void
ir_to_mesa_visitor::reladdr_to_temp(ir_instruction *ir,
				    src_reg *reg, int *num_reladdr)
{
   if (!reg->reladdr)
      return;

   emit(ir, OPCODE_ARL, address_reg, *reg->reladdr);

   if (*num_reladdr != 1) {
      src_reg temp = get_temp(glsl_type::vec4_type);

      emit(ir, OPCODE_MOV, dst_reg(temp), *reg);
      *reg = temp;
   }

   (*num_reladdr)--;
}

void
ir_to_mesa_visitor::emit_swz(ir_expression *ir)
{
   /* Assume that the vector operator is in a form compatible with OPCODE_SWZ.
    * This means that each of the operands is either an immediate value of -1,
    * 0, or 1, or is a component from one source register (possibly with
    * negation).
    */
   uint8_t components[4] = { 0 };
   bool negate[4] = { false };
   ir_variable *var = NULL;

   for (unsigned i = 0; i < ir->type->vector_elements; i++) {
      ir_rvalue *op = ir->operands[i];

      assert(op->type->is_scalar());

      while (op != NULL) {
	 switch (op->ir_type) {
	 case ir_type_constant: {

	    assert(op->type->is_scalar());

	    const ir_constant *const c = op->as_constant();
	    if (c->is_one()) {
	       components[i] = SWIZZLE_ONE;
	    } else if (c->is_zero()) {
	       components[i] = SWIZZLE_ZERO;
	    } else if (c->is_negative_one()) {
	       components[i] = SWIZZLE_ONE;
	       negate[i] = true;
	    } else {
	       assert(!"SWZ constant must be 0.0 or 1.0.");
	    }

	    op = NULL;
	    break;
	 }

	 case ir_type_dereference_variable: {
	    ir_dereference_variable *const deref =
	       (ir_dereference_variable *) op;

	    assert((var == NULL) || (deref->var == var));
	    components[i] = SWIZZLE_X;
	    var = deref->var;
	    op = NULL;
	    break;
	 }

	 case ir_type_expression: {
	    ir_expression *const expr = (ir_expression *) op;

	    assert(expr->operation == ir_unop_neg);
	    negate[i] = true;

	    op = expr->operands[0];
	    break;
	 }

	 case ir_type_swizzle: {
	    ir_swizzle *const swiz = (ir_swizzle *) op;

	    components[i] = swiz->mask.x;
	    op = swiz->val;
	    break;
	 }

	 default:
	    assert(!"Should not get here.");
	    return;
	 }
      }
   }

   assert(var != NULL);

   ir_dereference_variable *const deref =
      new(mem_ctx) ir_dereference_variable(var);

   this->result.file = PROGRAM_UNDEFINED;
   deref->accept(this);
   if (this->result.file == PROGRAM_UNDEFINED) {
      ir_print_visitor v;
      printf("Failed to get tree for expression operand:\n");
      deref->accept(&v);
      exit(1);
   }

   src_reg src;

   src = this->result;
   src.swizzle = MAKE_SWIZZLE4(components[0],
			       components[1],
			       components[2],
			       components[3]);
   src.negate = ((unsigned(negate[0]) << 0)
		 | (unsigned(negate[1]) << 1)
		 | (unsigned(negate[2]) << 2)
		 | (unsigned(negate[3]) << 3));

   /* Storage for our result.  Ideally for an assignment we'd be using the
    * actual storage for the result here, instead.
    */
   const src_reg result_src = get_temp(ir->type);
   dst_reg result_dst = dst_reg(result_src);

   /* Limit writes to the channels that will be used by result_src later.
    * This does limit this temp's use as a temporary for multi-instruction
    * sequences.
    */
   result_dst.writemask = (1 << ir->type->vector_elements) - 1;

   emit(ir, OPCODE_SWZ, result_dst, src);
   this->result = result_src;
}

void
ir_to_mesa_visitor::visit(ir_expression *ir)
{
   unsigned int operand;
   src_reg op[Elements(ir->operands)];
   src_reg result_src;
   dst_reg result_dst;

   /* Quick peephole: Emit OPCODE_MAD(a, b, c) instead of ADD(MUL(a, b), c)
    */
   if (ir->operation == ir_binop_add) {
      if (try_emit_mad(ir, 1))
	 return;
      if (try_emit_mad(ir, 0))
	 return;
   }
   if (try_emit_sat(ir))
      return;

   if (ir->operation == ir_quadop_vector) {
      this->emit_swz(ir);
      return;
   }

   for (operand = 0; operand < ir->get_num_operands(); operand++) {
      this->result.file = PROGRAM_UNDEFINED;
      ir->operands[operand]->accept(this);
      if (this->result.file == PROGRAM_UNDEFINED) {
	 ir_print_visitor v;
	 printf("Failed to get tree for expression operand:\n");
	 ir->operands[operand]->accept(&v);
	 exit(1);
      }
      op[operand] = this->result;

      /* Matrix expression operands should have been broken down to vector
       * operations already.
       */
      assert(!ir->operands[operand]->type->is_matrix());
   }

   int vector_elements = ir->operands[0]->type->vector_elements;
   if (ir->operands[1]) {
      vector_elements = MAX2(vector_elements,
			     ir->operands[1]->type->vector_elements);
   }

   this->result.file = PROGRAM_UNDEFINED;

   /* Storage for our result.  Ideally for an assignment we'd be using
    * the actual storage for the result here, instead.
    */
   result_src = get_temp(ir->type);
   /* convenience for the emit functions below. */
   result_dst = dst_reg(result_src);
   /* Limit writes to the channels that will be used by result_src later.
    * This does limit this temp's use as a temporary for multi-instruction
    * sequences.
    */
   result_dst.writemask = (1 << ir->type->vector_elements) - 1;

   switch (ir->operation) {
   case ir_unop_logic_not:
      emit(ir, OPCODE_SEQ, result_dst, op[0], src_reg_for_float(0.0));
      break;
   case ir_unop_neg:
      op[0].negate = ~op[0].negate;
      result_src = op[0];
      break;
   case ir_unop_abs:
      emit(ir, OPCODE_ABS, result_dst, op[0]);
      break;
   case ir_unop_sign:
      emit(ir, OPCODE_SSG, result_dst, op[0]);
      break;
   case ir_unop_rcp:
      emit_scalar(ir, OPCODE_RCP, result_dst, op[0]);
      break;

   case ir_unop_exp2:
      emit_scalar(ir, OPCODE_EX2, result_dst, op[0]);
      break;
   case ir_unop_exp:
   case ir_unop_log:
      assert(!"not reached: should be handled by ir_explog_to_explog2");
      break;
   case ir_unop_log2:
      emit_scalar(ir, OPCODE_LG2, result_dst, op[0]);
      break;
   case ir_unop_sin:
      emit_scalar(ir, OPCODE_SIN, result_dst, op[0]);
      break;
   case ir_unop_cos:
      emit_scalar(ir, OPCODE_COS, result_dst, op[0]);
      break;
   case ir_unop_sin_reduced:
      emit_scs(ir, OPCODE_SIN, result_dst, op[0]);
      break;
   case ir_unop_cos_reduced:
      emit_scs(ir, OPCODE_COS, result_dst, op[0]);
      break;

   case ir_unop_dFdx:
      emit(ir, OPCODE_DDX, result_dst, op[0]);
      break;
   case ir_unop_dFdy:
      emit(ir, OPCODE_DDY, result_dst, op[0]);
      break;

   case ir_unop_noise: {
      const enum prog_opcode opcode =
	 prog_opcode(OPCODE_NOISE1
		     + (ir->operands[0]->type->vector_elements) - 1);
      assert((opcode >= OPCODE_NOISE1) && (opcode <= OPCODE_NOISE4));

      emit(ir, opcode, result_dst, op[0]);
      break;
   }

   case ir_binop_add:
      emit(ir, OPCODE_ADD, result_dst, op[0], op[1]);
      break;
   case ir_binop_sub:
      emit(ir, OPCODE_SUB, result_dst, op[0], op[1]);
      break;

   case ir_binop_mul:
      emit(ir, OPCODE_MUL, result_dst, op[0], op[1]);
      break;
   case ir_binop_div:
      assert(!"not reached: should be handled by ir_div_to_mul_rcp");
   case ir_binop_mod:
      assert(!"ir_binop_mod should have been converted to b * fract(a/b)");
      break;

   case ir_binop_less:
      emit(ir, OPCODE_SLT, result_dst, op[0], op[1]);
      break;
   case ir_binop_greater:
      emit(ir, OPCODE_SGT, result_dst, op[0], op[1]);
      break;
   case ir_binop_lequal:
      emit(ir, OPCODE_SLE, result_dst, op[0], op[1]);
      break;
   case ir_binop_gequal:
      emit(ir, OPCODE_SGE, result_dst, op[0], op[1]);
      break;
   case ir_binop_equal:
      emit(ir, OPCODE_SEQ, result_dst, op[0], op[1]);
      break;
   case ir_binop_nequal:
      emit(ir, OPCODE_SNE, result_dst, op[0], op[1]);
      break;
   case ir_binop_all_equal:
      /* "==" operator producing a scalar boolean. */
      if (ir->operands[0]->type->is_vector() ||
	  ir->operands[1]->type->is_vector()) {
	 src_reg temp = get_temp(glsl_type::vec4_type);
	 emit(ir, OPCODE_SNE, dst_reg(temp), op[0], op[1]);
	 emit_dp(ir, result_dst, temp, temp, vector_elements);
	 emit(ir, OPCODE_SEQ, result_dst, result_src, src_reg_for_float(0.0));
      } else {
	 emit(ir, OPCODE_SEQ, result_dst, op[0], op[1]);
      }
      break;
   case ir_binop_any_nequal:
      /* "!=" operator producing a scalar boolean. */
      if (ir->operands[0]->type->is_vector() ||
	  ir->operands[1]->type->is_vector()) {
	 src_reg temp = get_temp(glsl_type::vec4_type);
	 emit(ir, OPCODE_SNE, dst_reg(temp), op[0], op[1]);
	 emit_dp(ir, result_dst, temp, temp, vector_elements);
	 emit(ir, OPCODE_SNE, result_dst, result_src, src_reg_for_float(0.0));
      } else {
	 emit(ir, OPCODE_SNE, result_dst, op[0], op[1]);
      }
      break;

   case ir_unop_any:
      assert(ir->operands[0]->type->is_vector());
      emit_dp(ir, result_dst, op[0], op[0],
	      ir->operands[0]->type->vector_elements);
      emit(ir, OPCODE_SNE, result_dst, result_src, src_reg_for_float(0.0));
      break;

   case ir_binop_logic_xor:
      emit(ir, OPCODE_SNE, result_dst, op[0], op[1]);
      break;

   case ir_binop_logic_or:
      /* This could be a saturated add and skip the SNE. */
      emit(ir, OPCODE_ADD, result_dst, op[0], op[1]);
      emit(ir, OPCODE_SNE, result_dst, result_src, src_reg_for_float(0.0));
      break;

   case ir_binop_logic_and:
      /* the bool args are stored as float 0.0 or 1.0, so "mul" gives us "and". */
      emit(ir, OPCODE_MUL, result_dst, op[0], op[1]);
      break;

   case ir_binop_dot:
      assert(ir->operands[0]->type->is_vector());
      assert(ir->operands[0]->type == ir->operands[1]->type);
      emit_dp(ir, result_dst, op[0], op[1],
	      ir->operands[0]->type->vector_elements);
      break;

   case ir_unop_sqrt:
      /* sqrt(x) = x * rsq(x). */
      emit_scalar(ir, OPCODE_RSQ, result_dst, op[0]);
      emit(ir, OPCODE_MUL, result_dst, result_src, op[0]);
      /* For incoming channels <= 0, set the result to 0. */
      op[0].negate = ~op[0].negate;
      emit(ir, OPCODE_CMP, result_dst,
			  op[0], result_src, src_reg_for_float(0.0));
      break;
   case ir_unop_rsq:
      emit_scalar(ir, OPCODE_RSQ, result_dst, op[0]);
      break;
   case ir_unop_i2f:
   case ir_unop_b2f:
   case ir_unop_b2i:
      /* Mesa IR lacks types, ints are stored as truncated floats. */
      result_src = op[0];
      break;
   case ir_unop_f2i:
      emit(ir, OPCODE_TRUNC, result_dst, op[0]);
      break;
   case ir_unop_f2b:
   case ir_unop_i2b:
      emit(ir, OPCODE_SNE, result_dst,
			  op[0], src_reg_for_float(0.0));
      break;
   case ir_unop_trunc:
      emit(ir, OPCODE_TRUNC, result_dst, op[0]);
      break;
   case ir_unop_ceil:
      op[0].negate = ~op[0].negate;
      emit(ir, OPCODE_FLR, result_dst, op[0]);
      result_src.negate = ~result_src.negate;
      break;
   case ir_unop_floor:
      emit(ir, OPCODE_FLR, result_dst, op[0]);
      break;
   case ir_unop_fract:
      emit(ir, OPCODE_FRC, result_dst, op[0]);
      break;

   case ir_binop_min:
      emit(ir, OPCODE_MIN, result_dst, op[0], op[1]);
      break;
   case ir_binop_max:
      emit(ir, OPCODE_MAX, result_dst, op[0], op[1]);
      break;
   case ir_binop_pow:
      emit_scalar(ir, OPCODE_POW, result_dst, op[0], op[1]);
      break;

   case ir_unop_bit_not:
   case ir_unop_u2f:
   case ir_binop_lshift:
   case ir_binop_rshift:
   case ir_binop_bit_and:
   case ir_binop_bit_xor:
   case ir_binop_bit_or:
   case ir_unop_round_even:
      assert(!"GLSL 1.30 features unsupported");
      break;

   case ir_quadop_vector:
      /* This operation should have already been handled.
       */
      assert(!"Should not get here.");
      break;
   }

   this->result = result_src;
}


void
ir_to_mesa_visitor::visit(ir_swizzle *ir)
{
   src_reg src;
   int i;
   int swizzle[4];

   /* Note that this is only swizzles in expressions, not those on the left
    * hand side of an assignment, which do write masking.  See ir_assignment
    * for that.
    */

   ir->val->accept(this);
   src = this->result;
   assert(src.file != PROGRAM_UNDEFINED);

   for (i = 0; i < 4; i++) {
      if (i < ir->type->vector_elements) {
	 switch (i) {
	 case 0:
	    swizzle[i] = GET_SWZ(src.swizzle, ir->mask.x);
	    break;
	 case 1:
	    swizzle[i] = GET_SWZ(src.swizzle, ir->mask.y);
	    break;
	 case 2:
	    swizzle[i] = GET_SWZ(src.swizzle, ir->mask.z);
	    break;
	 case 3:
	    swizzle[i] = GET_SWZ(src.swizzle, ir->mask.w);
	    break;
	 }
      } else {
	 /* If the type is smaller than a vec4, replicate the last
	  * channel out.
	  */
	 swizzle[i] = swizzle[ir->type->vector_elements - 1];
      }
   }

   src.swizzle = MAKE_SWIZZLE4(swizzle[0], swizzle[1], swizzle[2], swizzle[3]);

   this->result = src;
}

void
ir_to_mesa_visitor::visit(ir_dereference_variable *ir)
{
   variable_storage *entry = find_variable_storage(ir->var);
   ir_variable *var = ir->var;

   if (!entry) {
      switch (var->mode) {
      case ir_var_uniform:
	 entry = new(mem_ctx) variable_storage(var, PROGRAM_UNIFORM,
					       var->location);
	 this->variables.push_tail(entry);
	 break;
      case ir_var_in:
      case ir_var_inout:
	 /* The linker assigns locations for varyings and attributes,
	  * including deprecated builtins (like gl_Color), user-assign
	  * generic attributes (glBindVertexLocation), and
	  * user-defined varyings.
	  *
	  * FINISHME: We would hit this path for function arguments.  Fix!
	  */
	 assert(var->location != -1);
         entry = new(mem_ctx) variable_storage(var,
                                               PROGRAM_INPUT,
                                               var->location);
         if (this->prog->Target == GL_VERTEX_PROGRAM_ARB &&
             var->location >= VERT_ATTRIB_GENERIC0) {
            _mesa_add_attribute(this->prog->Attributes,
                                var->name,
                                _mesa_sizeof_glsl_type(var->type->gl_type),
                                var->type->gl_type,
                                var->location - VERT_ATTRIB_GENERIC0);
         }
         break;
      case ir_var_out:
	 assert(var->location != -1);
         entry = new(mem_ctx) variable_storage(var,
                                               PROGRAM_OUTPUT,
                                               var->location);
	 break;
      case ir_var_system_value:
         entry = new(mem_ctx) variable_storage(var,
                                               PROGRAM_SYSTEM_VALUE,
                                               var->location);
         break;
      case ir_var_auto:
      case ir_var_temporary:
	 entry = new(mem_ctx) variable_storage(var, PROGRAM_TEMPORARY,
					       this->next_temp);
	 this->variables.push_tail(entry);

	 next_temp += type_size(var->type);
	 break;
      }

      if (!entry) {
	 printf("Failed to make storage for %s\n", var->name);
	 exit(1);
      }
   }

   this->result = src_reg(entry->file, entry->index, var->type);
}

void
ir_to_mesa_visitor::visit(ir_dereference_array *ir)
{
   ir_constant *index;
   src_reg src;
   int element_size = type_size(ir->type);

   index = ir->array_index->constant_expression_value();

   ir->array->accept(this);
   src = this->result;

   if (index) {
      src.index += index->value.i[0] * element_size;
   } else {
      /* Variable index array dereference.  It eats the "vec4" of the
       * base of the array and an index that offsets the Mesa register
       * index.
       */
      ir->array_index->accept(this);

      src_reg index_reg;

      if (element_size == 1) {
	 index_reg = this->result;
      } else {
	 index_reg = get_temp(glsl_type::float_type);

	 emit(ir, OPCODE_MUL, dst_reg(index_reg),
	      this->result, src_reg_for_float(element_size));
      }

      /* If there was already a relative address register involved, add the
       * new and the old together to get the new offset.
       */
      if (src.reladdr != NULL)  {
	 src_reg accum_reg = get_temp(glsl_type::float_type);

	 emit(ir, OPCODE_ADD, dst_reg(accum_reg),
	      index_reg, *src.reladdr);

	 index_reg = accum_reg;
      }

      src.reladdr = ralloc(mem_ctx, src_reg);
      memcpy(src.reladdr, &index_reg, sizeof(index_reg));
   }

   /* If the type is smaller than a vec4, replicate the last channel out. */
   if (ir->type->is_scalar() || ir->type->is_vector())
      src.swizzle = swizzle_for_size(ir->type->vector_elements);
   else
      src.swizzle = SWIZZLE_NOOP;

   this->result = src;
}

void
ir_to_mesa_visitor::visit(ir_dereference_record *ir)
{
   unsigned int i;
   const glsl_type *struct_type = ir->record->type;
   int offset = 0;

   ir->record->accept(this);

   for (i = 0; i < struct_type->length; i++) {
      if (strcmp(struct_type->fields.structure[i].name, ir->field) == 0)
	 break;
      offset += type_size(struct_type->fields.structure[i].type);
   }

   /* If the type is smaller than a vec4, replicate the last channel out. */
   if (ir->type->is_scalar() || ir->type->is_vector())
      this->result.swizzle = swizzle_for_size(ir->type->vector_elements);
   else
      this->result.swizzle = SWIZZLE_NOOP;

   this->result.index += offset;
}

/**
 * We want to be careful in assignment setup to hit the actual storage
 * instead of potentially using a temporary like we might with the
 * ir_dereference handler.
 */
static dst_reg
get_assignment_lhs(ir_dereference *ir, ir_to_mesa_visitor *v)
{
   /* The LHS must be a dereference.  If the LHS is a variable indexed array
    * access of a vector, it must be separated into a series conditional moves
    * before reaching this point (see ir_vec_index_to_cond_assign).
    */
   assert(ir->as_dereference());
   ir_dereference_array *deref_array = ir->as_dereference_array();
   if (deref_array) {
      assert(!deref_array->array->type->is_vector());
   }

   /* Use the rvalue deref handler for the most part.  We'll ignore
    * swizzles in it and write swizzles using writemask, though.
    */
   ir->accept(v);
   return dst_reg(v->result);
}

/**
 * Process the condition of a conditional assignment
 *
 * Examines the condition of a conditional assignment to generate the optimal
 * first operand of a \c CMP instruction.  If the condition is a relational
 * operator with 0 (e.g., \c ir_binop_less), the value being compared will be
 * used as the source for the \c CMP instruction.  Otherwise the comparison
 * is processed to a boolean result, and the boolean result is used as the
 * operand to the CMP instruction.
 */
bool
ir_to_mesa_visitor::process_move_condition(ir_rvalue *ir)
{
   ir_rvalue *src_ir = ir;
   bool negate = true;
   bool switch_order = false;

   ir_expression *const expr = ir->as_expression();
   if ((expr != NULL) && (expr->get_num_operands() == 2)) {
      bool zero_on_left = false;

      if (expr->operands[0]->is_zero()) {
	 src_ir = expr->operands[1];
	 zero_on_left = true;
      } else if (expr->operands[1]->is_zero()) {
	 src_ir = expr->operands[0];
	 zero_on_left = false;
      }

      /*      a is -  0  +            -  0  +
       * (a <  0)  T  F  F  ( a < 0)  T  F  F
       * (0 <  a)  F  F  T  (-a < 0)  F  F  T
       * (a <= 0)  T  T  F  (-a < 0)  F  F  T  (swap order of other operands)
       * (0 <= a)  F  T  T  ( a < 0)  T  F  F  (swap order of other operands)
       * (a >  0)  F  F  T  (-a < 0)  F  F  T
       * (0 >  a)  T  F  F  ( a < 0)  T  F  F
       * (a >= 0)  F  T  T  ( a < 0)  T  F  F  (swap order of other operands)
       * (0 >= a)  T  T  F  (-a < 0)  F  F  T  (swap order of other operands)
       *
       * Note that exchanging the order of 0 and 'a' in the comparison simply
       * means that the value of 'a' should be negated.
       */
      if (src_ir != ir) {
	 switch (expr->operation) {
	 case ir_binop_less:
	    switch_order = false;
	    negate = zero_on_left;
	    break;

	 case ir_binop_greater:
	    switch_order = false;
	    negate = !zero_on_left;
	    break;

	 case ir_binop_lequal:
	    switch_order = true;
	    negate = !zero_on_left;
	    break;

	 case ir_binop_gequal:
	    switch_order = true;
	    negate = zero_on_left;
	    break;

	 default:
	    /* This isn't the right kind of comparison afterall, so make sure
	     * the whole condition is visited.
	     */
	    src_ir = ir;
	    break;
	 }
      }
   }

   src_ir->accept(this);

   /* We use the OPCODE_CMP (a < 0 ? b : c) for conditional moves, and the
    * condition we produced is 0.0 or 1.0.  By flipping the sign, we can
    * choose which value OPCODE_CMP produces without an extra instruction
    * computing the condition.
    */
   if (negate)
      this->result.negate = ~this->result.negate;

   return switch_order;
}

void
ir_to_mesa_visitor::visit(ir_assignment *ir)
{
   dst_reg l;
   src_reg r;
   int i;

   ir->rhs->accept(this);
   r = this->result;

   l = get_assignment_lhs(ir->lhs, this);

   /* FINISHME: This should really set to the correct maximal writemask for each
    * FINISHME: component written (in the loops below).  This case can only
    * FINISHME: occur for matrices, arrays, and structures.
    */
   if (ir->write_mask == 0) {
      assert(!ir->lhs->type->is_scalar() && !ir->lhs->type->is_vector());
      l.writemask = WRITEMASK_XYZW;
   } else if (ir->lhs->type->is_scalar()) {
      /* FINISHME: This hack makes writing to gl_FragDepth, which lives in the
       * FINISHME: W component of fragment shader output zero, work correctly.
       */
      l.writemask = WRITEMASK_XYZW;
   } else {
      int swizzles[4];
      int first_enabled_chan = 0;
      int rhs_chan = 0;

      assert(ir->lhs->type->is_vector());
      l.writemask = ir->write_mask;

      for (int i = 0; i < 4; i++) {
	 if (l.writemask & (1 << i)) {
	    first_enabled_chan = GET_SWZ(r.swizzle, i);
	    break;
	 }
      }

      /* Swizzle a small RHS vector into the channels being written.
       *
       * glsl ir treats write_mask as dictating how many channels are
       * present on the RHS while Mesa IR treats write_mask as just
       * showing which channels of the vec4 RHS get written.
       */
      for (int i = 0; i < 4; i++) {
	 if (l.writemask & (1 << i))
	    swizzles[i] = GET_SWZ(r.swizzle, rhs_chan++);
	 else
	    swizzles[i] = first_enabled_chan;
      }
      r.swizzle = MAKE_SWIZZLE4(swizzles[0], swizzles[1],
				swizzles[2], swizzles[3]);
   }

   assert(l.file != PROGRAM_UNDEFINED);
   assert(r.file != PROGRAM_UNDEFINED);

   if (ir->condition) {
      const bool switch_order = this->process_move_condition(ir->condition);
      src_reg condition = this->result;

      for (i = 0; i < type_size(ir->lhs->type); i++) {
	 if (switch_order) {
	    emit(ir, OPCODE_CMP, l, condition, src_reg(l), r);
	 } else {
	    emit(ir, OPCODE_CMP, l, condition, r, src_reg(l));
	 }

	 l.index++;
	 r.index++;
      }
   } else {
      for (i = 0; i < type_size(ir->lhs->type); i++) {
	 emit(ir, OPCODE_MOV, l, r);
	 l.index++;
	 r.index++;
      }
   }
}


void
ir_to_mesa_visitor::visit(ir_constant *ir)
{
   src_reg src;
   GLfloat stack_vals[4] = { 0 };
   GLfloat *values = stack_vals;
   unsigned int i;

   /* Unfortunately, 4 floats is all we can get into
    * _mesa_add_unnamed_constant.  So, make a temp to store an
    * aggregate constant and move each constant value into it.  If we
    * get lucky, copy propagation will eliminate the extra moves.
    */

   if (ir->type->base_type == GLSL_TYPE_STRUCT) {
      src_reg temp_base = get_temp(ir->type);
      dst_reg temp = dst_reg(temp_base);

      foreach_iter(exec_list_iterator, iter, ir->components) {
	 ir_constant *field_value = (ir_constant *)iter.get();
	 int size = type_size(field_value->type);

	 assert(size > 0);

	 field_value->accept(this);
	 src = this->result;

	 for (i = 0; i < (unsigned int)size; i++) {
	    emit(ir, OPCODE_MOV, temp, src);

	    src.index++;
	    temp.index++;
	 }
      }
      this->result = temp_base;
      return;
   }

   if (ir->type->is_array()) {
      src_reg temp_base = get_temp(ir->type);
      dst_reg temp = dst_reg(temp_base);
      int size = type_size(ir->type->fields.array);

      assert(size > 0);

      for (i = 0; i < ir->type->length; i++) {
	 ir->array_elements[i]->accept(this);
	 src = this->result;
	 for (int j = 0; j < size; j++) {
	    emit(ir, OPCODE_MOV, temp, src);

	    src.index++;
	    temp.index++;
	 }
      }
      this->result = temp_base;
      return;
   }

   if (ir->type->is_matrix()) {
      src_reg mat = get_temp(ir->type);
      dst_reg mat_column = dst_reg(mat);

      for (i = 0; i < ir->type->matrix_columns; i++) {
	 assert(ir->type->base_type == GLSL_TYPE_FLOAT);
	 values = &ir->value.f[i * ir->type->vector_elements];

	 src = src_reg(PROGRAM_CONSTANT, -1, NULL);
	 src.index = _mesa_add_unnamed_constant(this->prog->Parameters,
						values,
						ir->type->vector_elements,
						&src.swizzle);
	 emit(ir, OPCODE_MOV, mat_column, src);

	 mat_column.index++;
      }

      this->result = mat;
      return;
   }

   src.file = PROGRAM_CONSTANT;
   switch (ir->type->base_type) {
   case GLSL_TYPE_FLOAT:
      values = &ir->value.f[0];
      break;
   case GLSL_TYPE_UINT:
      for (i = 0; i < ir->type->vector_elements; i++) {
	 values[i] = ir->value.u[i];
      }
      break;
   case GLSL_TYPE_INT:
      for (i = 0; i < ir->type->vector_elements; i++) {
	 values[i] = ir->value.i[i];
      }
      break;
   case GLSL_TYPE_BOOL:
      for (i = 0; i < ir->type->vector_elements; i++) {
	 values[i] = ir->value.b[i];
      }
      break;
   default:
      assert(!"Non-float/uint/int/bool constant");
   }

   this->result = src_reg(PROGRAM_CONSTANT, -1, ir->type);
   this->result.index = _mesa_add_unnamed_constant(this->prog->Parameters,
						   values,
						   ir->type->vector_elements,
						   &this->result.swizzle);
}

function_entry *
ir_to_mesa_visitor::get_function_signature(ir_function_signature *sig)
{
   function_entry *entry;

   foreach_iter(exec_list_iterator, iter, this->function_signatures) {
      entry = (function_entry *)iter.get();

      if (entry->sig == sig)
	 return entry;
   }

   entry = ralloc(mem_ctx, function_entry);
   entry->sig = sig;
   entry->sig_id = this->next_signature_id++;
   entry->bgn_inst = NULL;

   /* Allocate storage for all the parameters. */
   foreach_iter(exec_list_iterator, iter, sig->parameters) {
      ir_variable *param = (ir_variable *)iter.get();
      variable_storage *storage;

      storage = find_variable_storage(param);
      assert(!storage);

      storage = new(mem_ctx) variable_storage(param, PROGRAM_TEMPORARY,
					      this->next_temp);
      this->variables.push_tail(storage);

      this->next_temp += type_size(param->type);
   }

   if (!sig->return_type->is_void()) {
      entry->return_reg = get_temp(sig->return_type);
   } else {
      entry->return_reg = undef_src;
   }

   this->function_signatures.push_tail(entry);
   return entry;
}

void
ir_to_mesa_visitor::visit(ir_call *ir)
{
   ir_to_mesa_instruction *call_inst;
   ir_function_signature *sig = ir->get_callee();
   function_entry *entry = get_function_signature(sig);
   int i;

   /* Process in parameters. */
   exec_list_iterator sig_iter = sig->parameters.iterator();
   foreach_iter(exec_list_iterator, iter, *ir) {
      ir_rvalue *param_rval = (ir_rvalue *)iter.get();
      ir_variable *param = (ir_variable *)sig_iter.get();

      if (param->mode == ir_var_in ||
	  param->mode == ir_var_inout) {
	 variable_storage *storage = find_variable_storage(param);
	 assert(storage);

	 param_rval->accept(this);
	 src_reg r = this->result;

	 dst_reg l;
	 l.file = storage->file;
	 l.index = storage->index;
	 l.reladdr = NULL;
	 l.writemask = WRITEMASK_XYZW;
	 l.cond_mask = COND_TR;

	 for (i = 0; i < type_size(param->type); i++) {
	    emit(ir, OPCODE_MOV, l, r);
	    l.index++;
	    r.index++;
	 }
      }

      sig_iter.next();
   }
   assert(!sig_iter.has_next());

   /* Emit call instruction */
   call_inst = emit(ir, OPCODE_CAL);
   call_inst->function = entry;

   /* Process out parameters. */
   sig_iter = sig->parameters.iterator();
   foreach_iter(exec_list_iterator, iter, *ir) {
      ir_rvalue *param_rval = (ir_rvalue *)iter.get();
      ir_variable *param = (ir_variable *)sig_iter.get();

      if (param->mode == ir_var_out ||
	  param->mode == ir_var_inout) {
	 variable_storage *storage = find_variable_storage(param);
	 assert(storage);

	 src_reg r;
	 r.file = storage->file;
	 r.index = storage->index;
	 r.reladdr = NULL;
	 r.swizzle = SWIZZLE_NOOP;
	 r.negate = 0;

	 param_rval->accept(this);
	 dst_reg l = dst_reg(this->result);

	 for (i = 0; i < type_size(param->type); i++) {
	    emit(ir, OPCODE_MOV, l, r);
	    l.index++;
	    r.index++;
	 }
      }

      sig_iter.next();
   }
   assert(!sig_iter.has_next());

   /* Process return value. */
   this->result = entry->return_reg;
}

void
ir_to_mesa_visitor::visit(ir_texture *ir)
{
   src_reg result_src, coord, lod_info, projector, dx, dy;
   dst_reg result_dst, coord_dst;
   ir_to_mesa_instruction *inst = NULL;
   prog_opcode opcode = OPCODE_NOP;

   ir->coordinate->accept(this);

   /* Put our coords in a temp.  We'll need to modify them for shadow,
    * projection, or LOD, so the only case we'd use it as is is if
    * we're doing plain old texturing.  Mesa IR optimization should
    * handle cleaning up our mess in that case.
    */
   coord = get_temp(glsl_type::vec4_type);
   coord_dst = dst_reg(coord);
   emit(ir, OPCODE_MOV, coord_dst, this->result);

   if (ir->projector) {
      ir->projector->accept(this);
      projector = this->result;
   }

   /* Storage for our result.  Ideally for an assignment we'd be using
    * the actual storage for the result here, instead.
    */
   result_src = get_temp(glsl_type::vec4_type);
   result_dst = dst_reg(result_src);

   switch (ir->op) {
   case ir_tex:
      opcode = OPCODE_TEX;
      break;
   case ir_txb:
      opcode = OPCODE_TXB;
      ir->lod_info.bias->accept(this);
      lod_info = this->result;
      break;
   case ir_txl:
      opcode = OPCODE_TXL;
      ir->lod_info.lod->accept(this);
      lod_info = this->result;
      break;
   case ir_txd:
      opcode = OPCODE_TXD;
      ir->lod_info.grad.dPdx->accept(this);
      dx = this->result;
      ir->lod_info.grad.dPdy->accept(this);
      dy = this->result;
      break;
   case ir_txf:
      assert(!"GLSL 1.30 features unsupported");
      break;
   }

   if (ir->projector) {
      if (opcode == OPCODE_TEX) {
	 /* Slot the projector in as the last component of the coord. */
	 coord_dst.writemask = WRITEMASK_W;
	 emit(ir, OPCODE_MOV, coord_dst, projector);
	 coord_dst.writemask = WRITEMASK_XYZW;
	 opcode = OPCODE_TXP;
      } else {
	 src_reg coord_w = coord;
	 coord_w.swizzle = SWIZZLE_WWWW;

	 /* For the other TEX opcodes there's no projective version
	  * since the last slot is taken up by lod info.  Do the
	  * projective divide now.
	  */
	 coord_dst.writemask = WRITEMASK_W;
	 emit(ir, OPCODE_RCP, coord_dst, projector);

	 /* In the case where we have to project the coordinates "by hand,"
	  * the shadow comparitor value must also be projected.
	  */
	 src_reg tmp_src = coord;
	 if (ir->shadow_comparitor) {
	    /* Slot the shadow value in as the second to last component of the
	     * coord.
	     */
	    ir->shadow_comparitor->accept(this);

	    tmp_src = get_temp(glsl_type::vec4_type);
	    dst_reg tmp_dst = dst_reg(tmp_src);

	    tmp_dst.writemask = WRITEMASK_Z;
	    emit(ir, OPCODE_MOV, tmp_dst, this->result);

	    tmp_dst.writemask = WRITEMASK_XY;
	    emit(ir, OPCODE_MOV, tmp_dst, coord);
	 }

	 coord_dst.writemask = WRITEMASK_XYZ;
	 emit(ir, OPCODE_MUL, coord_dst, tmp_src, coord_w);

	 coord_dst.writemask = WRITEMASK_XYZW;
	 coord.swizzle = SWIZZLE_XYZW;
      }
   }

   /* If projection is done and the opcode is not OPCODE_TXP, then the shadow
    * comparitor was put in the correct place (and projected) by the code,
    * above, that handles by-hand projection.
    */
   if (ir->shadow_comparitor && (!ir->projector || opcode == OPCODE_TXP)) {
      /* Slot the shadow value in as the second to last component of the
       * coord.
       */
      ir->shadow_comparitor->accept(this);
      coord_dst.writemask = WRITEMASK_Z;
      emit(ir, OPCODE_MOV, coord_dst, this->result);
      coord_dst.writemask = WRITEMASK_XYZW;
   }

   if (opcode == OPCODE_TXL || opcode == OPCODE_TXB) {
      /* Mesa IR stores lod or lod bias in the last channel of the coords. */
      coord_dst.writemask = WRITEMASK_W;
      emit(ir, OPCODE_MOV, coord_dst, lod_info);
      coord_dst.writemask = WRITEMASK_XYZW;
   }

   if (opcode == OPCODE_TXD)
      inst = emit(ir, opcode, result_dst, coord, dx, dy);
   else
      inst = emit(ir, opcode, result_dst, coord);

   if (ir->shadow_comparitor)
      inst->tex_shadow = GL_TRUE;

   inst->sampler = _mesa_get_sampler_uniform_value(ir->sampler,
						   this->shader_program,
						   this->prog);

   const glsl_type *sampler_type = ir->sampler->type;

   switch (sampler_type->sampler_dimensionality) {
   case GLSL_SAMPLER_DIM_1D:
      inst->tex_target = (sampler_type->sampler_array)
	 ? TEXTURE_1D_ARRAY_INDEX : TEXTURE_1D_INDEX;
      break;
   case GLSL_SAMPLER_DIM_2D:
      inst->tex_target = (sampler_type->sampler_array)
	 ? TEXTURE_2D_ARRAY_INDEX : TEXTURE_2D_INDEX;
      break;
   case GLSL_SAMPLER_DIM_3D:
      inst->tex_target = TEXTURE_3D_INDEX;
      break;
   case GLSL_SAMPLER_DIM_CUBE:
      inst->tex_target = TEXTURE_CUBE_INDEX;
      break;
   case GLSL_SAMPLER_DIM_RECT:
      inst->tex_target = TEXTURE_RECT_INDEX;
      break;
   case GLSL_SAMPLER_DIM_BUF:
      assert(!"FINISHME: Implement ARB_texture_buffer_object");
      break;
   default:
      assert(!"Should not get here.");
   }

   this->result = result_src;
}

void
ir_to_mesa_visitor::visit(ir_return *ir)
{
   if (ir->get_value()) {
      dst_reg l;
      int i;

      assert(current_function);

      ir->get_value()->accept(this);
      src_reg r = this->result;

      l = dst_reg(current_function->return_reg);

      for (i = 0; i < type_size(current_function->sig->return_type); i++) {
	 emit(ir, OPCODE_MOV, l, r);
	 l.index++;
	 r.index++;
      }
   }

   emit(ir, OPCODE_RET);
}

void
ir_to_mesa_visitor::visit(ir_discard *ir)
{
   struct gl_fragment_program *fp = (struct gl_fragment_program *)this->prog;

   if (ir->condition) {
      ir->condition->accept(this);
      this->result.negate = ~this->result.negate;
      emit(ir, OPCODE_KIL, undef_dst, this->result);
   } else {
      emit(ir, OPCODE_KIL_NV);
   }

   fp->UsesKill = GL_TRUE;
}

void
ir_to_mesa_visitor::visit(ir_if *ir)
{
   ir_to_mesa_instruction *cond_inst, *if_inst;
   ir_to_mesa_instruction *prev_inst;

   prev_inst = (ir_to_mesa_instruction *)this->instructions.get_tail();

   ir->condition->accept(this);
   assert(this->result.file != PROGRAM_UNDEFINED);

   if (this->options->EmitCondCodes) {
      cond_inst = (ir_to_mesa_instruction *)this->instructions.get_tail();

      /* See if we actually generated any instruction for generating
       * the condition.  If not, then cook up a move to a temp so we
       * have something to set cond_update on.
       */
      if (cond_inst == prev_inst) {
	 src_reg temp = get_temp(glsl_type::bool_type);
	 cond_inst = emit(ir->condition, OPCODE_MOV, dst_reg(temp), result);
      }
      cond_inst->cond_update = GL_TRUE;

      if_inst = emit(ir->condition, OPCODE_IF);
      if_inst->dst.cond_mask = COND_NE;
   } else {
      if_inst = emit(ir->condition, OPCODE_IF, undef_dst, this->result);
   }

   this->instructions.push_tail(if_inst);

   visit_exec_list(&ir->then_instructions, this);

   if (!ir->else_instructions.is_empty()) {
      emit(ir->condition, OPCODE_ELSE);
      visit_exec_list(&ir->else_instructions, this);
   }

   if_inst = emit(ir->condition, OPCODE_ENDIF);
}

ir_to_mesa_visitor::ir_to_mesa_visitor()
{
   result.file = PROGRAM_UNDEFINED;
   next_temp = 1;
   next_signature_id = 1;
   current_function = NULL;
   mem_ctx = ralloc_context(NULL);
}

ir_to_mesa_visitor::~ir_to_mesa_visitor()
{
   ralloc_free(mem_ctx);
}

static struct prog_src_register
mesa_src_reg_from_ir_src_reg(src_reg reg)
{
   struct prog_src_register mesa_reg;

   mesa_reg.File = reg.file;
   assert(reg.index < (1 << INST_INDEX_BITS));
   mesa_reg.Index = reg.index;
   mesa_reg.Swizzle = reg.swizzle;
   mesa_reg.RelAddr = reg.reladdr != NULL;
   mesa_reg.Negate = reg.negate;
   mesa_reg.Abs = 0;
   mesa_reg.HasIndex2 = GL_FALSE;
   mesa_reg.RelAddr2 = 0;
   mesa_reg.Index2 = 0;

   return mesa_reg;
}

static void
set_branchtargets(ir_to_mesa_visitor *v,
		  struct prog_instruction *mesa_instructions,
		  int num_instructions)
{
   int if_count = 0, loop_count = 0;
   int *if_stack, *loop_stack;
   int if_stack_pos = 0, loop_stack_pos = 0;
   int i, j;

   for (i = 0; i < num_instructions; i++) {
      switch (mesa_instructions[i].Opcode) {
      case OPCODE_IF:
	 if_count++;
	 break;
      case OPCODE_BGNLOOP:
	 loop_count++;
	 break;
      case OPCODE_BRK:
      case OPCODE_CONT:
	 mesa_instructions[i].BranchTarget = -1;
	 break;
      default:
	 break;
      }
   }

   if_stack = rzalloc_array(v->mem_ctx, int, if_count);
   loop_stack = rzalloc_array(v->mem_ctx, int, loop_count);

   for (i = 0; i < num_instructions; i++) {
      switch (mesa_instructions[i].Opcode) {
      case OPCODE_IF:
	 if_stack[if_stack_pos] = i;
	 if_stack_pos++;
	 break;
      case OPCODE_ELSE:
	 mesa_instructions[if_stack[if_stack_pos - 1]].BranchTarget = i;
	 if_stack[if_stack_pos - 1] = i;
	 break;
      case OPCODE_ENDIF:
	 mesa_instructions[if_stack[if_stack_pos - 1]].BranchTarget = i;
	 if_stack_pos--;
	 break;
      case OPCODE_BGNLOOP:
	 loop_stack[loop_stack_pos] = i;
	 loop_stack_pos++;
	 break;
      case OPCODE_ENDLOOP:
	 loop_stack_pos--;
	 /* Rewrite any breaks/conts at this nesting level (haven't
	  * already had a BranchTarget assigned) to point to the end
	  * of the loop.
	  */
	 for (j = loop_stack[loop_stack_pos]; j < i; j++) {
	    if (mesa_instructions[j].Opcode == OPCODE_BRK ||
		mesa_instructions[j].Opcode == OPCODE_CONT) {
	       if (mesa_instructions[j].BranchTarget == -1) {
		  mesa_instructions[j].BranchTarget = i;
	       }
	    }
	 }
	 /* The loop ends point at each other. */
	 mesa_instructions[i].BranchTarget = loop_stack[loop_stack_pos];
	 mesa_instructions[loop_stack[loop_stack_pos]].BranchTarget = i;
	 break;
      case OPCODE_CAL:
	 foreach_iter(exec_list_iterator, iter, v->function_signatures) {
	    function_entry *entry = (function_entry *)iter.get();

	    if (entry->sig_id == mesa_instructions[i].BranchTarget) {
	       mesa_instructions[i].BranchTarget = entry->inst;
	       break;
	    }
	 }
	 break;
      default:
	 break;
      }
   }
}

static void
print_program(struct prog_instruction *mesa_instructions,
	      ir_instruction **mesa_instruction_annotation,
	      int num_instructions)
{
   ir_instruction *last_ir = NULL;
   int i;
   int indent = 0;

   for (i = 0; i < num_instructions; i++) {
      struct prog_instruction *mesa_inst = mesa_instructions + i;
      ir_instruction *ir = mesa_instruction_annotation[i];

      fprintf(stdout, "%3d: ", i);

      if (last_ir != ir && ir) {
	 int j;

	 for (j = 0; j < indent; j++) {
	    fprintf(stdout, " ");
	 }
	 ir->print();
	 printf("\n");
	 last_ir = ir;

	 fprintf(stdout, "     "); /* line number spacing. */
      }

      indent = _mesa_fprint_instruction_opt(stdout, mesa_inst, indent,
					    PROG_PRINT_DEBUG, NULL);
   }
}


/**
 * Count resources used by the given gpu program (number of texture
 * samplers, etc).
 */
static void
count_resources(struct gl_program *prog)
{
   unsigned int i;

   prog->SamplersUsed = 0;

   for (i = 0; i < prog->NumInstructions; i++) {
      struct prog_instruction *inst = &prog->Instructions[i];

      if (_mesa_is_tex_instruction(inst->Opcode)) {
	 prog->SamplerTargets[inst->TexSrcUnit] =
	    (gl_texture_index)inst->TexSrcTarget;
	 prog->SamplersUsed |= 1 << inst->TexSrcUnit;
	 if (inst->TexShadow) {
	    prog->ShadowSamplers |= 1 << inst->TexSrcUnit;
	 }
      }
   }

   _mesa_update_shader_textures_used(prog);
}


/**
 * Check if the given vertex/fragment/shader program is within the
 * resource limits of the context (number of texture units, etc).
 * If any of those checks fail, record a linker error.
 *
 * XXX more checks are needed...
 */
static void
check_resources(const struct gl_context *ctx,
                struct gl_shader_program *shader_program,
                struct gl_program *prog)
{
   switch (prog->Target) {
   case GL_VERTEX_PROGRAM_ARB:
      if (_mesa_bitcount(prog->SamplersUsed) >
          ctx->Const.MaxVertexTextureImageUnits) {
         fail_link(shader_program, "Too many vertex shader texture samplers");
      }
      if (prog->Parameters->NumParameters > MAX_UNIFORMS) {
         fail_link(shader_program, "Too many vertex shader constants");
      }
      break;
   case MESA_GEOMETRY_PROGRAM:
      if (_mesa_bitcount(prog->SamplersUsed) >
          ctx->Const.MaxGeometryTextureImageUnits) {
         fail_link(shader_program, "Too many geometry shader texture samplers");
      }
      if (prog->Parameters->NumParameters >
          MAX_GEOMETRY_UNIFORM_COMPONENTS / 4) {
         fail_link(shader_program, "Too many geometry shader constants");
      }
      break;
   case GL_FRAGMENT_PROGRAM_ARB:
      if (_mesa_bitcount(prog->SamplersUsed) >
          ctx->Const.MaxTextureImageUnits) {
         fail_link(shader_program, "Too many fragment shader texture samplers");
      }
      if (prog->Parameters->NumParameters > MAX_UNIFORMS) {
         fail_link(shader_program, "Too many fragment shader constants");
      }
      break;
   default:
      _mesa_problem(ctx, "unexpected program type in check_resources()");
   }
}



struct uniform_sort {
   struct gl_uniform *u;
   int pos;
};

/* The shader_program->Uniforms list is almost sorted in increasing
 * uniform->{Frag,Vert}Pos locations, but not quite when there are
 * uniforms shared between targets.  We need to add parameters in
 * increasing order for the targets.
 */
static int
sort_uniforms(const void *a, const void *b)
{
   struct uniform_sort *u1 = (struct uniform_sort *)a;
   struct uniform_sort *u2 = (struct uniform_sort *)b;

   return u1->pos - u2->pos;
}

/* Add the uniforms to the parameters.  The linker chose locations
 * in our parameters lists (which weren't created yet), which the
 * uniforms code will use to poke values into our parameters list
 * when uniforms are updated.
 */
static void
add_uniforms_to_parameters_list(struct gl_shader_program *shader_program,
				struct gl_shader *shader,
				struct gl_program *prog)
{
   unsigned int i;
   unsigned int next_sampler = 0, num_uniforms = 0;
   struct uniform_sort *sorted_uniforms;

   sorted_uniforms = ralloc_array(NULL, struct uniform_sort,
				  shader_program->Uniforms->NumUniforms);

   for (i = 0; i < shader_program->Uniforms->NumUniforms; i++) {
      struct gl_uniform *uniform = shader_program->Uniforms->Uniforms + i;
      int parameter_index = -1;

      switch (shader->Type) {
      case GL_VERTEX_SHADER:
	 parameter_index = uniform->VertPos;
	 break;
      case GL_FRAGMENT_SHADER:
	 parameter_index = uniform->FragPos;
	 break;
      case GL_GEOMETRY_SHADER:
	 parameter_index = uniform->GeomPos;
	 break;
      }

      /* Only add uniforms used in our target. */
      if (parameter_index != -1) {
	 sorted_uniforms[num_uniforms].pos = parameter_index;
	 sorted_uniforms[num_uniforms].u = uniform;
	 num_uniforms++;
      }
   }

   qsort(sorted_uniforms, num_uniforms, sizeof(struct uniform_sort),
	 sort_uniforms);

   for (i = 0; i < num_uniforms; i++) {
      struct gl_uniform *uniform = sorted_uniforms[i].u;
      int parameter_index = sorted_uniforms[i].pos;
      const glsl_type *type = uniform->Type;
      unsigned int size;

      if (type->is_vector() ||
	  type->is_scalar()) {
	 size = type->vector_elements;
      } else {
	 size = type_size(type) * 4;
      }

      gl_register_file file;
      if (type->is_sampler() ||
	  (type->is_array() && type->fields.array->is_sampler())) {
	 file = PROGRAM_SAMPLER;
      } else {
	 file = PROGRAM_UNIFORM;
      }

      GLint index = _mesa_lookup_parameter_index(prog->Parameters, -1,
						 uniform->Name);

      if (index < 0) {
	 index = _mesa_add_parameter(prog->Parameters, file,
				     uniform->Name, size, type->gl_type,
				     NULL, NULL, 0x0);

	 /* Sampler uniform values are stored in prog->SamplerUnits,
	  * and the entry in that array is selected by this index we
	  * store in ParameterValues[].
	  */
	 if (file == PROGRAM_SAMPLER) {
	    for (unsigned int j = 0; j < size / 4; j++)
	       prog->Parameters->ParameterValues[index + j][0] = next_sampler++;
	 }

	 /* The location chosen in the Parameters list here (returned
	  * from _mesa_add_uniform) has to match what the linker chose.
	  */
	 if (index != parameter_index) {
	    fail_link(shader_program, "Allocation of uniform `%s' to target "
		      "failed (%d vs %d)\n",
		      uniform->Name, index, parameter_index);
	 }
      }
   }

   ralloc_free(sorted_uniforms);
}

static void
set_uniform_initializer(struct gl_context *ctx, void *mem_ctx,
			struct gl_shader_program *shader_program,
			const char *name, const glsl_type *type,
			ir_constant *val)
{
   if (type->is_record()) {
      ir_constant *field_constant;

      field_constant = (ir_constant *)val->components.get_head();

      for (unsigned int i = 0; i < type->length; i++) {
	 const glsl_type *field_type = type->fields.structure[i].type;
	 const char *field_name = ralloc_asprintf(mem_ctx, "%s.%s", name,
					    type->fields.structure[i].name);
	 set_uniform_initializer(ctx, mem_ctx, shader_program, field_name,
				 field_type, field_constant);
	 field_constant = (ir_constant *)field_constant->next;
      }
      return;
   }

   int loc = _mesa_get_uniform_location(ctx, shader_program, name);

   if (loc == -1) {
      fail_link(shader_program,
		"Couldn't find uniform for initializer %s\n", name);
      return;
   }

   for (unsigned int i = 0; i < (type->is_array() ? type->length : 1); i++) {
      ir_constant *element;
      const glsl_type *element_type;
      if (type->is_array()) {
	 element = val->array_elements[i];
	 element_type = type->fields.array;
      } else {
	 element = val;
	 element_type = type;
      }

      void *values;

      if (element_type->base_type == GLSL_TYPE_BOOL) {
	 int *conv = ralloc_array(mem_ctx, int, element_type->components());
	 for (unsigned int j = 0; j < element_type->components(); j++) {
	    conv[j] = element->value.b[j];
	 }
	 values = (void *)conv;
	 element_type = glsl_type::get_instance(GLSL_TYPE_INT,
						element_type->vector_elements,
						1);
      } else {
	 values = &element->value;
      }

      if (element_type->is_matrix()) {
	 _mesa_uniform_matrix(ctx, shader_program,
			      element_type->matrix_columns,
			      element_type->vector_elements,
			      loc, 1, GL_FALSE, (GLfloat *)values);
	 loc += element_type->matrix_columns;
      } else {
	 _mesa_uniform(ctx, shader_program, loc, element_type->matrix_columns,
		       values, element_type->gl_type);
	 loc += type_size(element_type);
      }
   }
}

static void
set_uniform_initializers(struct gl_context *ctx,
			 struct gl_shader_program *shader_program)
{
   void *mem_ctx = NULL;

   for (unsigned int i = 0; i < MESA_SHADER_TYPES; i++) {
      struct gl_shader *shader = shader_program->_LinkedShaders[i];

      if (shader == NULL)
	 continue;

      foreach_iter(exec_list_iterator, iter, *shader->ir) {
	 ir_instruction *ir = (ir_instruction *)iter.get();
	 ir_variable *var = ir->as_variable();

	 if (!var || var->mode != ir_var_uniform || !var->constant_value)
	    continue;

	 if (!mem_ctx)
	    mem_ctx = ralloc_context(NULL);

	 set_uniform_initializer(ctx, mem_ctx, shader_program, var->name,
				 var->type, var->constant_value);
      }
   }

   ralloc_free(mem_ctx);
}

/*
 * On a basic block basis, tracks available PROGRAM_TEMPORARY register
 * channels for copy propagation and updates following instructions to
 * use the original versions.
 *
 * The ir_to_mesa_visitor lazily produces code assuming that this pass
 * will occur.  As an example, a TXP production before this pass:
 *
 * 0: MOV TEMP[1], INPUT[4].xyyy;
 * 1: MOV TEMP[1].w, INPUT[4].wwww;
 * 2: TXP TEMP[2], TEMP[1], texture[0], 2D;
 *
 * and after:
 *
 * 0: MOV TEMP[1], INPUT[4].xyyy;
 * 1: MOV TEMP[1].w, INPUT[4].wwww;
 * 2: TXP TEMP[2], INPUT[4].xyyw, texture[0], 2D;
 *
 * which allows for dead code elimination on TEMP[1]'s writes.
 */
void
ir_to_mesa_visitor::copy_propagate(void)
{
   ir_to_mesa_instruction **acp = rzalloc_array(mem_ctx,
						    ir_to_mesa_instruction *,
						    this->next_temp * 4);
   int *acp_level = rzalloc_array(mem_ctx, int, this->next_temp * 4);
   int level = 0;

   foreach_iter(exec_list_iterator, iter, this->instructions) {
      ir_to_mesa_instruction *inst = (ir_to_mesa_instruction *)iter.get();

      assert(inst->dst.file != PROGRAM_TEMPORARY
	     || inst->dst.index < this->next_temp);

      /* First, do any copy propagation possible into the src regs. */
      for (int r = 0; r < 3; r++) {
	 ir_to_mesa_instruction *first = NULL;
	 bool good = true;
	 int acp_base = inst->src[r].index * 4;

	 if (inst->src[r].file != PROGRAM_TEMPORARY ||
	     inst->src[r].reladdr)
	    continue;

	 /* See if we can find entries in the ACP consisting of MOVs
	  * from the same src register for all the swizzled channels
	  * of this src register reference.
	  */
	 for (int i = 0; i < 4; i++) {
	    int src_chan = GET_SWZ(inst->src[r].swizzle, i);
	    ir_to_mesa_instruction *copy_chan = acp[acp_base + src_chan];

	    if (!copy_chan) {
	       good = false;
	       break;
	    }

	    assert(acp_level[acp_base + src_chan] <= level);

	    if (!first) {
	       first = copy_chan;
	    } else {
	       if (first->src[0].file != copy_chan->src[0].file ||
		   first->src[0].index != copy_chan->src[0].index) {
		  good = false;
		  break;
	       }
	    }
	 }

	 if (good) {
	    /* We've now validated that we can copy-propagate to
	     * replace this src register reference.  Do it.
	     */
	    inst->src[r].file = first->src[0].file;
	    inst->src[r].index = first->src[0].index;

	    int swizzle = 0;
	    for (int i = 0; i < 4; i++) {
	       int src_chan = GET_SWZ(inst->src[r].swizzle, i);
	       ir_to_mesa_instruction *copy_inst = acp[acp_base + src_chan];
	       swizzle |= (GET_SWZ(copy_inst->src[0].swizzle, src_chan) <<
			   (3 * i));
	    }
	    inst->src[r].swizzle = swizzle;
	 }
      }

      switch (inst->op) {
      case OPCODE_BGNLOOP:
      case OPCODE_ENDLOOP:
	 /* End of a basic block, clear the ACP entirely. */
	 memset(acp, 0, sizeof(*acp) * this->next_temp * 4);
	 break;

      case OPCODE_IF:
	 ++level;
	 break;

      case OPCODE_ENDIF:
      case OPCODE_ELSE:
	 /* Clear all channels written inside the block from the ACP, but
	  * leaving those that were not touched.
	  */
	 for (int r = 0; r < this->next_temp; r++) {
	    for (int c = 0; c < 4; c++) {
	       if (!acp[4 * r + c])
		  continue;

	       if (acp_level[4 * r + c] >= level)
		  acp[4 * r + c] = NULL;
	    }
	 }
	 if (inst->op == OPCODE_ENDIF)
	    --level;
	 break;

      default:
	 /* Continuing the block, clear any written channels from
	  * the ACP.
	  */
	 if (inst->dst.file == PROGRAM_TEMPORARY && inst->dst.reladdr) {
	    /* Any temporary might be written, so no copy propagation
	     * across this instruction.
	     */
	    memset(acp, 0, sizeof(*acp) * this->next_temp * 4);
	 } else if (inst->dst.file == PROGRAM_OUTPUT &&
		    inst->dst.reladdr) {
	    /* Any output might be written, so no copy propagation
	     * from outputs across this instruction.
	     */
	    for (int r = 0; r < this->next_temp; r++) {
	       for (int c = 0; c < 4; c++) {
		  if (!acp[4 * r + c])
		     continue;

		  if (acp[4 * r + c]->src[0].file == PROGRAM_OUTPUT)
		     acp[4 * r + c] = NULL;
	       }
	    }
	 } else if (inst->dst.file == PROGRAM_TEMPORARY ||
		    inst->dst.file == PROGRAM_OUTPUT) {
	    /* Clear where it's used as dst. */
	    if (inst->dst.file == PROGRAM_TEMPORARY) {
	       for (int c = 0; c < 4; c++) {
		  if (inst->dst.writemask & (1 << c)) {
		     acp[4 * inst->dst.index + c] = NULL;
		  }
	       }
	    }

	    /* Clear where it's used as src. */
	    for (int r = 0; r < this->next_temp; r++) {
	       for (int c = 0; c < 4; c++) {
		  if (!acp[4 * r + c])
		     continue;

		  int src_chan = GET_SWZ(acp[4 * r + c]->src[0].swizzle, c);

		  if (acp[4 * r + c]->src[0].file == inst->dst.file &&
		      acp[4 * r + c]->src[0].index == inst->dst.index &&
		      inst->dst.writemask & (1 << src_chan))
		  {
		     acp[4 * r + c] = NULL;
		  }
	       }
	    }
	 }
	 break;
      }

      /* If this is a copy, add it to the ACP. */
      if (inst->op == OPCODE_MOV &&
	  inst->dst.file == PROGRAM_TEMPORARY &&
	  !inst->dst.reladdr &&
	  !inst->saturate &&
	  !inst->src[0].reladdr &&
	  !inst->src[0].negate) {
	 for (int i = 0; i < 4; i++) {
	    if (inst->dst.writemask & (1 << i)) {
	       acp[4 * inst->dst.index + i] = inst;
	       acp_level[4 * inst->dst.index + i] = level;
	    }
	 }
      }
   }

   ralloc_free(acp_level);
   ralloc_free(acp);
}


/**
 * Convert a shader's GLSL IR into a Mesa gl_program.
 */
static struct gl_program *
get_mesa_program(struct gl_context *ctx,
                 struct gl_shader_program *shader_program,
		 struct gl_shader *shader)
{
   ir_to_mesa_visitor v;
   struct prog_instruction *mesa_instructions, *mesa_inst;
   ir_instruction **mesa_instruction_annotation;
   int i;
   struct gl_program *prog;
   GLenum target;
   const char *target_string;
   GLboolean progress;
   struct gl_shader_compiler_options *options =
         &ctx->ShaderCompilerOptions[_mesa_shader_type_to_index(shader->Type)];

   switch (shader->Type) {
   case GL_VERTEX_SHADER:
      target = GL_VERTEX_PROGRAM_ARB;
      target_string = "vertex";
      break;
   case GL_FRAGMENT_SHADER:
      target = GL_FRAGMENT_PROGRAM_ARB;
      target_string = "fragment";
      break;
   case GL_GEOMETRY_SHADER:
      target = GL_GEOMETRY_PROGRAM_NV;
      target_string = "geometry";
      break;
   default:
      assert(!"should not be reached");
      return NULL;
   }

   validate_ir_tree(shader->ir);

   prog = ctx->Driver.NewProgram(ctx, target, shader_program->Name);
   if (!prog)
      return NULL;
   prog->Parameters = _mesa_new_parameter_list();
   prog->Varying = _mesa_new_parameter_list();
   prog->Attributes = _mesa_new_parameter_list();
   v.ctx = ctx;
   v.prog = prog;
   v.shader_program = shader_program;
   v.options = options;

   add_uniforms_to_parameters_list(shader_program, shader, prog);

   /* Emit Mesa IR for main(). */
   visit_exec_list(shader->ir, &v);
   v.emit(NULL, OPCODE_END);

   /* Now emit bodies for any functions that were used. */
   do {
      progress = GL_FALSE;

      foreach_iter(exec_list_iterator, iter, v.function_signatures) {
	 function_entry *entry = (function_entry *)iter.get();

	 if (!entry->bgn_inst) {
	    v.current_function = entry;

	    entry->bgn_inst = v.emit(NULL, OPCODE_BGNSUB);
	    entry->bgn_inst->function = entry;

	    visit_exec_list(&entry->sig->body, &v);

	    ir_to_mesa_instruction *last;
	    last = (ir_to_mesa_instruction *)v.instructions.get_tail();
	    if (last->op != OPCODE_RET)
	       v.emit(NULL, OPCODE_RET);

	    ir_to_mesa_instruction *end;
	    end = v.emit(NULL, OPCODE_ENDSUB);
	    end->function = entry;

	    progress = GL_TRUE;
	 }
      }
   } while (progress);

   prog->NumTemporaries = v.next_temp;

   int num_instructions = 0;
   foreach_iter(exec_list_iterator, iter, v.instructions) {
      num_instructions++;
   }

   mesa_instructions =
      (struct prog_instruction *)calloc(num_instructions,
					sizeof(*mesa_instructions));
   mesa_instruction_annotation = ralloc_array(v.mem_ctx, ir_instruction *,
					      num_instructions);

   v.copy_propagate();

   /* Convert ir_mesa_instructions into prog_instructions.
    */
   mesa_inst = mesa_instructions;
   i = 0;
   foreach_iter(exec_list_iterator, iter, v.instructions) {
      const ir_to_mesa_instruction *inst = (ir_to_mesa_instruction *)iter.get();

      mesa_inst->Opcode = inst->op;
      mesa_inst->CondUpdate = inst->cond_update;
      if (inst->saturate)
	 mesa_inst->SaturateMode = SATURATE_ZERO_ONE;
      mesa_inst->DstReg.File = inst->dst.file;
      mesa_inst->DstReg.Index = inst->dst.index;
      mesa_inst->DstReg.CondMask = inst->dst.cond_mask;
      mesa_inst->DstReg.WriteMask = inst->dst.writemask;
      mesa_inst->DstReg.RelAddr = inst->dst.reladdr != NULL;
      mesa_inst->SrcReg[0] = mesa_src_reg_from_ir_src_reg(inst->src[0]);
      mesa_inst->SrcReg[1] = mesa_src_reg_from_ir_src_reg(inst->src[1]);
      mesa_inst->SrcReg[2] = mesa_src_reg_from_ir_src_reg(inst->src[2]);
      mesa_inst->TexSrcUnit = inst->sampler;
      mesa_inst->TexSrcTarget = inst->tex_target;
      mesa_inst->TexShadow = inst->tex_shadow;
      mesa_instruction_annotation[i] = inst->ir;

      /* Set IndirectRegisterFiles. */
      if (mesa_inst->DstReg.RelAddr)
         prog->IndirectRegisterFiles |= 1 << mesa_inst->DstReg.File;

      /* Update program's bitmask of indirectly accessed register files */
      for (unsigned src = 0; src < 3; src++)
         if (mesa_inst->SrcReg[src].RelAddr)
            prog->IndirectRegisterFiles |= 1 << mesa_inst->SrcReg[src].File;

      if (options->EmitNoIfs && mesa_inst->Opcode == OPCODE_IF) {
	 fail_link(shader_program, "Couldn't flatten if statement\n");
      }

      switch (mesa_inst->Opcode) {
      case OPCODE_BGNSUB:
	 inst->function->inst = i;
	 mesa_inst->Comment = strdup(inst->function->sig->function_name());
	 break;
      case OPCODE_ENDSUB:
	 mesa_inst->Comment = strdup(inst->function->sig->function_name());
	 break;
      case OPCODE_CAL:
	 mesa_inst->BranchTarget = inst->function->sig_id; /* rewritten later */
	 break;
      case OPCODE_ARL:
	 prog->NumAddressRegs = 1;
	 break;
      default:
	 break;
      }

      mesa_inst++;
      i++;

      if (!shader_program->LinkStatus)
         break;
   }

   if (!shader_program->LinkStatus) {
      free(mesa_instructions);
      _mesa_reference_program(ctx, &shader->Program, NULL);
      return NULL;
   }

   set_branchtargets(&v, mesa_instructions, num_instructions);

   if (ctx->Shader.Flags & GLSL_DUMP) {
      printf("\n");
      printf("GLSL IR for linked %s program %d:\n", target_string,
	     shader_program->Name);
      _mesa_print_ir(shader->ir, NULL);
      printf("\n");
      printf("\n");
      printf("Mesa IR for linked %s program %d:\n", target_string,
	     shader_program->Name);
      print_program(mesa_instructions, mesa_instruction_annotation,
		    num_instructions);
   }

   prog->Instructions = mesa_instructions;
   prog->NumInstructions = num_instructions;

   do_set_program_inouts(shader->ir, prog);
   count_resources(prog);

   check_resources(ctx, shader_program, prog);

   _mesa_reference_program(ctx, &shader->Program, prog);

   if ((ctx->Shader.Flags & GLSL_NO_OPT) == 0) {
      _mesa_optimize_program(ctx, prog);
   }

   return prog;
}

extern "C" {

/**
 * Link a shader.
 * Called via ctx->Driver.LinkShader()
 * This actually involves converting GLSL IR into Mesa gl_programs with
 * code lowering and other optimizations.
 */
GLboolean
_mesa_ir_link_shader(struct gl_context *ctx, struct gl_shader_program *prog)
{
   assert(prog->LinkStatus);

   for (unsigned i = 0; i < MESA_SHADER_TYPES; i++) {
      if (prog->_LinkedShaders[i] == NULL)
	 continue;

      bool progress;
      exec_list *ir = prog->_LinkedShaders[i]->ir;
      const struct gl_shader_compiler_options *options =
            &ctx->ShaderCompilerOptions[_mesa_shader_type_to_index(prog->_LinkedShaders[i]->Type)];

      do {
	 progress = false;

	 /* Lowering */
	 do_mat_op_to_vec(ir);
	 lower_instructions(ir, (MOD_TO_FRACT | DIV_TO_MUL_RCP | EXP_TO_EXP2
				 | LOG_TO_LOG2
				 | ((options->EmitNoPow) ? POW_TO_EXP2 : 0)));

	 progress = do_lower_jumps(ir, true, true, options->EmitNoMainReturn, options->EmitNoCont, options->EmitNoLoops) || progress;

	 progress = do_common_optimization(ir, true, options->MaxUnrollIterations) || progress;

	 progress = lower_quadop_vector(ir, true) || progress;

	 if (options->EmitNoIfs) {
	    progress = lower_discard(ir) || progress;
	    progress = lower_if_to_cond_assign(ir) || progress;
	 }

	 if (options->EmitNoNoise)
	    progress = lower_noise(ir) || progress;

	 /* If there are forms of indirect addressing that the driver
	  * cannot handle, perform the lowering pass.
	  */
	 if (options->EmitNoIndirectInput || options->EmitNoIndirectOutput
	     || options->EmitNoIndirectTemp || options->EmitNoIndirectUniform)
	   progress =
	     lower_variable_index_to_cond_assign(ir,
						 options->EmitNoIndirectInput,
						 options->EmitNoIndirectOutput,
						 options->EmitNoIndirectTemp,
						 options->EmitNoIndirectUniform)
	     || progress;

	 progress = do_vec_index_to_cond_assign(ir) || progress;
      } while (progress);

      validate_ir_tree(ir);
   }

   for (unsigned i = 0; i < MESA_SHADER_TYPES; i++) {
      struct gl_program *linked_prog;

      if (prog->_LinkedShaders[i] == NULL)
	 continue;

      linked_prog = get_mesa_program(ctx, prog, prog->_LinkedShaders[i]);

      if (linked_prog) {
         bool ok = true;

         switch (prog->_LinkedShaders[i]->Type) {
         case GL_VERTEX_SHADER:
            _mesa_reference_vertprog(ctx, &prog->VertexProgram,
                                     (struct gl_vertex_program *)linked_prog);
            ok = ctx->Driver.ProgramStringNotify(ctx, GL_VERTEX_PROGRAM_ARB,
                                                 linked_prog);
            break;
         case GL_FRAGMENT_SHADER:
            _mesa_reference_fragprog(ctx, &prog->FragmentProgram,
                                     (struct gl_fragment_program *)linked_prog);
            ok = ctx->Driver.ProgramStringNotify(ctx, GL_FRAGMENT_PROGRAM_ARB,
                                                 linked_prog);
            break;
         case GL_GEOMETRY_SHADER:
            _mesa_reference_geomprog(ctx, &prog->GeometryProgram,
                                     (struct gl_geometry_program *)linked_prog);
            ok = ctx->Driver.ProgramStringNotify(ctx, GL_GEOMETRY_PROGRAM_NV,
                                                 linked_prog);
            break;
         }
         if (!ok) {
            return GL_FALSE;
         }
      }

      _mesa_reference_program(ctx, &linked_prog, NULL);
   }

   return GL_TRUE;
}


/**
 * Compile a GLSL shader.  Called via glCompileShader().
 */
void
_mesa_glsl_compile_shader(struct gl_context *ctx, struct gl_shader *shader)
{
   struct _mesa_glsl_parse_state *state =
      new(shader) _mesa_glsl_parse_state(ctx, shader->Type, shader);

   const char *source = shader->Source;
   /* Check if the user called glCompileShader without first calling
    * glShaderSource.  This should fail to compile, but not raise a GL_ERROR.
    */
   if (source == NULL) {
      shader->CompileStatus = GL_FALSE;
      return;
   }

   state->error = preprocess(state, &source, &state->info_log,
			     &ctx->Extensions, ctx->API);

   if (ctx->Shader.Flags & GLSL_DUMP) {
      printf("GLSL source for %s shader %d:\n",
	     _mesa_glsl_shader_target_name(state->target), shader->Name);
      printf("%s\n", shader->Source);
   }

   if (!state->error) {
     _mesa_glsl_lexer_ctor(state, source);
     _mesa_glsl_parse(state);
     _mesa_glsl_lexer_dtor(state);
   }

   ralloc_free(shader->ir);
   shader->ir = new(shader) exec_list;
   if (!state->error && !state->translation_unit.is_empty())
      _mesa_ast_to_hir(shader->ir, state);

   if (!state->error && !shader->ir->is_empty()) {
      validate_ir_tree(shader->ir);

      /* Do some optimization at compile time to reduce shader IR size
       * and reduce later work if the same shader is linked multiple times
       */
      while (do_common_optimization(shader->ir, false, 32))
	 ;

      validate_ir_tree(shader->ir);
   }

   shader->symbols = state->symbols;

   shader->CompileStatus = !state->error;
   shader->InfoLog = state->info_log;
   shader->Version = state->language_version;
   memcpy(shader->builtins_to_link, state->builtins_to_link,
	  sizeof(shader->builtins_to_link[0]) * state->num_builtins_to_link);
   shader->num_builtins_to_link = state->num_builtins_to_link;

   if (ctx->Shader.Flags & GLSL_LOG) {
      _mesa_write_shader_to_file(shader);
   }

   if (ctx->Shader.Flags & GLSL_DUMP) {
      if (shader->CompileStatus) {
	 printf("GLSL IR for shader %d:\n", shader->Name);
	 _mesa_print_ir(shader->ir, NULL);
	 printf("\n\n");
      } else {
	 printf("GLSL shader %d failed to compile.\n", shader->Name);
      }
      if (shader->InfoLog && shader->InfoLog[0] != 0) {
	 printf("GLSL shader %d info log:\n", shader->Name);
	 printf("%s\n", shader->InfoLog);
      }
   }

   /* Retain any live IR, but trash the rest. */
   reparent_ir(shader->ir, shader->ir);

   ralloc_free(state);
}


/**
 * Link a GLSL shader program.  Called via glLinkProgram().
 */
void
_mesa_glsl_link_shader(struct gl_context *ctx, struct gl_shader_program *prog)
{
   unsigned int i;

   _mesa_clear_shader_program_data(ctx, prog);

   prog->LinkStatus = GL_TRUE;

   for (i = 0; i < prog->NumShaders; i++) {
      if (!prog->Shaders[i]->CompileStatus) {
	 fail_link(prog, "linking with uncompiled shader");
	 prog->LinkStatus = GL_FALSE;
      }
   }

   prog->Varying = _mesa_new_parameter_list();
   _mesa_reference_vertprog(ctx, &prog->VertexProgram, NULL);
   _mesa_reference_fragprog(ctx, &prog->FragmentProgram, NULL);
   _mesa_reference_geomprog(ctx, &prog->GeometryProgram, NULL);

   if (prog->LinkStatus) {
      link_shaders(ctx, prog);
   }

   if (prog->LinkStatus) {
      if (!ctx->Driver.LinkShader(ctx, prog)) {
	 prog->LinkStatus = GL_FALSE;
      }
   }

   set_uniform_initializers(ctx, prog);

   if (ctx->Shader.Flags & GLSL_DUMP) {
      if (!prog->LinkStatus) {
	 printf("GLSL shader program %d failed to link\n", prog->Name);
      }

      if (prog->InfoLog && prog->InfoLog[0] != 0) {
	 printf("GLSL shader program %d info log:\n", prog->Name);
	 printf("%s\n", prog->InfoLog);
      }
   }
}

} /* extern "C" */
