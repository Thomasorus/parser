#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void splitText(char *text, char *delimiter, char **splittedText)
{
    char *ptr = strtok(text, delimiter);
    int i;
    for (i = 0; ptr != NULL; i++)
    {
        printf("Adding line: %s \n", ptr);
        strcpy(*splittedText, ptr);
        ptr = strtok(NULL, delimiter);
    }
    //free(ptr);
}

int main()
{

    char textContent[100] = "Some\nMultiline\ntext";
    char *splittedText;
    char *delimiter = "\n";
    splitText(textContent, delimiter, &splittedText);
    //    printf("Number of lines without empty text:%d \n", *splittedText);
}

// char *