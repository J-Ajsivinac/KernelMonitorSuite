#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/signal.h>

SYSCALL_DEFINE2(kill_control, pid_t, pid, int, signal)
{
    struct pid *pid_struct;
    struct task_struct *task;
    int ret;

    // Buscar el proceso por PID
    pid_struct = find_get_pid(pid);
    if (!pid_struct)
        return -ESRCH;

    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (!task)
        return -ESRCH;

    // Verificar si la señal es permitida (solo SIGKILL o SIGCONT en este caso)
    if (signal != SIGKILL && signal != SIGCONT)
        return -EINVAL;

    // Enviar la señal
    ret = send_sig(signal, task, 0);

    put_task_struct(task);
    return ret;
}
