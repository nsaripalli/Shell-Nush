#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tokens.h"
#include "svec.h"

/*
 -----------------------   BNF ------------------------
command:= <program> <arg>...
        | <command>'; ' <command>
        | <command> && <command>
        | <command> || <command>
        | <command> &
        | <command> '<' <file>
        | <file> '>' <command>
        | <command> | <command>
        | cd <file>
        | exit
        | pwd
        | ls

 */


int eval(svec *command);

//Based off of lecture notes
/*
 * Pipes the left expression into the right side
 */
int
pipe_eval(svec *left, svec *right) {
    int cpid, rv;

    int pipes[2];
    rv = pipe(pipes);
    assert(rv != -1);

    // pipes[0] is for reading
    // pipes[1] is for writing

    if ((cpid = fork())) {

        // parent
        close(pipes[1]);
        close(0);
        rv = dup(pipes[0]);

        assert(rv != -1);

        int status;
        waitpid(cpid, &status, 0);
        exit(eval(right));

    } else {
        // child
        close(pipes[0]);
        close(1); // stdout

        rv = dup(pipes[1]);
        assert(rv != -1);
        exit(eval(left));
        assert(0);
    }

    return -1;
}

//Based on lectures notes
//Returns cpid
//WAIT FOR CHILD
int
execute_background(svec *left) {
    int cpid;


    if ((cpid = fork())) {


        return 0;
    } else {
        exit(eval(left));
    }

    return -1;
}

//Based on lectures notes
int
execute_pipe(svec *left, svec *right) {
    int cpid;


    if ((cpid = fork())) {

        int st;
        waitpid(cpid, &st, 0);
        return st;
    } else {
        exit(pipe_eval(left, right));
    }

    return -1;
}


//Returns the number
int redirIn(svec *cmd, char *file) {
    int cpid;
    if ((cpid = fork())) {
        waitpid(cpid, 0, 0);
    } else {
        fflush(0);
        int fd = open(file, O_RDONLY, 0);
        close(0);

        dup2(fd, STDIN_FILENO);
        close(fd);

        if ((cpid = fork())) {

            int st;
            waitpid(cpid, &st, 0);
            return st;

        } else {
            exit(eval(cmd));
        }

    }
    return -1;
}

//Based on lectures notes
int redirOut(svec *cmd, char *file) {
    int cpid;
    if ((cpid = fork())) {
        waitpid(cpid, 0, 0);
    } else {
        int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        close(1);
        dup(fd);
        close(fd);
        exit(eval(cmd));
    }
    return -1;
}


//Based on lectures notes
int
ls(const char *dir) {
    int rv;
    DIR *wd = opendir(dir);
    assert(wd != 0);

    printf("name\tmode\tsize\n");

    struct dirent *ent;
    struct stat st;
    // show: man readdir
    while ((ent = readdir(wd))) {
        char *name = ent->d_name;
        rv = stat(name, &st);
        assert(rv != -1);

        printf("%s\t%04o\t%ld\n", name, st.st_mode, st.st_size);
    }

    closedir(wd);
    return 0;
}

int cd(const char *dir) {
    if (dir == 0) {
        perror("cd has invalid number of arguments");
        return -1;
    }
    if ((chdir(dir) != 0)) { perror("cd failed"); }
    return 0;
}


int
execute_ret_code(char **cmd) {
    int cpid;


    if ((cpid = fork())) {

        int st;
        waitpid(cpid, &st, 0);
        return st;

    } else {
        execvp(cmd[0], cmd);
        printf("Error in command\n");
    }

    return -1;
}



//From Lecture notes 09

int
streq(const char *aa, const char *bb) {
    return strcmp(aa, bb) == 0;
}

svec *
slice(svec *xs, int i0, int i1) {
    svec *ys = make_svec();
    for (int ii = i0; ii < i1; ++ii) {
        svec_push_back(ys, xs->data[ii]);
    }
    return ys;
}


int
find_first_index(svec *toks) {
    const char *ops[] = {";", "|", ">", "<", "&", "||", "&&"};
    const int number_of_operators = 7;
    int tok_size = toks->size;
    int current_first_index = tok_size + 1;
    const char *current_operator;
    //Prioritize the ; operator
    for (int ii = 0; ii < tok_size; ++ii) {
        if (streq(toks->data[ii], ops[0]) && (ii < current_first_index)) {
            return ii;
        }
    }

    //Priortizie teh | operator
    for (int ii = 0; ii < tok_size; ++ii) {
        if (streq(toks->data[ii], ops[1]) && (ii < current_first_index)) {
            return ii;
        }
    }

    for (int jj = number_of_operators - 1; jj >= 0; --jj) {
        current_operator = ops[jj];
        for (int ii = 0; ii < tok_size; ++ii) {
            if (streq(toks->data[ii], current_operator) && (ii < current_first_index)) {
                current_first_index = ii;
            }
        }
    }

    if (current_first_index == tok_size + 1) {
        return -1;
    }

    return current_first_index;
}


int eval(svec *command) {

    int first_index = find_first_index(command);


    if (first_index != -1) {
        char *first_index_op = svec_get(command, first_index);

        svec *left = slice(command, 0, first_index);
        svec *right = slice(command, first_index + 1, command->size);

        if (strcmp(first_index_op, "|") == 0) {
            execute_pipe(left, right);
        } else if (strcmp(first_index_op, ">") == 0) {
            redirOut(left, right->data[0]);
        } else if (strcmp(first_index_op, "<") == 0) {
            redirIn(left, right->data[0]);
        } else if (strcmp(first_index_op, "&") == 0) {
            execute_background(left);
        } else if (strcmp(first_index_op, "||") == 0) {
            int left_exit_code = execute_ret_code(left->data);
            if (left_exit_code != 0) {
                eval(right);
            }
            return (left_exit_code);
        } else if (strcmp(first_index_op, "&&") == 0) {
            int left_exit_code = execute_ret_code(left->data);
            if (left_exit_code == 0) {
                exit(eval(right));
            }
            return (left_exit_code);
        } else if (strcmp(first_index_op, ";") == 0) {
            eval(left);
            eval(right);
            //Wait for the background processes
            int *st = NULL;
            wait(st);
            free(st);
        }

        free_svec(left);
        free_svec(right);

    } else if (strcmp(command->data[0], "pwd") == 0) {
        char *buf = getcwd(0, 255);
        fwrite(buf, strlen(buf), 1, stdout);
        printf("\n");
        fflush(stdout);
        free(buf);
    } else if (strcmp(command->data[0], "cd") == 0) {
        if (command->size != 2) {
            perror("cd has invalid number of arguments");
        } else {
            cd(command->data[1]);
        }
    } else if (strcmp(command->data[0], "exit") == 0) {
	free_svec(command);
        exit(0);
    } else if (strcmp(command->data[0], "ls") == 0) {
        if (command->size != 1) {
            perror("ls has invalid number of arguments");
        } else {
            char *buf = alloca(255);
            getcwd(buf, 255);
            ls(buf);
        }
    } else {
        char **out = command->data;

        return execute_ret_code(out);
    }


    return 0;
}

/**
 * Runs the nush, a mini shell, program
 *
 * @param argc number of arguments
 * @param argv includes optional file
 * @return exit code of 0 if sucessful
 */
int
main(int argc, char *argv[]) {
    char cmd[256];
    svec *next_line;

    if (argc != 1) {

        int file = open(argv[1], O_RDONLY, 0);


        fflush(stdout);
        do {
            next_line = getFileInput(file);
            eval(next_line);
            fflush(stdout);
            free_svec(next_line);
        } while (next_line != 0);

    } else {
        while (1) {
            printf("nush$ ");
            fflush(stdout);
            next_line = getInput();
            eval(next_line);
            free_svec(next_line);
            fflush(stdout);
        }
    }

    return 0;
}
