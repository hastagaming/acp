#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

#define CONFIG_DEFAULT_BRANCH "main"
#define CONFIG_DEFAULT_LARGE_FILE_MB 50L

static AcpConfig g_config;
static int g_config_loaded = 0;

static void apply_defaults(void) {
    snprintf(g_config.default_branch_fallback,
              sizeof(g_config.default_branch_fallback),
              "%s", CONFIG_DEFAULT_BRANCH);
    g_config.large_file_warning_bytes = CONFIG_DEFAULT_LARGE_FILE_MB * 1024L * 1024L;
    g_config.loaded_from_file = 0;
    g_config.loaded_path[0] = '\0';
}

static void parse_config_line(const char *line) {
    char key[128];
    char value[ACP_MAX_LINE];

    if (line[0] == '#' || line[0] == '\0') {
        return;
    }

    const char *eq = strchr(line, '=');
    if (!eq) {
        return;
    }

    size_t key_len = (size_t)(eq - line);
    if (key_len == 0 || key_len >= sizeof(key)) {
        return;
    }

    memcpy(key, line, key_len);
    key[key_len] = '\0';

    snprintf(value, sizeof(value), "%s", eq + 1);

    if (strcmp(key, "DEFAULT_BRANCH_FALLBACK") == 0) {
        size_t value_len = strlen(value);
        if (value_len > 0 && value_len < sizeof(g_config.default_branch_fallback)) {
            memcpy(g_config.default_branch_fallback, value, value_len + 1);
        }
    } else if (strcmp(key, "LARGE_FILE_WARNING_MB") == 0) {
        long mb = atol(value);
        if (mb > 0) {
            g_config.large_file_warning_bytes = mb * 1024L * 1024L;
        }
    }
}

static int try_load_from(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        return 0;
    }

    char line[ACP_MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        util_trim_newline(line);
        parse_config_line(line);
    }

    fclose(f);

    g_config.loaded_from_file = 1;
    snprintf(g_config.loaded_path, sizeof(g_config.loaded_path), "%s", path);
    return 1;
}

void config_load(void) {
    apply_defaults();

    /* Look for the installed system-wide config first, using $PREFIX
       so this works both on Termux ($PREFIX=/data/data/com.termux/files/usr)
       and on a regular Linux install (falls back to /usr/local). */
    const char *prefix = getenv("PREFIX");
    if (!prefix || strlen(prefix) == 0) {
        prefix = "/usr/local";
    }

    char system_path[ACP_MAX_PATH];
    snprintf(system_path, sizeof(system_path), "%s/etc/acp/default.conf", prefix);

    if (try_load_from(system_path)) {
        g_config_loaded = 1;
        return;
    }

    g_config_loaded = 1;
}

const AcpConfig *config_get(void) {
    if (!g_config_loaded) {
        config_load();
    }
    return &g_config;
}