#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list{
    char* str;
    struct list* next;
}list;

char* extendbuff(char* buff, int lenbuff){
    char* newBuff = malloc(sizeof(char) * lenbuff * 2);
    lenbuff *=2;
    strncpy(newBuff, buff, lenbuff+1); 
    free(buff);
    return newBuff;
}

void printlist(list* head){
    if (head != NULL){ 
        if (head->next != NULL) 
            printf("%s\n", head->str);
        printlist(head->next);
    }
}

void freemem(list* headlist){
    if (headlist != NULL){
        if (headlist->str != NULL)
            free(headlist->str);
        list* tmp = headlist;
        headlist = headlist->next;
        free(tmp);
        freemem(headlist);
    }
    return;
}

list* addtolist(list* head, char* str, int lenbuff){
    list* tmp = malloc(sizeof(*tmp));
    tmp->str = malloc(sizeof(char) * (lenbuff+1));
    strncpy(tmp->str, str, lenbuff);
    tmp->str[lenbuff] = 0;
    tmp->next = head;
    return tmp;
}

int main(){
    int c;
    int lenbuff = 8;
    int i = 0;
    list* headlist = malloc(sizeof(*headlist));
    headlist->next = NULL;
    headlist->str = NULL;
    char* buff = malloc(sizeof(char) * lenbuff);
    while((c = getchar()) != EOF){
        if (c != '\n'){
            if (c != ' '){
                if(i >= lenbuff){
                    buff = extendbuff(buff, lenbuff);
                }
                buff[i] = c;
                i++;
            }
            else if (i != 0){
                headlist = addtolist(headlist, buff, i);
                i = 0;
            }
        }
        else{
            headlist = addtolist(headlist, buff, i);
            i = 0;
        }
    }
    free(buff);
    freemem(headlist);
    //printlist(headlist);
    return 0;
}
