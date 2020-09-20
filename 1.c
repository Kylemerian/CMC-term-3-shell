#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <stdbool.h>

typedef struct list {
    char * str;
    struct list * next;
}
list;

list * init(list * head) {
    head = malloc(sizeof( * head));
    head -> str = NULL;
    head -> next = NULL;
    return head;
}

char * extendbuff(char * buff, int * lenbuff) {
    char * newBuff = malloc(sizeof(char) * ( * lenbuff) * 2);
    strncpy(newBuff, buff, * lenbuff);
    * lenbuff *= 2;
    free(buff);
    return newBuff;
}

void printlist(list * head) {
    if (head != NULL) {
        printlist(head -> next);
        if (head -> next != NULL)
            printf("%s\n", head -> str);
    }
    return;
}

void printRecurs(list * headlist) {
    printlist(headlist);
    return;
}

void freemem(list * headlist) {
    if (headlist != NULL) {
        if (headlist -> str != NULL)
            free(headlist -> str);
        list * tmp = headlist;
        headlist = headlist -> next;
        free(tmp);
        freemem(headlist);
    }
    return;
}

list * addtolist(list * head, char * str, int lenbuff) {
    list * tmp = malloc(sizeof( * tmp));
    tmp -> str = malloc(sizeof(char) * (lenbuff + 1));
    strncpy(tmp -> str, str, lenbuff);
    tmp -> str[lenbuff] = 0;
    tmp -> next = head;
    return tmp;
}

int main() {
    int c;
    int lenbuff = 8;
    int i = 0;
    bool quoteflag = false;
    list * headlist = NULL;
    headlist = init(headlist);
    char * buff = malloc(sizeof(char) * lenbuff);
    printf("%s", ">> ");
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((c != ' ' && c != '\"') || (c == ' ' && quoteflag)) {
                if (i >= lenbuff - 1) {
                    buff = extendbuff(buff, & lenbuff);
                }
                buff[i] = c;
                i++;
            } else if (c != ' ' && c == '\"') {
                quoteflag = !quoteflag;
                if (i != 0) {
                    headlist = addtolist(headlist, buff, i);
                    i = 0;
                }
            } else if (i != 0) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
        } else {
            if (i != 0)
                headlist = addtolist(headlist, buff, i);
            i = 0;
            if (quoteflag) {
                printf("%s\n", "incorrect input");
            } else {
                printRecurs(headlist);
            }
            freemem(headlist);
            headlist = init(headlist);
            quoteflag = false;
            printf("%s", ">> ");
        }
    }
    printf("\n");
    free(buff);
    return 0;
}