#include "sched.h"
#include <linux/list.h>
#include <linux/slab.h>

void init_myrr_rq(struct myrr_rq *myrr_rq)
{
	printk(KERN_INFO "***[MYRR] Mysched class is online\n");
	myrr_rq->nr_running = 0;
	INIT_LIST_HEAD(&myrr_rq->queue);
}
static void update_curr_myrr(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct myrr_rq *myrr_rq = &rq->myrr;
	struct sched_myrr_entity *m_entity = &curr->myrr;
	m_entity->update_num++;
	printk(KERN_INFO "***[MYRR] update_curr_myrr\tpid = %d update_num=%d\n",curr->pid,m_entity->update_num);
	if(m_entity->update_num>3){
		m_entity->update_num=0;
		list_del_init(&curr->myrr.run_list);
		list_add_tail(&m_entity->run_list,&myrr_rq->queue);	
		resched_curr(rq);
	}
}
static void enqueue_task_myrr(struct rq *rq, struct task_struct *p,int flags)
{
	struct myrr_rq *myrr_rq = &rq->myrr;
	struct list_head *head = &myrr_rq->queue;
	struct sched_myrr_entity *m_entity = &p->myrr;

	list_add_tail(&m_entity->run_list,head);
	myrr_rq->nr_running++;
	
	printk(KERN_INFO "***[MYRR] enqueue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),myrr_rq->nr_running,p->state,p->pid);
	/*todo*/
}
static void dequeue_task_myrr(struct rq *rq, struct task_struct *p,int flags)
{
	struct list_head *head = &rq->myrr.queue;
	struct myrr_rq *myrr_rq = &rq->myrr;
	if(!list_empty(head)){
		list_del_init(&p->myrr.run_list);
		rq->myrr.nr_running--;
		printk(KERN_INFO "\t***[MYRR] dequeue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),myrr_rq->nr_running,p->state,p->pid);
	}
	else{}
	/*todo*/
}
static void check_preempt_curr_myrr(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_myrr(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_myrr_entity *next_se = NULL;
	struct myrr_rq *myrr_rq = &rq->myrr;

	if(!myrr_rq->nr_running)
		return NULL;

	if(prev->sched_class != &myrr_sched_class)
	{
		printk(KERN_INFO "***[MYRR] pick_next_task: other class came in.. prev->pid=%d\n",prev->pid);
		put_prev_task(rq,prev);
	}
	next_se = container_of(myrr_rq->queue.next,struct sched_myrr_entity, run_list);
	next_p = container_of(next_se,struct task_struct,myrr);
	printk(KERN_INFO "\t***[MYRR] pick_next_task: cpu=%d,prev->pid=%d,next_p->pid=%d,nr_running=%d\n",cpu_of(rq),prev->pid,next_p->pid,myrr_rq->nr_running);
	return next_p;
	/*todo*/
}
static void put_prev_task_myrr(struct rq *rq, struct task_struct *p)
{	
	printk(KERN_INFO "\t***[MYRR] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_myrr(struct task_struct *p,int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_myrr(struct rq *rq){}
static void task_tick_myrr(struct rq *rq, struct task_struct *p,int queued)
{
	update_curr_myrr(rq);
}
static void prio_changed_myrr(struct rq *rq, struct task_struct *p, int oldprio){}

static void switched_to_myrr(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}
const struct sched_class myrr_sched_class={
	.next=&fair_sched_class,
	.enqueue_task=enqueue_task_myrr,
	.dequeue_task=dequeue_task_myrr,
	.check_preempt_curr=check_preempt_curr_myrr,
	.pick_next_task=pick_next_task_myrr,
	.put_prev_task=put_prev_task_myrr,
#ifdef CONFIG_SMP
	.select_task_rq=select_task_rq_myrr,
#endif
	.set_curr_task=set_curr_task_myrr,
	.task_tick=task_tick_myrr,
	.prio_changed=prio_changed_myrr,
	.switched_to=switched_to_myrr,
	.update_curr=update_curr_myrr,
};

