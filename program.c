#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct Document
{
    char text[100];
    char text_og[100]; // og = original
    char *words[100];
    char *words_og[100];
    int words_length;
    int words_og_length;
    int similarities[100];
} Doc;

void remove_character(char *str);
void split_words(Doc *doc);
void split(char *arr[], char *text, int *length);
void check_quotations(char *word[], int *quatation_check, int *quatation_length);
void compare(Doc *user_doc, Doc source_doc);
void check_plagiarism();
int read_file(char text[], char *filename);
void print_result(char *words[], int similar[], float percent, int sim_length, int user_len);

int main()
{
    check_plagiarism();
    return 0;
}

void check_plagiarism()
{
    Doc user_doc;
    Doc source_doc;
    read_file(user_doc.text, "user_doc.txt");
    read_file(source_doc.text, "source_doc.txt");
    split_words(&user_doc);
    split_words(&source_doc);
    compare(&user_doc, source_doc);
}

void remove_character(char *str)
{
    // Increment all "non-symbols"
    // strchr: Returns a pointer to the first occurrence of the character "str[dst]" in the string symbols, or NULL if the character is not found
    // Vi starter med src og dst = 0 hvis dst = symbol -> dst = 0, hvis dst = !symbol -> dst = 1.
    // I et array eksempel [!, H, e, j], fordi src "0" er et symbol bliver dst += NULL, str[dst] "0" bliver sat på str[src] "0" altså [0, H, e, j] og src++
    // vi kører for-løkken og fordi (str[dst] = str[src]), hvilket er (0 = 1), så bliver src incrementet altså [H, 0, e, j], og dst = 1 fordi det ikke er et symbol og src++
    // str[dst] "1" = str[src] "2" = [H, e, 0, j], dst = 2, src++
    // str[dst] "2" = str[src] "3" = [H, e, j, 0], dst = 3, src++ og til sidst rammer vi \0 og løkken afsluttes
    char symbols[] = "½§!@#£¤$%%&/{([)]=}?+`´|\\><;,:.-_¨^~'*";

    for (size_t src = 0, dst = 0; (str[dst] = tolower(str[src])) != '\0'; src++)
        dst += (strchr(symbols, str[dst]) == NULL);
}

// Splits the given sentence into an array of words whenever it encounters a whitespace
// The words and length gets stored in the struct output paramater
void split_words(Doc *doc)
{
    // str token destroys the orignal string and breaks it into smaller strings and returns a pointer to them
    // There needs to be two different text variables since strtok returns the pointer to the modified string
    // A local scope text variable wouldn't suffice since it would be deleted after function run
    strcpy(doc->text_og, doc->text);
    split(doc->words_og, doc->text_og, &doc->words_og_length);
    remove_character(doc->text);
    split(doc->words, doc->text, &doc->words_length);
}

void split(char *arr[], char *text, int *length)
{
    *length = 0;
    char *word = strtok(text, " ");
    while (word != NULL)
    {
        arr[*length] = word;
        *length += 1;
        word = strtok(NULL, " "); // <- Next word
    }
}

// void check_quotations(char *word[], int *quatation_check, int *quatation_length){
//     char quatation[100][20];
//     int pos = 0;
//     if(strchr(*word, '"') != NULL)
//     {
//         *quatation_check += 1;
//     }
//     if (*quatation_check % 2 == 0)
//     {
//         if (strchr(*word, '"') != NULL)
//         {
//             printf("\033[1;33m");
//             *quatation_length++;
//             strcpy(quatation[pos], *word);
//             pos++;
//         }
//     }
//     else if (*quatation_check % 2 != 0)
//     {
//     printf("\033[1;33m");
//     *quatation_length++;
//     strcpy(quatation[pos], *word);
//         pos++;
//     }
//     for(int i = 0; i < pos; i++){
//         printf("%s ", quatation[i]);
//     }
// }

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

    // Calc percentage: Can't devide an integer with a higher integer without float conversion
    float count_len = count ? count + 1 : count,
          percent = count_len / user_len * 100;

    // Change similarity values to reflect the right index in the unedited words array.
    // If the length of the edited and unedited array are the same then the indexes represents the same words.
    if (user_len != user_doc->words_og_length && percent)
    {
        int sim_i = 0, same_word;
        // Loop through the edited and unedited words
        for (int i = 0, j = 0; i < user_doc->words_og_length && sim_i <= count; i++, j++)
        {
            // the similarity value is greater or equal to the current index of the unedited word
            if (user_doc->similarities[sim_i] == j)
            {
                // Check if the word is similar to the edited word by taking the first letter and checking if it's in the string
                same_word = strchr(user_doc->words_og[i], user_doc->words[j][0]) != NULL ||
                            strchr(user_doc->words_og[i], toupper(user_doc->words[j][0])) != NULL;

                // If same then the rest of the similarity values is incremented each time the words are not similar
                if (same_word)
                {
                    user_doc->similarities[sim_i] += i - j;
                    sim_i++;
                }
                // If not the same then decrement the index value of j to check the same word on the next run
                else
                    j--;
            }
        }
    }

    print_result(user_doc->words_og,
                 user_doc->similarities,
                 percent,
                 count,
                 user_doc->words_og_length);
}

void print_result(char *words[], int similar[], float percent, int sim_length, int user_len)
{
    char *color = percent > 15
                      ? "\x1b[31m" // <- Red
                  : percent > 5
                      ? "\033[0;33m"  // <- Yellow
                      : "\033[0;32m"; // <- Green
    printf("\n-------------------------%s\nPLAGIARIZED TEXT (%.1f%%):\n"
           "\x1b[0m-------------------------\n",
           color, percent);
    if (sim_length)
        for (int i = 0; i <= sim_length; i++)
            printf("%s ", words[similar[i]]);
    printf("\n\n\n");

    // Print the entire document where the colored text is plagiarised
    char show_doc;
    printf("Show the plagiarised text in your document?  \033[0;32m[y] Yes  \x1b[31m[n] No: \x1b[0m");
    scanf(" %c", &show_doc);
    if (show_doc == 'y')
    {
        printf("\n---------\n%sDOCUMENT:\x1b[0m\n---------\n", color);
        int j = 0;
        for (int i = 0; i < user_len; i++)
        {
            if (i == similar[j])
            {
                printf("%s", color);
                j++;
            }
            printf("%s \x1b[0m", words[i]);
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