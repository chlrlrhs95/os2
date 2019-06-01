#include "sched.h"
#include <linux/list.h>
#include <linux/slab.h>

void init_task_mypriority(struct task_struct *p)
{
	struct sched_mypriority_entity *se = &p->mypriority;

	p->sched_class = &mypriority_sched_class;
	se->age = 0;	
}

void init_mypriority_rq(struct mypriority_rq *mypriority_rq)
{
	int i=0;
	printk(KERN_INFO "***[MYPRIORITY] Mysched class is online\n");
	mypriority_rq->nr_total = 0;
	for(i=0;i<10;i++){
		mypriority_rq->nr_running[i] = 0;
		INIT_LIST_HEAD(&mypriority_rq->queue[i]);
	}
}
static void update_curr_mypriority(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct sched_mypriority_entity * m_entity = &curr->mypriority;
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct sched_mypriority_entity *se = NULL;
	struct task_struct *p = NULL;
	int i=0;
	int j=0;

	printk(KERN_INFO " ***[MYPRIORITY] BEFORE_UPDATE pid=%d, priority=%d, age=%d\n",p->pid,m_entity->priority,m_entity->age);
	m_entity->age +=1;
	if(m_entity->age>5){
		m_entity->age = 0;
		m_entity->priority += 1;
		if(m_entity->priority==10)
			m_entity->priority -=1;	
	}

	for(i=0;i<10;i++){
		se=container_of(mypriority_rq->queue[i].next,struct sched_mypriority_entity, run_list);
		for(j=0;j<mypriority_rq->nr_running[i];j++){
			printk(KERN_INFO " ***[MYPRIORITY] AFTER UPDATE pid=%d, priority=%d, age=%d\n",p->pid,m_entity->priority,m_entity->age);	
			se=container_of(se->run_list.next,struct sched_mypriority_entity, run_list);	
		}
	}
	list_del_init(&m_entity->run_list);
	list_add_tail(&m_entity->run_list,&mypriority_rq->queue[m_entity->priority]);
	resched_curr(rq);	
}

static void enqueue_task_mypriority(struct rq *rq, struct task_struct *p,int flags)
{
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	struct sched_mypriority_entity *m_entity = &p->mypriority;
	struct list_head *head = &mypriority_rq->queue[m_entity->priority];
	
	list_add_tail(&m_entity->run_list,head);
	mypriority_rq->nr_running[m_entity->priority]++;
	mypriority_rq->nr_total++;
	printk(KERN_INFO "***[MYPRIORITY] enqueue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),mypriority_rq->nr_total,p->state,p->pid);
	/*todo*/
}
static void dequeue_task_mypriority(struct rq *rq, struct task_struct *p,int flags)
{
	struct sched_mypriority_entity *m_entity = &p->mypriority;
	struct list_head *head = &rq->mypriority.queue[m_entity->priority];
	struct mypriority_rq *mypriority_rq = &rq->mypriority;

	if(!list_empty(head)){
		list_del_init(&p->mypriority.run_list);
		rq->mypriority.nr_running[m_entity->priority]--;
		rq->mypriority.nr_total--;
		printk(KERN_INFO "\t***[MYPRIORITY] dequeue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),mypriority_rq->nr_total,p->state,p->pid);
	}
	else{}
	/*todo*/
}
static void check_preempt_curr_mypriority(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_mypriority(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_mypriority_entity *se = NULL;
	struct mypriority_rq *mypriority_rq = &rq->mypriority;
	int i=0;

	if(!mypriority_rq->nr_total)
		return NULL;

	if(prev->sched_class != &mypriority_sched_class)
	{
		printk(KERN_INFO "***[MYPRIORITY] pick_next_task: other class came in.. prev->pid=%d\n",prev->pid);
		put_prev_task(rq,prev);
	}
	for(i=0;i<10;i++){
		if(mypriority_rq->nr_running[i]>0){
			se=container_of(mypriority_rq->queue[i].next,struct sched_mypriority_entity, run_list);	
			next_p = container_of(se,struct task_struct,mypriority);
				
			printk(KERN_INFO "\t***[MYPRIORITY] pick_next_task: prev->pid=%d,next_p->pid=%d,nr_runnig=%d, prio=%d\n",prev->pid,next_p->pid,mypriority_rq->nr_total,se->priority);
			break;
		}
	}
	return next_p;
}
static void put_prev_task_mypriority(struct rq *rq, struct task_struct *p)
{	
	printk(KERN_INFO "\t***[MYPRIORITY] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_mypriority(struct task_struct *p,int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_mypriority(struct rq *rq){}
static void task_tick_mypriority(struct rq *rq, struct task_struct *p,int queued)
{
	update_curr_mypriority(rq);
}
static void prio_changed_mypriority(struct rq *rq, struct task_struct *p, int oldprio){}

static void switched_to_mypriority(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}
const struct sched_class mypriority_sched_class={
	.next=&fair_sched_class,
	.enqueue_task=enqueue_task_mypriority,
	.dequeue_task=dequeue_task_mypriority,
	.check_preempt_curr=check_preempt_curr_mypriority,
	.pick_next_task=pick_next_task_mypriority,
	.put_prev_task=put_prev_task_mypriority,
#ifdef CONFIG_SMP
	.select_task_rq=select_task_rq_mypriority,
#endif
	.set_curr_task=set_curr_task_mypriority,
	.task_tick=task_tick_mypriority,
	.prio_changed=prio_changed_mypriority,
	.switched_to=switched_to_mypriority,
	.update_curr=update_curr_mypriority,
};

