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