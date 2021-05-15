#include <stdio.h>

int main()
{
    FILE *file = fopen("demo.kaku", "r");
    char out[1024];
    int count = 0;
    char ch;
    char previous;

    int isTitle = 0;
    int isBold = 0;
    int isEm = 0;
    int isStrike = 0;
    int isCode = 0;

    while ((ch = (char)fgetc(file)) != EOF)
    {
        printf("%c\n", ch);
        switch (ch)
        {
        case '#':
            if (previous == '\n' || count == 0)
            {
                isTitle += 1;
                printf("%s\n", "Start Title");
            }
            if (previous == '#')
            {
                isTitle += 1;
                printf("%s\n", "Increment Title");
            }
            break;
        case '*':
            if (isBold == 0)
            {
                isBold = 1;
                printf("%s\n", "Start Bold");
            }
            else
            {
                printf("%s\n", "End Bold");
            }
            break;
        case '_':
            if (isEm == 0)
            {
                isEm = 1;
                printf("%s\n", "Start emphasis");
            }
            else
            {
                printf("%s\n", "End Emphasis");
            }
            break;
        case '~':
            if (isStrike == 0)
            {
                isStrike = 1;
                printf("%s\n", "Start strike");
            }
            else
            {
                printf("%s\n", "End strike");
            }
            break;
        case '`':
            if (isCode == 0)
            {
                isCode = 1;
                printf("%s\n", "Start code");
            }
            else
            {
                printf("%s\n", "End code");
            }
            break;
        case '\n':
            if (isTitle > 0)
            {
                printf("%s\n", "End title");
                printf("%d\n", isTitle);
                isTitle = 0;
            }
        default:
            return 1;
        }
        previous = ch;
        count += 1;
    }
    fclose(file);
}