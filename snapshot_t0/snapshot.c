#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>

int snapshot(char *ssname, char *progpath, char *readme)
{
    char* cur_dir = getcwd(NULL, 0); // get the cwd
    if(cur_dir == NULL)
        return -1;

    struct stat st = {0};

    if (stat(ssname, &st) == -1) {
        mkdir(ssname, 0700);
    }
    else
    {
        free(cur_dir);
        return -1;
    }

    chdir(ssname); // go into snapshot dir

    char* path_to_exe = NULL;
    asprintf(&path_to_exe, "/proc/%d/exe", getpid()); // uses pid of current program

    FILE* exe_source = fopen(path_to_exe, "r");
    if(exe_source == NULL)
    {
        free(cur_dir);
        free(path_to_exe);
        return -1;
    }

    FILE* exe_target = fopen(basename(progpath), "w");
    if(exe_target == NULL)
    {
        fclose(exe_source);
        free(cur_dir);
        free(path_to_exe);
        return -1;
    }

    int ch;
    while ((ch = fgetc(exe_source)) != EOF)
        fputc(ch, exe_target);
    fclose(exe_source);
    fclose(exe_target);

    int status = 0; // these vars are used in the wait() before we tar
    pid_t wpid;

    pid_t p = fork();

    if(p == 0) // we are in child
    {
        free(cur_dir); // free the stuff that child has because of fork
        free(path_to_exe);
        FILE* target = fopen("README.txt", "w"); // create README.txt in snapshot dir
        if(target == NULL)
        {
            fclose(target);
            return -1;
        }

        int strsuccess = fputs(readme, target);

        if(strsuccess == EOF)
        {
            fclose(target);
            return -1;
        }
        if(readme[strlen(readme) -1] != '\0') // if there isn't a newline, add one
        {
            int newlinesuccess = fputc('\n', target);

            if(newlinesuccess == EOF)
            {
                fclose(target);
                return -1;
            }
        }
        if(fclose(target) == EOF) // close README.txt file
            return -1;

        struct rlimit limit; // set ulimit to max

        limit.rlim_cur = RLIM_INFINITY;
        limit.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &limit) != 0)
            return -1;

        abort();
    }

    chdir(".."); // go back to original cwd

    char* tar_cmd = NULL;
    asprintf(&tar_cmd, "tar czf %s.tgz %s", ssname, ssname);

    while ((wpid = wait(&status)) > 0);

    system(tar_cmd);

    chdir(ssname); // go back into snapshot dir to delete contents

    char* name_of_dump = NULL;
    asprintf(&name_of_dump, "core.%d", p); // use the pid of the child that aborted

    remove(name_of_dump);
    remove("README.txt"); // delete README
    remove(basename(progpath)); // delete executable

    chdir(".."); // go back into original dir
    remove(ssname);

    free(cur_dir);
    free(name_of_dump);
    free(tar_cmd);

    return 0;
}