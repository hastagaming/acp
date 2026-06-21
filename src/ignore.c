#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/common.h"

int ignore_analyze(IgnoreResult *result) {
    result->normal_count = 0;
    result->ignored_count = 0;

    char output[ACP_MAX_LINE * 4];
    int rc = util_run_command("git status --porcelain --ignored", output, sizeof(output));
    if (rc != 0 && rc != -1) {
        /* git status rarely fails in a valid repo, continue with empty data */
    }

    char line[ACP_MAX_LINE];
    size_t i = 0, line_len = 0;
    size_t out_len = strlen(output);

    for (i = 0; i <= out_len; i++) {
        char c = output[i];
        if (c == '\n' || c == '\0') {
            line[line_len] = '\0';
            if (line_len >= 3) {
                /* format: "XY path" for tracked changes, "!! path" for ignored */
                if (line[0] == '!' && line[1] == '!') {
                    if (result->ignored_count < 256) {
                        char *dest = result->ignored_files[result->ignored_count];
                        size_t copy_len = line_len > 3 ? line_len - 3 : 0;
                        if (copy_len >= ACP_MAX_PATH) {
                            copy_len = ACP_MAX_PATH - 1;
                        }
                        memcpy(dest, line + 3, copy_len);
                        dest[copy_len] = '\0';
                        result->ignored_count++;
                    }
                } else if (line[0] != ' ' || line[1] != ' ') {
                    result->normal_count++;
                }
            }
            line_len = 0;
            if (c == '\0') break;
        } else {
            if (line_len < sizeof(line) - 1) {
                line[line_len++] = c;
            }
        }
    }

    return 0;
}