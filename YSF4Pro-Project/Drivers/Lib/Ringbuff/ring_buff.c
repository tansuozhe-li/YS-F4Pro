
//循环队列
#include <stdio.h>
#include <stdlib.h>
#include "ring_buff.h"

//#define RING_QUEUE_DEBUG

//#ifdef RING_QUEUE_DEBUG
//#define //rq_debug term_printf
//#else
//void rq_debug(const char *fmt, ...){};
//#endif

//初始化队列
int InitRingQueue(RingQueue *q, RQ_ElementType *buff, int size)
{
	q->elems = buff;
	q->size = size;
	q->front=q->rear=0;
	return RQ_OK;
}

//遍历队列, 
//消费者使用，故对生产者可能修改的rear先读取缓存
int ShowRingQueue(RingQueue *q)
{
	//int i;
	int rear = q->rear;

	if(q->front == rear)
	{
		//rq_debug("队列为空\n");
		return RQ_ERROR;
	}
	//rq_debug("队列中的元素为:\n");
	//for(i=((q->front)%q->size); i !=  rear; i=((i+1)%q->size))
		//rq_debug(" %c\n",q->elems[i]);

	//rq_debug("\n");
	//rq_debug("队首元素为%c\n",q->elems[q->front]);
	//rq_debug("队尾元素为%c\n",q->elems[rear - 1]);
	return RQ_OK;
}

//向队尾插入元素e
int InRingQueue(RingQueue *q,RQ_ElementType e)
{
	if(RingQueueFull(q))
	{
		//rq_debug("空间不足\n");
		return(RQ_OVERFLOW);
	}
	q->elems[q->rear] = e;
	q->rear = (q->rear+1) % q->size;
	//rq_debug("rear = %d\n",q->rear);
	return RQ_OK;
}

//从队首取回并删除元素
int OutRingQueue(RingQueue *q, RQ_ElementType *e)
{
	if(RingQueueEmpty(q))
	{
		//rq_debug("队列为空\n");
		return RQ_ERROR;
	}
        *e = q->elems[q->front];
	//rq_debug("被删除的队首元素为%c\n",q->elems[q->front]);
	q->front = (q->front+1) % q->size;
	return RQ_OK;
}

//队列中的元素个数
int RingQueueLength(RingQueue *q)
{
	return ((q->rear - q->front) + q->size) % q->size;
}

