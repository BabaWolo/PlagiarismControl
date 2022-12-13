#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SYNONYM_COLS 5

static int language = 1;
static const char *en_synonyms[][SYNONYM_COLS] = {{"this", "that"}, {"like", "alike", "comparable", "related"}, {"technically", "officially"}};
static const char *dk_synonyms[][SYNONYM_COLS] = {{"men", "alligevel", "derfor", "dog", "endda"}, {"og", "plus", "samt"}};

typedef struct Document
{
    char filename[200], *filenames[10], *text, *og_text, **words, **og_words;
    int words_len, og_words_len, cited_words[200], *similarities, cited_words_len, sim_len, percent;
} Doc;

void flush_stdin();
void check_plagiarism();
void split_words(Doc *doc);
void print_result(Doc doc);
void readd_symbols(Doc doc);
void find_quotations(Doc *doc);
void free_struct_vars(Doc *doc);
int is_quoted(Doc doc, int index);
void read_file(Doc *doc, char filename[]);
void compare(Doc *user_doc, Doc *source_doc);
void finalize_doc(Doc user_doc, Doc src_doc);
int check_similarity(char *word1, char *word2);
void delete_dublicates(int arr[], int *length);
void remove_characters(char *str, char *letters);
void split(char **arr[], char *text, int *length);
int comparator(const void *num_1, const void *num_2);
void get_file_configurations(char *user_file, char *source_files);
int check_conjugation(char *word1, char *word2, char *plural_suffix);
int check_synonyms(char *word1, char *word2, const char *synonyms[][SYNONYM_COLS], int rows);

// CuTest
void RunAllTests(void);