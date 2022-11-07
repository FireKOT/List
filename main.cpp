#include <stdio.h>
#include "list.h"
#include "config.h"


int main () {

    list_t list = ListCtor(2);
    RET_ON_VAL(!list.data, ERR_NULL_PTR, 1);

    PushTail(&list, 1);
    PushTail(&list, 2);
    PushTail(&list, 3);
    PushTail(&list, 4);
    PushTail(&list, 5);
    GraphDump(&list);
    ListPop(&list, 5);
    GraphDump(&list);
    ListPop(&list, 4);
    GraphDump(&list);
    ListPop(&list, 3);
    printf("size: %d\n", list.size);
    GraphDump(&list);
    ListPop(&list, 2);
    GraphDump(&list);

    ListDtor(&list);


    return 0;
}



