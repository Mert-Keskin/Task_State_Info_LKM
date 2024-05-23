#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>


#define MYBUF_SIZE 163840
#define MYDATA_SIZE 163840
static int status = 9; // kullanıcı girdisi için
struct my_data{
    int size;
    char *buf; /* my data starts here */
};

static const char * const task_state_array[] = {
    "R (running)",
    "S (sleeping)",
    "D (disk sleep)",
    "T (stopped)",
    "t (tracing stop)",
    "X (dead)",
    "Z (zombie)",
    "P (parked)",
    "I (idle)",
};
static const int reverse_task_state_func(char input){//my_write da kullandım.kullanıcının girdisini kıyaslama için sayıya donuşturmeye yarıyor. 
    if(input == 'R')
        return 0;
    else if(input == 'S')
        return 1;
    else if(input == 'D')
        return 2;
    else if(input == 'T')
        return 3;
    else if(input == 't')
        return 4;
    else if(input == 'X')
        return 5;
    else if(input == 'Z')
        return 6;
    else if(input == 'P')
        return 7;
    else if(input == 'I')
        return 8;
    else
        return 9;
}

/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief
 *
 * @param inode
 * @param file
 * @return int
 */
int my_open(struct inode *inode, struct file *file)
{
    struct my_data *my_data = kmalloc(sizeof(struct my_data) * MYBUF_SIZE, GFP_KERNEL);
    my_data->buf = kmalloc(sizeof(char) * MYBUF_SIZE, GFP_KERNEL);
    my_data->size = MYBUF_SIZE;

    my_data->size = snprintf(my_data->buf, MYBUF_SIZE,  "Hello World\n");

    /* validate access to data
    Not: diger fonksiyonlarda,
    private_datayi farkli adrese point ettirmeyin, 
    yada once my_datayi kfree ile free edin*/
    file->private_data = my_data;

    return 0;
}
/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 *
 * @param inode
 * @param file
 * @return int
 */
int my_release(struct inode *inode, struct file *file)
{
    /*free all memories*/
    struct my_data *my_data = file->private_data;
    kfree(my_data->buf);
    kfree(my_data);

    return 0;
}

/**
 * TODO: taskinfoya gore ayarlamaniz gerekiyor
 * @brief copy data from mydata->buffer to user_buf,
 * file: opened file data
 * usr_buf: usr buffer
 * offset: the cursor position on the mydata->buf from the last call
 * file:
 */
ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{
    struct my_data *my_data = (struct my_data *)file->private_data;

    if (!my_data || !usr_buf) {
        return -EFAULT;
    }

    int len = min((int)(my_data->size - *offset), (int)size);

    struct task_struct *task;

    for_each_process(task) {
        if ((MYBUF_SIZE - len) <= 500) {// buffer dolmaya yaklaşırsa diye.
            break; 
        }

        if(task_state_index(task) == status || status == 9){
            len += snprintf(my_data->buf + len, MYBUF_SIZE - len,
                        "pid = %d state = %s utime = %llu stime = %llu utime+stime = %llu vruntime = %llu\n",
                        task->pid, task_state_array[task_state_index(task)],
                        task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
        }

        if (len >= MYBUF_SIZE) {//bufferi geçmedigimden emin oluyorum.
            break;
        }
    }

    if (len <= 0) {
        return 0;
    }

    if (copy_to_user(usr_buf, my_data->buf + *offset, len)) {
        return -EFAULT;
    }

    *offset = *offset + len;

    return len; /*the number of bytes copied*/
}

ssize_t my_read_simple(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{

    char buf[MYBUF_SIZE] = {'\0'};

    int len = sprintf(buf, "Hello World\n");

    /* copy len byte to userspace usr_buf
     Returns number of bytes that could not be copied.
     On success, this will be zero.
    */
    if (copy_to_user(usr_buf, buf, len))
        return -EFAULT;

    return len; /*the number of bytes copied*/
}

/**
 * @brief TODO: task infoya gore ayarlamaniz gerekiyor
 *
 * @param file
 * @param usr_buf
 * @param size
 * @param offset
 * @return ssize_t
 */
ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
{
    char *buf = kmalloc(size + 1, GFP_KERNEL);

    /* copies user space usr_buf to kernel buffer */
    if (copy_from_user(buf, usr_buf, size)){
        printk(KERN_INFO "Error copying from user\n");
        return -EFAULT;
    }
    // *offset += size;
    /* yine offseti bazi durumlarda set etmeniz vs gerekebilir,
    user tekrar write yaptiginda buf+*offsete yaziyor */

    buf[size] = '\0';

    printk(KERN_INFO "the value of kernel buf: %s", buf);
    
    status = reverse_task_state_func(buf[0]);// userden girdi alınmış oldu ve status degiskenine atandı.

    kfree(buf);
    return size;
}
