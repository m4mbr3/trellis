#include <security/pam_misc.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MAX_PAYLOAD 1024
#define NETLINK_PS 22
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

static struct pam_conv local_conversation = {
    misc_conv,
    NULL
};
int 
authenticate_system(const char *username)
{
    return 0; 
}

int
main(void) 
{
        char *ps = (char *) malloc(sizeof(char)*4);
        char *ptr;
        char lev_buf[4];
        int flag;
        int num;
        sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_PS);
        if (sock_fd < 0) {
            printf ("Cannot open the socket\n");
            exit (-1);
        }
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid();
        bind(sock_fd, (struct sockaddr*) &src_addr, sizeof(src_addr));

        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0;
        dest_addr.nl_groups = 0;

        nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD));
        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;

        strcpy(NLMSG_DATA(nlh), "REGISTRATION");
        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *) &dest_addr;
        msg.msg_namelen = sizeof (dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        sendmsg(sock_fd, &msg, 0);
        while (1) {
           recvmsg(sock_fd, &msg, 0); 
           printf("Received the message\n");
           memset(lev_buf, 0, 4);
           strncpy(lev_buf, NLMSG_DATA(nlh),4);  
           memset(ps, 0, 4);
           strncpy(ps, "ps_", 3); 
           ptr = strncat(ps, lev_buf, 4);
           memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
           nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
           nlh->nlmsg_pid = getpid();
           nlh->nlmsg_flags = 0;
           if (authenticate_system(ptr) == 0) strncpy(NLMSG_DATA(nlh),"OK", 2);
           else strncpy(NLMSG_DATA(nlh), "NO", 2);
           printf("Received %s \n", NLMSG_DATA(nlh));
           sendmsg(sock_fd, &msg, 0);  
        }
   close(sock_fd);   
   exit(EXIT_SUCCESS);
}
