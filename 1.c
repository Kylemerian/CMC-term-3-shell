#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/wait.h>

typedef struct list {
    char * str;
    struct list * next;
}
list;

list * init(list * head)
{
    head = malloc(sizeof( * head));
    head -> str = NULL;
    head -> next = NULL;
    return head;
}

char * extendbuff(char * buff, int * lenbuff)
{
    char * newBuff = malloc((*lenbuff) * 2);
    strncpy(newBuff, buff, (*lenbuff) - 1);
    *lenbuff *= 2;
    free(buff);
    return newBuff;
}

void printlist(list * head)
{
    if (head != NULL) {
        printlist(head -> next);
        if (head -> next != NULL)
            printf("%s\n", head -> str);
    }
}

void printRecurs(list * headlist)
{
    printlist(headlist);
}

void freemem(list * headlist)
{
    if (headlist != NULL) {
        if (headlist -> str != NULL)
            free(headlist -> str);
        list * tmp = headlist;
        headlist = headlist -> next;
        free(tmp);
        freemem(headlist);
    }
}


list * addtolist(list * head, char * str, int lenbuff)
{
    list * tmp = malloc(sizeof( * tmp));
    tmp -> str = malloc(lenbuff + 1);
    strncpy(tmp -> str, str, lenbuff);
    tmp -> str[lenbuff] = 0;
    tmp -> next = head;
    return tmp;
}

int listsize(list * headlist)
{
    int iterator = 0;
    list * tmp = headlist;
    while(tmp != NULL){
        iterator++;
        tmp = tmp->next;
    }
    return (iterator - 1);
}

void execute(list * headlist)
{
    int size = listsize(headlist);
    char ** arr = malloc((size + 1)  * sizeof(*arr));
    list * tmp = headlist;
    int i, j;
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        arr[i] = malloc(strlen(tmp->str) + 1);
        strncpy(arr[i], tmp->str, strlen(tmp->str));
        arr[i][strlen(tmp->str)] = 0;
    }
    arr[size] = (char*)NULL;
    pid_t pid = fork();
    if(pid == 0){
        execvp(arr[0], arr);
        perror(NULL);
        exit(1);
    }
    int status = wait(NULL);
    while(status != -1){
        status = wait(NULL);
    }
    for(j = 0; j < size; j++)
          free(arr[j]);
    free(arr);
}

void endline(list ** headlist, char * buff, int * quoteflag, int * iterator)
{
    if (*iterator != 0)
        *headlist = addtolist(*headlist, buff, *iterator);
    *iterator = 0;
    if (*quoteflag)
        printf("%s\n", "incorrect input");
    else
        execute(*headlist);
    freemem(*headlist);
    *headlist = init(*headlist);
    *quoteflag = 0;
    printf("%s", ">>");
}

int main()
{
    int c;
    int lenbuff = 8;
    int i = 0;
    int quoteflag = 0;
    char * buff = malloc(lenbuff);
    list * headlist = NULL;
    headlist = init(headlist);
    printf("%s", ">> ");
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((c != ' ' && c != '\"') || (c == ' ' && quoteflag)) {
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i] = c;
                i++;
            }
            else if (c == '\"')
                quoteflag = !quoteflag;
            else if (i != 0) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
        }
        else
            endline(&headlist, buff, &quoteflag, &i);
    }
    freemem(headlist);
    free(buff);
    printf("\n");
    return 0;
}
