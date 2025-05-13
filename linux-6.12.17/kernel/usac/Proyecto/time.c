#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/rcupdate.h>
#include <linux/init.h>

// Estructuras necesarias
struct process_times {
    pid_t pid;
    char name[TASK_COMM_LEN];
    unsigned long start_time; // En segundos desde boot
    unsigned long end_time;   // En segundos desde boot, 0 si sigue activo
};

struct process_time_list {
    int max_processes;
    int num_processes;
    struct process_times __user *times;
};

// Definición de la syscall
SYSCALL_DEFINE1(get_all_process_times, struct process_time_list __user *, user_list) {
    struct process_time_list klist;
    struct process_times *buffer;
    struct task_struct *task;
    int count = 0, i = 0;
    int ret = -EINVAL;

    // Copiar estructura base desde espacio de usuario
    if (copy_from_user(&klist, user_list, sizeof(struct process_time_list)))
        return -EFAULT;

    if (klist.max_processes <= 0 || !klist.times)
        return -EINVAL;

    // Contar cuántos procesos válidos hay
    for_each_process(task) {
        if (task->pid == 0)
            continue;
        count++;
    }

    count = min(count, klist.max_processes);

    buffer = kmalloc_array(count, sizeof(struct process_times), GFP_KERNEL);
    if (!buffer)
        return -ENOMEM;

    i = 0;
    rcu_read_lock();
    for_each_process(task) {
        if (i >= count)
            break;
        if (task->pid == 0)
            continue;

        buffer[i].pid = task->pid;
        get_task_comm(buffer[i].name, task);
        buffer[i].start_time = task->start_time / HZ;

        if (task->__state == EXIT_ZOMBIE || task->__state == EXIT_DEAD || task->exit_state != 0)
            buffer[i].end_time = ktime_get_boottime_seconds();
        else
            buffer[i].end_time = 0;

        i++;
    }
    rcu_read_unlock();

    // Guardar el número de procesos encontrados
    if (put_user(i, &user_list->num_processes)) {
        ret = -EFAULT;
        goto cleanup;
    }

    // Copiar datos al espacio de usuario
    if (copy_to_user(klist.times, buffer, i * sizeof(struct process_times))) {
        ret = -EFAULT;
        goto cleanup;
    }

    ret = 0;

cleanup:
    kfree(buffer);
    return ret;
}
