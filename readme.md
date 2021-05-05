# 数据库实验四

*工程文档 2021-5-6*

*提供简单讲解与接口说明，具体请阅读源码，关键部分已附上注释*

*注：这个项目是一个赶出来的产物，赶出来的，赶出来的，赶出来的*

## 实验题目

借助所提供的 Extmem 库，模拟实现一下四种数据库查询算法：

1. 实现基于线性搜索的关系选择算法
2. 实现两阶段多路归并排序算法 
3. 实现基于索引的关系选择算法 
4. 基于排序的连接操作算法 
5. 基于排序的两趟扫描算法

## 文件结构

```
--extmem-c
	--bin
	--data
	--obj
	--extmem.c
	...
	--task.c	# 主要完成部分
	
```

## 核心代码

主函数：

```C++
int main(){
    // task1:实现基于线性搜索的关系选择算法
     LinerSearch(50);
    // task2:实现两阶段多路归并排序算法
     tpmms(S_START,S_END);
    // task3:实现基于索引的关系选择算法
     select_by_index(350,381);//task2中2表的数据存在250~265的部分
    // task4:基于排序的连接操作算法
     Sort_Merge_Join(250,265,350,381);
    // task5:基于排序的两趟扫描算法
    Sort_Merge_intersect(250,265,350,381);
    return 0;
}
```

### 实现基于线性搜索的关系选择算法

#### LinerSearch()

##### 函数功能

​	task1的主函数。

##### 传入参数

·key -int 表示需要匹配的 ``key``值

### 实现两阶段多路归并排序算法

### tpmms()

##### 函数功能

​	task2的主函数。

##### 传入参数

·start-int 待排序的Blk的起始id号

·end-int 待排序的Blk的结束id号

####  tpmms_step1()

##### 函数功能

​	task2第一阶段的函数,将待排序的Blk分组，然后在组内将其排为有序。

##### 传入参数

·start-int 待排序的Blk的起始id号

·end-int 待排序的Blk的结束id号

·buf-Buffer

·rid_s-int 第一阶段结果的起始id号

##### 传出参数

·rid_e-int 第一阶段结果的结束id号

#### tpmms_step2()

##### 函数功能

​	task2第二阶段函数，利用归并排序将所有组的blk排为整体有序。

##### 传入参数

·start-int 待排序的Blk的起始id号

·end-int 待排序的Blk的结束id号

·buf-Buffer

·rid_s_2-int 有序的Blk的起始id号

##### 传出参数

·rid_e_2-int 有序的Blk的结束id号

### 实现基于索引的关系选择算法

#### select_by_index()

##### 函数功能

​	task3的主函数。

##### 传入参数

·start-int 待选择的Blk的起始id号

·end-int 待选择的Blk的结束id号

#### create_index()

##### 函数功能

​	建立索引的函数。

##### 传入参数

·start-int 待建立索引的Blk的起始id号

·end-int 待建立索引的Blk的结束id号

·buf-Buffer

·rid_s_2-int 索引结果起始id号

##### 传出参数

·rid_e_2-int 索引结果结束id号

#### select_with_index()

##### 函数功能

​	根据索引选择符合条件的tuple

##### 传入参数

·start-int 索引的Blk的起始id号

·end-int 索引的Blk的结束id号

·buf-Buffer

·rid_s_2-int 结果起始id号

·key-int 待匹配的key值

##### 传出参数

·rid_e_2-int 结果结束id号

### 基于排序的连接操作算法

#### Sort_Merge_Join()

##### 函数功能

​	在排好序的blk的基础上进行连接操作

##### 传入参数

·R_start-int R表起始id号

·R_end-int R表结束id号

·S_start-int S表起始id号

·S_end-int S表结束id号

### 基于排序的两趟扫描算法

#### Sort_Merge_intersect()

##### 函数功能

​	在排好序的blk的基础上进行并操作

##### 传入参数

·R_start-int R表起始id号

·R_end-int R表结束id号

·S_start-int S表起始id号

·S_end-int S表结束id号

## 其他函数

#### int2str()

##### 函数功能

​	将Int值转为写入Blk所需要的str

##### 传入参数

·str-unsigned char * 

·a -Int

#### writeTuple2Dist()

##### 函数功能

​	将Tuple数组中存的值写入blk中

##### 传入参数

·tuple-struct Tuple 

·blk_id -Int

·buf -Buffer