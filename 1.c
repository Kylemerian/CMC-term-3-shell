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
    printf(">> ");
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

int spaceortab(int c)
{
    return ((c == ' ') || (c == '\t'));
}

int separator(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '&'));
}

char ** makearr(list * headlist, int size)
{
    list * tmp = headlist;
    int i;
    char ** arr = malloc((size + 1) * sizeof(*arr));
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        arr[i] = malloc(strlen(tmp->str) + 1);
        strncpy(arr[i], tmp->str, strlen(tmp->str));
        arr[i][strlen(tmp->str)] = 0;
    }
    arr[size] = (char*)NULL;
    return arr;
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

void execute(list * headlist, int mode)
{
    int j;
    pid_t pid;
    int size = listsize(headlist);
    char ** arr = makearr(headlist, size);
    if(!strcmp(arr[0], "cd")){
        changedir(arr, size);
    }
    else{
        pid = fork();
        if(pid == 0){
            execvp(arr[0], arr);
            perror(NULL);
            exit(1);
        }
        if(mode == 0){
            while(wait(NULL) != pid)
                ;
        }
    }
    for(j = 0; j < size; j++)
          free(arr[j]);
    free(arr);
}

list * reinit(list ** headlist)
{
    freemem(*headlist);
    return init(*headlist);
}

void processinglast(int * iterator, char * buff, list **headlist)
{
    if(*iterator != 0)
        *headlist = addtolist(*headlist, buff, *iterator);
    *iterator = 0;
}

void iscorrectcommand(int * quoteflag, int * ampers, int chr, list ** headlist)
{
    int mode = (*ampers == 1);
    if (*quoteflag || (*ampers > 1) || (*ampers == 1 && chr != '&'))
        printf("incorrect input\n");
    else if((*headlist)->str != NULL)
        execute(*headlist, mode);
    *quoteflag = 0;
    *ampers = 0;
}

void killbg()
{
    while(wait(NULL) != -1)
        ;
}

int main()
{
    int c;
    int lastchar = 0;
    int ampers = 0;
    int lenbuff = 8;
    int i = 0;
    int quoteflag = 0;
    char * buff = malloc(lenbuff);
    list * headlist = NULL;
    headlist = init(headlist);
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((!separator(c) && c != '\"') || (separator(c) && quoteflag)){
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i] = c;
                i++;
            }
            else if (c == '\"')
                quoteflag = !quoteflag;
            else if (c == '&')
                ampers++;
            else if (i != 0) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
            if(!spaceortab(c))
                lastchar = c;
        }
        else{
            processinglast(&i, buff, &headlist);
            iscorrectcommand(&quoteflag, &ampers, lastchar, &headlist);
            headlist = reinit(&headlist);
        }
    }
    freemem(headlist);
    free(buff);
    killbg();
    printf("\n");
    return 0;
}
