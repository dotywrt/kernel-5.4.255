// SPDX-License-Identifier: GPL-2.0-only
/* DOTY-AW1000-SLIC-IRQ-BEGIN */
/*
 * AW1000/FG360 compatibility bridge for Fibocom ght-slic userspace.
 *
 * DTS supplies an interrupt for compatible "fg360,slic_irq".  The legacy
 * application uses select() on /dev/slic_irq_pin and then read() merely as
 * an interrupt acknowledgement; it does not depend on payload contents.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>

struct fg360_slic_irq {
	struct device *dev;
	int irq;
	wait_queue_head_t waitq;
	atomic_t pending;
	struct miscdevice miscdev;
};

static irqreturn_t fg360_slic_irq_handler(int irq, void *data)
{
	struct fg360_slic_irq *s = data;

	atomic_inc(&s->pending);
	wake_up_interruptible(&s->waitq);
	return IRQ_HANDLED;
}

static int fg360_slic_irq_open(struct inode *inode, struct file *file)
{
	struct miscdevice *m = file->private_data;
	struct fg360_slic_irq *s = container_of(m, struct fg360_slic_irq, miscdev);

	file->private_data = s;
	return 0;
}

static ssize_t fg360_slic_irq_read(struct file *file, char __user *buf,
				   size_t count, loff_t *ppos)
{
	struct fg360_slic_irq *s = file->private_data;
	char event = '1';
	int ret;

	if (!count)
		return 0;

	if (file->f_flags & O_NONBLOCK) {
		if (atomic_read(&s->pending) <= 0)
			return -EAGAIN;
	} else {
		ret = wait_event_interruptible(s->waitq,
					       atomic_read(&s->pending) > 0);
		if (ret)
			return ret;
	}

	if (atomic_dec_if_positive(&s->pending) < 0)
		return -EAGAIN;

	if (copy_to_user(buf, &event, 1))
		return -EFAULT;
	return 1;
}

static __poll_t fg360_slic_irq_poll(struct file *file, poll_table *wait)
{
	struct fg360_slic_irq *s = file->private_data;
	__poll_t mask = 0;

	poll_wait(file, &s->waitq, wait);
	if (atomic_read(&s->pending) > 0)
		mask |= EPOLLIN | EPOLLRDNORM;
	return mask;
}

static const struct file_operations fg360_slic_irq_fops = {
	.owner = THIS_MODULE,
	.open = fg360_slic_irq_open,
	.read = fg360_slic_irq_read,
	.poll = fg360_slic_irq_poll,
	.llseek = no_llseek,
};

static int fg360_slic_irq_probe(struct platform_device *pdev)
{
	struct fg360_slic_irq *s;
	int ret;

	s = devm_kzalloc(&pdev->dev, sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->dev = &pdev->dev;
	s->irq = platform_get_irq(pdev, 0);
	if (s->irq < 0)
		return s->irq;

	init_waitqueue_head(&s->waitq);
	atomic_set(&s->pending, 0);

	ret = devm_request_irq(&pdev->dev, s->irq, fg360_slic_irq_handler,
			       0, dev_name(&pdev->dev), s);
	if (ret)
		return ret;

	s->miscdev.minor = MISC_DYNAMIC_MINOR;
	s->miscdev.name = "slic_irq_pin";
	s->miscdev.fops = &fg360_slic_irq_fops;
	s->miscdev.parent = &pdev->dev;

	ret = misc_register(&s->miscdev);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, s);
	dev_info(&pdev->dev, "/dev/slic_irq_pin ready on IRQ %d\n", s->irq);
	return 0;
}

static int fg360_slic_irq_remove(struct platform_device *pdev)
{
	struct fg360_slic_irq *s = platform_get_drvdata(pdev);

	misc_deregister(&s->miscdev);
	return 0;
}

static const struct of_device_id fg360_slic_irq_of_match[] = {
	{ .compatible = "fg360,slic_irq" },
	{ }
};
MODULE_DEVICE_TABLE(of, fg360_slic_irq_of_match);

static struct platform_driver fg360_slic_irq_driver = {
	.probe = fg360_slic_irq_probe,
	.remove = fg360_slic_irq_remove,
	.driver = {
		.name = "fg360-slic-irq",
		.of_match_table = fg360_slic_irq_of_match,
	},
};
module_platform_driver(fg360_slic_irq_driver);

MODULE_DESCRIPTION("FG360/AW1000 SLIC IRQ userspace compatibility");
MODULE_LICENSE("GPL");
/* DOTY-AW1000-SLIC-IRQ-END */
