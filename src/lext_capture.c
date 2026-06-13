/* lext_capture.c — Item #2: Native output capture engine */

#include "lext_capture.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#  include <fcntl.h>
#  define ISWIN 1
#else
#  include <unistd.h>
#  include <errno.h>
#  define ISWIN 0
#endif

/* ------------------------------------------------------------------ */
/* (lext-capture-output thunk)                                         */
/* Calls thunk with no args, captures stdout, returns a Scheme string. */
/* ------------------------------------------------------------------ */

static s7_pointer builtin_capture_output(s7_scheme *sc, s7_pointer args) {
    s7_pointer thunk = s7_car(args);
    if (!s7_is_procedure(thunk))
        return s7_wrong_type_arg_error(sc, "lext-capture-output", 1, thunk, "procedure");

#if ISWIN
    /* Windows: use a temporary file approach with _dup/_dup2 */
    fflush(stdout);
    int saved_fd = _dup(1);
    if (saved_fd < 0)
        return s7_error(sc, s7_make_symbol(sc, "capture-error"),
                        s7_list(sc, 1, s7_make_string(sc, "dup failed")));

    FILE *tmp = tmpfile();
    if (!tmp) {
        _close(saved_fd);
        return s7_error(sc, s7_make_symbol(sc, "capture-error"),
                        s7_list(sc, 1, s7_make_string(sc, "tmpfile failed")));
    }
    int tmp_fd = _fileno(tmp);
    _dup2(tmp_fd, 1);

    s7_call(sc, thunk, s7_nil(sc));

    fflush(stdout);
    _dup2(saved_fd, 1);
    _close(saved_fd);

    rewind(tmp);
    char *buf = NULL;
    size_t buf_sz = 0;
    size_t buf_cp = 0;
    int c;
    while ((c = fgetc(tmp)) != EOF) {
        if (buf_sz + 1 >= buf_cp) {
            buf_cp = (buf_cp + 1) * 2;
            buf = (char *)realloc(buf, buf_cp);
        }
        buf[buf_sz++] = (char)c;
    }
    fclose(tmp);
    if (buf) buf[buf_sz] = '\0';
    s7_pointer result = s7_make_string_with_length(sc, buf ? buf : "", buf_sz);
    free(buf);
    return result;

#else
    /* POSIX: pipe + dup2 */
    fflush(stdout);

    int pipefd[2];
    if (pipe(pipefd) != 0)
        return s7_error(sc, s7_make_symbol(sc, "capture-error"),
                        s7_list(sc, 1, s7_make_string(sc, "pipe() failed")));

    int saved_fd = dup(1);
    if (saved_fd < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return s7_error(sc, s7_make_symbol(sc, "capture-error"),
                        s7_list(sc, 1, s7_make_string(sc, "dup(1) failed")));
    }

    dup2(pipefd[1], 1);
    close(pipefd[1]);

    /* Run the thunk */
    s7_call(sc, thunk, s7_nil(sc));

    fflush(stdout);
    dup2(saved_fd, 1);
    close(saved_fd);

    /* Read all from read end of pipe */
    char *buf = NULL;
    size_t buf_sz = 0;
    size_t buf_cp = 0;
    char tmp[4096];
    ssize_t n;
    while ((n = read(pipefd[0], tmp, sizeof(tmp))) > 0) {
        if (buf_sz + (size_t)n + 1 > buf_cp) {
            buf_cp = (buf_sz + (size_t)n + 1) * 2;
            buf = (char *)realloc(buf, buf_cp);
        }
        memcpy(buf + buf_sz, tmp, (size_t)n);
        buf_sz += (size_t)n;
    }
    close(pipefd[0]);
    if (buf) buf[buf_sz] = '\0';

    s7_pointer result = s7_make_string_with_length(sc, buf ? buf : "", buf_sz);
    free(buf);
    return result;
#endif
}

void lext_capture_register(s7_scheme *sc) {
    s7_define_function(sc, "lext-capture-output", builtin_capture_output, 1, 0, false,
                       "(lext-capture-output thunk) calls thunk and returns stdout as string");
}
