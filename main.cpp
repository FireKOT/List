#include <stdio.h>
#include "list.h"
#include "config.h"


int main () {

    list_t list = ListCtor(2);
    RET_ON_VAL(!list.data, ERR_NULL_PTR, 1);

    

    ListDtor(&list);


    return 0;
}



