#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "extmem.h"
#define min(x, y) (((x)<(y))?(x):(y))
typedef struct tuple {
    int x, y;
} tuple;
int tupleCmp(tuple a, tuple b) {
    return a.x - b.x;
}
void swapTuple(tuple *a, tuple *b) {
    a->x ^= b->x ^= a->x ^= b->x;
    a->y ^= b->y ^= a->y ^= b->y;
}
void intToStr(unsigned char *str, int x) {
    static unsigned char st[4];
    int cnt = 0;
    while (x) st[cnt++] = x % 10 + '0', x /= 10;
    while (cnt) *(str++) = st[--cnt];
}
int strToInt(const unsigned char *str) {
    int ret = 0;
    for (int i = 0; i < 4; i++) if (isdigit(str[i])) ret = (ret << 3) + (ret << 1), ret += str[i] - '0';
    return ret;
}
void blockStrToInt(unsigned char *str, int n) {
    int *p = (int *) str;
    for (int i = 0; i < n; i++, p++, str += 4) *p = strToInt(str);
}
void blockIntToStr(unsigned char *str, int n) {
    int *p = (int *) str;
    for (int i = 0; i < n; i++, p++, str += 4) intToStr(str, *p);
}
void printblock(const tuple *p, int n) {
    for (int i = 0; i < n; i++, p++) printf("%d %d\n", p->x, p->y);
}
int linearSearch(Buffer *buf, int l, int r, int query, int *freeCnt) {
    int cnt = 0, ioCnt = buf->numIO, stWriteCnt = *freeCnt;
    unsigned char *readBlk;
    tuple *p, *writeBlk = (tuple *) getNewBlockInBuffer(buf);
    memset(writeBlk, 0, buf->blkSize);
    for (int i = l; i <= r; i++) {
        if ((readBlk = readBlockFromDisk(i, buf)) == NULL) return perror("Reading Block Failed!\n"), -1;
        printf("读入数据块%d\n", i);
        blockStrToInt(readBlk, 14);
        p = (tuple *) readBlk;
        for (int j = 0; j < 7; j++) {
            if (p[j].x == query) writeBlk[cnt++] = p[j], printf("(C=%d,D=%d)\n", p[j].x, p[j].y);
            if (cnt == 7) {
                blockIntToStr((unsigned char *) writeBlk, 14);
                if (writeBlockToDisk((unsigned char *) writeBlk, *freeCnt, buf) != 0)
                    return perror("Writing Block Failed!\n"), -1;
                printf("写入数据块%d\n", *freeCnt);
                cnt = 0, (*freeCnt)++;
                writeBlk = (tuple *) getNewBlockInBuffer(buf);
                memset(writeBlk, 0, buf->blkSize);
            }
        }
        freeBlockInBuffer(readBlk, buf);
    }
    if (cnt != 0) {
        blockIntToStr((unsigned char *) writeBlk, cnt << 1);
        if (writeBlockToDisk((unsigned char *) writeBlk, *freeCnt, buf) != 0)
            return perror("Writing Block Failed!\n"), -1;
        printf("写入数据块%d\n", (*freeCnt)++);
    }
    freeBlockInBuffer((unsigned char *) writeBlk, buf);
    if (stWriteCnt != (*freeCnt))printf("注：写入磁盘:%d - %d\n", stWriteCnt, (*freeCnt) - 1);
    printf("满足条件的元组个数为%d\n", ((*freeCnt) - stWriteCnt) * 7 - cnt);
    printf("IO次数:%d\n", buf->numIO - ioCnt);
    return 0;
}
void maxHeapify(tuple *a, int l, int r) {
    int p = (l << 1) + 1;
    while (p <= r) {
        if (p + 1 <= r && tupleCmp(a[p], a[p + 1]) < 0) p++;
        if (tupleCmp(a[l], a[p]) > 0) return;
        else swapTuple(a + l, a + p), l = p, p = (p << 1) + 1;
    }
}
void heapSort(tuple *a, int n) {
    for (int i = (n >> 1) - 1; i >= 0; i--) maxHeapify(a, i, n - 1);
    for (int i = n - 1; i > 0; i--) swapTuple(a, a + i), maxHeapify(a, 0, i - 1);
}
int extrenMerge(Buffer *buf, int l, int r, int *freeCnt) {
    int st[7], blockCnt[7];
    int ioCnt = buf->numIO, cnt = 0, stWriteCnt = *freeCnt;
    tuple *readBlk[7], *writeBlk;
    int writecnt = 0;
    for (int i = l, dis = min(r - i + 1, 7); i <= r; i += dis, dis = min(r - i + 1, 7)) {
        st[cnt] = *freeCnt, blockCnt[cnt++] = dis;
        for (int j = 0; j < dis; j++) {
            printf("读入数据块%d\n", i + j);
            if ((readBlk[j] = (tuple *) readBlockFromDisk(i + j, buf)) == NULL)
                return perror("Reading Block Failed!\n"), -1;
            blockStrToInt((unsigned char *) readBlk[j], 14);
            heapSort(readBlk[j], 7);
        }
        writeBlk = (tuple *) getNewBlockInBuffer(buf);
        while (1) {
            int index = -1;
            tuple mn = (tuple) {9999, 9999};
            for (int j = 0; j < dis; j++) {
                if (readBlk[j]->y == 0) continue;
                if (tupleCmp(mn, *readBlk[j]) > 0)index = j, mn = *readBlk[j];
            }
            if (index == -1) break;
            writeBlk[writecnt++] = mn, readBlk[index]++;
            if (writecnt == 7) {
                blockIntToStr((unsigned char *) writeBlk, 14);
                if (writeBlockToDisk((unsigned char *) writeBlk, *freeCnt, buf) != 0)
                    return perror("Writing Block Failed!\n"), -1;
                printf("写入数据块%d\n", *freeCnt);
                writecnt = 0, (*freeCnt)++;
                writeBlk = (tuple *) getNewBlockInBuffer(buf);
                memset(writeBlk, 0, buf->blkSize);
            }
        }
        for (int j = 0; j < dis; j++) freeBlockInBuffer((unsigned char *) (readBlk[j] - 7), buf);
        freeBlockInBuffer((unsigned char *) writeBlk, buf);
//        printf("cnt:%d st:%d blockcnt:%d\n", cnt - 1, st[cnt - 1], blockCnt[cnt - 1]);
//        for (int j = st[cnt - 1]; j < st[cnt - 1] + blockCnt[cnt - 1]; j++) {
//            unsigned char *blk = readBlockFromDisk(j, buf);
//            blockStrToInt(blk, 14);
//            printblock((tuple *) blk, 7);
//            freeBlockInBuffer(blk, buf);
//        }
    }
    int ansSt = *freeCnt;
    for (int i = 0; i < cnt; i++) {
        printf("读入数据块%d\n", st[i]);
        readBlk[i] = (tuple *) readBlockFromDisk(st[i], buf);
        blockStrToInt((unsigned char *) readBlk[i], 14);
    }
    writeBlk = (tuple *) getNewBlockInBuffer(buf);
    while (1) {
        int index = -1;
        tuple mn = (tuple) {9999, 9999};
        for (int i = 0; i < cnt; i++) {
            if (blockCnt[i] == 0) continue;
            if (tupleCmp(mn, *readBlk[i]) > 0) index = i, mn = *readBlk[i];
        }
        if (index == -1) break;
        writeBlk[writecnt++] = mn, readBlk[index]++;
        if (readBlk[index]->y == 0) {
            freeBlockInBuffer((unsigned char *) (readBlk[index] - 7), buf);
            st[index]++, blockCnt[index]--;
            if (blockCnt[index] != 0) {
                printf("读入数据块%d\n", st[index]);
                readBlk[index] = (tuple *) readBlockFromDisk(st[index], buf);
                blockStrToInt((unsigned char *) readBlk[index], 14);
            }
        }
        if (writecnt == 7) {
            blockIntToStr((unsigned char *) writeBlk, 14);
            if (writeBlockToDisk((unsigned char *) writeBlk, *freeCnt, buf) != 0)
                return perror("Writing Block Failed!\n"), -1;
            printf("写入数据块%d\n", *freeCnt);
            writecnt = 0, (*freeCnt)++;
            writeBlk = (tuple *) getNewBlockInBuffer(buf);
            memset(writeBlk, 0, buf->blkSize);
        }
    }
    freeBlockInBuffer((unsigned char *) writeBlk, buf);
    puts("输出结果:");
    for (int i = ansSt; i < *freeCnt; i++) {
        unsigned char *blk = readBlockFromDisk(i, buf);
        blockStrToInt(blk, 14);
        printblock((tuple *) blk, 7);
        freeBlockInBuffer(blk, buf);
    }
    if (stWriteCnt != (*freeCnt))printf("注：写入磁盘:%d - %d\n", stWriteCnt, (*freeCnt) - 1);
    if (ansSt != (*freeCnt))printf("注：答案写入磁盘:%d - %d\n", ansSt, (*freeCnt) - 1);
    printf("IO次数:%d\n", buf->numIO - ioCnt);
    return 0;
}

int main(int argc, char **argv) {
    Buffer buf; /* A buffer */
    if (!initBuffer(520, 64, &buf)) return perror("Buffer Initialization Failed!\n"), -1;
    int freeCnt = 49;
    //linearSearch(&buf, 1, 16, 50, &freeCnt);
    //extrenMerge(&buf, 1, 16, &freeCnt);
    extrenMerge(&buf, 17, 48, &freeCnt);

    return 0;
}

