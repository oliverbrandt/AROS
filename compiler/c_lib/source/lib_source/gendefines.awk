BEGIN {
    stderr="/dev/stderr";

    file = "libdefs.h";

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
	else if ($2 == "LIBBASETYPEPTR")
	{
	    libbtp = $3;
	    for (t=4; t<=NF; t++)
		libbtp=libbtp" "$t;
	}
	else if($2 == "NT_TYPE")
	{
	    if( $3 == "NT_RESOURCE" )
	    {
		firstlvo = 0;
		libext = ".resource";
	    }
	    else if ( $3 == "NT_DEVICE" )
	    {
		firstlvo = 6;
		libext = ".device";
	    }
	    else
	    {
		firstlvo = 4;
		libexit = ".library";
	    }
	}
	else if ($2 == "INCLUDE_PREFIX" )
	{
	    incname = $3;
	}
    }

    verbose_pattern = libbase"[ \\t]*,[ \\t]*[0-9]+[ \\t]*,[ \\t]*"basename;

    close (file);

    BASENAME=toupper(basename);
    INCNAME=toupper(incname);

    print "#ifndef DEFINES_"INCNAME"_PROTOS_H"
    print "#define DEFINES_"INCNAME"_PROTOS_H"
    print ""
    print "/*"
    print "    Copyright (C) 1995-1998 AROS - The Amiga Research OS"
    print "    $""Id$"
    print ""
    print "    Desc: Prototypes for "basename libext
    print "    Lang: english"
    print "*/"
    print ""
    print "#ifndef AROS_LIBCALL_H"
    print "#   include <aros/libcall.h>"
    print "#endif"
    print "#ifndef EXEC_TYPES_H"
    print "#   include <exec/types.h>"
    print "#endif"
    print ""

    file = "headers.tmpl"
    doprint = 0;
    emit = 0;

    while ((getline < file) > 0)
    {
	if ($1=="##begin" && $2 == "defines")
	    doprint = 1;
	else if ($1=="##end" && $2 == "defines")
	    doprint = 0;
	else if (doprint)
	{
	    print;
	    emit ++;
	}
    }

    if (emit > 0)
	print ""

    print "/* Defines */"
}
/AROS_LH(A|(QUAD)?[0-9])/ {
    line=$0;
    isarg=match($0,/AROS_LHA/);

    gsub(/AROS_LH/,"AROS_LC",line);
    gsub(/^[ \t]+/,"",line);

    if (!isarg)
    {
	args="";
	narg=0;
	match(line,/[a-zA-Z_0-9]+[ \t]*,[ \t]*$/);
	fname=substr(line,RSTART,RLENGTH);
	gsub(/[ \t]*,[ \t]*$/,"",fname);
	call=line;
    }
    else
    {
	arg[narg++] = line;
	match($0,/,[^)]+/);
	line=substr($0,RSTART+1,RLENGTH-1);
	gsub(/[ \t]+/,"",line);
	match(line,/[^,]+/);
	if (args!="")
	    args=args", "substr(line,RSTART,RLENGTH);
	else
	    args=substr(line,RSTART,RLENGTH);
    }
}
/LIBBASE[ \t]*,[ \t]*[0-9]+/ || $0 ~ verbose_pattern {
    line=$0;
    gsub(/LIBBASETYPEPTR/,libbtp,line);
    gsub(/LIBBASE/,libbase,line);
    gsub(/BASENAME/,basename,line);
    gsub(/[ \t]*[)][ \t]*$/,"",line);
    gsub(/^[ \t]+/,"",line);
    na=split(line,a,",");
    lvo=int(a[3]);

    if (lvo > firstlvo)
    {
	print "#define "fname"("args") \\"
	print "\t"call" \\";
	for (t=0; t<narg; t++)
	    print "\t"arg[t]" \\"
	print "\t"line")"
	print ""
    }
    narg=0;
}
END {
    print "#endif /* DEFINES_"INCNAME"_PROTOS_H */"
}
