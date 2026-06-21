#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/common.h"

int safety_check_branch_protected(const char *branch) {
    if (!branch) return 0;
    if (strcmp(branch, "main") == 0 || strcmp(branch, "master") == 0) {
        return 1;
    }
    return 0;
}

int safety_block_force_push(const char *push_command) {
    if (!push_command) return 0;
    if (strstr(push_command, "--force") || strstr(push_command, "-f ") ||
        strstr(push_command, " -f")) {
        return 1;
    }
    return 0;
}

static int looks_like_secret_line(const char *line) {
    const char *keywords[] = {
        "api_key", "API_KEY", "apikey", "secret", "SECRET",
        "password", "PASSWORD", "token", "TOKEN",
        "private_key", "PRIVATE_KEY", "access_key", "ACCESS_KEY",
        NULL
    };
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strstr(line, keywords[i])) {
            const char *eq = strchr(line, '=');
            const char *colon = strchr(line, ':');
            if (eq || colon) {
                return 1;
            }
        }
    }
    return 0;
}

static void scan_directory_for_secrets(const char *dirpath, int *warning_count,
                                         int depth, long large_file_threshold) {
    if (depth > 4) return;

    DIR *dir = opendir(dirpath);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (strcmp(entry->d_name, ".git") == 0) continue;
        if (strcmp(entry->d_name, "node_modules") == 0) continue;

        char fullpath[ACP_MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_directory_for_secrets(fullpath, warning_count, depth + 1, large_file_threshold);
            continue;
        }

        if (strcmp(entry->d_name, ".env") == 0 ||
            strncmp(entry->d_name, ".env.", 5) == 0) {
            printf("  [WARNING] Environment file detected: %s\n", fullpath);
            (*warning_count)++;
            continue;
        }

        if (st.st_size > large_file_threshold) {
            printf("  [WARNING] Large file (%.1f MB): %s\n",
                   (double)st.st_size / (1024.0 * 1024.0), fullpath);
            (*warning_count)++;
            continue;
        }

        const char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 ||
                    strcmp(ext, ".sh") == 0 || strcmp(ext, ".py") == 0 ||
                    strcmp(ext, ".js") == 0 || strcmp(ext, ".ts") == 0 ||
                    strcmp(ext, ".java") == 0 || strcmp(ext, ".kt") == 0 ||
                    strcmp(ext, ".json") == 0 || strcmp(ext, ".yml") == 0 ||
                    strcmp(ext, ".yaml") == 0 || strcmp(ext, ".env") == 0 ||
                    strcmp(ext, ".conf") == 0 || strcmp(ext, ".txt") == 0)) {

            if (st.st_size > 2L * 1024L * 1024L) continue;

            FILE *f = fopen(fullpath, "r");
            if (!f) continue;

            char line[ACP_MAX_LINE];
            int line_reported = 0;
            while (fgets(line, sizeof(line), f) && !line_reported) {
                if (looks_like_secret_line(line)) {
                    printf("  [WARNING] Possible secret/token in: %s\n", fullpath);
                    (*warning_count)++;
                    line_reported = 1;
                }
            }
            fclose(f);
        }
    }

    closedir(dir);
}

int safety_run_checks(void) {
    const AcpConfig *cfg = config_get();

    printf("Scanning for sensitive files...\n");
    if (cfg->loaded_from_file) {
        printf("(using config: %s)\n", cfg->loaded_path);
    }

    int warning_count = 0;
    scan_directory_for_secrets(".", &warning_count, 0, cfg->large_file_warning_bytes);

    if (warning_count == 0) {
        printf("No suspicious files found.\n");
        return 0;
    }

    printf("\nFound %d warning(s). Review them before committing/pushing.\n", warning_count);
    return warning_count;
}