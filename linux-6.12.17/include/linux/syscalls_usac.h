#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/types.h>

// Código de la llamada al sistema en kernel/usac/practica1/syscall_1.c
asmlinkage long sys_get_cpu_time(pid_t pid);

// Código de la llamada al sistema en kernel/usac/practica1/syscall_2.c
asmlinkage long sys_listen_key(char key);

// Código de la llamada al sistema en kernel/usac/practica1/syscall_3.c
asmlinkage long sys_get_cpu_temp(void);

// Código de la llamada al sistema en kernel/usac/practica2/syscall_1.c
asmlinkage long sys_real_time_sync(void);

// Código de la llamada al sistema en kernel/usac/Proyecto/process.c
asmlinkage long sys_detailed_process_list(struct process_list __user *user_list);
asmlinkage long sys_get_process_by_pid(pid_t pid, struct process_info __user *user_info);

// Código de la llamada al sistema en kernel/usac/Proyecto/energy.c
asmlinkage long sys_energy_usage(struct energy_usage __user *buffer, unsigned int max_entries, unsigned int __user *count_ret);

// Código de la llamada al sistema en kernel/usac/Proyecto/kill_control.c
asmlinkage long sys_kill_control(pid_t pid, int signal);

// Código de la llamada al sistema en kernel/usac/Proyecto/network.c
asmlinkage long sys_get_system_network_usage(struct net_usage_info __user *user_info);

// Código de la llamada al sistema en kernel/usac/Proyecto/resume.c
asmlinkage long sys_sysresources(struct sysresources_info __user *info);

// Código de la llamada al sistema en kernel/usac/Proyecto/time.c
asmlinkage long sys_get_all_process_times(struct process_time_list __user *user_list);


#endif

