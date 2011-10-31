/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$
    
    Print the library magic and init code in the file modname_start.c.
    This code is partly based on code in CLib37x.lha from Andreas R. Kleinert
*/
#include "genmodule.h"
#include "oopsupport.h"
#include "muisupport.h"
#include "dtsupport.h"
#include "boopsisupport.h"

static void writedecl(FILE *, struct config *);
static void writedeclsets(FILE *, struct config *);
static void writeresident(FILE *, struct config *);
static void writeinitlib(FILE *, struct config *);
static void writehandler(FILE *, struct config *);
static void writeopenlib(FILE *, struct config *);
static void writecloselib(FILE *, struct config *);
static void writeexpungelib(FILE *, struct config *);
static void writeextfunclib(FILE *, struct config *);
static void writefunctable(FILE *, struct config *);
static void writesets(FILE *, struct config *);

void writestart(struct config *cfg)
{
    FILE *out;
    char line[256], *banner;
    struct classinfo *cl;
    
    snprintf(line, 255, "%s/%s_start.c", cfg->gendir, cfg->modulename);
    out = fopen(line, "w");

    if (out == NULL)
    {
        perror(line);
        exit(20);
    }

    banner = getBanner(cfg);
    fprintf(out, "%s", banner);
    freeBanner(banner);

    writedecl(out, cfg);
    if (!(cfg->options & OPTION_NORESIDENT))
    {
	writeresident(out, cfg);
	writedeclsets(out, cfg);
	writeinitlib(out, cfg);
	if (cfg->modtype != RESOURCE && cfg->modtype != HANDLER)
	{
	    writeopenlib(out, cfg);
	    writecloselib(out, cfg);
	    writeexpungelib(out, cfg);
	    writeextfunclib(out, cfg);
	    if (cfg->modtype == MCC || cfg->modtype == MUI || cfg->modtype == MCP)
	        writemccquery(out, cfg);
	    else if (cfg->modtype == DATATYPE)
	        writeobtainengine(out, cfg);
	}
	writesets(out, cfg);
    }
    
    if (cfg->modtype != HANDLER)
	writefunctable(out, cfg);

    for (cl = cfg->classlist; cl != NULL; cl = cl->next)
    {
	switch (cl->classtype)
	{
	case MCC:
	case MUI:
	case MCP:
	    /* Second argument to next call: the class is not the main class if it is not
	     * the first class or the modtype is not a MUI class
	     */
	    writemccinit(out, cl != cfg->classlist || cfg->modtype != cl->classtype, cl);
	    break;
	case GADGET:
	case DATATYPE:
	case CLASS:
	case IMAGE:
	    writeclassinit(out, cl);
	    break;
	case HIDD:
	    writeoopinit(out, cl);
	    break;
	default:
	    fprintf(stdout, "Internal error: unsupported classtype in writestart\n");
	    exit(20);
	}
    }
    
    fclose(out);
}


static void writedecl(FILE *out, struct config *cfg)
{
    struct stringlist *linelistit;
    int boopsiinc=0, muiinc=0, oopinc=0;
    struct functionhead *funclistit;
    struct functionarg *arglistit;
    struct classinfo *classlistit;
    char *type, *name;
    
    if (cfg->modtype == DEVICE)
    {
	fprintf(out,
		"#include <exec/io.h>\n"
		"#include <exec/errors.h>\n"
	);
    }
    fprintf(out,
	    "#include <exec/types.h>\n"
	    "#include <exec/libraries.h>\n"
	    "#include <exec/resident.h>\n"
	    "#include <aros/libcall.h>\n"
	    "#include <aros/asmcall.h>\n"
	    "#include <aros/symbolsets.h>\n"
	    "#include <dos/dos.h>\n"
	    "\n"
	    "#include \"%s_libdefs.h\"\n"
	    "\n"
	    "#ifdef SysBase\n"
	    "#undef SysBase\n"
	    "#endif\n"
	    "\n"
	    "#include <proto/exec.h>\n"
            "#include <proto/alib.h>\n"
            "#ifndef __AROS__\n"
            "struct ExecBase *SysBase = NULL;\n"
            "#endif\n"
	    "\n",
	    cfg->modulename
    );
    if (cfg->options & OPTION_BASEREL)
       fprintf(out, "#include <aros/relbase.h>\n");

    /* Write out declaration section provided in the config file */
    for (linelistit = cfg->cdeflines; linelistit != NULL; linelistit = linelistit->next)
    {
        fprintf(out, "%s\n", linelistit->s);
    }

    /* Is there a variable for storing the segList ? */
    if (!(cfg->options & OPTION_NOEXPUNGE) && cfg->modtype!=RESOURCE && cfg->modtype != HANDLER)
    {
	fprintf(out,
		"#ifndef GM_SEGLIST_FIELD\n"
		"static BPTR __attribute__((unused)) GM_UNIQUENAME(seglist);\n"
		"#define GM_SEGLIST_FIELD(lh) (GM_UNIQUENAME(seglist))\n"
		"#endif\n"
	);
    }
    if (cfg->options & OPTION_DUPBASE)
        fprintf(out,
		"#ifndef GM_ROOTBASE_FIELD\n"
		"static LIBBASETYPEPTR GM_UNIQUENAME(rootbase);\n"
		"#define GM_ROOTBASE_FIELD(lh) (GM_UNIQUENAME(rootbase))\n"
		"#endif\n"
                "static int __baseslot;\n"
                "LIBBASETYPEPTR __GM_GetBase(void)\n"
                "{\n"
                "    return (LIBBASETYPEPTR)SysBase->ThisTask->tc_UnionETask.tc_ETask->et_TaskStorage[__baseslot];\n"
                "}\n"
                /* On AMD64 don't clobber registers used for argument passing,
                 * only clobber rax, r10 and r11
                 */
                "#if defined __x86_64__\n"
                "asm(\".global __comp_get_relbase\\n\"\n"
                "    \"__comp_get_relbase :\\n\"\n"
                "    \"\\tmovq SysBase(%%rip), %%rax\\n\"\n"
                "    \"\\tmovq 552(%%rax), %%rax\\n\"\n"
                "    \"\\tmovq 56(%%rax), %%rax\\n\"\n"
                "    \"\\tmovq 184(%%rax), %%r10\\n\"\n"
                "    \"\\tmovslq __baseslot(%%rip),%%r11\\n\"\n"
                "    \"\\tmovq (%%r10,%%r11,8), %%rax\\n\"\n"
                "    \"\\tret\\n\"\n"
                ");\n"
                "#else\n"
                "AROS_MAKE_ALIAS(__GM_GetBase, __comp_get_relbase);\n"
                "#endif\n"
                "static inline void __GM_SetBase(LIBBASETYPEPTR base)\n"
                "{\n"
                "    SysBase->ThisTask->tc_UnionETask.tc_ETask->et_TaskStorage[__baseslot] = (IPTR)base;\n"
                "}\n"
                "#if defined __i386__\n"
                "#define GM_INTERNALFUNCSTUB(fname)\\\n"
                "    asm(#fname \"_stub :\\n\"\\\n"
                "        \"\\tpushl %%edx\\n\"\\\n"
                "        \"\\tcall __GM_SetBase\\n\"\\\n"
                "        \"\\tpopl %%edx\\n\"\\\n"
                "        \"\\tjmp \" #fname \"\\n\"\\\n"
                "    )\n"
                "#elif defined __x86_64__\n"
                /* On AMD64 we can't call __GM_SetBase() as it will mess
                 * up with the parameters passed in registers
                 */
                "#define GM_INTERNALFUNCSTUB(fname)\\\n"
                /* On x86_64 we can use rax, r10 and r11 in between function call and entry
                   r11 contains libbase */
                "    asm(#fname \"_stub :\\n\"\\\n"
                "        \"\\tmovq SysBase(%%rip), %%rax\\n\"\\\n"
                "        \"\\tmovq 552(%%rax), %%rax\\n\"\\\n"
                "        \"\\tmovq 56(%%rax), %%rax\\n\"\\\n"
                "        \"\\tmovq 184(%%rax), %%r10\\n\"\\\n"
                "        \"\\tmovslq __baseslot(%%rip),%%rax\\n\"\\\n"
                "        \"\\tmovq %%r11, (%%r10,%%rax,8)\\n\"\\\n"
                "        \"\\tjmp \" #fname \"\\n\"\\\n"
                "    )\n"
                "#elif defined __mc68000__\n"
                "#define GM_INTERNALFUNCSTUB(fname)\\\n"
                "    asm(#fname \"_stub :\\n\"\\\n"
                "        \"\\tmove.l %%a1,%%sp@-\\n\"\\\n"
                "        \"\\tjsr __GM_SetBase\\n\"\\\n"
                "        \"\\tmove.l %%sp@+,%%a1\\n\"\\\n"
                "        \"\\tjmp \" #fname \"\\n\"\\\n"
                "    )\n"
                "#elif defined __powerpc__\n"
                /* FIXME: struct offsets are from i386,
                   need to be checked ! */
                "#define GM_INTERNALFUNCSTUB(fname)\\\n"
                /* On PPC we can use r0, r11, r12 in between function call and entry
                 * r12 contains libbase
                 */
                "    asm(#fname \"_stub :\\n\"\\\n"
                /* r0 = SysBase->ThisTask->tc_UnionETask.tc_ETask->et_TaskStorage */
                "        \"\\tlis 11, SysBase@ha\\n\"\\\n"
                "        \"\\tlwz 11, SysBase@l(11)\\n\"\\\n"
                "        \"\\tlwz 11, 284(11)\\n\"\\\n"
                "        \"\\tlwz 11, 36(11)\\n\"\\\n"
                "        \"\\tlwz 0, 96(11)\\n\"\\\n"
                /* r11 = r0 + __baseslot */
                "        \"\\tlis 11, __baseslot@ha\\n\"\\\n"
                "        \"\\tlwz 11, __baseslot@l(11)\\n\"\\\n"
                "        \"\\tadd 11,0,11\\n\"\\\n"
                /* *r11 = r12 (== libbase) */
                "        \"\\tstw 12,(11)\\n\"\\\n"
                "        \"\\tb \" #fname \"\\n\"\\\n"
                "#elif defined __arm__\n"
                "#define GM_INTERNALFUNCSTUB(fname)\\\n"
                "    asm(#fname \"_stub :\\n\"\\\n"
                "        \"\\tpop {r12}\\n\"\\\n"
                "        \"\\tpush {r0, r1, r2, r3}\\n\"\\\n"
                "        \"\\tldr r0, r12\\n\"\\\n"
                "        \"\\tbl __GM_SetBase\\n\"\\\n" /* r12 may be scratched */
                "        \"\\tpop {r0, r1, r2, r3}\\n\"\\\n"
                "        \"\\tb \" #fname \"\\n\"\\\n" /* call function */
                "#else\n"
                "#   error unsupported CPU type\n"
                "#endif\n"
        );

    if (cfg->options & OPTION_PERTASKBASE)
        fprintf(out,
                "struct __GM_TSBase {\n"
                "    LIBBASETYPE base;\n"
                "    ULONG taskopencount;\n"
                "    struct Task *task;\n"
                "    APTR retaddr;\n"
                "    LIBBASETYPEPTR oldbase;\n"
                "};\n"
                "#define LIBBASESIZE sizeof(struct __GM_TSBase)\n"
                "static int __tsslot;\n"
                "LIBBASETYPEPTR __GM_GetBaseParent(LIBBASETYPEPTR base)\n"
                "{\n"
                "    return ((struct __GM_TSBase *)base)->oldbase;\n"
                "}\n"
                "static inline LIBBASETYPEPTR __GM_GetTSBase(void)\n"
                "{\n"
                "    return (LIBBASETYPEPTR)SysBase->ThisTask->tc_UnionETask.tc_ETask->et_TaskStorage[__tsslot];\n"
                "}\n"
                "static inline void __GM_SetTSBase(LIBBASETYPEPTR base)\n"
                "{\n"
                "    SysBase->ThisTask->tc_UnionETask.tc_ETask->et_TaskStorage[__tsslot] = (IPTR)base;\n"
                "}\n"
        );
    else
        fprintf(out, "#define LIBBASESIZE sizeof(LIBBASETYPE)\n");
    
    for (classlistit = cfg->classlist; classlistit != NULL; classlistit = classlistit->next)
    {
        /* For the main class basename is the same as the module basename */
        if (strcmp(classlistit->basename, cfg->basename) == 0)
        {
            if (classlistit->classptr_var == NULL)
            {
                fprintf(out,
                        "#if !defined(GM_CLASSPTR_FIELD) && !defined(%s_CLASSPTR_FIELD)\n"
                        "static APTR GM_UNIQUENAME(%sClass);\n"
                        "#define GM_CLASSPTR_FIELD(lh) (GM_UNIQUENAME(%sClass))\n"
                        "#define %s_CLASSPTR_FIELD(lh) (GM_UNIQUENAME(%sClass))\n"
                        "#define %s_STORE_CLASSPTR 1\n"
                        "#elif defined(GM_CLASSPTR_FIELD) && !defined(%s_CLASSPTR_FIELD)\n"
                        "#define %s_CLASSPTR_FIELD(lh) (GM_CLASSPTR_FIELD(lh))\n"
                        "#elif !defined(GM_CLASSPTR_FIELD) && defined(%s_CLASSPTR_FIELD)\n"
                        "#define GM_CLASSPTR_FIELD(lh) (%s_CLASSPTR_FIELD(lh))\n"
                        "#endif\n",
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename, classlistit->basename,
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename
                );
            }
            else
            {
                fprintf(out,
                        "#define GM_CLASSPTR_FIELD(lh) (%s)\n"
                        "#define %s_CLASSPTR_FIELD(lh) (%s)\n"
                        "#define %s_STORE_CLASSPTR 1\n",
                        classlistit->classptr_var,
                        classlistit->basename, classlistit->classptr_var,
                        classlistit->basename
		);
            }
        }
        else
        {
            if (classlistit->classptr_var == NULL)
            {
                fprintf(out,
                        "#if !defined(%s_CLASSPTR_FIELD)\n"
                        "static APTR GM_UNIQUENAME(%sClass);\n"
                        "#define %s_CLASSPTR_FIELD(lh) (GM_UNIQUENAME(%sClass))\n"
                        "#define %s_STORE_CLASSPTR 1\n"
                        "#endif\n",
                        classlistit->basename,
                        classlistit->basename,
                        classlistit->basename, classlistit->basename,
                        classlistit->basename
		);
            }
            else
            {
                fprintf(out,
                        "#define %s_CLASSPTR_FIELD(lh) (%s)\n"
                        "#define %s_STORE_CLASSPTR 1\n",
                        classlistit->basename, classlistit->classptr_var,
                        classlistit->basename
                );
            }
        }
    }
    
    /* Write out the defines for the functions of the function table */
    writefuncdefs(out, cfg, cfg->funclist);
    /* Write internal stubs */
    writefuncinternalstubs(out, cfg, cfg->funclist);
    fprintf(out, "\n");


    /* Write out the includes needed for the classes */
    if (cfg->classlist != NULL)
	writeboopsiincludes(out);
    
    for (classlistit = cfg->classlist; classlistit != NULL; classlistit = classlistit->next)
    {
	switch (classlistit->classtype)
	{
	case MUI:
	case MCC:
	case MCP:
	    if (!muiinc)
	    {
		writemuiincludes(out);
		muiinc = 1;
	    }
	    /* Fall through: also write boopsi includes */
	case GADGET:
	case DATATYPE:
	case CLASS:
	case IMAGE:
	    if (!boopsiinc)
	    {
		writeboopsiincludes(out);
		boopsiinc = 1;
	    }
	    break;
	case HIDD:
	    if (!oopinc)
	    {
		writeoopincludes(out);
		oopinc = 1;
	    }
	    break;
	default:
	    fprintf(stderr, "Internal error: unhandled classtype in writedecl\n");
	    exit(20);
	}
    }
}


static void writedeclsets(FILE *out, struct config *cfg)
{
    fprintf(out,
	    "THIS_PROGRAM_HANDLES_SYMBOLSETS\n"
	    "DECLARESET(INIT)\n"
	    "DECLARESET(EXIT)\n"
	    "DECLARESET(CTORS)\n"
	    "DECLARESET(DTORS)\n"
	    "DECLARESET(INITLIB)\n"
	    "DECLARESET(EXPUNGELIB)\n"
    );
    if (!(cfg->options & OPTION_NOOPENCLOSE))
        fprintf(out,
	    "DECLARESET(OPENLIB)\n"
	    "DECLARESET(CLOSELIB)\n"
	);
    if (cfg->modtype == DEVICE)
        fprintf(out,
	    "DECLARESET(OPENDEV)\n"
	    "DECLARESET(CLOSEDEV)\n"
	);
    if (cfg->classlist != NULL)
	fprintf(out,
		"DECLARESET(CLASSESINIT)\n"
		"DECLARESET(CLASSESEXPUNGE)\n"
		"#define ADD2INITCLASSES(symbol, pri) ADD2SET(symbol, classesinit, pri)\n"
		"#define ADD2EXPUNGECLASSES(symbol, pri) ADD2SET(symbol, classesexpunge, pri)\n"
	);
    fprintf(out, "\n");
}


static void writeresident(FILE *out, struct config *cfg)
{
    char *rt_skip = cfg->addromtag;

    if (rt_skip)
    	fprintf(out, "extern const struct Resident %s;\n", rt_skip);
    else
    {
    	rt_skip = "GM_UNIQUENAME(End)";
	fprintf(out, "extern const int %s;\n", rt_skip);
    }
    fprintf(out,
	    "extern const APTR GM_UNIQUENAME(FuncTable)[];\n"
    );
    if (cfg->options & OPTION_RESAUTOINIT)
	fprintf(out, "static const struct InitTable GM_UNIQUENAME(InitTable);\n");
    fprintf(out,
	    "\n"
	    "extern const char GM_UNIQUENAME(LibName)[];\n"
	    "extern const char GM_UNIQUENAME(LibID)[];\n"
	    "extern const char GM_UNIQUENAME(Copyright)[];\n"
	    "\n"
    );

    if (cfg->options & OPTION_RESAUTOINIT)
    {
        if (cfg->options & OPTION_PERTASKBASE)
        {
            fprintf(out,
                    "#define __freebase(lh)\\\n"
                    "do { \\\n"
                    "    UWORD negsize, possize;\\\n"
                    "    UBYTE *negptr = (UBYTE *)lh;\\\n"
                    "    __GM_SetTSBase((LIBBASETYPEPTR)((struct __GM_TSBase *)lh)->oldbase);\\\n"
                    "    negsize = ((struct Library *)lh)->lib_NegSize;\\\n"
                    "    negptr -= negsize;\\\n"
                    "    possize = ((struct Library *)lh)->lib_PosSize;\\\n"
                    "    FreeMem (negptr, negsize+possize);\\\n"
                    "} while(0)\n"
                    "\n"
            );
        }
        else /* !pertaskbase */
        {
            fprintf(out,
                    "#define __freebase(lh)\\\n"
                    "do {\\\n"
                    "    UWORD negsize, possize;\\\n"
                    "    UBYTE *negptr = (UBYTE *)lh;\\\n"
                    "    negsize = ((struct Library *)lh)->lib_NegSize;\\\n"
                    "    negptr -= negsize;\\\n"
                    "    possize = ((struct Library *)lh)->lib_PosSize;\\\n"
                    "    FreeMem (negptr, negsize+possize);\\\n"
                    "} while(0)\n"
                    "\n"
       	    );
        }
    }
	
    fprintf(out,
	    "AROS_UFP3 (LIBBASETYPEPTR, GM_UNIQUENAME(InitLib),\n"
	    "    AROS_UFPA(LIBBASETYPEPTR, lh, D0),\n"
	    "    AROS_UFPA(BPTR, segList, A0),\n"
	    "    AROS_UFPA(struct ExecBase *, sysBase, A6)\n"
	    ");\n"
    );
    if (cfg->modtype != RESOURCE && cfg->modtype != HANDLER)
    {
	fprintf(out,
		"AROS_LP1(BPTR, GM_UNIQUENAME(ExpungeLib),\n"
		"    AROS_LPA(LIBBASETYPEPTR, extralh, D0),\n"
		"    LIBBASETYPEPTR, lh, 3, %s\n"
		");\n"
		"\n",
		cfg->basename
	);
    }
    fprintf(out,
	    "struct Resident const GM_UNIQUENAME(ROMTag) =\n"
	    "{\n"
	    "    RTC_MATCHWORD,\n"
	    "    (struct Resident *)&GM_UNIQUENAME(ROMTag),\n"
	    "    (APTR)&%s,\n"
	    "    RESIDENTFLAGS,\n"
	    "    VERSION_NUMBER,\n",
	    rt_skip
    );
    switch (cfg->modtype)
    {
    case LIBRARY:
    case MUI:
    case MCC:
    case MCP:
    case GADGET:
    case DATATYPE:
    case USBCLASS:
    case HIDD:
	fprintf(out, "    NT_LIBRARY,\n");
	break;
    case DEVICE:
	fprintf(out, "    NT_DEVICE,\n");
	break;
    case RESOURCE:
    case HANDLER:
	fprintf(out, "    NT_RESOURCE,\n");
	break;
    default:
	fprintf(stderr, "Internal error: unsupported modtype for NT_...\n");
	exit(20);
	break;
    }
    fprintf(out,
	    "    RESIDENTPRI,\n"
	    "    (CONST_STRPTR)&GM_UNIQUENAME(LibName)[0],\n"
	    "    (CONST_STRPTR)&GM_UNIQUENAME(LibID)[6],\n"
    );
    if (cfg->options & OPTION_RESAUTOINIT)
    {
	fprintf(out,
		"    (APTR)&GM_UNIQUENAME(InitTable)\n"
		"};\n"
		"\n"
		"static struct InitTable\n"
		"{\n"
		"    IPTR              Size;\n"
		"    const APTR       *FuncTable;\n"
		"    struct DataTable *DataTable;\n"
		"    APTR              InitLibTable;\n"
		"}\n"
		"const GM_UNIQUENAME(InitTable) =\n"
		"{\n"
		"    LIBBASESIZE,\n"
		"    &GM_UNIQUENAME(FuncTable)[0],\n"
		"    NULL,\n"
		"    (APTR)GM_UNIQUENAME(InitLib)\n"
		"};\n"
	);
    }
    else
	fprintf(out, "    (APTR)GM_UNIQUENAME(InitLib)\n};\n");
    
    fprintf(out,
	    "\n"
	    "const char GM_UNIQUENAME(LibName)[] = MOD_NAME_STRING;\n"
	    "const char GM_UNIQUENAME(LibID)[] = VERSION_STRING;\n"
	    "const char GM_UNIQUENAME(Copyright)[] = COPYRIGHT_STRING;\n"
	    "\n"
    );
}

static void writehandler(FILE *out, struct config *cfg)
{
    int i;
    struct handlerinfo *hl;

    fprintf(out,
               "\n"
               "#include <resources/filesysres.h>\n"
               "#include <aros/system.h>\n"
               "#include <proto/arossupport.h>\n"
               "#include <proto/dos.h>\n"
               "\n"
               );

    for (hl = cfg->handlerlist; hl != NULL; hl = hl->next) {
        fprintf(out,
               "extern void %s(void);\n",
               hl->handler);
    }

    fprintf(out,
               "\n"
               "void GM_UNIQUENAME(InitHandler)(void)\n"
               "{\n"
               "    struct FileSysResource *fsr;\n"
               "    int i;\n"
               "    const struct {\n"
               "        ULONG id;\n"
               "        BSTR name;\n"
               "        BYTE autodetect;\n"
               "        BYTE priority;\n"
               "        ULONG stacksize;\n"
               "        void (*handler)(void);\n"
               "    } const __handler[] = { \n");
    for (hl = cfg->handlerlist; hl != NULL; hl = hl->next)
    {
        switch (hl->type)
        {
        case HANDLER_RESIDENT:
            fprintf(out,
               "        { .id = 0, .name = AROS_CONST_BSTR(\"%s\"), .handler = %s }, \n",
               hl->name, hl->handler);
            break;
        case HANDLER_DOSTYPE:
            fprintf(out,
               "        { .id = 0x%08x, .name = AROS_CONST_BSTR(MOD_NAME_STRING), .handler = %s, .autodetect = %d, .priority = %d, .stacksize = %d*sizeof(IPTR) }, \n",
               hl->id, hl->handler, hl->autodetect, hl->priority, hl->stacksize);
            break;
        }
    }
    fprintf(out,
               "    };\n"
               "    BPTR seg[sizeof(__handler)/sizeof(__handler[0])] = { };\n"
               "\n"
               "    fsr = (struct FileSysResource *)OpenResource(\"FileSystem.resource\");\n"
               "    if (fsr == NULL)\n"
               "        return;\n"
               "\n"
               "    for (i = 0; i < sizeof(__handler)/sizeof(__handler[0]); i++) {\n"
               "        struct FileSysEntry *fse;\n"
               "        int j;\n"
               "\n"
               "        /* Check to see if we can allocate the memory for the fse */\n"
               "        fse = AllocMem(sizeof(*fse), MEMF_CLEAR);\n"
               "        if (!fse)\n"
               "            return;\n"
               "\n"
               "        /* Did we already make a segment for this handler? */\n"
               "        for (j = 0; j < i; j++)\n"
               "            if (__handler[i].handler == __handler[j].handler)\n"
               "                break;\n"
               "        if (seg[j] == (BPTR)0)\n"
               "            seg[j] = CreateSegList(__handler[j].handler);\n"
               "        if (seg[j] == BNULL) {\n"
               "            FreeMem(fse, sizeof(*fse));\n"
               "            return;\n"
               "        }\n"
               " \n"
               "        /* DOS ID based handlers\n"
               "         * NOTE: fse_DosType == 0 is a special flag use in\n"
               "         * dos.library's init to add the handler to the\n"
               "         * resident segment list\n"
               "         */\n"
               "        fse->fse_Node.ln_Name = VERSION_STRING;\n"
               "        fse->fse_Node.ln_Pri  = __handler[i].autodetect;\n"
               "        fse->fse_DosType = __handler[i].id;\n"
               "        fse->fse_Version = (MAJOR_VERSION << 16) | MINOR_VERSION;\n"
               "        fse->fse_PatchFlags = FSEF_SEGLIST | FSEF_GLOBALVEC | FSEF_HANDLER | FSEF_PRIORITY;\n"
               "        if (__handler[i].stacksize) {\n"
               "            fse->fse_PatchFlags |= FSEF_STACKSIZE;\n"
               "            fse->fse_StackSize = __handler[i].stacksize;\n"
               "        }\n"
               "        fse->fse_Handler = __handler[i].name;\n"
               "        fse->fse_Priority = __handler[i].priority;\n"
               "        fse->fse_SegList = seg[j];\n"
               "        fse->fse_GlobalVec = (BPTR)(SIPTR)-1;\n"
               "    \n"
               "        /* Add to the list. I know forbid and permit are\n"
               "         * a little unnecessary for the pre-multitasking state\n"
               "         * we should be in at this point, but you never know\n"
               "         * who's going to blindly copy this code as an example.\n"
               "         */\n"
               "        Forbid();\n"
               "        Enqueue(&fsr->fsr_FileSysEntries, (struct Node *)fse);\n"
               "        Permit();\n"
               "    }\n"
               "}\n");
}

static void writeinitlib(FILE *out, struct config *cfg)
{
    if (cfg->handlerlist)
        writehandler(out, cfg);

    fprintf(out,
	    "AROS_UFH3 (LIBBASETYPEPTR, GM_UNIQUENAME(InitLib),\n"
	    "    AROS_UFHA(LIBBASETYPEPTR, lh, D0),\n"
	    "    AROS_UFHA(BPTR, segList, A0),\n"
	    "    AROS_UFHA(struct ExecBase *, sysBase, A6)\n"
	    ")\n"
	    "{\n"
	    "    AROS_USERFUNC_INIT\n"
	    "\n"
	    "    int ok;\n"
    );
    if (cfg->modtype != HANDLER)
	fprintf(out,
	    "    int initcalled = 0;\n"
	);
    fprintf(out,
	    "\n"
    );

    if (cfg->options & OPTION_RESAUTOINIT) {
        fprintf(out,
                "#ifndef __AROS__\n"
                "    SysBase = sysBase;\n"
                "#endif\n"
                "#ifdef GM_SYSBASE_FIELD\n"
                "    GM_SYSBASE_FIELD(lh) = (APTR)sysBase;\n"
                "#endif\n"
                "#ifdef GM_OOPBASE_FIELD\n"
                "    GM_OOPBASE_FIELD(lh) = OpenLibrary(\"oop.library\",0);\n"
                "    if (GM_OOPBASE_FIELD(lh) == NULL)\n"
                "        return NULL;\n"
                "#endif\n"
        );
    }

    if (cfg->modtype != HANDLER)
    {
    	if (!(cfg->options & OPTION_RESAUTOINIT))
    	{
	    fprintf(out,
		"    int vecsize;\n"
		"    struct Node *n;\n"
		"    char *mem;\n"
		"\n"
		"    vecsize = FUNCTIONS_COUNT * LIB_VECTSIZE;\n"
		"    if (vecsize > 0)\n"
		"        vecsize = ((vecsize-1)/sizeof(IPTR) + 1)*sizeof(IPTR);\n"
		"    mem = AllocMem(vecsize+sizeof(LIBBASETYPE), MEMF_PUBLIC|MEMF_CLEAR);\n"
		"    if (mem == NULL)\n"
		"         return NULL;\n"
		"    lh = (LIBBASETYPEPTR)(mem + vecsize);\n"
		"    n = (struct Node *)lh;\n"
		"    n->ln_Type = NT_RESOURCE;\n"
		"    n->ln_Pri = RESIDENTPRI;\n"
		"    n->ln_Name = (char *)GM_UNIQUENAME(LibName);\n"
		"    MakeFunctions(lh, (APTR)GM_UNIQUENAME(FuncTable), NULL);\n"
	    );
	    if ((cfg->modtype != RESOURCE) && (cfg->options & OPTION_SELFINIT))
	    {
		fprintf(out,
			"    ((struct Library*)lh)->lib_NegSize = vecsize;\n"
			"    ((struct Library*)lh)->lib_PosSize = sizeof(LIBBASETYPE);\n"
		);
		
	    }
	}
    	else
    	{
	    fprintf(out,
		"    ((struct Library *)lh)->lib_Revision = REVISION_NUMBER;\n"
	    );
    	}
    }

    if (cfg->options & OPTION_BASEREL)
    	fprintf(out, "    void *oldbase = AROS_SET_LIBBASE(lh);\n");
    if (cfg->options & OPTION_DUPBASE)
        fprintf(out,
                "    __baseslot = AllocTaskStorageSlot();\n"
                "    __GM_SetBase(lh);\n"
        );
    if (cfg->options & OPTION_PERTASKBASE)
        fprintf(out,
                "    __tsslot = AllocTaskStorageSlot();\n"
        );

    if (!(cfg->options & OPTION_NOEXPUNGE) && cfg->modtype!=RESOURCE && cfg->modtype != HANDLER)
	fprintf(out, "    GM_SEGLIST_FIELD(lh) = segList;\n");
    if (cfg->options & OPTION_DUPBASE)
	fprintf(out, "    GM_ROOTBASE_FIELD(lh) = (LIBBASETYPEPTR)lh;\n");
    fprintf(out, "    if (");
    if (!(cfg->options & OPTION_NOAUTOLIB))
	fprintf(out, "set_open_libraries() && ");
    if (cfg->classlist != NULL)
	fprintf(out, "set_call_libfuncs(SETNAME(CLASSESINIT), 1, 1, lh) && ");
    fprintf(out,
	    "set_call_funcs(SETNAME(INIT), 1, 1) )\n"
	    "    {\n"
	    "        set_call_funcs(SETNAME(CTORS), -1, 0);\n"
	    "\n"
    );
    
    if (cfg->modtype == HANDLER)
    	fprintf(out,
    	    "        ok = 1;\n");
    else
    	fprintf(out,
	    "        initcalled = 1;\n"
	    "        ok = set_call_libfuncs(SETNAME(INITLIB), 1, 1, lh);\n");
    fprintf(out,
	    "    }\n"
	    "    else\n"
	    "        ok = 0;\n"
	    "\n"
	    "    if (!ok)\n"
	    "    {\n"
    );
    
    if (cfg->modtype != HANDLER)
    	fprintf(out,
	    "        if (initcalled)\n"
	    "            set_call_libfuncs(SETNAME(EXPUNGELIB), -1, 0, lh);\n");

    fprintf(out,
	    "        set_call_funcs(SETNAME(DTORS), 1, 0);\n"
	    "        set_call_funcs(SETNAME(EXIT), -1, 0);\n"
    );
    if (cfg->classlist != NULL)
	fprintf(out, "        set_call_libfuncs(SETNAME(CLASSESEXPUNGE), -1, 0, lh);\n");
    if (!(cfg->options & OPTION_NOAUTOLIB))
	fprintf(out, "        set_close_libraries();\n");
    if (cfg->options & OPTION_BASEREL)
	fprintf(out, "        (void)AROS_SET_LIBBASE(oldbase);\n");

    if (cfg->modtype != HANDLER)
    {
    	if (cfg->options & OPTION_RESAUTOINIT)
    	{
	    fprintf(out,
		"\n"
		"        __freebase(lh);\n"
	    );
    	}
    	else
    	{
	    fprintf(out,
		"\n"
		"        FreeMem(mem, vecsize+LIBBASESIZE);\n"
	    );
    	}
    }
    fprintf(out,
	    "        return NULL;\n"
	    "    }\n"
	    "    else\n"
	    "    {\n"
    );

    if (!(cfg->options & OPTION_RESAUTOINIT) && !(cfg->options & OPTION_SELFINIT))
    {
    	switch (cfg->modtype)
    	{
    	case RESOURCE:
    	    fprintf(out, "        AddResource(lh);\n");
    	    break;

    	case DEVICE:
    	    fprintf(out, "        AddDevice(lh);\n");

    	case HANDLER:
    	    /* Bare handlers don't require adding at all */
    	    break;

    	default:
    	    /* Everything else is library */
	    fprintf(out, "        AddLibrary(lh);\n");
	    break;
    	}
    }

    if (cfg->handlerlist)
        fprintf(out, "        GM_UNIQUENAME(InitHandler)();\n");

    if (cfg->options & OPTION_BASEREL)
	fprintf(out,
            "        (void)AROS_SET_LIBBASE(oldbase);\n");
    fprintf(out,
	    "        return  lh;\n"
	    "    }\n"
	    "\n"
	    "    AROS_USERFUNC_EXIT\n"
	    "}\n"
	    "\n"
    );
}

static void writeopenlib(FILE *out, struct config *cfg)
{
    switch (cfg->modtype)
    {
    case RESOURCE:
	fprintf(stderr, "Internal error: writeopenlib called for a resource\n");
	break;
    case HANDLER:
	fprintf(stderr, "Internal error: writeopenlib called for a handler\n");
	break;
    case DEVICE:
	if (cfg->options & OPTION_NOOPENCLOSE)
	    fprintf(out,
		"AROS_LD3 (void, GM_UNIQUENAME(OpenLib),\n"
		"    AROS_LHA(struct IORequest *, ioreq, A1),\n"
		"    AROS_LHA(ULONG, unitnum, D0),\n"
		"    AROS_LHA(ULONG, flags, D1),\n"
		"    LIBBASETYPEPTR, lh, 1, %s\n"
		");\n",
		cfg->basename
	    );
	else
	{
	    fprintf(out,
		"AROS_LH3 (void, GM_UNIQUENAME(OpenLib),\n"
		"    AROS_LHA(struct IORequest *, ioreq, A1),\n"
		"    AROS_LHA(IPTR, unitnum, D0),\n"
		"    AROS_LHA(ULONG, flags, D1),\n"
		"    LIBBASETYPEPTR, lh, 1, %s\n"
		")\n",
		cfg->basename
	    );
	    fprintf(out,
		"{\n"
		"    AROS_LIBFUNC_INIT\n"
		"\n"
		"    if ( set_call_libfuncs(SETNAME(OPENLIB), 1, 1, lh)\n"
		"         && set_call_devfuncs(SETNAME(OPENDEV), 1, 1, lh, ioreq, unitnum, flags)\n"
		"    )\n"
		"    {\n"
		"        ((struct Library *)lh)->lib_OpenCnt++;\n"
		"        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
		"\n"
		"        ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;\n"
		"    }\n"
		"    else\n"
		"    {\n"
		"        if (ioreq->io_Error >= 0)\n"
		"            ioreq->io_Error = IOERR_OPENFAIL;\n"
		"    }\n"
		"\n"
		"    return;\n"
		"\n"
		"    AROS_LIBFUNC_EXIT\n"
		"}\n"
		"\n"
	    );
	}
	break;
    default:
	if (cfg->options & OPTION_NOOPENCLOSE)
	{
	    fprintf(out,
		"AROS_LD1 (LIBBASETYPEPTR, GM_UNIQUENAME(OpenLib),\n"
		"    AROS_LHA (ULONG, version, D0),\n"
		"    LIBBASETYPEPTR, lh, 1, %s\n"
		");\n",
		cfg->basename
	    );
	    return;
	}
	fprintf(out,
		"AROS_LH1 (LIBBASETYPEPTR, GM_UNIQUENAME(OpenLib),\n"
		"    AROS_LHA (ULONG, version, D0),\n"
		"    LIBBASETYPEPTR, lh, 1, %s\n"
		")\n"
		"{\n"
		"    AROS_LIBFUNC_INIT\n"
		"\n",
		cfg->basename
	);
	if (!(cfg->options & OPTION_DUPBASE))
	{
	    fprintf(out,
		    "    if ( set_call_libfuncs(SETNAME(OPENLIB), 1, 1, lh) )\n"
		    "    {\n"
		    "        ((struct Library *)lh)->lib_OpenCnt++;\n"
		    "        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
		    "\n"
		    "        return lh;\n"
		    "    }\n"
		    "\n"
		    "    return NULL;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	    );
	}
	else /* OPTION_DUPBASE */
	{
	    fprintf(out,
		    "    struct Library *newlib = NULL;\n"
		    "    UWORD possize = ((struct Library *)lh)->lib_PosSize;\n"
            );
            if (cfg->options & OPTION_PERTASKBASE)
                fprintf(out,
                        "    struct Task *thistask = FindTask(NULL);\n"
                        "    LIBBASETYPEPTR oldbase = __GM_GetTSBase();\n"
                        "    newlib = (struct Library *)oldbase;\n"
                        "    if (newlib)\n"
                        "    {\n"
                        "        struct __GM_TSBase *tslib = (struct __GM_TSBase *)newlib;\n"
                        "        if (tslib->task != thistask)\n"
                        "            newlib = NULL;\n"
                        "        else if (thistask->tc_Node.ln_Type == NT_PROCESS\n"
                        "                 && tslib->retaddr != ((struct Process *)thistask)->pr_ReturnAddr\n"
                        "        )\n"
                        "            newlib = NULL;\n"
                        "        else\n"
                        "            tslib->taskopencount++;\n"
                        "    }\n"
                );

            fprintf(out,
		    "\n"
                    "    if (newlib == NULL)\n"
                    "    {\n"
		    "        newlib = MakeLibrary(GM_UNIQUENAME(InitTable).FuncTable,\n"
		    "                             GM_UNIQUENAME(InitTable).DataTable,\n"
		    "                             NULL,\n"
		    "                             GM_UNIQUENAME(InitTable).Size,\n"
		    "                             (BPTR)NULL\n"
		    "        );\n"
		    "        if (newlib == NULL)\n"
		    "            return NULL;\n"
		    "\n"
		    "        CopyMem(lh, newlib, possize);\n"
                    "        __GM_SetBase((LIBBASETYPEPTR)newlib);\n"
            );
            if (cfg->options & OPTION_PERTASKBASE)
                fprintf(out,
                        "        struct __GM_TSBase *tslib = (struct __GM_TSBase *)newlib;\n"
                        "        tslib->task = thistask;\n"
                        "        if (thistask->tc_Node.ln_Type == NT_PROCESS)\n"
                        "             tslib->retaddr = ((struct Process *)thistask)->pr_ReturnAddr;\n"
                        "        tslib->oldbase = oldbase;\n"
                        "        tslib->taskopencount = 1;\n"
                        "        __GM_SetTSBase((LIBBASETYPEPTR)newlib);\n"
                );

	    if (cfg->options & OPTION_BASEREL)
            	fprintf(out,
		    "\n"
                    "        void *oldbase = AROS_SET_LIBBASE(newlib);");
	    fprintf(out,
                    "\n"
		    "        if (!(set_open_rellibraries(newlib)\n"
                    "              && set_call_libfuncs(SETNAME(OPENLIB), 1, 1, newlib)\n"
                    "             )\n"
                    "        )\n"
		    "        {\n");
	    if (cfg->options & OPTION_BASEREL)
	    	fprintf(out,
                    "            (void)AROS_SET_LIBBASE(oldbase);\n");
            fprintf(out,
		    "            __freebase(newlib);\n"
		    "            return NULL;\n"
		    "        }\n");
	    if (cfg->options & OPTION_BASEREL)
	    	fprintf(out,
                    "        (void)AROS_SET_LIBBASE(oldbase);\n");
            fprintf(out,
		    "\n"
		    "        ((struct Library *)lh)->lib_OpenCnt++;\n"
		    "        ((struct Library *)lh)->lib_Flags &= ~LIBF_DELEXP;\n"
                    "    }\n"
		    "\n"
		    "    return (LIBBASETYPEPTR)newlib;\n"
		    "\n"
		    "    AROS_LIBFUNC_EXIT\n"
		    "}\n"
		    "\n"
	    );
	}
    }
}


static void writecloselib(FILE *out, struct config *cfg)
{
    if (cfg->options & OPTION_NOOPENCLOSE)
    {
	if (cfg->modtype != DEVICE)
	    fprintf(out,
		"AROS_LD0 (BPTR, GM_UNIQUENAME(CloseLib),\n"
		"    LIBBASETYPEPTR, lh, 2, %s\n"
		");\n",
		cfg->basename
	    );
	else
	    fprintf(out,
		"AROS_LD1(BPTR, GM_UNIQUENAME(CloseLib),\n"
		"    AROS_LHA(struct IORequest *, ioreq, A1),\n"
		"    LIBBASETYPEPTR, lh, 2, %s\n"
		");\n",
		cfg->basename
	    );
	return;
    }
    if (cfg->modtype != DEVICE)
	fprintf(out,
		"AROS_LH0 (BPTR, GM_UNIQUENAME(CloseLib),\n"
		"    LIBBASETYPEPTR, lh, 2, %s\n"
		")\n",
		cfg->basename
	);
    else
	fprintf(out,
		"AROS_LH1(BPTR, GM_UNIQUENAME(CloseLib),\n"
		"    AROS_LHA(struct IORequest *, ioreq, A1),\n"
		"    LIBBASETYPEPTR, lh, 2, %s\n"
		")\n",
		cfg->basename
	);
	
    fprintf(out,
	    "{\n"
	    "    AROS_LIBFUNC_INIT\n"
	    "\n"
    );
    if (cfg->options & OPTION_BASEREL)
    	fprintf(out,
            "    void *oldbase = AROS_SET_LIBBASE(lh);\n"
            "\n"
        );
    if (cfg->modtype == DEVICE)
    {
	fprintf(out,
		"    if (!set_call_devfuncs(SETNAME(CLOSEDEV), -1, 1, lh, ioreq, 0, 0))\n"
                "    {\n");
    	if (cfg->options & OPTION_BASEREL)
    	    fprintf(out,
                "        (void)AROS_SET_LIBBASE(oldbase);\n");
    	fprintf(out, 
		"        return BNULL;\n"
                "    }\n"
	);
    }
    if (!(cfg->options & OPTION_DUPBASE))
    {
	fprintf(out,
		"    ((struct Library *)lh)->lib_OpenCnt--;\n"
		"    set_call_libfuncs(SETNAME(CLOSELIB), -1, 0, lh);\n"
	);
    }
    else
    {
	fprintf(out,
		"    LIBBASETYPEPTR rootbase = GM_ROOTBASE_FIELD(lh);\n"
        );
        if (cfg->options & OPTION_PERTASKBASE)
            fprintf(out,
                    "    struct __GM_TSBase *tsbase = (struct __GM_TSBase *)lh;\n"
                    "    tsbase->taskopencount--;\n"
                    "    if (tsbase->taskopencount != 0)\n"
                    "        return BNULL;\n"
            );
        fprintf(out,
                "\n"
		"    set_call_libfuncs(SETNAME(CLOSELIB), -1, 0, lh);\n"
                "    set_close_rellibraries(lh);\n"
                "    __freebase(lh);\n"
		"    lh = rootbase;\n"
		"    ((struct Library *)lh)->lib_OpenCnt--;\n"
		"\n"
	);
    }
    if (!(cfg->options & OPTION_NOEXPUNGE))
    {
	fprintf(out,
		"    if\n"
		"    (\n"
		"        (((struct Library *)lh)->lib_OpenCnt == 0)\n"
		"        && (((struct Library *)lh)->lib_Flags & LIBF_DELEXP)\n"
		"    )\n"
		"    {\n");
	if (cfg->options & OPTION_BASEREL)
	    fprintf(out,
                "        (void)AROS_SET_LIBBASE(oldbase);\n");
        fprintf(out,
		"        return AROS_LC1(BPTR, GM_UNIQUENAME(ExpungeLib),\n"
		"                   AROS_LCA(LIBBASETYPEPTR, lh, D0),\n"
		"                   LIBBASETYPEPTR, lh, 3, %s\n"
		"        );\n"
		"    }\n",
		cfg->basename
	);
    }
    if (cfg->options & OPTION_BASEREL)
    	fprintf(out,
	    "\n"
            "    (void)AROS_SET_LIBBASE(oldbase);");
    fprintf(out,
    	    "\n"
	    "    return BNULL;\n"
	    "\n"
	    "    AROS_LIBFUNC_EXIT\n"
	    "}\n"
	    "\n"
    );
}


static void writeexpungelib(FILE *out, struct config *cfg)
{
    fprintf(out,
	    "AROS_LH1 (BPTR, GM_UNIQUENAME(ExpungeLib),\n"
	    "    AROS_LHA(LIBBASETYPEPTR, extralh, D0),\n"
	    "    LIBBASETYPEPTR, lh, 3, %s\n"
	    ")\n",
	    cfg->basename
    );
    fprintf(out,
	    "{\n"
	    "    AROS_LIBFUNC_INIT\n"
	    "\n"
    );
    if (!(cfg->options & OPTION_NOEXPUNGE))
    {
	fprintf(out,
		"\n"
		"    if ( ((struct Library *)lh)->lib_OpenCnt == 0 )\n"
		"    {\n"
		"        BPTR seglist = GM_SEGLIST_FIELD(lh);\n"
		"\n");
	if (cfg->options & OPTION_BASEREL)
	    fprintf(out,
                "        void *oldbase = AROS_SET_LIBBASE(lh);\n"
                "\n");
        fprintf(out,
		"        if(!set_call_libfuncs(SETNAME(EXPUNGELIB), -1, 1, lh))\n"
		"        {\n"
		"            ((struct Library *)lh)->lib_Flags |= LIBF_DELEXP;\n"
		"            return BNULL;\n"
		"        }\n"
		"\n"
		"        Remove((struct Node *)lh);\n"
		"\n"
		"        set_call_funcs(SETNAME(DTORS), 1, 0);\n"
		"        set_call_funcs(SETNAME(EXIT), -1, 0);\n"
	);
	if (cfg->classlist != NULL)
	    fprintf(out, "        set_call_libfuncs(SETNAME(CLASSESEXPUNGE), -1, 0, lh);\n");
	if (!(cfg->options & OPTION_NOAUTOLIB))
	    fprintf(out, "        set_close_libraries();\n"
	                 "#ifdef GM_OOPBASE_FIELD\n"
	                 "        CloseLibrary((struct Library *)GM_OOPBASE_FIELD(lh));\n"
	                 "#endif\n"
	            );
	if (cfg->options & OPTION_BASEREL)
	    fprintf(out,
	    	"\n"
                "        (void)AROS_SET_LIBBASE(oldbase);");
        if (cfg->options & OPTION_PERTASKBASE)
            fprintf(out,
                    "        FreeTaskStorageSlot(__tsslot);\n"
                    "        __tsslot = 0;\n"
            );
        fprintf(out,
        	"\n"
		"        __freebase(lh);\n"
		"\n"
		"        return seglist;\n"
		"    }\n"
		"\n"
		"    ((struct Library *)lh)->lib_Flags |= LIBF_DELEXP;\n"
	);
    }
    fprintf(out,
	    "\n"
	    "    return BNULL;\n"
	    "\n"
	    "    AROS_LIBFUNC_EXIT\n"
	    "}\n"
	    "\n"
    );
}


static void writeextfunclib(FILE *out, struct config *cfg)
{
    fprintf(out,
	    "AROS_LH0 (LIBBASETYPEPTR, GM_UNIQUENAME(ExtFuncLib),\n"
	    "    LIBBASETYPEPTR, lh, 4, %s\n"
	    ")\n"
	    "{\n"
	    "    AROS_LIBFUNC_INIT\n"
	    "    return NULL;\n"
	    "    AROS_LIBFUNC_EXIT\n"
	    "}\n"
	    "\n",
	    cfg->basename
    );
}


static void
writefunctable(FILE *out,
	       struct config *cfg
)
{
    struct functionhead *funclistit = cfg->funclist;
    struct functionarg *arglistit;
    unsigned int lvo;
    int i;
    char *name, *type;
    int lastversion = 0;
    
    /* lvo contains the number of functions already printed in the functable */
    lvo = 0;
    
    if (!(cfg->options & OPTION_NORESIDENT))
    {
	fprintf(out,
		"\n"
		"const APTR GM_UNIQUENAME(FuncTable)[]=\n"
		"{\n"
	);
	if (cfg->modtype != RESOURCE && cfg->modtype != HANDLER)
	{
	    fprintf(out,
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(OpenLib),%s,1),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(CloseLib),%s,2),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(ExpungeLib),%s,3),\n"
		    "    &AROS_SLIB_ENTRY(GM_UNIQUENAME(ExtFuncLib),%s,4),\n",
		    cfg->basename, cfg->basename, cfg->basename, cfg->basename
	    );
	    lvo += 4;
	}
        if (cfg->modtype == MCC || cfg->modtype == MUI || cfg->modtype == MCP)
	{
	    lvo++;
	    fprintf(out,
		    "    &AROS_SLIB_ENTRY(MCC_Query,%s,%d),\n",
		    cfg->basename, lvo
	    );
	}
        else if (cfg->modtype == DATATYPE)
	{
	    lvo++;
	    fprintf(out,
		    "    &AROS_SLIB_ENTRY(ObtainEngine,%s,%d),\n",
		   cfg->basename, lvo
	    );
	}
    }
    else /* NORESIDENT */
    {
	if (cfg->modtype != RESOURCE && cfg->modtype != HANDLER)
	{
	    int neednull = 0;
	    struct functionhead *funclistit2;
	
	    if (funclistit->lvo != 1)
	    {
		fprintf(stderr, "Module without a generated resident structure has to provide the Open function (LVO==1)\n");
		exit(20);
	    }
	    else
		funclistit = funclistit->next;
	
	    if (funclistit->lvo != 2)
	    {
		fprintf(stderr, "Module without a generated resident structure has to provide the Close function (LVO==2)\n");
		exit(20);
	    }
	    else
		funclistit = funclistit->next;
	
	    if (funclistit->lvo == 3)
		funclistit = funclistit->next;
	    else
		neednull = 1;
	
	    if (funclistit->lvo == 4)
		funclistit = funclistit->next;
	    else
		neednull = 1;

	    if (neednull)
		fprintf(out,
			"\n"
			"AROS_UFH1S(int, %s_null,\n"
			"          AROS_UFHA(struct Library *, libbase, A6)\n"
			")\n"
			"{\n"
			"    AROS_USERFUNC_INIT\n"
			"    return 0;\n"
			"    AROS_USERFUNC_EXIT\n"
			"}\n",
			cfg->modulename
		);
	
	    funclistit = cfg->funclist;
	    funclistit2 = funclistit->next;
	    fprintf(out,
		    "\n"
		    "const APTR GM_UNIQUENAME(FuncTable)[]=\n"
		    "{\n"
		    "    &AROS_SLIB_ENTRY(%s,%s,%d),\n"
		    "    &AROS_SLIB_ENTRY(%s,%s,%d),\n",
		    funclistit->internalname, cfg->basename, lvo+1,
		    funclistit2->internalname, cfg->basename, lvo+2
	    );
	    lvo += 2;
	    funclistit = funclistit2->next;

	    if (funclistit->lvo == 3)
	    {
		fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s,%d),\n",
			funclistit->internalname, cfg->basename, lvo+1
		);
		funclistit = funclistit->next;
	    }
	    else
		fprintf(out, "    &%s_null,\n", cfg->modulename);
	    lvo++;
	    
	    if (funclistit->lvo == 4)
	    {
		fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s,%d),\n",
			funclistit->internalname, cfg->basename, lvo+1
		);
		funclistit = funclistit->next;
	    }
	    else
		fprintf(out, "    &%s_null,\n", cfg->modulename);
	    lvo++;
	}
	else
	{
	    fprintf(out,
		    "\n"
		    "const APTR GM_UNIQUENAME(FuncTable)[]=\n"
		    "{\n");
	}
    }

    while (funclistit != NULL)
    {
	for (i = lvo+1; i<funclistit->lvo; i++)
	    fprintf(out, "    NULL,\n");
	lvo = funclistit->lvo;
	
	switch (funclistit->libcall)
	{
	case STACK:
	    fprintf(out, "    &%s%s,\n", funclistit->internalname,
                    cfg->options & OPTION_DUPBASE ? "_stub" : ""
            );
	    break;
	    
	case REGISTER:
	case REGISTERMACRO:
	    if (funclistit->version != lastversion) {
	        lastversion = funclistit->version;
	        fprintf(out, "    /* Version %d */\n", lastversion);
	    }
	    fprintf(out, "    &AROS_SLIB_ENTRY(%s,%s,%d),\n", funclistit->internalname, cfg->basename, lvo);
	    break;
	    
	default:
	    fprintf(stderr, "Internal error: unhandled libcall type in writestart\n");
	    exit(20);
	    break;
	}
	
	funclistit = funclistit->next;
    }

    fprintf(out, "    (void *)-1\n};\n");
}

	
static void writesets(FILE *out, struct config *cfg)
{
    fprintf(out,
	    "DEFINESET(INIT)\n"
	    "DEFINESET(EXIT)\n"
	    "DEFINESET(CTORS)\n"
	    "DEFINESET(DTORS)\n"
    );
    if (cfg->modtype != HANDLER)
	fprintf(out,
	    "DEFINESET(INITLIB)\n"
	    "DEFINESET(EXPUNGELIB)\n"
    	);
    if (!(cfg->options & OPTION_NOOPENCLOSE))
        fprintf(out,
	    "DEFINESET(OPENLIB)\n"
	    "DEFINESET(CLOSELIB)\n"
	);
    if (cfg->modtype == DEVICE)
        fprintf(out,
	    "DEFINESET(OPENDEV)\n"
	    "DEFINESET(CLOSEDEV)\n"
        );
    if (cfg->classlist != NULL)
	fprintf(out,
		"DEFINESET(CLASSESINIT)\n"
		"DEFINESET(CLASSESEXPUNGE)\n"
	);
    fprintf(out, "\n");
}

