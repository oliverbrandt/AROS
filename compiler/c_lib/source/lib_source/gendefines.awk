BEGIN {
    stderr="/dev/stderr";

    if (file == "")
    {
        file = "libdefs.h";
    }
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
    }

    verbose_pattern = libbase"[ \\t]*,[ \\t]*[0-9]+[ \\t]*,[ \\t]*"basename;

    close (file);

    BASENAME=toupper(basename);

    print "#ifndef DEFINES_"BASENAME"_PROTOS_H"
    print "#define DEFINES_"BASENAME"_PROTOS_H"
    print ""
    print "/*"
    print "    Copyright � 1995-2003, The AROS Development Team. All rights reserved."
    print "    *** Automatically generated by gendefines.awk. Do not edit ***"
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
    print "#ifndef AROS_PREPROCESSOR_VARIADIC_CAST2IPTR_HPP"
    print "#   include <aros/preprocessor/variadic/cast2iptr.hpp>"
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

}
#This function emits forward declarations for structures. To be used
#when emitting inlines rather than defines
function emit_struct(tname)
{
    if (match(tname,/struct.+[^ \t*]/))
    {
       struct_name = substr(tname,RSTART,RLENGTH);
       match(struct_name, /[^ \t]+$/)
       struct_name = substr(struct_name, RSTART,RLENGTH);

       if (!(struct_name in structures))
       {
           printf "struct %s;\n", struct_name
           structures[struct_name] = 1
       }
    }
}

#public function
/AROS_LH[0-9]/ { isprivate = 0; ntagarg = 0;}

#public function, do not attempt to emit inline tagcalls
/AROS_NTLH[0-9]/ { isprivate = 0; ntagarg = -1; }

#private function
/AROS_PLH[0-9]/ { isprivate = 1; ntagarg = 0; }

/AROS_(NT)?LH(A|(QUAD)?[0-9])/ {
    line=$0;
    isarg=match($0,/AROS_LHA/);

    gsub(/^[ \t]+/,"",line);

    if (!isarg)
    {
	args="";
	narg=0;

	match(line,/\(.+,.+[^,]/);
	line=substr(line,RSTART+1,RLENGTH-1);
	gsub(/[ \t]*,[ \t]*/,",",line);
	split(line, a, ",");

        tname = a[1]
        fname = a[2]

        #emit_struct(tname)
    }
    else
    {
	arg[narg] = line;

        if (match(line, /\/\*[ \t]*tagcall.+\*\//))
        {
	    tagcall=substr(line,RSTART+2,RLENGTH-4)

            valid_tagcall = 0

            to_add = ""
            if(match(tagcall, /\+[A-Za-z0-9]+/))
            {
	        to_add = substr(tagcall,RSTART+1,RLENGTH-1)

                valid_tagcall = 1
            }

            to_sub = 0
            if(match(tagcall, /\-[0-9]+/))
            {
	        to_sub = substr(tagcall,RSTART+1,RLENGTH-1)

                valid_tagcall = 1
            }

            if (!valid_tagcall)
            {
                print "Invalid tagcall statement for function \"" fname "\", argument n. " narg > "/dev/stderr"
                exit 1
            }

            tagarg[ntagarg, 1] = narg
            tagarg[ntagarg, 2] = to_sub
            tagarg[ntagarg, 3] = to_add

            ntagarg++
        }

        match(line,/\(.+\)/);
	line=substr(line,RSTART+1,RLENGTH-2);
	gsub(/[ \t]*,[ \t]*/,",",line);

        split(line, arg_args, ",");

 	arg[narg, 1] = arg_args[1]
 	arg[narg, 2] = arg_args[2]
 	arg[narg, 3] = arg_args[3]

        #emit_struct(arg_args[1])

        narg++
    }
}
/LIBBASE[ \t]*,[ \t]*[0-9]+/ || $0 ~ verbose_pattern {
    line=$0;

    match(line,/[a-zA-Z_0-9]+.*[^)]/);
    line = substr(line	,RSTART,RLENGTH);
    gsub(/[ \t]*,[ \t]*/,",",line);
    split(line,a,",");

    libbtp   = a[1]
    libbase  = a[2]
    lvo      = a[3]
    basename = a[4]

    #emit_struct(libbtp)

       #this commented out code is used for emitting inlines rather than macros
       #unused as for now. Put it after the below if()
       #-----------------------------------------------------------------------
       #header     = tname  " __" fname "_WB"
       #header_len = length(header)

       #printf "static __inline__\n%s(%s %s", header, libbtp, libbase

       #for (t=0; t<narg; t++)
       #{
       #     arg_to_print = arg[t, 1] " " arg[t, 2]
       #     printf ",\n%+*s", header_len + length (arg_to_print) + 1, arg_to_print
       #}
       #print ")"

       #print "{"

    if (lvo > firstlvo && !isprivate)
    {
        printf "#define __%s_WB(__%s", fname, libbase
        for (t=0; t<narg; t++)
	{
            printf ", __%s", arg[t, 2]
	}
        print ")\\"

        printf "    AROS_LC%d(%s, %s, \\\n", narg, tname, fname
        for (t=0; t<narg; t++)
	{
            printf "    AROS_LCA(%s, (__%s), %s), \\\n", arg[t, 1], arg[t, 2], arg[t, 3]
	}
        printf "    %s, (__%s), %d, %s)\n\n", libbtp, libbase, lvo, basename

        printf "#define %s(", fname
        for (t=0; t<narg; t++)
	{
            printf "%s", arg[t, 2]
            if (t != narg-1) printf ", "
	}
        print ")\\"

        printf "    __%s_WB(%s", fname, libbase
        for (t=0; t<narg; t++)
	{
            printf ", (%s)", arg[t, 2]
	}
        print ")\n"

        if (ntagarg == 0)
        {
            #emit variadic macros for tag-based functions

            if (fname ~ /A$/)
            {
                tagarg[0, 1] = narg-1
                tagarg[0, 2] = 1
                tagarg[0, 3] = ""
                ntagarg++
            }
            else
            if (fname ~ /TagList$/)
            {
                tagarg[0, 1] = narg-1
                tagarg[0, 2] = 7
                tagarg[0, 3] = "Tags"
                ntagarg++
            }
            else
            if (fname ~ /Args$/ && (tolower(arg[narg-1, 2]) == "args") || (tolower(arg[narg-1, 2]) == "arglist"))
            {
                tagarg[0, 1] = narg-1
                tagarg[0, 2] = 4
                tagarg[0, 3] = ""
                ntagarg++
            }
            else
            if (arg[narg-1, 1] ~ /struct[ \t]+TagItem[ \t]*[*]/)
            {
                tagarg[0, 1] = narg-1
                tagarg[0, 2] = 0
                tagarg[0, 3] = "Tags"
                ntagarg++
            }
        }

        if (ntagarg > 0)
        {
            print "#if !defined(NO_INLINE_STDARG) && !defined(" BASENAME "_NO_INLINE_STDARG)"
        }
        for (i = 0; i < ntagarg; i++)
        {
            printf "#define %s(", substr(fname, 0, length(fname) - tagarg[i, 2]) tagarg[i, 3];
            for (t=0; t<narg; t++)
	    {
                if (tagarg[i, 1] == t) continue

                printf "%s, ", arg[t, 2]
	    }
            print "...) \\"

            print "({ \\"
            print "     IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \\"

            printf "     %s(", fname
            for (t=0; t<narg; t++)
	    {
                if (tagarg[i, 1] == t)
                    printf "(%s)__args", arg[tagarg[i, 1], 1]
                else
                    printf "(%s)", arg[t, 2]

                if (t != narg-1)
                    printf ", "
	    }
            print "); \\\n})"
        }
        if (ntagarg > 0)
        {
            print "#endif /* !NO_INLINE_STDARG */\n"
        }
   }
    narg=0;
}
END {
    print "#endif /* DEFINES_"BASENAME"_PROTOS_H */"
}
