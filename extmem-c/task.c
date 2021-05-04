
#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include <string.h>
#define TUPLE_NUM_IN_BLK 7 //一个blk里头tuple的个数
#define TUPLE_ELE_NUM 7
#define BLK_NUM_IN_BUFFER 8 //一个buffer里头blk的个数
#define R_START 1
#define R_END 16
#define S_START 17
#define S_END 48
#define TASK1_BLK_RESULT 100
#define TASK2_BLK_RESULT 200
#define True 1
#define False 0

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

void get_min_data(int *blk_read_num, unsigned char *blk_r_ptr[BLK_NUM_IN_BUFFER - 1], Buffer buf,struct Tuple *tuple,int pos){
    int minn_x = 9999,minn_Y = 9999;
    int s = -1;//表示其在第几块
    for(int i = 0;i < BLK_NUM_IN_BUFFER - 1;i ++){
        if(blk_read_num[i] == 7){
            continue;//这个列已经被读完了
        }
        unsigned char *  ptr = blk_r_ptr[i];
        int X = -1,Y = -1;
        char str[5];
        struct Tuple tuple[TUPLE_ELE_NUM];
        //对于一个blk进行排序
        for (int k = 0; k < 4; k++){
            str[k] = *(ptr + k);
        }
        X = atoi(str);
        for (int k = 0; k < 4; k++){
            str[k] = *(ptr + 4 + k);
        }
        Y = atoi(str);
        if(minn_x > X){
            minn_x = X;
            minn_Y = Y;
            s = i;
        }
    }
    printf("minn = %d,s = %d\n",minn_x,s);
    blk_read_num[s]  = blk_read_num[s] + 1;
    unsigned char *  ptr = blk_r_ptr[s];
    blk_r_ptr[s] = ptr + 8;
    tuple[pos].X = minn_x;
    tuple[pos].Y = minn_Y;
    
}


int isNotFinist(int blk_read_num[BLK_NUM_IN_BUFFER - 1]){
    for(int i = 0;i < BLK_NUM_IN_BUFFER - 1;i ++){
        if(blk_read_num[i] < TUPLE_NUM_IN_BLK){
            return True;
        }
    }
    return False;
}

int writeTuple2Dist(struct Tuple *tuple,int blk_id,Buffer *buf){
    unsigned char *blk_w;   //用于写
    blk_w = getNewBlockInBuffer(buf); 
    unsigned char str_blk_w_id[4];
    int2str(str_blk_w_id, blk_id + 1);//下一个地址
    for(int i = 0;i < TUPLE_ELE_NUM;i ++){
        unsigned char str1[4];
        unsigned char str2[4];
        int2str(str1,tuple[i].X);//X的值
        int2str(str2,tuple[i].Y);//Y的值
        for (int k = 0;k < 4;k ++){
            *(blk_w + i * 8 + k) = str1[k];
        }
        for (int k = 0;k < 4;k ++){
            *(blk_w + i * 8 + k + 4) = str2[k];
        }

    }
    for (int k = 0;k < 4;k ++){
        *(blk_w + TUPLE_ELE_NUM * 8 + k) = str_blk_w_id[k];
    }
    if (writeBlockToDisk(blk_w, blk_id, buf) != 0){
        perror("Writing Block Failed!\n");
        return -1;
    }
    freeBlockInBuffer(blk_w, buf);
    printf("结果写入磁盘:%d\n",blk_id);

}
int tpmms_step1(int start,int end){
    //tpmms第一阶段：先将每个blk的数据读出来，按照第一个属性排序
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk_r[BLK_NUM_IN_BUFFER - 1];   //用于读
    unsigned char *blk_r_ptr[BLK_NUM_IN_BUFFER - 1];//读的首地址指针
    int blk_read_num[BLK_NUM_IN_BUFFER - 1];
    int buffer_blk_num = 0; //   用来记录buffler中blk的数量
    int result_blk_id = TASK2_BLK_RESULT;
    for(int blk_id = start;blk_id <= end ;){
        if(buffer_blk_num < BLK_NUM_IN_BUFFER - 1){
            //要留一个blk的位置来写，先将东西读到buffer中
            blk_r[buffer_blk_num]=readBlockFromDisk(blk_id, &buf);
            blk_r_ptr[buffer_blk_num] = blk_r[buffer_blk_num];
            blk_read_num[buffer_blk_num] = 0;
            buffer_blk_num ++;
            blk_id ++;
             
        }
        else{
            //将buffer里头的内容变为以blk为单位的有序，升序排序，最小的在最前面
            for(int t = 0;t < buffer_blk_num;t ++){
                unsigned char *blk_r_temp = blk_r[t];
                int X = -1,Y = -1;
                char str[5];
                struct Tuple tuple[TUPLE_ELE_NUM];
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
            //开始第一轮归并排序
            struct Tuple tuple[TUPLE_ELE_NUM];
            int tup_pos = 0;
            //一边排序一边将排序好的Blk写入磁盘中
            while(isNotFinist(blk_read_num)){
                get_min_data(blk_read_num,blk_r_ptr,buf,tuple,tup_pos);
                tup_pos ++;
                if(tup_pos == TUPLE_ELE_NUM){
                    for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                        printf("blk_read_num[%d] = %d\n",i,blk_read_num[i]);
                    }
                    for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                        printf("(%d,%d)\n",tuple[i].X,tuple[i].Y);
                    }
                    //  将一个排满的tuple写到外存中
                    writeTuple2Dist(tuple,result_blk_id,&buf);   
                    result_blk_id ++;
                    tup_pos = 0;
                    memset(tuple,0,sizeof(0));
                }
            }
            //处理完这一波之后，释放对buffer的占用，将buffer里头的内容清空
            for(int t = 0;t < buffer_blk_num;t ++){
                freeBlockInBuffer(blk_r[t],&buf);
            }
            buffer_blk_num = 0;
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