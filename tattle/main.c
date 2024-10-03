#define _XOPEN_SOURCE
#include <utmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
//#include <errno.h>

typedef struct user_struct_ // store utmp structs in linked-list
{
    struct utmp* log;
    struct user_struct_* next;
    char name[UT_NAMESIZE];
} record_link;

typedef struct user_names_link_ // store CL -u names in linked-list
{
    char* name;
    struct user_names_link_* next;
} name_link;

const size_t utmp_struct_size = sizeof(struct utmp);
const size_t name_link_struct_size = sizeof(name_link);
const size_t record_link_struct_size = sizeof(record_link);

void free_record_link_list(record_link* head)
{
    if(head->next != NULL)
    {
        free_record_link_list(head->next);
    }
    free(head->log);
    free(head);
}

void free_name_link_list(name_link* head)
{
    if(head->next != NULL)
    {
        free_name_link_list(head->next);
    }

    free(head->name);
    free(head);
}

name_link* create_user_list(char* str_to_parse)
{
    // turns string of names delim. by ',' into linked list
    name_link head;
    name_link* head_cpy = &head;

    char* cur_char = &str_to_parse[0];
    char* end_of_name = cur_char;

    while((end_of_name = strchr(cur_char, ',')) != NULL)
    {
        *end_of_name = '\0'; // set the comma to null so that cpy works easy
        name_link* tmp = (name_link*)malloc(name_link_struct_size);
        tmp->name = (char*)calloc(1,strlen(cur_char)+1); // +1 on calloc makes last byte stay as '\0'
        strcpy(tmp->name, cur_char);
        printf("current name: %s\n", tmp->name);
        cur_char = end_of_name+1; // advance past ','

        head_cpy->next = tmp;
        head_cpy = head_cpy->next;
    }

    if(*cur_char != '\0')
    {
        //printf("last char was not null: %c\n", cur_char[0]);
        // one final addition to list
        name_link* tmp = (name_link*)malloc(name_link_struct_size);
        tmp->name = (char*)calloc(1, strlen(cur_char)+1);
        strcpy(tmp->name, cur_char);
        //printf("current name: %s\n", tmp->name);
        head_cpy->next = tmp;
        tmp->next = NULL;
    }

    return head.next; // the first link is dummy link
}

record_link* init_list(char* file_to_read, name_link* user_names) // init list only check names
{
    FILE* fptr = fopen(file_to_read, "rb");
    if(fptr == NULL)
    {
        fprintf(stderr, "could not open: %s\n", file_to_read);
        return NULL;
    }

    struct stat st;
    stat(file_to_read, &st);

    size_t file_size = st.st_size;
    size_t num_logs = file_size / utmp_struct_size; // should be evenly divisible

    //printf("%zu / %zu = %zu logs\n", file_size, utmp_struct_size, num_logs); // debug statement

    record_link record_link_head; // create dummy head
    record_link_head.next = NULL;
    record_link* record_link_head_cpy = &record_link_head;

    name_link* name_cpy = user_names;

    char tmpname[UT_NAMESIZE];

    int count = 0;
    while(1)
    {
        struct utmp* fill_from_file = (struct utmp*)calloc(1,utmp_struct_size); // init memory for utmp record
        record_link* cur = (record_link*)calloc(1,record_link_struct_size); // create link for list of users

        size_t amount_read = fread(fill_from_file, utmp_struct_size, 1, fptr);

        if(amount_read == 1) // we read a record
        {
            memcpy(tmpname, fill_from_file->ut_user, UT_NAMESIZE); // cpy name so that strcmps don't complain

            if(strcmp(tmpname, "runlevel") == 0 || strcmp(tmpname, "LOGIN") == 0)
            {
                free(fill_from_file);
                free(cur);
                continue;
            }

            if (user_names != NULL) // loop over list of names
            {
                int found = 0;
                while (user_names != NULL)
                {
                    if (!strcmp(user_names->name, tmpname /*fill_from_file->ut_user*/))
                    {
                       // printf("%s    ==    %s\n", user_names->name, fill_from_file->ut_user);
                        found = 1;
                        break;
                    }

                    else
                        user_names = user_names->next;
                }
                user_names = name_cpy;

                if (found == 1) // add utmp struct to linked list
                {
                    cur->log = fill_from_file;
                    record_link_head_cpy->next = cur;
                    record_link_head_cpy = record_link_head_cpy->next;
                    count++;
                }
                else // have to free if not adding
                {
                    free(fill_from_file);
                    free(cur);
                }
            }
            else // simply add record to list
            {
               cur->log = fill_from_file;
               record_link_head_cpy->next = cur;
               record_link_head_cpy = record_link_head_cpy->next;
               count++;
            }
        }
        else // trouble reading file
        {
            if (ferror(fptr))
            {
                fprintf(stderr, "error reading %s\n", file_to_read);
                // free list up to this point
                return NULL;
            }
            else
            {
                free(cur);
                free(fill_from_file);
                break;
            }
        }
    }
    fclose(fptr);
    return record_link_head.next; // don't return dummy head
}

void print_date(time_t time)
{
    char buff[40];

    struct tm* time_struct = localtime(&time);

    // mm/dd/yy HH:MM”
    strftime(buff, 40, "%D %H:%M", time_struct);
    printf("%-20s", buff);
}

void print_users(record_link* start, record_link* end)
{
    printf("%-25s%-10s", start->log->ut_user, start->log->ut_line);

    print_date(start->log->ut_tv.tv_sec); // logon


    if(end != NULL) // print logoff time if logged off
        print_date(end->log->ut_tv.tv_sec);
    else // otherwise user is still on
        printf("%-20s", "still logged in");

    char tmphost[UT_HOSTSIZE];
    memcpy(tmphost, start->log->ut_host, UT_HOSTSIZE);
    printf("%s\n", tmphost /*start->log->ut_host*/); // host
}

int check_overlap(record_link* start, record_link* pot_end, int time_flag, int date_flag, struct tm time_struct)
{
    if(pot_end != NULL)
    {
        if (time_flag) // need to check if user was on during a certain day's certain time
        {
            // convert start time struct to time_t
            time_t logon_time = start->log->ut_tv.tv_sec;
            time_t loggoff_time = pot_end->log->ut_tv.tv_sec;

            time_t time_of_interest = mktime(&time_struct);

            if ((time_of_interest >= logon_time) && (time_of_interest <= loggoff_time)) // make sure time of interest in range
                return 1;
            else
                return 0;
        }
        else if (date_flag) // Lewis logic
        {
            time_t logon_time = start->log->ut_tv.tv_sec;
            time_t logoff_time = pot_end->log->ut_tv.tv_sec;

            time_t start_of_day = mktime(&time_struct);
            time_t start_of_next_day = start_of_day + 86399;

            if (logoff_time < start_of_day || start_of_next_day < logon_time)
                return 0;
            else // else there is overlap
                return 1;
        }
        else // no chrono-related flags set, so set overlap to 1
            return 1;
    }
    else // potential end is non-existent
    {
        if(time_flag)
        {
            // convert start time struct to time_t
            time_t logon_time = start->log->ut_tv.tv_sec;

            time_t time_of_interest = mktime(&time_struct);
            //printf("%s tof - logon_time: %ld\n", start->log->ut_user, time_of_interest-logon_time);

            if(logon_time < time_of_interest)
                return 1;
            else
                return 0;
        }
        else if(date_flag)
        {
            time_t logon_time = start->log->ut_tv.tv_sec;
            time_t start_of_day = mktime(&time_struct);
            time_t start_of_next_day = start_of_day + 86399;

            if(start_of_next_day < logon_time)
                return 0;
            else
                return 1;
        }
        else // no chrono related flag set
            return 1;
    }
}

void traverse_record_list(record_link* init_result, int time_flag, int date_flag, struct tm time_struct)
{
    record_link* start = init_result;
    printf("%-25s%-10s%-20s%-20s%-20s\n", "login", "tty", "log on", "log off", "from host");

    if (init_result == NULL)
        return;
    char tmputline1[UT_LINESIZE];
    char tmputline2[UT_LINESIZE];

    while(start != NULL) // loop over entire list
    {
        if(start->log->ut_type == DEAD_PROCESS || start->log->ut_type == BOOT_TIME) // it's a "killing" record
        {
            start = start->next;
            continue;
        }

        record_link* pot_end = NULL;
        if(start->next != NULL)
            pot_end = start->next;
        else // we are at end so check if user overlaps
        {
            if(check_overlap(start, NULL, time_flag, date_flag, time_struct))
                print_users(start, NULL);
            break;
        } // leave loop if no more records to check against

        int found_killer = 0;

        while(pot_end != NULL)
        {
            found_killer = 0;

            memcpy(tmputline1, start->log->ut_line, UT_LINESIZE);
            memcpy(tmputline2, pot_end->log->ut_line, UT_LINESIZE);

            if((pot_end->log->ut_type == DEAD_PROCESS || pot_end->log->ut_type == BOOT_TIME) && (strcmp(tmputline1, tmputline2/*start->log->ut_line, pot_end->log->ut_line*/) == 0))
            {
                found_killer = 1;

                if(check_overlap(start, pot_end, time_flag, date_flag, time_struct) == 1) // this record fits our requirements
                {
                    print_users(start, pot_end);
                    break;
                }
                else // no format
                {
                    break; // we found the pot killer with no overlap, so skip user
                }
            }
            else // it wasn't a relevant killing process
                pot_end = pot_end->next;
        }

        if(found_killer == 0)
        {
            // check for overlap
            if(check_overlap(start, NULL, time_flag, date_flag, time_struct) == 1)
                print_users(start, NULL);
        }
        start = start->next;
        found_killer = 0;
    }
}

int main(int argc, char* argv[])
{
    int date_flag = 0;
    int file_flag = 0;
    int time_flag = 0;
    int users_flag = 0;
    int c; // store getopt
    int index; // in need_to_print args from getopt

    char* user_file = NULL;
    char* user_list_str = NULL;

    time_t cur_time = time(NULL);
    struct tm time_struct = *localtime(&cur_time);
    time_struct.tm_sec = 0;
    time_struct.tm_hour = 0;
    time_struct.tm_min = 0;

    while ((c = getopt(argc, argv, "d:f:t:u:")) != -1)
    {
        switch(c)
        {
            case 'd':
                date_flag = 1;
                strptime(optarg, "%D", &time_struct);
                //printf("date: %d, %d, %d\n", time_struct.tm_mon, time_struct.tm_mday, time_struct.tm_year);
                break;

            case 'f':
                file_flag = 1;
                user_file = optarg;
                printf("optarg: %s\n", optarg);
                break;

            case 't':
                time_flag = 1;
                strptime(optarg, "%H:%M\n", &time_struct);
                //printf("HH:MM %d:%d\n", time_struct.tm_hour, time_struct.tm_min);
                break;

            case 'u':
                users_flag = 1;
                user_list_str = optarg;
                break;

            default:
                printf("in need_to_print user arg\n");
                return 1;
        }
        //for(index = optind; index < argc; index++)
         //   printf("Non-option argument %s\n", argv[index]);
    }

    // char elecFile[] = "wtmp_elec_2022_04_27";
    char sysFile[] = "/var/log/wtmp";

    char* str = NULL;

    if(date_flag != 1) // if date flag not set, default to current date
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        //printf("supposed current: %d, %d, %d\n", tm.tm_mon, tm.tm_mday,tm.tm_year);

        time_struct.tm_mon = tm.tm_mon;
        time_struct.tm_mday = tm.tm_mday;
        time_struct.tm_year = tm.tm_year;
    }

    //printf("date as sec: %ld\n", mktime(&time_struct));

    if(file_flag == 1)
        str = user_file;
    else
        str = sysFile;

    name_link* login_link_list = NULL;

    if(users_flag) // if given list of users
        login_link_list = create_user_list(user_list_str);

    record_link *init_result = init_list(str, login_link_list); // this list checks for users

//    while(init_result != NULL)
//    {
//        printf("user: %s\n", init_result->log->ut_user);
//        init_result = init_result->next;
//    }
    if(users_flag)
        free_name_link_list(login_link_list);

    traverse_record_list(init_result, time_flag, date_flag, time_struct);

    if(init_result != NULL)
        free_record_link_list(init_result);

    return 0;
}
