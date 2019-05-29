#include "sched.h"
#include <linux/list.h>
#include <linux/slab.h>

void init_task_mystride(struct task_struct *p)
{
	struct sched_mystride_entity *se = &p->mystride;

	p->sched_class = &mystride_sched_class;
	se->stride = 1000/(se->ticket);
	se->pass = se->stride;
	printk(KERN_INFO "***[MYSTRD] init_task_mystrd:ticket=%d,stride=%d,pass=%dp->pid=%d\n",se->ticket,se->stride,se->pass,p->pid);
}

void init_mystride_rq(struct mystride_rq *mystride_rq)
{
	printk(KERN_INFO "***[MYSTRIDE] Mysched class is online\n");
	mystride_rq->nr_running = 0;
	INIT_LIST_HEAD(&mystride_rq->queue);
}
static void update_curr_mystride(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct mystride_rq *mystride_rq = &rq->mystride;
	struct sched_mystride_entity *m_entity = &curr->mystride;
	struct sched_mystride_entity *se = NULL;
	struct task_struct *p = NULL;
	unsigned int box = 0;
	int i=0;
	int j=0;

	printk(KERN_INFO "***[MYSTRD] update_curr:update_curr starting!\n");
	m_entity->pass = m_entity->pass+m_entity->stride;
	
	for(i=0;i<mystride_rq->nr_running;i++){
		se = container_of(mystride_rq->queue.next,struct sched_mystride_entity,run_list);
		box = se->pass;
		for(j=0;j<mystride_rq->nr_running;j++){
			se = container_of(se->run_list.next,struct sched_mystride_entity,run_list);
			if(box>se->pass){
				list_del_init(&se->run_list);
				list_add(&se->run_list,&mystride_rq->queue);
				break;
			}
		}
	}

	se = container_of(mystride_rq->queue.next,struct sched_mystride_entity,run_list);
	for(i=0;i<mystride_rq->nr_running;i++){
		
		p = container_of(se,struct task_struct,mystride);
		printk(KERN_INFO "***[MYSTRD] update_curr: curr->pid=%d,p->pid=%d,pass=%d\n",curr->pid,p->pid,se->pass);
		se = container_of(se->run_list.next,struct sched_mystride_entity,run_list);
	}

	se = container_of(mystride_rq->queue.next,struct sched_mystride_entity,run_list);
	printk(KERN_INFO "***[MYSTRD] update_curr:update_curr end!\n");
	if(se->pass<m_entity->pass)
		resched_curr(rq);
	
}
static void enqueue_task_mystride(struct rq *rq, struct task_struct *p,int flags)
{
	struct mystride_rq *mystride_rq = &rq->mystride;
	struct list_head *head = &mystride_rq->queue;
	struct sched_mystride_entity *m_entity = &p->mystride;

	list_add_tail(&m_entity->run_list,head);
	mystride_rq->nr_running++;
	
	printk(KERN_INFO "***[MYSTRIDE] enqueue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),mystride_rq->nr_running,p->state,p->pid);
	/*todo*/
}
static void dequeue_task_mystride(struct rq *rq, struct task_struct *p,int flags)
{
	struct list_head *head = &rq->mystride.queue;
	struct mystride_rq *mystride_rq = &rq->mystride;
	if(!list_empty(head)){
		list_del_init(&p->mystride.run_list);
		rq->mystride.nr_running--;
		printk(KERN_INFO "\t***[MYSTRIDE] dequeue: success cpu=%d,nr_running=%d,p->state=%lu,p->pid=%d\n",cpu_of(rq),mystride_rq->nr_running,p->state,p->pid);
	}
	else{}
	/*todo*/
}
static void check_preempt_curr_mystride(struct rq *rq, struct task_struct *p, int flags){}
struct task_struct *pick_next_task_mystride(struct rq *rq, struct task_struct *prev)
{
	struct task_struct *next_p = NULL;
	struct sched_mystride_entity *next_se = NULL;
	struct mystride_rq *mystride_rq = &rq->mystride;

	if(!mystride_rq->nr_running)
		return NULL;

	if(prev->sched_class != &mystride_sched_class)
	{
		printk(KERN_INFO "***[MYSTRIDE] pick_next_task: other class came in.. prev->pid=%d\n",prev->pid);
		put_prev_task(rq,prev);
	}
	next_se = container_of(mystride_rq->queue.next,struct sched_mystride_entity, run_list);
	next_p = container_of(next_se,struct task_struct,mystride);
	printk(KERN_INFO "\t***[MYSTRIDE] pick_next_task: cpu=%d,prev->pid=%d,next_p->pid=%d,nr_running=%d\n",cpu_of(rq),prev->pid,next_p->pid,mystride_rq->nr_running);
	return next_p;
	/*todo*/
}
static void put_prev_task_mystride(struct rq *rq, struct task_struct *p)
{	
	printk(KERN_INFO "\t***[MYSTRIDE] put_prev_task: do nothing, p->pid=%d\n", p->pid);	
}
static int select_task_rq_mystride(struct task_struct *p,int cpu, int sd_flag, int flags)
{
	return task_cpu(p);
}
static void set_curr_task_mystride(struct rq *rq){}
static void task_tick_mystride(struct rq *rq, struct task_struct *p,int queued)
{
	update_curr_mystride(rq);
}
static void prio_changed_mystride(struct rq *rq, struct task_struct *p, int oldprio){}

static void switched_to_mystride(struct rq *rq, struct task_struct *p)
{
	resched_curr(rq);
}
const struct sched_class mystride_sched_class={
	.next=&fair_sched_class,
	.enqueue_task=enqueue_task_mystride,
	.dequeue_task=dequeue_task_mystride,
	.check_preempt_curr=check_preempt_curr_mystride,
	.pick_next_task=pick_next_task_mystride,
	.put_prev_task=put_prev_task_mystride,
#ifdef CONFIG_SMP
	.select_task_rq=select_task_rq_mystride,
#endif
	.set_curr_task=set_curr_task_mystride,
	.task_tick=task_tick_mystride,
	.prio_changed=prio_changed_mystride,
	.switched_to=switched_to_mystride,
	.update_curr=update_curr_mystride,
};

