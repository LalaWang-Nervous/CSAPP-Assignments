#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

struct lineNode {
    struct lineNode *prev;
    struct lineNode *next;
    bool valid;
    long int flag;
};

struct group {
    struct lineNode* dummyHead;
    struct lineNode* dummyTail;
};

void moveToFirst(struct  lineNode* node, struct lineNode* head){
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
    node->prev = head;
}

void printHelp();
void getOpt(int argc, char **argv, int *v, int *s, int *E, int *b, char **path);
void init(int groupNum, struct group *groups[]);

int v=0,s=0,E=0,b=0;
char *path = NULL;
int hit_count=0, miss_count=0, eviction_count=0;
int main(int argc, char **argv)
{
    // 1-read the command line parameters.
    getOpt(argc,argv,&v,&s,&E,&b,&path);
    // 2-get the parameters for cache
    int groupNum = pow(2,s);
    long int groupIndexMask = (-1 >> (64 -s)) << b;
    long int flagMask = (-1 >> (s + b)) << (s + b);
    // 3-create cache and initialize, groups are organized as array and
    // the lines in group are organized as Linked list to apply LRU
    struct group *groups[groupNum];
    init(groupNum, groups);

    // 4-handle the instructions
    unsigned int address;
    char type[10];
    int size;
    FILE *tracefile = fopen(path,"r");
    while(fscanf(tracefile,"%s %x,%d",type,&address,&size) != EOF){
        if(strcmp(type,"I")==0){   // ignore the intruction load.
            continue;
        }

        // first get the group index and flag by this address (if cache exists)
        long int targetGroupIndex = ((address & groupIndexMask) >> b) % groupNum;
        long int flag = (address & flagMask) >> (s + b);
        struct group *targetGroup = groups[targetGroupIndex];
        struct lineNode* pointer = targetGroup->dummyHead->next;
        // search the linked list from head
        while(pointer != targetGroup->dummyTail) {
            if (pointer->valid == false) {
                // if find a invalid lineNode, means miss, and there still empty line
                // miss
                miss_count = miss_count + 1;
                // Modify = load + save, will hit at the save operation
                if (strcmp(type, "M") == 0)hit_count = hit_count + 1;
                // write an empty line and move it to first
                pointer->flag = flag;
                pointer->valid = true;
                moveToFirst(pointer, targetGroup->dummyHead);
                if (v == 1) {
                    if (strcmp(type, "M") == 0) {
                        printf("%s %x, %d miss hit\n", type, address, size);
                    } else {
                        printf("%s %x, %d miss\n", type, address, size);
                    }

                }
                break;
            } else {
                if (pointer->flag == flag) {
                    // if find a same flag, means hit
                    // hit
                    hit_count = hit_count + 1;
                    if (strcmp(type, "M") == 0)hit_count = hit_count + 1;
                    moveToFirst(pointer, targetGroup->dummyHead);
                    if (v == 1) {
                        printf("%s %x, %d hit\n", type, address, size);
                    }
                    break;
                } else {
                    // find a valid line but not match, so find next line
                    pointer = pointer->next;
                }
            }
        }

        // if find to the dummyTail, means this group is full but target address not cached
        // so find the last one lineNode to replace
        if(pointer == targetGroup->dummyTail) {
            // eviction
            miss_count = miss_count + 1;
            eviction_count = eviction_count + 1;
            if(strcmp(type,"M")==0){
                hit_count = hit_count + 1;
            }

            if(v==1) {
                if(strcmp(type,"M")==0){
                    printf("%s %x, %d miss eviction hit\n",type,address,size);
                } else {
                    printf("%s %x, %d miss eviction\n",type,address,size);
                }
            }
            pointer = pointer->prev;
            pointer->flag = flag;
            moveToFirst(pointer, targetGroup->dummyHead);
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

void init(int groupNum, struct group *groups[]) {
    for(int groupIndex=0;groupIndex<groupNum;groupIndex++){
        struct group *thisGroup= (struct group *)malloc(sizeof(struct group));
        thisGroup->dummyHead = (struct lineNode *) malloc(sizeof(struct lineNode));
        thisGroup->dummyTail = (struct lineNode *) malloc(sizeof(struct lineNode));
        thisGroup->dummyHead->next = thisGroup->dummyTail;
        thisGroup->dummyTail->prev = thisGroup->dummyHead;
        thisGroup->dummyHead->prev = NULL;
        thisGroup->dummyTail->next = NULL;
        for(int lineIndex=0;lineIndex<E;lineIndex++){
            struct lineNode *thisLineNode = (struct lineNode *)malloc(sizeof(struct lineNode));
            thisLineNode->valid = false;
            thisLineNode->flag = 0;
            // insert into the link
            thisLineNode->next = thisGroup->dummyHead->next;
            thisGroup->dummyHead->next->prev = thisLineNode;
            thisGroup->dummyHead->next = thisLineNode;
            thisLineNode->prev = thisGroup->dummyHead;
        }
        groups[groupIndex] = thisGroup;
    }
}



