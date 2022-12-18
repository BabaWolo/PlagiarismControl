
void TestENPlagiarism(CuTest *tc)
{
    Doc user, src;
    char user_filename[] = "tests/TestEnUser.txt";
    char src_filenames[] = "tests/TestEnSrc.txt";
    int user_expected[] = {0, 1, 2, 3, 5, 6, 8, 9};
    int src_expected[] = {0, 1, 2, 3, 8};

    check_plagiarism(&user, &src, user_filename, src_filenames, English);

    CuAssertTrue(tc, !memcmp(user_expected, user.similarities, sizeof(user_expected)));
    CuAssertTrue(tc, !memcmp(src_expected, src.similarities, sizeof(src_expected)));
    free_struct_vars(&user);
    free_struct_vars(&src);
}

void TestDKPlagiarism(CuTest *tc)
{
    Doc user, src;
    char user_filename[] = "tests/TestDkuser.txt";
    char src_filenames[] = "tests/TestDkSrc.txt tests/TestDkSrc.txt";
    int user_expected[] = {2, 3, 4, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20};
    int src_expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 18};

    check_plagiarism(&user, &src, user_filename, src_filenames, Danish);

    CuAssertTrue(tc, !memcmp(user_expected, user.similarities, sizeof(user_expected)));
    CuAssertTrue(tc, !memcmp(src_expected, src.similarities, sizeof(src_expected)));
    free_struct_vars(&user);
    free_struct_vars(&src);
}

void TestTimeComplexity(CuTest *tc)
{
    Doc user, src;
    clock_t start_t, end_t;
    double seconds;
    char user_filename[] = "tests/Test200Lines.txt";
    char src_filenames[] = "tests/Test200Lines2.txt";

    start_t = clock();
    check_plagiarism(&user, &src, user_filename, src_filenames, English);
    end_t = clock();
    seconds = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("seconds: %f\n", seconds);
    CuAssertTrue(tc, seconds < .5);
    free_struct_vars(&user);
    free_struct_vars(&src);
}

CuSuite *BlackBoxGetSuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestENPlagiarism);
    SUITE_ADD_TEST(suite, TestDKPlagiarism);
    SUITE_ADD_TEST(suite, TestTimeComplexity);
    return suite;
}