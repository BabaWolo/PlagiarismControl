#include <stdio.h>
#include <string.h>

typedef struct Document
{
    char text[100];
    char *word_arr[100];
    int arr_length;
} Doc;

void split_words(Doc *doc);

int main()
{
    Doc user_doc;
    Doc source_doc;
    strcpy(user_doc.text, "This is some text that we'll check for plagiarism");
    strcpy(source_doc.text, "This is some source text that we'll compare with the user text");
    split_words(&user_doc);
    split_words(&source_doc);
    return 0;
}

void split_words(Doc *doc)
{
    int *length = &doc->arr_length;
    *length = 0;

    char *word = strtok(doc->text, " ");
    while (word != NULL)
    {
        doc->word_arr[*length] = word;
        *length += 1;
        word = strtok(NULL, " ");
    }
}