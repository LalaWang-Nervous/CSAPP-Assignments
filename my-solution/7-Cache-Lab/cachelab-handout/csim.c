#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct line {
    bool valid;
    long int flag;
};

struct lineNode {
    struct lineNode *prev;
    struct lineNode *next;
    struct line *value;
};

struct group
{
    int validLineNum;
    struct lineNode* dummyHead;
    struct lineNode* dummyTail;
};

void update(struct  lineNode* node, struct lineNode* head){
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
    node->prev = head;
}

void printHelp();
void getOpt(int argc, char **argv, int *v, int *s, int *E, int *b, char **path);

int main(int argc, char **argv)
{
    int v=0,s=0,E=0,b=0;
    char *path = NULL;
    int hit_count=0, miss_count=0, eviction_count=0;
    getOpt(argc,argv,&v,&s,&E,&b,&path);

    // -----------------
    int groupNum = pow(2,s);
    long int groupIndexMask = (-1 >> (64 -s)) << b;
    long int flagMask = (-1 >> (s + b)) << (s + b);

    struct group *groups[groupNum];
    for(int groupIndex=0;groupIndex<groupNum;groupIndex++){
        struct group *thisGroup= (struct group *)malloc(sizeof(struct group));
        thisGroup->dummyHead = (struct lineNode *) malloc(sizeof(struct lineNode));
        thisGroup->dummyTail = (struct lineNode *) malloc(sizeof(struct lineNode));
        thisGroup->dummyHead->next = thisGroup->dummyTail;
        thisGroup->dummyTail->prev = thisGroup->dummyHead;
        thisGroup->dummyHead->prev = NULL;
        thisGroup->dummyTail->next = NULL;
        thisGroup->validLineNum = E;
        for(int lineIndex=0;lineIndex<E;lineIndex++){
            struct line *thisLine = (struct line *)malloc(sizeof(struct line));
            thisLine->valid = false;
            thisLine->flag = 0;
            //create linkNode
            struct lineNode *thisLineNode = (struct lineNode *) malloc(sizeof(struct lineNode));
            thisLineNode->value = thisLine;
            // insert into the link
            thisLineNode->next = thisGroup->dummyHead->next;
            thisGroup->dummyHead->next->prev = thisLineNode;
            thisGroup->dummyHead->next = thisLineNode;
            thisLineNode->prev = thisGroup->dummyHead;
        }
        groups[groupIndex] = thisGroup;
    }

    // -----------------
    unsigned int address;
    char type[10];
    int size;
    FILE *tracefile = fopen(path,"r");

    while(fscanf(tracefile,"%s %x,%d",type,&address,&size) != EOF){
        if(strcmp(type,"I")==0)continue;
        long int targetGroupIndex = (address & groupIndexMask) >> b;
        long int flag = (address & flagMask) >> (s + b);
        struct group *targetGroup = groups[targetGroupIndex];
        struct lineNode* pointer = targetGroup->dummyHead->next;
        while(pointer != targetGroup->dummyTail) {
            if ((pointer->value->valid) && (pointer->value->flag == flag)) {
                // hit
                hit_count = hit_count + 1;
                update(pointer, targetGroup->dummyHead);
                if (v == 1) {
                    printf("%s %x, %d hit\n",type,address,size);
                }
                break;
            } else {
                if (pointer->value->valid == false) {
                    // miss
                    miss_count = miss_count + 1;
                    if (strcmp(type,"M")==0)hit_count = hit_count + 1;
                    pointer->value->flag = flag;
                    pointer->value->valid = true;
                    update(pointer, targetGroup->dummyHead);
                    if (v == 1) {
                        if (strcmp(type,"S")==0) {
                            printf("%s %x, %d miss\n",type,address,size);
                        } else {
                            printf("%s %x, %d miss hit\n",type,address,size);
                        }
                    }
                    break;
                } else {
                    pointer = pointer->next;
                }
            }

        }

        if(pointer == targetGroup->dummyTail) {
            // eviction
            miss_count = miss_count + 1;
            eviction_count = eviction_count + 1;
            if(strcmp(type,"M")==0){
                hit_count = hit_count + 1;
            }

            if(v==1) {
                if(strcmp(type,"S")==0){
                    printf("%s %x, %d miss eviction\n",type,address,size);
                } else {
                    printf("%s %x, %d miss eviction hit\n",type,address,size);
                }
            }
            pointer = pointer->prev;
            pointer->value->flag = flag;
            update(pointer, targetGroup->dummyHead);
        }
    }
    fclose(tracefile);
    free(path);

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

void printHelp() {
    printf("-h: Optional help flag that prints usage info.\n");
    printf("-v: Optional verbose flag that displays trace info.\n");
    printf("-s <s>: Number of set index bits (S = 2 s is the number of sets)\n");
    printf("-E <E>: Associativity (number of lines per set).\n");
    printf("-b <b>: Number of block bits (B = 2 b is the block size).\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n");
}


void getOpt(int argc, char **argv, int *v, int *s, int *E, int *b, char **path) {
    printf("begin to process paras input.\n");
    int opt = 0;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        printf("opt is %c,  ", (char) opt);
        switch (opt) {
            case 'h':
                printHelp();
                break;
            case 'v':
                printf("begin to process input v..\n");
                *v = 1;
                printf("begin to process input v done.\n");
                break;
            case 's':
                printf("begin to process input s..\n");
                *s = atoi(optarg);
                break;
            case 'E':
                printf("begin to process input E..\n");
                *E = atoi(optarg);
                break;
            case 'b':
                printf("begin to process input b..\n");
                *b = atoi(optarg);
                break;
            case 't':
                printf("begin to process input t..\n");
                *path = (char *)malloc((sizeof(char) * (strlen(optarg) + 1)));
                strcpy(*path, optarg);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }
}



