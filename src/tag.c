#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

static int tag_name_is_valid(const char *tag_name) {
    if (!tag_name || strlen(tag_name) == 0) {
        return 0;
    }
    /* Reject characters that would break the shell command or are not
       allowed in a git ref name. This is intentionally conservative. */
    for (size_t i = 0; tag_name[i] != '\0'; i++) {
        char c = tag_name[i];
        if (c == ' ' || c == '"' || c == '\'' || c == '\\' || c == '$' ||
            c == '`' || c == ';' || c == '|' || c == '&' || c == '\n') {
            return 0;
        }
    }
    return 1;
}

static int tag_exists_locally(const char *tag_name) {
    char cmd[ACP_MAX_CMD];
    char output[ACP_MAX_LINE];
    snprintf(cmd, sizeof(cmd), "git tag -l \"%s\"", tag_name);
    util_run_command(cmd, output, sizeof(output));
    util_trim_newline(output);
    return strlen(output) > 0;
}

/* Resolves the push target the same way for both tag_push and
   tag_remove_push: prefer the URL saved via 'acp --remote' in
   .git/acp.conf, otherwise fall back to the remote named "origin". */
static void resolve_push_target(char *target_out, size_t target_size) {
    char saved_url[ACP_MAX_LINE];
    if (remote_load(saved_url, sizeof(saved_url)) == 0 && strlen(saved_url) > 0) {
        snprintf(target_out, target_size, "%s", saved_url);
    } else {
        snprintf(target_out, target_size, "origin");
    }
}

int tag_create(const char *tag_name) {
    if (!git_is_repo()) {
        error_fatal("Not a git repository. Run 'acp' first to initialize one.");
        return 1;
    }

    if (!tag_name_is_valid(tag_name)) {
        error_fatal("Invalid tag name. Avoid spaces and shell special characters.");
        return 1;
    }

    if (tag_exists_locally(tag_name)) {
        char msg[ACP_MAX_LINE];
        snprintf(msg, sizeof(msg),
                 "Tag '%s' already exists locally. Remove it first with "
                 "'acp --rm-tag \"%s\"' if you want to recreate it.",
                 tag_name, tag_name);
        error_fatal(msg);
        return 1;
    }

    char cmd[ACP_MAX_CMD];
    char output[ACP_MAX_LINE];
    snprintf(cmd, sizeof(cmd), "git tag \"%s\"", tag_name);
    int rc = util_run_command(cmd, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_TAG, rc, output);
        return rc;
    }

    printf("Tag created: %s\n", tag_name);
    return 0;
}

int tag_remove(const char *tag_name) {
    if (!git_is_repo()) {
        error_fatal("Not a git repository.");
        return 1;
    }

    if (!tag_name_is_valid(tag_name)) {
        error_fatal("Invalid tag name. Avoid spaces and shell special characters.");
        return 1;
    }

    if (!tag_exists_locally(tag_name)) {
        char msg[ACP_MAX_LINE];
        snprintf(msg, sizeof(msg), "Tag '%s' does not exist locally.", tag_name);
        error_fatal(msg);
        return 1;
    }

    char cmd[ACP_MAX_CMD];
    char output[ACP_MAX_LINE];
    snprintf(cmd, sizeof(cmd), "git tag -d \"%s\"", tag_name);
    int rc = util_run_command(cmd, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_TAG, rc, output);
        return rc;
    }

    printf("Tag removed locally: %s\n", tag_name);
    return 0;
}

int tag_push(const char *tag_name) {
    if (!git_is_repo()) {
        error_fatal("Not a git repository.");
        return 1;
    }

    if (!tag_name_is_valid(tag_name)) {
        error_fatal("Invalid tag name. Avoid spaces and shell special characters.");
        return 1;
    }

    if (!tag_exists_locally(tag_name)) {
        char msg[ACP_MAX_LINE];
        snprintf(msg, sizeof(msg),
                 "Tag '%s' does not exist locally. Create it first with "
                 "'acp --tag \"%s\"'.", tag_name, tag_name);
        error_fatal(msg);
        return 1;
    }

    char target[ACP_MAX_LINE];
    resolve_push_target(target, sizeof(target));

    char cmd[ACP_MAX_CMD * 2];
    char output[ACP_MAX_LINE];
    snprintf(cmd, sizeof(cmd), "git push \"%s\" \"%s\"", target, tag_name);
    int rc = util_run_command(cmd, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_TAG, rc, output);
        return rc;
    }

    printf("Tag pushed: %s -> %s\n", tag_name, target);
    return 0;
}

int tag_remove_push(const char *tag_name) {
    if (!git_is_repo()) {
        error_fatal("Not a git repository.");
        return 1;
    }

    if (!tag_name_is_valid(tag_name)) {
        error_fatal("Invalid tag name. Avoid spaces and shell special characters.");
        return 1;
    }

    char target[ACP_MAX_LINE];
    resolve_push_target(target, sizeof(target));

    char cmd[ACP_MAX_CMD * 2];
    char output[ACP_MAX_LINE];
    snprintf(cmd, sizeof(cmd), "git push \"%s\" :refs/tags/%s", target, tag_name);
    int rc = util_run_command(cmd, output, sizeof(output));
    if (rc != 0) {
        error_report(STAGE_TAG, rc, output);
        return rc;
    }

    printf("Tag removed from remote: %s -> %s\n", tag_name, target);
    return 0;
}