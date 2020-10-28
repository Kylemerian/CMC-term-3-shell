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
    list * tmp = malloc(sizeof(* tmp));
    tmp -> str = malloc(lenbuff + 1);
    strncpy(tmp -> str, str, lenbuff);
    tmp -> str[lenbuff] = 0;
    tmp -> next = head;
    return tmp;
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
    /*printf("%s ", tpointer->str);*/
    return tpointer->str;
}

void changeIO(list * head, int * inOut)
{
    int fd;
    if(inOut[0]){
        fd = open(getFilename(inOut[0], head), O_RDONLY | O_CREAT);
        dup2(fd, 0);
    }
    if(inOut[2]){
        fd = open(getFilename(inOut[2], head),O_WRONLY | O_APPEND | O_CREAT, 0666);
        dup2(fd, 1);
    }
    if(inOut[2] == 0 && inOut[1]){
        fd = open(getFilename(inOut[1], head), O_WRONLY | O_CREAT, 0666);
        dup2(fd, 1);
    }
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

void handleIOquote(int lastc, int c, int * inOut, list * headlist, int i)
{
    int size = listsize(headlist) + 1;
    if(i > 0)
        size++;
    if(c == '<')
        if(inOut[0] == 0)
            inOut[0] = size;
        else
            inOut[0] = -1;
    else if(c == '>' && lastc == '>')
        if(inOut[2] == 0)
            inOut[2] = size;
        else
            inOut[0] = -1;
    else if(c == '>')
        if(inOut[1] == 0)
            inOut[1] = size;
        else
            inOut[0] = -1;
    else
        inOut[0] = -1;
}

int isIOsymbol(int c)
{
    return ((c == '>') || (c == '<'));
}

int spaceTab(int c)
{
    return ((c == '\t') || (c == ' '));
}

int spaceTabIOquote(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '<') || (c == '>'));
}

int separator(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '&') || (c == '>') || (c == '<'));
}

int inIO(int key, int * inOut)
{
    int i;
    int inFlag = 0;
    for(i = 0; i < 3; i++)
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
    for (int i =0; i < 3; i++)
        if(inOut[i] != 0)
            sizeIO++;
    if(inOut[2] > 0)
        sizeIO--;
    j = size - 1 - sizeIO;
    char ** arr = malloc((size + 1) * sizeof(*arr));
    for(i = size - 1; i >= 0; i--, tmp = tmp->next){
        if(!inIO(i + 1, inOut)){
            arr[i] = malloc(strlen(tmp->str) + 1);
            strncpy(arr[i], tmp->str, strlen(tmp->str));
            arr[i][strlen(tmp->str)] = 0;
            j--;
        }
    }
    arr[size - sizeIO] = (char*)NULL;
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

void execute(list * headlist, int mode, int * inOut)
{
    int j, pid;
    int size = listsize(headlist);
    char ** arr = makearr(headlist, size, inOut);
    if(!strcmp(arr[0], "cd"))
        changedir(arr, size);
    else{
        pid = fork();
        if(pid == 0){
            changeIO(headlist, inOut);
            execvp(arr[0], arr);
            perror(NULL);
            exit(1);
        }
        if(mode == 0)
            while(wait(NULL) != pid)
                ;
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

int isWrongIO(list * headlist, int * inOut)
{
    int size = listsize(headlist);
    int errflag = 0;
    int i = 0;
    for(i = 0; i < 3; i++)
        if(inOut[i] > size)
            errflag = 1;
    if(inOut[0] == -1)
        errflag = 1;
    return errflag;
}

void iscorrectcmd(int * qflag, int * amper, int chr, list ** head, int * inOut)
{
    int i;
    int mode = (*amper == 1);
    if (*qflag || (*amper > 1) || (*amper == 1 && chr != '&')
        || isWrongIO(*head, inOut))

        printf("incorrect input\n");
    else if((*head)->str != NULL){
        execute(*head, mode, inOut);
        /*printlist(*head);*/
    }
    *qflag = 0;
    *amper = 0;
    for(i = 0; i < 3; i++)
        inOut[i] = 0;
}

void killbg()
{
    while(wait4(-1, NULL, WNOHANG, NULL) > 0)
        ;
}

int main()
{
    int c, lastchar = 0;
    int ampers = 0, lenbuff = 8;
    int i = 0, quoteflag = 0;
    int inOutPos[3] = {0, 0, 0};
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
            else if (isIOsymbol(c))
                handleIOquote(lastchar, c, inOutPos, headlist, i);
            else if (c == '\"')
                quoteflag = !quoteflag;
            else if (c == '&')
                ampers++;
            if (i != 0 && !quoteflag && spaceTabIOquote(c)) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
            if(!spaceTab(c))
                lastchar = c;
        }
        else{
            processinglast(&i, buff, &headlist);
            iscorrectcmd(&quoteflag, &ampers, lastchar, &headlist, inOutPos);
            headlist = reinit(&headlist);
            killbg();
        }
    }
    freemem(headlist);
    free(buff);
    printf("\n");
    return 0;
}
