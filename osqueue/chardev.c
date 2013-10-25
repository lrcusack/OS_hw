#include "chardev.h"
#include "kqueue.h"

/* Globals localized to file (by use of static */
static int Major;		/* assigned to device driver */
static char msg[BUF_LEN];	/* a stored message */
kqueue* kq;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static int device_open(struct inode *inode, struct file *file)
{
	
	try_module_get(THIS_MODULE);
	kq = kq_create();
	return 0;
}


static int device_release(struct inode *inode, struct file *file)
{
	kq_delete(kq);
	module_put(THIS_MODULE);
	return 0;
}

/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char *buff,
			    size_t len, loff_t * off)
{
	int copy_len = len > BUF_LEN ? BUF_LEN : len;
	unsigned long amnt_copied = 0;

	/* NOTE: copy_from_user returns the amount of bytes _not_ copied */
	amnt_copied = copy_from_user(msg, buff, copy_len);
	//if (copy_len == amnt_copied) return -EINVAL;
		

	kq_enqueue(kq,(char*)msg);


	return copy_len - amnt_copied;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t len,
			   loff_t * offset)
{
	
	unsigned long amnt_copied = 0;
	int amnt_left = BUF_LEN;
	int copy_len = len > amnt_left ? amnt_left : len;
	printk("starting to read\n");

	/* are we at the end of the buffer? */
	if (amnt_left <= 0)
		return 0;
		
	kq_dequeue(kq,(char*) msg);
	
	/* NOTE: copy_to_user returns the amount of bytes _not_ copied */
	amnt_copied = copy_to_user(buffer, msg, copy_len);
	if (copy_len == amnt_copied)
		return 0;

	/* adjust the offset for this process */
	
	
	

	return copy_len - amnt_copied;
}

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Failed to register char device.\n");
		return Major;
	}

	memset(msg, '+', BUF_LEN);
	printk(KERN_INFO "chardev is assigned to major number %d.\n",
	       Major);

	return 0;
}
void cleanup_module(void)
{
	int ret = unregister_chrdev(Major, DEVICE_NAME);
	if (ret < 0)
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}
