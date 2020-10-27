#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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

int isChangedInOut(int c)
{
    if(c == '>' || c == '<')
        return 1;
    return 0;
}

void fillInOut(int prev, int cur, int * i, int * inOut, list * headlist)
{
    int size = listsize(headlist);
    if(*i > 0)
        size++;
    switch(cur){
        case '<':
            inOut[0] = size + 1;
            break;
        case '>':
            if(prev == '>')
                inOut[3] = size + 1;
            else if((prev == '1' || prev == '2') && *i > 0){
                inOut[prev - '0'] = size + 1;
                *i = 0;
            }
            else{
                inOut[1] = size + 1;
            }
            break;
    }
    /*printf("%d %d %d %d\n", inOut[0], inOut[1], inOut[2], inOut[3]);*/
}

int spacetabIOquote(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '<') || (c == '>'));
}

int spacetab(int c)
{
    return ((c == ' ') || (c == '\t'));
}

int separator(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '&') || (c == '>') || (c == '<'));
}

int inIO(int key, int * inOut)
{
    int i;
    int inFlag = 0;
    for(i = 0; i < 4; i++)
        if(inOut[i] == key){
            inFlag = 1;
            break;
        }
    return inFlag;
}

char ** makearr(list * headlist, int size, int * inOut)
{
    list * tmp = headlist;
    int i, j;
    int sizeIO = 0;
    for(i = 0; i < 4; i++){
        if(inOut[i] != 0)
            sizeIO++;
    }
    if(inOut[3] > 0)
        sizeIO--;
    j = size - 1 - sizeIO;
    char ** arr = malloc((size + 1 - sizeIO) * sizeof(*arr));
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        //printf("%d %s\n", i, tmp->str);
        if(!inIO(i + 1, inOut)){
            arr[j] = malloc(strlen(tmp->str) + 1);
            strncpy(arr[j], tmp->str, strlen(tmp->str));
            arr[j][strlen(tmp->str)] = 0;
            j--;
        }
    }
    /*for(i = 0; i < size - sizeIO; i++)
        printf("%d %s\n", i, arr[i]);*/
    arr[size - sizeIO] = (char*)NULL;
    return arr;
}

char * getFilename(int key, list * headlist)
{
    int it = 1;
    list * tpointer = headlist;
    while(tpointer != NULL){
        if(key == it)
            return tpointer->str;
        it++;
    }
    //printf("%s ", tpointer->str);
    return tpointer->str;
}

void changeIO(list * headlist, int * inOut)
{
    int fd;
    if(inOut[0]){
        fd = open(getFilename(inOut[0], headlist), O_RDONLY | O_CREAT);
        //printf("0 %d ", fd);
        dup2(fd, 0);
    }
    if(inOut[2]){
        fd = open(getFilename(inOut[2], headlist), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        //printf("2 %d ", fd);
        dup2(fd, 2);
    }
    if(inOut[3]){
        fd = open(getFilename(inOut[3], headlist),O_WRONLY | O_APPEND | O_CREAT, 0666);
        //printf("3 %d %s ", fd, getFilename(inOut[3], headlist));
        dup2(fd, 1);
    }
    if(inOut[3] == 0 && inOut[1]){
        fd = open(getFilename(inOut[1], headlist), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(fd, 1);
        //printf("1 %d %s ", fd, getFilename(inOut[1], headlist));
    }
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

void execute(list * headlist, int mode, int * inOut)
{
    int j;
    int pid;
    int size = listsize(headlist);
    char ** arr = makearr(headlist, size, inOut);
    if(!strcmp(arr[0], "cd")){
        changedir(arr, size);
    }
    else{
        pid = fork();
        if(pid == 0){
            changeIO(headlist, inOut);
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

void iscorrectcmd(int * quoteflag, int * amper, int chr, list ** head, int * inOut)
{
    int mode = (*amper == 1);
    int i;
    if (*quoteflag || (*amper > 1) || (*amper == 1 && chr != '&'))
        printf("incorrect input\n");
    else if((*head)->str != NULL)
        execute(*head, mode, inOut);
        /*printlist(*head);*/
    *quoteflag = 0;
    *amper = 0;
    for(i = 0; i < 4; i++)
        inOut[i] = 0;
}

void killbg()
{
    while(wait(NULL) != -1)
        ;
    printf("\n");
}

int main()
{
    int c, lastchar = 0, i = 0;
    int ampers = 0, quoteflag = 0, lenbuff = 8;
    int inOut[4] = {0, 0, 0, 0};
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
            else if (isChangedInOut(c))
                fillInOut(lastchar, c, &i, inOut, headlist);
            else if (c == '\"')
                quoteflag = !quoteflag;
            else if (c == '&')
                ampers++;
            if (i != 0 && !quoteflag && spacetabIOquote(c)) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
            if(!spacetab(c))
                lastchar = c;
        }
        else{
            processinglast(&i, buff, &headlist);
            iscorrectcmd(&quoteflag, &ampers, lastchar, &headlist, inOut);
            headlist = reinit(&headlist);
        }
    }
    freemem(headlist);
    free(buff);
    killbg();
    return 0;
}
