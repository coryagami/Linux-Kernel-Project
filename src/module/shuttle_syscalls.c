#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/export.h>

int (*STUB_start_shuttle)(void) = NULL;
int (*STUB_stop_shuttle)(void) = NULL;
int (*STUB_issue_request)(char pass_type, int init_term, int dest_term) = NULL;

EXPORT_SYMBOL(STUB_start_shuttle);
EXPORT_SYMBOL(STUB_stop_shuttle);
EXPORT_SYMBOL(STUB_issue_request);

asmlinkage int sys_start_shuttle(void)
{
	if(STUB_start_shuttle)
		return STUB_start_shuttle();
	else
		return -ENOSYS;
}

asmlinkage int sys_stop_shuttle(void)
{
	if(STUB_stop_shuttle)
		return STUB_stop_shuttle();
	else
		return -ENOSYS;
}

asmlinkage int sys_issue_request(char pass_type, int init_term, int dest_term)
{
	if(STUB_issue_request)
		return STUB_issue_request(pass_type, init_term, dest_term);
	else
		return -ENOSYS;
}
