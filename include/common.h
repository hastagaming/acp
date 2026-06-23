#ifndef ACP_COMMON_H
#define ACP_COMMON_H

#define ACP_VERSION "1.0.0"
#define ACP_MAX_PATH 4096
#define ACP_MAX_LINE 8192
#define ACP_MAX_CMD 8192

typedef struct {
    const char *commit_message;
    const char *remote_url;
    int show_mode;
    int safe_mode;
    int check_mode;
    int version_mode;
    int has_message;
    int has_remote_arg;

    const char *tag_name;
    int tag_create_mode;
    int tag_remove_mode;
    int tag_push_mode;
    int tag_remove_push_mode;
} AcpOptions;

/* error.c */
typedef enum {
    STAGE_ADD,
    STAGE_COMMIT,
    STAGE_PUSH,
    STAGE_INIT,
    STAGE_REMOTE,
    STAGE_TAG,
    STAGE_GENERAL
} AcpStage;

void error_report(AcpStage stage, int exit_code, const char *raw_output);
void error_fatal(const char *message);

/* git.c */
int git_is_repo(void);
int git_init(void);
int git_add_all(char *output, size_t output_size);
int git_commit(const char *message, char *output, size_t output_size);
int git_push(const char *branch, int safe_mode, char *output, size_t output_size);
int git_current_branch(char *branch_out, size_t branch_size);
int git_has_staged_changes(void);
int git_has_commits(void);

/* parser.c */
int parser_parse(int argc, char **argv, AcpOptions *opts);
void parser_print_usage(void);

/* remote.c */
int remote_save(const char *url);
int remote_load(char *url_out, size_t size);
int remote_is_configured(void);

/* tag.c */
int tag_create(const char *tag_name);
int tag_remove(const char *tag_name);
int tag_push(const char *tag_name);
int tag_remove_push(const char *tag_name);
const char *remote_get_git_dir_path(void);

/* safety.c */
int safety_check_branch_protected(const char *branch);
int safety_block_force_push(const char *push_command);
int safety_run_checks(void);

/* ignore.c */
typedef struct {
    int normal_count;
    int ignored_count;
    char ignored_files[256][ACP_MAX_PATH];
} IgnoreResult;

int ignore_analyze(IgnoreResult *result);

/* util.c */
int util_run_command(const char *cmd, char *output, size_t output_size);
int util_run_command_silent(const char *cmd);
void util_trim_newline(char *str);
int util_file_exists(const char *path);
long util_file_size(const char *path);

/* config.c */
typedef struct {
    char default_branch_fallback[64];
    long large_file_warning_bytes;
    int loaded_from_file;
    char loaded_path[ACP_MAX_PATH];
} AcpConfig;

void config_load(void);
const AcpConfig *config_get(void);

#endif