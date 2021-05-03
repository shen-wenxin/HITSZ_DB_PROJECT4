
#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include <string.h>
#define TUPLE_NUM_IN_BLK 7 //一个blk里头tuple的个数
#define BLK_NUM_IN_BUFFER 8 //一个buffer里头blk的个数
#define R_START 1
#define R_END 16
#define S_START 17
#define S_END 48
#define TASK1_BLK_RESULT 100
#define TASK2_BLK_RESULT 200

struct Tuple{
    int X;
    int Y;
};

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
    printf("--------------------\n");
    printf("基于线性搜索的选择算法\n");
    printf("--------------------\n");
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
        printf("读入数据块%d\n",blk_id);
        if ((blk = readBlockFromDisk(blk_id, &buf)) == NULL){
            perror("Reading Block Failed!\n");
            return -1;
        }
        int C = -1,D = -1,addr = -1;
        char str[5];
        for (i = 0; i < TUPLE_NUM_IN_BLK; i++){
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + k);
            }
            C = atoi(str);
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + 4 + k);
            }
            D = atoi(str);
            if(C == 50  || (blk_id == S_END && i == 6)){
                if(C == 50){
                    printf("C = %d,D = %d\n",C,D);
                    for (int k = 0;k < 8;k ++){
                        *(blk_w + blk_w_num * 8 + k) = *(blk + i*8 + k);
                    }
                    blk_w_num ++;
                }

                if(blk_w_num == TUPLE_NUM_IN_BLK || (blk_id == S_END && i == 6)){
                    unsigned char str_blk_w_id[4];
                    int2str(str_blk_w_id, blk_w_id + 1);
                    for (int k = 0;k < 4;k ++){
                        *(blk_w + TUPLE_NUM_IN_BLK * 8 + k) = str_blk_w_id[k];
                    }
                    if (writeBlockToDisk(blk_w, blk_w_id, &buf) != 0){
                        perror("Writing Block Failed!\n");
                        return -1;
                    }
                    printf("结果写入磁盘:%d\n",blk_w_id);
                    blk_w_id++;
                    freeBlockInBuffer(blk_w, &buf);
                    blk_w_num  = 0;
                    blk_w = getNewBlockInBuffer(&buf); 
                    memset(blk_w,0,64);
                    
                }
                /* Write the block to the hard disk */
            }
        }
        freeBlockInBuffer(blk, &buf);
        
    }
    

    return 0;
}

int cmpTule(const void *p1, const void *p2){
    struct Tuple t1 = *(struct Tuple *)p1;
    struct Tuple t2 = *(struct Tuple *)p2;
    return t1.X - t2.X;

}
int tpmms_step1(int start,int end){
    //tpmms第一阶段：先将每个blk的数据读出来，按照第一个属性排序
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk_r[BLK_NUM_IN_BUFFER - 1];   //用于读
    int buffer_blk_num = 0; //   用来记录buffler中blk的数量
    for(int blk_id = start;blk_id <= end ;blk_id ++){
        printf("blk_id = %d\n",blk_id);
        if(buffer_blk_num < BLK_NUM_IN_BUFFER - 1){
            //要留一个blk的位置来写，先将东西读到buffer中
            blk_r[buffer_blk_num]=readBlockFromDisk(blk_id, &buf);
            buffer_blk_num ++;
        }
        else{
            for(int t = 0;t < buffer_blk_num;t ++){
                unsigned char *blk_r_temp = blk_r[t];
                int X = -1,Y = -1;
                char str[5];
                struct Tuple tuple[7];
                for (int i = 0; i < TUPLE_NUM_IN_BLK; i++){
                    //对于一个blk进行排序
                    for (int k = 0; k < 4; k++){
                        str[k] = *(blk_r_temp + i*8 + k);
                    }
                    X = atoi(str);
                    for (int k = 0; k < 4; k++){
                        str[k] = *(blk_r_temp + i*8 + 4 + k);
                    }
                    Y = atoi(str);
                    tuple[i].X = X;
                    tuple[i].Y = Y;
                }
                qsort(tuple,TUPLE_NUM_IN_BLK,sizeof(struct Tuple), cmpTule);
                //将排好序的写回到buffer中
                for(int i = 0;i < TUPLE_NUM_IN_BLK;i ++){
                    unsigned char str1[4];
                    unsigned char str2[4];
                    int2str(str1,tuple[i].X);
                    int2str(str2,tuple[i].Y);
                    for (int k = 0; k < 4; k++){
                        *(blk_r_temp + i*8 + k) = str1[k];
                        *(blk_r_temp + i*8 + k + 4) = str2[k];
                    }
                }

                
            }
            return 0;



                
                
            
            return 0;

        }



    }

    
        
    
    

    return 0;

}
int tpmms(int start,int end){
    tpmms_step1(start,end);

}

int main(){
   // LinerSearch(50);
   tpmms(R_START,R_END);

    return 0;
}