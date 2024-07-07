#include "../hdr/threadlib.h"
#include <stdio.h>

int main()
{
    thread_t th = {0};
    thread_create( &th, "Charles");
    printf("Thread %s, THREAD_CREATED\n", th.name);

    return 0;
}