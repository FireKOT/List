#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "config.h"
#include "list.h"


static const size_t BUF_SIZE = 1;
static const size_t STR_SIZE = 1000;


list_t ListCtor (size_t base_size) {

    int err = 0;
    list_t err_list {
        .data = nullptr,

        .head      = 0,
        .tail      = 0,
        .free      = 0,
 
        .size      = 0,
        .capacity  = 0,
        .base_size = 0,
        .factor    = 0,
 
        .islinear  = 0,
 
        .gdumpnum  = 0,
    };

    RET_ON_VAL(base_size > sizeof(size_t) / sizeof(node_t) - 10 * sizeof(CANARY), ERR_ARG_INVAL, err_list);

    node_t *data = (node_t*) calloc((base_size + BUF_SIZE)*sizeof(node_t) + 2*sizeof(CANARY), 1);
    RET_ON_VAL(!data, ERR_NULL_PTR, err_list);
    err = SetBirds(data, base_size + BUF_SIZE);
    RET_ON_VAL(err, ERR_NULL_PTR, err_list);

    list_t list = {
        .data = (node_t*) ((char*) data + sizeof(CANARY)),

        .head      = BUF_SIZE,
        .tail      = BUF_SIZE,
        .free      = BUF_SIZE,
 
        .size      = 0,
        .capacity  = base_size,
        .base_size = base_size,
        .factor    = 2,
 
        .islinear  = true,
 
        .gdumpnum  = 0,
    };

    err = PrepareData(&list, BUF_SIZE, BUF_SIZE + base_size);
    RET_ON_VAL(err, ERR_NULL_PTR, err_list);

    FILE *dump = fopen("dump.txt", "w");
    RET_ON_VAL(!dump, ERR_INCRR_FILE, err_list);
    FILE *gdump = fopen("gdump.html", "w");
    RET_ON_VAL(!gdump, ERR_INCRR_FILE, err_list);
    fclose(dump);
    fclose(gdump);

    return list;
}

void ListDtor (list_t *list) {

    size_t id = list->head;
    for (size_t i = 0; i < list->size; i++) {
        size_t next = list->data[id].next;

        list->data[id].value = POISON;
        list->data[id].next = 0;
        list->data[id].prev = 0;

        id = next;
    }
    free((node_t*) ((char*)list->data - sizeof(CANARY)));

    list->data = nullptr;
}

size_t PushAfter (list_t *list, size_t pos, elem_t value) {

    RET_ON_VAL(!list || pos > BUF_SIZE + list->capacity, ERR_INCRR_INPUT, 0);
    int err = 0;

    size_t free_pos = list->free;
    list->free = list->data[free_pos].next;

    err = ResizeUpIfNeed(list, &free_pos);
    RET_ON_VAL(err, ERR_NULL_PTR, 1);

    size_t next = list->data[pos].next;

    list->data[free_pos].value = value;
    list->data[free_pos].next  = next;
    list->data[free_pos].prev  = pos;

    list->data[pos].next = free_pos;
    list->data[next].prev = free_pos;

    if (pos == list->tail) list->tail = free_pos;
    else list->islinear = false;

    list->size++;

    return free_pos;
}

size_t PushBefore (list_t *list, size_t pos, elem_t value) {

    RET_ON_VAL(!list || pos > BUF_SIZE + list->capacity, ERR_INCRR_INPUT, 0);
    int err = 0;

    size_t free_pos = list->free;
    list->free = list->data[free_pos].next;

    err = ResizeUpIfNeed(list, &free_pos);
    RET_ON_VAL(err, ERR_NULL_PTR, 1);

    size_t prev = list->data[pos].prev;

    list->data[free_pos].value = value;
    list->data[free_pos].next  = pos;
    list->data[free_pos].prev  = prev;

    list->data[prev].next = free_pos;
    list->data[pos].prev  = free_pos;

    if (pos == list->head) list->head = free_pos;
    list->islinear = false;

    list->size++;

    return free_pos;
}

size_t PushTail (list_t *list, elem_t value) {

    return PushAfter(list, list->tail, value);
}

size_t PushHead (list_t *list, elem_t value) {

    return PushBefore(list, list->head, value);
}

elem_t ListPop (list_t *list, size_t pos) {

    RET_ON_VAL(!list || pos > BUF_SIZE + list->capacity, ERR_INCRR_INPUT, 0);

    elem_t value = list->data[pos].value;
    size_t next  = list->data[pos].next;
    size_t prev  = list->data[pos].prev;

    list->data[prev].next = next;
    list->data[next].prev = prev;

    if (pos == list->tail) list->tail = prev;
    else if (pos == list->head) list->head = next;
    else list->islinear = false;

    list->data[pos].value = POISON;
    list->data[pos].next  = list->free;
    list->data[pos].prev  = 0;

    list->size--;
    list->free = pos;

    printf("c1: %d\n", list->capacity);
    ResizeDownIfNeed(list);
    printf("c2: %d\n", list->capacity);

    return value;
}

size_t GetElemByNum (list_t *list, size_t num) {

    RET_ON_VAL(!list || num >=list->size, ERR_INCRR_INPUT, 0);

    if (list->islinear) {
        return list->data[list->head + num].value;
    }
    
    printf("Oh, it's long operation so you have enough time to play in the game while it's running\n");
    //system("sleep 3");                                //cringe
    //system("..\\Jibril-master\\Jibril-Menu.exe");
    abort();
}

void ListPrint (list_t *list) {

    RET_ON_VAL(!list, ERR_INCRR_INPUT, );

    size_t id = list->head;
    for (size_t i = 0; i < list->size; i++) {
        printf("%d ", list->data[id].value);
        id = list->data[id].next;
    }
    printf("\n");
}

int SetBirds (node_t *data, size_t size) {

    RET_ON_VAL(!data, ERR_NULL_PTR, 1);

    *((unsigned long*) data) = CANARY;
    *((unsigned long*) ((char*)data + sizeof(CANARY) + size*sizeof(node_t))) = CANARY;

    return 0;
}

int PrepareData (list_t *list, size_t start, size_t end) {

    RET_ON_VAL(!list, ERR_NULL_PTR, 1);

    for (size_t i = start; i < end-1; i++) {
        list->data[i].value = POISON;
        list->data[i].next  = i+1;
        list->data[i].prev  = 0;
    }

    list->data[end-1].value = POISON;
    list->data[end-1].next  = 0;
    list->data[end-1].prev  = 0;

    return 0;
}

int ListResizeUp (list_t *list) {

    RET_ON_VAL(!list, ERR_NULL_PTR, 1);

    node_t *data = (node_t*) realloc((char*)list->data - sizeof(CANARY),\
                                 (BUF_SIZE + list->capacity*list->factor)*sizeof(node_t) + 2*sizeof(CANARY));
    RET_ON_VAL(!data, ERR_NULL_PTR, 1);

    SetBirds(data, list->capacity*list->factor + BUF_SIZE);
    list->data = (node_t*) ((char*)data + sizeof(CANARY));
    PrepareData(list, BUF_SIZE + list->capacity, BUF_SIZE + list->capacity*2);
    list->capacity *= list->factor;

    return 0;
}

int ResizeUpIfNeed (list_t *list, size_t *free_pos) {

    RET_ON_VAL(!list || !free_pos, ERR_INCRR_INPUT, 1);

    if (*free_pos == 0) {
        *free_pos = list->capacity + BUF_SIZE;
        ListResizeUp(list);
        list->free = list->data[*free_pos].next;
    }

    return 0;
}

int ListResizeDown (list_t *list) {

    RET_ON_VAL(!list, ERR_NULL_PTR, 1);

    node_t *data = (node_t*) realloc((char*)list->data - sizeof(CANARY),\
                                 (BUF_SIZE + list->capacity/list->factor)*sizeof(node_t) + 2*sizeof(CANARY));
    RET_ON_VAL(!data, ERR_NULL_PTR, 1);

    SetBirds(data, list->capacity/list->factor + BUF_SIZE);
    list->data = (node_t*) ((char*)data + sizeof(CANARY));
    list->capacity /= list->factor;
    PrepareData(list, list->free, BUF_SIZE + list->capacity);

    return 0;
}

int ResizeDownIfNeed (list_t *list) {

    RET_ON_VAL(!list, ERR_INCRR_INPUT, 1);

    if (list->islinear == 1 && list->capacity > list->base_size && list->size * list->factor * list->factor <= list->capacity) {
        ListResizeDown(list);
    }

    return 0;
}

int ListLinear (list_t *list) {

    RET_ON_VAL(!list, ERR_NULL_PTR, 1);

    size_t id = list->head;
    for (size_t i = 0; i < list->size; i++) {
        list->data[i + BUF_SIZE].prev = list->data[id].value;
        id = list->data[id].next;
    }

    for (size_t i = 0; i < list->size; i++) {
        list->data[i + BUF_SIZE].value = list->data[i + BUF_SIZE].prev;
        list->data[i + BUF_SIZE].next  = i + BUF_SIZE + 1;
        list->data[i + BUF_SIZE].prev  = i + BUF_SIZE - 1;
    }
    list->data[BUF_SIZE].prev                  = BUF_SIZE + list->size - 1;
    list->data[BUF_SIZE + list->size - 1].next = BUF_SIZE;

    PrepareData(list, BUF_SIZE + list->size, BUF_SIZE + list->capacity);

    list->head = BUF_SIZE;
    list->tail = BUF_SIZE + list->size - 1;
    list->islinear = true;

    return 0;
}

void ListDump (list_t *list) {

    RET_ON_VAL(!list, ERR_NULL_PTR, );

    RET_ON_VAL(!list, ERR_NULL_PTR, );

    FILE *dump = fopen("dump.txt", "a");
    RET_ON_VAL(!dump, ERR_NULL_PTR, );


    fprintf(dump, "Head: %d\n", list->head);
    fprintf(dump, "Tail: %d\n", list->tail);
    fprintf(dump, "Next free: %d\n", list->free);
    fprintf(dump, "Is Linear: %d\n", list->islinear);

    fprintf(dump, "Value: ");
    for (size_t i = 0; i < list->capacity + BUF_SIZE; i++) {
        if (list->data[i].value == POISON) fprintf(dump, "%-7s ", "POISON");
        else fprintf(dump, "%-7d ", list->data[i].value);
    }
    fprintf(dump, "\n");

    fprintf(dump, "Next:  ");
    for (size_t i = 0; i < list->capacity + BUF_SIZE; i++) {

        fprintf(dump, "%-7d ", list->data[i].next);
    }
    fprintf(dump, "\n");

    fprintf(dump, "Prev:  ");
    for (size_t i = 0; i < list->capacity + BUF_SIZE; i++) {

        fprintf(dump, "%-7d ", list->data[i].prev);
    }
    fprintf(dump, "\n");


    fprintf(dump, "<-------------------------------");
    fprintf(dump, "\n\n");

    fclose(dump);
}

void GraphDump (list_t *list) {

    char buf[STR_SIZE] = {}, file_name[STR_SIZE] = {};
    sprintf(file_name, "gdump\\GraphImageCode#%s.txt", toStr(list->gdumpnum, buf));

    printf("%s\n", file_name);

    FILE *gdump = fopen(file_name, "w");
    printf("%s\n", file_name);

    RET_ON_VAL(!gdump, ERR_ACC_DENi, );
    fprintf(gdump, "digraph G{\n");
    fprintf(gdump, "rankdir = LR\n");
    fprintf(gdump, "\n");
    
    fprintf(gdump, "%s\n", CreateListNodeCode(buf, list));
    fprintf(gdump, "\n");

    for (size_t i = 0, id = list->head; i < list->size; i++) {
        char name[STR_SIZE] = {}, label[STR_SIZE] = {}, value[STR_SIZE] = {}, next[STR_SIZE] = {}, prev[STR_SIZE] = {};
        char color[STR_SIZE] = "\"#00FF7F\"";

        if (id == list->head || id == list->tail) strcpy(color, "\"#3FC391\"");

        sprintf(name, "node%s", toStr(id, buf));
        sprintf(label, "\"<value> value = %s | <id> id = %s | <next> next = %s| <prev> prev = %s\"",\
                toStr(list->data[id].value, value), toStr(id, buf), toStr(list->data[id].next, next), toStr(list->data[id].prev, prev));

        fprintf(gdump, "%s\n", CreateNodeCode(buf, name, "record", color, label));
        id = list->data[id].next;
    }
    fprintf(gdump, "\n");

    for (size_t i = 0, id = list->free; i < list->capacity - list->size; i++) {
        char name[STR_SIZE] = {}, label[STR_SIZE] = {}, value[STR_SIZE] = {}, next[STR_SIZE] = {}, prev[STR_SIZE] = {};

        sprintf(name, "free%s", toStr(id, buf));
        sprintf(label, "\"<value> value = %s | <id> id = %s | <next> next = %s| <prev> prev = %s\"",\
                toStr(list->data[id].value, value), toStr(id, buf), toStr(list->data[id].next, next), toStr(list->data[id].prev, prev));

        fprintf(gdump, "%s\n", CreateNodeCode(buf, name, "record", "\"#BA55D3\"", label));
        id = list->data[id].next;
    }
    fprintf(gdump, "\n");

    MakeInvisChain(gdump, list);

    MakeNodeChain(gdump, list);
    MakeFreeChain(gdump, list);

    fprintf(gdump, "}\n");
    fclose(gdump);

    FILE *html = fopen("gdump.html", "a");

    char cmd[STR_SIZE] = {}, img_name[STR_SIZE] = {};
    sprintf(img_name, "gdump\\GraphImage%s.svg", toStr(list->gdumpnum, buf));
    sprintf(cmd, "dot %s -T svg -o %s", file_name, img_name);
    system(cmd);

    fprintf(html, "<img src = %s>\n", img_name);
    fprintf(html, "<hr>\n");

    list->gdumpnum++;
}

char *CreateNodeCode (char *buf, const char *name, const char *shape, const char *color, const char *label) {

    *buf = '\0';
    sprintf(buf, "%s [shape = %s, style = filled, fillcolor = %s, label = %s];", name, shape, color, label);

    return buf;
}

char *CreateListNodeCode (char *buf, const list_t *list) {

    char label[STR_SIZE] = {}, head[STR_SIZE] = {}, tail[STR_SIZE] = {}, free[STR_SIZE] = {}, islinear[STR_SIZE] = {}, size[STR_SIZE] = {}, capacity[STR_SIZE] = {};

    sprintf(label, "\"head = %s | tail = %s | Free = %s | Is linear = %s | Used %s/%s nodes\"",\
                toStr(list->head, head), toStr(list->tail, tail), toStr(list->free, free), toStr(list->islinear, islinear), toStr(list->size, size), toStr(list->capacity, capacity));

    return CreateNodeCode(buf, "list", "record", "\"#7FFFD4\"", label);
}

char *CreateArrowCode (char *buf, const char *from, const char *where, const char *style, const char *color, const char *weight) {

    sprintf(buf, "%s -> %s [style = %s, color = %s, weight = %s];", from, where, style, color, weight);

    return buf;
}

void MakeInvisChain (FILE *output, const list_t *list) {

    char buf[STR_SIZE] = {};

    if (list->size > 0) {
        char name[STR_SIZE] = {};
        sprintf(name, "node%s", toStr(list->head, buf));

        fprintf(output, "%s\n", CreateArrowCode(buf, "list", name, "invis", "\"#9370DB\"", "13"));
    }
    else if (list->capacity - list->size > 0) {
        char name[STR_SIZE] = {};
        sprintf(name, "free%s", toStr(list->head, buf));
        
        fprintf(output, "%s\n", CreateArrowCode(buf, "list", name, "invis", "\"#9370DB\"", "13"));
    }
    fprintf(output, "\n");

    for (size_t i = 0, id = list->head; i + 1 < list->size; i++) {
        char from[STR_SIZE] = {}, where[STR_SIZE] = {};

        sprintf(from, "node%s", toStr(id, buf));
        sprintf(where, "node%s", toStr(list->data[id].next, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "invis", "\"#9370DB\"", "13"));

        id = list->data[id].next;
    }
    fprintf(output, "\n");

    if (list->size > 0 && list->capacity - list->size > 0) {
        char from[STR_SIZE] = {}, where[STR_SIZE] = {};

        sprintf(from, "node%s", toStr(list->tail, buf));
        sprintf(where, "free%s", toStr(list->free, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "invis", "\"#9370DB\"", "13"));
    }

    for (size_t i = 0, id = list->free; i + 1 < list->capacity - list->size; i++) {
        char from[STR_SIZE] = {}, where[STR_SIZE] = {};

        sprintf(from, "free%s", toStr(id, buf));
        sprintf(where, "free%s", toStr(list->data[id].next, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "invis", "\"#9370DB\"", "13"));

        id = list->data[id].next;
    }
}

void MakeNodeChain (FILE *output, const list_t *list) {

    char buf[STR_SIZE] = {};

    for (size_t i = 0, id = list->head; i + 1 < list->size; i++) {
        char from[STR_SIZE] = {}, where[STR_SIZE] = {};

        sprintf(from, "node%s:<next>:e", toStr(id, buf));
        sprintf(where, "node%s:<id>:w", toStr(list->data[id].next, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "bold", "\"#00FF66\"", "1"));
        *from  = '\0';
        *where = '\0';

        sprintf(from, "node%s:<prev>:w", toStr(list->data[id].next, buf));
        sprintf(where, "node%s:<id>:e", toStr(list->data[list->data[id].next].prev, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "bold", "\"#33CCCC\"", "1"));

        id = list->data[id].next;
    }  
}

void MakeFreeChain (FILE *output, const list_t *list) {

    char buf[STR_SIZE] = {};

    for (size_t i = 0, id = list->free; i + 1 < list->capacity - list->size; i++) {
        char from[STR_SIZE] = {}, where[STR_SIZE] = {};

        sprintf(from, "free%s:<next>:e", toStr(id, buf));
        sprintf(where, "free%s:<id>:w", toStr(list->data[id].next, buf));
        fprintf(output, "%s\n", CreateArrowCode(buf, from, where, "bold", "\"#00FF66\"", "1"));

        id = list->data[id].next;
    }  
}

char *toStr(int num, char *buf) {

    *buf = '\0';
    itoa(num, buf, 10);
    if (num == POISON) strcpy(buf, "POISON");

    return buf;
}