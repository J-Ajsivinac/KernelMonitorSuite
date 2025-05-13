#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/jiffies.h>  // Para HZ

/**
 * Syscall para obtener tiempo de CPU de un proceso en milisegundos
 * @param pid: ID del proceso
 * @return Tiempo total de CPU en milisegundos, o error
 */
SYSCALL_DEFINE1(get_cpu_time, pid_t, pid)
{
    struct task_struct *task;
    u64 elapsed, elapsed_ms;

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task == NULL) {
        printk(KERN_INFO "Cannot find a process with that PID: %d\n", pid);
        return -2;
    }

    // Obtener el tiempo total de CPU usado en ticks (jiffies)
    elapsed = task->utime + task->stime;

    elapsed_ms = div_u64(elapsed,1000000);

    return elapsed_ms;
}
