#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/keyboard.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static DECLARE_WAIT_QUEUE_HEAD(keypress_wait);
static int key_pressed = 0;
static int target_key = 0;

static void open_terminal(void)
{
    char *argv[] = { "/usr/bin/x-terminal-emulator", NULL };
    static char *envp[] = {
        "SHELL=/bin/bash",
        "HOME=/home/ajsivinac",
        "USER=ajsivinac",
        "PATH=/home/ajsivinac/bin:/home/ajsivinac/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/ajsivinac",
        "DISPLAY=:0",
        "PWD=/home/ajsivinac", 
        NULL};

    call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
}

// Manejador del teclado
static int key_event_handler(struct notifier_block *nb, unsigned long action, void *data) {
    struct keyboard_notifier_param *param = data;

    if (action == KBD_KEYSYM && param->down) {
        int detected_key = param->value & 0xFF;  // 🔹 Aseguramos que solo tomamos el keycode

        printk(KERN_INFO "[Syscall] Tecla detectada: %d (Esperando: %d)\n", detected_key, param->value);
        //printk(KERN_INFO "Key press: value=%d, shift=%d, ledstate=%d\n", param->value, param->shift, param->ledstate);
        if (param->value == 64353) {  // 🔹 Comparación con el código correcto
            printk(KERN_INFO "[Syscall] Tecla objetivo %d presionada, despertando syscall.\n", target_key);
            key_pressed = 1;
            wake_up(&keypress_wait);
        }
    }
    return NOTIFY_OK;
}

// Registramos el handler
static struct notifier_block nb = {
    .notifier_call = key_event_handler
};

/**
 * Syscall para escuchar una tecla específica
 * @param key_code: Código de la tecla a escuchar, de la estructura keyboard_notifier_param
 * @return 0 en éxito, error en caso contrario
 */
SYSCALL_DEFINE1(listen_key, int, keycode) {
    printk(KERN_INFO "[Syscall] Iniciando syscall wait_keypress con keycode: %d\n", keycode);
    target_key = keycode;
    key_pressed = 0;
    
    register_keyboard_notifier(&nb);

    // Esperamos hasta que se presione la tecla
    wait_event_interruptible(keypress_wait, key_pressed);

    open_terminal();
    printk(KERN_INFO "[Syscall] Tecla detectada, saliendo de wait_event_interruptible.\n");
    unregister_keyboard_notifier(&nb);
    printk(KERN_INFO "[Syscall] Notificador de teclado desregistrado.\n");
    
    return 0;
}