void checkDirectory(char * path, int currentDepth) {
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *newpath = malloc(MAX_STRING_SIZE);
    int folderSize = 0;
    pid_t pid;

    if ((dirp = opendir(path)) == NULL)
    {
        perror(path);
        exit(2);
    }

    /* struct dirent {
        ino_t          d_ino;        Inode number 
        off_t          d_off;        Not an offset; see below 
        unsigned short d_reclen;     Length of this record 
        unsigned char  d_type;       Type of file; not supported by all filesystem types 
        char           d_name[256];  Null-terminated filename 
    }; */
    while ((direntp = readdir( dirp)) != NULL)
    {
        sprintf(newpath, "%s/%s", path, direntp->d_name);

        /* struct stat {
            dev_t     st_dev;      ID of device containing file 
            ino_t     st_ino;      inode number 
            mode_t    st_mode;     protection 
            nlink_t   st_nlink;    number of hard links 
            uid_t     st_uid;      user ID of owner 
            gid_t     st_gid;      group ID of owner 
            dev_t     st_rdev;     device ID (if special file) 
            off_t     st_size;     total size, in bytes 
            blksize_t st_blksize;  blocksize for file system I/O 
            blkcnt_t  st_blocks;   number of 512B blocks allocated 
            time_t    st_atime;    time of last access 
            time_t    st_mtime;    time of last modification 
            time_t    st_ctime;    time of last status change 
        };
        returns information about the file in the struct stat format */
        if (lstat(newpath, &stat_buf) != 0) {
            perror(path);
            exit(3);
        }

        // In case it refers to parent directory
        if (strcmp(direntp->d_name, "..") == 0) continue; 

        // Maybe stat_buf.st_blocks/512?
        int file_space = stat_buf.st_blksize * stat_buf.st_blocks/8;
        
        /* S_ISREG(m) is it a regular file?
        S_ISDIR(m) directory?
        S_ISCHR(m) character device?
        S_ISBLK(m) block device?
        S_ISFIFO(m) FIFO (named pipe)?
        S_ISLNK(m) symbolic link? (Not in POSIX.1-1996.)
        S_ISSOCK(m) socket? (Not in POSIX.1-1996.) */

        
        
        if (S_ISREG(stat_buf.st_mode)){ // Regular file
            if (all && max_depth != 0){
                char temp[100];
                sprintf(temp, "%-7d %s\n", (int)ceil(file_space/block_size), newpath);
                write(STDOUT_FILENO, &temp, strlen(temp) + 1);
            } 
            if (separate_dirs){
                folderSize += file_space;
            }
        } else if (S_ISDIR(stat_buf.st_mode)){ // Directory
            if(strcmp(direntp->d_name, ".") != 0){
                if ((pid = fork()) > 0) {
                    wait(NULL);
                } else if (pid == 0) {
                    //Mantém sem o NULL porque senão dá erro na prepare_command e não faz em depth
                    /*char *command[100];
                    prepare_command(command, newpath);
                    closedir(dirp);
                    execv(command[0], command);*/
                    checkDirectory(newpath, currentDepth-1);
                } else {
                    printf("Error forking\n");
                    exit(5);
                }
            } else {
                folderSize += file_space;
            }
        } else if (S_ISLNK(stat_buf.st_mode)){ // Symbolic link
            if ((all && dereference) || dereference){
                if ((pid = fork()) > 0){
                    wait(NULL);
                } else if (pid == 0) {
                    //Mantém sem o NULL porque senão dá erro na prepare_command e não faz em depth
                    char *command[100];
                    prepare_command(command, newpath);
                    closedir(dirp);
                    execv(command[0], command);
                    checkDirectory(newpath, currentDepth-1);
                } else {
                    printf("Error forking\n");
                    exit(5);
                }
            } else if (all){
                char temp[100];
                sprintf(temp, "%-7d %s\n", (int)ceil(file_space/block_size), newpath);
                write(STDOUT_FILENO, &temp, strlen(temp) + 1);
                folderSize += file_space;
            }
        }
    }
    if (max_depth > 0) printf("%-7d %s\n", folderSize/block_size, path);

    closedir(dirp);
}