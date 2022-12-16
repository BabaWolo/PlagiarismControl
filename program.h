#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SYNONYM_COLS 5

enum Languages
{
    English = 1,
    Danish
};

static int g_language = 1;
static int g_lang_len = 2;
static const char *g_en_synonyms[][SYNONYM_COLS] = {{"this", "that"}, {"like", "alike", "comparable", "related"}, {"technically", "officially"}};
static const char *g_dk_synonyms[][SYNONYM_COLS] = {{"men", "alligevel", "derfor", "dog", "endda"}, {"man", "de", "du", "en", "folk"}, {"og", "plus", "samt"}};

typedef struct Document
{
    char filename[200], *filenames[10], *text, *og_text, *token_text, *token_og_text, **words, **og_words;
    int words_len, og_words_len, cited_words[200], *similarities, cited_words_len, sim_len;
    double percent;
} Doc;

void flush_stdin();
void split_words(Doc *doc);
void readd_symbols(Doc doc);
void find_quotations(Doc *doc);
void free_struct_vars(Doc *doc);
int is_quoted(Doc doc, int index);
void check_plagiarism_via_terminal();
void read_file(Doc *doc, char filename[]);
void print_result(Doc doc, int is_src_doc);
void compare(Doc *user_doc, Doc *source_doc);
void finalize_doc(Doc user_doc, Doc src_doc);
int check_similarity(char *word1, char *word2);
void delete_dublicates(int arr[], int *length);
void remove_characters(char *str, int language);
void split(char **arr[], char *text, int *length);
int comparator(const void *num_1, const void *num_2);
void get_file_configurations(char *user_file, char *source_files);
int check_conjugation(char *word1, char *word2, char *plural_suffix);
int check_synonyms(char *word1, char *word2, const char *synonyms[][SYNONYM_COLS], int rows);
void check_plagiarism(Doc *user, Doc *src, char user_filename[], char src_filename[], int language);

// CuTest
void RunAllTests(void);