#include <stdlib.h>
#include <stdio.h>
#include <dirent.h> // opendir
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

typedef struct filenode_
{
    char* path_to_file;
    __off_t file_size;
    __ino_t inode;
    int count;
    struct filenode_* next;
    struct filenode_* down_dup;
} filenode;

int cmp_files(char* path1, char* path2)
{
    FILE* file1 = fopen(path1, "r");
    if(file1 == NULL) // return 2 if failed to open file
    {fprintf(stderr, "Could not open: %s for comparison (%s)\n", path1, strerror(errno)); return 2;}

    FILE* file2 = fopen(path2, "r");
    if(file2 == NULL) // return 2 if failed to open file
    {fprintf(stderr, "Could not open: %s for comparison (%s)\n", path2, strerror(errno)); return 2;}

    while(1)
    {
        int ch1 = fgetc(file1);
        int ch2 = fgetc(file2);

        if(ch1 == ch2)
        {
            if(ch1 == EOF) // meaning they both are EOF
            {
                if(feof(file1) && feof(file2)) // check to make sure both are at EOF
                {
                    if(fclose(file1))
                        fprintf(stderr, "Could not close: %s\n", path1);
                    if(fclose(file2))
                        fprintf(stderr, "Could not close: %s\n", path2);
                    return 0;
                }
                else // we weren't at EOF, so return 2
                {fprintf(stderr, "Error when reading file (%s)\n", strerror(errno)); return 2;}
            }
        }
        else
            return 1; // return 1 for didn't match
    }
}

void add_file_to_list(filenode* node_to_add, filenode* start_of_list)
{
    if(start_of_list == NULL)
    {fprintf(stderr, "error with start of list to append\n"); return;}

    filenode* cur = start_of_list;

    while(1) // travel horizontally right with cur
    {
        if(node_to_add->inode == cur->inode) // inodes match
        {
            return;
        }
        else if(node_to_add->file_size == cur->file_size) // sizes match
        {
            int cmp_result = cmp_files(node_to_add->path_to_file, cur->path_to_file);
            if(cmp_result == 1) // they were not equal
            {
                if(cur->next != NULL) // if we are not at end of list, so recurse, starting one link further
                    {add_file_to_list(node_to_add, cur->next);}
                else // we are end of list, so just add to end of list
                {
                  node_to_add->count = 1;
                  cur->next = node_to_add;
                }
            }
            else if (cmp_result == 2) // issue when reading
                return;
            else // result is 0 meaning they are equal
            {
                cur->count++;
                while(cur->down_dup != NULL) // use cur to travel vertically down
                {cur = cur->down_dup;}

                // we are now at bottom of a list of dups
                cur->down_dup = node_to_add;
                return;
            }
        } // end of logic for byte-by-byte comp
        if(cur->next != NULL)
            cur = cur->next;
        else
            break;
    } // execution after this point means no matches and we are last link of horizontal list
    // node to add doesn't match anything so append to list
    node_to_add->count = 1;
    cur->next = node_to_add;
}

void find_all_files(char* path, filenode* startnode) // returns beginning of linkedlist
{
    struct stat sb;
    if(stat(path, &sb) == -1) // error, so just return
    {fprintf(stderr, "Could not open path: %s\t(%s)\n", path, strerror(errno)); return;}

    if((sb.st_mode & S_IFMT) == S_IFREG) // regular file, just add to list correctly
    {
        filenode* node = (filenode*)malloc(sizeof(filenode)); // add filenode to linkedlist
        node->path_to_file = (char*)malloc(strlen(path)+1); // just need room to paste path and '\0'
        strcpy(node->path_to_file, path); // put path in node's path buffer
        node->file_size = sb.st_size;
        node->inode = sb.st_ino;
        node->next = NULL;
        node->down_dup = NULL;
        add_file_to_list(node, startnode);
    }
    else if((sb.st_mode & S_IFMT) == S_IFDIR) // open dir and go over contents of dir
    {
        DIR* dir = opendir(path);
        if(dir == NULL) // error, so just return
        {fprintf(stderr, "Error opening dir: %s\t(%s)", path, strerror(errno)); return;}

        struct dirent* dir_struct = NULL;
        errno = 0; // THEN NEED TO CHECK IF errno IS STILL 0 WHEN readdir RETURNS NULL
        while((dir_struct = readdir(dir)) != NULL)
        {
            if(strcmp(dir_struct->d_name, ".") == 0 || strcmp(dir_struct->d_name, "..") == 0)
            {/*printf("skipped: %s\n", dir_struct->d_name);*/ continue;} // skip parent and current dir

            if(dir_struct->d_type == DT_REG) // regular file
            {
                filenode* node = (filenode*)malloc(sizeof(filenode)); // add filenode to linkedlist
                node->path_to_file = (char*)malloc(strlen(path) + strlen(dir_struct->d_name) + 2); // +2 for '/' and '\0'
                strcpy(node->path_to_file, path); // put path in new buffer
                strcat(node->path_to_file, "/"); // put / to separate appended filename
                strcat(node->path_to_file, dir_struct->d_name);

                if(stat(node->path_to_file, &sb) == -1)
                {fprintf(stderr, "Error using stat on: %s\t(%s)\n", node->path_to_file, strerror(errno)); continue;}

                node->file_size = sb.st_size;
                node->inode = dir_struct->d_ino;
                node->next = NULL;
                node->down_dup = NULL;

                //printf("FILE: %s\t(length: %ld)\n", dir_struct->d_name, node->file_size);
                add_file_to_list(node, startnode);
            }
            else if(dir_struct->d_type == DT_DIR) // dir
            {
                // CHECK HERE FIRST
                // creating path to follow recursively
                char* path_to_follow = NULL;
                path_to_follow = (char*)malloc(strlen(path) + strlen(dir_struct->d_name) + 2);
                strcpy(path_to_follow, path);
                strcat(path_to_follow, "/");
                strcat(path_to_follow, dir_struct->d_name);

                find_all_files(path_to_follow, startnode); // og: dir_struct->d_name
                free(path_to_follow);
            }
        }
        closedir(dir);
    }
}

void print_dups(filenode* start)
{
    while(1)
    {
        int amount;
        if((amount = start->count) > 1)
        {
            filenode* travel_down = start;
            int label = 1;
            while(1)
            {
                printf("%d %d %s\n", amount, label, travel_down->path_to_file);
                label++;
                if(travel_down->down_dup != NULL)
                    travel_down = travel_down->down_dup;
                else
                    break;
            }
        }
        if (start->next != NULL)
            start = start->next;
        else
            break;
    }
}

void free_filenode_list(filenode* head)
{
    if(head->next != NULL)
        free_filenode_list(head->next);
    if(head->down_dup != NULL)
        free_filenode_list(head->down_dup);

    free(head->path_to_file);
    free(head);
}

int main(int argc, char* argv[])
{
    clock_t start, end;
    double cpu_time_used;

    start = clock();


    filenode* head = (filenode*)malloc(sizeof(filenode));
    head->file_size = -1;
    head->next = NULL;
    head->inode = 0;
    head->down_dup = NULL;
    head->count = 1;

    if (argc == 1)
    {
        char* cur_dir = getcwd(NULL, 0);
        if(cur_dir == NULL)
        {fprintf(stderr, "Could not determine starting path (%s)\n", strerror(errno)); return 0;}

        find_all_files(cur_dir, head);
        free(cur_dir);
    }
    else // CLI arguments given
    {
        for (int i = 1; i < argc; i++) // loop over initial entry point
        {
            find_all_files(argv[i], head);
        }
    }

    if(head->next == NULL) // did not link to first dummy node
    {
        free(head);
        return 0;
    }

    print_dups(head->next); // now we are printing dup
    free_filenode_list(head); // free all links in list

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("time to run: %f", cpu_time_used);

    return 0;
}
