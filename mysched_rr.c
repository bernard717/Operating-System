#include <linux/module.h>
#include "../sched.h"

#define TRUE 1
#define FALSE 0

extern void(*set_class_my) (struct task_struct *p);   //커널에서 선언한 함수 포인터

struct task_struct *p;

typedef struct _node {
	struct task_struct *p;
	struct _node *next;
} Node;

typedef struct lQueue {
	Node *front;
	Node *rear;
}LQueue;

LQueue pq;

int QIsEmpty(LQueue x) {
	if (x.front == NULL)
		return TRUE;
	else
		return FALSE;
}

static void
check_preempt_curr_fifo(struct rq *rq, struct task_struct *p, int flags)
{
	/* we're never preempted */
	printk("MYMOD: check_preempt_curr_fifo CALLED\n");
}

static struct task_struct *
pick_next_task_fifo(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
	//printk("MYMOD: pick_next_task_fifo CALLED");

	if (pq.front->p != NULL)
		p = pq.front->p;
	else
		p = fair_sched_class.pick_next_task(rq, prev, rf);

	if (p != NULL)
	{
		put_prev_task(rq, prev);
		printk("MYMOD: pick_next_task_fifo [prev: %u] [next: %u]\n", prev->pid, p->pid);
		return p;
	}

	return NULL;
}

static void
enqueue_task_fifo(struct rq *rq, struct task_struct *p, int flags)
{
	Node *newNode = (Node*)kmalloc(sizeof(Node), GFP_KERNEL);
	newNode->next = NULL;
	newNode->p = p;
	if (QIsEmpty(pq)) {
		pq.front = newNode;
		pq.rear = newNode;
	}
	else {
		pq.rear->next = newNode;
		pq.rear = newNode;
	}
	printk("MYMOD: enqueue_task_fifo CALLED\n");
	add_nr_running(rq, 1);
}

static void
dequeue_task_fifo(struct rq *rq, struct task_struct *p, int flags)
{
	Node *temp, *temp2;
	temp = pq.front;
	temp2 = NULL;

	while (p->pid != temp->p->pid) {
		temp2 = temp;
		temp = temp->next;
	}
	if (temp->next == NULL) {
		if (temp2 != NULL) {
			pq.rear = temp2;
			temp2->next = NULL;
			kfree(temp);
		}
		else {
			pq.front = NULL;
			pq.rear = NULL;
			kfree(temp);
		}
	}
	else {
		if (temp2 != NULL) {
			temp2->next = temp->next;
			kfree(temp);
		}
		else {
			pq.front = temp->next;
			kfree(temp);
		}
	}

	printk("MYMOD: dequeue_task_fifo CALLED\n");
	sub_nr_running(rq, 1);
}

static void yield_task_fifo(struct rq *rq)
{
	printk("MYMOD: yield_task_fifo CALLED\n");
	BUG();
}

static void put_prev_task_fifo(struct rq *rq, struct task_struct *prev)
{
	//printk("MYMOD: task_tick_fifo CALLED\n");
}

static void task_tick_fifo(struct rq *rq, struct task_struct *curr, int queued)
{
	Node *temp3;
	
	if (pq.front != pq.rear) {
		pq.rear->next = pq.front;
		temp3 = pq.front->next;
		pq.rear = pq.front;
		pq.front->next = NULL;
		pq.front = temp3;
	}
	//printk("MYMOD: task_tick_fifo CALLED\n");
}

static void set_curr_task_fifo(struct rq *rq)
{
	printk("MYMOD: set_curr_task_fifo CALLED\n");
}

static void switched_to_fifo(struct rq *rq, struct task_struct *new)
{
	printk("MYMOD: switched_to_fifo CALLED new = %p", new);
	p = new;
}

static void
prio_changed_fifo(struct rq *rq, struct task_struct *p, int oldprio)
{
	printk("MYMOD: prio_changed_fifo CALLED\n");
}

static unsigned int
get_rr_interval_fifo(struct rq *rq, struct task_struct *task)
{
	printk("MYMOD: get_rr_interval_fifo CALLED\n");
	return 0;
}

static void update_curr_fifo(struct rq *rq)
{
}

const struct sched_class my_sched_class =
{
	.next = &idle_sched_class,

	.enqueue_task = enqueue_task_fifo,
	.dequeue_task = dequeue_task_fifo,
	.yield_task = yield_task_fifo,

	.check_preempt_curr = check_preempt_curr_fifo,

	.pick_next_task = pick_next_task_fifo,
	.put_prev_task = put_prev_task_fifo,


	.set_curr_task = set_curr_task_fifo,
	.task_tick = task_tick_fifo,

	.get_rr_interval = get_rr_interval_fifo,

	.prio_changed = prio_changed_fifo,
	.switched_to = switched_to_fifo,
	.update_curr = update_curr_fifo,
};

void myclass(struct task_struct *p)
{
	printk("MYMOD: myclass CALLED\n");
	p->sched_class = &my_sched_class;
}

static int __init init_mysched(void)
{
	const struct sched_class *class;

	for_each_class(class)
		printk("INIT_MOD: class = %p\n", class);

	set_class_my = myclass;
	fair_sched_class.next = &my_sched_class;

	p = NULL;
	pq.front = NULL;
	pq.rear = NULL;

	for_each_class(class)
		printk(KERN_DEBUG "INIT_MOD: class = %p\n", class);

	return 0;
}

static void __exit exit_mysched(void)				
{
	const struct sched_class *class;

	p = NULL;
	set_class_my = NULL;
	fair_sched_class.next = &idle_sched_class;

	for_each_class(class)
		printk(KERN_DEBUG "EXIT_MOD: class = %p\n", class);

	return;
}

MODULE_AUTHOR("YoungNam Seo");
MODULE_DESCRIPTION("RR Scheduler");
MODULE_LICENSE("GPL");

module_init(init_mysched)
module_exit(exit_mysched)