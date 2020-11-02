#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

enum Flags {
    QUOTEFLAG,
    AMPERFLAG
};

enum InOut {
    IN,
    OUT,
    OUTAPP
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
    //if(pipelist == NULL) printf("dffefd\n");
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
    //printf("%d\n", tmp -> pos);
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
    if(inOut[IN]){
        fd = open(getFilename(inOut[IN], head), O_RDONLY | O_CREAT);
        if(fd != -1)
            dup2(fd, 0);
        else
            errflag = 1;
    }
    if(inOut[OUTAPP]){
        fd = open(getFilename(inOut[OUTAPP], head),O_WRONLY | O_APPEND | O_CREAT, 0666);
        if(fd != -1)
            dup2(fd, 1);
        else
            errflag = 1;
    }
    if(inOut[OUTAPP] == 0 && inOut[OUT]){
        fd = open(getFilename(inOut[OUT], head), O_WRONLY | O_CREAT, 0666);
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
        if(inOut[IN] == 0)
            inOut[IN] = size;
        else
            inOut[IN] = -1;
    else if(c == '>' && lastc == '>')
        if(inOut[OUTAPP] == 0)
            inOut[OUTAPP] = size;
        else
            inOut[IN] = -1;
    else if(c == '>')
        if(inOut[OUT] == 0)
            inOut[OUT] = size;
        else
            inOut[IN] = -1;
    else
        inOut[IN] = -1;
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

int separator(int c)
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

int * makeEdges(pline * pipelist, int psize)
{
    pline * tmp = pipelist;
    int i;
    int * arr = malloc((psize + 1) * sizeof(int));
    for(i = psize; i > 0; i--){
        arr[i] = tmp -> pos;
        tmp = tmp -> next;
    }
    arr[0] = 1;
    for(i = 0; i < psize + 1; i++)
        printf("%d\n", arr[i]);
    return arr;
}

char ** makearr(list * headlist, int size, int * inOut)
{
    list * tmp = headlist;
    int i, j;
    int sizeIO = 0;
    for (i = 0; i < 3; i++)
        if(inOut[i] != 0)
            sizeIO++;
    if(inOut[OUTAPP] > 0)
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
    //printf("%d ", pipelist -> pos);
    int psize = listsize2(pipelist);
    int * edges = makeEdges(pipelist, psize);
    char ** arr = makearr(headlist, size, inOut);
    int ** fd = malloc(sizeof(*fd) * (psize + 1));
    for(k = 0; k < psize + 1; k++)
        fd[k] = malloc(2 * sizeof(int));
    for(k = 0; k < psize + 1; k++){
        pipe(fd[k]);
        if(!strcmp(arr[0], "cd"))
            changedir(arr, size);
        else{
            pid = fork();
            if(pid == 0){
                /*if(!changeIO(headlist, inOut)){
                    execvp(arr[0], arr);
                }*/
                dup2(1, fd[k][1]);
                if(k != 0)
                    dup2(0, fd[k - 1][0]);
                close(fd[k][0]);
                /**/
                execvp(arr[0], arr);
                perror(NULL);
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
    for(j = 0; j < size; j++)
        free(arr[j]);
    free(arr);
}

void reinit(list ** headlist, pline ** pipelist)
{
    freemem(*headlist);
    freemem2(*pipelist);
    init(headlist, pipelist);
}

void processinglast(int * iterator, char * buff, list **headlist)
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
    int mode = (flags[AMPERFLAG] == 1);
    if (flags[AMPERFLAG] || (flags[AMPERFLAG] > 1) || (flags[AMPERFLAG] == 1 && chr != '&')
        || isWrongIO(*head, inOut) || isWrongPipeline(*head, plist))

        printf("incorrect input\n");
    else if((*head)->str != NULL){
        /*printRecurs2(plist);*/
        execute(*head, plist,  mode, inOut);
        /*printlist(*head);*/
    }
    flags[QUOTEFLAG] = 0;
    flags[AMPERFLAG] = 0;
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
            if ((!separator(c) && c != '\"') || (separator(c) && flags[QUOTEFLAG])){
                if (i >= lenbuff - 1)
                    buff = extendbuff(buff, &lenbuff);
                buff[i++] = c;
            }
            else if (isIOsymbol(c))
                handleIOquote(lastchar, c, inOutPos, headlist, i);
            else if (c == '\"')
                flags[QUOTEFLAG] = !flags[QUOTEFLAG];
            else if (c == '&')
                flags[AMPERFLAG]++;
            else if (c == '|')
                handlePLine(c, &pipelist, headlist, i);
            if (i != 0 && !flags[QUOTEFLAG] && spaceTabIOPipeline(c)) {
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
