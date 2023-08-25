#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/time.h>
#include<time.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h> 

#define MAX_PATH_LENGTH PATH_MAX
#define PROMPT_FORMAT "<%s@%s:%s> "
#define MAX_ARGS 64
#define TABLE_SIZE 100

int past_count=0;
char* arr[15];
char home_dir[MAX_PATH_LENGTH];

void remove_non_alphabet_prefix(char *str) {
    int first_alpha_pos = -1;
    
    for (int i = 0; str[i] != '\0'; i++) {
        if (isalpha((unsigned char)str[i])) {
            first_alpha_pos = i;
            break;
        }
    }
    
    if (first_alpha_pos != -1) {
        memmove(str, str + first_alpha_pos, strlen(str + first_alpha_pos) + 1);
    } else {
        str[0] = '\0'; // No alphabet found, resulting in an empty string
    }
}


void removeFirstCharacter(char *str) {
    if (str == NULL || *str == '\0') {
        // If the string is empty or NULL, there's nothing to remove.
        return;
    }
    
    // Shift all characters one position to the left to remove the first character.
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        str[i] = str[i + 1];
    }
}

void print_permissions(mode_t mode) {
    putchar((S_ISDIR(mode)) ? 'd' : '-');
    putchar((mode & S_IRUSR) ? 'r' : '-');
    putchar((mode & S_IWUSR) ? 'w' : '-');
    putchar((mode & S_IXUSR) ? 'x' : '-');
    putchar((mode & S_IRGRP) ? 'r' : '-');
    putchar((mode & S_IWGRP) ? 'w' : '-');
    putchar((mode & S_IXGRP) ? 'x' : '-');
    putchar((mode & S_IROTH) ? 'r' : '-');
    putchar((mode & S_IWOTH) ? 'w' : '-');
    putchar((mode & S_IXOTH) ? 'x' : '-');
}

void removeSubstring(char *str, const char *sub) {
    char *match;
    int len = strlen(sub);

    while ((match = strstr(str, sub)) != NULL) {
        memmove(match, match + len, strlen(match + len) + 1);
    }
}
void subtractFromRight(char *str, char character) {
    char *lastOccurrence = strrchr(str, character);
    if (lastOccurrence != NULL) {
        *lastOccurrence = '\0'; // Replace '/' with null terminator
    }
}

void print_prompt(const char *username, const char *systemname, const char *current_dir,char* home_dir) {
    char prompt[MAX_PATH_LENGTH + sizeof(PROMPT_FORMAT)];

    if (strcmp(current_dir, home_dir) == 0) {
        snprintf(prompt, sizeof(prompt), PROMPT_FORMAT, username, systemname, "~");
    } 
    else if(strlen(current_dir)<strlen(home_dir))
    {
        snprintf(prompt, sizeof(prompt), PROMPT_FORMAT, username, systemname, current_dir);
    }
    else {
        char new_dir[MAX_PATH_LENGTH];
        strcpy(new_dir, current_dir);

        removeSubstring(new_dir, home_dir);

        char yo[MAX_PATH_LENGTH + 1] = "~";

        if (strlen(new_dir) > 0) {
            strcat(yo, new_dir);
        }

        snprintf(prompt, sizeof(prompt), PROMPT_FORMAT, username, systemname, yo);
    }

    printf("%s", prompt);
    fflush(stdout);
}



void reduceContinuousSpaces(char *input) 
{
    char prevChar = '\0';  
    int inputIndex = 0;    
    int outputIndex = 0;   

    while (input[inputIndex] != '\0') 
    {
        if (!(prevChar == ' ' && input[inputIndex] == ' ')) 
        {
            input[outputIndex] = input[inputIndex];
            prevChar = input[inputIndex];
            outputIndex++;
        }
        inputIndex++;
    }
    input[outputIndex] = '\0';  
}

int isDirectory(const char *path) {
    struct stat statbuf;

    if (stat(path, &statbuf) != 0) 
    {
        return 0;
    }

    return S_ISDIR(statbuf.st_mode);  
}

void warp_command(char *path, char *current_dir, char *home_dir, char *prev_dir) 
{
    char new_path[MAX_PATH_LENGTH];

    if (strcmp(path, "~") == 0) 
    {
        snprintf(new_path, sizeof(new_path), "%s", home_dir);
    } 

    else if (strcmp(path, "-") == 0) 
    {
        snprintf(new_path, sizeof(new_path), "%s", prev_dir);
    }

    else if (strcmp(path, ".") == 0) 
    {
        snprintf(new_path, sizeof(new_path), "%s", current_dir);
    } 
    
    else if (strcmp(path, "..") == 0) 
    {
        snprintf(new_path, sizeof(new_path), "%s", current_dir);
        char *last_slash = strrchr(new_path, '/');
        if (last_slash != NULL) 
        {
            *last_slash = '\0';
        }

    }

    else if (path[0] == '~') 
    {
        char trest[MAX_PATH_LENGTH];
        char trest1[MAX_PATH_LENGTH];

        strcpy(trest,path);
        strcpy(trest1,home_dir);

        removeSubstring(trest,"~");

        strcat(trest1,trest);

        snprintf(new_path, sizeof(new_path), "%s",trest1);
    } 
    
    else if (path[0] == '/') 
    {
        snprintf(new_path, sizeof(new_path), "%s", path);
    }

    else 
    {
        char check_path[MAX_PATH_LENGTH];
        char yo[] = "/";

        strcpy(check_path, current_dir);
        strcat(yo, path);

        strcat(check_path, yo);
        if (isDirectory(check_path)) {
            snprintf(new_path, sizeof(new_path), "%s/%s", current_dir, path);
        } else {
            strcpy(new_path, current_dir);
            printf("No Such Directory\n");
        }
    }

    strcpy(prev_dir, current_dir);
    printf("%s\n", new_path);

    strcpy(current_dir,new_path);

    // chdir(new_path);
}

int compare_strings(const void *a, const void *b) {
    char *str_a = *(char **)a;
    char *str_b = *(char **)b;

    char copy_1[200];
    char copy_2[200];

    strcpy(copy_1,str_a);
    strcpy(copy_2,str_b);

    int hidden_a = (strcmp(".",str_a)==0 || strcmp("..",str_a)==0);
    int hidden_b = (strcmp(".",str_b)==0 || strcmp("..",str_b)==0);

    if (hidden_a && !hidden_b) {
        return -1; // Hidden file comes before non-hidden
    } else if (!hidden_a && hidden_b) {
        return 1; // Non-hidden file comes after hidden
    }

    else if(hidden_a && hidden_b)
    {
        if(strcmp(".",str_a)==0 && strcmp("..",str_b)==0)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

    else
    {
        remove_non_alphabet_prefix(copy_1);
        remove_non_alphabet_prefix(copy_2);

        // Compare lowercased strings
        char lower_a[MAX_PATH_LENGTH];
        char lower_b[MAX_PATH_LENGTH];
        for (int i = 0; copy_1[i]; i++) {
            lower_a[i] = tolower(copy_1[i]);
        }
        lower_a[strlen(copy_1)] = '\0';

        for (int i = 0; copy_2[i]; i++) {
            lower_b[i] = tolower(copy_2[i]);
        }
        lower_b[strlen(copy_2)] = '\0';

        return strcmp(lower_a, lower_b);
    }
}

void print_colored(const char *str, const char *color_code) {
    printf("%s%s%s\n", color_code, str, "\033[0m");
}

void peek_command(const char *path, int show_all, int show_details) {
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char **names = NULL;
    int num_entries = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue; // Skip hidden files if not showing all
        }

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (lstat(full_path, &entry_stat) == -1) {
            perror("lstat");
            closedir(dir);
            return;
        }

        names = (char **)realloc(names, (num_entries + 1) * sizeof(char *));
        names[num_entries] = strdup(entry->d_name);
        num_entries++;
    }

    closedir(dir);

    qsort(names, num_entries, sizeof(char *), compare_strings);

    for (int i = 0; i < num_entries; i++) {
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, names[i]);

        struct stat entry_stat;
        if (lstat(full_path, &entry_stat) == -1) {
            perror("lstat");
            continue;
        }

        if (show_details) {
            print_permissions(entry_stat.st_mode);
            
            struct passwd *user_info = getpwuid(entry_stat.st_uid);
            struct group *group_info = getgrgid(entry_stat.st_gid);
            printf(" %2ld %s %s %6ld ", entry_stat.st_nlink, user_info->pw_name, group_info->gr_name, entry_stat.st_size);

            char time_buffer[80];
            struct tm *timeinfo = localtime(&entry_stat.st_mtime);
            strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", timeinfo);
            printf("%s ", time_buffer);
            if (S_ISDIR(entry_stat.st_mode)) {
                print_colored(names[i], "\033[34m"); // Blue for directories
            } else if (entry_stat.st_mode & S_IXUSR) {
                print_colored(names[i], "\033[32m"); // Green for executables
            } else {
                print_colored(names[i], "\033[0m");  // White for regular files
            }
        } else {
            if (S_ISDIR(entry_stat.st_mode)) {
                print_colored(names[i], "\033[34m"); // Blue for directories
            } else if (entry_stat.st_mode & S_IXUSR) {
                print_colored(names[i], "\033[32m"); // Green for executables
            } else {
                print_colored(names[i], "\033[0m");  // White for regular files
            }
        }

        free(names[i]);
    }

    free(names);
}

void handle_peek_command(char *command, char *current_dir, char *home_dir, char *prev_dir) {
    char delim[] = " ";
    char *mid[4];
    char *yes;
    int show_all = 0;
    int show_details = 0;
    int count1 = 0;

    yes = strtok(command, delim);

    while (yes != NULL) {
        mid[count1] = yes;
        count1++;
        yes = strtok(NULL, delim);
    }

    char peek_path[MAX_PATH_LENGTH];

    if (count1 == 1) {
        snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
    } else if (count1 == 2) 
    {
        snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
                         if (strcmp(mid[1], "-a") == 0) {
                            show_all = 1;
                        } if (strcmp(mid[1], "-l") == 0) {
                            show_details = 1;
                        }
                        if(strcmp(mid[1], "-la") == 0)
                        {
                            show_all = 1;
                            show_details = 1;
                        }
                        if(strcmp(mid[1], "-al") == 0)
                        {
                            show_all = 1;
                            show_details = 1;
                        }

                        if(show_all==0 && show_details==0)
                        {
                            if(strcmp(mid[1], ".") == 0)
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
                            }
                            else if (strcmp(mid[1], "~") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", home_dir);
                            } 
                            else if (strcmp(mid[1], "-") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", prev_dir);
                            } 
                            else if (strcmp(mid[1], "..") == 0) 
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                subtractFromRight(test_path, '/');
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }   
                            else if(mid[1][0]=='/')
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", mid[1]);
                            }
                            else if(mid[1][0]=='~' && mid[1][1]=='/')
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path,home_dir);
                                removeSubstring(mid[1],"~");
                                strcat(test_path,mid[1]);
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                            else
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                strcat(test_path,"/");
                                strcat(test_path,mid[1]);

                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                        }
       
    } 
    else if(count1 == 3)
    {
         if (strcmp(mid[1], "-a") == 0) {
                            show_all = 1;
                        } if (strcmp(mid[1], "-l") == 0) {
                            show_details = 1;
                        }
                        if(strcmp(mid[1], "-la") == 0)
                        {
                            show_all = 1;
                            show_details = 1;
                        }
                        if(strcmp(mid[1], "-al") == 0)
                        {
                            show_all = 1;
                            show_details = 1;
                        }

                        if(strcmp(mid[2], "-a")==0 || strcmp(mid[2], "-l")==0)
                        {
                            show_all = 1;
                            show_details = 1;
                            snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
                        }
                        else
                        {
                            if(strcmp(mid[2], ".") == 0)
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
                            }
                            else if (strcmp(mid[2], "~") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", home_dir);
                            } 
                            else if (strcmp(mid[2], "-") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", prev_dir);
                            } 
                            else if (strcmp(mid[2], "..") == 0) 
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                subtractFromRight(test_path, '/');
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                            else if(mid[2][0]=='/')
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", mid[2]);
                            }
                            else if(mid[2][0]=='~' && mid[2][1]=='/')
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path,home_dir);
                                removeSubstring(mid[2],"~");
                                strcat(test_path,mid[2]);
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                            else
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                strcat(test_path,"/");
                                strcat(test_path,mid[2]);

                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                        }
    }

    else if(count1 == 4)
    {

                            show_all = 1;
                            show_details = 1;
                            if(strcmp(mid[3], ".") == 0)
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", current_dir);
                            }
                            else if (strcmp(mid[3], "~") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", home_dir);
                            } 
                            else if (strcmp(mid[3], "-") == 0) 
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", prev_dir);
                            } 
                            else if (strcmp(mid[3], "..") == 0) 
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                subtractFromRight(test_path, '/');
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                            else if(mid[3][0]=='/')
                            {
                                snprintf(peek_path, sizeof(peek_path), "%s", mid[3]);
                            }
                            else if(mid[3][0]=='~' && mid[3][1]=='/')
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path,home_dir);
                                removeSubstring(mid[3],"~");
                                strcat(test_path,mid[3]);
                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
                            else
                            {
                                char test_path[MAX_PATH_LENGTH];
                                strcpy(test_path, current_dir);
                                strcat(test_path,"/");
                                strcat(test_path,mid[3]);

                                snprintf(peek_path, sizeof(peek_path), "%s", test_path);
                            }
    }
    peek_command(peek_path, show_all, show_details);
}

void handle_warp_command(char* command,char* current_dir,char* home_dir,char* prev_dir)
{
                    if(strcmp(command, "warp")==0)
                    {

                        // chdir(home_dir);
                        // strcpy(current_dir,home_dir);
                    }
                    else
                    {
                        char warp_commands[256];
                        strncpy(warp_commands, command + 5, sizeof(warp_commands));
                        char *warp_token;
                        char *warp_saveptr = warp_commands;

                        while ((warp_token = strtok_r(warp_saveptr, " ", &warp_saveptr))) 
                        {
                            warp_command(warp_token, current_dir, home_dir, prev_dir);
                        }
                    }

}
void executeSystemCommand(const char *command) {
    char system_command[256];
    snprintf(system_command, sizeof(system_command), "%s 2> /dev/null", command);
    int return_code = system(system_command);
    if (return_code != 0) {
        char command_only[256];
        sscanf(command, "%s", command_only);
        printf("ERROR : '%s' is not a valid command\n", command_only);
    }
}
void print_process_info(pid_t pid,char* home_dir) 
{
    char status;
    char process_status[10];
    pid_t process_group;
    unsigned int virtual_memory;
    char executable_path[1024];
    
    // Read process status from /proc/<pid>/stat
    FILE *status_file = fopen("/proc/self/status", "r");
    if (status_file == NULL) {
        perror("Error opening status file");
        return;
    }
    fscanf(status_file, "State: %c", &status);
    char line1[MAX_PATH_LENGTH];
    char final_line[MAX_PATH_LENGTH];

    while(fgets(line1, sizeof(line1), status_file))
    {
                if (strncmp(line1, "State:", 6) == 0) 
                {
                    strcpy(final_line,line1+7);
                }
    }
    char *token = strtok(final_line, " ");

    fclose(status_file);
    
    // Read process group from /proc/<pid>/stat
    FILE *stat_file = fopen("/proc/self/stat", "r");
    if (stat_file == NULL) {
        perror("Error opening stat file");
        return;
    }
    fscanf(stat_file, "%*d %*s %*c %d", &process_group);
    fclose(stat_file);
    
    // Read virtual memory size from /proc/<pid>/status
    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
    FILE *vm_file = fopen(status_path, "r");
    if (vm_file == NULL) {
        perror("Error opening status file");
        return;
    }
    while (fgets(executable_path, sizeof(executable_path), vm_file)) {
        if (sscanf(executable_path, "VmSize: %u", &virtual_memory) == 1) {
            break;
        }
    }
    fclose(vm_file);
    
    // Read executable path from /proc/<pid>/exe
    char exe_path[1024];
    snprintf(exe_path, sizeof(exe_path), "/proc/%d/exe", pid);
    ssize_t path_size = readlink(exe_path, executable_path, sizeof(executable_path) - 1);
    if (path_size != -1) {
        executable_path[path_size] = '\0';
    } else {
        strcpy(executable_path, "Unknown");
    }
    char line[MAX_PATH_LENGTH];

    if(strlen(executable_path)<strlen(home_dir))
    {
        snprintf(line, sizeof(line), "%s", executable_path);
    }
    else 
    {
        char new_dir[MAX_PATH_LENGTH];
        strcpy(new_dir, executable_path);

        removeSubstring(new_dir, home_dir);

        char yo[MAX_PATH_LENGTH + 1] = "~";

        if (strlen(new_dir) > 0) {
            strcat(yo, new_dir);
        }

        strcpy(line,yo);
    }
    
    printf("pid : %d\n", pid);
    printf("process status : %s\n", token);
    printf("Process Group : %d\n", process_group);
    printf("Virtual memory : %u\n", virtual_memory);
    printf("executable path : %s\n", line);
}


void search_directory_tree(char *dir_path, char *target,char* current_dir,char* home_dir,int dir_count,int file_count,int e_count,int* arr,char* rel_path,char** store) 
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) 
    {
        return;
    } 
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024]; // Adjust the size as needed
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        char copy[1024];
        strcpy(copy,entry->d_name);
        char* token=strtok(copy,".");
        struct stat path_stat;

        if (strcmp(token, target) == 0) 
        {
            char copy_mid[MAX_PATH_LENGTH];
            strcpy(copy_mid,full_path);

                if(lstat(copy_mid, &path_stat)==0)
                {
                    if(dir_count && S_ISDIR(path_stat.st_mode))
                    {
                        if(strlen(copy_mid)>=strlen(rel_path))
                        {
                            removeSubstring(copy_mid,rel_path);
                            char new_string[MAX_PATH_LENGTH]=".";
                            strcat(new_string,copy_mid);
                            strcpy(store[arr[0]],new_string);
                        }
                        else
                        {
                            strcpy(store[arr[0]],copy_mid);
                        }
                        arr[0]++;         
                    }

                    else if(file_count && !(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                    {
                        if(strlen(copy_mid)>strlen(rel_path))
                        {
                            removeSubstring(copy_mid,rel_path);
                            char new_string[MAX_PATH_LENGTH]=".";
                            strcat(new_string,copy_mid);
                            strcpy(store[arr[0]],new_string);
                        }
                        else
                        {
                            strcpy(store[arr[0]],copy_mid);
                        }
                        arr[0]++; 
                    }
                }
        }

        if (entry->d_type == DT_DIR) 
        {
            search_directory_tree(full_path, target,current_dir,home_dir,dir_count,file_count,e_count,arr,rel_path,store); // Recursive call for subdirectory
        }
    }

    closedir(dir);
}

void handle_seek(char* command,char* current_dir,char* home_dir)
{
    char* token=strtok(command, " ");
    int count =0;

    char *arguments[5];

    for(int i=0;i<5;i++)
    {
        arguments[i]=(char*)malloc(1024*sizeof(char));
    }

    char * store[MAX_PATH_LENGTH];

    for(int i=0;i<MAX_PATH_LENGTH;i++)
    {
        arguments[i]=(char*)malloc(1024*sizeof(char));
    }

    int arr[1];
    arr[0]=0;
    int dir_count=0;
    int file_count=0;
    int e_count=0;

    while(token!=NULL)
    {
        strcpy(arguments[count],token);
        count++;

        if(strcmp(token,"-d")==0)
        {
            dir_count=1;
        }

        if(strcmp(token,"-f")==0)
        {
            file_count=1;
        }

        if(strcmp(token,"-e")==0)
        {
            e_count=1;
        }

        token = strtok(NULL, " ");
    }

    if(dir_count==file_count && dir_count==1)
    {
        printf("Invalid flags!\n");
        return;
    }

    if(dir_count==0 && file_count==0)
    {
        dir_count=1;
        file_count=1;
    }

    if(count==2)
    {
        search_directory_tree(current_dir,arguments[1],current_dir,home_dir,dir_count,file_count,e_count,arr,current_dir,store);
        if(arr[0]==0)
        {
            printf("No match found!\n");
        }
        else if(arr[0]==1)
        {
            if(e_count==1)
            {
                struct stat path_stat;
                char exec[1024];
                strcpy(exec,current_dir);
                char ok[1024];
                strcpy(ok,store[0]);
                removeFirstCharacter(ok);
                strcat(exec,ok);

                if(lstat(exec, &path_stat)==0)
                {
                    if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                    {
                        const char *greenCode = "\x1B[32m";
                        const char *resetCode = "\x1B[0m";
                        printf("%s%s%s\n", greenCode, store[0], resetCode);
                    }
                    else if((S_ISDIR(path_stat.st_mode)))
                    {
                            // if(store[0][0]=='.')
                            // {
                            //     // char need[MAX_PATH_LENGTH];
                            //     // strcpy(need,current_dir);
                            //     // removeSubstring(store[0],".");
                            //     // strcat(need,store[0]);
                            //     chdir(exec);
                            //     strcpy(current_dir,exec);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                    }
                }

            }
            else
            {
                struct stat path_stat;
                char exec[1024];
                strcpy(exec,current_dir);
                char ok[1024];
                strcpy(ok,store[0]);
                removeFirstCharacter(ok);
                strcat(exec,ok);

                if(lstat(exec, &path_stat)==0)
                {
                    if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                    {
                        const char *greenCode = "\x1B[32m";
                        const char *resetCode = "\x1B[0m";
                        printf("%s%s%s\n", greenCode, store[0], resetCode);
                    }
                    else if((S_ISDIR(path_stat.st_mode)))
                    {
                        const char *blueCode = "\x1B[34m";
                        const char *resetCode = "\x1B[0m";

                        printf("%s%s%s\n", blueCode, store[0], resetCode);
                    }
                }
                
            }
        }   

        else if(arr[0]>1)
        {
            for(int i=0;i<arr[0];i++)
            {
                struct stat path_stat;
                char exec[1024];
                strcpy(exec,current_dir);
                char ok[1024];
                strcpy(ok,store[i]);
                removeFirstCharacter(ok);
                strcat(exec,ok);

                if(lstat(exec, &path_stat)==0)
                {
                    if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                    {
                        const char *greenCode = "\x1B[32m";
                        const char *resetCode = "\x1B[0m";
                        printf("%s%s%s\n", greenCode, store[i], resetCode);
                    }
                    else if((S_ISDIR(path_stat.st_mode)))
                    {
                        const char *blueCode = "\x1B[34m";
                        const char *resetCode = "\x1B[0m";

                        printf("%s%s%s\n", blueCode, store[i], resetCode);
                    }
                }
            }
        }
    }
    else if(count==3)
    {
        if(strcmp(arguments[1],"-f")!=0 && strcmp(arguments[1],"-d")!=0 && strcmp(arguments[1],"-e")!=0)
        {
            char path[MAX_PATH_LENGTH];

            if(arguments[2][0]=='.')
            {
                removeSubstring(arguments[2],".");
                strcpy(path,current_dir);
                strcat(path,arguments[2]);
            }

            else if(arguments[2][0]=='~')
            {
               removeSubstring(arguments[2],"~");
                strcpy(path,home_dir);
                strcat(path,arguments[2]); 
            }
            else if(arguments[2][0]=='/')
            {
                strcpy(path,arguments[2]);
            }

            search_directory_tree(path,arguments[count-2],current_dir,home_dir,dir_count,file_count,e_count,arr,path,store);

            if(arr[0]==0)
            {
                printf("No match found!\n");
            }
            else if(arr[0]==1)
            {
                if(e_count==1)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            // if(store[0][0]=='.')
                            // {
                            //     char need[MAX_PATH_LENGTH];
                            //     strcpy(need,current_dir);
                            //     removeSubstring(store[0],".");
                            //     strcat(need,store[0]);
                            //     chdir(need);
                            //     strcpy(current_dir,need);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                        }
                    }

                }

                else
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[0], resetCode);
                        }
                    }
                    
                }
            }   

            else if(arr[0]>1)
            {
                for(int i=0;i<arr[0];i++)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[i]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[i], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[i], resetCode);
                        }
                    }
                }
            }
        }
        else
        {
            search_directory_tree(current_dir,arguments[count-1],current_dir,home_dir,dir_count,file_count,e_count,arr,current_dir,store);

            if(arr[0]==0)
            {
                printf("No match found!\n");
            }
            else if(arr[0]==1)
            {
                if(e_count==1)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            // if(store[0][0]=='.')
                            // {
                            //     char need[MAX_PATH_LENGTH];
                            //     strcpy(need,current_dir);
                            //     removeSubstring(store[0],".");
                            //     strcat(need,store[0]);
                            //     chdir(need);
                            //     strcpy(current_dir,need);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                        }
                    }

                }
                else
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[0], resetCode);
                        }
                    }
                    
                }
            }   

            else if(arr[0]>1)
            {
                for(int i=0;i<arr[0];i++)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[i]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[i], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[i], resetCode);
                        }
                    }
                }
            }
        } 
    }
    else if(count==4)
    {
        if(strcmp(arguments[2],"-f")!=0 && strcmp(arguments[2],"-d")!=0 && strcmp(arguments[2],"-e")!=0)
        {
            char path[MAX_PATH_LENGTH];
            if(arguments[3][0]=='.')
            {
                removeSubstring(arguments[3],".");
                strcpy(path,current_dir);
                strcat(path,arguments[3]);
            }

            else if(arguments[3][0]=='~')
            {
               removeSubstring(arguments[3],"~");
                strcpy(path,home_dir);
                strcat(path,arguments[3]); 
            }

            else if(arguments[3][0]=='/')
            {
                strcpy(path,arguments[3]);
            }

            search_directory_tree(path,arguments[count-2],current_dir,home_dir,dir_count,file_count,e_count,arr,path,store);

            if(arr[0]==0)
            {
                printf("No match found!\n");
            }
            else if(arr[0]==1)
            {
                if(e_count==1)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            // if(store[0][0]=='.')
                            // {
                            //     char need[MAX_PATH_LENGTH];
                            //     strcpy(need,current_dir);
                            //     removeSubstring(store[0],".");
                            //     strcat(need,store[0]);
                            //     chdir(need);
                            //     strcpy(current_dir,need);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                        }
                    }

                }
                else
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[0], resetCode);
                        }
                    }
                    
                }
            }   

            else if(arr[0]>1)
            {
                for(int i=0;i<arr[0];i++)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[i]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[i], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[i], resetCode);
                        }
                    }
                }
            }
        }
        else
        {
            search_directory_tree(current_dir,arguments[count-1],current_dir,home_dir,dir_count,file_count,e_count,arr,current_dir,store);

            if(arr[0]==0)
            {
                printf("No match found!\n");
            }
            else if(arr[0]==1)
            {
                if(e_count==1)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            // if(store[0][0]=='.')
                            // {
                            //     char need[MAX_PATH_LENGTH];
                            //     strcpy(need,current_dir);
                            //     removeSubstring(store[0],".");
                            //     strcat(need,store[0]);
                            //     chdir(need);
                            //     strcpy(current_dir,need);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                        }
                    }

                }
                else
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[0], resetCode);
                        }
                    }
                    
                }
            }   

            else if(arr[0]>1)
            {
                for(int i=0;i<arr[0];i++)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,current_dir);
                    char ok[1024];
                    strcpy(ok,store[i]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[i], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[i], resetCode);
                        }
                    }
                }
            }
        }
    }

    else if(count==5)
    {
            char path[MAX_PATH_LENGTH];
            if(arguments[4][0]=='.')
            {
                removeSubstring(arguments[4],".");
                strcpy(path,current_dir);
                strcat(path,arguments[4]);
            }

            else if(arguments[4][0]=='~')
            {
               removeSubstring(arguments[4],"~");
                strcpy(path,home_dir);
                strcat(path,arguments[4]); 
            }

            else if(arguments[4][0]=='/')
            {
                strcpy(path,arguments[4]);
            }

            search_directory_tree(path,arguments[count-2],current_dir,home_dir,dir_count,file_count,e_count,arr,path,store);

            if(arr[0]==0)
            {
                printf("No match found!\n");
            }
            else if(arr[0]==1)
            {
                if(e_count==1)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            // if(store[0][0]=='.')
                            // {
                            //     char need[MAX_PATH_LENGTH];
                            //     strcpy(need,current_dir);
                            //     removeSubstring(store[0],".");
                            //     strcat(need,store[0]);
                            //     chdir(need);
                            //     strcpy(current_dir,need);
                            // }
                            // else if(store[0][0]=='/')
                            // {
                            //     chdir(store[0]);
                            //     strcpy(current_dir,store[0]);
                            // }
                            // chdir(exec);
                            strcpy(current_dir,exec);
                        }
                    }

                }
                else
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[0]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[0], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[0], resetCode);
                        }
                    }
                    
                }
            }   

            else if(arr[0]>1)
            {
                for(int i=0;i<arr[0];i++)
                {
                    struct stat path_stat;
                    char exec[1024];
                    strcpy(exec,path);
                    char ok[1024];
                    strcpy(ok,store[i]);
                    removeFirstCharacter(ok);
                    strcat(exec,ok);

                    if(lstat(exec, &path_stat)==0)
                    {
                        if(!(path_stat.st_mode & S_IXUSR) && !(S_ISDIR(path_stat.st_mode)))
                        {
                            const char *greenCode = "\x1B[32m";
                            const char *resetCode = "\x1B[0m";
                            printf("%s%s%s\n", greenCode, store[i], resetCode);
                        }
                        else if((S_ISDIR(path_stat.st_mode)))
                        {
                            const char *blueCode = "\x1B[34m";
                            const char *resetCode = "\x1B[0m";

                            printf("%s%s%s\n", blueCode, store[i], resetCode);
                        }
                    }
                }
            }
    }

    
}
void handle_past_events(char * command,char* current_dir,char* home_dir,char* prev_dir,char*arr[15])
{
    if(strcmp(command,"pastevents")==0)
                    {
                        for(int i=0;i<past_count;i++)
                        {
                            printf("%s\n",arr[i]);
                        }
                    }

    else if(strcmp(command,"pastevents purge")==0)
                    {
                        for(int i=0;i<past_count;i++)
                        {
                            free(arr[i]);  
                            arr[i] = NULL; 
                        }
                        for (int i = 0; i < 15; i++) {
                            arr[i] = (char*)malloc(256);
                        }

                        past_count=0;
                    }

                    else
                    {
                        char limit[]=" ";

                        int need=0;
                        char* yes;
                        char *mid[3];

                        yes=strtok(command,limit);

                        while(yes!=NULL)
                        {
                            mid[need]=yes;
                            need++;
                            yes=strtok(NULL,limit);
                        }

                        if(need==3)
                        {
                            int x=atoi(mid[2]);

                            char pleb[MAX_PATH_LENGTH];

                            strcpy(pleb,arr[past_count-x]);

                            if(past_count==15)
                            {
                                if(strcmp(pleb,arr[14])!=0)
                                {
                                    for(int i=1;i<15;i++)
                                    {
                                        strcpy(arr[i-1],arr[i]);
                                    }
                                    strcpy(arr[14],pleb);
                                }
                            }
                            else
                            {
                                if(past_count>0)
                                {
                                    if(strcmp(pleb,arr[past_count-1])!=0)
                                    {
                                        strcpy(arr[past_count],pleb);
                                        past_count++;
                                    }
                                }
                                else
                                {
                                        strcpy(arr[past_count],pleb);
                                        past_count++;
                                }
                            }


                            char *delimiter1 = " ; ";
                            char *ptr = pleb;

                            char *token;

                            int max_tokens = 100;  
                            char *tokens[max_tokens];
                            int token_count = 0;

                            while ((token = strstr(ptr, delimiter1)) != NULL && token_count < max_tokens) 
                            {
                                *token = '\0';
                                tokens[token_count] = ptr; 
                                token_count++;
                                ptr = token + strlen(delimiter1);
                            }
                            tokens[token_count] = ptr;  
                            token_count++;

                            for(int i=0;i<token_count;i++)
                            {
                                double time_t;
                                struct timeval start, end;
                                if (strncmp(tokens[i], "peek", 4) == 0) 
                                {
                                    gettimeofday(&start, NULL);
                                    handle_peek_command(tokens[i], current_dir, home_dir, prev_dir);
                                    gettimeofday(&end, NULL);
                                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                                }
                                else if(strncmp(tokens[i], "warp", 4) == 0)
                                {
                                    gettimeofday(&start, NULL);
                                    handle_warp_command(tokens[i], current_dir, home_dir, prev_dir);
                                    gettimeofday(&end, NULL);
                                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                                }

                                else if(strncmp(tokens[i],"proclore",8)==0)
                                {
                                    gettimeofday(&start, NULL);
                                    int check_count=0;

                                char *token_space = strtok(tokens[i], " ");

                                char* new[2];
                                    new[0] = (char *)malloc(MAX_PATH_LENGTH);  
                                    new[1] = (char *)malloc(MAX_PATH_LENGTH); 

                                    while(token_space!=NULL)
                                    {
                                        strcpy(new[check_count],token_space);
                                        check_count++;
                                        token_space = strtok(NULL, " ");
                                    }

                                    if(check_count==1)
                                    {
                                        pid_t pid;
                                        pid=getpid();
                                        print_process_info(pid,home_dir);
                                    }

                                    else if(check_count==2)
                                    {
                                        int b=atoi(new[1]);

                                        print_process_info(b,home_dir);
                                    }

                                    else
                                    {
                                        printf("Invalid Command\n");
                                    }

                                    gettimeofday(&end, NULL);
                                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                                    free(new[0]);
                                    free(new[1]);
                                }
                                else if(strncmp(tokens[i], "seek", 4) == 0)
                                {
                                    handle_seek(tokens[i],current_dir,home_dir);
                                }

                                else
                                {
                                    gettimeofday(&start, NULL);
                                    executeSystemCommand(tokens[i]);
                                    gettimeofday(&end, NULL);
                                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                                }

                                if(time_t>2)
                                {
                                    char* check1=strtok(tokens[i]," ");
                                    printf("%s for %0.0f seconds\n",check1,time_t);
                                }
                            }
                        }
                    }
}

void childHandler(int signal) 
{
    int status;
    int pid;
    chdir(home_dir);
    FILE *output_file = fopen("output.txt", "a"); 
    FILE* file=fopen("output1.txt","r");
    FILE* file1=fopen("output2.txt","w");
    fseek(file,0,SEEK_SET);
    fseek(file1,0,SEEK_SET);

    char buffer[1024]; 
    char copy[1024];
    char temp[1024];
    char del[]=" ";
    char* token;
    int count=0;
    int flag=0;

    if (output_file == NULL) {
        perror("Failed to open output file");
        return;
    }
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) 
    {
        if (WIFEXITED(status)) 
        {
            while(fgets(buffer, sizeof(buffer), file) != NULL)
            {
                flag=0;
                count=0;
                strcpy(copy,buffer);
                char* token=strtok(buffer,del);
                 while (token != NULL) {
                    if(count==0)
                    {
                        if(atoi(token)==pid)
                        {
                            flag=1;
                        }
                    }
                    if(count==1)
                    {
                        if(flag==1)
                        {
                            strcpy(temp,token);
                        }
                        break;
                    }
                    token = strtok(NULL, del);
                    count++;
                }

                if(flag==0)
                {
                    fprintf(file1,"%s\n",copy);
                }
                
            }
            fclose(file1);
            fclose(file);
            file=fopen("output1.txt","w");
            file1=fopen("output2.txt","r");
            fseek(file,0,SEEK_SET);
            fseek(file1,0,SEEK_SET);

            while(fgets(buffer,sizeof(buffer),file1)!=NULL)
            {
                fprintf(file,"%s\n",buffer);
            }

            fclose(file);
            fclose(file1);

            file1=fopen("output2.txt","w");

            fclose(file1);
            
            fprintf(output_file, "%s with pid %d exited normally\n",temp,pid);
        } 
        else 
        {
            fprintf(output_file, "Background process with PID %d did not exit normally.\n", pid);
        }
    }

    fclose(output_file);
}

void exitHandler() 
{
    chdir(home_dir);
    FILE *f=fopen("mid.txt","w");

    for (int i = 0; i < past_count; i++) 
    {
        if(arr[i]!=NULL)
        {
            fprintf(f, "%s\n", arr[i]);
            free(arr[i]);
        }
    }
    if(past_count!=0 && strcmp(arr[past_count-1],"exit")!=0)
    {
        fprintf(f,"%s\n","exit");
    }
    past_count++;
    FILE *file=fopen("bas.txt","w");
    fprintf(file,"%d\n",past_count);
    fclose(file);
    fclose(f);
    exit(0);
}

void sigintHandler(int sig) 
{
    exitHandler();
}
void removeLastNewline(char *str) {
    int length = strlen(str);

    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0'; // Replace the newline with a null terminator
    }
}

int main() 
{
    signal(SIGINT, sigintHandler);
    char *username = getenv("USER");
    char systemname[256];

    for (int i = 0; i < 15; i++)
    {
        arr[i] = (char*)malloc(256); 
    }

    FILE* file=fopen("mid.txt","r");
    FILE* f=fopen("bas.txt","r");
    char *buffer_mid=NULL;
    size_t len = 0;     
    ssize_t read;

    int count_yes=0;

    while((read = getline(&buffer_mid, &len, f)) != -1)
    {
        removeLastNewline(buffer_mid);
        past_count=atoi(buffer_mid);   
    }
    fclose(f);
    f=fopen("bas.txt","w");
    fclose(f);

    if(past_count<=15)
    {
        while((read = getline(&buffer_mid, &len, file)) != -1)
        {
            removeLastNewline(buffer_mid);
            strcpy(arr[count_yes],buffer_mid);
            count_yes++;
        }
    }
    else if(past_count > 15)
    {
        while((read = getline(&buffer_mid, &len, file)) != -1)
        {
            if(count_yes>=1)
            {
                removeLastNewline(buffer_mid);
                strcpy(arr[count_yes],buffer_mid);
                past_count++;
            }
            count_yes++;
        }
        past_count=15;
    }

    fclose(file);
    file=fopen("mid.txt","w");
    fclose(file);

    gethostname(systemname, sizeof(systemname));

    char current_dir[MAX_PATH_LENGTH];
    char prev_dir[MAX_PATH_LENGTH];

    if (getcwd(home_dir, sizeof(home_dir)) == NULL) 
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    strcpy(prev_dir,home_dir);
    strcpy(current_dir,home_dir);

    while (1) 
    {
        // if (getcwd(current_dir, sizeof(current_dir)) == NULL) 
        // {
        //     perror("getcwd");
        //     exit(EXIT_FAILURE);
        // }

        print_prompt(username, systemname, current_dir,home_dir);

        char input[256];

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0'; 

        reduceContinuousSpaces(input);

        char *delimiter1 = " ; ";
        char *saveptr1 = input;
           char pseudo[MAX_PATH_LENGTH];

        strcpy(pseudo,input);
        
        int flag=0;
        char *token;

        int max_tokens = 100;  
        char *tokens[max_tokens];
        int token_count = 0;

        if ((token = strstr(saveptr1, delimiter1)) != saveptr1) 
        {
            while ((token = strstr(saveptr1, delimiter1)) != NULL && token_count < max_tokens) 
            {
                *token = '\0';
                tokens[token_count] = saveptr1; 
                if(strncmp(tokens[token_count],"pastevents",10)==0)
                {
                    flag=1;
                }
                token_count++;
                saveptr1 = token + strlen(delimiter1);
            }
            tokens[token_count] = saveptr1;  
            if(strncmp(tokens[token_count],"pastevents",10)==0)
                {
                    flag=1;
                }
            token_count++;
        }

    
        // printf("%d\n",flag);
        if(past_count==15)
            {
                if(flag==0 && strcmp(pseudo,arr[14])!=0)
                {
                    for(int i=1;i<15;i++)
                    {
                        strcpy(arr[i-1],arr[i]);
                    }
                    strcpy(arr[14],pseudo);
                }
            }
            else
            {
                if(past_count>0)
                {
                    if(strcmp(pseudo,arr[past_count-1])!=0 && flag==0)
                    {
                        strcpy(arr[past_count],pseudo);
                        past_count++;
                    }
                }
                else
                {
                    if(flag==0)
                    {
                        strcpy(arr[past_count],pseudo);
                        past_count++;
                    }
                }
            }

        for (int i = 0; i < token_count; i++) 
        {

            if (strchr(tokens[i], '&') != NULL) 
            {
                struct sigaction sa;
                sa.sa_handler = childHandler;
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

                if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                    perror("Signal handler registration failed");
                    exit(EXIT_FAILURE);
                }
                 char *command_copy=strdup(tokens[i]);
                char *ampersand_pos = strchr(command_copy, '&');
                if (ampersand_pos != NULL) {
                    *ampersand_pos = '\0'; // Remove the '&' character from the command
                }

                pid_t pid = fork(); // Create a child process

                if (pid < 0) {
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                } 
                else if (pid == 0) {
                    // Child process
                    pid_t test=getpid();
                    FILE* file=fopen("output1.txt","a");
                    fprintf(file,"%d %s\n",test,command_copy);
                    fclose(file);
                    executeSystemCommand(command_copy); // Execute the comman
                    exit(EXIT_SUCCESS);
                } 
                else {
                    double time_t;
                    struct timeval start, end;
                    printf("%d\n",pid);
                    char foreground_command[MAX_PATH_LENGTH]; 
                    snprintf(foreground_command,sizeof(foreground_command),"%s",ampersand_pos+1);
                    removeFirstCharacter(foreground_command); 

                    if (strncmp(foreground_command, "peek", 4) == 0) 
                    {
                        gettimeofday(&start, NULL);
                        handle_peek_command(foreground_command, current_dir, home_dir, prev_dir);
                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                    }
                    else if(strncmp(foreground_command, "warp", 4) == 0)
                    {
                        gettimeofday(&start, NULL);
                        handle_warp_command(foreground_command, current_dir, home_dir, prev_dir);
                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                    }
                    else if(strncmp(foreground_command, "pastevents", 10) == 0)
                    {
                        gettimeofday(&start, NULL);
                        handle_past_events(foreground_command,current_dir,home_dir,prev_dir,arr);
                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                    }
                    else if(strncmp(foreground_command,"proclore",8)==0)
                    {
                        gettimeofday(&start, NULL);
                        int check_count=0;

                        char *token_space = strtok(foreground_command, " ");

                        char* new[2];
                        new[0] = (char *)malloc(MAX_PATH_LENGTH);  
                        new[1] = (char *)malloc(MAX_PATH_LENGTH); 

                        while(token_space!=NULL)
                        {
                            strcpy(new[check_count],token_space);
                            check_count++;
                            token_space = strtok(NULL, " ");
                        }

                        if(check_count==1)
                        {
                            pid_t pid;
                            pid=getpid();
                            print_process_info(pid,home_dir);
                        }

                        else if(check_count==2)
                        {
                            int b=atoi(new[1]);

                            print_process_info(b,home_dir);
                        }

                        else
                        {
                            printf("Invalid Command\n");
                        }

                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;

                        free(new[0]);
                        free(new[1]);
                    }
                    else if(strncmp(foreground_command,"seek",4)==0)
                    {
                        gettimeofday(&start, NULL);
                        handle_seek(tokens[i],current_dir,home_dir);
                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                    }
                    else
                    {
                        gettimeofday(&start, NULL);
                        executeSystemCommand(foreground_command);
                        gettimeofday(&end, NULL);
                        time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                    }

                    if(time_t>2)
                    {
                        char* check1=strtok(foreground_command," ");
                        printf("%s for %0.0f seconds\n",check1,time_t);
                    }
                    // free(command_copy); 
                }
    
            } 
            else
            {
                double time_t;
                struct timeval start, end;
                if (strncmp(tokens[i], "peek", 4) == 0) 
                {
                    gettimeofday(&start, NULL);
                    handle_peek_command(tokens[i], current_dir, home_dir, prev_dir);
                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                }
                else if(strncmp(tokens[i], "warp", 4) == 0)
                {
                    gettimeofday(&start, NULL);
                    handle_warp_command(tokens[i], current_dir, home_dir, prev_dir);
                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                }
                else if(strncmp(tokens[i], "pastevents", 10) == 0)
                {
                    gettimeofday(&start, NULL);
                    handle_past_events(tokens[i],current_dir,home_dir,prev_dir,arr);
                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;;
                }
                else if(strncmp(tokens[i],"proclore",8)==0)
                {
                    gettimeofday(&start, NULL);
                    int check_count=0;

                   char *token_space = strtok(tokens[i], " ");

                   char* new[2];
                    new[0] = (char *)malloc(MAX_PATH_LENGTH);  
                    new[1] = (char *)malloc(MAX_PATH_LENGTH); 

                    while(token_space!=NULL)
                    {
                        strcpy(new[check_count],token_space);
                        check_count++;
                        token_space = strtok(NULL, " ");
                    }

                    if(check_count==1)
                    {
                        pid_t pid;
                        pid=getpid();
                        print_process_info(pid,home_dir);
                    }

                    else if(check_count==2)
                    {
                        int b=atoi(new[1]);

                        print_process_info(b,home_dir);
                    }

                    else
                    {
                        printf("Invalid Command\n");
                    }

                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;

                    free(new[0]);
                    free(new[1]);
                }
                else if(strncmp(tokens[i], "seek", 4) == 0)
                {
                    gettimeofday(&start, NULL);
                    handle_seek(tokens[i],current_dir,home_dir);
                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                }
                else
                {
                    gettimeofday(&start, NULL);
                    executeSystemCommand(tokens[i]);
                    gettimeofday(&end, NULL);
                    time_t = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/ 1e6;
                }
                if(time_t>2)
                {
                    char* check1=strtok(tokens[i]," ");
                    printf("%s for %0.0f seconds\n",check1,time_t);
                }
            }
        }

        // if (getcwd(current_dir, sizeof(current_dir)) == NULL) 
        // {
        //     perror("getcwd");
        //     exit(EXIT_FAILURE);
        // }
        chdir(home_dir);

        FILE *file = fopen("output.txt", "r");
        if (file == NULL) {
            perror("File opening failed");
        }

        char buffer[1024];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            // Print the read buffer to the shell
            fwrite(buffer, 1, bytes_read, stdout);
        }

        fclose(file);

        // Open the file for truncation (clearing its content)
        file = fopen("output.txt", "w");
        if (file == NULL) {
            perror("File opening failed");
        }
        // chdir(current_dir);

        // Close the file to truncate it
        fclose(file);

    }

    return 0;
}