#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

// Function to format permissions
void format_permissions(mode_t mode, char *str) {
    str[0] = (S_ISDIR(mode)) ? 'd' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

// Function for long listing format
void print_long_format(const char *filename) {
    struct stat file_stat;
    
    if (lstat(filename, &file_stat) == -1) {
        perror("lstat");
        return;
    }
    
    // Format permissions
    char permissions[11];
    format_permissions(file_stat.st_mode, permissions);
    
    // Get user and group names
    struct passwd *pwd = getpwuid(file_stat.st_uid);
    struct group *grp = getgrgid(file_stat.st_gid);
    
    // Format time
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat.st_mtime));
    
    // Print all information
    printf("%s %2ld %s %s %5ld %s %s\n", 
           permissions,
           file_stat.st_nlink,
           pwd ? pwd->pw_name : "unknown",
           grp ? grp->gr_name : "unknown",
           file_stat.st_size,
           time_str,
           filename);
}

// Function to list directory contents
void list_directory(const char *dirname, int long_format) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        if (long_format) {
            print_long_format(entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int long_format = 0;
    int opt;
    
    // Parse command line options
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // List current directory
    list_directory(".", long_format);
    
    return 0;
}
