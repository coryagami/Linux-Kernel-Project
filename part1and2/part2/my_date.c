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

/*
* This function gets ran every time there is a system call to read the
* /proc/currentdate file. It calls current_kernel_time() to get a struct of
* seconds in epoch, and then via time_to_tm() it fills the timespec struct
* xtime with the converted date/time.
*
* We also imlemented via the timezone struct sys_tz a simple correction of
* result.tm_hour to reflect the current date based on the current timezone.
*/
static int my_date_show(struct seq_file* m, void* v)
{
	xtime = current_kernel_time();
	time_to_tm(xtime.tv_sec, 0, &result);

	seq_printf(m, "%li-%02d-%02d %02d:%02d:%02d\n",
			result.tm_year+1900, result.tm_mon+1, result.tm_mday,
			result.tm_hour-sys_tz.tz_minuteswest/60, result.tm_min, result.tm_sec);
	return 0;
}

/*
* This function specifies what to do when an attempt is made to open the
* currentdate file. Here we specify that my_date_show should be used to
* display the file contents
*/
static int my_date_open(struct inode* inode, struct file* file)
{
	return single_open(file, my_date_show, NULL);
}

/*
* Here we fill the my_date_fops struct with the correct file operations data
* for the currentdate file we create.
*/
static const struct file_operations my_date_fops =
{
	.owner = THIS_MODULE,
	.open = my_date_open,
	.read = seq_read,
	.release = single_release,
};

/*
* my_date_init initializes the module and creates the proc file currentdate
* with the given file operations from my_date_fops.
*/
static int my_date_init(void)
{
	proc_create(filename, 0, NULL, &my_date_fops);
	printk(KERN_ALERT "my_date module loaded. currentdate proc file created.\n");

	return 0;
}

/*
* my_date_exit exits the module while also taking care of cleaning up the
* currentdate proc file we created in my_date_init.
*/
static void my_date_exit(void)
{
	remove_proc_entry(filename, NULL);
	printk(KERN_ALERT "currentdate proc file removed. my_date module removed.\n");
}

// Setting proper init/exit module functions
module_init(my_date_init);
module_exit(my_date_exit);
