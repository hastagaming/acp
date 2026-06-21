#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

static char g_conf_path[ACP_MAX_PATH] = {0};

const char *remote_get_git_dir_path(void) {
    if (g_conf_path[0] == '\0') {
        snprintf(g_conf_path, sizeof(g_conf_path), ".git/acp.conf");
    }
    return g_conf_path;
}

int remote_save(const char *url) {
    if (!url || strlen(url) == 0) {
        error_fatal("Remote URL cannot be empty.");
        return -1;
    }

    if (!util_file_exists(".git")) {
        error_fatal("Not a git repository. Run plain 'acp' first to initialize, or run this from inside a repository.");
        return -1;
    }

    const char *path = remote_get_git_dir_path();
    FILE *f = fopen(path, "w");
    if (!f) {
        error_report(STAGE_REMOTE, 1, "Could not write to .git/acp.conf");
        return -1;
    }

    fprintf(f, "REMOTE=%s\n", url);
    fclose(f);

    char setcmd[ACP_MAX_CMD];
    char output[ACP_MAX_LINE];
    int has_remote = util_run_command("git remote get-url origin", output, sizeof(output));

    if (has_remote == 0) {
        snprintf(setcmd, sizeof(setcmd), "git remote set-url origin %s", url);
    } else {
        snprintf(setcmd, sizeof(setcmd), "git remote add origin %s", url);
    }

    char remote_output[ACP_MAX_LINE];
    int rc = util_run_command(setcmd, remote_output, sizeof(remote_output));
    if (rc != 0) {
        error_report(STAGE_REMOTE, rc, remote_output);
        return -1;
    }

    printf("Remote saved successfully: %s\n", url);
    printf("Stored at: %s\n", path);
    return 0;
}

int remote_load(char *url_out, size_t size) {
    const char *path = remote_get_git_dir_path();
    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char line[ACP_MAX_LINE];
    int found = -1;
    while (fgets(line, sizeof(line), f)) {
        util_trim_newline(line);
        if (strncmp(line, "REMOTE=", 7) == 0) {
            snprintf(url_out, size, "%s", line + 7);
            found = 0;
            break;
        }
    }

    fclose(f);
    return found;
}

int remote_is_configured(void) {
    char buf[ACP_MAX_LINE];
    return remote_load(buf, sizeof(buf)) == 0;
}