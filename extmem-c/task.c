
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
#define TASK3_INDEX_S_START 400 //task3中索引表的起始位置
#define TASK3_S_RESULT 450      //task3中表R的结果的位置,450,451
#define TASK4_RESULT 500
#define TASK5_RESULT 600



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
struct Myptr{
    int blk_id;//blk_id标志第几个blk
    int pos_id;//pos_id标志这个blk里的第几个tuple
    int value;//里头的key值
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
        printf("(%d,%d)",tuple[i].X,tuple[i].Y);
    }
    printf("\n");
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
                if(stop_flag){
                    *(rid_e_2) = (result_blk_id - 1);
                    return 0;
                }   
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
void get_blk_first_tuple(unsigned char * ptr,struct Tuple *tuple,int tup_pos,int blk_id){
    int X = -1,Y = -1;
    char str[5];
    for (int k = 0; k < 4; k++){
        str[k] = *(ptr  + k);
    }
    X = atoi(str);
    for (int k = 0; k < 4; k++){
        str[k] = *(ptr + 4 + k);
    }
    Y = atoi(str);
    tuple[tup_pos].X = X;
    tuple[tup_pos].Y = blk_id;
    

}
void create_index(int start,int end,int rid_s,int *rid_e,Buffer *buf){
    struct Tuple tuple[TUPLE_ELE_NUM];
    int tup_pos = 0; 
    int result_blk_id = rid_s;
    for(int blk_id = start;blk_id <= end;blk_id ++){
        unsigned char * ptr = readBlockFromDisk(blk_id,buf);
        
        //得到这个Blk的第一个tuple的值
        get_blk_first_tuple(ptr,tuple,tup_pos,blk_id);
        freeBlockInBuffer(ptr,buf);
        tup_pos ++;
        if((tup_pos == TUPLE_NUM_IN_BLK) || (blk_id == end)){
            //将其写道那个里头去
            writeTuple2Dist(tuple,result_blk_id,buf);
            tup_pos = 0;
            memset(tuple,0,sizeof(tuple));
            result_blk_id ++;
        }
    }
    *(rid_e) = (result_blk_id - 1);

}
int get_large_blk_id(unsigned char * ptr,int key){
    int pre = -1;
    int C = -1,D = -1,addr = -1;
    char str[5]; 
    for (int i = 0; i < 7; i++){
        for (int k = 0; k < 4; k++){
            str[k] = *(ptr + i*8 + k);
        }
        C = atoi(str);
        for (int k = 0; k < 4; k++){
            str[k] = *(ptr + i*8 + 4 + k);
        }
        D = atoi(str);
        printf("(%d %d)",C,D);
        if(C >= key){
            //返回一个id值比它小的value的值
            for (int k = 0; k < 4; k++){
                str[k] = *(ptr + (i-1)*8 + 4 + k);
            }
            D = atoi(str);
            return D;
        }
    }
    return -1;
}
int select_with_index(int start,int end,int rid_s,int * rid_e, Buffer * buf,int key,int s_end){
    printf("------------------get here-------------\n");
    struct Tuple tuple[TUPLE_ELE_NUM];
    int tup_pos = 0;
    int result_blk_id = rid_s;
    for(int blk_id = start;blk_id <= end;blk_id ++){
        unsigned char * ptr = readBlockFromDisk(blk_id,buf);
        int id = get_large_blk_id(ptr, key);
        if(id != -1){
            printf("\nid = %d\n",id);
            freeBlockInBuffer(ptr,buf);
            memset(ptr,0,sizeof(ptr));
            //得到id值之后
            for(int sid = id;sid < s_end;sid ++){
                printf("\nread block = %d\n",sid);
                ptr = readBlockFromDisk(sid,buf);
                int C = -1,D = -1;
                char str[5]; 
                for (int i = 0; i < 7; i++){
                    for (int k = 0; k < 4; k++){
                        str[k] = *(ptr + i*8 + k);
                    }
                    C = atoi(str);
                    for (int k = 0; k < 4; k++){
                        str[k] = *(ptr + i*8 + 4 + k);
                    }
                    D = atoi(str);
                    printf("(%d %d)",C,D);
                    if(C == key){
                        tuple[tup_pos].X = C;
                        tuple[tup_pos].Y = D;
                        printf("\ntuple[%d] :(%d,%d)\n",tup_pos,tuple[tup_pos].X,tuple[tup_pos].Y);
                        tup_pos ++;
                        if(tup_pos == 7){
                            writeTuple2Dist(tuple,result_blk_id,buf);
                            result_blk_id ++;
                            tup_pos = 0;
                            memset(tuple,0,sizeof(tuple));
                        }
                    }
                    if(C> key){
                        writeTuple2Dist(tuple,result_blk_id,buf);
                        result_blk_id ++;
                        tup_pos = 0;
                        memset(tuple,0,sizeof(tuple));
                        return 0;

                    }
                }
            }
            return 0;
        }
        else{
            freeBlockInBuffer(ptr,buf);
            memset(ptr,0,sizeof(ptr));
        }

    }

}
int select_by_index(int start,int end){
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int rid_s = TASK3_INDEX_S_START,rid_e = TASK3_INDEX_S_START;
    create_index(start,end,rid_s,&rid_e,&buf);
    int rid_s_2 = TASK3_S_RESULT,rid_e_2 = TASK3_S_RESULT;
    select_with_index(rid_s,rid_e,rid_s_2,&rid_e_2,&buf,50,381);
}
int get_tuple_num(unsigned char * ptr,int i,int *X,int *Y){
    char str[5];
    for (int k = 0; k < 4; k++){
        str[k] = *(ptr + i*8 + k);
    }
    *X = atoi(str);
    for (int k = 0; k < 4; k++){
        str[k] = *(ptr + i*8 + 4 + k);
    }
    *Y = atoi(str);
}

int Sort_Merge_Join(int R_start,int R_end,int S_start,int S_end){
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int result_blk_id = TASK4_RESULT;
    struct Myptr pre_ptr;
    pre_ptr.blk_id = R_start;
    pre_ptr.pos_id = 0;
    pre_ptr.value = 0;
    struct Tuple tuple[7];
    int tup_pos = 0;
    int join = 0;
    for(int blk_s_id = S_start;blk_s_id <= S_end;blk_s_id ++){
        //对于S的每一个blk，都将其读入buffer中
        unsigned char * ptr = readBlockFromDisk(blk_s_id,&buf);
        int S_X = -1,S_Y = -1;
        for(int i = 0; i < TUPLE_NUM_IN_BLK;i ++){
            get_tuple_num(ptr,i,&S_X,&S_Y);
            // printf("S_X = %d,S_Y = %d\n",S_X,S_Y);
            //得到S里头的一个tuple
            //在R里头寻找
            int R_X = -1,R_Y = -1;
            for(int blk_r_id = R_start;blk_r_id <= R_end;blk_r_id++){
                unsigned char * ptr_r = readBlockFromDisk(blk_r_id,&buf);
                for(int pos_r_id = 0;pos_r_id < TUPLE_ELE_NUM;pos_r_id ++){
                    get_tuple_num(ptr_r,pos_r_id,&R_X,&R_Y);
                    // printf("R_X = %d,R_Y = %d\n",R_X,R_Y);
                    if(S_X < R_X){
                        break;
                    }
                    if(S_X == R_X){
                        //此时join
                        //将其写入数据库中
                        //更新ptr指针
                        join ++;
                        
                        if(pre_ptr.value != R_X){
                            pre_ptr.blk_id = blk_r_id;
                            pre_ptr.pos_id = pos_r_id;
                            pre_ptr.value = R_X;
                        }
                        tuple[tup_pos].X = S_X;
                        tuple[tup_pos].Y = S_Y;
                        tup_pos ++;
                        if(tup_pos == TUPLE_NUM_IN_BLK){
                        //将其写道那个里头去
                            writeTuple2Dist(tuple,result_blk_id,&buf);
                            tup_pos = 0;
                            memset(tuple,0,sizeof(tuple));
                            result_blk_id ++;
                        }
                        tuple[tup_pos].X = R_X;
                        tuple[tup_pos].Y = R_Y;
                        tup_pos ++;
                        if(tup_pos == TUPLE_NUM_IN_BLK){
                        //将其写道那个里头去
                            writeTuple2Dist(tuple,result_blk_id,&buf);
                            tup_pos = 0;
                            memset(tuple,0,sizeof(tuple));
                            result_blk_id ++;
                        }
                    }
                }
                //小的时候要写没有考虑
                if(S_X < R_X){
                    freeBlockInBuffer(ptr_r,&buf);
                    memset(ptr_r,0,sizeof(ptr_r));
                    break;
                }   
                freeBlockInBuffer(ptr_r,&buf);
                memset(ptr_r,0,sizeof(ptr_r));
            }
        }
        //处理完了，将这个读出来的扔掉
        freeBlockInBuffer(ptr,&buf);
        memset(ptr,0,sizeof(ptr));
        printf("\n");



    }
    if(tup_pos != 0){
        writeTuple2Dist(tuple,result_blk_id,&buf);
        tup_pos = 0;
        memset(tuple,0,sizeof(tuple));
        result_blk_id ++;

    }
    printf("join = %d\n",join);
    
}

int Sort_Merge_intersect(int R_start,int R_end,int S_start,int S_end){
    Buffer buf; 
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int result_blk_id = TASK5_RESULT;
    struct Myptr pre_ptr;
    pre_ptr.blk_id = R_start;
    pre_ptr.pos_id = 0;
    pre_ptr.value = 0;
    struct Tuple tuple[7];
    int tup_pos = 0;
    int join = 0;
    for(int blk_s_id = S_start;blk_s_id <= S_end;blk_s_id ++){
        //对于S的每一个blk，都将其读入buffer中
        unsigned char * ptr = readBlockFromDisk(blk_s_id,&buf);
        int S_X = -1,S_Y = -1;
        for(int i = 0; i < TUPLE_NUM_IN_BLK;i ++){
            get_tuple_num(ptr,i,&S_X,&S_Y);
            // printf("S_X = %d,S_Y = %d\n",S_X,S_Y);
            //得到S里头的一个tuple
            //在R里头寻找
            int R_X = -1,R_Y = -1;
            for(int blk_r_id = R_start;blk_r_id <= R_end;blk_r_id++){
                unsigned char * ptr_r = readBlockFromDisk(blk_r_id,&buf);
                for(int pos_r_id = 0;pos_r_id < TUPLE_ELE_NUM;pos_r_id ++){
                    get_tuple_num(ptr_r,pos_r_id,&R_X,&R_Y);
                    // printf("R_X = %d,R_Y = %d\n",R_X,R_Y);
                    if(S_X < R_X){
                        break;
                    }
                    if(S_X == R_X && S_Y == R_Y){
                        //此时join
                        //将其写入数据库中
                        //更新ptr指针
                        join ++;
                        
                        if(pre_ptr.value != R_X){
                            pre_ptr.blk_id = blk_r_id;
                            pre_ptr.pos_id = pos_r_id;
                            pre_ptr.value = R_X;
                        }
                        tuple[tup_pos].X = S_X;
                        tuple[tup_pos].Y = S_Y;
                        tup_pos ++;
                        if(tup_pos == TUPLE_NUM_IN_BLK){
                        //将其写道那个里头去
                            writeTuple2Dist(tuple,result_blk_id,&buf);
                            tup_pos = 0;
                            memset(tuple,0,sizeof(tuple));
                            result_blk_id ++;
                        }
                    }
                }
                //小的时候要写没有考虑
                if(S_X < R_X){
                    freeBlockInBuffer(ptr_r,&buf);
                    memset(ptr_r,0,sizeof(ptr_r));
                    break;
                }   
                freeBlockInBuffer(ptr_r,&buf);
                memset(ptr_r,0,sizeof(ptr_r));
            }
        }
        //处理完了，将这个读出来的扔掉
        freeBlockInBuffer(ptr,&buf);
        memset(ptr,0,sizeof(ptr));



    }
    if(tup_pos != 0){
        writeTuple2Dist(tuple,result_blk_id,&buf);
        tup_pos = 0;
        memset(tuple,0,sizeof(tuple));
        result_blk_id ++;

    }
    printf("join = %d\n",join);
    
}

int main(){
    // task1
    // LinerSearch(50);
    // task2
    // tpmms(S_START,S_END);
    // task3
    // select_by_index(350,381);//task2中2表的数据存在250~265的部分
    // task4
    // Sort_Merge_Join(250,265,350,381);
    // Sort_Merge_intersect(250,265,350,381);

    return 0;
}