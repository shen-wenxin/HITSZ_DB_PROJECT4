
#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#define R_START 1
#define R_END 16
#define S_START 17
#define S_END 48
#define TASK1_BLK_RESULT 100

void int2str(unsigned char *dst, int a) {
    int base = 1000;
    for (int i = 0; i < 4; i++) {
        dst[i] = a / base + '0';
        a %= base;
        base /= 10;
    }
}

int LinerSearch(int key){
    //任务一：基于线性搜索的关系选择算法，实现线性搜索(S.C,S.D)where S.C = 50
    Buffer buf; 
    unsigned char *blk;
    unsigned char *blk_w;   //用于写
    int blk_w_num = 0;  //统计写的blk中有几个，有7个之后就要写到外存中
    int blk_w_id = TASK1_BLK_RESULT;
    int i = 0;
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    
    blk_w = getNewBlockInBuffer(&buf); 

    for(int blk_id = S_START;blk_id <= S_END;blk_id ++){
        if ((blk = readBlockFromDisk(blk_id, &buf)) == NULL){
            perror("Reading Block Failed!\n");
            return -1;
        }
        printf("blk_id = %d\n",blk_id);
        int C = -1,D = -1,addr = -1;
        char str[5];
        for (i = 0; i < 7; i++){
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + k);
            }
            C = atoi(str);
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + 4 + k);
            }
            D = atoi(str);
            printf("(%d,%d)\n",C,D);
            if(C == 50  || (blk_id == S_END && i == 6)){
                if(C == 50){
                    for (int k = 0;k < 8;k ++){
                        *(blk_w + blk_w_num * 8 + k) = *(blk + i*8 + k);
                    }
                    blk_w_num ++;
                }
                
                printf("blk_w_num = %d\n",blk_w_num);
                if(blk_w_num == 7 || (blk_id == S_END && i == 6)){
                    unsigned char str_blk_w_id[4];
                    int2str(str_blk_w_id, blk_w_id + 1);
                    for (int k = 0;k < 4;k ++){
                        *(blk_w + 7 * 8 + k) = str_blk_w_id[k];
                    }
                    if (writeBlockToDisk(blk_w, blk_w_id, &buf) != 0){
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    blk_w_id++;
                    freeBlockInBuffer(blk_w, &buf);
                    blk_w_num  = 0;
                    blk_w = getNewBlockInBuffer(&buf); 
                    memset(blk_w,0,64);
                    printf("get new blk\n");
                    // int CC,DD;
                    // for(int v = 0;v < 7;v ++){
                    //     for(int b = 0;b < 4;b ++){
                    //         str[b] = *(blk_w + v*8 + b);
                    //     }
                    //     CC = atoi(str);
                    //     for(int b = 0;b < 4;b ++){
                    //         str[b] = *(blk_w + i*8 + 4 + b);
                    //     }
                    //     DD = atoi(str);
                    //     printf("CC(%d,%d)\n",CC,DD);
                    // }
                    
                }
                /* Write the block to the hard disk */
            }
        }
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + k);
        }
        addr = atoi(str);
        printf("\nnext address = %d \n", addr);
        freeBlockInBuffer(blk, &buf);
        printf("\n");
        printf("IO's is %d\n", buf.numIO); /* Check the number of IO's */
        
    }
    

    return 0;
}


int main(){
    LinerSearch(50);
    return 0;
}