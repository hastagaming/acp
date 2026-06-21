#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

static int ask_yes_no(const char *question) {
    char answer[16];
    printf("%s (y/n) ", question);
    fflush(stdout);

    if (fgets(answer, sizeof(answer), stdin) == NULL) {
        return 0;
    }
    util_trim_newline(answer);
    return (answer[0] == 'y' || answer[0] == 'Y');
}

static int ensure_repo_exists(void) {
    if (git_is_repo()) {
        return 0;
    }

    int proceed = ask_yes_no("Not a git repository. Initialize now?");
    if (!proceed) {
        printf("Cancelled.\n");
        return -1;
    }

    return git_init();
}

static void print_show_mode(const char *branch, const char *message, int safe_mode) {
    printf("\n=== ACP SHOW MODE ===\n");
    printf("git add .\n");
    printf("git commit -m \"%s\"\n", message);
    if (safe_mode) {
        printf("git push origin %s   (safe mode active)\n", branch);
    } else {
        printf("git push origin %s\n", branch);
    }
    printf("======================\n\n");
}

static int run_acp_flow(AcpOptions *opts) {
    if (ensure_repo_exists() != 0) {
        return 1;
    }

    char branch[256];
    if (git_current_branch(branch, sizeof(branch)) != 0) {
        error_fatal("Failed to detect the current branch.");
        return 1;
    }

    if (opts->safe_mode && safety_check_branch_protected(branch)) {
        printf("Safe mode active: branch '%s' is protected from dangerous pushes.\n", branch);
    }

    if (opts->show_mode) {
        print_show_mode(branch, opts->commit_message, opts->safe_mode);
    }

    IgnoreResult ignore_result;
    ignore_analyze(&ignore_result);

    if (ignore_result.normal_count == 0 && ignore_result.ignored_count > 0) {
        printf("Nothing to commit (all files are ignored).\n");
        return 0;
    }

    if (ignore_result.ignored_count > 0) {
        printf("Warning: %d file(s) skipped because they match .gitignore:\n", ignore_result.ignored_count);
        int max_show = ignore_result.ignored_count < 10 ? ignore_result.ignored_count : 10;
        for (int i = 0; i < max_show; i++) {
            printf("  - %s\n", ignore_result.ignored_files[i]);
        }
        if (ignore_result.ignored_count > 10) {
            printf("  ... and %d more file(s)\n", ignore_result.ignored_count - 10);
        }
    }

    char output[ACP_MAX_LINE * 2];

    int rc = git_add_all(output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_ADD, rc, output);
        return rc;
    }

    if (!git_has_staged_changes()) {
        printf("Nothing to commit.\n");
        return 0;
    }

    rc = git_commit(opts->commit_message, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_COMMIT, rc, output);
        return rc;
    }
    printf("Commit successful: \"%s\"\n", opts->commit_message);

    rc = git_push(branch, opts->safe_mode, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_PUSH, rc, output);
        return rc;
    }
    printf("Push successful to origin/%s\n", branch);

    return 0;
}

int main(int argc, char **argv) {
    config_load();

    AcpOptions opts;
    int parse_result = parser_parse(argc, argv, &opts);

    if (parse_result == -3) {
        parser_print_usage();
        return 0;
    }

    if (parse_result == -1) {
        parser_print_usage();
        return 1;
    }

    if (parse_result == -2) {
        error_fatal("--remote requires a URL. Example: acp --remote \"https://github.com/user/repo.git\"");
        return 1;
    }

    if (opts.version_mode) {
        printf("ACP (Add Commit Push) v%s\n", ACP_VERSION);
        return 0;
    }

    if (opts.check_mode) {
        int warnings = safety_run_checks();
        return warnings > 0 ? 1 : 0;
    }

    if (opts.has_remote_arg) {
        return remote_save(opts.remote_url) == 0 ? 0 : 1;
    }

    if (!opts.has_message) {
        parser_print_usage();
        return 1;
    }

    return run_acp_flow(&opts);
}