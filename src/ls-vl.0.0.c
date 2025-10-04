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

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_PINK    "\033[1;35m"
#define COLOR_REVERSE "\033[7m"

// Comparison function for qsort
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to get file color based on type and permissions
const char *get_file_color(const char *filename, mode_t mode) {
    // Check if it's a directory
    if (S_ISDIR(mode)) {
        return COLOR_BLUE;
    }
    
    // Check if it's executable (any of the execute bits set)
    if ((mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH)) {
        return COLOR_GREEN;
    }
    
    // Check if it's a symbolic link
    if (S_ISLNK(mode)) {
        return COLOR_PINK;
    }
    
    // Check for archive files by extension
    const char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0 || 
            strcmp(ext, ".zip") == 0 || strcmp(ext, ".tgz") == 0 ||
            strcmp(ext, ".deb") == 0 || strcmp(ext, ".rpm") == 0) {
            return COLOR_RED;
        }
    }
    
    // Default color (no color)
    return COLOR_RESET;
}

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
    printf("%s %2ld %s %s %5ld %s %s%s%s\n", 
           permissions,
           file_stat.st_nlink,
           pwd ? pwd->pw_name : "unknown",
           grp ? grp->gr_name : "unknown",
           file_stat.st_size,
           time_str,
           get_file_color(filename, file_stat.st_mode),
           filename,
           COLOR_RESET);
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
                struct stat file_stat;
                if (lstat(filenames[index], &file_stat) == 0) {
                    const char *color = get_file_color(filenames[index], file_stat.st_mode);
                    printf("%s%-*s%s", color, col_width, filenames[index], COLOR_RESET);
                } else {
                    printf("%-*s", col_width, filenames[index]);
                }
            }
        }
        printf("\n");
    }
}

// Function for horizontal display (across)
void print_horizontal_format(char **filenames, int count, int max_len) {
    int term_width = get_terminal_width();
    int col_width = max_len + 2;
    int current_width = 0;
    
    for (int i = 0; i < count; i++) {
        struct stat file_stat;
        const char *color = COLOR_RESET;
        
        if (lstat(filenames[i], &file_stat) == 0) {
            color = get_file_color(filenames[i], file_stat.st_mode);
        }
        
        int needed_width = strlen(filenames[i]) + 2;
        
        // Check if we need to go to next line
        if (current_width + needed_width > term_width && current_width > 0) {
            printf("\n");
            current_width = 0;
        }
        
        printf("%s%-*s%s", color, col_width, filenames[i], COLOR_RESET);
        current_width += col_width;
    }
    
    if (count > 0) {
        printf("\n");
    }
}

// Function to list directory contents
void list_directory(const char *dirname, int display_mode) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("opendir");
        return;
    }
    
    // Read all filenames first
    char **filenames = NULL;
    int count = 0;
    int max_len = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        // Store filename
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        
        // Update max filename length
        int len = strlen(entry->d_name);
        if (len > max_len) {
            max_len = len;
        }
        
        count++;
    }
    
    closedir(dir);
    
    // SORT THE FILENAMES ALPHABETICALLY
    if (count > 0) {
        qsort(filenames, count, sizeof(char *), compare_strings);
    }
    
    // Display based on mode
    if (display_mode == 1) { // Long format
        for (int i = 0; i < count; i++) {
            print_long_format(filenames[i]);
        }
    } else if (display_mode == 2) { // Horizontal format
        print_horizontal_format(filenames, count, max_len);
    } else { // Default column format
        print_column_format(filenames, count, max_len);
    }
    
    // Free allocated memory
    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

int main(int argc, char *argv[]) {
    int display_mode = 0; // 0=column, 1=long, 2=horizontal
    int opt;
    
    // Parse command line options
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                display_mode = 1;
                break;
            case 'x':
                display_mode = 2;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // List current directory
    list_directory(".", display_mode);
    
    return 0;
}
