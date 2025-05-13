#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/cpumask.h>
#include <linux/kernel_stat.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/slab.h>
#include <linux/vmstat.h>

/* Estructura para almacenar la información del sistema que devuelve la syscall */
struct sysresources_info {
    unsigned long cpu_usage;     /* Uso de CPU en décimas de porcentaje (ej: 456 = 45.6%) */
    unsigned long ram_total;     /* RAM total en KB */
    unsigned long ram_used;      /* RAM usada en KB */
    unsigned long ram_usage;     /* Uso de RAM en décimas de porcentaje */
};

/* Variables estáticas para cálculo de CPU */
static u64 last_idle = 0;
static u64 last_total = 0;

/* 
 * Implementación de la syscall sysresources
 * Obtiene información sobre el uso actual de CPU y RAM
 * 
 * @param info: Puntero a estructura donde se almacenará la información
 * @return: 0 en caso de éxito, código de error en caso contrario
 */
SYSCALL_DEFINE1(sysresources, struct sysresources_info __user *, info)
{
    int cpu;
    u64 idle = 0, total = 0;
    u64 diff_total, diff_idle;
    struct sysinfo si;
    struct sysresources_info kinfo;
    unsigned long total_ram, free_ram, buffers, cached, available, used_ram;

    /* Verificar si el puntero de usuario es válido */
    if (!info)
        return -EINVAL;

    /* Calcular uso de CPU */
    for_each_online_cpu(cpu) {
        struct kernel_cpustat kcs = kcpustat_cpu(cpu);

        u64 user = kcs.cpustat[CPUTIME_USER];
        u64 nice = kcs.cpustat[CPUTIME_NICE];
        u64 system = kcs.cpustat[CPUTIME_SYSTEM];
        u64 irq = kcs.cpustat[CPUTIME_IRQ];
        u64 softirq = kcs.cpustat[CPUTIME_SOFTIRQ];
        u64 steal = kcs.cpustat[CPUTIME_STEAL];
        u64 idle_time = kcs.cpustat[CPUTIME_IDLE];
        u64 iowait = kcs.cpustat[CPUTIME_IOWAIT];

        idle += idle_time + iowait;
        total += user + nice + system + irq + softirq + steal + idle_time + iowait;
    }

    diff_total = total - last_total;
    diff_idle = idle - last_idle;

    last_total = total;
    last_idle = idle;

    if (diff_total > 0) {
        kinfo.cpu_usage = (1000 * (diff_total - diff_idle)) / diff_total;
    } else {
        kinfo.cpu_usage = 0;
    }

    /* Calcular uso de RAM */
    si_meminfo(&si);
    
    total_ram = si.totalram * si.mem_unit;
    free_ram = si.freeram * si.mem_unit;
    buffers = si.bufferram * si.mem_unit;
    
    cached = global_node_page_state(NR_FILE_PAGES) * PAGE_SIZE;
    available = si_mem_available() * PAGE_SIZE;
    
    used_ram = total_ram - available;
    
    kinfo.ram_total = total_ram / 1024;        /* Convertir a KB */
    kinfo.ram_used = used_ram / 1024;          /* Convertir a KB */
    kinfo.ram_usage = (used_ram * 1000) / total_ram;  /* En décimas de porcentaje */

    /* Copiar datos a espacio de usuario */
    if (copy_to_user(info, &kinfo, sizeof(struct sysresources_info)))
        return -EFAULT;

    return 0;
}