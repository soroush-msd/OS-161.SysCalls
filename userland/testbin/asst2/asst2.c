#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#define OPEN_MAX 128
#define MAX_BUF 500
char teststr[] = "The quick brown fox jumped over the lazy dog.";
char buf[MAX_BUF];

void additional_asst2_tests(void);

int
main(int argc, char * argv[])
{
        int fd, r, i, j , k;
        (void) argc;
        (void) argv;

        printf("\n**********\n* File Tester\n");

        snprintf(buf, MAX_BUF, "**********\n* write() works for stdout\n");
        write(1, buf, strlen(buf));
        snprintf(buf, MAX_BUF, "**********\n* write() works for stderr\n");
        write(2, buf, strlen(buf));

        printf("**********\n* opening new file \"test.file\"\n");
        fd = open("test.file", O_RDWR | O_CREAT, 0600); /* mode u=rw in octal */
        printf("* open() got fd %d\n", fd);
        if (fd < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* writing test string\n");
        r = write(fd, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* writing test string again\n");
        r = write(fd, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }
        printf("* closing file\n");
        close(fd);

        printf("**********\n* opening old file \"test.file\"\n");
        fd = open("test.file", O_RDONLY);
        printf("* open() got fd %d\n", fd);
        if (fd < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading entire file into buffer \n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", MAX_BUF -i);
                r = read(fd, &buf[i], MAX_BUF - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < MAX_BUF && r > 0);

        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }
        k = j = 0;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = k % r;
        } while (k < i);
        printf("* file content okay\n");

        printf("**********\n* testing lseek\n");
        r = lseek(fd, 5, SEEK_SET);
        if (r < 0) {
                printf("ERROR lseek: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading 10 bytes of file into buffer \n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", 10 - i );
                r = read(fd, &buf[i], 10 - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < 10 && r > 0);
        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }

        k = 0;
        j = 5;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = (k + 5)% r;
        } while (k < 5);

        printf("* file lseek  okay\n");
        printf("* closing file\n");
        close(fd);

        additional_asst2_tests();

        return 0;
}

void additional_asst2_tests(void) {

        printf("\n**********\n* Additional Assignment 2 Tests\n");

        int i, j, k, r, fd1, fd2;

        printf("**********\n* opening new file \"test1.file\"\n");
        fd1 = open("test1.file", O_RDWR | O_CREAT);
        printf("* open() got fd %d\n", fd1);
        if (fd1 < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("**********\n* opening file \"test1.file\" again\n");
        fd2 = open("test1.file", O_RDWR);
        printf("* open() got fd %d\n", fd2);
        if (fd2 < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* writing test string using file descriptor 1\n");
        r = write(fd1, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading the entire file using file descriptor 2\n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", MAX_BUF -i);
                r = read(fd2, &buf[i], MAX_BUF - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < MAX_BUF && r > 0);

        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }
        k = j = 0;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = k % r;
        } while (k < i);
        printf("* file content okay\n");

        printf("* closing file descriptor 2\n");
        close(fd2);

        printf("* testing lseek with SEEK_END\n");
        r = lseek(fd1, -4, SEEK_END);
        if (r < 0) {
                printf("ERROR lseek: %s\n", strerror(errno));
                exit(1);
        }

        printf("* attempting to read last 4 bytes of the file into buffer \n");
        r = read(fd1, &buf, 10);
        printf("* read %d bytes\n", r);
        printf("* content read was \"%s\"\n", &buf[0]);
        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* testing dup2 with newfd = oldfd\n");
        r = dup2(fd1, fd1);
        if (r != fd1) {
                printf("ERROR duplicating fd1 onto itself\n");
                exit(1);
        }

        printf("* opening new file \"test2.file\"\n");
        fd2 = open("test2.file", O_RDWR | O_CREAT);
        printf("* open() got fd %d\n", fd2);
        if (fd2 < 0) {
                printf("ERROR opening file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* duplicating fd2 onto fd1\n");
        r = dup2(fd2, fd1);
        if (r != fd1) {
                printf("ERROR duplicating fd2 to fd1\n");
                exit(1);
        }

        printf("* writing test string using file descriptor 1\n");
        r = write(fd1, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* writing test string using file descriptor 2\n");
        r = write(fd2, teststr, strlen(teststr));
        printf("* wrote %d bytes\n", r);
        if (r < 0) {
                printf("ERROR writing file: %s\n", strerror(errno));
                exit(1);
        }

        printf("* closing file descriptor 2\n");
        close(fd2);

        printf("seeking back to the start of the file with fd1\n");
        r = lseek(fd1, 0, SEEK_SET);
        if (r < 0) {
                printf("ERROR lseek: %s\n", strerror(errno));
                exit(1);
        }

        printf("* reading using file descriptor 1\n");
        i = 0;
        do  {
                printf("* attempting read of %d bytes\n", MAX_BUF -i);
                r = read(fd1, &buf[i], MAX_BUF - i);
                printf("* read %d bytes\n", r);
                i += r;
        } while (i < MAX_BUF && r > 0);

        printf("* reading complete\n");
        if (r < 0) {
                printf("ERROR reading file: %s\n", strerror(errno));
                exit(1);
        }
        k = j = 0;
        r = strlen(teststr);
        do {
                if (buf[k] != teststr[j]) {
                        printf("ERROR  file contents mismatch\n");
                        exit(1);
                }
                k++;
                j = k % r;
        } while (k < i);
        printf("* file content okay\n");

        printf("* closing fd1\n");
        close(fd1);

        printf("* opening more than OPEN_MAX files\n");
        for (i = 0; i < OPEN_MAX - 3; i++) {
                fd1 = open("test1.file", O_RDWR);
                printf("* open() got fd %d\n", fd1);
                if (fd1 < 0) {
                        printf("ERROR opening file: %s\n", strerror(errno));
                        exit(1);
                }
        }
        fd1 = open("test1.file", O_RDWR);
        if (fd1 != -1) {
                printf("ERROR - did not return error when opening more than OPEN_MAX files\n");
                exit(1);
        }

        for (i = 3; i < OPEN_MAX; i++) {
                close(i);
        }
        printf("Successful in opening more than OPEN_MAX files\n");


        printf("**********\n Testing complete\n**********\n");

        return;
}