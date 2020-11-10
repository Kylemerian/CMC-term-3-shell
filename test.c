#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

enum Flags {
    quoteflag,
    amperflag
};

enum InOut {
    in,
    out,
    outapp
};

typedef struct list {
    char * str;
    struct list * next;
}
list;

typedef struct pline{
    int pos;
    struct pline * next;
}
pline;

void init(list ** head, pline ** pipelist)
{
    *pipelist = malloc(sizeof(** pipelist));
    (*pipelist) -> pos = 1;
    (*pipelist) -> next = NULL;
    *head = malloc(sizeof(** head));
    (*head) -> str = NULL;
    (*head) -> next = NULL;
    printf(">> ");
}

char * extendbuff(char * buff, int * lenbuff)
{
    char * newBuff = malloc((*lenbuff) * 2);
    strncpy(newBuff, buff, (*lenbuff) - 1);
    *lenbuff *= 2;
    free(buff);
    return newBuff;
}

void printlist2(pline * pipelist)
{
    if (pipelist != NULL) {
        printlist2(pipelist -> next);
        if (pipelist -> next != NULL)
            printf("%d\n", pipelist -> pos);
    }
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

void printRecurs2(pline * pipelist)
{
    printlist2(pipelist);
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

void freemem2(pline * pipelist)
{
    if (pipelist != NULL) {
        pline * tmp = pipelist;
        pipelist = pipelist -> next;
        free(tmp);
        freemem2(pipelist);
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

pline * addtolist2(pline * pipelist, int key)
{
    pline * tmp = malloc(sizeof(* tmp));
    tmp -> pos = key;
    tmp -> next = pipelist;
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

int changeIO(list * head, int * inOut)
{
    int fd;
    int errflag = 0;
    if(inOut[in]){
        fd = open(getFilename(inOut[in], head), O_RDONLY|O_CREAT);
        if(fd != -1)
            dup2(fd, 0);
        else
            errflag = 1;
    }
    if(inOut[outapp]){
        fd = open(getFilename(inOut[outapp], head),O_WRONLY|O_APPEND|O_CREAT, 0666);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    if(inOut[outapp] == 0 && inOut[out]){
        fd = open(getFilename(inOut[out], head), O_WRONLY|O_CREAT, 0666);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    return errflag;
}

int listsize2(pline * pipelist)
{
    int i = 0;
    pline * tmp = pipelist;
    while(tmp != NULL){
        i++;
        tmp = tmp -> next;
    }
    return (i - 1);
}

int listsize(list * headlist)
{
    int iterator = 0;
    list * tmp = headlist;
    while(tmp != NULL){
        iterator++;
        tmp = tmp -> next;
    }
    return (iterator - 1);
}

void handlePLine(int c, pline ** pipelist, list * head, int i){
    int size = listsize(head) + 1;
    if(i > 0)
        size++;
    *pipelist = addtolist2(*pipelist, size);
}

void handleIOquote(int lastc, int c, int * inOut, list * headlist, int i)
{
    int size = listsize(headlist) + 1;
    if(i > 0)
        size++;
    if(c == '<')
        if(inOut[in] == 0)
            inOut[in] = size;
        else
            inOut[in] = -1;
    else if(c == '>' && lastc == '>')
        if(inOut[outapp] == 0)
            inOut[outapp] = size;
        else
            inOut[in] = -1;
    else if(c == '>')
        if(inOut[out] == 0)
            inOut[out] = size;
        else
            inOut[in] = -1;
    else
        inOut[in] = -1;
}

int isIOsymbol(int c)
{
    return ((c == '>') || (c == '<'));
}

int spaceTab(int c)
{
    return ((c == '\t') || (c == ' '));
}

int spaceTabIOPipeline(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '<') || (c == '>') || (c == '|'));
}

int separ(int c)
{
    return ((c == ' ') || (c == '\t') || (c == '&') || (c == '>')
            || (c == '<') || (c == '|'));
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

int * makeEdges(pline * pipelist, int psize, int lsize)
{
    pline * tmp = pipelist;
    int i;
    int * arr = malloc((psize + 2) * sizeof(int));
    for(i = psize; i > 0; i--){
        arr[i] = tmp -> pos;
        tmp = tmp -> next;
    }
    arr[0] = 1;
    arr[psize + 1] = lsize + 1;
    /*for(i = 0; i < psize + 2; i++)
        printf("%d\n", arr[i]);*/
    return arr;
}

char ** makecurarr(char ** arr, int * edges, int iterator)
{
    int left, right, i;
    left = edges[iterator];
    right = edges[iterator + 1];
    char ** curarr = malloc((right - left + 1) * sizeof(*curarr));
    for(i = left; i < right; i++){
        curarr[i - left] = malloc(strlen(arr[i - 1]) + 1);
        strncpy(curarr[i - left], arr[i - 1], strlen(arr[i - 1]));
        curarr[i - left][strlen(arr[i - 1])] = 0;
    }
    curarr[right - left] = NULL;
    return curarr;
}

char ** makearr(list * headlist, int size, int * inOut)
{
    list * tmp = headlist;
    int i, j;
    int sizeIO = 0;
    for (i = 0; i < 3; i++)
        if(inOut[i] != 0)
            sizeIO++;
    if(inOut[outapp] > 0)
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

void execute(list * headlist, pline * pipelist, int mode, int * inOut)
{
    int j, k, pid;
    int size = listsize(headlist);
    int psize = listsize2(pipelist);
    int * edges = makeEdges(pipelist, psize, size);
    char ** arr = makearr(headlist, size, inOut);
    char ** curarr;
    int ** fd = malloc(sizeof(*fd) * (psize + 1));
    for(k = 0; k < psize + 1; k++)
        fd[k] = malloc(2 * sizeof(int));
    for(k = 0; k < psize + 1; k++){
        pipe(fd[k]);
        curarr = makecurarr(arr, edges, k);
        if(!strcmp(curarr[0], "cd"))
            changedir(curarr, size);/*to chnge size*/
        else{
            pid = fork();
            if(pid == 0){
                if(k != psize)
                    dup2(fd[k][1], 1);
                if(k != 0)
                    dup2(fd[k - 1][0], 0);
                close(fd[k][0]);
                execvp(curarr[0], curarr);
                perror(curarr[0]);
                exit(1);
            }
            close(fd[k][1]);
            if(k != 0)
                close(fd[k - 1][0]);
            if(mode == 0)
                while(wait(NULL) != pid)
                    ;
        }
    }
    for(j = 0; j < size; j++)/*to fix*/
        free(arr[j]);
    free(arr);
}

void reinit(list ** headlist, pline ** pipelist)
{
    freemem(*headlist);
    freemem2(*pipelist);
    init(headlist, pipelist);
}

void processinglast(int * iterator, char * buff, list ** headlist)
{
    if(*iterator != 0)
        *headlist = addtolist(*headlist, buff, *iterator);
    *iterator = 0;
}

int isWrongPipeline(list * head, pline * plist)
{
    int size = listsize(head);
    int errflag = 0;
    pline * tmp = plist;
    while(tmp != NULL){
        if(tmp -> pos > size)
            errflag = 1;
        tmp = tmp -> next;
    }
    return errflag;
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

void iscorrectcmd(int * flags, int chr, list ** head, pline * plist, int * inOut)
{
    int i;
    int mode = (flags[amperflag] == 1);
    if (flags[quoteflag] || (flags[amperflag] > 1) 
            || (flags[amperflag] == 1 && chr != '&') 
            || isWrongIO(*head, inOut) || isWrongPipeline(*head, plist))

        printf("incorrect input\n");
    else if((*head)->str != NULL){
        execute(*head, plist,  mode, inOut);
    }
    flags[quoteflag] = 0;
    flags[amperflag] = 0;
    for(i = 0; i < 3; i++)
        inOut[i] = 0;
}

void killbg()
{
    while(wait4(-1, NULL, WNOHANG, NULL) > 0)
        ;
}

void freememory(list * head, pline * pipe, char * buff)
{
    free(buff);
    freemem(head);
    freemem2(pipe);
    printf("\n");
}

int main()
{
    int c, lastchar = 0, lenbuff = 8, i = 0;
    int flags[2] = {0, 0};
    int inOutPos[3] = {0, 0, 0};
    char * buff = malloc(lenbuff);
    list * headlist = NULL;
    pline * pipelist = NULL;
    init(&headlist, &pipelist);
    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if ((!separ(c) && c != '\"') || (separ(c) && flags[quoteflag])){
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i++] = c;
            }
            else if (isIOsymbol(c))
                handleIOquote(lastchar, c, inOutPos, headlist, i);
            else if (c == '\"')
                flags[quoteflag] = !flags[quoteflag];
            else if (c == '&')
                flags[amperflag]++;
            else if (c == '|')
                handlePLine(c, &pipelist, headlist, i);
            if (i != 0 && !flags[quoteflag] && spaceTabIOPipeline(c)) {
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
            if(!spaceTab(c))
                lastchar = c;
        }
        else{
            processinglast(&i, buff, &headlist);
            iscorrectcmd(flags, lastchar, &headlist, pipelist, inOutPos);
            reinit(&headlist, &pipelist);
            killbg();
        }
    }
    freememory(headlist, pipelist, buff);
    return 0;
}

