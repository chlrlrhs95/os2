#include "sched.h"
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/slab.h>

void init_task_myprio(struct task_struct *p)
{
	struct sched_myprio_entity *se = &p->myprio;

	p->sched_class = &myprio_sched_class;
	se->age = 0;
	se->prev_tp = 0;
	printk(KERN_INFO "***[MYPRIO] init_task_myprio: priority=%d, p->pid=%d\n, age=%d",
				se->prio, p->pid, se->age);
}

void init_myprio_rq (struct myprio_rq *myprio_rq)
{
	printk(KERN_INFO "***[MYPRIO] Mypriority class is online \n");
	myprio_rq->nr_running = 0;
	myprio_rq->totalprio = 0;
	INIT_LIST_HEAD(&myprio_rq->queue);
}

static void update_curr_myprio(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct task_struct *p = NULL;
	struct myprio_rq *myprio_rq = &rq->myprio;
	struct list_head *pos;
	struct sched_myprio_entity *se = NULL;
	int i;
	printk(KERN_INFO "***[MYPRIO] update_curr: update_curr starting!\n");
		
	// update curr tasks' pass and make myprio_rq sorted by pass
	
	pos = &myprio_rq->queue;

	for(i=0; i<myprio_rq->nr_running; i++){
		pos = pos->next;
		se = container_of(pos, struct sched_myprio_entity, run_list);
		p = container_of(se, struct task_struct, myprio);
		se->age += se->prio;
		printk(KERN_INFO "\t\t***[MYPRIO] update_curr: curr->pid=%d, p->pid=%d,nr_running=%d ,prio=%d, age=%d, prev_total=%d\n",
							curr->pid, p->pid,myprio_rq->nr_running ,se->prio, se->age,se->prev_tp);
			
	}
	/*
	}*/
	printk(KERN_INFO "***[MYPRIO] update_curr: update_curr end!\n");
	resched_curr(rq);
	//  resched
}

static void enqueue_task_myprio(struct rq *rq, struct task_struct *p, int flags)
{
	struct myprio_rq *myprio_rq = &rq->myprio;
	struct sched_myprio_entity *front_se = NULL;

	struct list_head *pos = NULL;
	struct task_struct *pos_p = NULL;
	struct sched_myprio_entity *pos_se = NULL;
	
	int is_enqueued=0;
	// TODO
	if(!rq->myprio.nr_running){
		printk(KERN_INFO "***[MYPRIO] enqueue first one: success cpu=%d, nr_running=%d, pid=%d\n",
				cpu_of(rq), rq->myprio.nr_running, p->pid);
		list_add_tail(&p->myprio.run_list, &rq->myprio.queue);
	} else {
		for(pos = myprio_rq->queue.next; pos != &myprio_rq->queue; pos = pos->next){
			pos_se = container_of(pos, struct sched_myprio_entity, run_list);
			pos_p = container_of(pos_se, struct task_struct, myprio);

			if(p->myprio.prio > pos_se->prio) {
				printk(KERN_INFO "***[MYPRIO] enqueue middle of queue: success cpu=%d, \
				nr_running=%d, pid=%d, next_prio=%d\n",
				 cpu_of(rq), myprio_rq->nr_running, p->pid, pos_se->prio);
				__list_add(&p->myprio.run_list, pos->prev, pos);
				is_enqueued=1;
				break;
			}
		}
		if(!is_enqueued){
			list_add_tail(&p->myprio.run_list, &myprio_rq->queue);
		}
	}
	rq->myprio.totalprio += p->myprio.prio;  /*add*/
	rq->myprio.nr_running++;
	p->myprio.prev_tp = rq->myprio.totalprio;

}

static void dequeue_task_myprio(struct rq *rq, struct task_struct *p, int flags)
{
	//TODO

	if( !list_empty(&rq->myprio.queue) )
	{
		list_del_init(&p->myprio.run_list);
		rq->myprio.nr_running--;
		rq->myprio.totalprio -= p->myprio.prio;

		printk(KERN_INFO "\t***[MYPRIO] dequeue: success cpu=%d, nr_running=%d, pid=%d\n",	
				cpu_of(rq), rq->myprio.nr_running, p->pid);
	
	}
	else
	{
	}
}

static void check_preempt_curr_myprio(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_myprio(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_myprio_entity *next_se = NULL;
	struct myprio_rq *myprio_rq = &rq->myprio;

	struct list_head *pos = NULL;
	struct task_struct *pos_p = NULL;
	struct sched_myprio_entity *pos_se = NULL;

	int i;

	//TODO
	if( !myprio_rq->nr_running ) 
	{
		return NULL;
	}

	if(prev->sched_class != &myprio_sched_class)
	{
		printk(KERN_INFO "***[MYPRIO] pick_next_task: other class came in.. prev->pid=%d\n",
																				 prev->pid);
		put_prev_task(rq, prev);
	}
	
	next_se = container_of(myprio_rq->queue.next, struct sched_myprio_entity, run_list);
	next_se->age = 0;
	pos = myprio_rq->queue.next;
	printk(KERN_INFO "\t***[MYPRIO] pick_next_task: show queue\n");

	for(i=1; i<myprio_rq->nr_running; i++){
		pos = pos->next;
		pos_se = container_of(pos, struct sched_myprio_entity, run_list);
		//pos_p = container_of(pos_se, struct task_struct, myprio);
		if(pos_se->age >= pos_se->prev_tp){
			pos_se->prev_tp = myprio_rq->totalprio;
			pos_se->age = 0;
			next_se = pos_se;
			break;
		}

		/*printk(KERN_INFO "\t\t***[MYPRIO] pick_next_task: pos->pid=%d, pos->prio=%d, age=%d\n",
						pos_p->pid, pos_se->prio, pos_se->age);*/
	}

	//TODO
	next_p = container_of(next_se, struct task_struct, myprio);

	printk(KERN_INFO "\t***[MYPRIO] pick_next_task: cpu=%d, prev->pid=%d, next_p->pid=%d, nr_running=%d, next_se->prio=%d\n",
		 cpu_of(rq), prev->pid, next_p->pid, myprio_rq->nr_running, next_se->prio);

	return next_p;
}

static void put_prev_task_myprio(struct rq *rq, struct task_struct *p)
{
	printk(KERN_INFO "\t***[MYPRIO] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_myprio(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_myprio(struct rq *rq){}
static void task_tick_myprio(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_myprio(rq);
}

static void prio_changed_myprio(struct rq *rq, struct task_struct *p, int oldprio) {}
static void switched_to_myprio(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}

const struct sched_class myprio_sched_class={
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_myprio,
	.dequeue_task = dequeue_task_myprio,
	.check_preempt_curr = check_preempt_curr_myprio,
	.pick_next_task = pick_next_task_myprio,
	.put_prev_task = put_prev_task_myprio,
#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_myprio,
#endif
	.set_curr_task = set_curr_task_myprio,
	.task_tick = task_tick_myprio,
	.prio_changed = prio_changed_myprio,
	.switched_to = switched_to_myprio,
	.update_curr = update_curr_myprio,
};

