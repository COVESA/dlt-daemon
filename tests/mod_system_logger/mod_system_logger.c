#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

int i;

static int system_proc_show(struct seq_file *m, void *v)
{
    for (i = 0; i < 1000; i++)
        seq_printf(m, "Test Systemlogger %i\n", i);

    return 0;
}

static int system_proc_open(struct inode *inode, struct  file *file)
{
    return single_open(file, system_proc_show, NULL);
}

static const struct file_operations system_proc_fops = {
    .owner = THIS_MODULE,
    .open = system_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init system_proc_init(void)
{
    proc_create("systemlogger", 0, NULL, &system_proc_fops);
    return 0;
}

static void __exit system_proc_exit(void)
{
    remove_proc_entry("systemlogger", NULL);
}

MODULE_LICENSE("GPL");
module_init(system_proc_init);
module_exit(system_proc_exit);
