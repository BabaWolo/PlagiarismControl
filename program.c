#include "program.h"
#include "tests/AllTests.c"

int main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp(argv[1], "--test"))
        RunAllTests();
    else
        check_plagiarism_via_terminal();
    return 0;
}

// Run all the top functions that allows the user to check their documents for potential plagiarism
void check_plagiarism_via_terminal()
{
    Doc user;
    Doc src;
    get_file_configurations(user.filename, src.filename);
    read_file(&user, user.filename);
    read_file(&src, src.filename);
    split_words(&user);
    split_words(&src);
    find_quotations(&user);
    src.cited_words_len = 0;
    compare(&user, &src);
    finalize_doc(user, src);
    free_struct_vars(&user);
    free_struct_vars(&src);
}

// Main API function: Run all the top functions that allows the user to check their documents for potential plagiarism
void check_plagiarism(Doc *user, Doc *src, char user_filename[], char src_filename[], int language)
{
    g_language = language;
    strcpy(user->filename, user_filename);
    strcpy(src->filename, src_filename);
    read_file(user, user_filename);
    read_file(src, src_filename);
    split_words(user);
    split_words(src);
    find_quotations(user);
    src->cited_words_len = 0;
    compare(user, src);
    readd_symbols(*user);
    readd_symbols(*src);
}

// Get user input on the language and names of the files
void get_file_configurations(char *user_file, char *source_files)
{
    int i = 0, similar;
    printf("What language is your files written in? \033[0;30m[1]\x1b[0m English  \033[0;30m[2]\x1b[0m Danish: ");
    if (!scanf("%d", &g_language) || g_lang_len - g_language < 0)
    {
        g_language = English;
        printf("Program defaulted to english\n");
    }
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
    int j, run = 1, i = 0, size;

    filename = strtok(filenames, " ");
    while (filename != NULL)
    {
        if ((file = fopen(filename, "r")) == NULL)
        {
            printf("Could not open file: %s", filename);
            exit(0);
        }

        // Move cursor to the end of the file, use ftell to give you the position (length) and move the cursor back again
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Newline characters needs a space in front of them so words doesn't get merched together when removing symbols
        while ((c = fgetc(file)) != EOF)
            if (c == '\n')
                size++;
        fseek(file, 0, SEEK_SET);

        if (run < 2)
        {
            doc->text = (char *)malloc(size + 1); // Add 1 to make space for '\0'
            i = 0;
        }
        else
        {
            size += i + 2;
            doc->text = (char *)realloc(doc->text, size);
            char *string = " \a"; // Add bell character to indicate where a new file starts
            for (j = 0; j < 2; i++, j++)
                doc->text[i] = string[j]; // Remove \0 with a space and add bell character
        }

        // Read one character at a time until it reaches the end of the file (EOF)
        while ((c = fgetc(file)) != EOF)
        {
            // Support Windows OSs using CRLF by not seperating the newline and carriage return characters
            if ((c == '\n' || c == '\r') && (prev_c != '\r' && prev_c != '\n'))
            {
                doc->text[i] = ' ';
                i++;
            }
            doc->text[i] = c;
            i++;
            prev_c = c;
        }
        doc->text[i] = '\0';

        run++;
        doc->filenames[run - 1] = filename;
        filename = strtok(NULL, " "); // <- Next word
        fclose(file);
    }

    doc->og_text = (char *)malloc(size + 1);
    doc->token_og_text = (char *)malloc(size + 1);
    strcpy(doc->og_text, doc->text);
    strcpy(doc->token_og_text, doc->text);
}

// Prepares and splits text into words
void split_words(Doc *doc)
{
    split(&doc->og_words, doc->token_og_text, &doc->og_words_len);
    remove_characters(doc->text, g_language);
    doc->token_text = (char *)malloc(strlen(doc->text));
    strcpy(doc->token_text, doc->text);
    split(&doc->words, doc->token_text, &doc->words_len);
    doc->similarities = (int *)malloc(sizeof(int) * doc->words_len);
}

// Splits the given sentence into an array of words whenever it encounters a whitespace
void split(char **arr[], char *text, int *length)
{
    int i = 0;
    char *text_cpy = text;
    *length = 0;

    // Count number of words without destroying the original array
    while (*(text_cpy += strspn(text_cpy, " ")) != '\0') // Pointer = end of spacings
    {
        *length += 1;
        text_cpy += strcspn(text_cpy, " "); // Pointer = end of word
    }
    *arr = (char **)malloc(sizeof(char *) * (*length));

    // Split words and insert into array
    char *word = strtok(text, " ");
    while (word != NULL)
    {
        (*arr)[i] = word;
        i++;
        word = strtok(NULL, " ");
    }
}

// Remove all characters except keep_chars from a string
void remove_characters(char *str, int language)
{
    char keep_chars[50] = "0123456789abcdefghijklmnopqrstuvwxyz ";
    char dk_letters[] = "æøå", *text_cpy, *og_text_cpy;
    if (language == 2)
        strcat(keep_chars, dk_letters);

    for (size_t src = 0, dst = 0; (str[dst] = tolower(str[src])) != '\0'; src++)
        dst += (strchr(keep_chars, str[dst]) != NULL);
}

// Detect placement of words containing one quote
void find_quotations(Doc *doc)
{
    int i, quotations = 0;
    char *result;

    for (i = 0; i < doc->og_words_len && quotations < 200; i++)
    {
        result = strchr(doc->og_words[i], '"');
        if (result != NULL && strchr(result + 1, '"') == NULL)
            doc->cited_words[quotations++] = i;
    }
    doc->cited_words_len = quotations;
}

// Returns a bool value indicating if the given index ranges between two other indexes
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
void compare(Doc *user, Doc *src)
{
    int i, j, k, similar, strcmp_val = 0, sim_len = 0;
    int user_len = user->words_len, src_len = src->words_len;

    // Goes through each word in the user text and tries to detect it in the source text
    for (i = 0; i < user_len; i++)
    {
        for (j = 0; j < src_len; j++)
        {
            if (check_similarity(user->words[i], src->words[j]))
            {
                similar = 1;

                // Checks similarities of the following words
                while (i + similar < user_len &&
                       j + similar < src_len &&
                       check_similarity(user->words[i + similar], src->words[j + similar]))
                    similar++;

                // Check if the word(s) is an extension to other similarities
                if (similar <= 2 && sim_len && j > 0 && user->similarities[sim_len - 1] == i - 1)
                {
                    strcmp_val = check_similarity(user->words[i - 1], src->words[j - 1]);
                    if (strcmp_val && similar == 1 && j > 1)
                        strcmp_val = check_similarity(user->words[i - 2], src->words[j - 2]);
                }

                // If theres 3 or more identical words in a row then store the indexes of the words
                if (similar >= 3 || strcmp_val)
                {
                    for (k = 0; k < similar; k++)
                    {
                        user->similarities[sim_len + k] = i + k;
                        src->similarities[sim_len + k] = j + k;
                    }
                    i += similar - 1;
                    sim_len += similar;
                    break;
                }
            }
        }
    }
    user->sim_len = sim_len;
    src->sim_len = sim_len;
    delete_dublicates(src->similarities, &src->sim_len);
    qsort(src->similarities, src->sim_len, sizeof(int), comparator);
    user->percent = (float)user->sim_len / user->words_len * 100;
    src->percent = (float)src->sim_len / src->words_len * 100;
}

// Check if words are identical, synonyms and conjugations of eachother
int check_similarity(char *word1, char *word2)
{
    int result = !strcmp(word1, word2);
    if (!result)
    {
        char plural_suffix[3] = "s";
        int rows;
        if (g_language == Danish)
        {
            rows = sizeof(g_dk_synonyms) / sizeof(g_dk_synonyms[0]);
            result = check_synonyms(word1, word2, g_dk_synonyms, rows);
            strcpy(plural_suffix, "r");
        }
        else
        {
            rows = sizeof(g_en_synonyms) / sizeof(g_en_synonyms[0]);
            result = check_synonyms(word1, word2, g_en_synonyms, rows);
        }
        if (!result)
            result = check_conjugation(word1, word2, plural_suffix);
    }
    return result;
}

// Check if two words are eachothers synonyms in their respective language
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

// Check if two words are the same with different conjugations
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

// Prepares the document for printing
void finalize_doc(Doc user, Doc src)
{
    char show_doc;
    readd_symbols(user);
    print_result(user, 0);

    printf("Show the plagiarised text in the source document?  \033[0;32m[y] Yes  \x1b[31m[n] No: \x1b[0m");
    scanf(" %c", &show_doc);
    if (show_doc == 'y')
    {
        readd_symbols(src);
        print_result(src, 1);
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

// Prints the entire document where potentially plagiarised text is colored
void print_result(Doc doc, int is_src_doc)
{
    int i, j = 0, k = 1;
    char title[8] = " YOUR ", *color = doc.percent > 0 ? "\x1b[31m" : "\033[0;32m"; // <- Red or Green

    if (is_src_doc)
        strcpy(title, "SOURCE ");

    printf("\n\033[0;30m--------------------------------------------\n%s%s"
           "POTENTIALLY PLAGIARISED TEXT (%.1f%%):"
           "\n\033[0;30m--------------------------------------------\n%s\x1b[0m\n",
           color, title, doc.percent, doc.filenames[0]);

    for (i = 0; i < doc.og_words_len; i++)
    {
        if (doc.og_words[i][0] == '\a' && doc.filenames[k])
            printf("\n\n\033[0;30m%s\x1b[0m\n", doc.filenames[k++]);

        if (doc.sim_len && i == doc.similarities[j])
        {
            if (doc.cited_words_len > 1 && is_quoted(doc, i))
                printf("\033[0;33m");
            else
                printf("%s", color);
            j++;
        }
        printf("%s \x1b[0m", doc.og_words[i]);
    }
    printf("\n\n\n");
}

// Free all the dynamically allocated struct variables
void free_struct_vars(Doc *doc)
{
    free(doc->text);
    free(doc->og_text);
    free(doc->words);
    free(doc->og_words);
    free(doc->similarities);
    free(doc->token_og_text);
    free(doc->token_text);
}
