BEGIN {
    maxlvo = 0;
    stderr="/dev/stderr";

    file = "libdefs.h";
    libheader = 1;

    while ((getline < file) > 0)
    {
	if ($2 == "BASENAME")
	{
	    lib = $3;
	    basename = $3;
	}
	else if ($2 == "LIBBASE")
	{
	    libbase = $3;
	}
	else if ($2 == "NOLIBHEADER")
	{
	    libheader = 0;
	}
    }

    print "/*";
    print "    Copyright (C) 1995-2000 AROS - The Amiga Research OS";
    print "    *** Automatically generated by genfunctable.awk. Do not edit ***";
    print "    Desc: Function table for " lib;
    print "    Lang: english";
    print "*/";
    print "#ifdef __MORPHOS__\n#include <exec/libraries.h>\n#include <aros/libcall.h>"
    print "#else\n#ifndef LIBCORE_COMPILER_H\n#   include <libcore/compiler.h>\n#endif\n#endif";
    print "#ifndef NULL\n#define NULL ((void *)0)\n#endif\n";
    print "#include \"libdefs.h\"";

    f[1] = "open";
    f[2] = "close";
    f[3] = "expunge";
    f[4] = "null";

    verbose_pattern = libbase"[ \\t]*,[ \\t]*[0-9]+[ \\t]*,[ \\t]*"basename;

#print verbose_pattern > "/dev/stderr";

    if (maxlvo < 4)
	maxlvo = 4;
}
/AROS_LH(QUAD)?[0-9]/ {
    line=$0;
    sub(/[ \t]*$/,"",line);
    if (match(line,/[a-zA-Z0-9_]+,$/))
    {
	name=substr(line,RSTART,RLENGTH-1);
#print "/* FOUND " name " */";
    }
}
/LIBBASE[ \t]*,[ \t]*[0-9]+/ || $0 ~ verbose_pattern {
#print "/* LOC " $0 " */"
    match ($0, /,[ \t]*[0-9]+/);
    line=substr($0,RSTART,RLENGTH);
#print "line2="line
    sub (/,[ \t]*/,"",line);
#print "line3="line
    lvo=int(line);

#print "lvo="lvo;

    if (f[lvo] != "")
	print "Error: lvo "lvo" is used by "f[lvo]" and "name" in "FN >> stderr;

    f[lvo] = name;
    if (lvo > maxlvo)
	maxlvo = lvo;
}
/^\/\*AROS/ {
    if ($2 == "alias") {
	a[lvo] = $3;
	f[lvo] = $4;
    }
}
END {
    # Are we using the c_lib stuff to create our lib?
    if (libheader == 1)
    {
	print "extern void AROS_SLIB_ENTRY(LC_BUILDNAME(OpenLib),LibHeader) (void);";
	print "extern void AROS_SLIB_ENTRY(LC_BUILDNAME(CloseLib),LibHeader) (void);";
	print "extern void AROS_SLIB_ENTRY(LC_BUILDNAME(ExpungeLib),LibHeader) (void);";
	print "extern void AROS_SLIB_ENTRY(LC_BUILDNAME(ExtFuncLib),LibHeader) (void);";
    }
    else
    {
	print "extern void AROS_SLIB_ENTRY(open, BASENAME) (void);";
	print "extern void AROS_SLIB_ENTRY(close, BASENAME) (void);";
	print "extern void AROS_SLIB_ENTRY(expunge, BASENAME) (void);";
	print "extern void AROS_SLIB_ENTRY(null, BASENAME) (void);";
    }

    for (t=5; t<=maxlvo; t++)
    {
	if (t in f && !(t in a))
	    print "extern void AROS_SLIB_ENTRY(" f[t] ",BASENAME) (void);";
    }

    print "\nvoid *const LIBFUNCTABLE[]=\n{";

    show=0;

    if ( libheader == 1 )
    {
	print "#ifdef __MORPHOS__\n    (void *const) FUNCARRAY_32BIT_NATIVE,\n#endif";
    print "    AROS_SLIB_ENTRY(LC_BUILDNAME(OpenLib),LibHeader),";
	print "    AROS_SLIB_ENTRY(LC_BUILDNAME(CloseLib),LibHeader),";
	print "    AROS_SLIB_ENTRY(LC_BUILDNAME(ExpungeLib),LibHeader),";
	print "    AROS_SLIB_ENTRY(LC_BUILDNAME(ExtFuncLib),LibHeader),";
    }
    else
    {
	print "#ifdef __MORPHOS__\n    (void *const) FUNCARRAY_32BIT_NATIVE,\n#endif";
    print "    AROS_SLIB_ENTRY(open, BASENAME),";
	print "    AROS_SLIB_ENTRY(close, BASENAME),";
	print "    AROS_SLIB_ENTRY(expunge, BASENAME),";
	print "    AROS_SLIB_ENTRY(null, BASENAME),";
    }

    if (maxlvo <= 4)
    {
	print "Error: No matching functions found" > "/dev/stderr";
	exit (10);
    }

    for (t=5; t<=maxlvo; t++)
    {
	line="    ";

	if (t in f)
	{
	    line=line "AROS_SLIB_ENTRY(" f[t] ",BASENAME),";

	    if (t in a)
		line=line " /* " a[t] " " t " */";
	    else
		line=line " /* " t " */";
	}
	else
	    line=line "NULL, /* " t " */";

	print line;

	show=0;
    }

    print "    (void *)-1L";
    print "};";
}

