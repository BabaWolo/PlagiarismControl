#include <stdio.h>
#include <string.h>

typedef struct Document
{
    char text[100];
    char *word_arr[100];
    int similarities[100];
    int arr_length;
} Doc;

void split_words(Doc *doc);
void compare(Doc *user_doc, Doc source_doc);

int main()
{
    Doc user_doc;
    Doc source_doc;
    strcpy(user_doc.text, "This is some text that we'll check for plagiarism");
    strcpy(source_doc.text, "This is some source text that we'll compare with the user text");
    split_words(&user_doc);
    split_words(&source_doc);
    compare(&user_doc, source_doc);
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

void compare(Doc *user_doc, Doc source_doc)
{
    int i, j, similar, count = 0;

    for (i = 0; i < user_doc->arr_length; i++)
    {
        for (j = 0; j < source_doc.arr_length; j++)
        {
            if (!strcmp(user_doc->word_arr[i], source_doc.word_arr[j]))
            {
                similar = 1;
                while (!strcmp(user_doc->word_arr[1 + i++], source_doc.word_arr[1 + j++]))
                    similar++;

                if (similar >= 3)
                {
                    count += similar;
                    do
                        user_doc->similarities[count - similar] = i - similar;
                    while (similar-- > 0);
                }
            }
        }
    }
    printf("\x1b[31m"
           "\nPLAGIARIZED TEXT: \n"
           "\x1b[0m");
    for (i = 0; i < count; i++)
        printf("%s ", user_doc->word_arr[user_doc->similarities[i]]);
    printf("\n\n");
}