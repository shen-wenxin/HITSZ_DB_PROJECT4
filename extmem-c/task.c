
#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include <string.h>
#define TPMMS_STEP1_ELE_GROUT 7 //每一路里头的blk个数
#define TUPLE_NUM_IN_BLK 7 //一个blk里头tuple的个数
#define TUPLE_ELE_NUM 7
#define BLK_NUM_IN_BUFFER 8 //一个buffer里头blk的个数
#define R_START 1
#define R_END 16
#define S_START 17
#define S_END 48
#define TASK1_BLK_RESULT 100
#define TASK2_BLK_RESULT_STEP1 300
#define TASK2_BLK_RESULT_STEP2 350
#define True 1
#define False 0
struct Blk_ptr{
        int start;
        int end;
        int now;
        int next;
};
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

void get_min_data(int *blk_read_num, unsigned char *blk_r_ptr[BLK_NUM_IN_BUFFER - 1], Buffer *buf,struct Tuple *tuple,int pos,int buffer_blk_num){
    int minn_x = 9999,minn_Y = 9999;
    int s = -1;//表示其在第几块
    for(int i = 0;i < buffer_blk_num;i ++){
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
    printf("minn_x = %d,minn_y = %d,s = %d\n",minn_x,minn_Y,s);
    blk_read_num[s]  = blk_read_num[s] + 1;
    unsigned char *  ptr = blk_r_ptr[s];
    blk_r_ptr[s] = ptr + 8;
    tuple[pos].X = minn_x;
    tuple[pos].Y = minn_Y;
    
}


int isNotFinish(int blk_read_num[BLK_NUM_IN_BUFFER - 1],int buffer_blk_num){
    for(int i = 0;i < buffer_blk_num;i ++){
        if(blk_read_num[i] < TUPLE_NUM_IN_BLK){
            return True;
        }
    }
    return False;
}

int writeTuple2Dist(struct Tuple *tuple,int blk_id,Buffer *buf){
    printf("writeTuple2Dist\n");
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
    memset(blk_w,0,64);
    printf("结果写入磁盘:%d\n",blk_id);

}
int tpmms_step1(int start,int end,int rid_s,int *rid_e, Buffer *buf){
    unsigned char *blk_r[BLK_NUM_IN_BUFFER - 1];   //用于读
    unsigned char *blk_r_ptr[BLK_NUM_IN_BUFFER - 1];//读的首地址指针
    int blk_read_num[BLK_NUM_IN_BUFFER - 1];
    int buffer_blk_num = 0; //   用来记录buffler中blk的数量
    int result_blk_id = rid_s;
    for(int blk_id = start;blk_id <= end ;){
        if(buffer_blk_num < BLK_NUM_IN_BUFFER - 1){
            //要留一个blk的位置来写，先将东西读到buffer中
            // printf("write to bffer:blk_id = %d\n",blk_id);
            blk_r[buffer_blk_num]=readBlockFromDisk(blk_id, buf);
            blk_r_ptr[buffer_blk_num] = blk_r[buffer_blk_num];
            blk_read_num[buffer_blk_num] = 0;
            buffer_blk_num ++;
            blk_id ++;
             
        }
        if((buffer_blk_num == BLK_NUM_IN_BUFFER - 1) || (blk_id > end)){
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
                //对每一个blk排序
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
            while(isNotFinish(blk_read_num,buffer_blk_num)){
                get_min_data(blk_read_num,blk_r_ptr,buf,tuple,tup_pos,buffer_blk_num);
                tup_pos ++;
                if(tup_pos == TUPLE_ELE_NUM){
                    // for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                    //     printf("blk_read_num[%d] = %d\n",i,blk_read_num[i]);
                    // }
                    // for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                    //     printf("(%d,%d)\n",tuple[i].X,tuple[i].Y);
                    // }
                    //  将一个排满的tuple写到外存中
                    writeTuple2Dist(tuple,result_blk_id,buf);   
                    result_blk_id ++;
                    tup_pos = 0;
                    memset(tuple,0,sizeof(0));
                }
            }
            //处理完这一波之后，释放对buffer的占用，将buffer里头的内容清空
            for(int t = 0;t < buffer_blk_num;t ++){
                freeBlockInBuffer(blk_r[t],buf);
                memset(blk_r[t],0,64);
            }
            buffer_blk_num = 0;
        }
    }
    *(rid_e) = (result_blk_id - 1);
    return 0;

}
int initBlkPrt(int groupnum,int start,int end,struct Blk_ptr *blk_ptr){
    for(int i = 0;i < groupnum;i ++){
        blk_ptr[i].start = start + i * TPMMS_STEP1_ELE_GROUT;
        blk_ptr[i].end = blk_ptr[i].start + TPMMS_STEP1_ELE_GROUT - 1;
        if(blk_ptr[i].end > end)
            blk_ptr[i].end = end;
        blk_ptr[i].now = -1;
        blk_ptr[i].next = blk_ptr[i].start;
    }
}
int hasBlkInDisk(struct Blk_ptr *blk_ptr,int groupnum ){
    //检测dist是否还有blk没有读进来
    // printf("-----begin:hasBlkInDisk-----\n");
    for(int i = 0;i < groupnum;i ++){
        printf("blk_ptr[%d]:",i);
        printf("now = %d,next = %d, end = %d\n",blk_ptr[i].now,blk_ptr[i].next,blk_ptr[i].end);

    }
    for(int i = 0;i < groupnum;i ++){
        if(blk_ptr[i].now < blk_ptr[i].end){
            //还有块没读
            return True;
        }
    }
    return False;
}
int hasTupleInBlk(int * tulpe_read_num, int groupnum,struct Blk_ptr *blk_ptr){
    for(int i = 0;i < groupnum;i ++){
        if (tulpe_read_num[i] >= TUPLE_NUM_IN_BLK){
            if(blk_ptr[i].now < blk_ptr[i].end){
                return False;
            }
        }
    }
    return True;
}
void updataBlkInBuffer(int groupnum,int *tulpe_read_num,struct Blk_ptr *blk_ptr,Buffer *buf,unsigned char *blk_ptr_bffer[BLK_NUM_IN_BUFFER] ){
    for(int i = 0;i < groupnum;i ++){
        if((tulpe_read_num[i] == TUPLE_NUM_IN_BLK)&&(blk_ptr[i].next < blk_ptr[i].end)){
            blk_ptr[i].next ++;
            freeBlockInBuffer(blk_ptr_bffer[i],buf);
            memset(blk_ptr_bffer[i],0,sizeof(64));
        }
    }
}
int updateStopFlag( struct Blk_ptr *blk_ptr,int groupnum, int * tulpe_read_num){
    int stop_flag=False;
    for(int i = 0;i < groupnum;i ++){
        if(blk_ptr[i].now < blk_ptr[i].end){
            return False;
        }
        if(tulpe_read_num[i] < TUPLE_NUM_IN_BLK){
            return False;
        }
    }
    return True;
}
int tpmms_step2(int start,int end,Buffer *buf,int rid_s_2 , int *rid_e_2){
    //先统计结果中一共有多少个blk文件
    int num = end - start + 1;
    int result_blk_id = rid_s_2;
    int groupnum = (num % TPMMS_STEP1_ELE_GROUT) == 0?
                    (num / TPMMS_STEP1_ELE_GROUT):
                    (num / TPMMS_STEP1_ELE_GROUT + 1);
    struct Blk_ptr blk_ptr[groupnum];//指向内存块的每个block
    initBlkPrt(groupnum,start,end,blk_ptr);
    unsigned char * blk_ptr_bffer[groupnum];
    unsigned char * tuple_ptr_blk[TUPLE_NUM_IN_BLK];//记录buffer中的blk的tuple
    int tulpe_read_num[TUPLE_ELE_NUM];
    struct Tuple tuple[TUPLE_ELE_NUM];
    int tup_pos = 0;
    int stop_flag = 0;
    while(hasBlkInDisk(blk_ptr,groupnum)){
        //更新每一路
        for(int i = 0;i < groupnum;i ++){
            printf("%d:start:%d,end:%d,now:%d,next:%d\n",i,blk_ptr[i].start,blk_ptr[i].end,blk_ptr[i].now,blk_ptr[i].next);
            if(blk_ptr[i].now != blk_ptr[i].next){
                    printf("upload blk:%d\n",blk_ptr[i].next);
                    unsigned char * pptr = readBlockFromDisk(blk_ptr[i].next, buf);;
                    blk_ptr_bffer[i] = pptr;
                    blk_ptr[i].now = blk_ptr[i].next;
                    tuple_ptr_blk[i] = blk_ptr_bffer[i];
                    tulpe_read_num[i] = 0;
            }
        }
        //对着groupnum路开始归并排序

        while(hasTupleInBlk(tulpe_read_num,groupnum,blk_ptr)){
            
            get_min_data(tulpe_read_num,tuple_ptr_blk,buf,tuple,tup_pos,groupnum);
            tup_pos ++;
            if(tup_pos == TUPLE_ELE_NUM){
                for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                    printf("blk_read_num[%d] = %d\n",i,tulpe_read_num[i]);
                }
                for(int i = 0;i < TUPLE_ELE_NUM;i ++){
                    printf("(%d,%d)\n",tuple[i].X,tuple[i].Y);
                }
                //  将一个排满的tuple写到外存中
                printf("writeTuple2Dist\n");
                writeTuple2Dist(tuple,result_blk_id,buf); 
                stop_flag = updateStopFlag(blk_ptr,groupnum, tulpe_read_num);
                if(stop_flag)   return 0;
                result_blk_id ++;
                tup_pos = 0;
                memset(tuple,0,sizeof(0));
            }
        }
        updataBlkInBuffer(groupnum,tulpe_read_num,blk_ptr,buf,blk_ptr_bffer);
    }
    return 0;

}
int tpmms(int start,int end){
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int rid_s = TASK2_BLK_RESULT_STEP1,rid_e = TASK2_BLK_RESULT_STEP1;//step1 的结果存储的id的起始地址
    //第一阶段，按照每组7个的大小，将其读入buffer中，进行排序后写入结果中
    tpmms_step1(start,end,rid_s,&rid_e,&buf);
    //第二阶段：将上一阶段的结果继续归并排序
    printf("\n--------------------------------------------\n-----------------step2---------------\n");
    int rid_s_2 = TASK2_BLK_RESULT_STEP2, rid_e_2 = TASK2_BLK_RESULT_STEP2;
    tpmms_step2(rid_s,rid_e,&buf,rid_s_2,&rid_e_2);

}

int main(){
    // LinerSearch(50);
    // tpmms(S_START,S_END);

    return 0;
}