#ifndef _LIST_H
#define _LIST_H


typedef int elem_t;


const unsigned long CANARY = 0xDEADBABE;
const elem_t        POISON = 0xADADADAD;


struct node_t {

    elem_t value;

    size_t next;
    size_t prev;
};

struct list_t {

    node_t *data;

    size_t head;
    size_t tail;
    size_t free;

    size_t size;
    size_t capacity;
    size_t base_size;
    size_t factor;

    int islinear;

    size_t gdumpnum;
};



list_t ListCtor (size_t base_size = 1024);
void   ListDtor (list_t *list);

size_t PushAfter     (list_t *list, size_t pos, elem_t value);
size_t PushBefore    (list_t *list, size_t pos, elem_t value);
size_t PushTail      (list_t *list, elem_t value);
size_t PushHead      (list_t *list, elem_t value);
elem_t ListPop       (list_t *list, size_t pos);
size_t GetElemByNum  (list_t *list, size_t num);

int ListResizeUp     (list_t *list);
int ResizeUpIfNeed   (list_t *list, size_t *free_pos);
int ListResizeDown   (list_t *list);
int ResizeDownIfNeed (list_t *list);
int ListLinear       (list_t *list);

void  ListDump           (list_t *list);
void  GraphDump          (list_t *list);
char *CreateNodeCode     (char *buf, const char *name, const char *shape, const char *color, const char *label);
char *CreateListNodeCode (char *buf, const list_t *list);
char *CreateArrowCode    (char *buf, const char *from, const char *where, const char *style, const char *color, const char *weight);
void  MakeInvisChain     (FILE *output, const list_t *list);
void  MakeNodeChain      (FILE *output, const list_t *list);
void  MakeFreeChain      (FILE *output, const list_t *list);

void  ListPrint   (list_t *list);
int   PrepareData (list_t *list, size_t start, size_t end);
int   SetBirds    (node_t *data, size_t size);
char *toStr       (int num, char *buf);



#endif