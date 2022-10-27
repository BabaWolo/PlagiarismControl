#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct Document
{
    char text[100];
    char *words[100];
    int words_length;
    int similarities[100];
} Doc;

void split_words(Doc *doc);
void check_quotations(char *word[], int *quatation_check, int *quatation_length);
void compare(Doc *user_doc, Doc source_doc);
void check_plagiarism();
int read_file(char text[], char *filename);

int main()
{
    Doc user_doc;
    Doc source_doc;
    strcpy(user_doc.text, "d This \"is some text\" that we'll \"check for plagiarism\". this is also some text");
    strcpy(source_doc.text, "This is some source  text that we'll compare with the user text");
    split_words(&user_doc);
    split_words(&source_doc);
    return 0;
}

void check_plagiarism()
{
    Doc user_doc;
    Doc source_doc;
    read_file(user_doc.text, "user_doc.txt");
    read_file(source_doc.text, "source_doc.txt");
    //strcpy(user_doc.text, "This is some text that we'll check for plagiarism");
    //strcpy(source_doc.text, "This is some source text that we'll compare with the user text");
    split_words(&user_doc);
    split_words(&source_doc);
    compare(&user_doc, source_doc);
}

// Splits the given sentence into an array of words whenever it encounters a whitespace
// The words and length gets stored in the struct output paramater
void split_words(Doc *doc)
{
    int *length = &doc->words_length;
    *length = 0;
    int quatation_check = 0, quatation_length = 0;

    char *word = strtok(doc->text, " ");
    while (word != NULL)
    {   
        *word = tolower(*word);
        doc->words[*length] = word;
        *length += 1;
        word = strtok(NULL, " "); // <- Next word

        check_quotations(&word, &quatation_check, &quatation_length);
    }
}

void check_quotations(char *word[], int *quatation_check, int *quatation_length){
    if(strchr(*word, '"') != NULL)
    {
        *quatation_check += 1;
    }
    if (*quatation_check % 2 == 0)
    {   
        if (strchr(*word, '"') != NULL)
        {
            printf("\033[1;33m");
            printf("%s\n", *word);
            *quatation_length++;
        } 
        else 
        {
        printf("\033[0;37m");
        printf("%s\n", *word);
        }
    } 
    else if (*quatation_check % 2 != 0) 
    {
    printf("\033[1;33m");
    printf("%s\n", *word);
    *quatation_length++;
    }
}

// Checks if two documents contains the exact same order of 3 or more words
void compare(Doc *user_doc, Doc source_doc)
{
    int i, j, similar, count = 0;

    // Goes through each word in the user text and tries to detect it in the source text
    for (i = 0; i < user_doc->words_length; i++)
    {
        for (j = 0; j < source_doc.words_length; j++)
        {
            if (!strcmp(user_doc->words[i], source_doc.words[j]))
            {
                similar = 1;

                // Checks similarities of the following words
                while (!strcmp(user_doc->words[1 + i++], source_doc.words[1 + j++]))
                    similar++;

                // If theres 3 or more identical words in a row then store the indexes of the words
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
        printf("%s ", user_doc->words[user_doc->similarities[i]]);
    printf("\n\n");
}

int read_file(char text[], char *filename) {
    
    FILE * doc; 

    // Opening file, set to read mode and checking if possible to open file
    doc = fopen(filename, "r");
    if (doc == NULL) { printf("Could not open file!"); return -1; }

    // Read the file content until either end of file or reached the maximum characters
    while(!feof(doc)) {
        fgets(text, 100, doc);
    }

    fclose(doc);

    return 0;
}