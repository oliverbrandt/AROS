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
        else if ($2 == "NT_TYPE")
        {
            if( $3 == "NT_RESOURCE" )
            {
                firstlvo = 0;
                libext = ".resource";
            }
            else if( $3 == "NT_DEVICE" )
            {
                firstlvo = 6;
                libext = ".device";
            }
            else
            {
                firstlvo = 4;
                libext = ".library";
            }
        }
    }

    verbose_pattern = libbase"[ \\t]*,[ \\t]*[0-9]+[ \\t]*,[ \\t]*"basename;

    close (file);

    BASENAME=toupper(basename);

    print "#ifndef CLIB_"BASENAME"_PROTOS_H"
    print "#define CLIB_"BASENAME"_PROTOS_H"
    print ""
    print "/*"
    print "    Copyright (C) 1995-2000 AROS - The Amiga Research OS"
    print "    *** Automatically generated by genclib.awk. Do not edit ***"
    print ""
    print "    Desc: Prototypes for "basename libext
    print "    Lang: english"
    print "*/"
    print ""
    print "#ifndef AROS_LIBCALL_H"
    print "#   include <aros/libcall.h>"
    print "#endif"
    print ""

    file = "headers.tmpl"
    doprint     = 0;
    emit        = 0;
    doParse     = 0;
    nifct       = 0;
    doInsertFct = 0;

    while ((getline < file) > 0)
    {
        if ($1=="##begin" && $2 == "clib")
            doprint = 1;
        else if ($1=="##end" && $2 == "clib")
            doprint = 0;
        else if ($1=="##begin" && $2 == "insert_fct")
            doParse = 1;
        else if ($1=="##end" && $2 == "insert_fct")
            doParse = 0
        else if (doprint)
        {
            print;
            emit ++;
        }
        else if(doParse)
        {
            {
                if($1 == "PRE") {sub(/^PRE / , ""); insert_pre  = $0;}
                if($1 == "POST"){sub(/^POST /, ""); insert_post = $0;}
                if($1 == "FCT") {insert_fct[nifct++] = $2;}
            }
         }
    }

    if (emit > 0)
        print ""

    print "/* Prototypes */"
}
/AROS_LH(A|(QUAD)?[0-9])/ {
    line=$0;
    isarg=match(line,/AROS_LHA/);
    gsub(/AROS_LH/,"AROS_LP",line);
    gsub(/^[ \t]+/,"",line);
    if (!isarg)
    {
        call=line;
        narg=0;

        doInsertFct = 0;
        for (i = 0; (i < nifct && doInterFct == 0); i++)
        {
          if(call ~ insert_fct[i]) doInsertFct = 1;
        }

    }
    else
    {
        arg[narg++]=line;
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
        if(doInsertFct) print insert_pre
        print call

        for (t=0; t<narg; t++)
            print "\t"arg[t]
        print "\t"line")";

        if(doInsertFct) print insert_post
        print ""
    }
    narg=0;
}
END {
    print "#endif /* CLIB_"BASENAME"_PROTOS_H */"
}
