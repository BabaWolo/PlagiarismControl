#define STR_VARS "str"
#define STR_INT_VARS "str_int_arr"
#define WORDS_SIM_VARS "words_sim"

void FreeTestStructVars(Doc *doc, char *vars)
{
    if (!strcmp(vars, "str"))
    {
        free(doc->text);
        free(doc->og_text);
        free(doc->token_og_text);
    }
    else if (!strcmp(vars, "str_int_arr"))
    {
        free(doc->words);
        free(doc->og_words);
        free(doc->similarities);
    }
    else if (!strcmp(vars, "words_sim"))
    {
        free(doc->words);
        free(doc->similarities);
    }
}

void TestRemoveChars(CuTest *tc)
{
    char actual[] = "» #Hellø? W0-rld !";
    char *expected = " hellø w0rld ";

    remove_characters(actual, Danish);

    CuAssertStrEquals(tc, expected, actual);
}

void TestSplitWords(CuTest *tc)
{ // clang-format off
    char input[] = " Hello   World ";
    char **actual;
    char *expected[] = {"Hello", "World"};
    int len, result;

    split(&actual, input, &len);

    for (int i = 0; i < len && (result = !strcmp(expected[i], actual[i])); i++);
    CuAssertTrue(tc, result);
    free(actual);
} // clang-format on

void TestFindQuotations(CuTest *tc)
{
    Doc doc;
    char *input[] = {"This", "\"is\"", "\"some", "quoted", "text\""};
    int expected_len = 2, expected_arr[] = {2, 4}, inp_len = 5;
    doc.og_words_len = inp_len;
    doc.og_words = (char **)malloc(sizeof(char *) * inp_len);
    for (int i = 0; i < inp_len; i++)
        doc.og_words[i] = input[i];

    find_quotations(&doc);

    CuAssertIntEquals(tc, expected_len, doc.cited_words_len);
    CuAssertTrue(tc, !memcmp(expected_arr, doc.cited_words, sizeof(expected_arr)));
    free(doc.og_words);
}

void TestIsQuoted(CuTest *tc)
{
    Doc doc;
    int cited_words[] = {2, 4, 7, 10, 12}, len = 5;
    int input[] = {1, 2, 3, 4, 5, 8, 9, 11}, inp_len = 8;
    int actual = 0;
    int expected = 5;
    doc.cited_words_len = len;
    for (int i = 0; i < len; i++)
        doc.cited_words[i] = cited_words[i];

    for (int i = 0; i < inp_len; i++)
        actual += is_quoted(doc, input[i]);

    CuAssertIntEquals(tc, expected, actual);
}

void TestReadFile(CuTest *tc)
{
    Doc doc;
    char *expected = "This is TestDoc.txt \nThis is TestDoc.txt \aThis is TestDoc2.txt \nThis is TestDoc2.txt";
    strcpy(doc.filename, "tests/TestDoc.txt tests/TestDoc2.txt");

    read_file(&doc, doc.filename);

    CuAssertStrEquals(tc, expected, doc.text);
    FreeTestStructVars(&doc, STR_VARS);
}

void TestRead200Lines(CuTest *tc)
{
    Doc doc;
    char line[] = "This text consists of exactly 200 lines containing 10 words ? \n";
    int expected_text_len = strlen(line) * 200 - 2;
    strcpy(doc.filename, "tests/Test200Lines.txt");

    read_file(&doc, doc.filename);

    CuAssertIntEquals(tc, expected_text_len, strlen(doc.text));
    FreeTestStructVars(&doc, STR_VARS);
}

void TestReaddSymbols(CuTest *tc)
{
    Doc doc;
    char *og_words[] = {"?", "!This!", "?", "?", "Is", "?", "t«h»e", "?", "WORDS!", "?", "\"Indeed\""};
    char *words[] = {"this", "is", "the", "words", "indeed"};
    int input[] = {0, 2, 3, 4};
    int expected[] = {1, 6, 8, 10};
    doc.og_words_len = 11;
    doc.words_len = 5;
    doc.sim_len = 4;
    doc.og_words = (char **)malloc(sizeof(char *) * doc.og_words_len);
    doc.words = (char **)malloc(sizeof(char *) * doc.words_len);
    doc.similarities = (int *)malloc(sizeof(int) * doc.sim_len);
    for (int i = 0; i < doc.og_words_len; i++)
    {
        doc.og_words[i] = og_words[i];
        if (i < doc.words_len)
        {
            doc.words[i] = words[i];
            if (i < doc.sim_len)
                doc.similarities[i] = input[i];
        }
    }

    readd_symbols(doc);

    CuAssertTrue(tc, !memcmp(expected, doc.similarities, sizeof(expected)));
    FreeTestStructVars(&doc, STR_INT_VARS);
}

void TestCheckDKSynonyms(CuTest *tc)
{
    g_language = 2;
    char *word1 = "men";
    char *word2 = "endda";
    CuAssertTrue(tc, check_similarity(word1, word2));

    word1 = "og";
    word2 = "samt";
    CuAssertTrue(tc, check_similarity(word1, word2));

    word1 = "ikke";
    word2 = "synonymer";
    CuAssertTrue(tc, !check_similarity(word1, word2));
}

void TestCheckENSynonyms(CuTest *tc)
{
    g_language = 1;
    char *word1 = "officially";
    char *word2 = "technically";
    CuAssertTrue(tc, check_similarity(word1, word2));

    word1 = "like";
    word2 = "related";
    CuAssertTrue(tc, check_similarity(word1, word2));

    word1 = "not";
    word2 = "synonyms";
    CuAssertTrue(tc, !check_similarity(word1, word2));
}

void TestCheckDKConjugation(CuTest *tc)
{
    g_language = 2;
    char *word1 = "løve";
    char *word2 = "løver";
    CuAssertTrue(tc, check_similarity(word1, word2));

    // False
    word1 = "men";
    word2 = "mener";
    CuAssertTrue(tc, !check_similarity(word1, word2));
}

void TestCheckENConjugation(CuTest *tc)
{
    g_language = 1;
    char *word1 = "make";
    char *word2 = "makes";
    CuAssertTrue(tc, check_similarity(word1, word2));

    // False
    word1 = "cap";
    word2 = "capes";
    CuAssertTrue(tc, !check_similarity(word1, word2));
}

void TestDeleteDubs(CuTest *tc)
{
    int actual[] = {1, 1, 2, 2, 2, 5, 6, 6}, len = 8;
    int expected[] = {1, 2, 5, 6};

    delete_dublicates(actual, &len);

    CuAssertTrue(tc, !memcmp(expected, actual, sizeof(expected)));
}

void TestQsort(CuTest *tc)
{
    int actual[] = {6, 3, 4, 1, 9, 5, 2, 8}, len = 8;
    int expected[] = {1, 2, 3, 4, 5, 6, 8, 9};

    qsort(actual, len, sizeof(int), comparator);

    CuAssertTrue(tc, !memcmp(expected, actual, sizeof(expected)));
}

void TestCompare(CuTest *tc)
{
    Doc user, src;
    char user_input[] = "this is version two this is versions one";
    char src_input[] = "this is version one and that is version two";
    int user_expected[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int src_expected[] = {0, 1, 2, 3, 8};
    split(&user.words, user_input, &user.words_len);
    split(&src.words, src_input, &src.words_len);
    user.similarities = (int *)malloc(sizeof(int) * user.words_len);
    src.similarities = (int *)malloc(sizeof(int) * src.words_len);

    compare(&user, &src);

    CuAssertTrue(tc, !memcmp(user_expected, user.similarities, sizeof(user_expected)));
    CuAssertTrue(tc, !memcmp(src_expected, src.similarities, sizeof(src_expected)));
    FreeTestStructVars(&user, WORDS_SIM_VARS);
    FreeTestStructVars(&src, WORDS_SIM_VARS);
}

CuSuite *WhiteBoxGetSuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestRemoveChars);
    SUITE_ADD_TEST(suite, TestSplitWords);
    SUITE_ADD_TEST(suite, TestFindQuotations);
    SUITE_ADD_TEST(suite, TestIsQuoted);
    SUITE_ADD_TEST(suite, TestReadFile);
    SUITE_ADD_TEST(suite, TestRead200Lines);
    SUITE_ADD_TEST(suite, TestReaddSymbols);
    SUITE_ADD_TEST(suite, TestCheckDKSynonyms);
    SUITE_ADD_TEST(suite, TestCheckDKConjugation);
    SUITE_ADD_TEST(suite, TestCheckENSynonyms);
    SUITE_ADD_TEST(suite, TestCheckENConjugation);
    SUITE_ADD_TEST(suite, TestDeleteDubs);
    SUITE_ADD_TEST(suite, TestQsort);
    SUITE_ADD_TEST(suite, TestCompare);
    return suite;
}
