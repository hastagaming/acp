#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../include/common.h"

int util_run_command(const char *cmd, char *output, size_t output_size) {
    if (output && output_size > 0) {
        output[0] = '\0';
    }

    char full_cmd[ACP_MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);

    FILE *pipe = popen(full_cmd, "r");
    if (!pipe) {
        return -1;
    }

    if (output && output_size > 0) {
        size_t used = 0;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            size_t len = strlen(buffer);
            if (used + len < output_size - 1) {
                memcpy(output + used, buffer, len);
                used += len;
                output[used] = '\0';
            }
        }
    } else {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            /* discard */
        }
    }

    int status = pclose(pipe);
    if (status == -1) {
        return -1;
    }
    return WEXITSTATUS(status);
}

int util_run_command_silent(const char *cmd) {
    char full_cmd[ACP_MAX_CMD];
    snprintf(full_cmd, sizeof(full_cmd), "%s >/dev/null 2>&1", cmd);
    int status = system(full_cmd);
    if (status == -1) {
        return -1;
    }
    return WEXITSTATUS(status);
}

void util_trim_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

int util_file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

long util_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return (long)st.st_size;
}
