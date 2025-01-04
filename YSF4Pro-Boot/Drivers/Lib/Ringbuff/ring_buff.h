#ifndef _RING_BUFF_H_
#define _RING_BUFF_H_

#include "common.h"
//#define RING_QUEUE_DEBUG //定义本宏会打印RingQueue的调试信息

#define RQ_OK 1
#define RQ_ERROR 0
#define RQ_OVERFLOW -2
typedef u_int8_t RQ_ElementType;//元素类型

typedef struct _ring_queue
{
	RQ_ElementType *elems;
	int size;
	volatile int front, rear;
}RingQueue;

//初始化队列,需传入保存队列状态的结构q，队列使用的buffer和buffer大小
int InitRingQueue(RingQueue *q, RQ_ElementType *buff, int size);

#define RingQueueFull(q) (((q)->rear+1) % (q)->size == (q)->front)
#define RingQueueEmpty(q) ((q)->front == (q)->rear)

//遍历队列, 
//消费者使用，故对生产者可能修改的rear先读取缓存
int ShowRingQueue(RingQueue *q);

//向队尾插入元素e
int InRingQueue(RingQueue *q,RQ_ElementType e);

//从队首删除元素
int OutRingQueue(RingQueue *q, RQ_ElementType *e);

//队列中的元素个数
int RingQueueLength(RingQueue *q);

#endif 
