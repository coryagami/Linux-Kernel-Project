/*
 * File        : my_date.c
 * Author      : Cory Agami / Brandom Hillan(?)
 * Class       : Operating Systems, Project 2, Part 2
 *
 * Description : Kernel module that creates a "currentdate" file in /proc/
 * 		 that stores the current kernel time in Y-m-d H:M:S format.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");

extern struct timezone sys_tz;

struct timespec xtime;
struct tm result;
static const char filename[] = "currentdate";

static int my_date_show(struct seq_file* m, void* v)
{
	xtime = current_kernel_time();
	time_to_tm(xtime.tv_sec, 0, &result);
	
	seq_printf(m, "%li-%02d-%02d %02d:%02d:%02d\n", 
					result.tm_year+1900, result.tm_mon+1, result.tm_mday, 
					result.tm_hour-sys_tz.tz_minuteswest/60, result.tm_min, result.tm_sec);
	return 0;
}

static int my_date_open(struct inode* inode, struct file* file)
{
	return single_open(file, my_date_show, NULL);
}

static const struct file_operations my_date_fops = 
{
	.owner = THIS_MODULE,
	.open = my_date_open,
	.read = seq_read,
	.release = single_release,
};

static int my_date_init(void)
{	
	proc_create(filename, 0, NULL, &my_date_fops);
	printk(KERN_ALERT "my_date module loaded. currentdate proc file created.\n");

	return 0;
}

static void my_date_exit(void)
{
	remove_proc_entry(filename, NULL);
	printk(KERN_ALERT "currentdate proc file removed. my_date module removed.\n");
}

module_init(my_date_init);
module_exit(my_date_exit);
