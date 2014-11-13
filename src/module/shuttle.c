#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>	// for msleep()
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

extern int (*STUB_start_shuttle)(void);
extern int (*STUB_stop_shuttle)(void);
extern int (*STUB_issue_request)(char p, int i, int d);

void moveto(int);
int run(void*);
void printlist(void);
int getweight(char);
void update_pass_now(char, char);
void update_pass_had(char);
void update_term_pass(char, char, int);
void update_term_deliv(int);
void free_line(void);
static int shuttle_info_open(struct inode*, struct file*);
static int shuttle_info_show(struct seq_file*, void*);

struct mutex startstop_lock;	// lock for starting/stopping shuttle
struct mutex line_lock;			// lock for people waiting queue
struct task_struct* task = NULL;
typedef enum {OFFLINE, PARKED, MOVING, DEACTIVATING} status_enum;
typedef enum {FORWARD, REVERSE} direction_enum;
typedef struct { int C; int A; int L; } passenger_count;
typedef struct { char pass_type; int init_term; 
				int dest_term; struct list_head list; } person;
typedef struct
{
	status_enum status; // = OFFLINE;
	bool online; // = false;
	int capacity; // = 50;
	int load; // = 0;
	direction_enum direction; // = FORWARD;
	passenger_count pass_now; // = { 0, 0, 0 }
	passenger_count pass_had; // = { 0, 0, 0 }
	int curr_term; // = 0;
	int dest_term; // = 0;
	person passengers;
} shuttle;

// terminals[] holds count of the types of people at each temrinal
passenger_count terminals[5]={ {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };
int terminals_delivered[5] = { 0,0,0,0,0 };

passenger_count weight = { 1, 2, 4 };
int tcoeff = 100;	// ms * tcoeff = delay interval. 100 for msleep(1) to
					// sleep for .1 seconds, 1000 for 1, etc

shuttle S;
person line; // for people queue

MODULE_LICENSE("Dual BSD/GPL");

int start_shuttle(void)
{
	mutex_lock(&startstop_lock);
	if( S.online == false )
	{
		printk(KERN_DEBUG "Shuttle starting.\n");
		S.online = true;
		S.load = 0;
		S.capacity = 50;
		S.pass_now.C = 0; S.pass_now.A = 0; S.pass_now.L = 0;
		S.pass_had.C = 0; S.pass_had.A = 0; S.pass_had.L = 0;
		S.status = PARKED;
		INIT_LIST_HEAD(&S.passengers.list);

		moveto(3);	// starts at terminal 3

		task = kthread_run(run, (void*)NULL, "running shuttle");

		printk(KERN_DEBUG "Shuttle started.\n");
		
		mutex_unlock(&startstop_lock);
		return 0;
	}
	else
	{
		printk(KERN_DEBUG "Shuttle already started.\n");
		mutex_unlock(&startstop_lock);
		return 1;
	}
}

int stop_shuttle(void)
{
	// printlist();

	mutex_lock(&startstop_lock);
	if( S.status != DEACTIVATING && S.status != OFFLINE )
	{
		printk(KERN_DEBUG "Shuttle deactivating.\n");

		// changing status to DEACTIVATING and stopping asking the 
		// thread to stop ensure that run() drops all passengers off 
		// before actually stopping
		S.status = DEACTIVATING;
		kthread_stop(task);

		moveto(3);

		S.online = false;
		S.status = OFFLINE;
		mutex_unlock(&startstop_lock);

		printk(KERN_DEBUG "Shuttle deactivated.\n");
		return 0;
	}
	else
	{
		printk(KERN_DEBUG "Shuttle already deactivating.\n");
		mutex_unlock(&startstop_lock);
		return 1;
	}
}

int issue_request(char pass_type, int init_term, int dest_term)
{
	person* tmp;
	mutex_lock(&line_lock);

	if( ( pass_type != 'C' && pass_type != 'A' && pass_type != 'L' ) || 
		init_term > 5 || init_term < 1 || dest_term > 5 || dest_term < 1 )
	{
		printk(KERN_DEBUG "Invalid request for passenger(%c, %d, %d)\n",
			pass_type, init_term, dest_term);
		mutex_unlock(&line_lock);
		return 1;
	}
	else
	{
		printk(KERN_DEBUG "Adding request for passenger (%c, %d, %d)\n",
			pass_type, init_term, dest_term);

		tmp = (person*)kmalloc(sizeof(person), GFP_KERNEL);
		tmp->pass_type = pass_type;
		tmp->init_term = init_term; 
		tmp->dest_term = dest_term;
		list_add(&(tmp->list), &(line.list));
		update_term_pass('A', tmp->pass_type, tmp->init_term);
		mutex_unlock(&line_lock);

		return 0;
	}
}

// simple helper function to print status of passengers
// and people still waiting at terminals
void printlist()
{
	person* tmp;
	struct list_head* pos;
	int i = 0;

	list_for_each(pos, &S.passengers.list)
	{
		tmp = list_entry(pos, person, list);
		printk(KERN_ALERT "passenger[%d]: %c %d %d\n", 
						i, tmp->pass_type, tmp->init_term, tmp->dest_term);
		i++;
	}

	mutex_lock(&line_lock);
	list_for_each(pos, &line.list)
	{
		tmp = list_entry(pos, person, list);
		printk(KERN_ALERT "list[%d]: %c %d %d\n", 
						i, tmp->pass_type, tmp->init_term, tmp->dest_term);
		i++;
	}
	mutex_unlock(&line_lock);
}

// function to move teh shuttle to given terminal, and
// recalculate the route/destination according to the travel
// pattern 1-2-3-4-5-4-3-2-1... wait times are implemented here
void moveto(int term)
{
	printk(KERN_ALERT "Moving to %d\n", term);

	if( S.status != DEACTIVATING )
		S.status = MOVING;
	msleep( abs(S.curr_term - term) * 30 * tcoeff );
	S.curr_term = term;
	if( S.status != DEACTIVATING )
		S.status = PARKED;

	// setting direction. shuttle moves 1-2-3-4-5-4-3-2-1...
	//									f-f-f-f-r-r-r-r-f...
	if( S.curr_term == 5 )
		S.direction = REVERSE;
	else if( S.curr_term == 1 )
		S.direction = FORWARD;

	// update dest_term accordingly
	if( S.direction == FORWARD )
		S.dest_term = S.curr_term + 1;
	else if( S.direction == REVERSE )
		S.dest_term = S.curr_term - 1;

	printk(KERN_ALERT "Arrived at %d\n", term);

	msleep( 10 * tcoeff );
}

// main function to continually move shuttle terminal by terminal and
// first remove, then add people at each stop. run() is the funciton the
// kthread runs and it is set to stop when kthread_should_stop is true
// AND there are no more people. if the shuttle is DEACTIVATING then
// run wil only remove people, it will not take on any more passengers.
int run(void* data)
{
	while( !kthread_should_stop() || S.load > 0 )
	{
		int people_getting_onoff = 0;

		person* tmp, *tmp2;
		struct list_head* pos, *q;

		moveto(S.dest_term);

		// removing people
		list_for_each_safe(pos, q, &S.passengers.list)
		{
			tmp = list_entry(pos, person, list);
			if( tmp->dest_term == S.curr_term )
			{
				people_getting_onoff++;
				S.load -= getweight(tmp->pass_type);
				update_pass_now('R', tmp->pass_type);
				update_pass_had(tmp->pass_type);
				update_term_deliv(tmp->dest_term);

				printk(KERN_ALERT "removing %c %d %d\n",
							tmp->pass_type, tmp->init_term, tmp->dest_term);
				list_del(pos); kfree(tmp);
			}
		}

		// adding people
		if( S.status != DEACTIVATING )
		{
			mutex_lock(&line_lock);
			list_for_each_safe(pos, q, &line.list)
			{
				tmp = list_entry(pos, person, list);
				if( tmp->init_term == S.curr_term )	
				{
					if( S.load + getweight(tmp->pass_type) <= S.capacity )
					{
						tmp2 = (person*)kmalloc(sizeof(person), GFP_KERNEL);
						tmp2->pass_type = tmp->pass_type;
						tmp2->init_term = tmp->init_term;
						tmp2->dest_term = tmp->dest_term;

						people_getting_onoff++;
						S.load += getweight(tmp2->pass_type);
						list_add(&(tmp2->list), &(S.passengers.list));
						update_pass_now('A', tmp2->pass_type);
						update_term_pass('R', 
							tmp2->pass_type, tmp2->init_term);

						printk(KERN_ALERT "added %c %d %d\n",
							tmp2->pass_type, tmp2->init_term, tmp2->dest_term);
						list_del(pos); kfree(tmp);
					}
				}
			}
			mutex_unlock(&line_lock);
		}

		// waiting 3 sec for additional people
		if( people_getting_onoff > 4)
			msleep( (people_getting_onoff - 4) * 3 * tcoeff );
	}
	return 0;
}

// called durint the module exit to ensure if people were still waiting
// at terminals, that they are all freed accordingly before stopping.
void free_line()
{
	person* tmp;
	struct list_head* pos, *q;

	list_for_each_safe(pos, q, &line.list)
	{
		tmp = list_entry(pos, person, list);
		list_del(pos); kfree(tmp);
	}
}

// helper to update passenger count of each type at each terminal
void update_term_pass(char type, char p_type, int term)
{
	if( type == 'R' )
	{
		switch(p_type)
		{
			case 'C':
				terminals[term-1].C--; break;
			case 'A':
				terminals[term-1].A--; break;
			case 'L':
				terminals[term-1].L--; break;
		}
	}
	else if( type == 'A' )
	{
		switch(p_type)
		{
			case 'C':
				terminals[term-1].C++; break;
			case 'A':
				terminals[term-1].A++; break;
			case 'L':
				terminals[term-1].L++; break;
		}
	}
}

// helper to increment total delivered at each terminal
void update_term_deliv(int term)
{
	terminals_delivered[term-1]++;
}

// helper to get passenger weight according to weight struct
int getweight(char type)
{
	if( type == 'C' )
		return weight.C;
	else if( type == 'A' )
		return weight.A;
	else if( type == 'L' )
		return weight.L;
	else
		return -1;
}

// helper to update pasengers the shuttle has had total
void update_pass_had(char p_type)
{
	switch(p_type)
	{
		case 'C':
			S.pass_had.C++; break;
		case 'A':
			S.pass_had.A++; break;
		case 'L':
			S.pass_had.L++; break;
	}
}

// helper to update passengers the shuttle has now
void update_pass_now(char type, char p_type)
{
	if( type == 'R' )
	{
		switch(p_type)
		{
			case 'C':
				S.pass_now.C--; break;
			case 'A':
				S.pass_now.A--; break;
			case 'L':
				S.pass_now.L--; break;
		}
	}
	else if( type == 'A' )
	{
		switch(p_type)
		{
			case 'C':
				S.pass_now.C++; break;
			case 'A':
				S.pass_now.A++; break;
			case 'L':
				S.pass_now.L++; break;
		}
	}
}

static const struct file_operations shuttle_info_ops = 
{
	.owner = THIS_MODULE,
	.open = shuttle_info_open,
	.read = seq_read,
	.release = single_release
};

static int shuttle_info_open(struct inode* inode, struct file* file)
{
	return single_open(file, shuttle_info_show, NULL);
}

// proc file outputting/showing function
static int shuttle_info_show(struct seq_file* out, void* data)
{
	char* status = NULL;
	int avail_seats = (S.capacity - S.load) / 2;
	int avail_seats_dec = (S.capacity - S.load) % 2;
	int used_seats = S.load / 2;
	int used_seats_dec = S.load % 2;
	int curr_passengers_total = S.pass_now.C + S.pass_now.A + S.pass_now.L;
	int child_now = S.pass_now.C;
	int adult_now = S.pass_now.A;
	int luggage_now = S.pass_now.L;
	int delivered_total = S.pass_had.C + S.pass_had.A + S.pass_had.L;
	int child_had = S.pass_had.C;
	int adult_had = S.pass_had.A;
	int luggage_had = S.pass_had.L;
	int i = 0;

	if( S.status == PARKED )
		status = "PARKED";
	else if( S.status == MOVING )
		status = "MOVING";
	else if( S.status == DEACTIVATING )
		status = "DEACTIVATING";
	else if( S.status == OFFLINE )
		status = "OFFLINE";

	seq_printf(out, "Status:\t\t%s\n", status);
	if( used_seats_dec )
		seq_printf(out, "\nSeats:\t\t%d.5 used", used_seats);
	else
		seq_printf(out, "\nSeats:\t\t%d used", used_seats);
	if( avail_seats_dec )
		seq_printf(out, " %d.5 available\n", avail_seats);
	else
		seq_printf(out, " %d available\n", avail_seats);
	seq_printf(out, 
			"Passengers: \t%d (%d adult with luggage, "
			"%d adult without luggage, "
			"%d children)\n",
			curr_passengers_total, adult_now, luggage_now, child_now);
	seq_printf(out, "Location:\t%d\n", S.curr_term);
	seq_printf(out, "Destination:\t%d\n", S.dest_term);
	seq_printf(out, 
			"Delivered: \t%d (%d adult with luggage, "
			"%d adult without luggage, "
			"%d children)\n",
			delivered_total, adult_had, luggage_had, child_had);

	seq_printf(out, "-----\n");

	for(; i<5; i++)
	{
		seq_printf(out, 
			"Terminal %d:%d adult with luggage, "
			"%d adult without luggage, "
			"%d children in queue.\n"
			"%d passengers delivered so far.\n", 
			i+1, terminals[i].L, terminals[i].A, 
			terminals[i].C, terminals_delivered[i]);
	}

	return 0;
}

// initialize the line list here so people can be added before
// the shuttle is officially started with start_shuttle()
static int __init shuttle_init(void)
{
	mutex_init(&startstop_lock);
	mutex_init(&line_lock);

	INIT_LIST_HEAD(&line.list);

	STUB_start_shuttle = start_shuttle;
	STUB_stop_shuttle = stop_shuttle;
	STUB_issue_request = issue_request;

	proc_create("terminal", 0, NULL, &shuttle_info_ops);

	return 0;
}

// module exit function to make sure shuttle is stopped and all
// passengers evacuated before stopping. Also makes sure memory
// of line is freed in case smodule is closed before all people
// have been transported.
static void __exit shuttle_exit(void)
{
	
	if( S.status != OFFLINE )
		stop_shuttle();

	free_line();

	STUB_start_shuttle = NULL;
	STUB_stop_shuttle = NULL;
	STUB_issue_request = NULL;

	remove_proc_entry("terminal", NULL);
}

module_init(shuttle_init);
module_exit(shuttle_exit);
