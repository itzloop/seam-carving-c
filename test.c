#include <stdio.h>
#include <stdlib.h>

typedef struct fext
{
    float val;
    int from;
} fext_t;
void test(fext_t ***m)
{
    *m = (fext_t **)(*m == NULL ? malloc(1080 * sizeof(fext_t *)) : realloc(*m, 1080 * sizeof(fext_t *)));
    for (int i = 0; i < 1080; ++i)
    {
        (*m)[i] = (*m)[i] == NULL ? malloc(1920 * sizeof(fext_t)) : realloc((*m)[i], 1920 * sizeof(fext_t));
    }
}

int main(int argc, char const *argv[])
{
    fext_t **a = NULL;
    test(&a);
    return 0;
}
