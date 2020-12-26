#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// OLD TEST WILL REDO LATER
// void parser(char** str) {
//      *str = "lol";
// char rules[2][20] = {
//      "/(#+)(.*)/g",
//      "/`{3,}(?!.*`)/g"
//       };
// int len = sizeof(rules)/sizeof(rules[0]);
// printf("%d\n", len);
// int i = 0;
// for(i = 0; i < len; i++){
//      printf("%d\n", i);
//      printf("%s\n", rules[i]);
// }
// }

void readFile(char *path, char **textContent)
{
     FILE *file;
     file = fopen(path, "r");
     if (file == NULL)
     {
          printf("Error file not found!");
          exit(1);
     }

     char buffer[10];    // 10 chars dans la stack
     char *input = 0;    //Size of input is unknow and will be changed
     size_t cur_len = 0; //size_t is an unsigned integer. This type is used to represent the size of an object

     // Reading file
     while (fgets(buffer, sizeof(buffer), file) != 0)
     {
          size_t buf_len = strlen(buffer);
          char *extra = realloc(input, buf_len + cur_len + 1);
          if (extra == 0)
               break;
          input = extra;
          strcpy(input + cur_len, buffer);
          cur_len += buf_len;
     }
     *textContent = input;
     fclose(file);
}

void splitText(char *text, char *delimiter, char **splittedText)
{
     char *ptr = strtok(text, delimiter);
     char test[] = "";
     int i;
     for (i = 0; ptr != NULL; i++)
     {
          printf("Line: %s \n", ptr);
          ptr = strtok(NULL, delimiter);
     }
}

int main(void)
{
     char *path = "demo.kaku";

     // Recover text file and get the number of lines
     char *textContent = 0; //Point vers rien aka le début de la ram ptdr je comprends rien mais un peu
     readFile(path, &textContent);
     int contentLength = strlen(textContent);
     printf("Number of characters: %d \n", contentLength);

     char *splittedText = 0;
     char *delimiter = "\n";
     splitText(textContent, delimiter, &splittedText);
     printf("Number of lines without empty text:%d \n", *splittedText);

     return 0;
}