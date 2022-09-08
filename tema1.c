#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#define MAX_INPUT_LINE_SIZE 300

struct Dir;
struct File;

typedef struct Dir {
    char* name;
    struct Dir* parent;
    struct File* head_children_files;
    struct Dir* head_children_dirs;
    struct Dir* next;
} Dir;

typedef struct File {
    char* name;
    struct Dir* parent;
    struct File* next;
} File;

File* create_file(const char* const name) {
    File* file = (File*)malloc(sizeof(File));
    file->name = (char*)malloc(strlen(name) + 1);
    strcpy(file->name, name);
    return file;
}

void touch(Dir* parent, char* name) {
    if (!parent->head_children_files) {
        parent->head_children_files = create_file(name);
    } else {
        File* temp = parent->head_children_files;
        File* prev = NULL;
        while (temp) {
            if (!strcmp(temp->name, name)) {
                printf("File already exists\n");
                return;
            }
            prev = temp;
            temp = temp->next;
        }
        prev->next = create_file(name);
    }
}

Dir* create_dir(Dir* const parent, const char* const name) {
    Dir* dir= (Dir*)malloc(sizeof(Dir));
    dir->head_children_dirs = NULL;
    dir->head_children_files = NULL;
    dir->parent = parent;
    dir->name = (char*)malloc(strlen(name) + 1);
    strcpy(dir->name, name);
    return dir;
}

void mkdir(Dir* parent, char* name) {
    if (!parent->head_children_dirs)
        parent->head_children_dirs = create_dir(parent, name);
    else {
        Dir* temp = parent->head_children_dirs;
        Dir* prev = NULL;
        while (temp != NULL) {
            if (!strcmp(temp->name, name)) {
                printf("Directory already exists\n");
                return;
            }
            prev = temp;
            temp = temp->next;
        }
        prev->next = create_dir(parent, name);
    }
}

void ls(Dir* parent) {
    Dir* temp_dir = parent->head_children_dirs;
    while (temp_dir) {
        printf("%s", temp_dir->name);
        if (temp_dir->next) { printf("\n"); }
        temp_dir = temp_dir->next;
    }
    if (parent->head_children_dirs) { printf("\n"); }
    File* temp_file = parent->head_children_files;
    while (temp_file) {
        printf("%s", temp_file->name);
        if (temp_file->next) { printf("\n"); }
        temp_file = temp_file->next;
    }
    if (parent->head_children_files) { printf("\n"); }
}

void remove_file(File* file) {
    free(file->name);
    free(file);
}

void rm(Dir* parent, char* name) {
    if (!parent) { return; }

    if (!parent->head_children_files) {
        printf("Could not find the file\n");
        return;
    }

    File* temp = parent->head_children_files;
    if (!strcmp(parent->head_children_files->name, name)) {
        parent->head_children_files = parent->head_children_files->next;
        remove_file(temp);
        return;
    } else {
        while (temp->next) {
            if (!strcmp(temp->next->name, name)) {
                File* file_to_be_deleted = temp->next;
                temp->next = temp->next->next;
                remove_file(file_to_be_deleted);
                return;
            }
            temp = temp->next;
        }
    }
    printf("Could not find the file\n");
}

void remove_files_from_dir(Dir* dir) {
    while (dir->head_children_files) {
        File* temp = dir->head_children_files;
        dir->head_children_files = dir->head_children_files->next;
        remove_file(temp);
    }
}

void remove_dir(Dir* dir) {
    if (!dir) { return; }
    while (dir->head_children_dirs) {
        Dir* temp = dir->head_children_dirs;
        dir->head_children_dirs = dir->head_children_dirs->next;
        remove_dir(temp);
    }
    remove_files_from_dir(dir);
    free(dir->name);
    free(dir);
}

void rmdir(Dir* parent, char* name) {
    if (!parent) { return; }

    if (!parent->head_children_dirs) {
        printf("Could not find the dir\n");
        return;
    }

    Dir* temp = parent->head_children_dirs;
    if (!strcmp(parent->head_children_dirs->name, name)) {
        parent->head_children_dirs = parent->head_children_dirs->next;
        remove_dir(temp);
        return;
    } else {
        while (temp->next) {
            if (!strcmp(temp->next->name, name)) {
                Dir* dir_to_be_deleted = temp->next;
                temp->next = temp->next->next;
                remove_dir(dir_to_be_deleted);
                return;
            }
            temp = temp->next;
        }
    }
    printf("Could not find the dir\n");
}

void cd(Dir** target, char *name) {
    if (!(*target)) { return; }

    if(!strcmp(name, "..")) {
        if ((*target)->parent) {
            *target = (*target)->parent;
        }
        return;
    } else {
        Dir* temp = (*target)->head_children_dirs;
        while (temp) {
            if (!strcmp(temp->name, name)) {
                *target = temp;
                return;
            }
            temp = temp->next;
        }
    }
    printf("No directories found!\n");
}

char* pwd(Dir* target) {
    int total_length_of_path = 0;
    Dir* temp = target;
    while (temp) {
        total_length_of_path += 1 + strlen(temp->name); 
        temp = temp->parent;
    }

    char* path = (char*)calloc(total_length_of_path + 1, 1);
    while (target) {
        char temp_name[strlen(target->name) + 1];
        for (int i = 0; i < strlen(target->name); ++i) {
            temp_name[i] = target->name[strlen(target->name) - i - 1];
        }
        temp_name[strlen(target->name)] = '\0';
        strcat(path, temp_name);
        strcat(path, "/");
        target = target->parent;
    }

    for (int i = 0; i < strlen(path) / 2; ++i) {
        char tmp = path[i];
        path[i] = path[strlen(path) - i - 1];
        path[strlen(path) - i - 1] = tmp;
    }

    return path;
}

void stop(Dir* target) {
    if (!target) { return; }
    stop(target->parent);
    remove_dir(target);
}

void tree(Dir* target, int level) {
    for (Dir* temp = target->head_children_dirs; temp; temp = temp->next) {
        for (int i = 0; i < level; ++i) {
            printf("    ");
        }
        printf("%s\n", temp->name);
        tree(temp, level + 1);
    }
    for (File* temp = target->head_children_files; temp; temp = temp->next) {
        for (int i = 0; i< level; ++i) {
            printf("    ");
        }
        printf("%s", temp->name);
        if (temp->next) { printf(" "); }
    }
    if (target->head_children_files) { printf("\n"); }
}

void mv(Dir* parent, char *oldname, char *newname) {
    File* temp_file = parent->head_children_files;
    File* prev_file = NULL;
    File* file_before_file_to_be_removed = NULL;
    int found_file = 0;
    while (temp_file) {
        if (!strcmp(temp_file->name, newname)) {
            printf("File/Director already exists\n");
            return;
        }
        if (!strcmp(temp_file->name, oldname)) {
            file_before_file_to_be_removed = prev_file;
            found_file = 1;
        }
        prev_file = temp_file;
        temp_file = temp_file->next;
    }
    if (found_file) {
        if (file_before_file_to_be_removed) {
            temp_file = file_before_file_to_be_removed->next;
            file_before_file_to_be_removed->next = file_before_file_to_be_removed->next->next;
        } else {
            temp_file = parent->head_children_files;
            parent->head_children_files = parent->head_children_files->next;
        }
        remove_file(temp_file);
        return;
    }


    Dir* temp_dir = parent->head_children_dirs;
    Dir* prev_dir = NULL;
    Dir* dir_before_dir_to_be_removed = NULL;
    int found_dir = 0;
    while (temp_dir) {
        if (!strcmp(temp_dir->name, newname)) {
            printf("File/Director already exists\n");
            return;
        }
        if (!strcmp(temp_dir->name, oldname)) {
            dir_before_dir_to_be_removed = prev_dir;
            found_dir = 1;
        }
        prev_dir = temp_dir;
        temp_dir = temp_dir->next;
    }
    if (found_dir) {
        if (dir_before_dir_to_be_removed) {
            temp_dir = dir_before_dir_to_be_removed->next;
            dir_before_dir_to_be_removed->next = dir_before_dir_to_be_removed->next->next;
        } else {
            temp_dir = parent->head_children_dirs;
            parent->head_children_dirs = parent->head_children_dirs->next;
        }
        remove_dir(temp_dir);
        return;
    }
}

int main() {
    Dir* curr_dir = (Dir*)malloc(sizeof(Dir));
    curr_dir->name = malloc(strlen("home") + 1);
    strcpy(curr_dir->name, "home");
    curr_dir->head_children_files = NULL;
    curr_dir->head_children_dirs = NULL;
    curr_dir->next = NULL;
    curr_dir->parent = NULL;
    do {
        char command[MAX_INPUT_LINE_SIZE] = {0};
        char name[MAX_INPUT_LINE_SIZE] = {0};
        char new_name[MAX_INPUT_LINE_SIZE] = {0};
        scanf("%s", command);
        if (!strcmp(command, "touch")) {
            scanf("%s", name);
            touch(curr_dir, name);
        } else if (!strcmp(command, "mkdir")) {
            scanf("%s", name);
            mkdir(curr_dir, name);
        } else if (!strcmp(command, "ls")) {
            ls(curr_dir);
        } else if (!strcmp(command, "pwd")) {
            char* str = pwd(curr_dir);
            printf("%s\n", str);
            free(str);
        } else if (!strcmp(command, "rm")) {
            scanf("%s", name);
            rm(curr_dir, name);
        } else if (!strcmp(command, "rmdir")) {
            scanf("%s", name);
            rmdir(curr_dir, name);
        } else if (!strcmp(command, "cd")) {
            scanf("%s", name);
            cd(&curr_dir, name);
        } else if (!strcmp(command, "tree")) {
            tree(curr_dir, 0);
        } else if (!strcmp(command, "mv")) {
            scanf("%s %s", name, new_name);
            mv(curr_dir, name, new_name);
        } else if (!strcmp(command, "stop")) {
            stop(curr_dir);
            break;
        }
    } while(1);
    return 0;
}
