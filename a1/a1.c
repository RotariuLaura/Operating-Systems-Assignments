#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


void listDir(const char *path, long size, char *permissions)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[1024];
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL)
    {
        perror("ERROR\ninvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            if(path[strlen(path)-1]=='/')
            {
                snprintf(fullPath, 1024, "%s%s", path, entry->d_name);
            }
            else
            {
                snprintf(fullPath, 1024, "%s/%s", path, entry->d_name);
            }
            if(lstat(fullPath, &statbuf) == 0)
            {
                if(S_ISREG(statbuf.st_mode) || S_ISDIR(statbuf.st_mode))
                {
                    char perm[10];
                    snprintf(perm, 10, "%c%c%c%c%c%c%c%c%c",
                             (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                             (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                             (statbuf.st_mode & S_IXUSR) ? 'x' : '-',
                             (statbuf.st_mode & S_IRGRP) ? 'r' : '-',
                             (statbuf.st_mode & S_IWGRP) ? 'w' : '-',
                             (statbuf.st_mode & S_IXGRP) ? 'x' : '-',
                             (statbuf.st_mode & S_IROTH) ? 'r' : '-',
                             (statbuf.st_mode & S_IWOTH) ? 'w' : '-',
                             (statbuf.st_mode & S_IXOTH) ? 'x' : '-');
                    perm[9] = '\0';
                    if ((size == -1 || (S_ISREG(statbuf.st_mode) && statbuf.st_size >= size)) && (strcmp(permissions, "---------") == 0 || strcmp(permissions, perm) == 0))
                    {
                        printf("%s\n", fullPath);
                    }
                }
            }
        }
    }
    closedir(dir);
}


void listDirRec(const char *path, long size, char *permissions)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[1024];
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL)
    {
        perror("ERROR\ninvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            if(path[strlen(path)-1]=='/')
            {
                snprintf(fullPath, 1024, "%s%s", path, entry->d_name);
            }
            else
            {
                snprintf(fullPath, 1024, "%s/%s", path, entry->d_name);
            }
            if(lstat(fullPath, &statbuf) == 0)
            {
                char perm[10];
                snprintf(perm, 10, "%c%c%c%c%c%c%c%c%c",
                         (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                         (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                         (statbuf.st_mode & S_IXUSR) ? 'x' : '-',
                         (statbuf.st_mode & S_IRGRP) ? 'r' : '-',
                         (statbuf.st_mode & S_IWGRP) ? 'w' : '-',
                         (statbuf.st_mode & S_IXGRP) ? 'x' : '-',
                         (statbuf.st_mode & S_IROTH) ? 'r' : '-',
                         (statbuf.st_mode & S_IWOTH) ? 'w' : '-',
                         (statbuf.st_mode & S_IXOTH) ? 'x' : '-');
                perm[9] = '\0';
                if ((size == -1 || (S_ISREG(statbuf.st_mode) && statbuf.st_size >= size)) && (strcmp(permissions, "---------") == 0 || strcmp(permissions, perm) == 0))
                {
                    printf("%s\n", fullPath);
                }
                if(S_ISDIR(statbuf.st_mode))
                {
                    listDirRec(fullPath, size, permissions);
                }
            }
        }
    }
    closedir(dir);
}

int parse(const char *path)
{
    int fd;
    fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        perror("Could not open input file");
        close(fd);
        return -1;
    }
    lseek(fd, -1, SEEK_END);
    char c;
    read(fd, &c, 1);
    int magic = c - '0';
    if(magic != 8)
    {
        printf("ERROR\nwrong magic");
        close(fd);
        return -1;
    }
    lseek(fd, -3, SEEK_CUR);
    int header_size = 0;
    read(fd, &header_size, 2);
    lseek(fd, -header_size + 1, SEEK_CUR);
    int version = 0;
    read(fd, &version, 4);
    if(version < 106 || version > 193)
    {
        printf("ERROR\nwrong version");
        close(fd);
        return -1;
    }
    int no_of_sections = 0;
    read(fd, &no_of_sections, 1);
    if(no_of_sections < 5 || no_of_sections > 11)
    {
        printf("ERROR\nwrong sect_nr");
        close(fd);
        return -1;
    }
    char name[12];
    int type = 0, offset = 0, size = 0;
    char names[no_of_sections][12];
    int types[no_of_sections], sizes[no_of_sections];
    for(int i = 0; i < no_of_sections; i++)
    {
        int read_bytes = read(fd, &name, 11);
        name[read_bytes] = '\0';
        strcpy(names[i], name);
        read(fd, &type, 1);
        if(type != 59 && type != 52 && type != 32 &&
                type != 60 && type != 77 && type != 21 && type != 37)
        {
            printf("ERROR\nwrong sect_types");
            close(fd);
            return -1;
        }
        types[i] = type;
        read(fd, &offset, 4);
        read(fd, &size, 4);
        sizes[i] = size;
    }
   
    printf("SUCCESS\n");
    printf("version=%d", version);
    printf("\nnr_sections=%d", no_of_sections);
    for(int i = 0; i < no_of_sections; i++)
    {
        printf("\nsection%d: %s %d %d", i+1, names[i], types[i], sizes[i]);
    }
    close(fd);
    return 0;
}

void extract(const char * path, int section, int line)
{
    int fd;
    fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        perror("Could not open input file");
        return;
    }
    lseek(fd, -1, SEEK_END);
    char c;
    read(fd, &c, 1);
    lseek(fd, -3, SEEK_CUR);
    int header_size = 0;
    read(fd, &header_size, 2);
    lseek(fd, -header_size + 1, SEEK_CUR);
    int version = 0;
    read(fd, &version, 4);
    int no_of_sections = 0;
    read(fd, &no_of_sections, 1);
    if(section > no_of_sections)
    {
        printf("ERROR\nsection");
        close(fd);
        return;
    }
    char name[12];
    int type = 0, offset = 0, size = 0;
    for(int i = 0; i < no_of_sections; i++)
    {
        read(fd, &name, 11);
        read(fd, &type, 1);
        read(fd, &offset, 4);
        read(fd, &size, 4);
    }
    int file_size = lseek(fd, 0, SEEK_END);
    int body_size = file_size - header_size;
    lseek(fd, 0, SEEK_SET);
    lseek(fd, body_size + 5 + 20 * (section - 1) + 12, SEEK_CUR);
    int sect_offset, sect_size;
    read(fd, &sect_offset, 4);
    read(fd, &sect_size, 4);
    lseek(fd, sect_offset, SEEK_SET);
    char the_section[sect_size];
    read(fd, the_section, sect_size);
    lseek(fd, sect_offset, SEEK_CUR);
    printf("SUCCESS");
    int cnt = 1;
    int i = sect_size - 1;
    while(i >= 0)
    {
        if(the_section[i] == '\n')
        {
            cnt++;
        }
        if(cnt == line)
        {
            break;
        }
        i--;
    }
    if(line > cnt)
    {
        printf("ERROR\nline");
        close(fd);
        return;
    }
    int position = i + 1;
    if(the_section[i] != '\n')
    {
        printf("\n");
    }
    while(i >= 0)
    {
        printf("%c", the_section[i]);
        i--;
        if(the_section[i] == '\n')
        {
            break;
        }
    }
    lseek(fd, sect_offset + position, SEEK_SET);
    close(fd);
}

void findall(const char *path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[1024];
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL)
    {
        perror("ERROR\ninvalid directory path");
        return;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0)
        {
            int ok = 0, isSF = 1;
            if(path[strlen(path)-1]=='/')
            {
                snprintf(fullPath, 1024, "%s%s", path, entry->d_name);
            }
            else
            {
                snprintf(fullPath, 1024, "%s/%s", path, entry->d_name);
            }
            if(lstat(fullPath, &statbuf) == 0)
            {
                if(S_ISDIR(statbuf.st_mode))
                {
                    findall(fullPath);
                }
                if (S_ISREG(statbuf.st_mode))
                {
                    int fd = open(fullPath, O_RDONLY);
                    if(fd == -1)
                    {
                        perror("Could not open input file");
                        break;
                    }
                    lseek(fd, -1, SEEK_END);
                    char c;
                    read(fd, &c, 1);
                    int magic = c - '0';
                    if(magic != 8)
                    {
                        isSF = 0;
                        close(fd);
                        continue;
                    }
                    lseek(fd, -3, SEEK_CUR);
                    int header_size = 0;
                    read(fd, &header_size, 2);
                    lseek(fd, -header_size + 1, SEEK_CUR);
                    int version = 0;
                    read(fd, &version, 4);
                    if(version < 106 || version > 193)
                    {
                        isSF = 0;
                        close(fd);
                        continue;
                    }
                    int no_of_sections = 0;
                    read(fd, &no_of_sections, 1);
                    if(no_of_sections < 5 || no_of_sections > 11)
                    {
                        isSF = 0;
                        close(fd);
                        continue;
                    }
                    char name[12];
                    int type = 0, offset = 0, size = 0;
                    for(int i = 0; i < no_of_sections; i++)
                    {
                        int bytes = read(fd, &name, 11);
                        name[bytes] = '\0';
                        read(fd, &type, 1);
                        if(type != 59 && type != 52 && type != 32 &&
                                type != 60 && type != 77 && type != 21 && type != 37)
                        {
                            isSF = 0;
                            break;
                        }
                        if(type == 60)
                        {
                            ok = 1;
                        }
                        read(fd, &offset, 4);
                        read(fd, &size, 4);
                    }
                    if(ok == 1 && isSF == 1)
                    {
                        printf("%s\n", fullPath);
                    }
                    close(fd);                
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char ** argv)
{
    if(argc >= 2)
    {
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("47421\n");
        }
        else if(strcmp(argv[1], "list") == 0)
        {
            char * path = NULL;
            int recursive = 0;
            long size = -1;
            char permissions[10] = "---------";
            for (int i = 2; i < argc; i++)
            {
                if (strstr(argv[i], "path=") == argv[i])
                {
                    path = argv[i] + strlen("path=");
                }
                else if (strcmp(argv[i], "recursive") == 0)
                {
                    recursive = 1;
                }
                else if(strstr(argv[i], "size_greater=") == argv[i])
                {
                    size = strtol((argv[i] + strlen("size_greater=")), NULL, 10);
                }
                else if(strstr(argv[i], "permissions=") == argv[i])
                {
                    strncpy(permissions, argv[i] + strlen("permissions="), sizeof(permissions));
                }
            }
            if (path != NULL)
            {
                printf("SUCCESS\n");
                if (recursive == 1)
                {
                    listDirRec(path, size, permissions);
                }
                else
                {
                    listDir(path, size, permissions);
                }
            }
        }
        else if(strcmp(argv[1], "parse") == 0)
        {
            char * path = strtok(argv[2], "=");
            path = strtok(NULL, "");
            if(path != NULL)
            {
                parse(path);
            }
        }
        else if(strcmp(argv[1], "extract") == 0)
        {
            char *path;
            int section, line;
            path = argv[2] + strlen("path=");
            section = strtol((argv[3] + strlen("section=")), NULL, 10);
            line = strtol((argv[4] + strlen("line=")), NULL, 10);
            extract(path, section, line);
        }
        else if(strcmp(argv[1], "findall") == 0)
        {
            char * path = strtok(argv[2], "=");
            path = strtok(NULL, "");
            if(path != NULL)
            {
                printf("SUCCESS\n");
                findall(path);
            }
        }
    }
    return 0;
}
