#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"
/* Read a line of characters from stdin. */
int getcmd(char *buf, int nbuf) {

    // ##### Place your code here
    printf(">>>");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);

    return 0;
}

/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn)) void run_command(char *buf, int nbuf, int *pcp) {

    /* Useful data structures and flags. */
    char *arguments[10];

    int numargs = 0;
    /* Word start/end */
    /* int ws = 1; */
    int we = 0;
    int ws = 0;
    int redirection_left = 0;
    int redirection_right = 0;
    char *file_name_l = 0;
    char *file_name_r = 0;

    int p[2];
    int pipe_cmd = 0;

    int sequence_cmd = 0;

    int i = 0;
    for (int n = 0; n < 10; n++) {
        arguments[n] = malloc(sizeof(char) * nbuf);
        memset(arguments[n], 0, sizeof(char) * nbuf);
    }
    /* Parse the command character by character. */
    for (; i < nbuf; i++) {

        /* /\* Parse the current character and set-up various flags: */
        /*    sequence_cmd, redirection, pipe_cmd and similar. *\/ */

        /* ##### Place your code here. */
        // redirection

        if (!(redirection_left || redirection_right)) {
            /* No redirection, continue parsing command. */

            // Place your code here.
            // // Check redirection
            /* printf("get r and buf[i] is %d and i is %d \n", buf[i], i); */
            if (buf[i] == '<') {
                redirection_left = i;
                /* printf("l %d \n", redirection_left); */
            } else if (buf[i] == '>') {
                redirection_right = i;
                /* printf(" r %d \n", redirection_right); */
            }

            //

        } else {
            /* Redirection command. Capture the file names. */

            // ##### Place your code here.
            if (!file_name_l && redirection_left) {
                file_name_l = arguments[numargs];
                /* printf("file_l is %s\n", file_name_l); */
            } else if (!file_name_r && redirection_right) {
                file_name_r = arguments[numargs];

                /* printf("file_r is %s\n", file_name_r); */
            }
        }
        if (i == 0) {

            while (i < nbuf && buf[i] == ' ') {
                i++;
                /* printf("skip head space"); */
            }
            if (i >= nbuf) {
                break;
            }
            ws = i;
        }

        // argument part
        if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == ';' || buf[i] == '|' ||
            buf[i] == '<' || buf[i] == '>') {
            we = i;
            while (i < nbuf &&
                   (buf[i] == ' ' || buf[i] == '\0' || buf[i] == '\n' ||
                    buf[i] == '>' || buf[i] == '<')) {
                if (buf[i] == '<') {
                    redirection_left = i;
                }
                if (buf[i] == '>') {
                    redirection_right = i;
                }

                i++;
            }
            strcpy(arguments[numargs], buf + ws);
            arguments[numargs][we - ws] = '\0';
            ws = i;
            /* printf("argument is %s len is %d\n", arguments[numargs], */
                   /* strlen(arguments[numargs])); */
            numargs++;
        }

        // sequence
        if (buf[i] == ';') {
            sequence_cmd = i;
            /* printf("sequece_cmd %d\n", sequence_cmd); */
            break;
        }

        // pipe_cmd
        if (buf[i] == '|') {
            pipe_cmd = i;
            break;
        }
    }
    /*
      Sequence command. Continue this command in a new process.
      Wait for it to complete and execute the command following ';'.
    */
    if (sequence_cmd) {
        sequence_cmd = 0;
        if (fork() != 0) {
            wait(0);
            // ##### Place your code here.
            /* printf("exec sequence_cmd\n"); */
            /* printf("right is %s", buf + we + 1); */

            run_command(buf + we + 1, nbuf - we - 1, pcp);
        }
    }

    /*
      If this is a redirection command,
      tie the specified files to std in/out.
    */
    if (redirection_left) {
        // ##### Place your code here.
        /* printf("exec red_lef"); */
        if (file_name_l) {
            int fd = open(file_name_l, O_RDONLY);
            if (fd < 0) {
                printf("error of %s", file_name_l);
            }
            /* printf("file name is %s", file_name_l); */

            close(0);
            dup(fd);
            /* int redirection = 0; */
            /* for (; redirection < nbuf; redirection++) { */
            /*     if (buf[redirection] == '<') { */
            /*         buf[redirection] = '\0'; */
            /*         break; */
            /*     } */
            /* } */

            /* printf("redirction is %d",redirection); */
            run_command(buf, redirection_left + 1, pcp);
            close(fd);
            run_command(buf + redirection_left + 1, nbuf - redirection_left - 1,
                        pcp);
        }
    }
    if (redirection_right) {
        // ##### Place your code here.
        if (file_name_r) {
            /* printf("exec left"); */
            int fd = open(file_name_r, O_WRONLY | O_CREATE);
            if (fd < 0) {
                printf("error of %s", file_name_r);
            }

            close(1);

            dup(fd);
            /* printf("yes dup"); */

            /* int redirection = 0; */
            /* for (; redirection < nbuf; redirection++) { */
            /*     if (buf[redirection] == '>') { */
            /*         buf[redirection] = '\0'; */
            /*         break; */
            /*     } */
            /* } */
            /* printf("redirction is %d",redirection); */
            run_command(buf, redirection_right + 1, pcp);
            close(fd);
            run_command(buf + redirection_right + 1,
                        nbuf - redirection_right - 1, pcp);
        }
    }

    /* Parsing done. Execute the command. */

    /*
      If this command is a CD command, write the arguments to the pcp pipe
      and exit with '2' to tell the parent process about this.
    */
    if (strcmp(arguments[0], "cd") == 0) {
        // ##### Place your code here.
        write(pcp[1], arguments[1], strlen(arguments[1]));
        close(pcp[1]);
        exit(2);
    } else {
        /*
          Pipe command: fork twice. Execute the left hand side directly.
          Call run_command recursion for the right side of the pipe.
        */
        if (pipe_cmd) {
            // ##### Place your code here
            pipe(p);
            /* printf("pipe %d\n", pipe_cmd); */
            /* if (pipe(p) < 0) { */
            /* printf("fuck"); */
            /* } else { */
            /* printf("yes"); */
            /* } */
            if (fork() == 0) { // 模仿了xv6自带的sh.c
                /* printf("wit fork"); */
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                for (int n = numargs; n < 10; n++) {
                    /* printf("arguments[n] is %s\n",arguments[n]); */
                    arguments[n] = 0;
                }
                exec(arguments[0], arguments);
            }

            if (fork() == 0) {
                /* printf("read fork"); */
                close(0);
                dup(p[0]);
                close(p[0]);
                close(p[1]);
                /* printf("%s \n", buf + pipe_cmd + 1); */
                run_command(buf + pipe_cmd + 1, nbuf - pipe_cmd - 1, pcp);
            }

            close(p[0]);
            close(p[1]);
            wait(0);
            wait(0);

        } else {
            // ##### Place your code here.
            if (fork() == 0) {
                /* printf("exec other cmd: %s,numargs %d\n", arguments[0],
                 */
                /* numargs); */
                for (int n = numargs; n < 10; n++) {
                    arguments[n] = 0;
                }
                exec(arguments[0], arguments);
                exit(-1);
            } else {
                int s;
                wait(&s);
                /* printf("cs : %d",s); */
            }
        }
        exit(0);
    }
}
int main(void) {

    static char buf[100];

    int pcp[2];
    pipe(pcp);

    /* Read and run input commands. */
    while (getcmd(buf, sizeof(buf)) >= 0) {
        if (fork() == 0)
            run_command(buf, 100, pcp);

        /*
          Check if run_command found this is
          a CD command and run it if required.
        */
        int child_status;
        // ##### Place your code here
        wait(&child_status);
        /* printf("child_status %d\n", child_status); */
        if (child_status == 2) {
            char path[100];
            memset(path, 0, sizeof(path));
            read(pcp[0], path, sizeof(path));
            chdir(path);
            /* if (chdir(path) < 0) { */
            /* printf("切换到路径 %s 失败", buf + 3); */
            /* } else { */
            /* printf("切换成功"); */
            /* } */
        }
    }
    exit(0);
}
