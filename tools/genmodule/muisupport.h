#include "config.h"
#include "functionhead.h"

#include <stdio.h>

void writemuiincludes(FILE *out);
void writemccinit(FILE *out, int inclass, struct classinfo *);
void writemccquery(FILE *out, struct config *);
