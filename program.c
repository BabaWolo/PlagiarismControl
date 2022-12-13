#include "program.h"
#include "tests/AllTests.c"

int main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp(argv[1], "--test"))
    {
        RunAllTests();
    }
    else
        check_plagiarism();
    return 0;
}

// Run all the top functions that allows the user to check their documents for potential plagiarism
void check_plagiarism()
{
    Doc user_doc;
    Doc source_doc;
    get_file_configurations(user_doc.filename, source_doc.filename);
    read_file(&user_doc, user_doc.filename);
    read_file(&source_doc, source_doc.filename);
    split_words(&user_doc);
    split_words(&source_doc);
    find_quotations(&user_doc);
    compare(&user_doc, &source_doc);
    finalize_doc(user_doc, source_doc);
    free_struct_vars(&user_doc);
    free_struct_vars(&source_doc);
}

// Get user input on the language and names of the files
void get_file_configurations(char *user_file, char *source_files)
{
    int i = 0, lang_len = 2, similar;
    printf("What language is your files written in? \033[0;30m[1]\x1b[0m English  \033[0;30m[2]\x1b[0m Danish: ");
    if (!scanf("%d", &language) || lang_len - language < 0)
        language = 1;
    flush_stdin();
    printf("Input your filename: ");
    scanf("%s", user_file);
    flush_stdin();
    printf("Input source filenames (divide mulitple filenames with a space): ");
    scanf("%[^\n]", source_files);
    // In development you should uncomment the two lines below and comment the lines above so you don't need to give an input all the time. !REMEMBER! to revert it back when pushing the code to github
    // strcpy(user_file, "user.txt");
    // strcpy(source_files, "src_1.txt src_2.txt");
}

// Remove buffer stdin values
void flush_stdin()
{ // clang-format off
    int c;
    while ((c = fgetc(stdin)) != '\n' && c != EOF);
} // clang-format on

// Read the contents of a given file and save the result in Doc
void read_file(Doc *doc, char filenames[])
{
    FILE *file;
    char c, prev_c, *filename;
    int j, run = 1, i = 0;

    filename = strtok(filenames, " ");
    while (filename != NULL)
    {
        // Opening file, set to read mode and checking if possible to open file
        if ((file = fopen(filename, "r")) == NULL)
        {
            printf("Could not open file: %s", filename);
            exit(0);
        }

        // Move cursor to the end of the file, use ftell to give you the position (length) and move the cursor back again
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Newline characters needs a space in front of them so words doesn't get merched together when removing symbols
        // Therefore the length of the file needs to be incremented again each time we meet a new line character
        while ((c = fgetc(file)) != EOF)
            if (c == '\n' || c == '\r')
                size++;
        fseek(file, 0, SEEK_SET);

        if (run < 2)
        {
            // Allocate memory based on the length of the file and add 1 to make space for '\0' at the end
            doc->text = (char *)malloc(size + 1);
            doc->og_text = (char *)malloc(size + 1);
            i = 0;
        }
        else
        {
            size += i + 3;
            doc->text = (char *)realloc(doc->text, size);
            doc->og_text = (char *)realloc(doc->og_text, size);
            char *string = " \a "; // Add bell character to indicate where a new file starts
            for (j = 0; j < 3; i++, j++)
            {
                doc->text[i] = string[j]; // Remove \0 with a space and add bell character
                doc->og_text[i] = string[j];
            }
        }

        doc->filenames[run - 1] = filename;

        // Read one character at a time until it reaches the end of the file (EOF)
        while ((c = fgetc(file)) != EOF)
        {
            // Support Windows OSs using CRLF by not seperating the newline and carriage return characters
            if ((c == '\n' && prev_c != '\r') || c == '\r')
            {
                doc->text[i] = ' ';
                doc->og_text[i] = ' ';
                i++;
            }
            doc->text[i] = c;
            doc->og_text[i] = c;
            i++;
            prev_c = c;
        }

        doc->text[i] = '\0';
        doc->og_text[i] = '\0';

        run++;
        filename = strtok(NULL, " "); // <- Next word
        fclose(file);
    }
}

// Splits the given sentence into an array of words whenever it encounters a whitespace
// The words and length gets stored in the struct output paramater
void split_words(Doc *doc)
{
    char keep_chars[50] = "0123456789abcdefghijklmnopqrstuvwxyz ";
    char dk_letters[] = "æøå";
    if (language == 2)
        strcat(keep_chars, dk_letters);

    // str token destroys the orignal string and breaks it into smaller strings and returns a pointer to them
    // There needs to be two different text variables since strtok returns the pointer to the modified string
    // A local scope text variable wouldn't suffice since it would be deleted after function run
    split(&doc->og_words, doc->og_text, &doc->og_words_len);
    remove_characters(doc->text, keep_chars);
    split(&doc->words, doc->text, &doc->words_len);
    doc->similarities = (int *)malloc(sizeof(int) * doc->words_len);
}

void split(char **arr[], char *text, int *length)
{
    int i = 0;
    *length = 0;
    char *text_cpy = text;
    // strspn doesn't destroy the original array which makes it ideal to get the amount of words
    // The first time strspn meets a character that's both in string 1 and 2 it begins to count
    // how many times string 2 characters appear in a row from that point and returns the value
    while (*(text_cpy += strspn(text_cpy, " ")) != '\0') // Moves the length of the whitespaces
    {
        // strcspn returns the length of the start position to the first occurance of a character in string 2
        *length += 1;
        text_cpy += strcspn(text_cpy, " "); // Moves the string pointer to the end of the word
    }
    *arr = (char **)malloc(sizeof(char *) * (*length));

    char *word = strtok(text, " ");
    while (word != NULL)
    {
        (*arr)[i] = word;
        i++;
        word = strtok(NULL, " "); // <- Next word
    }
}

void remove_characters(char *str, char *keep_chars)
{
    // Increment all "non-symbols"
    // strchr: Returns a pointer to the first occurrence of the character "str[dst]" in the string symbols, or NULL if the character is not found
    // Vi starter med src og dst = 0 hvis dst = symbol -> dst = 0, hvis dst = !symbol -> dst = 1.
    // I et array eksempel [!, H, e, j], fordi src "0" er et symbol bliver dst += NULL, str[dst] "0" bliver sat på str[src] "0" altså [0, H, e, j] og src++
    // vi kører for-løkken og fordi (str[dst] = str[src]), hvilket er (0 = 1), så bliver src incrementet altså [H, 0, e, j], og dst = 1 fordi det ikke er et symbol og src++
    // str[dst] "1" = str[src] "2" = [H, e, 0, j], dst = 2, src++
    // str[dst] "2" = str[src] "3" = [H, e, j, 0], dst = 3, src++ og til sidst rammer vi \0 og løkken afsluttes

    for (size_t src = 0, dst = 0; (str[dst] = tolower(str[src])) != '\0'; src++)
        dst += (strchr(keep_chars, str[dst]) != NULL);
}

void find_quotations(Doc *doc)
{
    int i, quotations = 0;
    char *result;

    for (i = 0; i < doc->og_words_len && quotations < 200; i++)
    {
        result = strchr(doc->og_words[i], '"');
        // If the word contains only one quote then add the index
        if (result != NULL && strchr(result + 1, '"') == NULL)
            doc->cited_words[quotations++] = i;
    }
    doc->cited_words_len = quotations;
}

int is_quoted(Doc doc, int index)
{
    int min, max;
    for (int i = 1; i < doc.cited_words_len; i += 2)
    {
        min = doc.cited_words[i - 1];
        max = doc.cited_words[i];
        if ((index - min) * (index - max) <= 0)
            return 1;
        else if (index < min)
            break;
    }
    return 0;
}

// Checks if two documents contains the exact same order of 3 or more words
void compare(Doc *user_doc, Doc *source_doc)
{
    int i, j, k, similar, strcmp_val = 0, sim_len = 0;
    int user_len = user_doc->words_len, source_len = source_doc->words_len;

    // Goes through each word in the user text and tries to detect it in the source text
    for (i = 0; i < user_len; i++)
    {
        for (j = 0; j < source_len; j++)
        {
            if (check_similarity(user_doc->words[i], source_doc->words[j]))
            {
                similar = 1;

                // Checks similarities of the following words
                while (i + similar < user_len &&
                       j + similar < source_len &&
                       check_similarity(user_doc->words[i + similar], source_doc->words[j + similar]))
                    similar++;

                // Check if the word(s) is an extension to other similarities
                if (similar <= 2 && sim_len && j > 0 && user_doc->similarities[sim_len - 1] == i - 1)
                {
                    strcmp_val = check_similarity(user_doc->words[i - 1], source_doc->words[j - 1]);
                    if (strcmp_val && similar == 1 && j > 1)
                        strcmp_val = check_similarity(user_doc->words[i - 2], source_doc->words[j - 2]);
                }

                // If theres 3 or more identical words in a row then store the indexes of the words
                if (similar >= 3 || strcmp_val)
                {
                    for (k = 0; k < similar; k++)
                    {
                        user_doc->similarities[sim_len + k] = i + k;
                        source_doc->similarities[sim_len + k] = j + k;
                    }
                    i += similar - 1;
                    sim_len += similar;
                    break;
                }
            }
        }
    }
    user_doc->sim_len = sim_len;
    source_doc->sim_len = sim_len;
    delete_dublicates(source_doc->similarities, &source_doc->sim_len);
    qsort(source_doc->similarities, source_doc->sim_len, sizeof(int), comparator);
}

int check_similarity(char *word1, char *word2)
{
    int result = !strcmp(word1, word2);
    if (!result)
    {
        char plural_suffix[3] = "s";
        int rows;
        if (language == 2)
        {
            rows = sizeof(dk_synonyms) / sizeof(dk_synonyms[0]);
            result = check_synonyms(word1, word2, dk_synonyms, rows);
            strcpy(plural_suffix, "r");
        }
        else
        {
            rows = sizeof(en_synonyms) / sizeof(en_synonyms[0]);
            result = check_synonyms(word1, word2, en_synonyms, rows);
        }
        if (!result)
            result = check_conjugation(word1, word2, plural_suffix);
    }
    return result;
}

int check_synonyms(char *word1, char *word2, const char *synonyms[][SYNONYM_COLS], int rows)
{
    int i, j = 0, result = 0, columns;

    for (i = 0; i < rows; i++)
        for (j = 0; j < SYNONYM_COLS && synonyms[i][j]; j++)
            if (!strcmp(synonyms[i][j], word1))
            {
                j = 0;
                do
                {
                    result = !strcmp(synonyms[i][j], word2);
                    if (result)
                        return result;
                } while (j < SYNONYM_COLS && synonyms[i][++j]);
                return result;
            }
    return result;
}

int check_conjugation(char *word1, char *word2, char *plural_suffix)
{
    char *word1_cpy = word1, *word2_cpy = word2;
    int result = 0;

    while (word1_cpy[0] && word2_cpy[0] && word1_cpy[0] == word2_cpy[0])
    {
        word1_cpy++;
        word2_cpy++;
    }

    if (!word1_cpy[0] || !word2_cpy[0])
        result = !strcmp(word1_cpy, plural_suffix) || !strcmp(word2_cpy, plural_suffix);
    return result;
}

void finalize_doc(Doc user_doc, Doc src_doc)
{
    char show_doc;
    readd_symbols(user_doc);
    print_result(user_doc);

    printf("Show the plagiarised text in the source document?  \033[0;32m[y] Yes  \x1b[31m[n] No: \x1b[0m");
    scanf(" %c", &show_doc);
    if (show_doc == 'y')
    {
        readd_symbols(src_doc);
        print_result(src_doc);
    }
}

// Change similarity values to reflect the right index in the unedited words array.
void readd_symbols(Doc doc)
{
    // If the length of the edited and unedited array are the same then the indexes represents the same words.
    if (doc.words_len != doc.og_words_len && doc.sim_len)
    {
        int sim_i = 0, same_word, sim_len = doc.sim_len;
        char edited_letter;

        // Loop through the edited and unedited words
        for (int i = 0, j = 0; i < doc.og_words_len && j < doc.words_len && sim_i < sim_len; i++, j++)
        {
            edited_letter = doc.words[j][0];
            // Check if the word is similar to the edited word by taking the first letter and checking if it's in the string
            same_word = strchr(doc.og_words[i], edited_letter) != NULL ||
                        strchr(doc.og_words[i], toupper(edited_letter)) != NULL;

            // If same then the rest of the similarity values is incremented each time the words are not similar
            if (doc.similarities[sim_i] == j && same_word)
                doc.similarities[sim_i++] += i - j;

            // If not the same then decrement the index value of j to check the same word on the next run
            else if (!same_word)
                j--;
        }
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

// Returns a value that indicates if num_1 is less, equal or higher than num_2
int comparator(const void *num_1, const void *num_2)
{
    return (*(int *)num_1 - *(int *)num_2); // Retreive value by type casting and dereferencing
}

// Prints the entire document where the colored text is potentially plagiarised
void print_result(Doc doc)
{
    int j = 0, k = 1;
    float percent = (float)doc.sim_len / doc.words_len * 100;
    static int run = 1;
    char title[8] = " YOUR ";
    char *color = percent > 0 ? "\x1b[31m" : "\033[0;32m"; // <- Red or Green

    if (run)
        strcpy(title, "SOURCE ");

    printf("\n\033[0;30m--------------------------------\n%s%s"
           "PLAGIARISED TEXT (%.1f%%):"
           "\n\033[0;30m--------------------------------\n%s\x1b[0m\n",
           color, title, percent, doc.filenames[0]);

    for (int i = 0; i < doc.og_words_len; i++)
    {
        if (i == doc.similarities[j])
        {
            // Only color the cited words in user text
            if (run % 2 && doc.cited_words_len > 1 && is_quoted(doc, i))
                printf("\033[0;33m");
            else
                printf("%s", color);
            j++;
        }
        if (doc.og_words[i][0] == '\a' && doc.filenames[k])
            printf("\n\n\033[0;30m%s\x1b[0m\n", doc.filenames[k++]);
        else
            printf("%s \x1b[0m", doc.og_words[i]);
    }
    printf("\n\n\n");
    run++;
}

void free_struct_vars(Doc *doc)
{
    free(doc->text);
    free(doc->og_text);
    free(doc->words);
    free(doc->og_words);
    free(doc->similarities);
}
