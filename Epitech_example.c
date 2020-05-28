#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include "Epitech_ioctl.h"


MODULE_LICENSE("GPL");

MODULE_AUTHOR("TOTO");

MODULE_DESCRIPTION("Example Module");

MODULE_VERSION("0.01");


#define DEVICE_NAME "Epitech_example"
#define EXAMPLE_MSG "Hello, World!\n"
#define MSG_BUFFER_LEN 16

struct my_device_data {
    struct cdev cdev;
    int size;
    char msg_buffer[MSG_BUFFER_LEN];
};

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;

static int device_open_count = 0;

static char msg_buffer[MSG_BUFFER_LEN];

static char *msg_ptr;

/* This structure points to all of the device functions */
static struct file_operations file_ops = 
{
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) 
{
    struct my_device_data *my_data;

    /* If device is open, return busy */
	if (device_open_count) 
	{
		printk(KERN_ALERT " Epitech Could not Open \n");
		return -EBUSY;
	}
	else
	{
		printk(KERN_ALERT "Epitech  Open \n");
        my_data = container_of(inode->i_cdev, struct my_device_data, cdev);
        file->private_data = my_data;
		device_open_count++;
	}

	try_module_get(THIS_MODULE);

	return 0;
}



/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data = (struct my_device_data *) flip->private_data;
    ssize_t len = min(my_data->size - (ssize_t) *offset, size);

    printk(KERN_INFO "%ui\n", len);
    if (len <= 0)
        return 0;
    /* read data from my_data->buffer to user buffer */
    if (copy_to_user(msg_buffer, my_data->msg_buffer + *offset, len))
        return -EFAULT;
    *offset += len;
    return len;
}

/* When a process writes from our device, this gets called. */
static ssize_t device_write(struct file *flip, const char *buffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data = (struct my_device_data *) flip->private_data;
    ssize_t len = min(my_data->size - *offset, size);

    if (len <= 0)
        return 0;

    /* read data from user buffer to my_data->buffer */
    if (copy_from_user(my_data->msg_buffer + *offset, msg_buffer, len))
        return -EFAULT;

    *offset += len;
    return len;
}



/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) 
{
	module_put(THIS_MODULE);
    device_open_count--;
	return 0;
}

static int __init Epitech_example_init(void) 
{
	/* Fill buffer with our message */
	strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);

	/* Set the msg_ptr to the buffer */
	msg_ptr = msg_buffer;

	/* Try to register character device */
	major_num = register_chrdev(0, DEVICE_NAME, &file_ops);

	if (major_num < 0) {
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num;
	} else {
		printk(KERN_INFO  " Hello Epitech_example module loaded with device major number %d\n", major_num);
		return 0;
	}
}

static void __exit Epitech_example_exit(void) 
{

	/* Remember  we have to clean up after ourselves. Unregister the character device. */
	unregister_chrdev(major_num, DEVICE_NAME);

	printk(KERN_INFO "Goodbye, World!\n");

}

/* Register module functions */
module_init(Epitech_example_init);

module_exit(Epitech_example_exit);
