#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct Document
{
    char text[100];
    char og_text[100]; // og = original
    char *words[100];
    char *og_words[100];
    int words_len;
    int og_words_len;
} Doc;

void remove_character(char *str);
void split_words(Doc *doc);
void split(char *arr[], char *text, int *length);
void check_quotations(char *word[], int *quatation_check, int *quatation_len);
void compare(Doc *user_doc, Doc source_doc);
void check_plagiarism();
int read_file(char text[], char *filename);
void print_result(char *words[], int similar[], float percent, int user_len);
void delete_dublicates(int arr[], int *length);
void bubbleSort(int arr[], int n);
void swap(int *high, int *low);
void readd_symbols(Doc doc, int similar[], int sim_len);

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
    char letters[] = "abcdefghijklmnopqrstuvwxyzæøåABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ ";

    for (size_t src = 0, dst = 0; (str[dst] = tolower(str[src])) != '\0'; src++)
        dst += (strchr(letters, str[dst]) != NULL);
}

// Splits the given sentence into an array of words whenever it encounters a whitespace
// The words and length gets stored in the struct output paramater
void split_words(Doc *doc)
{
    // str token destroys the orignal string and breaks it into smaller strings and returns a pointer to them
    // There needs to be two different text variables since strtok returns the pointer to the modified string
    // A local scope text variable wouldn't suffice since it would be deleted after function run
    strcpy(doc->og_text, doc->text);
    split(doc->og_words, doc->og_text, &doc->og_words_len);
    remove_character(doc->text);
    split(doc->words, doc->text, &doc->words_len);
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

// void check_quotations(char *word[], int *quatation_check, int *quatation_len){
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
//             *quatation_len++;
//             strcpy(quatation[pos], *word);
//             pos++;
//         }
//     }
//     else if (*quatation_check % 2 != 0)
//     {
//     printf("\033[1;33m");
//     *quatation_len++;
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
    int i, j, k, similar, strcmp_val = 0, count = 0, similarities[100], similarities_src[100];
    int user_len = user_doc->words_len, source_len = source_doc.words_len;
    char show_doc;

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
                if (similar <= 2 && count && j - 1 >= 0)
                {
                    strcmp_val = !strcmp(user_doc->words[similarities[count - 1]], source_doc.words[j - 1]);
                    if (strcmp_val && similar == 1 && j - 2 >= 0)
                        strcmp_val = !strcmp(user_doc->words[similarities[count - 2]], source_doc.words[j - 2]);
                }

                // If theres 3 or more identical words in a row then store the indexes of the words
                if (similar >= 3 || strcmp_val)
                {
                    for (k = 0; k < similar; k++)
                    {
                        similarities[count + k] = i + k;
                        similarities_src[count + k] = j + k;
                    }
                    i += similar - 1;
                    count += similar;
                    break;
                }
            }
        }
    }

    // If the length of the edited and unedited array are the same then the indexes represents the same words.
    if (user_len != user_doc->og_words_len && count)
        readd_symbols(*user_doc, similarities, count);

    // Print the entire document where the colored text is plagiarised
    print_result(user_doc->og_words,
                 similarities,
                 (float)count / user_len * 100, // <- Percent
                 user_doc->og_words_len);

    // >>> Comment the two following lines if you don't want to give an input all the time <<<
    printf("Show the plagiarised text in the source document?  \033[0;32m[y] Yes  \x1b[31m[n] No: \x1b[0m");
    scanf(" %c", &show_doc);
    if (show_doc == 'y')
    {
        delete_dublicates(similarities_src, &count);
        bubbleSort(similarities_src, count);

        if (source_len != source_doc.og_words_len && count)
            readd_symbols(source_doc, similarities_src, count);

        print_result(source_doc.og_words,
                     similarities_src,
                     (float)count / source_len * 100, // <- Percent
                     source_doc.og_words_len);
    }
}

// Change similarity values to reflect the right index in the unedited words array.
void readd_symbols(Doc doc, int similar[], int sim_len)
{
    int sim_i = 0, same_word;
    char edited_letter;

    // Loop through the edited and unedited words
    for (int i = 0, j = 0; i < doc.og_words_len && sim_i <= sim_len; i++, j++)
    {
        edited_letter = doc.words[j][0];
        // Check if the word is similar to the edited word by taking the first letter and checking if it's in the string
        same_word = strchr(doc.og_words[i], edited_letter) != NULL ||
                    strchr(doc.og_words[i], toupper(edited_letter)) != NULL;

        // If same then the rest of the similarity values is incremented each time the words are not similar
        if (similar[sim_i] == j && same_word)
        {
            similar[sim_i] += i - j;
            sim_i++;
        }
        // If not the same then decrement the index value of j to check the same word on the next run
        else if (!same_word)
            j--;
    }
}

// Delete other instances of a value to keep the elements unique
void delete_dublicates(int arr[], int *length)
{
    int i, j, k;
    for (i = 0; i < *length; i++)
    {
        for (j = i + 1; j < *length; j++)
        {
            if (arr[i] == arr[j])
            {
                for (k = j; k < *length; k++)
                    arr[k] = arr[k + 1];

                j--;
                *length -= 1;
            }
        }
    }
}

// Sort the array from low to high values
void bubbleSort(int arr[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1])
                swap(&arr[j], &arr[j + 1]);
}

// Swap the high and low value so the higher value gets a higher index
void swap(int *high_val, int *low_val)
{
    int temp = *low_val;
    *low_val = *high_val;
    *high_val = temp;
}

void print_result(char *words[], int similar[], float percent, int user_len)
{
    int j = 0;
    static int run = 0;
    char title[8] = " YOUR ";
    char *color = percent > 0 ? "\x1b[31m" : "\033[0;32m"; // <- Red or Green

    if (run)
        strcpy(title, "SOURCE ");
    run++;

    printf("\n--------------------------------\n%s%s"
           "PLAGIARISED TEXT (%.1f%%):\x1b[0m"
           "\n--------------------------------\n",
           color, title, percent);

    for (int i = 0; i < user_len; i++)
    {
        if (i == similar[j])
        {
            printf("%s", color);
            j++;
        }
        printf("%s \x1b[0m", words[i]);
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