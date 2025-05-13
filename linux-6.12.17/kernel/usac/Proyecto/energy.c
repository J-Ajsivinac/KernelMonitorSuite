#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/time.h>
/* Eliminamos linux/cputime.h que ya no existe */

struct energy_usage {
    pid_t pid;                  /* ID del proceso */
    unsigned long cpu_usage;    /* Tiempo de CPU en jiffies */
    unsigned long ram_usage;    /* Uso de RAM en KB */
    unsigned long io_read;      /* Bytes leídos de I/O */
    unsigned long io_write;     /* Bytes escritos a I/O */
    unsigned long energy_est;   /* Estimación de energía (unidades arbitrarias) */
};

SYSCALL_DEFINE3(energy_usage, struct energy_usage __user *, buffer, 
                unsigned int, max_entries, unsigned int __user *, count_ret)
{
    struct task_struct *task;
    struct energy_usage *kernel_buffer;
    unsigned int count = 0;
    unsigned long cpu_weight = 13;     /* Factor de peso para CPU */
    unsigned long ram_weight = 5;      /* Factor de peso para RAM */
    unsigned long io_weight = 3;       /* Factor de peso para I/O */
    int ret = 0;
    unsigned long energy_est;
    
    /* Validar parámetros */
    if (!buffer || max_entries == 0 || !count_ret)
        return -EINVAL;
    
    /* Asignar buffer temporal en espacio de kernel */
    kernel_buffer = kmalloc_array(max_entries, sizeof(struct energy_usage), GFP_KERNEL);
    if (!kernel_buffer)
        return -ENOMEM;
    
    /* Recorrer la lista de procesos */
    rcu_read_lock();
    for_each_process(task) {
        /* Detener si alcanzamos el límite */
        if (count >= max_entries)
            break;
        
        /* En kernels modernos, usamos task->utime y task->stime directamente */
        unsigned long cpu_usage = task->utime + task->stime;
        unsigned long ram_usage = 0;
        unsigned long io_read = 0;
        unsigned long io_write = 0;
        
        /* Uso de RAM */
        if (task->mm) {
            ram_usage = get_mm_rss(task->mm) << PAGE_SHIFT >> 10;  /* KB */
        }
        
        /* En kernels 6.x, usamos la interfaz io_accounting para estadísticas IO */
        io_read = task->ioac.read_bytes;
        io_write = task->ioac.write_bytes;
        
        /* Calcular estimación de energía preliminar */
        energy_est = ((cpu_usage/1000000) * cpu_weight) +
                     (ram_usage * ram_weight) +
                     ((io_read + io_write) * io_weight);
        
        /* Solo incluir procesos con energía > 0 */
        if (energy_est > 0) {
            kernel_buffer[count].pid = task_pid_nr(task);
            kernel_buffer[count].cpu_usage = cpu_usage;
            kernel_buffer[count].ram_usage = ram_usage;
            kernel_buffer[count].io_read = io_read;
            kernel_buffer[count].io_write = io_write;
            kernel_buffer[count].energy_est = energy_est;
            count++;
        }
    }
    rcu_read_unlock();
    
    /* Copiar datos al espacio de usuario */
    if (copy_to_user(buffer, kernel_buffer, count * sizeof(struct energy_usage))) {
        ret = -EFAULT;
        goto out;
    }
    
    /* Devolver el número de procesos */
    if (copy_to_user(count_ret, &count, sizeof(unsigned int))) {
        ret = -EFAULT;
        goto out;
    }
    
out:
    kfree(kernel_buffer);
    return ret;
}