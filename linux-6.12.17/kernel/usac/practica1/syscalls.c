
#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <linux/time.h>
#include <linux/ktime.h>

SYSCALL_DEFINE0(hello_usac)
{
    printk(KERN_INFO "Hola desde la syscall USAC!\n");
    return 0; 
}