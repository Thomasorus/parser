/* to_gem
 * by grimmware <grimm@grimmwa.re> <grimmware@republic.circumlunar.space>
 *
 * # What
 * Convert regular markdown to `text/gemini` in an opinionated manner
 *
 * # How
 * Reads from STDIN and writes to STDOUT by default, alternatively the input
 * file can be supplied as the first argument and the output file as the
 * second. In both positions if you supply "-" then the default will be used so
 * you can (e.g.) specify stdin and an outfile thusly: `to_gem - outfile`
 *
 * Text reflow is considered out of scope (for now at least), and you should
 * check the output manually for errors
 *
 * # Why
 * tl;dr I wanted the practice and to have fun!
 *
 * I write a lot of notes in markdown and I'd like to be able to trivially
 * publish them in `text/gemini` on my capsule.
 *
 * This is very much "artisanal" C, in that it targets minimalism and
 * terseness even though for the specified use case it's more in the spirit of
 * low resource usage and minimalism of gemini and anachronistic byte-shaving
 * than is actually sensible for the problem - text conversion is not an easy
 * problem in C compared to other languages and the bytes shaved are very much
 * a drop in the ocean for such a low-resource problem.
 *
 * But sometimes you just like to express how you wish the world could be :)
 *
 * # Opinions
 * The opinion is that for every link we should replace it with a numbered
 * reference to a link below the block of text or whatever.
 *
 * To do this, we need to iteratively read through text, and for everything
 * that looks like a markdown link we'll replace it with `[link text][N]` where
 * N will be the number of the link in the file. Then when we reach the next
 * block we'll insert the link lines *first* and then move on.
 *
 * To get the full lowdown on intended behaviour, compare testfile.md and
 * testfile.gmi
 *
 * ## Format rules
 * - No closing square brackets in a link name - we could accomodate for this
 *   or it could just be sensible markdown :P
 * - Links in blocks will be ignored
 * - Link references should already be URI escaped - otherwise this is
 *   undefined behaviour. I've added code to URI escape newlines and spaces to
 *   URI encoded spaces (%20) because my own markdown was riddled with this
 *   problem, but that's it
 * - Alt-text is not supported so it will cause a breakage due to the last
 *   point - this is due to ease of implementation rather than philosophy, as
 *   I'd prefer that alt-text *was* supported for accessibility.
 *
 * # Contributions
 * Contributions should aim to use minimal memory and avoid reading through the
 * input stream more than once. `valgrind` should be used to ensure that all
 * memory is freed, and all memory allocations should aim to be reasonably
 * dynamic so as to not hog resources unnecessarily.
 *
 * If you don't know where else to reach me, `grimm at grimmwa.re` is the place
 * to email, or alternatively inside circumlunar.space I can be reached at
 * `grimmware at republic.circumlunar.space`! I'll walk you through `git
 * send-email` or other diff sending workflows if you've not done it before :)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHUNK 128
#define lines_to_print last_printed < linknames->size
#define realloc_error "ERROR: Could not allocate memory! Exiting\n"

enum boolean
{
    false,
    true
};

enum modes
{
    plaintext,
    block,
};

typedef struct
{
    char **strings;
    size_t size;
} List;

static List *initList(const char *string, size_t size)
{
    /* Initialize a List and return a pointer to it
   *
   * string: First entry as a null terminated string
   * size: Size of the string (NOT of the intended list, it grows dynamically)
   *
   * return: Pointer to a new List */
    List *l = malloc(sizeof(List));
    l->strings = malloc(sizeof(char *));
    l->strings[0] = (char *)malloc(size);
    strcpy(l->strings[0], string);
    l->size = 1;
    // printf("%s\n", l->strings[0]);
    return l;
}

static void freeList(List *l)
{
    /* Recursively free all malloc'd memory in a List
   *
   * l: Pointer to a list */
    for (size_t i = 0; i < l->size; i++)
    {
        free(l->strings[i]);
    }
    free(l->strings);
    free(l);
}

static void push(List *l, const char *string, size_t size)
{
    /* Push a new item onto the end of the list
   *
   * l: Pointer to a list
   * string: String to add to the list
   * size: size of the string */
    size_t s = l->size;
    char **tmp = realloc(l->strings, ++(l->size) * sizeof(char *));
    if (tmp == NULL)
    {
        fwrite(realloc_error, 1, sizeof(realloc_error), stderr);
        exit(1);
    }
    l->strings = tmp;
    l->strings[s] = (char *)malloc(size);
    strcpy(l->strings[s], string);
}

static void pop(List *l)
{
    /* Pop item from the list. This is not returned.
   *
   * l: Pointer to a list */
    if (l == NULL)
        return;
    if (l->size == 1)
    {
        l->size = 0;
        return;
    }
    free(l->strings[--l->size]);
    char **tmp = realloc(l->strings, l->size * sizeof(char *));
    if (tmp == NULL)
    {
        fwrite(realloc_error, 1, sizeof(realloc_error), stderr);
        exit(1);
    }
    l->strings = tmp;
}

static size_t print_from(List *names, List *refs, size_t from, FILE *out)
{
    /* Print links from a pair of lists from item with index `from` to the end
   *
   * names: pointer to a list of link names
   * refs: pointer to a list of link references
   * from: index to print from
   * out: output stream
   *
   * returns: index of last printed item */
    for (; from < names->size; from++)
    {
        fprintf(out, "=> ");
        char c;
        for (size_t i = 0; (c = (char)refs->strings[from][i]) != 0; i++)
        {
            if (c == ' ')
                fprintf(out, "%%20");
            else
                fputc(c, out);
        }
        fprintf(out, " %s [%ld]\n", names->strings[from], from);
    }
    return (from);
}

static FILE *safe_open(const char *filepath, const char *mode)
{
    /* Open and return a file pointer with error checking
   *
   * filepath: path to the file
   * mode: `fopen` format file mode
   *
   * returns: file pointer as returned by `fopen` */
    FILE *file = fopen(filepath, mode);
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file %s in mode %s\n", filepath, mode);
        exit(1);
    }
    return (file);
}

static size_t read_until(FILE *in, char until, char **ptr)
{
    /* Read input filestream until char `until` into `ptr` or bail out if EOF is
   * reached first. Handles input buffering with dynamic resizing in increments
   * of size `CHUNK`
   *
   * in: pointer to input file stream until: The target character to stop
   * reading at
   * ptr: pointer to the memory to read in to
   *
   * returns: number of bytes read */
    size_t bufsize = CHUNK;
    char *buf = malloc(bufsize);
    char c;
    size_t i;
    for (i = 0; (c = (char)fgetc(in)) != until; i++)
    {
        if (c == EOF)
        {
            fprintf(stderr, "ERROR: Reached EOF whilst looking for matching `%c`\n", until);
            exit(1);
        }
        if (i == bufsize - 2)
        { // Always leave space for \0
            char *newbuf = malloc(bufsize + CHUNK);
            if (newbuf == NULL)
            {
                fprintf(stderr, "Could not allocate memory!");
                exit(1);
            }
            memcpy(newbuf, buf, bufsize);
            bufsize += CHUNK;
            free(buf);
            buf = newbuf;
        }
        buf[i] = c == '\n' ? ' ' : c;
    }
    buf[i] = 0;
    *ptr = buf;
    return (i + 1);
}

static void read_push_until(FILE *in, char until, List **l, FILE *out)
{
    /* VERY specific to this program, wrapper for `read_until` that also handles
   * pushing the returned string onto a list, and optionally prints the
   * characters including the closing one to an output stream if provided
   *
   * in: pointer to input stream
   * until: char to stop reading at
   * l: list to append the read bytes to
   * out: output stream to print to. No output will be printed if this is NULL.
   */
    char *text = NULL;
    size_t s = read_until(in, until, &text);
    if (*l == NULL)
    {
        *l = initList(text, s);
    }
    else
    {
        push(*l, text, s);
    }
    if (out != NULL)
    {
        fwrite(text, 1, s - 1, out);
        fputc(until, out);
    }
    free(text);
    return;
}

int main(int argc, char *argv[])
{
    FILE *in = stdin;
    FILE *out = stdout;
    if (argc > 1 && strcmp(argv[1], "-") != 0)
        in = safe_open(argv[1], "r");
    if (argc > 2 && strcmp(argv[2], "-") != 0)
        out = safe_open(argv[2], "w");
    char c, last = 0;
    size_t linenum = 1;
    size_t last_printed = 0;
    enum modes mode = plaintext;      // Links are not respected in `block` mode
    enum boolean likely_link = false; // Used to indicate whether a `(` is
                                      // expected to be a link reference
    List *linknames = NULL;
    List *linkrefs = NULL;
    while ((c = (char)fgetc(in)) != EOF)
    {
        if (likely_link && (c != '('))
        {
            likely_link = false;
            // If we just parsed what we thought was a link name but it wasn't
            // trailed by a ref then it wasn't a link, so we pop it back off again
            pop(linknames);
        }
        switch (c)
        {
        case '[':
            putc(c, out);
            if (mode == plaintext && last != '\\')
            {
                read_push_until(in, ']', &linknames, out);
                likely_link = true;
            }
            break;
        case '(':
            if (likely_link)
            {
                read_push_until(in, ')', &linkrefs, NULL);
                fprintf(out, "[%ld]", linkrefs->size);
                likely_link = false;
                break;
            }
            fputc(c, out);
            break;
        case '`':
            fputc(c, out);
            if (last == '\n')
            {
                if ((c = (char)fgetc(in)) != '`')
                {
                    fputc(c, out);
                    break;
                }
                if ((c = (char)fgetc(in)) != '`')
                {
                    fputc(c, out);
                    break;
                }
                fprintf(out, "``");
                if (mode == block)
                {
                    mode = plaintext;
                }
                else if (mode == plaintext)
                    mode = block;
            }
            break;
        case '#':
            if (last == '\n')
            {
                if (linknames != NULL && lines_to_print)
                {
                    fputc('\n', out);
                    last_printed = print_from(linknames, linkrefs, last_printed, out);
                    fputc('\n', out);
                }
            }
            fputc(c, out);
            break;
        case '\n':
            linenum++;
            if (last == '\n' && linknames != NULL && lines_to_print)
            {
                fputc(c, out);
                last_printed = print_from(linknames, linkrefs, last_printed, out);
            }
            __attribute__((fallthrough));
        default:
            fputc(c, out);
        }
        last = c;
    }

    if (linknames)
    {
        if (lines_to_print)
            fputc('\n', out);
        last_printed = print_from(linknames, linkrefs, last_printed, out);
    }

    fclose(in);
    fclose(out);
    freeList(linknames);
    freeList(linkrefs);
    return 0;
}
