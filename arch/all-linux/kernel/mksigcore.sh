#!/bin/sh

if [ ! -f /usr/include/bits/sigcontext.h ] ; then
    echo "Could not find bits/sigcontext.h"
    exit 20
fi

type=`${CC} -D_SIGNAL_H -E /usr/include/bits/sigcontext.h | grep "^struct sigcontext" | sed 's/{//'`

handler=__sighandler_t
if [ ${CPU} = "m68k" ]; then
  sed "s/@sigcontext@/$type/" ${1-.}/sigcore.h.m68k.src > ${2}
else
  sed "s/@sigcontext@/$type/;s/@sighandler@/$handler/" ${1-.}/sigcore.h.${CPU}.src > ${2}
fi
