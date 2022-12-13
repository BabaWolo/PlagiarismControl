#include <stdio.h>
#include "CuTest.c"
#include "../program.h"
#include "WhiteBox.c"

CuSuite *WhiteBoxGetSuite();

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, WhiteBoxGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}