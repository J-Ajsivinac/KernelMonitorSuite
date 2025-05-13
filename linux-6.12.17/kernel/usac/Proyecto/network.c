#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/netdevice.h>
#include <linux/uaccess.h>
#include <linux/rtnetlink.h> // 👈 Necesario para usar rtnl_lock

struct net_usage_info {
    unsigned long bytes_sent;
    unsigned long bytes_received;
};

SYSCALL_DEFINE1(get_system_network_usage, struct net_usage_info __user *, user_info)
{
    struct net_device *dev;
    struct net_usage_info info = {0};
    struct rtnl_link_stats64 stats;

    rtnl_lock(); // Bloquea el acceso a interfaces de red

    for_each_netdev(&init_net, dev) {
        // Ignorar la interfaz loopback
        if (strcmp(dev->name, "lo") == 0)
            continue;

        memset(&stats, 0, sizeof(stats));
        dev_get_stats(dev, &stats);
        info.bytes_sent += stats.tx_bytes;
        info.bytes_received += stats.rx_bytes;
    }

    rtnl_unlock(); // Desbloquea

    if (copy_to_user(user_info, &info, sizeof(info)))
        return -EFAULT;

    return 0;
}
