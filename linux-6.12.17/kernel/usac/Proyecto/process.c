#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/jiffies.h>

#define MAX_USERNAME_LEN 32

#ifndef HZ
#define HZ 100
#endif

// Estructuras necesarias
struct process_info {
    pid_t pid;
    char name[TASK_COMM_LEN];
    unsigned int cpu_percent;
    unsigned int ram_percent;
    int priority;
    char state_str[20];
    long state;
    uid_t uid;
    int num_threads;
    unsigned long start_time;
};

struct process_list {
    int max_processes;
    int num_processes;
    struct process_info *processes;
};

struct cpu_snapshot {
    pid_t pid;
    u64 last_time;
};


// Variables para almacenar instantáneas de uso de CPU
static DEFINE_SPINLOCK(cpu_usage_lock);
static struct cpu_usage {
    pid_t pid;
    u64 last_total_time;
    u64 last_update_jiffies;
} *cpu_usage_array;
static int cpu_usage_array_size = 0;
static int max_cpu_usage_entries = 1024;

// Convertir estado del proceso a string
static void get_task_state_str(long state, char *state_str, size_t size) {
    if (state & TASK_RUNNING)
        strncpy(state_str, "Running", size);
    else if (state & TASK_INTERRUPTIBLE)
        strncpy(state_str, "Sleeping", size);
    else if (state & TASK_UNINTERRUPTIBLE)
        strncpy(state_str, "Disk Sleep", size);
    else if (state & __TASK_STOPPED)
        strncpy(state_str, "Stopped", size);
    else if (state & __TASK_TRACED)
        strncpy(state_str, "Tracing", size);
    else if (state & EXIT_DEAD)
        strncpy(state_str, "Dead", size);
    else if (state & EXIT_ZOMBIE)
        strncpy(state_str, "Zombie", size);
    else if (state & TASK_DEAD)
        strncpy(state_str, "Wakekill", size);
    else if (state & TASK_NOLOAD)
        strncpy(state_str, "No Load", size);
    else
        strncpy(state_str, "Unknown", size);

    state_str[size - 1] = '\0';
}


static int initialize_cpu_usage_array(void)
{
    if (cpu_usage_array)
        return 0;
    
    cpu_usage_array = kmalloc_array(max_cpu_usage_entries, sizeof(struct cpu_usage), GFP_KERNEL);
    if (!cpu_usage_array)
        return -ENOMEM;
    
    memset(cpu_usage_array, 0, max_cpu_usage_entries * sizeof(struct cpu_usage));
    cpu_usage_array_size = max_cpu_usage_entries;
    return 0;
}

// Función para limpiar el array de uso de CPU
static void cleanup_cpu_usage_array(void)
{
    if (cpu_usage_array) {
        kfree(cpu_usage_array);
        cpu_usage_array = NULL;
        cpu_usage_array_size = 0;
    }
}

// Obtener el tiempo total de CPU de un proceso (user + system)
static u64 get_task_cpu_time(struct task_struct *task)
{
    u64 utime, stime;
    struct task_struct *t;
    
    utime = task->utime;
    stime = task->stime;
    
    // Acumular tiempo de todos los hilos
    if (thread_group_leader(task)) {
        struct task_struct *thread;
        rcu_read_lock();
        for_each_thread(task, thread) {
            if (thread == task)
                continue;
            
            utime += thread->utime;
            stime += thread->stime;
        }
        rcu_read_unlock();
    }
    
    return utime + stime;
}

static unsigned int get_num_cpus(void)
{
    return num_online_cpus();
}


/**
 * Calcula el porcentaje de CPU utilizado por un proceso, similar a top/htop
 * @param task Estructura del proceso a evaluar
 * @return Porcentaje de CPU * 100 (ej: 1234 = 12.34%)
 */
static unsigned int calculate_cpu_percent(struct task_struct *task)
{
    int i;
    u64 total_time, delta_time;
    u64 current_jiffies, delta_jiffies;
    unsigned int percent = 0;
    unsigned int num_cpus;
    int found = 0;
    
    if (!task || task->pid == 0)
        return 0;
    
    // Inicializar array si aún no se ha hecho
    if (!cpu_usage_array && initialize_cpu_usage_array() != 0)
        return 0;
    
    // Obtener tiempo actual del proceso
    total_time = get_task_cpu_time(task);
    current_jiffies = jiffies;
    num_cpus = get_num_cpus();
    
    spin_lock(&cpu_usage_lock);
    
    // Buscar si ya teníamos datos de este proceso
    for (i = 0; i < cpu_usage_array_size; i++) {
        if (cpu_usage_array[i].pid == task->pid) {
            found = 1;
            
            // Calcular el delta de tiempo
            delta_time = total_time - cpu_usage_array[i].last_total_time;
            delta_jiffies = current_jiffies - cpu_usage_array[i].last_update_jiffies;
            
            // Actualizar para la próxima medición
            cpu_usage_array[i].last_total_time = total_time;
            cpu_usage_array[i].last_update_jiffies = current_jiffies;
            
            if (delta_jiffies > 0) {
                // Calcular el porcentaje (multiplicado por 100 para tener dos decimales)
                // total_time está en nanosegundos, convertir jiffies a nanosegundos también
                // (HZ jiffies = 1 segundo = 1,000,000,000 nanosegundos)
                u64 jiffies_to_nsec = delta_jiffies * (100000000ULL / HZ);
                if (jiffies_to_nsec > 0) {
                    // Dividir por número de CPUs para obtener valor similar a top/htop
                    // top/htop muestra el porcentaje relativo al total de CPUs disponibles
                    percent = div64_u64(delta_time * 10000ULL, jiffies_to_nsec);
                    percent = div_u64(percent, num_cpus);
                }
            }
            break;
        }
    }
    
    // Si no se encontró, buscar una entrada libre o reutilizar la más antigua
    if (!found) {
        int oldest_idx = 0;
        u64 oldest_time = current_jiffies + 1;
        
        for (i = 0; i < cpu_usage_array_size; i++) {
            if (cpu_usage_array[i].pid == 0) {
                oldest_idx = i;
                break;
            }
            
            if (cpu_usage_array[i].last_update_jiffies < oldest_time) {
                oldest_time = cpu_usage_array[i].last_update_jiffies;
                oldest_idx = i;
            }
        }
        
        // Registrar para la próxima medición
        cpu_usage_array[oldest_idx].pid = task->pid;
        cpu_usage_array[oldest_idx].last_total_time = total_time;
        cpu_usage_array[oldest_idx].last_update_jiffies = current_jiffies;
    }
    
    spin_unlock(&cpu_usage_lock);
    return percent;
}


// Calcular porcentaje de RAM usado por el proceso
static unsigned int calculate_ram_percent(struct task_struct *task) {
    struct mm_struct *mm;
    unsigned long task_pages = 0;
    unsigned long total_pages;
    unsigned int percent;

    mm = task->mm;
    if (mm) {
        task_pages = get_mm_rss(mm) + get_mm_counter(mm, MM_SWAPENTS);
    }

    total_pages = totalram_pages();
    if (total_pages == 0)
        return 0;

    percent = (unsigned int)((task_pages * 10000ULL) / total_pages);
    return percent;
}

// Llenar los datos del proceso
static void get_process_metrics(struct task_struct *task, struct process_info *info) {
    info->pid = task->pid;
    get_task_comm(info->name, task);
    info->priority = task->prio;
    info->state = task_state_index(task);
    get_task_state_str(task->__state, info->state_str, sizeof(info->state_str));
    info->uid = task->cred->uid.val;
    info->num_threads = get_nr_threads(task);
    info->start_time = task->start_time;
    info->cpu_percent = calculate_cpu_percent(task);
    info->ram_percent = calculate_ram_percent(task);
}

// SYSCALL principal
SYSCALL_DEFINE1(detailed_process_list, struct process_list __user *, user_list) {
    struct process_list kernel_list;
    struct process_info *buffer = NULL;
    struct cpu_snapshot *snapshots = NULL;
    struct task_struct *task;
    int count = 0, i = 0;
    int ret = -EINVAL;
    char task_name[TASK_COMM_LEN];

    if (copy_from_user(&kernel_list, user_list, sizeof(struct process_list)))
        return -EFAULT;

    if (kernel_list.max_processes <= 0 || !kernel_list.processes)
        return -EINVAL;

    for_each_process(task) {
        if (task->pid == 0)
            continue;
        get_task_comm(task_name, task);
        if (task_name[0] == '\0')
            continue;
        count++;
    }

    count = min(count, kernel_list.max_processes);

    buffer = kmalloc_array(count, sizeof(struct process_info), GFP_KERNEL);
    if (!buffer)
        return -ENOMEM;

    snapshots = kmalloc_array(count, sizeof(struct cpu_snapshot), GFP_KERNEL);
    if (!snapshots) {
        kfree(buffer);
        return -ENOMEM;
    }

    i = 0;
    for_each_process(task) {
        if (i >= count)
            break;
        if (task->pid == 0)
            continue;
        get_task_comm(task_name, task);
        if (task_name[0] == '\0')
            continue;
        get_process_metrics(task, &buffer[i]);
        i++;
    }

    kernel_list.num_processes = i;

    if (put_user(i, &user_list->num_processes)) {
        ret = -EFAULT;
        goto cleanup;
    }

    if (copy_to_user(kernel_list.processes, buffer, i * sizeof(struct process_info))) {
        ret = -EFAULT;
        goto cleanup;
    }

    ret = 0;

cleanup:
    kfree(buffer);
    kfree(snapshots);
    return ret;
}

SYSCALL_DEFINE2(get_process_by_pid, pid_t, pid, struct process_info __user *, user_info) {
    struct task_struct *task;
    struct process_info info;

    if (!user_info)
        return -EINVAL;

    // Buscar el proceso por PID
    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        rcu_read_unlock();
        return -ESRCH; // No existe el proceso con ese PID
    }

    // Obtener la info del proceso
    get_process_metrics(task, &info);
    rcu_read_unlock();

    // Copiar la información al espacio de usuario
    if (copy_to_user(user_info, &info, sizeof(struct process_info)))
        return -EFAULT;

    return 0;
}

