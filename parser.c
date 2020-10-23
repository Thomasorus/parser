#include <stdio.h>

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

int main(void) {
     char* test = "# Title";
     parser(&test);
     printf("%s\n", test);
     return 0;
}