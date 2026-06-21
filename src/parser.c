#include <stdio.h>
#include <string.h>
#include "../include/common.h"

void parser_print_usage(void) {
    printf("ACP (Add Commit Push) v%s\n\n", ACP_VERSION);
    printf("Usage:\n");
    printf("  acp \"commit message\"      Add, commit, and push all changes\n");
    printf("  acp -s \"commit message\"   Show the git commands before running them\n");
    printf("  acp --remote \"URL\"        Save the remote for this repository\n");
    printf("  acp --safe \"message\"      Run in safe mode (blocks force push)\n");
    printf("  acp --check               Scan for sensitive files before commit/push\n");
    printf("  acp --version             Show the installed ACP version\n");
    printf("\n");
    printf("Examples:\n");
    printf("  acp \"fix login bug\"\n");
    printf("  acp --remote \"https://github.com/user/repo.git\"\n");
    printf("  acp -s \"update readme\"\n");
}

int parser_parse(int argc, char **argv, AcpOptions *opts) {
    opts->commit_message = NULL;
    opts->remote_url = NULL;
    opts->show_mode = 0;
    opts->safe_mode = 0;
    opts->check_mode = 0;
    opts->version_mode = 0;
    opts->has_message = 0;
    opts->has_remote_arg = 0;

    if (argc < 2) {
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--remote") == 0) {
            opts->has_remote_arg = 1;
            if (i + 1 < argc) {
                opts->remote_url = argv[i + 1];
                i++;
            } else {
                return -2;
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            opts->show_mode = 1;
        } else if (strcmp(argv[i], "--safe") == 0) {
            opts->safe_mode = 1;
        } else if (strcmp(argv[i], "--check") == 0) {
            opts->check_mode = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            opts->version_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            return -3;
        } else {
            opts->commit_message = argv[i];
            opts->has_message = 1;
        }
    }

    return 0;
}