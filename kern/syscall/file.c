#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>
#include <copyinout.h>

/*
 * Add your file-related functions here ...
 */

int sys_open(const_userptr_t filename, int openflags, mode_t mode, int *fd_value) {

    int result;
    char kernFile[NAME_MAX];
    size_t kernFile_size;
    int next_fd;
    int next_of;

    // copy the file name from application stack into the kernel stack
    result = copyinstr(filename, kernFile, NAME_MAX, &kernFile_size);
    if(result != 0) { return result; }

    // find the next availabe file descriptor
    for(next_fd = 0; next_fd < OPEN_MAX; next_fd++) {
        if(curproc -> fd_table[next_fd].of == NULL) {
            break;
        }
    }

    // No empty fd entry
    if(next_fd == OPEN_MAX) { return EMFILE; }

    // find empty slot in open file table and assign the values
    for (next_of = 0; next_of < OPEN_MAX; next_of++) {
        if(of_table[next_of].fd_ref_count == 0) {
            curproc -> fd_table[next_fd].of = &(of_table[next_of]);
	        curproc -> fd_table[next_fd].of -> file_pointer = 0;
	        curproc -> fd_table[next_fd].of -> flags = openflags;
            curproc -> fd_table[next_fd].of -> fd_ref_count = 1;

            result = vfs_open(kernFile,curproc -> fd_table[next_fd].of -> flags, mode,
                        &(curproc -> fd_table[next_fd].of -> v_node));
            if(result != 0) { return result; }

            break;
        }
    }

    // No empty of entry
    if(next_of == OPEN_MAX) { return ENFILE; }

    *fd_value = next_fd;
    return 0;

}

int sys_close(int fd) {

    // bad fd number
    if( fd < 0 || fd >= OPEN_MAX) { return EBADF; }

    // inactive fd entry
    if(curproc -> fd_table[fd].of == NULL) { return EBADF; }

    // invalid vnode
    if(curproc -> fd_table[fd].of -> v_node == NULL) { return EBADF; }

    curproc -> fd_table[fd].of -> fd_ref_count--;

    if(curproc -> fd_table[fd].of -> fd_ref_count == 0) {
        vfs_close(curproc -> fd_table[fd].of -> v_node);
        curproc -> fd_table[fd].of -> file_pointer = -1;
	    curproc -> fd_table[fd].of -> flags = -1;
        curproc -> fd_table[fd].of -> v_node = NULL;
    }
    // inside or outside if?
    curproc -> fd_table[fd].of = NULL;

    return 0;
}


int sys_read(int fd, void *buf, size_t buflen, size_t *bytes_read) {

    struct iovec iov_read;
    struct uio uio_read;
    int read_result;

    // bad fd number
    if( fd < 0 || fd >= OPEN_MAX) { return EBADF; }

    // inactive fd entry
    if(curproc -> fd_table[fd].of == NULL) { return EBADF; }

    // invalid vnode
    if(curproc -> fd_table[fd].of -> v_node == NULL) { return EBADF; }

    // bad access
    if((curproc -> fd_table[fd].of -> flags & O_ACCMODE) == O_WRONLY) { return EBADF; }

    uio_uinit(&iov_read, &uio_read, buf, buflen,
            (curproc -> fd_table[fd].of -> file_pointer), UIO_READ);
    
    read_result = VOP_READ(curproc -> fd_table[fd].of -> v_node, &uio_read);
    if(read_result != 0) { return read_result; }

    curproc -> fd_table[fd].of -> file_pointer = uio_read.uio_offset;
    *bytes_read = buflen - uio_read.uio_resid;

    return 0;
}


int sys_write(int fd, userptr_t buf, size_t buflen, size_t *bytes_write) {

    struct iovec iov_write;
    struct uio uio_write;
    int write_result;

    // bad fd number
    if( fd < 0 || fd >= OPEN_MAX) { return EBADF; }

    // inactive fd entry
    if(curproc -> fd_table[fd].of == NULL) { return EBADF; }

    // invalid vnode
    if( curproc -> fd_table[fd].of -> v_node == NULL) { return EBADF; }

    // bad access
    if((curproc -> fd_table[fd].of -> flags & O_ACCMODE) == O_RDONLY) { return EBADF; }

    uio_uinit(&iov_write, &uio_write, buf, buflen,
            (curproc -> fd_table[fd].of -> file_pointer), UIO_WRITE);
    
    write_result = VOP_WRITE(curproc -> fd_table[fd].of -> v_node, &uio_write);
    if(write_result != 0) { return write_result; }

    curproc -> fd_table[fd].of -> file_pointer = uio_write.uio_offset;
    *bytes_write = buflen - uio_write.uio_resid;

    return 0;

}


int sys_dup2(int oldfd, int newfd, int *retval) {
    *retval = newfd;
    
    // check file descriptors are valid
    if (oldfd < 0 || oldfd >= OPEN_MAX || newfd < 0 || newfd >= OPEN_MAX) {
        return EBADF;
    }

    if(curproc -> fd_table[oldfd].of == NULL) {
        return EBADF;
    }

    if (oldfd == newfd) {
        return 0;
    }

    // if newfd is an already open file, close it
    if (curproc->fd_table[newfd].of != NULL) {
        int err = sys_close(newfd);
        if (err) {
            return err;
        }
    }

    curproc->fd_table[newfd].of = curproc->fd_table[oldfd].of;

    // update the reference count
    curproc->fd_table[newfd].of->fd_ref_count++;

    return 0;
}


int sys_lseek(int fd, off_t pos, int whence, off_t *new_pos) {
    // check fd is valid
    if (fd < 0 || fd >= OPEN_MAX) {
        return EBADF;
    }

    if(curproc -> fd_table[fd].of == NULL) {
        return EBADF;
    }

    // check fd refers to a seekable object
    if (!VOP_ISSEEKABLE(curproc->fd_table[fd].of->v_node)) {
        return ESPIPE;
    }

    // calculate new_pos
    if (whence == SEEK_SET) {
        *new_pos = pos;
    }
    else if (whence == SEEK_CUR) {
        *new_pos = curproc->fd_table[fd].of->file_pointer + pos;
    }
    else if (whence == SEEK_END) {
        struct stat s;
        VOP_STAT(curproc->fd_table[fd].of->v_node, &s);
        *new_pos = s.st_size + pos;
    }
    else {
        return EINVAL;
    }

    // check new_pos is valid and set file_pointer to new_pos
    if (*new_pos < 0) {
        return EINVAL;
    }
    else {
        curproc->fd_table[fd].of->file_pointer = *new_pos;
    }

    return 0;
}


int create_stds() {

    int stds_result;
    char conname[5];

    /* Attaching stdin */
    curproc -> fd_table[0].of = &(of_table[0]);
	curproc -> fd_table[0].of -> file_pointer = 0;
	curproc -> fd_table[0].of -> flags = O_RDONLY;
    curproc -> fd_table[0].of -> fd_ref_count = 1;

	strcpy(conname,"con:");
	stds_result = vfs_open(conname,curproc -> fd_table[0].of -> flags, 0,
                        &(curproc -> fd_table[0].of -> v_node));

	if(stds_result != 0) { return stds_result; }

    /* Attaching stdout */
    curproc -> fd_table[1].of = &(of_table[1]);
	curproc -> fd_table[1].of -> file_pointer = 0;
	curproc -> fd_table[1].of -> flags = O_WRONLY;
    curproc -> fd_table[1].of -> fd_ref_count = 1;

	strcpy(conname,"con:");
	stds_result = vfs_open(conname,curproc -> fd_table[1].of -> flags, 0,
                        &(curproc -> fd_table[1].of -> v_node));

	if(stds_result != 0) { return stds_result; }

    /* Attaching stderr */
    curproc -> fd_table[2].of = &(of_table[2]);
	curproc -> fd_table[2].of -> file_pointer = 0;
	curproc -> fd_table[2].of -> flags = O_WRONLY;
    curproc -> fd_table[2].of -> fd_ref_count = 1;

	strcpy(conname,"con:");
	stds_result = vfs_open(conname,curproc -> fd_table[2].of -> flags, 0,
                        &(curproc -> fd_table[2].of -> v_node));

	if(stds_result != 0) { return stds_result; }


    return 0;

}

