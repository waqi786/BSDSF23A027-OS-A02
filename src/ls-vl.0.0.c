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
#include <sys/ioctl.h>

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

// Function to get terminal width
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return 80; // Default width if ioctl fails
    }
    return w.ws_col;
}

// Function for column display (down then across)
void print_column_format(char **filenames, int count, int max_len) {
    int term_width = get_terminal_width();
    int col_width = max_len + 2; // Add spacing between columns
    int num_cols = term_width / col_width;
    
    if (num_cols < 1) num_cols = 1;
    
    int num_rows = (count + num_cols - 1) / num_cols; // Ceiling division
    
    for (int row = 0; row < num_rows; row++) {
        for (int col = 0; col < num_cols; col++) {
            int index = row + col * num_rows;
            if (index < count) {
                printf("%-*s", col_width, filenames[index]);
            }
        }
        printf("\n");
    }
}

// Function to list directory contents
void list_directory(const char *dirname, int long_format) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        return;
    }
    
    // For column display: read all filenames first
    char **filenames = NULL;
    int count = 0;
    int max_len = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        if (long_format) {
            print_long_format(entry->d_name);
        } else {
            // Store filename for column display
            filenames = realloc(filenames, (count + 1) * sizeof(char *));
            filenames[count] = strdup(entry->d_name);
            
            // Update max filename length
            int len = strlen(entry->d_name);
            if (len > max_len) {
                max_len = len;
            }
            
            count++;
        }
    }
    
    // If not long format, print in columns
    if (!long_format && count > 0) {
        print_column_format(filenames, count, max_len);
        
        // Free allocated memory
        for (int i = 0; i < count; i++) {
            free(filenames[i]);
        }
        free(filenames);
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
