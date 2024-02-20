#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define FIFO_NAME1 "RESP_PIPE_47421"
#define FIFO_NAME2 "REQ_PIPE_47421"
#define SHARED_MEMORY_NAME "/6DtRQIbU"
#define MEMORY_SIZE 3574945

int main(void)
{
    if(mkfifo(FIFO_NAME1, 0644) != 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }
    int fd1 = - 1;
    fd1 = open(FIFO_NAME2, O_RDONLY);
    if(fd1 == -1)
    {
        printf("ERROR\ncannot open the request pipe\n");
        return 1;
    }
    int fd2 = - 1;
    fd2 = open(FIFO_NAME1, O_WRONLY);
    if(fd2 == -1)
    {
        printf("ERROR\n");
        close(fd1);
        unlink(FIFO_NAME1);
        return 1;
    }
    if(write(fd2, "BEGIN#", 6) != -1)
    {
        printf("SUCCESS\n");
    }
    char request[512];
    int size = 0;
    char c;
    while(1)
    {
        read(fd1, &c, 1);
        if(c == '#')
        {
            request[size] = '\0';
            if(strcmp(request, "VARIANT") == 0)
            {
                write(fd2, "VARIANT#", 8);
                unsigned int x = 47421;
                write(fd2, &x, sizeof(x));
                write(fd2, "VALUE#", 6);
            }
            if(strcmp(request, "EXIT") == 0)
            {
                close(fd1);
                close(fd2);
                unlink(FIFO_NAME1);
                break;
            }
            if(strcmp(request, "CREATE_SHM") == 0)
            {
                int shmFd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0644);
                int ftr = ftruncate(shmFd, MEMORY_SIZE);
                volatile char *sharedChar = NULL;
                sharedChar = (volatile char*)mmap(0, MEMORY_SIZE,
                                                  PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
                if(shmFd == -1 || ftr == -1 || sharedChar == (void*)(-1))
                {
                    write(fd2, "CREATE_SHM#", 11);
                    write(fd2, "ERROR#", 6);
                }
                else
                {
                    write(fd2, "CREATE_SHM#", 11);
                    write(fd2, "SUCCESS#", 8);
                }
            }
            if(strcmp(request, "MAP_FILE") == 0)
            {
                char filename[512];
                int filesize = 0;
                char *data = NULL;
                while(1)
                {
                    read(fd1, &c, 1);
                    if(c == '#')
                    {
                        filename[filesize] = '\0';
                        int fd = open(filename, O_RDWR);
                        int theSize = lseek(fd, 0, SEEK_END);
                        data = (char*)mmap(NULL, theSize, PROT_READ, MAP_SHARED, fd, 0);
                        if(fd == -1 || data == (void*)-1)
                        {
                            write(fd2, "MAP_FILE#", 9);
                            write(fd2, "ERROR#", 6);
                        }
                        else
                        {
                            write(fd2, "MAP_FILE#", 9);
                            write(fd2, "SUCCESS#", 8);
                        }
                        break;
                    }
                    else
                    {
                        filename[filesize] = c;
                        filesize++;
                    }
                }
            }
            break;
        }
        else
        {
            request[size] = c;
            size++;
        }
    }
    return 0;
}