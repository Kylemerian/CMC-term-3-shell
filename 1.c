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

int spaceortab(char c)
{
    return ((c == ' ') || (c == '\t'));
}


void changedir(char ** arr, int size)
{
    int status;
    if(size == 2){
        status = chdir(arr[1]);
        if(status == -1)
            perror(arr[1]);
    }
    else if(size == 1){
        printf("Too few args\n");
    }
    else
        printf("Too many args\n");
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
    if(!strcmp(arr[0], "cd")){
        changedir(arr, size);
    }
    else{
        pid_t pid = fork();
        if(pid == 0){
            execvp(arr[0], arr);
            perror(NULL);
            exit(1);
        }
        wait(NULL);
    }
    for(j = 0; j < size; j++)
          free(arr[j]);
    free(arr);
}

list * reinit(list ** headlist)
{
    freemem(*headlist);
    printf(">>");
    return init(*headlist);
}

void processinglast(int * iterator, char * buff, list **headlist)
{
    if(*iterator != 0)
        *headlist = addtolist(*headlist, buff, *iterator);
    *iterator = 0;
}

void iscorrectquote(int * quoteflag, list ** headlist)
{
    if (*quoteflag)
        printf("incorrect input\n");
    else if((*headlist)->str != NULL)
        execute(*headlist);
    *quoteflag = 0;
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
    printf(">> ");
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((!spaceortab(c) && c != '\"') || (spaceortab(c) && quoteflag)){
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
        else{
            processinglast(&i, buff, &headlist);
            iscorrectquote(&quoteflag, &headlist);
            headlist = reinit(&headlist);
        }
    }
    freemem(headlist);
    free(buff);
    printf("\n");
    return 0;
}
