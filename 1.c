#include <stdio.h>

#include <stdlib.h>

#include <string.h>

<<<<<<< HEAD
typedef struct list {
    char * str;
    struct list * next;
} list;

list * init(list * head)
{
    head = malloc(sizeof( * head));
    head -> str = NULL;
    head -> next = NULL;
    return head;
}

char * extendbuff(char * buff, int lenbuff)
{
    char * newBuff = malloc(lenbuff * 2);
    strncpy(newBuff, buff, lenbuff - 1);
=======
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
    strncpy(newBuff, buff, (* lenbuff) - 1);
    * lenbuff *= 2;
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
    free(buff);
    return newBuff;
}

<<<<<<< HEAD
void printlist(list * head)
{
=======
void printlist(list * head) {
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
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

<<<<<<< HEAD
void printRecurs(list * headlist)
{
    printlist(headlist);
}

void freemem(list * headlist)
{
=======
void freemem(list * headlist) {
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
    if (headlist != NULL) {
        if (headlist -> str != NULL)
            free(headlist -> str);
        list * tmp = headlist;
        headlist = headlist -> next;
        free(tmp);
        freemem(headlist);
    }
}

<<<<<<< HEAD
list * addtolist(list * head, char * str, int lenbuff)
{
    list * tmp = malloc(sizeof( * tmp));
    tmp -> str = malloc(lenbuff + 1);
=======
list * addtolist(list * head, char * str, int lenbuff) {
    list * tmp = malloc(sizeof( * tmp));
    tmp -> str = malloc(sizeof(char) * (lenbuff + 1));
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
    strncpy(tmp -> str, str, lenbuff);
    tmp -> str[lenbuff] = 0;
    tmp -> next = head;
    return tmp;
}

<<<<<<< HEAD
int main()
{
    int c;
    int lenbuff = 8;
    int i = 0;
    int quoteflag = 0;
    char * buff = malloc(lenbuff);
    list * headlist = NULL;
    headlist = init(headlist);
=======
int main() {
    int c;
    int lenbuff = 8;
    int i = 0;
    bool quoteflag = false;
    list * headlist = NULL;
    headlist = init(headlist);
    char * buff = malloc(sizeof(char) * lenbuff);
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
    printf("%s", ">> ");
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((c != ' ' && c != '\"') || (c == ' ' && quoteflag)) {
                if (i >= lenbuff - 1) {
<<<<<<< HEAD
                    buff = extendbuff(buff, lenbuff);
					lenbuff *= 2;
=======
                    buff = extendbuff(buff, & lenbuff);
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
                }
				buff[i] = c;
                i++;
<<<<<<< HEAD
            }
			else if (c == '\"')
				quoteflag = !quoteflag;
			else if (i != 0) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
        }
		else {
=======
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
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
            if (i != 0)
                headlist = addtolist(headlist, buff, i);
            i = 0;
            if (quoteflag) {
                printf("%s\n", "incorrect input");
<<<<<<< HEAD
            }
			else {
=======
            } else {
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
                printRecurs(headlist);
            }
            freemem(headlist);
            headlist = init(headlist);
<<<<<<< HEAD
            quoteflag = 0;
=======
            quoteflag = false;
>>>>>>> b876df41fefa6f7c6541c285e5fbb8c81c5be066
            printf("%s", ">> ");
        }
    }
    freemem(headlist);
    free(buff);
    printf("\n");
    return 0;
}