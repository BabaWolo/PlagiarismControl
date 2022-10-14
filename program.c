#include <stdio.h>
#include <string.h>

char your_text[] = "This is some text that we'll check for plagiarism";
char controlled_text[] = "This is some text that we'll compare with sdfsf sdfsf sdfdsf";
char your_text_arr[10];

int main()
{
    int text_size = strlen(your_text);
    
    char * word = strtok(controlled_text, " "); 

    int length = 0;
    printf("%d", length);

    for(int i = 0; i > 9; i++) {
        your_text_arr[i] = word;
        word = strtok(NULL, " ");
    }

    for(int i = 0; i > 10; i++) {
        printf("%d\n", your_text_arr[i]);
    }

    /*while (word != NULL) {
        printf("%d\n", your_text_arr[length]);
        length++;
        ///word = strtok(NULL, " ");
    }*/
    
    return 0;
}

