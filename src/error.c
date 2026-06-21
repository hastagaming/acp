#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/common.h"

static const char *stage_name(AcpStage stage) {
    switch (stage) {
        case STAGE_ADD: return "git add";
        case STAGE_COMMIT: return "git commit";
        case STAGE_PUSH: return "git push";
        case STAGE_INIT: return "git init";
        case STAGE_REMOTE: return "remote config";
        default: return "general";
    }
}

static const char *stage_explanation(AcpStage stage, const char *raw_output) {
    if (stage == STAGE_PUSH) {
        if (raw_output && strstr(raw_output, "rejected")) {
            return "Push was rejected. The remote branch has new commits that are not in your local branch. Run 'git pull' then try again.";
        }
        if (raw_output && (strstr(raw_output, "Could not resolve host") || strstr(raw_output, "unable to access"))) {
            return "Could not connect to the remote. Check your internet connection or the remote URL.";
        }
        if (raw_output && strstr(raw_output, "Permission denied")) {
            return "Access denied by the remote. Check your credentials (SSH key / token) for this repository.";
        }
        if (raw_output && strstr(raw_output, "no upstream branch")) {
            return "This branch has no upstream on the remote yet. ACP will try to push while setting the upstream automatically.";
        }
        return "Push failed. Check your connection, access rights, and whether the remote is correct.";
    }
    if (stage == STAGE_COMMIT) {
        if (raw_output && strstr(raw_output, "nothing to commit")) {
            return "There are no changes to commit.";
        }
        if (raw_output && (strstr(raw_output, "Please tell me who you are") || strstr(raw_output, "user.email"))) {
            return "Git identity is not configured yet. Run:\n  git config --global user.name \"Your Name\"\n  git config --global user.email \"your@email.com\"";
        }
        return "Commit failed. Check the commit message and file status.";
    }
    if (stage == STAGE_ADD) {
        return "Failed to add files to the staging area. Check file/folder permissions.";
    }
    if (stage == STAGE_INIT) {
        return "Failed to initialize a git repository in this folder.";
    }
    if (stage == STAGE_REMOTE) {
        return "There is a problem with the remote configuration. Check the .git/acp.conf file.";
    }
    return "An unexpected error occurred.";
}

void error_report(AcpStage stage, int exit_code, const char *raw_output) {
    char cwd[ACP_MAX_PATH];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "(could not be detected)");
    }

    fprintf(stderr, "\n=== ACP ERROR ===\n");
    fprintf(stderr, "Stage   : %s\n", stage_name(stage));
    fprintf(stderr, "Code    : %d\n", exit_code);
    fprintf(stderr, "Folder  : %s\n", cwd);

    if (raw_output && strlen(raw_output) > 0) {
        fprintf(stderr, "Details :\n");
        const char *line_start = raw_output;
        const char *p = raw_output;
        while (*p) {
            if (*p == '\n') {
                fprintf(stderr, "  | %.*s\n", (int)(p - line_start), line_start);
                line_start = p + 1;
            }
            p++;
        }
        if (line_start != p) {
            fprintf(stderr, "  | %s\n", line_start);
        }
    }

    fprintf(stderr, "Hint    : %s\n", stage_explanation(stage, raw_output));
    fprintf(stderr, "=================\n\n");
}

void error_fatal(const char *message) {
    fprintf(stderr, "\nACP: %s\n\n", message);
    exit(1);
}