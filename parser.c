#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parser(char** str) {
     *str = "lol";

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
}

void readFile(char** path, char** textContent) {
     FILE *file;
     file = fopen(*path, "r");
     if(file == NULL)
     {
          printf("Error!");   
          exit(1);             
     }

     char buffer[10];
     char *input = 0; //Size of input is unknow and will be changed
     size_t cur_len = 0; //size_t is an unsigned integer. This type is used to represent the size of an object

     //Reading file
     while (fgets(buffer, sizeof(buffer), file) != 0) {
          size_t buf_len = strlen(buffer);
          char *extra = realloc(input, buf_len + cur_len + 1);
          if (extra == 0)
               break;
          input = extra;
          strcpy(input + cur_len, buffer);
          cur_len += buf_len;
     }
     free(input);
     *textContent = input;
     fclose(file);
}

void splitText(char** text) {
     char delim[] = "\n";
     //strtok modifies the original string. It puts NULL characters ('\0') at the delimiter position after every call to strtok so that tokens can be tracked. strtok also internally remembers the next token's starting position.
     char *ptr = strtok(*text, delim);

     //In the next call to strtok, the first parameter needs to be NULL so that strtok starts splitting the string from the next token's starting position it remembers.
     int i;
     for (i = 0; ptr != NULL; i++)
	{
          printf("%s \n", ptr);
		ptr = strtok(NULL, delim);
	}
}

int main(void) {
     char* path = "demo.kaku";
     char* textContent;
     readFile(&path, &textContent);

     splitText(&textContent);

     // char* test = "# Title";
     // parser(&test);
     // printf("%s\n", ptr);
     return 0;
}