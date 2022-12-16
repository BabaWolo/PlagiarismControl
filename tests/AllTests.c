#include <stdio.h>
#include <time.h>
#include "CuTest.c"
#include "../program.h"
#include "WhiteBox.c"
#include "BlackBox.c"

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, WhiteBoxGetSuite());
    CuSuiteAddSuite(suite, BlackBoxGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}