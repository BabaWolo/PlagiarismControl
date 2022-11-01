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

void remove_character(Doc *doc);
void split_words(Doc *doc);
void check_quotations(char *word[], int *quatation_check, int *quatation_length);
void compare(Doc *user_doc, Doc source_doc);
void check_plagiarism();
int read_file(char text[], char *filename);

int main()
{
    check_plagiarism();
    return 0;
}

void remove_character(Doc *doc)
{
    int i, j;
    
    // Checks if the a character in the document text is not in the alphabet or "" or . or null
    // If true, the character equals NULL and everything else is pushed left
    for (i = 0, j; doc->text[i] != '\0'; i++){
        while (!(doc->text[i] >= 'a' && doc->text[i] <= 'z') && !(doc->text[i] >= 'A' && doc->text[i] <= 'Z') && 
               !(doc->text[i] == '"') && !(doc->text[i] == '.') && !(doc->text[i] != '\0'))
        {
            for (j=i; doc->text[j] != '\0'; j++)
            {
                doc->text[j] = doc->text[j+1];
            }
            doc->text[j] = '\0';
        }
        
    }
}

void check_plagiarism()
{
    
    Doc user_doc;
    Doc source_doc;
    read_file(user_doc.text, "user_doc.txt");
    read_file(source_doc.text, "source_doc.txt");
    remove_character(&user_doc);
    remove_character(&source_doc);
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
    int i, j, similar, strcmp_val = 0, count = 0;
    int user_len = user_doc->words_length, source_len = source_doc.words_length;

    // Goes through each word in the user text and tries to detect it in the source text
    for (i = 0; i < user_len; i++)
    {
        for (j = 0; j < source_len; j++)
        {
            if (!strcmp(user_doc->words[i], source_doc.words[j]))
            {
                similar = 1;

                // Checks similarities of the following words
                while (i + similar < user_len &&
                       j + similar < source_len &&
                       !strcmp(user_doc->words[i + similar], source_doc.words[j + similar]))
                    similar++;

                // Check if the word(s) is an extension to other similarities
                if (similar <= 2 && count)
                {
                    strcmp_val = !strcmp(user_doc->words[user_doc->similarities[count]], source_doc.words[j - 1]);
                    if (strcmp_val && similar == 1)
                        strcmp_val = !strcmp(user_doc->words[user_doc->similarities[count - 1]], source_doc.words[j - 2]);
                }

                // If theres 3 or more identical words in a row then store the indexes of the words
                if (similar >= 3 || strcmp_val)
                {
                    count += count ? similar : similar - 1;
                    i += similar - 1;

                    while (--similar >= 0)
                        user_doc->similarities[count - similar] = i - similar;
                    break;
                }
            }
        }
    }
    // Can't devide an integer with a higher integer without making it into a decimal
    float count_len = count ? count + 1 : count,
          percent = count_len / user_len * 100;
    char *color = percent > 15
                      ? "\x1b[31m" // <- Red
                  : percent > 5
                      ? "\033[0;33m"  // <- Yellow
                      : "\033[0;32m"; // <- Green
    printf("\n-------------------------"
           "%s"
           "\nPLAGIARIZED TEXT (%.1f%%): \n"
           "\x1b[0m"
           "-------------------------\n",
           color,
           percent);
    if (count)
        for (i = 0; i <= count; i++)
            printf("%s ", user_doc->words[user_doc->similarities[i]]);
    printf("\n\n\n");

    char show_doc;
    printf("Show the plagiarised text in your document, (y)es or (n)o: ");
    scanf(" %c", &show_doc);
    if (show_doc == 'y')
    {
        printf("\n---------\n"
               "%s"
               "DOCUMENT:"
               "\x1b[0m"
               "\n---------\n",
               color);
        j = 0;
        for (i = 0; i < user_len; i++)
        {
            if (i == user_doc->similarities[j])
            {
                printf("%s", color);
                j++;
            }

            printf("%s "
                   "\x1b[0m",
                   user_doc->words[i]);
        }
    }
    printf("\n\n\n");
}

int read_file(char text[], char *filename)
{

    FILE *doc;

    // Opening file, set to read mode and checking if possible to open file
    doc = fopen(filename, "r");
    if (doc == NULL)
    {
        printf("Could not open file!");
        return -1;
    }

    // Read the file content until either end of file or reached the maximum characters
    while (!feof(doc))
        fgets(text, 100, doc);

    fclose(doc);

    return 0;
}