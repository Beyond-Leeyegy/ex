#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pwd.h> 
#include <grp.h> 
#include <libgen.h>


#define ERR_EXIT(m) \
    do\
    {\
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)\

void lsdir(char *dirname);
void lsfile(char *filename);
void lsfile(char *filename);
char getFileType(struct stat *fstat);
void getFilePerm(struct stat *st, char *perm);

int main(int argc,char **argv)
{

    if(argc != 2){
        fprintf(stderr,"usage:%s [filepath]\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    struct stat fstat;
    if(lstat(argv[1],&fstat) == -1)
        ERR_EXIT("STAT ERROR");

    if(S_ISDIR(fstat.st_mode))
    {
        lsdir(argv[1]);
    }
    else{
        lsfile(argv[1]);
    }
    return 0;
}

void lsdir(char *dirname)
{
    DIR *dir;
    char filename[100] = {0};
    dir =opendir(dirname); 
    if(dir == NULL)
        ERR_EXIT("opendir error");
    struct dirent *dentry;
    while((dentry = readdir(dir)) != NULL)
    {
        
        char *fname;
        fname = dentry->d_name;
        if(strncmp(fname,".",1) == 0)
            continue;
        sprintf(filename,"%s/%s",dirname,fname);
        lsfile(filename);
    }

    closedir(dir);

}

//-rw-r--r--.   1            zxy        zxy        2586        Jul 10 17:00    ls.c
//类型及权限  硬链接数        拥有者    所属组    文件大小    创建时间        文件名
void lsfile(char *filename)
{
    struct stat tmpstat;
    if(lstat(filename,&tmpstat) == -1)
        ERR_EXIT("STAT ERROR");
    char buf[11]= {0};
    strcpy(buf,"----------");
    char type;
    type = getFileType(&tmpstat);
    char *bname = basename(filename);
    buf[0] = type;
    if(type == 'l'){
        char content[1024];
        if(readlink(filename,content,1024) == -1)
            ERR_EXIT("readlink error");
        sprintf(bname,"%s -> %s",bname,content);

    }
    getFilePerm(&tmpstat,buf);
    struct tm *ftime;
    ftime = localtime(&tmpstat.st_mtime);
    
    printf("%s %d %s %s %10ld %02d %02d %02d:%02d %s\n",
        buf,tmpstat.st_nlink,
        getpwuid(tmpstat.st_uid)->pw_name,
        getgrgid(tmpstat.st_gid)->gr_name,
        tmpstat.st_size,
        ftime->tm_mon+1,
        ftime->tm_mday,
        ftime->tm_hour,
        ftime->tm_min,
        bname);

}

//获得文件类型
char getFileType(struct stat *st)
{
    char type = '-';
    switch (st->st_mode  & S_IFMT)
    {
        case S_IFSOCK:
        type = 's';
                break;
        case S_IFLNK:
        type = 'l';
                break;
        case S_IFREG:
        type = '-';
                break;
        case S_IFBLK:
        type = 'b';
                break;
        case S_IFDIR:
        type = 'd';
                break;
        case S_IFCHR:
        type = 'c';
                break;
        case S_IFIFO:
        type = 'p';
                break;
    }
    return type;
}

//获得文件访问权限
void getFilePerm(struct stat *st, char *perm)
{
    mode_t mode = st->st_mode;
    if (mode & S_IRUSR)
        perm[1] = 'r';
    if (mode & S_IWUSR)
        perm[2] = 'w';
    if (mode & S_IXUSR)
        perm[3] = 'x';
    if (mode & S_IRGRP)
        perm[4] = 'r';
    if (mode & S_IWGRP)
        perm[5] = 'w';
    if (mode & S_IXGRP)
        perm[6] = 'x';
    if (mode & S_IROTH)
        perm[7] = 'r';
    if (mode & S_IWOTH)
        perm[8] = 'w';
    if (mode & S_IXOTH)
        perm[9] = 'x';
}
