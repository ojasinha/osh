#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define OSH_RL_BUFSIZE 1024
#define OSH_TOK_BUFSIZE 64
#define OSH_TOK_DELIM " \t\r\n\a"

void osh_loop();
char *osh_read_line();
char **osh_split_line(char *line);
int osh_execute(char **args);

int main(int argc, char **argv) {
    osh_loop();

    return EXIT_SUCCESS;
}

void osh_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = osh_read_line();
        args = osh_split_line(line);
        status = osh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *osh_read_line() {
    /* Old school */

    // Allocate initial buffer size
    int bufsize = OSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "osh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Our actual reading of line
    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If string length exceeds buffer size we double buffer size
        if (position >= bufsize) {
            bufsize += OSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "osh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return buffer;

    /* With getline() */

    /*
    char *line = NULL;
    ssize_t bufsize = 0; // string size type?
    
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }
    } else {
        perror("readline");
        exit(EXIT_FAILURE);
    }

    return line;
    */
}

char **osh_split_line(char *line) {
    int bufsize = OSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char *) * bufsize);
    char *token;

    if (!tokens) {
        fprintf(stderr, "osh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, OSH_TOK_DELIM);
    while (token) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += OSH_TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char *) * bufsize);
            if (!tokens) {
                fprintf(stderr, "osh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // NULL tells the func to continue tokenizing from where it left off
        token = strtok(NULL, OSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int osh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child
        if (execvp(args[0], args) == -1) {
            perror("osh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("osh");
    } else {
        // Parent
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
Builtin shell commands
*/
int osh_cd(char **args);
int osh_help(char **args);
int osh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

// Array of function pointers
int (*builtin_func[]) (char **) = {
    &osh_cd,
    &osh_help,
    &osh_exit
};

int osh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int osh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "osh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("osh");
        }
    }
    return 1;
}

int osh_help(char **args) {
    int i;
    printf("Ojas' OSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < osh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int osh_exit(char **args) {
    return 0;
}

int osh_execute(char **args) {
    int i;

    // Empty command
    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < osh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return osh_launch(args);
}