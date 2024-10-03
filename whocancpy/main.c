#include <stdio.h>
#include <stdlib.h> // malloc()
#include <string.h>
#include <sys/stat.h> // stat(), getpwent()
#include <pwd.h> // getpwent()
#include <errno.h> // errno
#include <linux/limits.h> // max path length (4096)
#include <grp.h> // getgrouplist
#include <libgen.h> // dirname()
#include <dirent.h>
int everyone = 1;

enum
{
    CD,
    DELETE,
    EXECUTE,
    LS,
    READ,
    SEARCH,
    WRITE,
    INVALID
};

typedef struct usernode_
{
    char* pw_name;
    uid_t   pw_uid;        /* user ID */
    gid_t   pw_gid;        /* main group ID */
    struct usernode_* next;
}usernode;

void free_user_list(usernode* head)
{
    if(head->next != NULL)
    {free_user_list(head->next);} // get to end of user_list
    if(head->pw_name)
        free(head->pw_name);
    free(head);
}

usernode* init_user_list(void) // void otherwise get warning that "function declaration isn't prototype"
{
    struct passwd* userinfo = getpwent();

    if(userinfo == NULL)
    {fprintf(stderr, "error reading first entry of password database (%s)\n", strerror(errno)); return NULL;}

    usernode* head = (usernode*)malloc(sizeof(usernode));
    head->pw_uid = userinfo->pw_uid;
    head->pw_gid = userinfo->pw_gid;
    head->next = NULL;
    head->pw_name = (char*)calloc(strlen(userinfo->pw_name)+1, sizeof(char));
    strcpy(head->pw_name, userinfo->pw_name);

    userinfo = getpwent();

    usernode* current = head;

    while(userinfo != NULL)
    {
        usernode *tmp = (usernode *) malloc(sizeof(usernode));
        tmp->pw_uid = userinfo->pw_uid;
        tmp->pw_gid = userinfo->pw_gid;
        tmp->next = NULL;
        tmp->pw_name = (char *) calloc(strlen(userinfo->pw_name)+1, sizeof(char));
        strcpy(tmp->pw_name, userinfo->pw_name);

        current->next = tmp;

        current = current->next;

        errno = 0;
        userinfo = getpwent();
        if(userinfo == NULL && errno != 0)
        {
            fprintf(stderr, "error reading entry of password database (%s)\n", strerror(errno));
            free_user_list(head);
            return NULL;
        }
    } // end of while(userinfo != NULL)

    endpwent(); // close password file

    return head;
}

void set_action(int* action, char* user_input)
{
    if(!strcmp("cd", user_input))
        *action = CD;
    else if(!strcmp("delete", user_input))
        *action = DELETE;
    else if(!strcmp("execute", user_input))
        *action = EXECUTE;
    else if(!strcmp("ls", user_input))
        *action = LS;
    else if(!strcmp("read", user_input))
        *action = READ;
    else if(!strcmp("search", user_input))
        *action = SEARCH;
    else if(!strcmp("write", user_input))
        *action = WRITE;
    else
        *action = INVALID;
}

typedef struct piece_of_path_
{
    char* name;
    struct piece_of_path_* next;
}piece_of_path;

void free_piece_of_path_list(piece_of_path* head)
{
    if(head->next != NULL)
        free_piece_of_path_list(head->next);

    if(head->name)
        free(head->name);
    free(head);
}

piece_of_path* parse(char* path_to_file)
{
    piece_of_path* head = (piece_of_path*)malloc(sizeof(piece_of_path)); // head will hold first slash
    head->name = (char*)calloc(2, sizeof(char)); // for '/' and '\0'
    head->name[0] = '/'; // don't need to null terminate explicitly because of calloc

    if(strlen(path_to_file) == 1) // only provided '/'
    {return head;}

    int index_last_slash = 0;
    int amount; // amount to allocate for name of piece of path
    int i = 1; // it's longer than 1 char, so we can start index past the '/'
    char ch;
    char* start = path_to_file + i;
    piece_of_path* helper = head;

    while(1)
    {
        ch = path_to_file[i];
        if (ch == '/' || ch == '\0')
        {
            amount = i - index_last_slash - 1;
            if (amount != 0) // if its zero that means we either hit an immediate slash
            {
                piece_of_path *tmp = (piece_of_path *) malloc(sizeof(piece_of_path));
                tmp->name = (char*) calloc(amount + 1, sizeof(char));
                memcpy(tmp->name, start, amount);
                helper->next = tmp;
                helper = tmp;
            }
            if (ch == '\0'){return head;}
            index_last_slash = i;
            start = path_to_file + i+1; // have start be 1 after '/'
        }
        i++;
    }
}

// helper to make sure user has execute permissions on specific part of parent path
usernode* check_execute_for_prelim_check(char* path, usernode* head)
{
    usernode* head_new_list = head;
    usernode* prev = NULL;

    struct stat sb;
    stat(path, &sb);
    if (!S_ISDIR(sb.st_mode))
    {return NULL;}

    uid_t owner_uid = sb.st_uid;
    gid_t owner_gid = sb.st_gid;
    mode_t user_execute = sb.st_mode & S_IXUSR; // user execute
    mode_t group_execute = sb.st_mode & S_IXGRP; // group execute
    mode_t other_execute = sb.st_mode & S_IXOTH; // group execute

    while(head != NULL)
    {
        int found = 0;

        if (head->pw_uid == owner_uid) // check if owner
        {
            if(user_execute) // check owner execute bit
            {found = 1;}
        }
        else if(head->pw_gid == owner_gid) // check if user in group
        {
            if(group_execute) // check group execute bit
            {found = 1;}
        }
        else if(other_execute) // check if other can execute
        {found = 1;}

        if(found == 0) // remove user from list
        {
            if(head->pw_uid == 0) // do not remove root
            {
                prev = head;
                head = head->next;
            }
            else
            {
                everyone = 0;
                usernode *to_delete = head;
                if (prev == NULL)
                {
                    head = head->next;
                    head_new_list = head;
                }
                else
                {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
        }
        else
        {
            prev = head;
            head = head->next;
        }
    }
    return head_new_list;
}

usernode* check_cd_or_search(usernode* head, char* path, struct stat obj_of_interest)
{
    if(!S_ISDIR(obj_of_interest.st_mode))
    {return NULL;}

    usernode* head_new_list = head;
    usernode* prev = NULL;

    while(head != NULL)
    {
        int found = 0;

        if (head->pw_uid == 0) // check if root
        {
            found = 1;
        }
        else if(head->pw_uid == obj_of_interest.st_uid) // check if user is owner
        {
            if(obj_of_interest.st_mode & S_IXUSR) // check owner execute bit
            {found = 1;}
        }
        else if(head->pw_gid == obj_of_interest.st_gid) // check if user is in group
        {
            if(obj_of_interest.st_mode & S_IXGRP) // check group execute bit
            {found = 1;}
        }
        else if(obj_of_interest.st_mode & S_IXOTH) // check other execute bit
        {found = 1;}

        if(found == 0) // remove user from list
        {
            everyone = 0;
            usernode *to_delete = head;
            if (prev == NULL)
            {
                head = head->next;
                head_new_list = head;
            }
            else
            {
                head = head->next;
                prev->next = head;
            }
            free(to_delete->pw_name);
            free(to_delete);
        }
        else
        {
            prev = head;
            head = head->next;
        }
    }
    return head_new_list;
}

usernode* check_delete(usernode* head, char* path, struct stat obj_of_interest)
{
    usernode* head_new_list = head;
    usernode* prev = NULL;

    if(obj_of_interest.st_mode & S_ISVTX) // check sticky bit
    {
        char* parent_path = dirname(path); // will need to look at parent dir
        struct stat parent;
        stat(parent_path, &parent);

        // iterate over users
        while(head != NULL)
        {
            if (head->pw_uid == 0) // check if root
            {
                prev = head;
                head = head->next;
                continue;
            }

            // check if any writes exist on parent dir
            int write_on_parent = 0;

            if(head->pw_uid == parent.st_uid)
            {
                if(parent.st_mode & S_IWUSR)
                {write_on_parent = 1;}
            }
            else if(head->pw_gid == parent.st_gid)
            {
                if (parent.st_mode & S_IWGRP)
                {write_on_parent = 1;}
            }
            else if (parent.st_mode & S_IWOTH)
            {write_on_parent = 1;}

            // two options if user has write bit on parent
            int found = 0;
            if (write_on_parent == 1)
            {
                if (head->pw_uid == parent.st_uid) // check if user owns parent dir
                {
                    found = 1;
                }
                else if (!S_ISDIR(obj_of_interest.st_mode)) // check if user owns non-dir file to delete
                {
                    if(head->pw_uid == obj_of_interest.st_uid)
                    {found = 1;}
                }
                else // check if user owns all files of the dir
                {
                    DIR *dir = opendir(path);
                    if (dir == NULL) // error, so just return
                    {
                        fprintf(stderr, "Error opening dir: %s\t(%s)", path, strerror(errno));
                        return NULL;
                    }

                    struct dirent *dir_struct = NULL;
                    errno = 0; // THEN NEED TO CHECK IF errno IS STILL 0 WHEN readdir RETURNS NULL
                    int owns_all = 1;

                    while ((dir_struct = readdir(dir)) != NULL)
                    {
                        char *tmp_path = (char *) calloc(strlen(path) + strlen(dir_struct->d_name + 2), sizeof(char));
                        strcat(tmp_path, path);
                        strcat(tmp_path, "/");
                        strcat(tmp_path, dir_struct->d_name);

                        struct stat tmp;
                        stat(tmp_path, &tmp);
                        free(tmp_path);

                        if (head->pw_uid != tmp.st_uid)
                            {owns_all = 0; break;}
                    }

                    if(owns_all == 1)
                    {
                        found = 1;
                    }

                    if(found == 0)
                    {
                        everyone = 0;
                        // remove user from list
                        usernode *to_delete = head;
                        if (prev == NULL)
                        {
                            head = head->next;
                            head_new_list = head;
                        }
                        else
                        {
                            head = head->next;
                            prev->next = head;
                        }
                        free(to_delete->pw_name);
                        free(to_delete);
                    }
                    else
                    {
                        prev = head;
                        head = head->next;
                    }
                }
                if(found == 1)
                {
                    prev = head;
                    head = head->next;
                }
                else
                {
                    everyone = 0;
                    // remove user from list
                    usernode *to_delete = head;
                    if (prev == NULL)
                    {
                        head = head->next;
                        head_new_list = head;
                    }
                    else
                    {
                        head = head->next;
                        prev->next = head;
                    }
                    free(to_delete->pw_name);
                    free(to_delete);
                }

            }
            else // user did not have write on parent dir
            {
                usernode *to_delete = head;
                if (prev == NULL)
                {
                    head = head->next;
                    head_new_list = head;
                }
                else
                {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
        }
    }
    else // not sticky bit
    {
        while (head != NULL)
        {
            int found = 0;
            // check if root
            if(head->pw_uid == 0)
            {found = 1;}

            // check for owner write and execute
            else if(head->pw_uid == obj_of_interest.st_uid)
            {
                if((obj_of_interest.st_mode & S_IWUSR) && (obj_of_interest.st_mode & S_IXUSR))
                {found = 1;}
            }

            // check for group write and execute
            else if (head->pw_gid == obj_of_interest.st_gid)
            {
                if ((obj_of_interest.st_mode & S_IWGRP) && (obj_of_interest.st_mode & S_IXGRP))
                    {found = 1; }
            }
            // check in other
            else if((obj_of_interest.st_mode & S_IWOTH) && (obj_of_interest.st_mode & S_IXOTH))
            {found = 1;}

            // otherwise, delete user from list

            if(found == 0)
            {
                everyone = 0;

                usernode *to_delete = head;
                if (prev == NULL)
                {
                    head = head->next;
                    head_new_list = head;
                }
                else
                {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
            else
            {
                prev = head;
                head = head->next;
            }
        }
    }
    return head_new_list;
}

usernode* check_execute(usernode* head, char* path, struct stat obj_of_interest)
{
    if(!S_ISREG(obj_of_interest.st_mode))
    {fprintf(stderr, "cannot execute nonfile: %s", path); return NULL;}

    usernode* head_new_list = head;
    usernode* prev = NULL;

    while(head != NULL)
    {
        int found = 0;

        if(head->pw_uid == 0) // if user is root and has the execute bit in any of the permission groups
        {
            if ((obj_of_interest.st_mode & S_IXUSR) ||
                (obj_of_interest.st_mode & S_IXGRP) ||
                (obj_of_interest.st_mode & S_IXOTH))
            {found = 1;}
        }
        else if(obj_of_interest.st_uid == head->pw_uid)
        {
            if(obj_of_interest.st_mode & S_IXUSR)
            {found = 1;}
        }
        else if(obj_of_interest.st_gid == head->pw_gid)
        {
            if(obj_of_interest.st_mode & S_IXGRP)
            {found = 1;}
        }
        else if(obj_of_interest.st_mode & S_IXOTH)
        {found = 1;}

        if(found == 0) // remove user from list
        {
            everyone = 0;
            usernode *to_delete = head;
            if (prev == NULL)
            {
                head = head->next;
                head_new_list = head;
            }
            else
            {
                head = head->next;
                prev->next = head;
            }
            free(to_delete->pw_name);
            free(to_delete);
        }
        else
        {
            prev = head;
            head = head->next;
        }
    }
    return head_new_list;
}

usernode* check_ls(usernode* head, char* path, struct stat obj_of_interest)
{
    usernode* head_new_list = head;
    usernode* prev = NULL;

    if(S_ISDIR(obj_of_interest.st_mode))
    {
        while(head != NULL)
        {
            int found = 0;

            if (head->pw_uid == 0) // check if root
            {
                found = 1;
            }
            else if(head->pw_uid == obj_of_interest.st_uid) // check owner read bit
            {
                if(obj_of_interest.st_mode & S_IRUSR)
                {found = 1;}
            }
            else if(head->pw_gid == obj_of_interest.st_gid) // check the read bit for group
            {
                if(obj_of_interest.st_mode & S_IRGRP)
                {found = 1;}
            }
            else if(obj_of_interest.st_mode & S_IROTH)
            {found = 1;}

            if(found == 0) // remove user from list
            {
                everyone = 0;
                usernode *to_delete = head;
                if (prev == NULL)
                {
                    head = head->next;
                    head_new_list = head;
                }
                else
                {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
            else
            {
                prev = head;
                head = head->next;
            }
        }
    }
    else if (S_ISREG(obj_of_interest.st_mode) || S_ISBLK(obj_of_interest.st_mode) || S_ISCHR(obj_of_interest.st_mode))
    {
        char* parent = dirname(path);
        struct stat sb;
        stat(parent, &sb);

        if(!(sb.st_mode & S_IXUSR) && !(sb.st_mode & S_IXGRP) && !(sb.st_mode & S_IXOTH))
        {fprintf(stderr, "no execute bits set on parent directory: %s\n", parent); return NULL;}

        while(head != NULL)
        {
            int found = 0;

            if(head->pw_uid == sb.st_uid) // check owner execute bit
            {
                if (sb.st_mode & S_IXUSR)
                {found = 1;}
            }
            else if(head->pw_gid == obj_of_interest.st_gid) // check group execute bit
            {
                if(obj_of_interest.st_mode & S_IXGRP)
                {found=1;}
            }
            else if(obj_of_interest.st_mode & S_IXOTH)
            {found = 1;}

            if(found == 0) // remove user from list
            {
                everyone = 0;
                usernode *to_delete = head;
                if (prev == NULL)
                {
                    head = head->next;
                    head_new_list = head;
                }
                else
                {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
            else
            {
                prev = head;
                head = head->next;
            }
        }
    }
    else // not dir, file, or block device
    {fprintf(stderr, "cannot ls %s\n",path); return NULL;}

    return head_new_list;
}

usernode* check_read(usernode* head, struct stat obj_of_interest)
{
    usernode* head_new_list = head;
    usernode* prev = NULL;

    while(head != NULL)
    {
        int allowed = 0;

        // check if root
        if(head->pw_uid == 0)
        {
            allowed = 1;
        }
        else if(head->pw_uid == obj_of_interest.st_uid) // check if owner
        {
            if(obj_of_interest.st_mode & S_IRUSR) // check owner read bit
            {allowed = 1;}
        }
        else if(head->pw_gid == obj_of_interest.st_gid) // check group read bit
        {
            if(obj_of_interest.st_mode & S_IRGRP) // check group read bit
            {allowed = 1;}
        }
        else if(obj_of_interest.st_mode & S_IROTH)
        {allowed = 1;}

        if(allowed == 0) // remove user from list
        {
            everyone = 0;

            usernode *to_delete = head;
            if (prev == NULL)
            {
                head = head->next;
                head_new_list = head;
            }
            else
            {
                head = head->next;
                prev->next = head;
            }
            free(to_delete->pw_name);
            free(to_delete);
        }
        else
        {
            prev = head; head = head->next;
        }
    }
    return head_new_list;
}

usernode* check_write(usernode* head, struct stat obj_of_interest)
{
    usernode* head_new_list = head;
    usernode* prev = NULL;

    if(S_ISDIR(obj_of_interest.st_mode))
    {
        while (head != NULL)
        {
            int found = 0;

            if(head->pw_uid == 0) // check if root
            {found = 1;}
            else if(head->pw_uid == obj_of_interest.st_uid)
            {
                // check owner write and owner execute
                if((obj_of_interest.st_mode & S_IWUSR) && (obj_of_interest.st_mode & S_IXUSR))
                {found = 1;}
            }
            else if(head->pw_gid == obj_of_interest.st_gid)
            {
                // check group write and group execute
                if((obj_of_interest.st_mode & S_IWGRP) && (obj_of_interest.st_mode & S_IXGRP))
                {found = 1;}
            }
            else if((obj_of_interest.st_mode & S_IWOTH) && (obj_of_interest.st_mode & S_IROTH))
            {found = 1;}

            if(found == 0) // remove user from list
            {
                everyone = 0;
                usernode *to_delete = head;
                if (prev == NULL) {
                    head = head->next;
                    head_new_list = head;
                } else {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
            else
            {
                prev = head;
                head = head->next;
            }
        }
    }
    else // not dir
    {
        while(head != NULL)
        {
            int found = 0;
            if(head->pw_uid == 0) // check root
            {
                found = 1;
            }
            else if(head->pw_uid == obj_of_interest.st_uid) // check owner write bit
            {
                if(obj_of_interest.st_mode & S_IWUSR)
                {found = 1;}
            }
            else if(head->pw_gid == obj_of_interest.st_gid) // check group write bit
            {
                if(obj_of_interest.st_mode & S_IWGRP)
                {found = 1;}
            }
            else if(obj_of_interest.st_mode & S_IWOTH) // check other write bit
            {found = 1;}

            if(found == 0) // remove user from list
            {
                everyone = 0;
                usernode *to_delete = head;
                if (prev == NULL) {
                    head = head->next;
                    head_new_list = head;
                } else {
                    head = head->next;
                    prev->next = head;
                }
                free(to_delete->pw_name);
                free(to_delete);
            }
            else
            {
                prev = head;
                head = head->next;
            }
        }
    }
    return head_new_list;
}

usernode* check_important_permissions(int action_flag, char* path, usernode* head)
{
    struct stat sb;
    stat(path, &sb);

    if(action_flag == CD || action_flag == SEARCH) // cd into dir
    {return check_cd_or_search(head, path, sb);}
    else if(action_flag == DELETE)
    {return check_delete(head, path, sb);}
    else if(action_flag == EXECUTE)
    {return check_execute(head, path, sb);}
    else if(action_flag == LS)
    {return check_ls(head, path,sb);}
    else if(action_flag == READ)
    {
        //check_if_root_in_list(head);
        return check_read(head, sb);
    }
    else if(action_flag == WRITE)
    {return check_write(head, sb);}
    else {return NULL;}
}

usernode* find_who_can(int action_flag, piece_of_path* list_of_path_pieces, usernode* head)
{
    char path_buffer[PATH_MAX];
    memset(path_buffer, 0, PATH_MAX);
    usernode* remaining_users = head;

    int first = 1;

    while(list_of_path_pieces->next != NULL)
    {
        strcat(path_buffer, list_of_path_pieces->name);

        remaining_users = check_execute_for_prelim_check(path_buffer, remaining_users);
        if(remaining_users == NULL)
        {return NULL;}

        if(first == 1) // if we only have '/' in path, skip the concat of a slash
        {first = 0; list_of_path_pieces = list_of_path_pieces->next; continue;}

        strcat(path_buffer, "/");
        list_of_path_pieces = list_of_path_pieces->next;
    }
    // we are now at last piece of path
    if(path_buffer[strlen(path_buffer)-1] != '/')
        strcat(path_buffer, "/");
    strcat(path_buffer, list_of_path_pieces->name);
    return check_important_permissions(action_flag,path_buffer, remaining_users);
}

int get_num_remaining_users(usernode* remaining_users)
{
    int count = 0;
    while(remaining_users != 0)
    {
        count++;
        remaining_users = remaining_users->next;
    }
    return count;
}

static int compareFunction(const void* u1, const void* u2)
{
    return strcmp(*(const char **) u1, *(const char **) u2);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {printf("Improper usage (whocan action fsobj)\n"); return 0;}

    int action_flag = INVALID;

    set_action(&action_flag, argv[1]);

    if(action_flag == INVALID)
    {printf("Invalid option (cd, delete, execute, ls, read, search, write)\n"); return 0;}

    usernode* head = init_user_list();
    //check_if_root_in_list(head);

    if(head == NULL)
    {fprintf(stderr, "Error reading password file. Exiting\n"); free_user_list(head); return 1;}

    char* complete_path = realpath(argv[2], NULL); // NULL will allocate storage for complete_path

    if(complete_path == NULL)
    {fprintf(stderr, "Problem calling realpath: %s\n", strerror(errno));
        free_user_list(head); return 1;}

    piece_of_path* list_of_file_names = parse(complete_path); // creates linked list containing the parts of the path
    usernode* remaining_users = find_who_can(action_flag, list_of_file_names, head);
    usernode* helper  =remaining_users;
    if(remaining_users != NULL)
    {
        if(everyone == 1)
        {printf("(everyone)\n");
            free_user_list(remaining_users);
            free(complete_path);
            free_piece_of_path_list(list_of_file_names);
            return 0;
        }
        int num_remaining_user = get_num_remaining_users(remaining_users);
        char** array_of_names = (char**)malloc(sizeof(char*)*num_remaining_user);
        for(int i =0; i < num_remaining_user; i++)
        {
            array_of_names[i] = helper->pw_name;
            helper = helper->next;
        }
        qsort(array_of_names, num_remaining_user, sizeof(char*), compareFunction);
        for(int i =0; i < num_remaining_user; i++)
        {
            printf("%s\n", array_of_names[i]);
        }
        free_user_list(remaining_users);
    }

    free(complete_path);
    free_piece_of_path_list(list_of_file_names);
    return 0;
}

// pseudo
// create list of users that contains UID, GID, and suppl. GIDs
// from start of path, check if every user in the list can execute (search bit)
// check GIDs too
// if they cannot, remove them from list
// continue until we are in directory that holds file of interest
// check if relevant permissions of file match any users that remain in list
// if perms don't match, remove user from list
// display list of remaining users from list
