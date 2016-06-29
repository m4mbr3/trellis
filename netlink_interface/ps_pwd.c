#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/time.h>
#define FAIL -1
#define SUCCESS 0
extern struct sock *sk_b;
extern wait_queue_head_t ps_wait_for_msg;
extern char ps_buffer[20];
extern int ps_daemon_pid;

static void ps_rcv_msg(struct sk_buff *skb) 
{
    struct nlmsghdr *nlh;
    nlh =(struct nlmsghdr*) skb->data;
    if (strncmp("REGISTRATION", (char *)nlmsg_data(nlh), sizeof("REGISTRATION")) == 0) {
        printk("PS_PWD: received message with REGISTRATION \n");
        ps_daemon_pid = nlh->nlmsg_pid;
        printk("PS_PWD: the pid is %d \n", ps_daemon_pid);
    }
    else {
        strncpy(ps_buffer,(char *) nlmsg_data(nlh) , sizeof(ps_buffer));           
        printk("PS_PWD: waking up the syscall \n");
        while (list_empty(&ps_wait_for_msg.task_list)) udelay(20);
        wake_up_interruptible(&ps_wait_for_msg);
        printk("PS_PWD: waked up the syscall \n");
    }
}    

/*PS INIT*/
int ps_init(void)
{	
    struct netlink_kernel_cfg cfg;
    cfg.groups = 0;
    cfg.flags = 0;
    cfg.input = ps_rcv_msg;
    cfg.cb_mutex = NULL;
    cfg.bind = NULL;
    printk ("PS_PWD: Creating the PS Net Link \n");
    sk_b = netlink_kernel_create(&init_net, NETLINK_PS, &cfg);
    if (!sk_b) {
        printk("PS_PWD: Error creating the socket.\n");
        return FAIL;
    }
    return SUCCESS;
}

/*PS CLEANUP*/
void ps_cleanup(void)
{
    sk_b = NULL;
    memset(ps_buffer, 0, sizeof(ps_buffer));
    ps_daemon_pid = -1;
    printk ("PS_PWD: Destroying net link \n");
    netlink_kernel_release(sk_b);
}


module_init(ps_init);
module_exit(ps_cleanup);

MODULE_LICENSE ("GPL");

MODULE_AUTHOR("Andrea Mambretti");
MODULE_DESCRIPTION("Manage the socket of my privilege separation system to provide the authentication functionality.");

MODULE_SUPPORTED_DEVICE("ps_pwd");
