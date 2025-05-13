#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/dirent.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/namei.h>
#include <linux/mutex.h>
#include <linux/fsnotify_backend.h>
#include <linux/inotify.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/atomic.h>

/* Declaración incompleta para cumplir con la firma de notify_change */
struct mnt_idmap;

#define PATH_MAX 4096
#define BUF_SIZE 256

/* Mutex global para evitar condiciones de carrera durante la sincronización */
static DEFINE_MUTEX(sync_mutex);

/* Variables globales para FS-notify y monitorización */
static struct fsnotify_group *global_group = NULL;
static LIST_HEAD(monitor_list);
static struct task_struct *sync_thread = NULL;

/* Estructura que almacena cada entrada de monitorización y sincronización */
struct monitor_entry {
    struct list_head list;
    char *dir;                  /* Ruta del directorio */
    char *other;                /* Ruta del directorio contrario (donde se sincronizará) */
    struct fsnotify_mark *mark; /* Mark asociado para FS-notify */
    struct inode *inode;        /* Inodo del directorio */
    atomic_t dirty;             /* Flag que indica que se detectó un evento */
};

/* Función auxiliar para copiar un archivo de src_file_path a dst_file_path */
/* Tras copiar, se actualizan los atributos de propietario y grupo */
static int copy_single_file(const char *src_file_path, const char *dst_file_path)
{
    struct file *in = NULL, *out = NULL;
    char *buf;
    ssize_t bytes;
    loff_t pos_in = 0, pos_out = 0;
    int ret = 0;

    buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!buf)
         return -ENOMEM;

    in = filp_open(src_file_path, O_RDONLY, 0);
    if (IS_ERR(in)) {
         ret = PTR_ERR(in);
         goto out;
    }

    /* Se abre (o crea) el archivo destino usando el mismo modo que el origen */
    out = filp_open(dst_file_path, O_WRONLY | O_CREAT | O_TRUNC, in->f_inode->i_mode);
    if (IS_ERR(out)) {
         ret = PTR_ERR(out);
         goto out_close_in;
    }

    while ((bytes = kernel_read(in, buf, PAGE_SIZE, &pos_in)) > 0) {
         ssize_t written = kernel_write(out, buf, bytes, &pos_out);
         if (written < 0) {
             ret = written;
             break;
         }
    }

    /* Si la copia fue exitosa, se actualizan el propietario y el grupo del archivo destino */
    if (ret >= 0) {
         struct iattr attr;
         memset(&attr, 0, sizeof(attr));
         attr.ia_valid = ATTR_UID | ATTR_GID;
         attr.ia_uid = in->f_inode->i_uid;
         attr.ia_gid = in->f_inode->i_gid;
         ret = notify_change((struct mnt_idmap *)out->f_path.mnt, out->f_path.dentry, &attr, 0);
         if (ret < 0) {
            if (ret == -EOVERFLOW) {
                pr_warn("EOVERFLOW al actualizar atributos de %s, se ignora\n", dst_file_path);
                out->f_inode->i_uid = in->f_inode->i_uid;
                out->f_inode->i_gid = in->f_inode->i_gid;
                mark_inode_dirty(out->f_inode);
                ret = 0;
            } else {
                pr_err("Error al actualizar atributos de %s: %d\n", dst_file_path, ret);
            }
        }
    }

    filp_close(out, NULL);
out_close_in:
    filp_close(in, NULL);
out:
    kfree(buf);
    return ret;
}

/* Estructura de contexto para la iteración de directorios para copia */
struct copy_ctx {
    struct dir_context dctx;
    char *src_path;
    char *dst_path;
    int error;
};

/* Callback que se invoca por cada entrada encontrada en el directorio y copia los archivos regulares */
static bool copy_file_cb(struct dir_context *ctx, const char *name, int namelen,
                          loff_t offset, u64 ino, unsigned int d_type)
{
    struct copy_ctx *cctx = container_of(ctx, struct copy_ctx, dctx);
    char *src_file_path, *dst_file_path;
    int ret;

    if ((namelen == 1 && name[0] == '.') ||
        (namelen == 2 && name[0] == '.' && name[1] == '.'))
         return true;

    if (d_type != DT_REG)
         return true;

    src_file_path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!src_file_path) {
         cctx->error = -ENOMEM;
         return true;
    }
    dst_file_path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!dst_file_path) {
         kfree(src_file_path);
         cctx->error = -ENOMEM;
         return true;
    }

    snprintf(src_file_path, PATH_MAX, "%s/%.*s", cctx->src_path, namelen, name);
    snprintf(dst_file_path, PATH_MAX, "%s/%.*s", cctx->dst_path, namelen, name);

    ret = copy_single_file(src_file_path, dst_file_path);
    if (ret < 0) {
         cctx->error = ret;
         pr_err("Error copying file %s: %d\n", name, ret);
    }

    kfree(src_file_path);
    kfree(dst_file_path);
    return true;
}

/* Función que sincroniza el contenido de src_path a dst_path (copiando archivos) */
static int sync_dir(const char *src_path, const char *dst_path)
{
    int ret;
    struct file *src_dir = NULL;
    struct copy_ctx ctx;

    src_dir = filp_open(src_path, O_RDONLY | O_DIRECTORY, 0);
    if (IS_ERR(src_dir)) {
         ret = PTR_ERR(src_dir);
         return ret;
    }
    ctx.src_path = (char *)src_path;
    ctx.dst_path = (char *)dst_path;
    ctx.error = 0;
    ctx.dctx.actor = copy_file_cb;
    ctx.dctx.pos = 0;
    ret = iterate_dir(src_dir, &ctx.dctx);
    filp_close(src_dir, NULL);
    return (ret < 0) ? ret : ctx.error;
}

/* Nueva estructura para el contexto de eliminación */
struct delete_ctx {
    struct dir_context dctx;
    const char *src_path;
    const char *dst_path;
    struct file *dst_dir;
    int error;
};

/* Callback para iterar en el directorio destino y eliminar archivos no presentes en el origen */
static bool delete_file_cb(struct dir_context *ctx, const char *name, int namelen,
                             loff_t offset, u64 ino, unsigned int d_type)
{
    struct delete_ctx *dctx = container_of(ctx, struct delete_ctx, dctx);
    char *src_file_path;
    int ret;

    /* Ignorar entradas "." y ".." */
    if ((namelen == 1 && name[0] == '.') ||
        (namelen == 2 && name[0] == '.' && name[1] == '.'))
         return true;

    /* Solo procesamos archivos regulares */
    if (d_type != DT_REG)
         return true;

    src_file_path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!src_file_path) {
         dctx->error = -ENOMEM;
         return true;
    }
    snprintf(src_file_path, PATH_MAX, "%s/%.*s", dctx->src_path, namelen, name);

    /* Verificar si el archivo existe en el directorio origen */
    {
        struct path path;
        ret = kern_path(src_file_path, LOOKUP_FOLLOW, &path);
        if (ret == 0) {
            /* El archivo existe, no se elimina */
            path_put(&path);
            kfree(src_file_path);
            return true;
        }
    }
    kfree(src_file_path);

    /* Si el archivo no existe en el origen, se elimina del destino */
    {
        struct dentry *child;
        child = lookup_one_len(name, dctx->dst_dir->f_path.dentry, namelen);
        if (IS_ERR(child))
            return true;
        ret = vfs_unlink(dctx->dst_dir->f_path.mnt->mnt_idmap, dctx->dst_dir->f_inode, child, NULL);
        if (ret < 0) {
            if (ret == -EOVERFLOW) {
                pr_warn("EOVERFLOW al eliminar %s en %s, se ignora\n", name, dctx->dst_path);
                ret = 0;
            } else {
                pr_err("Error al eliminar %s en %s: %d\n", name, dctx->dst_path, ret);
                dctx->error = ret;
            }
        } else {
            printk(KERN_INFO "Eliminado %s en %s\n", name, dctx->dst_path);
        }
        dput(child);
    }
    return true;
}

/* Función que sincroniza las eliminaciones: elimina del destino archivos que no existen en el origen */
static int sync_dir_deletions(const char *src_path, const char *dst_path)
{
    int ret;
    struct file *dst_dir = filp_open(dst_path, O_RDONLY | O_DIRECTORY, 0);
    if (IS_ERR(dst_dir)) {
         ret = PTR_ERR(dst_dir);
         return ret;
    }
    struct delete_ctx dctx;
    dctx.src_path = src_path;
    dctx.dst_path = dst_path;
    dctx.dst_dir = dst_dir;
    dctx.error = 0;
    dctx.dctx.actor = delete_file_cb;
    dctx.dctx.pos = 0;
    ret = iterate_dir(dst_dir, &dctx.dctx);
    filp_close(dst_dir, NULL);
    return (ret < 0) ? ret : dctx.error;
}

/* Callback de FS-notify: al recibir un evento se marca la entrada correspondiente como "dirty" */
static int my_fsnotify_handle_event(struct fsnotify_group *group,
                                    u32 mask,
                                    const void *data,
                                    int data_type,
                                    struct inode *inode,
                                    const struct qstr *name,
                                    u32 cookie,
                                    struct fsnotify_iter_info *iter_info)
{
    struct monitor_entry *entry;
    bool found = false;

    list_for_each_entry(entry, &monitor_list, list) {
        if (entry->inode == inode) {
            found = true;
            atomic_set(&entry->dirty, 1);
            printk(KERN_INFO "Evento en el directorio: %s\n", entry->dir);
            break;
        }
    }

    if (!found)
        printk(KERN_INFO "Evento en directorio no registrado (inode: %lu)\n", inode->i_ino);

    if (mask & IN_CREATE)
        printk(KERN_INFO "fsnotify: CREACIÓN (inode: %lu)\n", inode->i_ino);
    if (mask & IN_DELETE)
        printk(KERN_INFO "fsnotify: ELIMINACIÓN (inode: %lu)\n", inode->i_ino);
    if (mask & IN_MODIFY)
        printk(KERN_INFO "fsnotify: MODIFICACIÓN (inode: %lu)\n", inode->i_ino);

    return 0;
}

static struct fsnotify_ops my_fsnotify_ops = {
    .handle_event = my_fsnotify_handle_event,
};

/* Hilo de kernel que, periódicamente, revisa si algún directorio está "dirty"
 * y sincroniza la carpeta modificada con la otra.
 */
static int real_time_sync_thread_fn(void *data)
{
    struct monitor_entry *entry;
    int ret;

    while (!kthread_should_stop()) {
        list_for_each_entry(entry, &monitor_list, list) {
            if (atomic_read(&entry->dirty)) {
                mutex_lock(&sync_mutex);
                printk(KERN_INFO "Sincronizando de %s a %s\n", entry->dir, entry->other);
                ret = sync_dir(entry->dir, entry->other);
                if (ret < 0)
                    printk(KERN_ERR "Error al sincronizar (copiar) %s -> %s: %d\n", entry->dir, entry->other, ret);
                /* Nueva llamada para eliminar archivos borrados en el origen */
                ret = sync_dir_deletions(entry->dir, entry->other);
                if (ret < 0)
                    printk(KERN_ERR "Error al sincronizar (eliminar) %s -> %s: %d\n", entry->dir, entry->other, ret);
                mutex_unlock(&sync_mutex);
                atomic_set(&entry->dirty, 0);
            }
        }
        msleep(500);
    }
    return 0;
}

/*
 * Syscall: real_time_sync
 * Recibe dos rutas de directorios y queda bloqueada, monitoreando ambos.
 * Cuando se detecta un evento en uno de ellos, se sincroniza con la otra carpeta.
 */
SYSCALL_DEFINE2(real_time_sync, const char __user *, dir_path1, const char __user *, dir_path2)
{
    char *kbuf1 = NULL, *kbuf2 = NULL;
    struct path path;
    int ret;
    struct inode *dir_inode;
    struct monitor_entry *entry1, *entry2;
    u32 monitor_mask = IN_CREATE | IN_DELETE | IN_MODIFY;

    kbuf1 = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!kbuf1)
        return -ENOMEM;
    if (copy_from_user(kbuf1, dir_path1, BUF_SIZE - 1)) {
        kfree(kbuf1);
        return -EFAULT;
    }
    kbuf1[BUF_SIZE - 1] = '\0';

    kbuf2 = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!kbuf2) {
        kfree(kbuf1);
        return -ENOMEM;
    }
    if (copy_from_user(kbuf2, dir_path2, BUF_SIZE - 1)) {
        kfree(kbuf1);
        kfree(kbuf2);
        return -EFAULT;
    }
    kbuf2[BUF_SIZE - 1] = '\0';

    if (!global_group) {
        global_group = fsnotify_alloc_group(&my_fsnotify_ops, 0);
        if (IS_ERR(global_group)) {
            ret = PTR_ERR(global_group);
            global_group = NULL;
            kfree(kbuf1);
            kfree(kbuf2);
            return ret;
        }
    }

    ret = kern_path(kbuf1, LOOKUP_FOLLOW, &path);
    if (ret) {
        kfree(kbuf1);
        kfree(kbuf2);
        return ret;
    }
    if (!S_ISDIR(path.dentry->d_inode->i_mode)) {
        printk(KERN_INFO "real_time_sync: %s no es un directorio.\n", kbuf1);
        path_put(&path);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOTDIR;
    }
    dir_inode = path.dentry->d_inode;
    entry1 = kzalloc(sizeof(*entry1), GFP_KERNEL);
    if (!entry1) {
        path_put(&path);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    entry1->dir = kstrdup(kbuf1, GFP_KERNEL);
    entry1->other = kstrdup(kbuf2, GFP_KERNEL);
    if (!entry1->dir || !entry1->other) {
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        path_put(&path);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    atomic_set(&entry1->dirty, 0);
    entry1->mark = kzalloc(sizeof(*(entry1->mark)), GFP_KERNEL);
    if (!entry1->mark) {
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        path_put(&path);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    entry1->mark->mask = monitor_mask;
    entry1->mark->group = global_group;
    entry1->inode = dir_inode;
    ret = fsnotify_add_mark(entry1->mark, dir_inode, FSNOTIFY_OBJ_TYPE_INODE, 0);
    if (ret) {
        printk(KERN_INFO "real_time_sync: Error agregando mark para %s\n", kbuf1);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        path_put(&path);
        kfree(kbuf1);
        kfree(kbuf2);
        return ret;
    }
    list_add_tail(&entry1->list, &monitor_list);
    path_put(&path);

    ret = kern_path(kbuf2, LOOKUP_FOLLOW, &path);
    if (ret) {
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return ret;
    }
    if (!S_ISDIR(path.dentry->d_inode->i_mode)) {
        printk(KERN_INFO "real_time_sync: %s no es un directorio.\n", kbuf2);
        path_put(&path);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOTDIR;
    }
    dir_inode = path.dentry->d_inode;
    entry2 = kzalloc(sizeof(*entry2), GFP_KERNEL);
    if (!entry2) {
        path_put(&path);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    entry2->dir = kstrdup(kbuf2, GFP_KERNEL);
    entry2->other = kstrdup(kbuf1, GFP_KERNEL);
    if (!entry2->dir || !entry2->other) {
        kfree(entry2->dir);
        kfree(entry2->other);
        kfree(entry2);
        path_put(&path);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    atomic_set(&entry2->dirty, 0);
    entry2->mark = kzalloc(sizeof(*(entry2->mark)), GFP_KERNEL);
    if (!entry2->mark) {
        kfree(entry2->dir);
        kfree(entry2->other);
        kfree(entry2);
        path_put(&path);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return -ENOMEM;
    }
    entry2->mark->mask = monitor_mask;
    entry2->mark->group = global_group;
    entry2->inode = dir_inode;
    ret = fsnotify_add_mark(entry2->mark, dir_inode, FSNOTIFY_OBJ_TYPE_INODE, 0);
    if (ret) {
        printk(KERN_INFO "real_time_sync: Error agregando mark para %s\n", kbuf2);
        kfree(entry2->mark);
        kfree(entry2->dir);
        kfree(entry2->other);
        kfree(entry2);
        path_put(&path);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        kfree(kbuf1);
        kfree(kbuf2);
        return ret;
    }
    list_add_tail(&entry2->list, &monitor_list);
    path_put(&path);

    kfree(kbuf1);
    kfree(kbuf2);

    sync_thread = kthread_run(real_time_sync_thread_fn, NULL, "real_time_sync_thread");
    if (IS_ERR(sync_thread)) {
        ret = PTR_ERR(sync_thread);
        fsnotify_destroy_mark(entry1->mark, global_group);
        list_del(&entry1->list);
        kfree(entry1->mark);
        kfree(entry1->dir);
        kfree(entry1->other);
        kfree(entry1);
        fsnotify_destroy_mark(entry2->mark, global_group);
        list_del(&entry2->list);
        kfree(entry2->mark);
        kfree(entry2->dir);
        kfree(entry2->other);
        kfree(entry2);
        return ret;
    }

    printk(KERN_INFO "real_time_sync: Monitorización y sincronización iniciadas.\n");

    set_current_state(TASK_INTERRUPTIBLE);
    while (!signal_pending(current))
        schedule();

    if (sync_thread) {
        kthread_stop(sync_thread);
        sync_thread = NULL;
    }
    while (!list_empty(&monitor_list)) {
        struct monitor_entry *entry, *tmp;
        list_for_each_entry_safe(entry, tmp, &monitor_list, list) {
            fsnotify_destroy_mark(entry->mark, global_group);
            list_del(&entry->list);
            kfree(entry->mark);
            kfree(entry->dir);
            kfree(entry->other);
            kfree(entry);
        }
    }
    if (global_group) {
        fsnotify_put_group(global_group);
        global_group = NULL;
    }

    printk(KERN_INFO "real_time_sync: Monitorización detenida.\n");
    return 0;
}
