#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

int git_is_repo(void) {
    return util_file_exists(".git");
}

int git_init(void) {
    char output[ACP_MAX_LINE];
    int rc = util_run_command("git init", output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_INIT, rc, output);
        return rc;
    }
    printf("%s", output);
    return 0;
}

int git_current_branch(char *branch_out, size_t branch_size) {
    char output[ACP_MAX_LINE];
    int rc = util_run_command("git branch --show-current", output, sizeof(output));
    if (rc != 0) {
        return rc;
    }
    util_trim_newline(output);

    if (strlen(output) == 0) {
        /* New repo with no commits yet, no active branch registered.
           Use the configured fallback (DEFAULT_BRANCH_FALLBACK) instead
           of a hardcoded value. */
        const AcpConfig *cfg = config_get();
        snprintf(branch_out, branch_size, "%s", cfg->default_branch_fallback);
        return 0;
    }

    snprintf(branch_out, branch_size, "%s", output);
    return 0;
}

int git_has_commits(void) {
    char output[ACP_MAX_LINE];
    int rc = util_run_command("git rev-parse HEAD", output, sizeof(output));
    return rc == 0;
}

int git_has_staged_changes(void) {
    char output[ACP_MAX_LINE];
    int rc = util_run_command("git diff --cached --name-only", output, sizeof(output));
    if (rc != 0) return 0;
    util_trim_newline(output);
    return strlen(output) > 0;
}

int git_add_all(char *output, size_t output_size) {
    int rc = util_run_command("git add .", output, output_size);
    return rc;
}

int git_commit(const char *message, char *output, size_t output_size) {
    char cmd[ACP_MAX_CMD * 2];
    char escaped[ACP_MAX_CMD];
    size_t j = 0;

    /* Escape characters that would break the double-quoted shell argument */
    for (size_t i = 0; message[i] != '\0' && j < sizeof(escaped) - 2; i++) {
        if (message[i] == '"' || message[i] == '\\' || message[i] == '$' || message[i] == '`') {
            escaped[j++] = '\\';
        }
        escaped[j++] = message[i];
    }
    escaped[j] = '\0';

    snprintf(cmd, sizeof(cmd), "git commit -m \"%s\"", escaped);
    int rc = util_run_command(cmd, output, output_size);
    return rc;
}

int git_push(const char *branch, int safe_mode, char *output, size_t output_size) {
    char cmd[ACP_MAX_CMD];

    if (safe_mode && safety_block_force_push(branch)) {
        snprintf(output, output_size, "Force push blocked by --safe mode.");
        return 1;
    }

    /* Check whether the current branch already has an upstream configured */
    char check_output[ACP_MAX_LINE];
    char check_cmd[ACP_MAX_CMD];
    snprintf(check_cmd, sizeof(check_cmd),
             "git rev-parse --abbrev-ref --symbolic-full-name %s@{upstream}", branch);
    int has_upstream = util_run_command(check_cmd, check_output, sizeof(check_output));

    if (has_upstream == 0) {
        snprintf(cmd, sizeof(cmd), "git push origin %s", branch);
    } else {
        snprintf(cmd, sizeof(cmd), "git push -u origin %s", branch);
    }

    int rc = util_run_command(cmd, output, output_size);
    return rc;
}

/* Reads the file-level changes (added / removed) introduced by the most
   recent commit (HEAD), using "git show --name-status". This single
   command works correctly even for the very first commit in a repo,
   unlike a diff against HEAD~1 which would fail in that case since no
   parent commit exists yet.

   Only files with status 'A' (added) or 'D' (deleted) are recorded.
   Modified files ('M') and renames ('R...') are intentionally skipped,
   since they don't represent a pure addition or removal. */
int git_get_last_commit_changes(CommitFileChanges *changes) {
    changes->added_count = 0;
    changes->removed_count = 0;

    char output[ACP_MAX_LINE * 4];
    int rc = util_run_command("git show --name-status --format=\"\" HEAD",
                               output, sizeof(output));
    if (rc != 0) {
        return rc;
    }

    char *line = output;
    while (line && *line) {
        char *line_end = strchr(line, '\n');
        size_t line_len = line_end ? (size_t)(line_end - line) : strlen(line);

        if (line_len >= 2 && line[1] == '\t') {
            char status = line[0];
            const char *filename = line + 2;
            size_t name_len = line_len - 2;

            if (status == 'A' && changes->added_count < 256 && name_len > 0) {
                size_t copy_len = name_len < ACP_MAX_PATH - 1 ? name_len : ACP_MAX_PATH - 1;
                memcpy(changes->added_files[changes->added_count], filename, copy_len);
                changes->added_files[changes->added_count][copy_len] = '\0';
                changes->added_count++;
            } else if (status == 'D' && changes->removed_count < 256 && name_len > 0) {
                size_t copy_len = name_len < ACP_MAX_PATH - 1 ? name_len : ACP_MAX_PATH - 1;
                memcpy(changes->removed_files[changes->removed_count], filename, copy_len);
                changes->removed_files[changes->removed_count][copy_len] = '\0';
                changes->removed_count++;
            }
            /* Any other status (M, R100, C100, etc.) is intentionally
               not recorded here. */
        }

        line = line_end ? line_end + 1 : NULL;
    }

    return 0;
}