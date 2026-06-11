#define NOB_UNSTRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

// Scheme-friendly wrappers for FFI:

Nob_Cmd* cmd_new(void) {
    Nob_Cmd *cmd = calloc(1, sizeof(Nob_Cmd));
    return cmd;
}

void cmd_append(Nob_Cmd *cmd, const char *arg) {
    nob__cmd_append(cmd, 1, &arg);
}

bool cmd_run(Nob_Cmd *cmd) {
    Nob_Cmd_Opt opt = {0};
    return nob_cmd_run_opt(cmd, opt);
}

bool cmd_run_redirect(Nob_Cmd *cmd, const char *stdout_path) {
    Nob_Cmd_Opt opt = {0};
    opt.stdout_path = stdout_path;
    return nob_cmd_run_opt(cmd, opt);
}

void cmd_free(Nob_Cmd *cmd) {
    if (cmd) {
        if (cmd->items) {
            free((void*)cmd->items);
        }
        free(cmd);
    }
}

int needs_rebuild(const char *output_path, const char *input_path) {
    return nob_needs_rebuild1(output_path, input_path);
}
