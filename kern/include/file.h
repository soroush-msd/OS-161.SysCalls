/*
 * Declarations for file handle and file table management.
 */

#ifndef _FILE_H_
#define _FILE_H_

/*
 * Contains some file-related maximum length constants
 */
#include <limits.h>


/*
 * Put your function declarations and data types here ...
 */

/* refer to main.c for initialisation */
struct OFTable_entry {
    struct vnode* v_node;
    off_t file_pointer;
    int flags;
    int fd_ref_count;
};

/* refer to proc.h/.c for declaration and initialisation */
struct FDTable_entry {
    struct OFTable_entry* of;
};


/* Declaring a shared global open file table of type struct FDTable */
/* refer to boot and shutdown in main.c for initilisation and freeing */
struct OFTable_entry *of_table;

int sys_open(const_userptr_t filename, int openflags, mode_t mode, int *fd_value);
int sys_close(int fd);
int sys_read(int fd, void *buf, size_t buflen, size_t *bytes_read);
int sys_write(int fd, userptr_t buf, size_t buflen, size_t *bytes_write);
int sys_dup2(int oldfd, int newfd, int *retval);
int sys_lseek(int fd, off_t pos, int whence, off_t *new_pos);

// helper function to attach stdin/out/err
int create_stds(void);

#endif /* _FILE_H_ */
