#include <limits.h>
#include <float.h>
#include <stdint.h>
#include <endian.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <glob.h>
#include <wordexp.h>
#include <pwd.h>
#include <grp.h>
#include <fenv.h>
#include "s7/s7.h"

s7_pointer lext_make_libc_constants(s7_scheme *sc, s7_pointer args) {
    s7_pointer env = s7_inlet(sc, s7_nil(sc));
#ifdef SCHAR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "SCHAR_MIN"), s7_make_integer(sc, SCHAR_MIN));
#endif
#ifdef SCHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "SCHAR_MAX"), s7_make_integer(sc, SCHAR_MAX));
#endif
#ifdef UCHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UCHAR_MAX"), s7_make_integer(sc, UCHAR_MAX));
#endif
#ifdef CHAR_BIT
    s7_define(sc, env, s7_make_symbol(sc, "CHAR_BIT"), s7_make_integer(sc, CHAR_BIT));
#endif
#ifdef CHAR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "CHAR_MIN"), s7_make_integer(sc, CHAR_MIN));
#endif
#ifdef CHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "CHAR_MAX"), s7_make_integer(sc, CHAR_MAX));
#endif
#ifdef __WORDSIZE
    s7_define(sc, env, s7_make_symbol(sc, "__WORDSIZE"), s7_make_integer(sc, __WORDSIZE));
#endif
#ifdef SHRT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "SHRT_MIN"), s7_make_integer(sc, SHRT_MIN));
#endif
#ifdef SHRT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "SHRT_MAX"), s7_make_integer(sc, SHRT_MAX));
#endif
#ifdef USHRT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "USHRT_MAX"), s7_make_integer(sc, USHRT_MAX));
#endif
#ifdef INT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_MIN"), s7_make_integer(sc, INT_MIN));
#endif
#ifdef INT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_MAX"), s7_make_integer(sc, INT_MAX));
#endif
#ifdef UINT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_MAX"), s7_make_integer(sc, UINT_MAX));
#endif
#ifdef LONG_MIN
    s7_define(sc, env, s7_make_symbol(sc, "LONG_MIN"), s7_make_integer(sc, LONG_MIN));
#endif
#ifdef LONG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "LONG_MAX"), s7_make_integer(sc, LONG_MAX));
#endif
#ifdef ULONG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "ULONG_MAX"), s7_make_integer(sc, ULONG_MAX));
#endif
#ifdef LLONG_MIN
    s7_define(sc, env, s7_make_symbol(sc, "LLONG_MIN"), s7_make_integer(sc, LLONG_MIN));
#endif
#ifdef LLONG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "LLONG_MAX"), s7_make_integer(sc, LLONG_MAX));
#endif
#ifdef ULLONG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "ULLONG_MAX"), s7_make_integer(sc, ULLONG_MAX));
#endif
#ifdef _POSIX_AIO_LISTIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_AIO_LISTIO_MAX"), s7_make_integer(sc, _POSIX_AIO_LISTIO_MAX));
#endif
#ifdef _POSIX_AIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_AIO_MAX"), s7_make_integer(sc, _POSIX_AIO_MAX));
#endif
#ifdef _POSIX_ARG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_ARG_MAX"), s7_make_integer(sc, _POSIX_ARG_MAX));
#endif
#ifdef _POSIX_CHILD_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_CHILD_MAX"), s7_make_integer(sc, _POSIX_CHILD_MAX));
#endif
#ifdef _POSIX_DELAYTIMER_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_DELAYTIMER_MAX"), s7_make_integer(sc, _POSIX_DELAYTIMER_MAX));
#endif
#ifdef _POSIX_HOST_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_HOST_NAME_MAX"), s7_make_integer(sc, _POSIX_HOST_NAME_MAX));
#endif
#ifdef _POSIX_LINK_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_LINK_MAX"), s7_make_integer(sc, _POSIX_LINK_MAX));
#endif
#ifdef _POSIX_LOGIN_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_LOGIN_NAME_MAX"), s7_make_integer(sc, _POSIX_LOGIN_NAME_MAX));
#endif
#ifdef _POSIX_MAX_CANON
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MAX_CANON"), s7_make_integer(sc, _POSIX_MAX_CANON));
#endif
#ifdef _POSIX_MAX_INPUT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MAX_INPUT"), s7_make_integer(sc, _POSIX_MAX_INPUT));
#endif
#ifdef _POSIX_MQ_OPEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MQ_OPEN_MAX"), s7_make_integer(sc, _POSIX_MQ_OPEN_MAX));
#endif
#ifdef _POSIX_MQ_PRIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MQ_PRIO_MAX"), s7_make_integer(sc, _POSIX_MQ_PRIO_MAX));
#endif
#ifdef _POSIX_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_NAME_MAX"), s7_make_integer(sc, _POSIX_NAME_MAX));
#endif
#ifdef _POSIX_NGROUPS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_NGROUPS_MAX"), s7_make_integer(sc, _POSIX_NGROUPS_MAX));
#endif
#ifdef _POSIX_OPEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_OPEN_MAX"), s7_make_integer(sc, _POSIX_OPEN_MAX));
#endif
#ifdef _POSIX_FD_SETSIZE
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_FD_SETSIZE"), s7_make_integer(sc, _POSIX_FD_SETSIZE));
#endif
#ifdef _POSIX_PATH_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_PATH_MAX"), s7_make_integer(sc, _POSIX_PATH_MAX));
#endif
#ifdef _POSIX_PIPE_BUF
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_PIPE_BUF"), s7_make_integer(sc, _POSIX_PIPE_BUF));
#endif
#ifdef _POSIX_RE_DUP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_RE_DUP_MAX"), s7_make_integer(sc, _POSIX_RE_DUP_MAX));
#endif
#ifdef _POSIX_RTSIG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_RTSIG_MAX"), s7_make_integer(sc, _POSIX_RTSIG_MAX));
#endif
#ifdef _POSIX_SEM_NSEMS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SEM_NSEMS_MAX"), s7_make_integer(sc, _POSIX_SEM_NSEMS_MAX));
#endif
#ifdef _POSIX_SEM_VALUE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SEM_VALUE_MAX"), s7_make_integer(sc, _POSIX_SEM_VALUE_MAX));
#endif
#ifdef _POSIX_SIGQUEUE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SIGQUEUE_MAX"), s7_make_integer(sc, _POSIX_SIGQUEUE_MAX));
#endif
#ifdef _POSIX_SSIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SSIZE_MAX"), s7_make_integer(sc, _POSIX_SSIZE_MAX));
#endif
#ifdef _POSIX_STREAM_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_STREAM_MAX"), s7_make_integer(sc, _POSIX_STREAM_MAX));
#endif
#ifdef _POSIX_SYMLINK_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SYMLINK_MAX"), s7_make_integer(sc, _POSIX_SYMLINK_MAX));
#endif
#ifdef _POSIX_SYMLOOP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SYMLOOP_MAX"), s7_make_integer(sc, _POSIX_SYMLOOP_MAX));
#endif
#ifdef _POSIX_TIMER_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TIMER_MAX"), s7_make_integer(sc, _POSIX_TIMER_MAX));
#endif
#ifdef _POSIX_TTY_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TTY_NAME_MAX"), s7_make_integer(sc, _POSIX_TTY_NAME_MAX));
#endif
#ifdef _POSIX_TZNAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TZNAME_MAX"), s7_make_integer(sc, _POSIX_TZNAME_MAX));
#endif
#ifdef _POSIX_QLIMIT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_QLIMIT"), s7_make_integer(sc, _POSIX_QLIMIT));
#endif
#ifdef _POSIX_HIWAT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_HIWAT"), s7_make_integer(sc, _POSIX_HIWAT));
#endif
#ifdef _POSIX_UIO_MAXIOV
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_UIO_MAXIOV"), s7_make_integer(sc, _POSIX_UIO_MAXIOV));
#endif
#ifdef _POSIX_CLOCKRES_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_CLOCKRES_MIN"), s7_make_integer(sc, _POSIX_CLOCKRES_MIN));
#endif
#ifdef SSIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "SSIZE_MAX"), s7_make_integer(sc, SSIZE_MAX));
#endif
#ifdef NGROUPS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "NGROUPS_MAX"), s7_make_integer(sc, NGROUPS_MAX));
#endif
#ifdef _POSIX2_BC_BASE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_BC_BASE_MAX"), s7_make_integer(sc, _POSIX2_BC_BASE_MAX));
#endif
#ifdef _POSIX2_BC_DIM_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_BC_DIM_MAX"), s7_make_integer(sc, _POSIX2_BC_DIM_MAX));
#endif
#ifdef _POSIX2_BC_SCALE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_BC_SCALE_MAX"), s7_make_integer(sc, _POSIX2_BC_SCALE_MAX));
#endif
#ifdef _POSIX2_BC_STRING_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_BC_STRING_MAX"), s7_make_integer(sc, _POSIX2_BC_STRING_MAX));
#endif
#ifdef _POSIX2_COLL_WEIGHTS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_COLL_WEIGHTS_MAX"), s7_make_integer(sc, _POSIX2_COLL_WEIGHTS_MAX));
#endif
#ifdef _POSIX2_EXPR_NEST_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_EXPR_NEST_MAX"), s7_make_integer(sc, _POSIX2_EXPR_NEST_MAX));
#endif
#ifdef _POSIX2_LINE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_LINE_MAX"), s7_make_integer(sc, _POSIX2_LINE_MAX));
#endif
#ifdef _POSIX2_RE_DUP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_RE_DUP_MAX"), s7_make_integer(sc, _POSIX2_RE_DUP_MAX));
#endif
#ifdef _POSIX2_CHARCLASS_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_CHARCLASS_NAME_MAX"), s7_make_integer(sc, _POSIX2_CHARCLASS_NAME_MAX));
#endif
#ifdef BC_BASE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "BC_BASE_MAX"), s7_make_integer(sc, BC_BASE_MAX));
#endif
#ifdef BC_DIM_MAX
    s7_define(sc, env, s7_make_symbol(sc, "BC_DIM_MAX"), s7_make_integer(sc, BC_DIM_MAX));
#endif
#ifdef BC_SCALE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "BC_SCALE_MAX"), s7_make_integer(sc, BC_SCALE_MAX));
#endif
#ifdef BC_STRING_MAX
    s7_define(sc, env, s7_make_symbol(sc, "BC_STRING_MAX"), s7_make_integer(sc, BC_STRING_MAX));
#endif
#ifdef COLL_WEIGHTS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "COLL_WEIGHTS_MAX"), s7_make_integer(sc, COLL_WEIGHTS_MAX));
#endif
#ifdef EXPR_NEST_MAX
    s7_define(sc, env, s7_make_symbol(sc, "EXPR_NEST_MAX"), s7_make_integer(sc, EXPR_NEST_MAX));
#endif
#ifdef LINE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "LINE_MAX"), s7_make_integer(sc, LINE_MAX));
#endif
#ifdef CHARCLASS_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "CHARCLASS_NAME_MAX"), s7_make_integer(sc, CHARCLASS_NAME_MAX));
#endif
#ifdef RE_DUP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "RE_DUP_MAX"), s7_make_integer(sc, RE_DUP_MAX));
#endif
#ifdef FLT_RADIX
    s7_define(sc, env, s7_make_symbol(sc, "FLT_RADIX"), s7_make_integer(sc, FLT_RADIX));
#endif
#ifdef FLT_MANT_DIG
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MANT_DIG"), s7_make_integer(sc, FLT_MANT_DIG));
#endif
#ifdef DBL_MANT_DIG
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MANT_DIG"), s7_make_integer(sc, DBL_MANT_DIG));
#endif
#ifdef LDBL_MANT_DIG
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MANT_DIG"), s7_make_integer(sc, LDBL_MANT_DIG));
#endif
#ifdef FLT_DIG
    s7_define(sc, env, s7_make_symbol(sc, "FLT_DIG"), s7_make_integer(sc, FLT_DIG));
#endif
#ifdef DBL_DIG
    s7_define(sc, env, s7_make_symbol(sc, "DBL_DIG"), s7_make_integer(sc, DBL_DIG));
#endif
#ifdef LDBL_DIG
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_DIG"), s7_make_integer(sc, LDBL_DIG));
#endif
#ifdef FLT_MIN_EXP
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MIN_EXP"), s7_make_integer(sc, FLT_MIN_EXP));
#endif
#ifdef DBL_MIN_EXP
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MIN_EXP"), s7_make_integer(sc, DBL_MIN_EXP));
#endif
#ifdef LDBL_MIN_EXP
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MIN_EXP"), s7_make_integer(sc, LDBL_MIN_EXP));
#endif
#ifdef FLT_MIN_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MIN_10_EXP"), s7_make_integer(sc, FLT_MIN_10_EXP));
#endif
#ifdef DBL_MIN_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MIN_10_EXP"), s7_make_integer(sc, DBL_MIN_10_EXP));
#endif
#ifdef LDBL_MIN_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MIN_10_EXP"), s7_make_integer(sc, LDBL_MIN_10_EXP));
#endif
#ifdef FLT_MAX_EXP
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MAX_EXP"), s7_make_integer(sc, FLT_MAX_EXP));
#endif
#ifdef DBL_MAX_EXP
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MAX_EXP"), s7_make_integer(sc, DBL_MAX_EXP));
#endif
#ifdef LDBL_MAX_EXP
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MAX_EXP"), s7_make_integer(sc, LDBL_MAX_EXP));
#endif
#ifdef FLT_MAX_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MAX_10_EXP"), s7_make_integer(sc, FLT_MAX_10_EXP));
#endif
#ifdef DBL_MAX_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MAX_10_EXP"), s7_make_integer(sc, DBL_MAX_10_EXP));
#endif
#ifdef LDBL_MAX_10_EXP
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MAX_10_EXP"), s7_make_integer(sc, LDBL_MAX_10_EXP));
#endif
#ifdef FLT_ROUNDS
    s7_define(sc, env, s7_make_symbol(sc, "FLT_ROUNDS"), s7_make_integer(sc, FLT_ROUNDS));
#endif
#ifdef FLT_EVAL_METHOD
    s7_define(sc, env, s7_make_symbol(sc, "FLT_EVAL_METHOD"), s7_make_integer(sc, FLT_EVAL_METHOD));
#endif
#ifdef INT8_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT8_MIN"), s7_make_integer(sc, INT8_MIN));
#endif
#ifdef INT16_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT16_MIN"), s7_make_integer(sc, INT16_MIN));
#endif
#ifdef INT32_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT32_MIN"), s7_make_integer(sc, INT32_MIN));
#endif
#ifdef INT64_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT64_MIN"), s7_make_integer(sc, INT64_MIN));
#endif
#ifdef INT8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT8_MAX"), s7_make_integer(sc, INT8_MAX));
#endif
#ifdef INT16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT16_MAX"), s7_make_integer(sc, INT16_MAX));
#endif
#ifdef INT32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT32_MAX"), s7_make_integer(sc, INT32_MAX));
#endif
#ifdef INT64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT64_MAX"), s7_make_integer(sc, INT64_MAX));
#endif
#ifdef UINT8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT8_MAX"), s7_make_integer(sc, UINT8_MAX));
#endif
#ifdef UINT16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT16_MAX"), s7_make_integer(sc, UINT16_MAX));
#endif
#ifdef UINT32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT32_MAX"), s7_make_integer(sc, UINT32_MAX));
#endif
#ifdef UINT64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT64_MAX"), s7_make_integer(sc, UINT64_MAX));
#endif
#ifdef INT_LEAST8_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST8_MIN"), s7_make_integer(sc, INT_LEAST8_MIN));
#endif
#ifdef INT_LEAST16_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST16_MIN"), s7_make_integer(sc, INT_LEAST16_MIN));
#endif
#ifdef INT_LEAST32_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST32_MIN"), s7_make_integer(sc, INT_LEAST32_MIN));
#endif
#ifdef INT_LEAST64_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST64_MIN"), s7_make_integer(sc, INT_LEAST64_MIN));
#endif
#ifdef INT_LEAST8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST8_MAX"), s7_make_integer(sc, INT_LEAST8_MAX));
#endif
#ifdef INT_LEAST16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST16_MAX"), s7_make_integer(sc, INT_LEAST16_MAX));
#endif
#ifdef INT_LEAST32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST32_MAX"), s7_make_integer(sc, INT_LEAST32_MAX));
#endif
#ifdef INT_LEAST64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_LEAST64_MAX"), s7_make_integer(sc, INT_LEAST64_MAX));
#endif
#ifdef UINT_LEAST8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_LEAST8_MAX"), s7_make_integer(sc, UINT_LEAST8_MAX));
#endif
#ifdef UINT_LEAST16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_LEAST16_MAX"), s7_make_integer(sc, UINT_LEAST16_MAX));
#endif
#ifdef UINT_LEAST32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_LEAST32_MAX"), s7_make_integer(sc, UINT_LEAST32_MAX));
#endif
#ifdef UINT_LEAST64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_LEAST64_MAX"), s7_make_integer(sc, UINT_LEAST64_MAX));
#endif
#ifdef INT_FAST8_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST8_MIN"), s7_make_integer(sc, INT_FAST8_MIN));
#endif
#ifdef INT_FAST16_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST16_MIN"), s7_make_integer(sc, INT_FAST16_MIN));
#endif
#ifdef INT_FAST32_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST32_MIN"), s7_make_integer(sc, INT_FAST32_MIN));
#endif
#ifdef INT_FAST64_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST64_MIN"), s7_make_integer(sc, INT_FAST64_MIN));
#endif
#ifdef INT_FAST8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST8_MAX"), s7_make_integer(sc, INT_FAST8_MAX));
#endif
#ifdef INT_FAST16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST16_MAX"), s7_make_integer(sc, INT_FAST16_MAX));
#endif
#ifdef INT_FAST32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST32_MAX"), s7_make_integer(sc, INT_FAST32_MAX));
#endif
#ifdef INT_FAST64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INT_FAST64_MAX"), s7_make_integer(sc, INT_FAST64_MAX));
#endif
#ifdef UINT_FAST8_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_FAST8_MAX"), s7_make_integer(sc, UINT_FAST8_MAX));
#endif
#ifdef UINT_FAST16_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_FAST16_MAX"), s7_make_integer(sc, UINT_FAST16_MAX));
#endif
#ifdef UINT_FAST32_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_FAST32_MAX"), s7_make_integer(sc, UINT_FAST32_MAX));
#endif
#ifdef UINT_FAST64_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINT_FAST64_MAX"), s7_make_integer(sc, UINT_FAST64_MAX));
#endif
#ifdef INTPTR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INTPTR_MIN"), s7_make_integer(sc, INTPTR_MIN));
#endif
#ifdef INTPTR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INTPTR_MAX"), s7_make_integer(sc, INTPTR_MAX));
#endif
#ifdef UINTPTR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINTPTR_MAX"), s7_make_integer(sc, UINTPTR_MAX));
#endif
#ifdef INTMAX_MIN
    s7_define(sc, env, s7_make_symbol(sc, "INTMAX_MIN"), s7_make_integer(sc, INTMAX_MIN));
#endif
#ifdef INTMAX_MAX
    s7_define(sc, env, s7_make_symbol(sc, "INTMAX_MAX"), s7_make_integer(sc, INTMAX_MAX));
#endif
#ifdef UINTMAX_MAX
    s7_define(sc, env, s7_make_symbol(sc, "UINTMAX_MAX"), s7_make_integer(sc, UINTMAX_MAX));
#endif
#ifdef PTRDIFF_MIN
    s7_define(sc, env, s7_make_symbol(sc, "PTRDIFF_MIN"), s7_make_integer(sc, PTRDIFF_MIN));
#endif
#ifdef PTRDIFF_MAX
    s7_define(sc, env, s7_make_symbol(sc, "PTRDIFF_MAX"), s7_make_integer(sc, PTRDIFF_MAX));
#endif
#ifdef SIG_ATOMIC_MIN
    s7_define(sc, env, s7_make_symbol(sc, "SIG_ATOMIC_MIN"), s7_make_integer(sc, SIG_ATOMIC_MIN));
#endif
#ifdef SIG_ATOMIC_MAX
    s7_define(sc, env, s7_make_symbol(sc, "SIG_ATOMIC_MAX"), s7_make_integer(sc, SIG_ATOMIC_MAX));
#endif
#ifdef SIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "SIZE_MAX"), s7_make_integer(sc, SIZE_MAX));
#endif
#ifdef WCHAR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "WCHAR_MIN"), s7_make_integer(sc, WCHAR_MIN));
#endif
#ifdef WCHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "WCHAR_MAX"), s7_make_integer(sc, WCHAR_MAX));
#endif
#ifdef WINT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "WINT_MIN"), s7_make_integer(sc, WINT_MIN));
#endif
#ifdef WINT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "WINT_MAX"), s7_make_integer(sc, WINT_MAX));
#endif
#ifdef __BYTE_ORDER
    s7_define(sc, env, s7_make_symbol(sc, "__BYTE_ORDER"), s7_make_integer(sc, __BYTE_ORDER));
#endif
#ifdef __BIG_ENDIAN
    s7_define(sc, env, s7_make_symbol(sc, "__BIG_ENDIAN"), s7_make_integer(sc, __BIG_ENDIAN));
#endif
#ifdef __LITTLE_ENDIAN
    s7_define(sc, env, s7_make_symbol(sc, "__LITTLE_ENDIAN"), s7_make_integer(sc, __LITTLE_ENDIAN));
#endif
#ifdef S_IFMT
    s7_define(sc, env, s7_make_symbol(sc, "S_IFMT"), s7_make_integer(sc, S_IFMT));
#endif
#ifdef S_IFDIR
    s7_define(sc, env, s7_make_symbol(sc, "S_IFDIR"), s7_make_integer(sc, S_IFDIR));
#endif
#ifdef S_IFCHR
    s7_define(sc, env, s7_make_symbol(sc, "S_IFCHR"), s7_make_integer(sc, S_IFCHR));
#endif
#ifdef S_IFBLK
    s7_define(sc, env, s7_make_symbol(sc, "S_IFBLK"), s7_make_integer(sc, S_IFBLK));
#endif
#ifdef S_IFREG
    s7_define(sc, env, s7_make_symbol(sc, "S_IFREG"), s7_make_integer(sc, S_IFREG));
#endif
#ifdef S_IFIFO
    s7_define(sc, env, s7_make_symbol(sc, "S_IFIFO"), s7_make_integer(sc, S_IFIFO));
#endif
#ifdef __S_IFLNK
    s7_define(sc, env, s7_make_symbol(sc, "__S_IFLNK"), s7_make_integer(sc, __S_IFLNK));
#endif
#ifdef S_IFSOCK
    s7_define(sc, env, s7_make_symbol(sc, "S_IFSOCK"), s7_make_integer(sc, S_IFSOCK));
#endif
#ifdef S_ISUID
    s7_define(sc, env, s7_make_symbol(sc, "S_ISUID"), s7_make_integer(sc, S_ISUID));
#endif
#ifdef S_ISGID
    s7_define(sc, env, s7_make_symbol(sc, "S_ISGID"), s7_make_integer(sc, S_ISGID));
#endif
#ifdef S_IRUSR
    s7_define(sc, env, s7_make_symbol(sc, "S_IRUSR"), s7_make_integer(sc, S_IRUSR));
#endif
#ifdef S_IWUSR
    s7_define(sc, env, s7_make_symbol(sc, "S_IWUSR"), s7_make_integer(sc, S_IWUSR));
#endif
#ifdef S_IXUSR
    s7_define(sc, env, s7_make_symbol(sc, "S_IXUSR"), s7_make_integer(sc, S_IXUSR));
#endif
#ifdef S_IRWXU
    s7_define(sc, env, s7_make_symbol(sc, "S_IRWXU"), s7_make_integer(sc, S_IRWXU));
#endif
#ifdef S_IRGRP
    s7_define(sc, env, s7_make_symbol(sc, "S_IRGRP"), s7_make_integer(sc, S_IRGRP));
#endif
#ifdef S_IWGRP
    s7_define(sc, env, s7_make_symbol(sc, "S_IWGRP"), s7_make_integer(sc, S_IWGRP));
#endif
#ifdef S_IXGRP
    s7_define(sc, env, s7_make_symbol(sc, "S_IXGRP"), s7_make_integer(sc, S_IXGRP));
#endif
#ifdef S_IRWXG
    s7_define(sc, env, s7_make_symbol(sc, "S_IRWXG"), s7_make_integer(sc, S_IRWXG));
#endif
#ifdef S_IROTH
    s7_define(sc, env, s7_make_symbol(sc, "S_IROTH"), s7_make_integer(sc, S_IROTH));
#endif
#ifdef S_IWOTH
    s7_define(sc, env, s7_make_symbol(sc, "S_IWOTH"), s7_make_integer(sc, S_IWOTH));
#endif
#ifdef S_IXOTH
    s7_define(sc, env, s7_make_symbol(sc, "S_IXOTH"), s7_make_integer(sc, S_IXOTH));
#endif
#ifdef S_IRWXO
    s7_define(sc, env, s7_make_symbol(sc, "S_IRWXO"), s7_make_integer(sc, S_IRWXO));
#endif
#ifdef R_OK
    s7_define(sc, env, s7_make_symbol(sc, "R_OK"), s7_make_integer(sc, R_OK));
#endif
#ifdef W_OK
    s7_define(sc, env, s7_make_symbol(sc, "W_OK"), s7_make_integer(sc, W_OK));
#endif
#ifdef X_OK
    s7_define(sc, env, s7_make_symbol(sc, "X_OK"), s7_make_integer(sc, X_OK));
#endif
#ifdef F_OK
    s7_define(sc, env, s7_make_symbol(sc, "F_OK"), s7_make_integer(sc, F_OK));
#endif
#ifdef SEEK_SET
    s7_define(sc, env, s7_make_symbol(sc, "SEEK_SET"), s7_make_integer(sc, SEEK_SET));
#endif
#ifdef SEEK_CUR
    s7_define(sc, env, s7_make_symbol(sc, "SEEK_CUR"), s7_make_integer(sc, SEEK_CUR));
#endif
#ifdef SEEK_END
    s7_define(sc, env, s7_make_symbol(sc, "SEEK_END"), s7_make_integer(sc, SEEK_END));
#endif
#ifdef F_ULOCK
    s7_define(sc, env, s7_make_symbol(sc, "F_ULOCK"), s7_make_integer(sc, F_ULOCK));
#endif
#ifdef F_LOCK
    s7_define(sc, env, s7_make_symbol(sc, "F_LOCK"), s7_make_integer(sc, F_LOCK));
#endif
#ifdef F_TLOCK
    s7_define(sc, env, s7_make_symbol(sc, "F_TLOCK"), s7_make_integer(sc, F_TLOCK));
#endif
#ifdef F_TEST
    s7_define(sc, env, s7_make_symbol(sc, "F_TEST"), s7_make_integer(sc, F_TEST));
#endif
#ifdef O_ACCMODE
    s7_define(sc, env, s7_make_symbol(sc, "O_ACCMODE"), s7_make_integer(sc, O_ACCMODE));
#endif
#ifdef O_RDONLY
    s7_define(sc, env, s7_make_symbol(sc, "O_RDONLY"), s7_make_integer(sc, O_RDONLY));
#endif
#ifdef O_WRONLY
    s7_define(sc, env, s7_make_symbol(sc, "O_WRONLY"), s7_make_integer(sc, O_WRONLY));
#endif
#ifdef O_RDWR
    s7_define(sc, env, s7_make_symbol(sc, "O_RDWR"), s7_make_integer(sc, O_RDWR));
#endif
#ifdef O_CREAT
    s7_define(sc, env, s7_make_symbol(sc, "O_CREAT"), s7_make_integer(sc, O_CREAT));
#endif
#ifdef O_EXCL
    s7_define(sc, env, s7_make_symbol(sc, "O_EXCL"), s7_make_integer(sc, O_EXCL));
#endif
#ifdef O_NOCTTY
    s7_define(sc, env, s7_make_symbol(sc, "O_NOCTTY"), s7_make_integer(sc, O_NOCTTY));
#endif
#ifdef O_TRUNC
    s7_define(sc, env, s7_make_symbol(sc, "O_TRUNC"), s7_make_integer(sc, O_TRUNC));
#endif
#ifdef O_APPEND
    s7_define(sc, env, s7_make_symbol(sc, "O_APPEND"), s7_make_integer(sc, O_APPEND));
#endif
#ifdef O_NONBLOCK
    s7_define(sc, env, s7_make_symbol(sc, "O_NONBLOCK"), s7_make_integer(sc, O_NONBLOCK));
#endif
#ifdef O_NDELAY
    s7_define(sc, env, s7_make_symbol(sc, "O_NDELAY"), s7_make_integer(sc, O_NDELAY));
#endif
#ifdef O_SYNC
    s7_define(sc, env, s7_make_symbol(sc, "O_SYNC"), s7_make_integer(sc, O_SYNC));
#endif
#ifdef O_FSYNC
    s7_define(sc, env, s7_make_symbol(sc, "O_FSYNC"), s7_make_integer(sc, O_FSYNC));
#endif
#ifdef O_ASYNC
    s7_define(sc, env, s7_make_symbol(sc, "O_ASYNC"), s7_make_integer(sc, O_ASYNC));
#endif
#ifdef O_DSYNC
    s7_define(sc, env, s7_make_symbol(sc, "O_DSYNC"), s7_make_integer(sc, O_DSYNC));
#endif
#ifdef O_RSYNC
    s7_define(sc, env, s7_make_symbol(sc, "O_RSYNC"), s7_make_integer(sc, O_RSYNC));
#endif
#ifdef O_LARGEFILE
    s7_define(sc, env, s7_make_symbol(sc, "O_LARGEFILE"), s7_make_integer(sc, O_LARGEFILE));
#endif
#ifdef F_DUPFD
    s7_define(sc, env, s7_make_symbol(sc, "F_DUPFD"), s7_make_integer(sc, F_DUPFD));
#endif
#ifdef F_GETFD
    s7_define(sc, env, s7_make_symbol(sc, "F_GETFD"), s7_make_integer(sc, F_GETFD));
#endif
#ifdef F_SETFD
    s7_define(sc, env, s7_make_symbol(sc, "F_SETFD"), s7_make_integer(sc, F_SETFD));
#endif
#ifdef F_GETFL
    s7_define(sc, env, s7_make_symbol(sc, "F_GETFL"), s7_make_integer(sc, F_GETFL));
#endif
#ifdef F_SETFL
    s7_define(sc, env, s7_make_symbol(sc, "F_SETFL"), s7_make_integer(sc, F_SETFL));
#endif
#ifdef F_GETLK
    s7_define(sc, env, s7_make_symbol(sc, "F_GETLK"), s7_make_integer(sc, F_GETLK));
#endif
#ifdef F_SETLK
    s7_define(sc, env, s7_make_symbol(sc, "F_SETLK"), s7_make_integer(sc, F_SETLK));
#endif
#ifdef F_SETLKW
    s7_define(sc, env, s7_make_symbol(sc, "F_SETLKW"), s7_make_integer(sc, F_SETLKW));
#endif
#ifdef F_GETLK64
    s7_define(sc, env, s7_make_symbol(sc, "F_GETLK64"), s7_make_integer(sc, F_GETLK64));
#endif
#ifdef F_SETLK64
    s7_define(sc, env, s7_make_symbol(sc, "F_SETLK64"), s7_make_integer(sc, F_SETLK64));
#endif
#ifdef F_SETLKW64
    s7_define(sc, env, s7_make_symbol(sc, "F_SETLKW64"), s7_make_integer(sc, F_SETLKW64));
#endif
#ifdef FD_CLOEXEC
    s7_define(sc, env, s7_make_symbol(sc, "FD_CLOEXEC"), s7_make_integer(sc, FD_CLOEXEC));
#endif
#ifdef F_RDLCK
    s7_define(sc, env, s7_make_symbol(sc, "F_RDLCK"), s7_make_integer(sc, F_RDLCK));
#endif
#ifdef F_WRLCK
    s7_define(sc, env, s7_make_symbol(sc, "F_WRLCK"), s7_make_integer(sc, F_WRLCK));
#endif
#ifdef F_UNLCK
    s7_define(sc, env, s7_make_symbol(sc, "F_UNLCK"), s7_make_integer(sc, F_UNLCK));
#endif
#ifdef POSIX_FADV_NORMAL
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_NORMAL"), s7_make_integer(sc, POSIX_FADV_NORMAL));
#endif
#ifdef POSIX_FADV_RANDOM
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_RANDOM"), s7_make_integer(sc, POSIX_FADV_RANDOM));
#endif
#ifdef POSIX_FADV_SEQUENTIAL
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_SEQUENTIAL"), s7_make_integer(sc, POSIX_FADV_SEQUENTIAL));
#endif
#ifdef POSIX_FADV_WILLNEED
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_WILLNEED"), s7_make_integer(sc, POSIX_FADV_WILLNEED));
#endif
#ifdef POSIX_FADV_DONTNEED
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_DONTNEED"), s7_make_integer(sc, POSIX_FADV_DONTNEED));
#endif
#ifdef POSIX_FADV_NOREUSE
    s7_define(sc, env, s7_make_symbol(sc, "POSIX_FADV_NOREUSE"), s7_make_integer(sc, POSIX_FADV_NOREUSE));
#endif
#ifdef FE_INEXACT
    s7_define(sc, env, s7_make_symbol(sc, "FE_INEXACT"), s7_make_integer(sc, FE_INEXACT));
#endif
#ifdef FE_DIVBYZERO
    s7_define(sc, env, s7_make_symbol(sc, "FE_DIVBYZERO"), s7_make_integer(sc, FE_DIVBYZERO));
#endif
#ifdef FE_UNDERFLOW
    s7_define(sc, env, s7_make_symbol(sc, "FE_UNDERFLOW"), s7_make_integer(sc, FE_UNDERFLOW));
#endif
#ifdef FE_OVERFLOW
    s7_define(sc, env, s7_make_symbol(sc, "FE_OVERFLOW"), s7_make_integer(sc, FE_OVERFLOW));
#endif
#ifdef FE_INVALID
    s7_define(sc, env, s7_make_symbol(sc, "FE_INVALID"), s7_make_integer(sc, FE_INVALID));
#endif
#ifdef FE_ALL_EXCEPT
    s7_define(sc, env, s7_make_symbol(sc, "FE_ALL_EXCEPT"), s7_make_integer(sc, FE_ALL_EXCEPT));
#endif
#ifdef FE_TONEAREST
    s7_define(sc, env, s7_make_symbol(sc, "FE_TONEAREST"), s7_make_integer(sc, FE_TONEAREST));
#endif
#ifdef FE_UPWARD
    s7_define(sc, env, s7_make_symbol(sc, "FE_UPWARD"), s7_make_integer(sc, FE_UPWARD));
#endif
#ifdef FE_DOWNWARD
    s7_define(sc, env, s7_make_symbol(sc, "FE_DOWNWARD"), s7_make_integer(sc, FE_DOWNWARD));
#endif
#ifdef FE_TOWARDZERO
    s7_define(sc, env, s7_make_symbol(sc, "FE_TOWARDZERO"), s7_make_integer(sc, FE_TOWARDZERO));
#endif
#ifdef FNM_PATHNAME
    s7_define(sc, env, s7_make_symbol(sc, "FNM_PATHNAME"), s7_make_integer(sc, FNM_PATHNAME));
#endif
#ifdef FNM_NOESCAPE
    s7_define(sc, env, s7_make_symbol(sc, "FNM_NOESCAPE"), s7_make_integer(sc, FNM_NOESCAPE));
#endif
#ifdef FNM_PERIOD
    s7_define(sc, env, s7_make_symbol(sc, "FNM_PERIOD"), s7_make_integer(sc, FNM_PERIOD));
#endif
#ifdef FNM_FILE_NAME
    s7_define(sc, env, s7_make_symbol(sc, "FNM_FILE_NAME"), s7_make_integer(sc, FNM_FILE_NAME));
#endif
#ifdef FNM_LEADING_DIR
    s7_define(sc, env, s7_make_symbol(sc, "FNM_LEADING_DIR"), s7_make_integer(sc, FNM_LEADING_DIR));
#endif
#ifdef FNM_CASEFOLD
    s7_define(sc, env, s7_make_symbol(sc, "FNM_CASEFOLD"), s7_make_integer(sc, FNM_CASEFOLD));
#endif
#ifdef FNM_EXTMATCH
    s7_define(sc, env, s7_make_symbol(sc, "FNM_EXTMATCH"), s7_make_integer(sc, FNM_EXTMATCH));
#endif
#ifdef FNM_NOMATCH
    s7_define(sc, env, s7_make_symbol(sc, "FNM_NOMATCH"), s7_make_integer(sc, FNM_NOMATCH));
#endif
#ifdef _IOFBF
    s7_define(sc, env, s7_make_symbol(sc, "_IOFBF"), s7_make_integer(sc, _IOFBF));
#endif
#ifdef _IOLBF
    s7_define(sc, env, s7_make_symbol(sc, "_IOLBF"), s7_make_integer(sc, _IOLBF));
#endif
#ifdef _IONBF
    s7_define(sc, env, s7_make_symbol(sc, "_IONBF"), s7_make_integer(sc, _IONBF));
#endif
#ifdef BUFSIZ
    s7_define(sc, env, s7_make_symbol(sc, "BUFSIZ"), s7_make_integer(sc, BUFSIZ));
#endif
#ifdef EOF
    s7_define(sc, env, s7_make_symbol(sc, "EOF"), s7_make_integer(sc, EOF));
#endif
#ifdef L_tmpnam
    s7_define(sc, env, s7_make_symbol(sc, "L_tmpnam"), s7_make_integer(sc, L_tmpnam));
#endif
#ifdef TMP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "TMP_MAX"), s7_make_integer(sc, TMP_MAX));
#endif
#ifdef FILENAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "FILENAME_MAX"), s7_make_integer(sc, FILENAME_MAX));
#endif
#ifdef L_ctermid
    s7_define(sc, env, s7_make_symbol(sc, "L_ctermid"), s7_make_integer(sc, L_ctermid));
#endif
#ifdef L_cuserid
    s7_define(sc, env, s7_make_symbol(sc, "L_cuserid"), s7_make_integer(sc, L_cuserid));
#endif
#ifdef FOPEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "FOPEN_MAX"), s7_make_integer(sc, FOPEN_MAX));
#endif
#ifdef IOV_MAX
    s7_define(sc, env, s7_make_symbol(sc, "IOV_MAX"), s7_make_integer(sc, IOV_MAX));
#endif
#ifdef RAND_MAX
    s7_define(sc, env, s7_make_symbol(sc, "RAND_MAX"), s7_make_integer(sc, RAND_MAX));
#endif
#ifdef EXIT_FAILURE
    s7_define(sc, env, s7_make_symbol(sc, "EXIT_FAILURE"), s7_make_integer(sc, EXIT_FAILURE));
#endif
#ifdef EXIT_SUCCESS
    s7_define(sc, env, s7_make_symbol(sc, "EXIT_SUCCESS"), s7_make_integer(sc, EXIT_SUCCESS));
#endif
#ifdef MB_CUR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "MB_CUR_MAX"), s7_make_integer(sc, MB_CUR_MAX));
#endif
#ifdef __GLIBC__
    s7_define(sc, env, s7_make_symbol(sc, "__GLIBC__"), s7_make_integer(sc, __GLIBC__));
#endif
#ifdef __GLIBC_MINOR__
    s7_define(sc, env, s7_make_symbol(sc, "__GLIBC_MINOR__"), s7_make_integer(sc, __GLIBC_MINOR__));
#endif
#ifdef ECANCELED
    s7_define(sc, env, s7_make_symbol(sc, "ECANCELED"), s7_make_integer(sc, ECANCELED));
#endif
#ifdef EOWNERDEAD
    s7_define(sc, env, s7_make_symbol(sc, "EOWNERDEAD"), s7_make_integer(sc, EOWNERDEAD));
#endif
#ifdef ENOTRECOVERABLE
    s7_define(sc, env, s7_make_symbol(sc, "ENOTRECOVERABLE"), s7_make_integer(sc, ENOTRECOVERABLE));
#endif
#ifdef ERFKILL
    s7_define(sc, env, s7_make_symbol(sc, "ERFKILL"), s7_make_integer(sc, ERFKILL));
#endif
#ifdef EILSEQ
    s7_define(sc, env, s7_make_symbol(sc, "EILSEQ"), s7_make_integer(sc, EILSEQ));
#endif
#ifdef EPERM
    s7_define(sc, env, s7_make_symbol(sc, "EPERM"), s7_make_integer(sc, EPERM));
#endif
#ifdef ENOENT
    s7_define(sc, env, s7_make_symbol(sc, "ENOENT"), s7_make_integer(sc, ENOENT));
#endif
#ifdef ESRCH
    s7_define(sc, env, s7_make_symbol(sc, "ESRCH"), s7_make_integer(sc, ESRCH));
#endif
#ifdef EINTR
    s7_define(sc, env, s7_make_symbol(sc, "EINTR"), s7_make_integer(sc, EINTR));
#endif
#ifdef EIO
    s7_define(sc, env, s7_make_symbol(sc, "EIO"), s7_make_integer(sc, EIO));
#endif
#ifdef ENXIO
    s7_define(sc, env, s7_make_symbol(sc, "ENXIO"), s7_make_integer(sc, ENXIO));
#endif
#ifdef E2BIG
    s7_define(sc, env, s7_make_symbol(sc, "E2BIG"), s7_make_integer(sc, E2BIG));
#endif
#ifdef ENOEXEC
    s7_define(sc, env, s7_make_symbol(sc, "ENOEXEC"), s7_make_integer(sc, ENOEXEC));
#endif
#ifdef EBADF
    s7_define(sc, env, s7_make_symbol(sc, "EBADF"), s7_make_integer(sc, EBADF));
#endif
#ifdef ECHILD
    s7_define(sc, env, s7_make_symbol(sc, "ECHILD"), s7_make_integer(sc, ECHILD));
#endif
#ifdef EAGAIN
    s7_define(sc, env, s7_make_symbol(sc, "EAGAIN"), s7_make_integer(sc, EAGAIN));
#endif
#ifdef ENOMEM
    s7_define(sc, env, s7_make_symbol(sc, "ENOMEM"), s7_make_integer(sc, ENOMEM));
#endif
#ifdef EACCES
    s7_define(sc, env, s7_make_symbol(sc, "EACCES"), s7_make_integer(sc, EACCES));
#endif
#ifdef EFAULT
    s7_define(sc, env, s7_make_symbol(sc, "EFAULT"), s7_make_integer(sc, EFAULT));
#endif
#ifdef ENOTBLK
    s7_define(sc, env, s7_make_symbol(sc, "ENOTBLK"), s7_make_integer(sc, ENOTBLK));
#endif
#ifdef EBUSY
    s7_define(sc, env, s7_make_symbol(sc, "EBUSY"), s7_make_integer(sc, EBUSY));
#endif
#ifdef EEXIST
    s7_define(sc, env, s7_make_symbol(sc, "EEXIST"), s7_make_integer(sc, EEXIST));
#endif
#ifdef EXDEV
    s7_define(sc, env, s7_make_symbol(sc, "EXDEV"), s7_make_integer(sc, EXDEV));
#endif
#ifdef ENODEV
    s7_define(sc, env, s7_make_symbol(sc, "ENODEV"), s7_make_integer(sc, ENODEV));
#endif
#ifdef ENOTDIR
    s7_define(sc, env, s7_make_symbol(sc, "ENOTDIR"), s7_make_integer(sc, ENOTDIR));
#endif
#ifdef EISDIR
    s7_define(sc, env, s7_make_symbol(sc, "EISDIR"), s7_make_integer(sc, EISDIR));
#endif
#ifdef EINVAL
    s7_define(sc, env, s7_make_symbol(sc, "EINVAL"), s7_make_integer(sc, EINVAL));
#endif
#ifdef ENFILE
    s7_define(sc, env, s7_make_symbol(sc, "ENFILE"), s7_make_integer(sc, ENFILE));
#endif
#ifdef EMFILE
    s7_define(sc, env, s7_make_symbol(sc, "EMFILE"), s7_make_integer(sc, EMFILE));
#endif
#ifdef ENOTTY
    s7_define(sc, env, s7_make_symbol(sc, "ENOTTY"), s7_make_integer(sc, ENOTTY));
#endif
#ifdef ETXTBSY
    s7_define(sc, env, s7_make_symbol(sc, "ETXTBSY"), s7_make_integer(sc, ETXTBSY));
#endif
#ifdef EFBIG
    s7_define(sc, env, s7_make_symbol(sc, "EFBIG"), s7_make_integer(sc, EFBIG));
#endif
#ifdef ENOSPC
    s7_define(sc, env, s7_make_symbol(sc, "ENOSPC"), s7_make_integer(sc, ENOSPC));
#endif
#ifdef ESPIPE
    s7_define(sc, env, s7_make_symbol(sc, "ESPIPE"), s7_make_integer(sc, ESPIPE));
#endif
#ifdef EROFS
    s7_define(sc, env, s7_make_symbol(sc, "EROFS"), s7_make_integer(sc, EROFS));
#endif
#ifdef EMLINK
    s7_define(sc, env, s7_make_symbol(sc, "EMLINK"), s7_make_integer(sc, EMLINK));
#endif
#ifdef EPIPE
    s7_define(sc, env, s7_make_symbol(sc, "EPIPE"), s7_make_integer(sc, EPIPE));
#endif
#ifdef EDOM
    s7_define(sc, env, s7_make_symbol(sc, "EDOM"), s7_make_integer(sc, EDOM));
#endif
#ifdef ERANGE
    s7_define(sc, env, s7_make_symbol(sc, "ERANGE"), s7_make_integer(sc, ERANGE));
#endif
#ifdef LC_CTYPE
    s7_define(sc, env, s7_make_symbol(sc, "LC_CTYPE"), s7_make_integer(sc, LC_CTYPE));
#endif
#ifdef LC_NUMERIC
    s7_define(sc, env, s7_make_symbol(sc, "LC_NUMERIC"), s7_make_integer(sc, LC_NUMERIC));
#endif
#ifdef LC_TIME
    s7_define(sc, env, s7_make_symbol(sc, "LC_TIME"), s7_make_integer(sc, LC_TIME));
#endif
#ifdef LC_COLLATE
    s7_define(sc, env, s7_make_symbol(sc, "LC_COLLATE"), s7_make_integer(sc, LC_COLLATE));
#endif
#ifdef LC_MONETARY
    s7_define(sc, env, s7_make_symbol(sc, "LC_MONETARY"), s7_make_integer(sc, LC_MONETARY));
#endif
#ifdef LC_MESSAGES
    s7_define(sc, env, s7_make_symbol(sc, "LC_MESSAGES"), s7_make_integer(sc, LC_MESSAGES));
#endif
#ifdef LC_ALL
    s7_define(sc, env, s7_make_symbol(sc, "LC_ALL"), s7_make_integer(sc, LC_ALL));
#endif
#ifdef LC_PAPER
    s7_define(sc, env, s7_make_symbol(sc, "LC_PAPER"), s7_make_integer(sc, LC_PAPER));
#endif
#ifdef LC_NAME
    s7_define(sc, env, s7_make_symbol(sc, "LC_NAME"), s7_make_integer(sc, LC_NAME));
#endif
#ifdef LC_ADDRESS
    s7_define(sc, env, s7_make_symbol(sc, "LC_ADDRESS"), s7_make_integer(sc, LC_ADDRESS));
#endif
#ifdef LC_TELEPHONE
    s7_define(sc, env, s7_make_symbol(sc, "LC_TELEPHONE"), s7_make_integer(sc, LC_TELEPHONE));
#endif
#ifdef LC_MEASUREMENT
    s7_define(sc, env, s7_make_symbol(sc, "LC_MEASUREMENT"), s7_make_integer(sc, LC_MEASUREMENT));
#endif
#ifdef LC_IDENTIFICATION
    s7_define(sc, env, s7_make_symbol(sc, "LC_IDENTIFICATION"), s7_make_integer(sc, LC_IDENTIFICATION));
#endif
#ifdef _POSIX_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_VERSION"), s7_make_integer(sc, _POSIX_VERSION));
#endif
#ifdef _POSIX2_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_VERSION"), s7_make_integer(sc, _POSIX2_VERSION));
#endif
#ifdef _POSIX_JOB_CONTROL
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_JOB_CONTROL"), s7_make_integer(sc, _POSIX_JOB_CONTROL));
#endif
#ifdef _POSIX_SAVED_IDS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SAVED_IDS"), s7_make_integer(sc, _POSIX_SAVED_IDS));
#endif
#ifdef _POSIX_PRIORITY_SCHEDULING
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_PRIORITY_SCHEDULING"), s7_make_integer(sc, _POSIX_PRIORITY_SCHEDULING));
#endif
#ifdef _POSIX_SYNCHRONIZED_IO
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SYNCHRONIZED_IO"), s7_make_integer(sc, _POSIX_SYNCHRONIZED_IO));
#endif
#ifdef _POSIX_FSYNC
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_FSYNC"), s7_make_integer(sc, _POSIX_FSYNC));
#endif
#ifdef _POSIX_MAPPED_FILES
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MAPPED_FILES"), s7_make_integer(sc, _POSIX_MAPPED_FILES));
#endif
#ifdef _POSIX_MEMLOCK
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MEMLOCK"), s7_make_integer(sc, _POSIX_MEMLOCK));
#endif
#ifdef _POSIX_MEMLOCK_RANGE
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MEMLOCK_RANGE"), s7_make_integer(sc, _POSIX_MEMLOCK_RANGE));
#endif
#ifdef _POSIX_MEMORY_PROTECTION
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MEMORY_PROTECTION"), s7_make_integer(sc, _POSIX_MEMORY_PROTECTION));
#endif
#ifdef _POSIX_CHOWN_RESTRICTED
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_CHOWN_RESTRICTED"), s7_make_integer(sc, _POSIX_CHOWN_RESTRICTED));
#endif
#ifdef _POSIX_VDISABLE
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_VDISABLE"), s7_make_integer(sc, _POSIX_VDISABLE));
#endif
#ifdef _POSIX_NO_TRUNC
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_NO_TRUNC"), s7_make_integer(sc, _POSIX_NO_TRUNC));
#endif
#ifdef _POSIX_THREADS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREADS"), s7_make_integer(sc, _POSIX_THREADS));
#endif
#ifdef _POSIX_REENTRANT_FUNCTIONS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_REENTRANT_FUNCTIONS"), s7_make_integer(sc, _POSIX_REENTRANT_FUNCTIONS));
#endif
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_SAFE_FUNCTIONS"), s7_make_integer(sc, _POSIX_THREAD_SAFE_FUNCTIONS));
#endif
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_PRIORITY_SCHEDULING"), s7_make_integer(sc, _POSIX_THREAD_PRIORITY_SCHEDULING));
#endif
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_ATTR_STACKSIZE"), s7_make_integer(sc, _POSIX_THREAD_ATTR_STACKSIZE));
#endif
#ifdef _POSIX_THREAD_ATTR_STACKADDR
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_ATTR_STACKADDR"), s7_make_integer(sc, _POSIX_THREAD_ATTR_STACKADDR));
#endif
#ifdef _POSIX_THREAD_PRIO_INHERIT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_PRIO_INHERIT"), s7_make_integer(sc, _POSIX_THREAD_PRIO_INHERIT));
#endif
#ifdef _POSIX_THREAD_PRIO_PROTECT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_PRIO_PROTECT"), s7_make_integer(sc, _POSIX_THREAD_PRIO_PROTECT));
#endif
#ifdef _POSIX_SEMAPHORES
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SEMAPHORES"), s7_make_integer(sc, _POSIX_SEMAPHORES));
#endif
#ifdef _POSIX_REALTIME_SIGNALS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_REALTIME_SIGNALS"), s7_make_integer(sc, _POSIX_REALTIME_SIGNALS));
#endif
#ifdef _POSIX_ASYNCHRONOUS_IO
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_ASYNCHRONOUS_IO"), s7_make_integer(sc, _POSIX_ASYNCHRONOUS_IO));
#endif
#ifdef _POSIX_ASYNC_IO
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_ASYNC_IO"), s7_make_integer(sc, _POSIX_ASYNC_IO));
#endif
#ifdef _POSIX_PRIORITIZED_IO
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_PRIORITIZED_IO"), s7_make_integer(sc, _POSIX_PRIORITIZED_IO));
#endif
#ifdef _POSIX_SHARED_MEMORY_OBJECTS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SHARED_MEMORY_OBJECTS"), s7_make_integer(sc, _POSIX_SHARED_MEMORY_OBJECTS));
#endif
#ifdef _POSIX_CPUTIME
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_CPUTIME"), s7_make_integer(sc, _POSIX_CPUTIME));
#endif
#ifdef _POSIX_THREAD_CPUTIME
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_CPUTIME"), s7_make_integer(sc, _POSIX_THREAD_CPUTIME));
#endif
#ifdef _POSIX_REGEXP
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_REGEXP"), s7_make_integer(sc, _POSIX_REGEXP));
#endif
#ifdef _POSIX_READER_WRITER_LOCKS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_READER_WRITER_LOCKS"), s7_make_integer(sc, _POSIX_READER_WRITER_LOCKS));
#endif
#ifdef _POSIX_SHELL
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SHELL"), s7_make_integer(sc, _POSIX_SHELL));
#endif
#ifdef _POSIX_TIMEOUTS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TIMEOUTS"), s7_make_integer(sc, _POSIX_TIMEOUTS));
#endif
#ifdef _POSIX_SPIN_LOCKS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SPIN_LOCKS"), s7_make_integer(sc, _POSIX_SPIN_LOCKS));
#endif
#ifdef _POSIX_SPAWN
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SPAWN"), s7_make_integer(sc, _POSIX_SPAWN));
#endif
#ifdef _POSIX_TIMERS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TIMERS"), s7_make_integer(sc, _POSIX_TIMERS));
#endif
#ifdef _POSIX_BARRIERS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_BARRIERS"), s7_make_integer(sc, _POSIX_BARRIERS));
#endif
#ifdef _POSIX_MESSAGE_PASSING
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MESSAGE_PASSING"), s7_make_integer(sc, _POSIX_MESSAGE_PASSING));
#endif
#ifdef _POSIX_THREAD_PROCESS_SHARED
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_PROCESS_SHARED"), s7_make_integer(sc, _POSIX_THREAD_PROCESS_SHARED));
#endif
#ifdef _POSIX_MONOTONIC_CLOCK
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_MONOTONIC_CLOCK"), s7_make_integer(sc, _POSIX_MONOTONIC_CLOCK));
#endif
#ifdef _POSIX_CLOCK_SELECTION
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_CLOCK_SELECTION"), s7_make_integer(sc, _POSIX_CLOCK_SELECTION));
#endif
#ifdef _POSIX_ADVISORY_INFO
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_ADVISORY_INFO"), s7_make_integer(sc, _POSIX_ADVISORY_INFO));
#endif
#ifdef _POSIX_IPV6
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_IPV6"), s7_make_integer(sc, _POSIX_IPV6));
#endif
#ifdef _POSIX_RAW_SOCKETS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_RAW_SOCKETS"), s7_make_integer(sc, _POSIX_RAW_SOCKETS));
#endif
#ifdef _POSIX2_CHAR_TERM
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX2_CHAR_TERM"), s7_make_integer(sc, _POSIX2_CHAR_TERM));
#endif
#ifdef _POSIX_SPORADIC_SERVER
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_SPORADIC_SERVER"), s7_make_integer(sc, _POSIX_SPORADIC_SERVER));
#endif
#ifdef _POSIX_THREAD_SPORADIC_SERVER
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_THREAD_SPORADIC_SERVER"), s7_make_integer(sc, _POSIX_THREAD_SPORADIC_SERVER));
#endif
#ifdef _POSIX_TRACE
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TRACE"), s7_make_integer(sc, _POSIX_TRACE));
#endif
#ifdef _POSIX_TRACE_EVENT_FILTER
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TRACE_EVENT_FILTER"), s7_make_integer(sc, _POSIX_TRACE_EVENT_FILTER));
#endif
#ifdef _POSIX_TRACE_INHERIT
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TRACE_INHERIT"), s7_make_integer(sc, _POSIX_TRACE_INHERIT));
#endif
#ifdef _POSIX_TRACE_LOG
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TRACE_LOG"), s7_make_integer(sc, _POSIX_TRACE_LOG));
#endif
#ifdef _POSIX_TYPED_MEMORY_OBJECTS
    s7_define(sc, env, s7_make_symbol(sc, "_POSIX_TYPED_MEMORY_OBJECTS"), s7_make_integer(sc, _POSIX_TYPED_MEMORY_OBJECTS));
#endif
#ifdef STDIN_FILENO
    s7_define(sc, env, s7_make_symbol(sc, "STDIN_FILENO"), s7_make_integer(sc, STDIN_FILENO));
#endif
#ifdef STDOUT_FILENO
    s7_define(sc, env, s7_make_symbol(sc, "STDOUT_FILENO"), s7_make_integer(sc, STDOUT_FILENO));
#endif
#ifdef STDERR_FILENO
    s7_define(sc, env, s7_make_symbol(sc, "STDERR_FILENO"), s7_make_integer(sc, STDERR_FILENO));
#endif
#ifdef _PC_LINK_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_PC_LINK_MAX"), s7_make_integer(sc, _PC_LINK_MAX));
#endif
#ifdef _PC_MAX_CANON
    s7_define(sc, env, s7_make_symbol(sc, "_PC_MAX_CANON"), s7_make_integer(sc, _PC_MAX_CANON));
#endif
#ifdef _PC_MAX_INPUT
    s7_define(sc, env, s7_make_symbol(sc, "_PC_MAX_INPUT"), s7_make_integer(sc, _PC_MAX_INPUT));
#endif
#ifdef _PC_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_PC_NAME_MAX"), s7_make_integer(sc, _PC_NAME_MAX));
#endif
#ifdef _PC_PATH_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_PC_PATH_MAX"), s7_make_integer(sc, _PC_PATH_MAX));
#endif
#ifdef _PC_PIPE_BUF
    s7_define(sc, env, s7_make_symbol(sc, "_PC_PIPE_BUF"), s7_make_integer(sc, _PC_PIPE_BUF));
#endif
#ifdef _PC_CHOWN_RESTRICTED
    s7_define(sc, env, s7_make_symbol(sc, "_PC_CHOWN_RESTRICTED"), s7_make_integer(sc, _PC_CHOWN_RESTRICTED));
#endif
#ifdef _PC_NO_TRUNC
    s7_define(sc, env, s7_make_symbol(sc, "_PC_NO_TRUNC"), s7_make_integer(sc, _PC_NO_TRUNC));
#endif
#ifdef _PC_VDISABLE
    s7_define(sc, env, s7_make_symbol(sc, "_PC_VDISABLE"), s7_make_integer(sc, _PC_VDISABLE));
#endif
#ifdef _PC_SYNC_IO
    s7_define(sc, env, s7_make_symbol(sc, "_PC_SYNC_IO"), s7_make_integer(sc, _PC_SYNC_IO));
#endif
#ifdef _PC_ASYNC_IO
    s7_define(sc, env, s7_make_symbol(sc, "_PC_ASYNC_IO"), s7_make_integer(sc, _PC_ASYNC_IO));
#endif
#ifdef _PC_PRIO_IO
    s7_define(sc, env, s7_make_symbol(sc, "_PC_PRIO_IO"), s7_make_integer(sc, _PC_PRIO_IO));
#endif
#ifdef _PC_SOCK_MAXBUF
    s7_define(sc, env, s7_make_symbol(sc, "_PC_SOCK_MAXBUF"), s7_make_integer(sc, _PC_SOCK_MAXBUF));
#endif
#ifdef _PC_FILESIZEBITS
    s7_define(sc, env, s7_make_symbol(sc, "_PC_FILESIZEBITS"), s7_make_integer(sc, _PC_FILESIZEBITS));
#endif
#ifdef _PC_REC_INCR_XFER_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_PC_REC_INCR_XFER_SIZE"), s7_make_integer(sc, _PC_REC_INCR_XFER_SIZE));
#endif
#ifdef _PC_REC_MAX_XFER_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_PC_REC_MAX_XFER_SIZE"), s7_make_integer(sc, _PC_REC_MAX_XFER_SIZE));
#endif
#ifdef _PC_REC_MIN_XFER_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_PC_REC_MIN_XFER_SIZE"), s7_make_integer(sc, _PC_REC_MIN_XFER_SIZE));
#endif
#ifdef _PC_REC_XFER_ALIGN
    s7_define(sc, env, s7_make_symbol(sc, "_PC_REC_XFER_ALIGN"), s7_make_integer(sc, _PC_REC_XFER_ALIGN));
#endif
#ifdef _PC_ALLOC_SIZE_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_PC_ALLOC_SIZE_MIN"), s7_make_integer(sc, _PC_ALLOC_SIZE_MIN));
#endif
#ifdef _PC_SYMLINK_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_PC_SYMLINK_MAX"), s7_make_integer(sc, _PC_SYMLINK_MAX));
#endif
#ifdef _PC_2_SYMLINKS
    s7_define(sc, env, s7_make_symbol(sc, "_PC_2_SYMLINKS"), s7_make_integer(sc, _PC_2_SYMLINKS));
#endif
#ifdef _SC_ARG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_ARG_MAX"), s7_make_integer(sc, _SC_ARG_MAX));
#endif
#ifdef _SC_CHILD_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CHILD_MAX"), s7_make_integer(sc, _SC_CHILD_MAX));
#endif
#ifdef _SC_CLK_TCK
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CLK_TCK"), s7_make_integer(sc, _SC_CLK_TCK));
#endif
#ifdef _SC_NGROUPS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NGROUPS_MAX"), s7_make_integer(sc, _SC_NGROUPS_MAX));
#endif
#ifdef _SC_OPEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_OPEN_MAX"), s7_make_integer(sc, _SC_OPEN_MAX));
#endif
#ifdef _SC_STREAM_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_STREAM_MAX"), s7_make_integer(sc, _SC_STREAM_MAX));
#endif
#ifdef _SC_TZNAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TZNAME_MAX"), s7_make_integer(sc, _SC_TZNAME_MAX));
#endif
#ifdef _SC_JOB_CONTROL
    s7_define(sc, env, s7_make_symbol(sc, "_SC_JOB_CONTROL"), s7_make_integer(sc, _SC_JOB_CONTROL));
#endif
#ifdef _SC_SAVED_IDS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SAVED_IDS"), s7_make_integer(sc, _SC_SAVED_IDS));
#endif
#ifdef _SC_REALTIME_SIGNALS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_REALTIME_SIGNALS"), s7_make_integer(sc, _SC_REALTIME_SIGNALS));
#endif
#ifdef _SC_PRIORITY_SCHEDULING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PRIORITY_SCHEDULING"), s7_make_integer(sc, _SC_PRIORITY_SCHEDULING));
#endif
#ifdef _SC_TIMERS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TIMERS"), s7_make_integer(sc, _SC_TIMERS));
#endif
#ifdef _SC_ASYNCHRONOUS_IO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_ASYNCHRONOUS_IO"), s7_make_integer(sc, _SC_ASYNCHRONOUS_IO));
#endif
#ifdef _SC_PRIORITIZED_IO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PRIORITIZED_IO"), s7_make_integer(sc, _SC_PRIORITIZED_IO));
#endif
#ifdef _SC_SYNCHRONIZED_IO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SYNCHRONIZED_IO"), s7_make_integer(sc, _SC_SYNCHRONIZED_IO));
#endif
#ifdef _SC_FSYNC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FSYNC"), s7_make_integer(sc, _SC_FSYNC));
#endif
#ifdef _SC_MAPPED_FILES
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MAPPED_FILES"), s7_make_integer(sc, _SC_MAPPED_FILES));
#endif
#ifdef _SC_MEMLOCK
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MEMLOCK"), s7_make_integer(sc, _SC_MEMLOCK));
#endif
#ifdef _SC_MEMLOCK_RANGE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MEMLOCK_RANGE"), s7_make_integer(sc, _SC_MEMLOCK_RANGE));
#endif
#ifdef _SC_MEMORY_PROTECTION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MEMORY_PROTECTION"), s7_make_integer(sc, _SC_MEMORY_PROTECTION));
#endif
#ifdef _SC_MESSAGE_PASSING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MESSAGE_PASSING"), s7_make_integer(sc, _SC_MESSAGE_PASSING));
#endif
#ifdef _SC_SEMAPHORES
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SEMAPHORES"), s7_make_integer(sc, _SC_SEMAPHORES));
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SHARED_MEMORY_OBJECTS"), s7_make_integer(sc, _SC_SHARED_MEMORY_OBJECTS));
#endif
#ifdef _SC_AIO_LISTIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_AIO_LISTIO_MAX"), s7_make_integer(sc, _SC_AIO_LISTIO_MAX));
#endif
#ifdef _SC_AIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_AIO_MAX"), s7_make_integer(sc, _SC_AIO_MAX));
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_AIO_PRIO_DELTA_MAX"), s7_make_integer(sc, _SC_AIO_PRIO_DELTA_MAX));
#endif
#ifdef _SC_DELAYTIMER_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_DELAYTIMER_MAX"), s7_make_integer(sc, _SC_DELAYTIMER_MAX));
#endif
#ifdef _SC_MQ_OPEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MQ_OPEN_MAX"), s7_make_integer(sc, _SC_MQ_OPEN_MAX));
#endif
#ifdef _SC_MQ_PRIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MQ_PRIO_MAX"), s7_make_integer(sc, _SC_MQ_PRIO_MAX));
#endif
#ifdef _SC_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_VERSION"), s7_make_integer(sc, _SC_VERSION));
#endif
#ifdef _SC_PAGESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PAGESIZE"), s7_make_integer(sc, _SC_PAGESIZE));
#endif
#ifdef _SC_PAGE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PAGE_SIZE"), s7_make_integer(sc, _SC_PAGE_SIZE));
#endif
#ifdef _SC_RTSIG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_RTSIG_MAX"), s7_make_integer(sc, _SC_RTSIG_MAX));
#endif
#ifdef _SC_SEM_NSEMS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SEM_NSEMS_MAX"), s7_make_integer(sc, _SC_SEM_NSEMS_MAX));
#endif
#ifdef _SC_SEM_VALUE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SEM_VALUE_MAX"), s7_make_integer(sc, _SC_SEM_VALUE_MAX));
#endif
#ifdef _SC_SIGQUEUE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SIGQUEUE_MAX"), s7_make_integer(sc, _SC_SIGQUEUE_MAX));
#endif
#ifdef _SC_TIMER_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TIMER_MAX"), s7_make_integer(sc, _SC_TIMER_MAX));
#endif
#ifdef _SC_BC_BASE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BC_BASE_MAX"), s7_make_integer(sc, _SC_BC_BASE_MAX));
#endif
#ifdef _SC_BC_DIM_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BC_DIM_MAX"), s7_make_integer(sc, _SC_BC_DIM_MAX));
#endif
#ifdef _SC_BC_SCALE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BC_SCALE_MAX"), s7_make_integer(sc, _SC_BC_SCALE_MAX));
#endif
#ifdef _SC_BC_STRING_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BC_STRING_MAX"), s7_make_integer(sc, _SC_BC_STRING_MAX));
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_COLL_WEIGHTS_MAX"), s7_make_integer(sc, _SC_COLL_WEIGHTS_MAX));
#endif
#ifdef _SC_EQUIV_CLASS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_EQUIV_CLASS_MAX"), s7_make_integer(sc, _SC_EQUIV_CLASS_MAX));
#endif
#ifdef _SC_EXPR_NEST_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_EXPR_NEST_MAX"), s7_make_integer(sc, _SC_EXPR_NEST_MAX));
#endif
#ifdef _SC_LINE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LINE_MAX"), s7_make_integer(sc, _SC_LINE_MAX));
#endif
#ifdef _SC_RE_DUP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_RE_DUP_MAX"), s7_make_integer(sc, _SC_RE_DUP_MAX));
#endif
#ifdef _SC_CHARCLASS_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CHARCLASS_NAME_MAX"), s7_make_integer(sc, _SC_CHARCLASS_NAME_MAX));
#endif
#ifdef _SC_2_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_VERSION"), s7_make_integer(sc, _SC_2_VERSION));
#endif
#ifdef _SC_2_C_BIND
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_C_BIND"), s7_make_integer(sc, _SC_2_C_BIND));
#endif
#ifdef _SC_2_C_DEV
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_C_DEV"), s7_make_integer(sc, _SC_2_C_DEV));
#endif
#ifdef _SC_2_FORT_DEV
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_FORT_DEV"), s7_make_integer(sc, _SC_2_FORT_DEV));
#endif
#ifdef _SC_2_FORT_RUN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_FORT_RUN"), s7_make_integer(sc, _SC_2_FORT_RUN));
#endif
#ifdef _SC_2_SW_DEV
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_SW_DEV"), s7_make_integer(sc, _SC_2_SW_DEV));
#endif
#ifdef _SC_2_LOCALEDEF
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_LOCALEDEF"), s7_make_integer(sc, _SC_2_LOCALEDEF));
#endif
#ifdef _SC_PII
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII"), s7_make_integer(sc, _SC_PII));
#endif
#ifdef _SC_PII_XTI
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_XTI"), s7_make_integer(sc, _SC_PII_XTI));
#endif
#ifdef _SC_PII_SOCKET
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_SOCKET"), s7_make_integer(sc, _SC_PII_SOCKET));
#endif
#ifdef _SC_PII_INTERNET
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_INTERNET"), s7_make_integer(sc, _SC_PII_INTERNET));
#endif
#ifdef _SC_PII_OSI
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_OSI"), s7_make_integer(sc, _SC_PII_OSI));
#endif
#ifdef _SC_POLL
    s7_define(sc, env, s7_make_symbol(sc, "_SC_POLL"), s7_make_integer(sc, _SC_POLL));
#endif
#ifdef _SC_SELECT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SELECT"), s7_make_integer(sc, _SC_SELECT));
#endif
#ifdef _SC_UIO_MAXIOV
    s7_define(sc, env, s7_make_symbol(sc, "_SC_UIO_MAXIOV"), s7_make_integer(sc, _SC_UIO_MAXIOV));
#endif
#ifdef _SC_IOV_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_IOV_MAX"), s7_make_integer(sc, _SC_IOV_MAX));
#endif
#ifdef _SC_PII_INTERNET_STREAM
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_INTERNET_STREAM"), s7_make_integer(sc, _SC_PII_INTERNET_STREAM));
#endif
#ifdef _SC_PII_INTERNET_DGRAM
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_INTERNET_DGRAM"), s7_make_integer(sc, _SC_PII_INTERNET_DGRAM));
#endif
#ifdef _SC_PII_OSI_COTS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_OSI_COTS"), s7_make_integer(sc, _SC_PII_OSI_COTS));
#endif
#ifdef _SC_PII_OSI_CLTS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_OSI_CLTS"), s7_make_integer(sc, _SC_PII_OSI_CLTS));
#endif
#ifdef _SC_PII_OSI_M
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PII_OSI_M"), s7_make_integer(sc, _SC_PII_OSI_M));
#endif
#ifdef _SC_T_IOV_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_T_IOV_MAX"), s7_make_integer(sc, _SC_T_IOV_MAX));
#endif
#ifdef _SC_THREADS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREADS"), s7_make_integer(sc, _SC_THREADS));
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_SAFE_FUNCTIONS"), s7_make_integer(sc, _SC_THREAD_SAFE_FUNCTIONS));
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_GETGR_R_SIZE_MAX"), s7_make_integer(sc, _SC_GETGR_R_SIZE_MAX));
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_GETPW_R_SIZE_MAX"), s7_make_integer(sc, _SC_GETPW_R_SIZE_MAX));
#endif
#ifdef _SC_LOGIN_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LOGIN_NAME_MAX"), s7_make_integer(sc, _SC_LOGIN_NAME_MAX));
#endif
#ifdef _SC_TTY_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TTY_NAME_MAX"), s7_make_integer(sc, _SC_TTY_NAME_MAX));
#endif
#ifdef _SC_THREAD_DESTRUCTOR_ITERATIONS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_DESTRUCTOR_ITERATIONS"), s7_make_integer(sc, _SC_THREAD_DESTRUCTOR_ITERATIONS));
#endif
#ifdef _SC_THREAD_KEYS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_KEYS_MAX"), s7_make_integer(sc, _SC_THREAD_KEYS_MAX));
#endif
#ifdef _SC_THREAD_STACK_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_STACK_MIN"), s7_make_integer(sc, _SC_THREAD_STACK_MIN));
#endif
#ifdef _SC_THREAD_THREADS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_THREADS_MAX"), s7_make_integer(sc, _SC_THREAD_THREADS_MAX));
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_ATTR_STACKADDR"), s7_make_integer(sc, _SC_THREAD_ATTR_STACKADDR));
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_ATTR_STACKSIZE"), s7_make_integer(sc, _SC_THREAD_ATTR_STACKSIZE));
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_PRIO_INHERIT"), s7_make_integer(sc, _SC_THREAD_PRIO_INHERIT));
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_PRIO_PROTECT"), s7_make_integer(sc, _SC_THREAD_PRIO_PROTECT));
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_PROCESS_SHARED"), s7_make_integer(sc, _SC_THREAD_PROCESS_SHARED));
#endif
#ifdef _SC_NPROCESSORS_CONF
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NPROCESSORS_CONF"), s7_make_integer(sc, _SC_NPROCESSORS_CONF));
#endif
#ifdef _SC_NPROCESSORS_ONLN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NPROCESSORS_ONLN"), s7_make_integer(sc, _SC_NPROCESSORS_ONLN));
#endif
#ifdef _SC_PHYS_PAGES
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PHYS_PAGES"), s7_make_integer(sc, _SC_PHYS_PAGES));
#endif
#ifdef _SC_AVPHYS_PAGES
    s7_define(sc, env, s7_make_symbol(sc, "_SC_AVPHYS_PAGES"), s7_make_integer(sc, _SC_AVPHYS_PAGES));
#endif
#ifdef _SC_ATEXIT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_ATEXIT_MAX"), s7_make_integer(sc, _SC_ATEXIT_MAX));
#endif
#ifdef _SC_PASS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PASS_MAX"), s7_make_integer(sc, _SC_PASS_MAX));
#endif
#ifdef _SC_2_CHAR_TERM
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_CHAR_TERM"), s7_make_integer(sc, _SC_2_CHAR_TERM));
#endif
#ifdef _SC_2_C_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_C_VERSION"), s7_make_integer(sc, _SC_2_C_VERSION));
#endif
#ifdef _SC_2_UPE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_UPE"), s7_make_integer(sc, _SC_2_UPE));
#endif
#ifdef _SC_CHAR_BIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CHAR_BIT"), s7_make_integer(sc, _SC_CHAR_BIT));
#endif
#ifdef _SC_CHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CHAR_MAX"), s7_make_integer(sc, _SC_CHAR_MAX));
#endif
#ifdef _SC_CHAR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CHAR_MIN"), s7_make_integer(sc, _SC_CHAR_MIN));
#endif
#ifdef _SC_INT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_INT_MAX"), s7_make_integer(sc, _SC_INT_MAX));
#endif
#ifdef _SC_INT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_INT_MIN"), s7_make_integer(sc, _SC_INT_MIN));
#endif
#ifdef _SC_LONG_BIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LONG_BIT"), s7_make_integer(sc, _SC_LONG_BIT));
#endif
#ifdef _SC_WORD_BIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_WORD_BIT"), s7_make_integer(sc, _SC_WORD_BIT));
#endif
#ifdef _SC_MB_LEN_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MB_LEN_MAX"), s7_make_integer(sc, _SC_MB_LEN_MAX));
#endif
#ifdef _SC_NZERO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NZERO"), s7_make_integer(sc, _SC_NZERO));
#endif
#ifdef _SC_SSIZE_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SSIZE_MAX"), s7_make_integer(sc, _SC_SSIZE_MAX));
#endif
#ifdef _SC_SCHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SCHAR_MAX"), s7_make_integer(sc, _SC_SCHAR_MAX));
#endif
#ifdef _SC_SCHAR_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SCHAR_MIN"), s7_make_integer(sc, _SC_SCHAR_MIN));
#endif
#ifdef _SC_SHRT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SHRT_MAX"), s7_make_integer(sc, _SC_SHRT_MAX));
#endif
#ifdef _SC_SHRT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SHRT_MIN"), s7_make_integer(sc, _SC_SHRT_MIN));
#endif
#ifdef _SC_UCHAR_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_UCHAR_MAX"), s7_make_integer(sc, _SC_UCHAR_MAX));
#endif
#ifdef _SC_UINT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_UINT_MAX"), s7_make_integer(sc, _SC_UINT_MAX));
#endif
#ifdef _SC_ULONG_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_ULONG_MAX"), s7_make_integer(sc, _SC_ULONG_MAX));
#endif
#ifdef _SC_USHRT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_USHRT_MAX"), s7_make_integer(sc, _SC_USHRT_MAX));
#endif
#ifdef _SC_NL_ARGMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_ARGMAX"), s7_make_integer(sc, _SC_NL_ARGMAX));
#endif
#ifdef _SC_NL_LANGMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_LANGMAX"), s7_make_integer(sc, _SC_NL_LANGMAX));
#endif
#ifdef _SC_NL_MSGMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_MSGMAX"), s7_make_integer(sc, _SC_NL_MSGMAX));
#endif
#ifdef _SC_NL_NMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_NMAX"), s7_make_integer(sc, _SC_NL_NMAX));
#endif
#ifdef _SC_NL_SETMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_SETMAX"), s7_make_integer(sc, _SC_NL_SETMAX));
#endif
#ifdef _SC_NL_TEXTMAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NL_TEXTMAX"), s7_make_integer(sc, _SC_NL_TEXTMAX));
#endif
#ifdef _SC_ADVISORY_INFO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_ADVISORY_INFO"), s7_make_integer(sc, _SC_ADVISORY_INFO));
#endif
#ifdef _SC_BARRIERS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BARRIERS"), s7_make_integer(sc, _SC_BARRIERS));
#endif
#ifdef _SC_BASE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_BASE"), s7_make_integer(sc, _SC_BASE));
#endif
#ifdef _SC_C_LANG_SUPPORT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_C_LANG_SUPPORT"), s7_make_integer(sc, _SC_C_LANG_SUPPORT));
#endif
#ifdef _SC_C_LANG_SUPPORT_R
    s7_define(sc, env, s7_make_symbol(sc, "_SC_C_LANG_SUPPORT_R"), s7_make_integer(sc, _SC_C_LANG_SUPPORT_R));
#endif
#ifdef _SC_CLOCK_SELECTION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CLOCK_SELECTION"), s7_make_integer(sc, _SC_CLOCK_SELECTION));
#endif
#ifdef _SC_CPUTIME
    s7_define(sc, env, s7_make_symbol(sc, "_SC_CPUTIME"), s7_make_integer(sc, _SC_CPUTIME));
#endif
#ifdef _SC_THREAD_CPUTIME
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_CPUTIME"), s7_make_integer(sc, _SC_THREAD_CPUTIME));
#endif
#ifdef _SC_DEVICE_IO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_DEVICE_IO"), s7_make_integer(sc, _SC_DEVICE_IO));
#endif
#ifdef _SC_DEVICE_SPECIFIC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_DEVICE_SPECIFIC"), s7_make_integer(sc, _SC_DEVICE_SPECIFIC));
#endif
#ifdef _SC_DEVICE_SPECIFIC_R
    s7_define(sc, env, s7_make_symbol(sc, "_SC_DEVICE_SPECIFIC_R"), s7_make_integer(sc, _SC_DEVICE_SPECIFIC_R));
#endif
#ifdef _SC_FD_MGMT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FD_MGMT"), s7_make_integer(sc, _SC_FD_MGMT));
#endif
#ifdef _SC_FIFO
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FIFO"), s7_make_integer(sc, _SC_FIFO));
#endif
#ifdef _SC_PIPE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_PIPE"), s7_make_integer(sc, _SC_PIPE));
#endif
#ifdef _SC_FILE_ATTRIBUTES
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FILE_ATTRIBUTES"), s7_make_integer(sc, _SC_FILE_ATTRIBUTES));
#endif
#ifdef _SC_FILE_LOCKING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FILE_LOCKING"), s7_make_integer(sc, _SC_FILE_LOCKING));
#endif
#ifdef _SC_FILE_SYSTEM
    s7_define(sc, env, s7_make_symbol(sc, "_SC_FILE_SYSTEM"), s7_make_integer(sc, _SC_FILE_SYSTEM));
#endif
#ifdef _SC_MONOTONIC_CLOCK
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MONOTONIC_CLOCK"), s7_make_integer(sc, _SC_MONOTONIC_CLOCK));
#endif
#ifdef _SC_MULTI_PROCESS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_MULTI_PROCESS"), s7_make_integer(sc, _SC_MULTI_PROCESS));
#endif
#ifdef _SC_SINGLE_PROCESS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SINGLE_PROCESS"), s7_make_integer(sc, _SC_SINGLE_PROCESS));
#endif
#ifdef _SC_NETWORKING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_NETWORKING"), s7_make_integer(sc, _SC_NETWORKING));
#endif
#ifdef _SC_READER_WRITER_LOCKS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_READER_WRITER_LOCKS"), s7_make_integer(sc, _SC_READER_WRITER_LOCKS));
#endif
#ifdef _SC_SPIN_LOCKS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SPIN_LOCKS"), s7_make_integer(sc, _SC_SPIN_LOCKS));
#endif
#ifdef _SC_REGEXP
    s7_define(sc, env, s7_make_symbol(sc, "_SC_REGEXP"), s7_make_integer(sc, _SC_REGEXP));
#endif
#ifdef _SC_REGEX_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_SC_REGEX_VERSION"), s7_make_integer(sc, _SC_REGEX_VERSION));
#endif
#ifdef _SC_SHELL
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SHELL"), s7_make_integer(sc, _SC_SHELL));
#endif
#ifdef _SC_SIGNALS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SIGNALS"), s7_make_integer(sc, _SC_SIGNALS));
#endif
#ifdef _SC_SPAWN
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SPAWN"), s7_make_integer(sc, _SC_SPAWN));
#endif
#ifdef _SC_SPORADIC_SERVER
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SPORADIC_SERVER"), s7_make_integer(sc, _SC_SPORADIC_SERVER));
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_SPORADIC_SERVER"), s7_make_integer(sc, _SC_THREAD_SPORADIC_SERVER));
#endif
#ifdef _SC_SYSTEM_DATABASE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SYSTEM_DATABASE"), s7_make_integer(sc, _SC_SYSTEM_DATABASE));
#endif
#ifdef _SC_SYSTEM_DATABASE_R
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SYSTEM_DATABASE_R"), s7_make_integer(sc, _SC_SYSTEM_DATABASE_R));
#endif
#ifdef _SC_TIMEOUTS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TIMEOUTS"), s7_make_integer(sc, _SC_TIMEOUTS));
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TYPED_MEMORY_OBJECTS"), s7_make_integer(sc, _SC_TYPED_MEMORY_OBJECTS));
#endif
#ifdef _SC_USER_GROUPS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_USER_GROUPS"), s7_make_integer(sc, _SC_USER_GROUPS));
#endif
#ifdef _SC_USER_GROUPS_R
    s7_define(sc, env, s7_make_symbol(sc, "_SC_USER_GROUPS_R"), s7_make_integer(sc, _SC_USER_GROUPS_R));
#endif
#ifdef _SC_2_PBS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS"), s7_make_integer(sc, _SC_2_PBS));
#endif
#ifdef _SC_2_PBS_ACCOUNTING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS_ACCOUNTING"), s7_make_integer(sc, _SC_2_PBS_ACCOUNTING));
#endif
#ifdef _SC_2_PBS_LOCATE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS_LOCATE"), s7_make_integer(sc, _SC_2_PBS_LOCATE));
#endif
#ifdef _SC_2_PBS_MESSAGE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS_MESSAGE"), s7_make_integer(sc, _SC_2_PBS_MESSAGE));
#endif
#ifdef _SC_2_PBS_TRACK
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS_TRACK"), s7_make_integer(sc, _SC_2_PBS_TRACK));
#endif
#ifdef _SC_SYMLOOP_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SYMLOOP_MAX"), s7_make_integer(sc, _SC_SYMLOOP_MAX));
#endif
#ifdef _SC_STREAMS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_STREAMS"), s7_make_integer(sc, _SC_STREAMS));
#endif
#ifdef _SC_2_PBS_CHECKPOINT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_2_PBS_CHECKPOINT"), s7_make_integer(sc, _SC_2_PBS_CHECKPOINT));
#endif
#ifdef _SC_HOST_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_HOST_NAME_MAX"), s7_make_integer(sc, _SC_HOST_NAME_MAX));
#endif
#ifdef _SC_TRACE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE"), s7_make_integer(sc, _SC_TRACE));
#endif
#ifdef _SC_TRACE_EVENT_FILTER
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_EVENT_FILTER"), s7_make_integer(sc, _SC_TRACE_EVENT_FILTER));
#endif
#ifdef _SC_TRACE_INHERIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_INHERIT"), s7_make_integer(sc, _SC_TRACE_INHERIT));
#endif
#ifdef _SC_TRACE_LOG
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_LOG"), s7_make_integer(sc, _SC_TRACE_LOG));
#endif
#ifdef _SC_LEVEL1_ICACHE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_ICACHE_SIZE"), s7_make_integer(sc, _SC_LEVEL1_ICACHE_SIZE));
#endif
#ifdef _SC_LEVEL1_ICACHE_ASSOC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_ICACHE_ASSOC"), s7_make_integer(sc, _SC_LEVEL1_ICACHE_ASSOC));
#endif
#ifdef _SC_LEVEL1_ICACHE_LINESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_ICACHE_LINESIZE"), s7_make_integer(sc, _SC_LEVEL1_ICACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL1_DCACHE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_DCACHE_SIZE"), s7_make_integer(sc, _SC_LEVEL1_DCACHE_SIZE));
#endif
#ifdef _SC_LEVEL1_DCACHE_ASSOC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_DCACHE_ASSOC"), s7_make_integer(sc, _SC_LEVEL1_DCACHE_ASSOC));
#endif
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL1_DCACHE_LINESIZE"), s7_make_integer(sc, _SC_LEVEL1_DCACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL2_CACHE_SIZE"), s7_make_integer(sc, _SC_LEVEL2_CACHE_SIZE));
#endif
#ifdef _SC_LEVEL2_CACHE_LINESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL2_CACHE_LINESIZE"), s7_make_integer(sc, _SC_LEVEL2_CACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL3_CACHE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL3_CACHE_SIZE"), s7_make_integer(sc, _SC_LEVEL3_CACHE_SIZE));
#endif
#ifdef _SC_LEVEL3_CACHE_ASSOC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL3_CACHE_ASSOC"), s7_make_integer(sc, _SC_LEVEL3_CACHE_ASSOC));
#endif
#ifdef _SC_LEVEL3_CACHE_LINESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL3_CACHE_LINESIZE"), s7_make_integer(sc, _SC_LEVEL3_CACHE_LINESIZE));
#endif
#ifdef _SC_LEVEL4_CACHE_SIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL4_CACHE_SIZE"), s7_make_integer(sc, _SC_LEVEL4_CACHE_SIZE));
#endif
#ifdef _SC_LEVEL4_CACHE_LINESIZE
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL4_CACHE_LINESIZE"), s7_make_integer(sc, _SC_LEVEL4_CACHE_LINESIZE));
#endif
#ifdef _SC_IPV6
    s7_define(sc, env, s7_make_symbol(sc, "_SC_IPV6"), s7_make_integer(sc, _SC_IPV6));
#endif
#ifdef _SC_RAW_SOCKETS
    s7_define(sc, env, s7_make_symbol(sc, "_SC_RAW_SOCKETS"), s7_make_integer(sc, _SC_RAW_SOCKETS));
#endif
#ifdef _SC_SS_REPL_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_SS_REPL_MAX"), s7_make_integer(sc, _SC_SS_REPL_MAX));
#endif
#ifdef _SC_TRACE_EVENT_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_EVENT_NAME_MAX"), s7_make_integer(sc, _SC_TRACE_EVENT_NAME_MAX));
#endif
#ifdef _SC_TRACE_NAME_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_NAME_MAX"), s7_make_integer(sc, _SC_TRACE_NAME_MAX));
#endif
#ifdef _SC_TRACE_SYS_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_SYS_MAX"), s7_make_integer(sc, _SC_TRACE_SYS_MAX));
#endif
#ifdef _SC_TRACE_USER_EVENT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "_SC_TRACE_USER_EVENT_MAX"), s7_make_integer(sc, _SC_TRACE_USER_EVENT_MAX));
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_INHERIT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_ROBUST_PRIO_INHERIT"), s7_make_integer(sc, _SC_THREAD_ROBUST_PRIO_INHERIT));
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_PROTECT
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_ROBUST_PRIO_PROTECT"), s7_make_integer(sc, _SC_THREAD_ROBUST_PRIO_PROTECT));
#endif
#ifdef _CS_PATH
    s7_define(sc, env, s7_make_symbol(sc, "_CS_PATH"), s7_make_integer(sc, _CS_PATH));
#endif
#ifdef _CS_GNU_LIBC_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_CS_GNU_LIBC_VERSION"), s7_make_integer(sc, _CS_GNU_LIBC_VERSION));
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
    s7_define(sc, env, s7_make_symbol(sc, "_SC_THREAD_PRIORITY_SCHEDULING"), s7_make_integer(sc, _SC_THREAD_PRIORITY_SCHEDULING));
#endif
#ifdef _SC_LEVEL2_CACHE_ASSOC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL2_CACHE_ASSOC"), s7_make_integer(sc, _SC_LEVEL2_CACHE_ASSOC));
#endif
#ifdef _SC_LEVEL4_CACHE_ASSOC
    s7_define(sc, env, s7_make_symbol(sc, "_SC_LEVEL4_CACHE_ASSOC"), s7_make_integer(sc, _SC_LEVEL4_CACHE_ASSOC));
#endif
#ifdef _CS_GNU_LIBPTHREAD_VERSION
    s7_define(sc, env, s7_make_symbol(sc, "_CS_GNU_LIBPTHREAD_VERSION"), s7_make_integer(sc, _CS_GNU_LIBPTHREAD_VERSION));
#endif
#ifdef FTW_F
    s7_define(sc, env, s7_make_symbol(sc, "FTW_F"), s7_make_integer(sc, FTW_F));
#endif
#ifdef FTW_D
    s7_define(sc, env, s7_make_symbol(sc, "FTW_D"), s7_make_integer(sc, FTW_D));
#endif
#ifdef FTW_DNR
    s7_define(sc, env, s7_make_symbol(sc, "FTW_DNR"), s7_make_integer(sc, FTW_DNR));
#endif
#ifdef FTW_NS
    s7_define(sc, env, s7_make_symbol(sc, "FTW_NS"), s7_make_integer(sc, FTW_NS));
#endif
#ifdef S_IFLNK
    s7_define(sc, env, s7_make_symbol(sc, "S_IFLNK"), s7_make_integer(sc, S_IFLNK));
#endif
#ifdef CLOCKS_PER_SEC
    s7_define(sc, env, s7_make_symbol(sc, "CLOCKS_PER_SEC"), s7_make_integer(sc, CLOCKS_PER_SEC));
#endif
#ifdef CLOCK_REALTIME
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_REALTIME"), s7_make_integer(sc, CLOCK_REALTIME));
#endif
#ifdef CLOCK_MONOTONIC
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_MONOTONIC"), s7_make_integer(sc, CLOCK_MONOTONIC));
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_PROCESS_CPUTIME_ID"), s7_make_integer(sc, CLOCK_PROCESS_CPUTIME_ID));
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_THREAD_CPUTIME_ID"), s7_make_integer(sc, CLOCK_THREAD_CPUTIME_ID));
#endif
#ifdef CLOCK_MONOTONIC_RAW
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_MONOTONIC_RAW"), s7_make_integer(sc, CLOCK_MONOTONIC_RAW));
#endif
#ifdef CLOCK_REALTIME_COARSE
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_REALTIME_COARSE"), s7_make_integer(sc, CLOCK_REALTIME_COARSE));
#endif
#ifdef CLOCK_MONOTONIC_COARSE
    s7_define(sc, env, s7_make_symbol(sc, "CLOCK_MONOTONIC_COARSE"), s7_make_integer(sc, CLOCK_MONOTONIC_COARSE));
#endif
#ifdef VINTR
    s7_define(sc, env, s7_make_symbol(sc, "VINTR"), s7_make_integer(sc, VINTR));
#endif
#ifdef VQUIT
    s7_define(sc, env, s7_make_symbol(sc, "VQUIT"), s7_make_integer(sc, VQUIT));
#endif
#ifdef VERASE
    s7_define(sc, env, s7_make_symbol(sc, "VERASE"), s7_make_integer(sc, VERASE));
#endif
#ifdef VKILL
    s7_define(sc, env, s7_make_symbol(sc, "VKILL"), s7_make_integer(sc, VKILL));
#endif
#ifdef VEOF
    s7_define(sc, env, s7_make_symbol(sc, "VEOF"), s7_make_integer(sc, VEOF));
#endif
#ifdef VTIME
    s7_define(sc, env, s7_make_symbol(sc, "VTIME"), s7_make_integer(sc, VTIME));
#endif
#ifdef VMIN
    s7_define(sc, env, s7_make_symbol(sc, "VMIN"), s7_make_integer(sc, VMIN));
#endif
#ifdef VSWTC
    s7_define(sc, env, s7_make_symbol(sc, "VSWTC"), s7_make_integer(sc, VSWTC));
#endif
#ifdef VSTART
    s7_define(sc, env, s7_make_symbol(sc, "VSTART"), s7_make_integer(sc, VSTART));
#endif
#ifdef VSTOP
    s7_define(sc, env, s7_make_symbol(sc, "VSTOP"), s7_make_integer(sc, VSTOP));
#endif
#ifdef VSUSP
    s7_define(sc, env, s7_make_symbol(sc, "VSUSP"), s7_make_integer(sc, VSUSP));
#endif
#ifdef VEOL
    s7_define(sc, env, s7_make_symbol(sc, "VEOL"), s7_make_integer(sc, VEOL));
#endif
#ifdef VREPRINT
    s7_define(sc, env, s7_make_symbol(sc, "VREPRINT"), s7_make_integer(sc, VREPRINT));
#endif
#ifdef VDISCARD
    s7_define(sc, env, s7_make_symbol(sc, "VDISCARD"), s7_make_integer(sc, VDISCARD));
#endif
#ifdef VWERASE
    s7_define(sc, env, s7_make_symbol(sc, "VWERASE"), s7_make_integer(sc, VWERASE));
#endif
#ifdef VLNEXT
    s7_define(sc, env, s7_make_symbol(sc, "VLNEXT"), s7_make_integer(sc, VLNEXT));
#endif
#ifdef VEOL2
    s7_define(sc, env, s7_make_symbol(sc, "VEOL2"), s7_make_integer(sc, VEOL2));
#endif
#ifdef IGNBRK
    s7_define(sc, env, s7_make_symbol(sc, "IGNBRK"), s7_make_integer(sc, IGNBRK));
#endif
#ifdef BRKINT
    s7_define(sc, env, s7_make_symbol(sc, "BRKINT"), s7_make_integer(sc, BRKINT));
#endif
#ifdef IGNPAR
    s7_define(sc, env, s7_make_symbol(sc, "IGNPAR"), s7_make_integer(sc, IGNPAR));
#endif
#ifdef PARMRK
    s7_define(sc, env, s7_make_symbol(sc, "PARMRK"), s7_make_integer(sc, PARMRK));
#endif
#ifdef INPCK
    s7_define(sc, env, s7_make_symbol(sc, "INPCK"), s7_make_integer(sc, INPCK));
#endif
#ifdef ISTRIP
    s7_define(sc, env, s7_make_symbol(sc, "ISTRIP"), s7_make_integer(sc, ISTRIP));
#endif
#ifdef INLCR
    s7_define(sc, env, s7_make_symbol(sc, "INLCR"), s7_make_integer(sc, INLCR));
#endif
#ifdef IGNCR
    s7_define(sc, env, s7_make_symbol(sc, "IGNCR"), s7_make_integer(sc, IGNCR));
#endif
#ifdef ICRNL
    s7_define(sc, env, s7_make_symbol(sc, "ICRNL"), s7_make_integer(sc, ICRNL));
#endif
#ifdef IUCLC
    s7_define(sc, env, s7_make_symbol(sc, "IUCLC"), s7_make_integer(sc, IUCLC));
#endif
#ifdef IXON
    s7_define(sc, env, s7_make_symbol(sc, "IXON"), s7_make_integer(sc, IXON));
#endif
#ifdef IXANY
    s7_define(sc, env, s7_make_symbol(sc, "IXANY"), s7_make_integer(sc, IXANY));
#endif
#ifdef IXOFF
    s7_define(sc, env, s7_make_symbol(sc, "IXOFF"), s7_make_integer(sc, IXOFF));
#endif
#ifdef IMAXBEL
    s7_define(sc, env, s7_make_symbol(sc, "IMAXBEL"), s7_make_integer(sc, IMAXBEL));
#endif
#ifdef IUTF8
    s7_define(sc, env, s7_make_symbol(sc, "IUTF8"), s7_make_integer(sc, IUTF8));
#endif
#ifdef OPOST
    s7_define(sc, env, s7_make_symbol(sc, "OPOST"), s7_make_integer(sc, OPOST));
#endif
#ifdef OLCUC
    s7_define(sc, env, s7_make_symbol(sc, "OLCUC"), s7_make_integer(sc, OLCUC));
#endif
#ifdef ONLCR
    s7_define(sc, env, s7_make_symbol(sc, "ONLCR"), s7_make_integer(sc, ONLCR));
#endif
#ifdef OCRNL
    s7_define(sc, env, s7_make_symbol(sc, "OCRNL"), s7_make_integer(sc, OCRNL));
#endif
#ifdef ONOCR
    s7_define(sc, env, s7_make_symbol(sc, "ONOCR"), s7_make_integer(sc, ONOCR));
#endif
#ifdef ONLRET
    s7_define(sc, env, s7_make_symbol(sc, "ONLRET"), s7_make_integer(sc, ONLRET));
#endif
#ifdef OFILL
    s7_define(sc, env, s7_make_symbol(sc, "OFILL"), s7_make_integer(sc, OFILL));
#endif
#ifdef OFDEL
    s7_define(sc, env, s7_make_symbol(sc, "OFDEL"), s7_make_integer(sc, OFDEL));
#endif
#ifdef ISIG
    s7_define(sc, env, s7_make_symbol(sc, "ISIG"), s7_make_integer(sc, ISIG));
#endif
#ifdef ICANON
    s7_define(sc, env, s7_make_symbol(sc, "ICANON"), s7_make_integer(sc, ICANON));
#endif
#ifdef ECHO
    s7_define(sc, env, s7_make_symbol(sc, "ECHO"), s7_make_integer(sc, ECHO));
#endif
#ifdef ECHOE
    s7_define(sc, env, s7_make_symbol(sc, "ECHOE"), s7_make_integer(sc, ECHOE));
#endif
#ifdef ECHOK
    s7_define(sc, env, s7_make_symbol(sc, "ECHOK"), s7_make_integer(sc, ECHOK));
#endif
#ifdef ECHONL
    s7_define(sc, env, s7_make_symbol(sc, "ECHONL"), s7_make_integer(sc, ECHONL));
#endif
#ifdef NOFLSH
    s7_define(sc, env, s7_make_symbol(sc, "NOFLSH"), s7_make_integer(sc, NOFLSH));
#endif
#ifdef TOSTOP
    s7_define(sc, env, s7_make_symbol(sc, "TOSTOP"), s7_make_integer(sc, TOSTOP));
#endif
#ifdef IEXTEN
    s7_define(sc, env, s7_make_symbol(sc, "IEXTEN"), s7_make_integer(sc, IEXTEN));
#endif
#ifdef TCOOFF
    s7_define(sc, env, s7_make_symbol(sc, "TCOOFF"), s7_make_integer(sc, TCOOFF));
#endif
#ifdef TCOON
    s7_define(sc, env, s7_make_symbol(sc, "TCOON"), s7_make_integer(sc, TCOON));
#endif
#ifdef TCIOFF
    s7_define(sc, env, s7_make_symbol(sc, "TCIOFF"), s7_make_integer(sc, TCIOFF));
#endif
#ifdef TCION
    s7_define(sc, env, s7_make_symbol(sc, "TCION"), s7_make_integer(sc, TCION));
#endif
#ifdef TCIFLUSH
    s7_define(sc, env, s7_make_symbol(sc, "TCIFLUSH"), s7_make_integer(sc, TCIFLUSH));
#endif
#ifdef TCOFLUSH
    s7_define(sc, env, s7_make_symbol(sc, "TCOFLUSH"), s7_make_integer(sc, TCOFLUSH));
#endif
#ifdef TCIOFLUSH
    s7_define(sc, env, s7_make_symbol(sc, "TCIOFLUSH"), s7_make_integer(sc, TCIOFLUSH));
#endif
#ifdef TCSANOW
    s7_define(sc, env, s7_make_symbol(sc, "TCSANOW"), s7_make_integer(sc, TCSANOW));
#endif
#ifdef TCSADRAIN
    s7_define(sc, env, s7_make_symbol(sc, "TCSADRAIN"), s7_make_integer(sc, TCSADRAIN));
#endif
#ifdef TCSAFLUSH
    s7_define(sc, env, s7_make_symbol(sc, "TCSAFLUSH"), s7_make_integer(sc, TCSAFLUSH));
#endif
#ifdef NSS_BUFLEN_PASSWD
    s7_define(sc, env, s7_make_symbol(sc, "NSS_BUFLEN_PASSWD"), s7_make_integer(sc, NSS_BUFLEN_PASSWD));
#endif
#ifdef GLOB_ERR
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_ERR"), s7_make_integer(sc, GLOB_ERR));
#endif
#ifdef GLOB_MARK
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_MARK"), s7_make_integer(sc, GLOB_MARK));
#endif
#ifdef GLOB_NOSORT
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOSORT"), s7_make_integer(sc, GLOB_NOSORT));
#endif
#ifdef GLOB_DOOFFS
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_DOOFFS"), s7_make_integer(sc, GLOB_DOOFFS));
#endif
#ifdef GLOB_NOCHECK
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOCHECK"), s7_make_integer(sc, GLOB_NOCHECK));
#endif
#ifdef GLOB_APPEND
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_APPEND"), s7_make_integer(sc, GLOB_APPEND));
#endif
#ifdef GLOB_NOESCAPE
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOESCAPE"), s7_make_integer(sc, GLOB_NOESCAPE));
#endif
#ifdef GLOB_PERIOD
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_PERIOD"), s7_make_integer(sc, GLOB_PERIOD));
#endif
#ifdef GLOB_MAGCHAR
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_MAGCHAR"), s7_make_integer(sc, GLOB_MAGCHAR));
#endif
#ifdef GLOB_ALTDIRFUNC
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_ALTDIRFUNC"), s7_make_integer(sc, GLOB_ALTDIRFUNC));
#endif
#ifdef GLOB_BRACE
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_BRACE"), s7_make_integer(sc, GLOB_BRACE));
#endif
#ifdef GLOB_NOMAGIC
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOMAGIC"), s7_make_integer(sc, GLOB_NOMAGIC));
#endif
#ifdef GLOB_TILDE
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_TILDE"), s7_make_integer(sc, GLOB_TILDE));
#endif
#ifdef GLOB_ONLYDIR
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_ONLYDIR"), s7_make_integer(sc, GLOB_ONLYDIR));
#endif
#ifdef GLOB_TILDE_CHECK
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_TILDE_CHECK"), s7_make_integer(sc, GLOB_TILDE_CHECK));
#endif
#ifdef GLOB_NOSPACE
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOSPACE"), s7_make_integer(sc, GLOB_NOSPACE));
#endif
#ifdef GLOB_ABORTED
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_ABORTED"), s7_make_integer(sc, GLOB_ABORTED));
#endif
#ifdef GLOB_NOMATCH
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOMATCH"), s7_make_integer(sc, GLOB_NOMATCH));
#endif
#ifdef GLOB_NOSYS
    s7_define(sc, env, s7_make_symbol(sc, "GLOB_NOSYS"), s7_make_integer(sc, GLOB_NOSYS));
#endif
#ifdef SIGHUP
    s7_define(sc, env, s7_make_symbol(sc, "SIGHUP"), s7_make_integer(sc, SIGHUP));
#endif
#ifdef SIGINT
    s7_define(sc, env, s7_make_symbol(sc, "SIGINT"), s7_make_integer(sc, SIGINT));
#endif
#ifdef SIGQUIT
    s7_define(sc, env, s7_make_symbol(sc, "SIGQUIT"), s7_make_integer(sc, SIGQUIT));
#endif
#ifdef SIGILL
    s7_define(sc, env, s7_make_symbol(sc, "SIGILL"), s7_make_integer(sc, SIGILL));
#endif
#ifdef SIGTRAP
    s7_define(sc, env, s7_make_symbol(sc, "SIGTRAP"), s7_make_integer(sc, SIGTRAP));
#endif
#ifdef SIGABRT
    s7_define(sc, env, s7_make_symbol(sc, "SIGABRT"), s7_make_integer(sc, SIGABRT));
#endif
#ifdef SIGIOT
    s7_define(sc, env, s7_make_symbol(sc, "SIGIOT"), s7_make_integer(sc, SIGIOT));
#endif
#ifdef SIGBUS
    s7_define(sc, env, s7_make_symbol(sc, "SIGBUS"), s7_make_integer(sc, SIGBUS));
#endif
#ifdef SIGFPE
    s7_define(sc, env, s7_make_symbol(sc, "SIGFPE"), s7_make_integer(sc, SIGFPE));
#endif
#ifdef SIGKILL
    s7_define(sc, env, s7_make_symbol(sc, "SIGKILL"), s7_make_integer(sc, SIGKILL));
#endif
#ifdef SIGUSR1
    s7_define(sc, env, s7_make_symbol(sc, "SIGUSR1"), s7_make_integer(sc, SIGUSR1));
#endif
#ifdef SIGSEGV
    s7_define(sc, env, s7_make_symbol(sc, "SIGSEGV"), s7_make_integer(sc, SIGSEGV));
#endif
#ifdef SIGUSR2
    s7_define(sc, env, s7_make_symbol(sc, "SIGUSR2"), s7_make_integer(sc, SIGUSR2));
#endif
#ifdef SIGPIPE
    s7_define(sc, env, s7_make_symbol(sc, "SIGPIPE"), s7_make_integer(sc, SIGPIPE));
#endif
#ifdef SIGALRM
    s7_define(sc, env, s7_make_symbol(sc, "SIGALRM"), s7_make_integer(sc, SIGALRM));
#endif
#ifdef SIGTERM
    s7_define(sc, env, s7_make_symbol(sc, "SIGTERM"), s7_make_integer(sc, SIGTERM));
#endif
#ifdef SIGSTKFLT
    s7_define(sc, env, s7_make_symbol(sc, "SIGSTKFLT"), s7_make_integer(sc, SIGSTKFLT));
#endif
#ifdef SIGCLD
    s7_define(sc, env, s7_make_symbol(sc, "SIGCLD"), s7_make_integer(sc, SIGCLD));
#endif
#ifdef SIGCHLD
    s7_define(sc, env, s7_make_symbol(sc, "SIGCHLD"), s7_make_integer(sc, SIGCHLD));
#endif
#ifdef SIGCONT
    s7_define(sc, env, s7_make_symbol(sc, "SIGCONT"), s7_make_integer(sc, SIGCONT));
#endif
#ifdef SIGSTOP
    s7_define(sc, env, s7_make_symbol(sc, "SIGSTOP"), s7_make_integer(sc, SIGSTOP));
#endif
#ifdef SIGTSTP
    s7_define(sc, env, s7_make_symbol(sc, "SIGTSTP"), s7_make_integer(sc, SIGTSTP));
#endif
#ifdef SIGTTIN
    s7_define(sc, env, s7_make_symbol(sc, "SIGTTIN"), s7_make_integer(sc, SIGTTIN));
#endif
#ifdef SIGTTOU
    s7_define(sc, env, s7_make_symbol(sc, "SIGTTOU"), s7_make_integer(sc, SIGTTOU));
#endif
#ifdef SIGURG
    s7_define(sc, env, s7_make_symbol(sc, "SIGURG"), s7_make_integer(sc, SIGURG));
#endif
#ifdef SIGXCPU
    s7_define(sc, env, s7_make_symbol(sc, "SIGXCPU"), s7_make_integer(sc, SIGXCPU));
#endif
#ifdef SIGXFSZ
    s7_define(sc, env, s7_make_symbol(sc, "SIGXFSZ"), s7_make_integer(sc, SIGXFSZ));
#endif
#ifdef SIGVTALRM
    s7_define(sc, env, s7_make_symbol(sc, "SIGVTALRM"), s7_make_integer(sc, SIGVTALRM));
#endif
#ifdef SIGPROF
    s7_define(sc, env, s7_make_symbol(sc, "SIGPROF"), s7_make_integer(sc, SIGPROF));
#endif
#ifdef SIGWINCH
    s7_define(sc, env, s7_make_symbol(sc, "SIGWINCH"), s7_make_integer(sc, SIGWINCH));
#endif
#ifdef SIGPOLL
    s7_define(sc, env, s7_make_symbol(sc, "SIGPOLL"), s7_make_integer(sc, SIGPOLL));
#endif
#ifdef SIGIO
    s7_define(sc, env, s7_make_symbol(sc, "SIGIO"), s7_make_integer(sc, SIGIO));
#endif
#ifdef SIGPWR
    s7_define(sc, env, s7_make_symbol(sc, "SIGPWR"), s7_make_integer(sc, SIGPWR));
#endif
#ifdef SIGSYS
    s7_define(sc, env, s7_make_symbol(sc, "SIGSYS"), s7_make_integer(sc, SIGSYS));
#endif
#ifdef SIGUNUSED
    s7_define(sc, env, s7_make_symbol(sc, "SIGUNUSED"), s7_make_integer(sc, SIGUNUSED));
#endif
#ifdef WNOHANG
    s7_define(sc, env, s7_make_symbol(sc, "WNOHANG"), s7_make_integer(sc, WNOHANG));
#endif
#ifdef WUNTRACED
    s7_define(sc, env, s7_make_symbol(sc, "WUNTRACED"), s7_make_integer(sc, WUNTRACED));
#endif
#ifdef WSTOPPED
    s7_define(sc, env, s7_make_symbol(sc, "WSTOPPED"), s7_make_integer(sc, WSTOPPED));
#endif
#ifdef WEXITED
    s7_define(sc, env, s7_make_symbol(sc, "WEXITED"), s7_make_integer(sc, WEXITED));
#endif
#ifdef WCONTINUED
    s7_define(sc, env, s7_make_symbol(sc, "WCONTINUED"), s7_make_integer(sc, WCONTINUED));
#endif
#ifdef WNOWAIT
    s7_define(sc, env, s7_make_symbol(sc, "WNOWAIT"), s7_make_integer(sc, WNOWAIT));
#endif
#ifdef RLIMIT_CPU
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_CPU"), s7_make_integer(sc, RLIMIT_CPU));
#endif
#ifdef RLIMIT_FSIZE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_FSIZE"), s7_make_integer(sc, RLIMIT_FSIZE));
#endif
#ifdef RLIMIT_DATA
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_DATA"), s7_make_integer(sc, RLIMIT_DATA));
#endif
#ifdef RLIMIT_STACK
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_STACK"), s7_make_integer(sc, RLIMIT_STACK));
#endif
#ifdef RLIMIT_CORE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_CORE"), s7_make_integer(sc, RLIMIT_CORE));
#endif
#ifdef RLIMIT_RSS
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_RSS"), s7_make_integer(sc, RLIMIT_RSS));
#endif
#ifdef RLIMIT_NOFILE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_NOFILE"), s7_make_integer(sc, RLIMIT_NOFILE));
#endif
#ifdef RLIMIT_OFILE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_OFILE"), s7_make_integer(sc, RLIMIT_OFILE));
#endif
#ifdef RLIMIT_AS
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_AS"), s7_make_integer(sc, RLIMIT_AS));
#endif
#ifdef RLIMIT_NPROC
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_NPROC"), s7_make_integer(sc, RLIMIT_NPROC));
#endif
#ifdef RLIMIT_MEMLOCK
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_MEMLOCK"), s7_make_integer(sc, RLIMIT_MEMLOCK));
#endif
#ifdef RLIMIT_LOCKS
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_LOCKS"), s7_make_integer(sc, RLIMIT_LOCKS));
#endif
#ifdef RLIMIT_SIGPENDING
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_SIGPENDING"), s7_make_integer(sc, RLIMIT_SIGPENDING));
#endif
#ifdef RLIMIT_MSGQUEUE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_MSGQUEUE"), s7_make_integer(sc, RLIMIT_MSGQUEUE));
#endif
#ifdef RLIMIT_NICE
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_NICE"), s7_make_integer(sc, RLIMIT_NICE));
#endif
#ifdef RLIMIT_RTPRIO
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_RTPRIO"), s7_make_integer(sc, RLIMIT_RTPRIO));
#endif
#ifdef RLIMIT_NLIMITS
    s7_define(sc, env, s7_make_symbol(sc, "RLIMIT_NLIMITS"), s7_make_integer(sc, RLIMIT_NLIMITS));
#endif
#ifdef RLIM_NLIMITS
    s7_define(sc, env, s7_make_symbol(sc, "RLIM_NLIMITS"), s7_make_integer(sc, RLIM_NLIMITS));
#endif
#ifdef RLIM_INFINITY
    s7_define(sc, env, s7_make_symbol(sc, "RLIM_INFINITY"), s7_make_integer(sc, RLIM_INFINITY));
#endif
#ifdef RLIM_SAVED_MAX
    s7_define(sc, env, s7_make_symbol(sc, "RLIM_SAVED_MAX"), s7_make_integer(sc, RLIM_SAVED_MAX));
#endif
#ifdef RLIM_SAVED_CUR
    s7_define(sc, env, s7_make_symbol(sc, "RLIM_SAVED_CUR"), s7_make_integer(sc, RLIM_SAVED_CUR));
#endif
#ifdef RUSAGE_SELF
    s7_define(sc, env, s7_make_symbol(sc, "RUSAGE_SELF"), s7_make_integer(sc, RUSAGE_SELF));
#endif
#ifdef RUSAGE_CHILDREN
    s7_define(sc, env, s7_make_symbol(sc, "RUSAGE_CHILDREN"), s7_make_integer(sc, RUSAGE_CHILDREN));
#endif
#ifdef RUSAGE_THREAD
    s7_define(sc, env, s7_make_symbol(sc, "RUSAGE_THREAD"), s7_make_integer(sc, RUSAGE_THREAD));
#endif
#ifdef RUSAGE_LWP
    s7_define(sc, env, s7_make_symbol(sc, "RUSAGE_LWP"), s7_make_integer(sc, RUSAGE_LWP));
#endif
#ifdef PRIO_MIN
    s7_define(sc, env, s7_make_symbol(sc, "PRIO_MIN"), s7_make_integer(sc, PRIO_MIN));
#endif
#ifdef PRIO_MAX
    s7_define(sc, env, s7_make_symbol(sc, "PRIO_MAX"), s7_make_integer(sc, PRIO_MAX));
#endif
#ifdef PRIO_PROCESS
    s7_define(sc, env, s7_make_symbol(sc, "PRIO_PROCESS"), s7_make_integer(sc, PRIO_PROCESS));
#endif
#ifdef PRIO_PGRP
    s7_define(sc, env, s7_make_symbol(sc, "PRIO_PGRP"), s7_make_integer(sc, PRIO_PGRP));
#endif
#ifdef PRIO_USER
    s7_define(sc, env, s7_make_symbol(sc, "PRIO_USER"), s7_make_integer(sc, PRIO_USER));
#endif
#ifdef SA_NOCLDSTOP
    s7_define(sc, env, s7_make_symbol(sc, "SA_NOCLDSTOP"), s7_make_integer(sc, SA_NOCLDSTOP));
#endif
#ifdef SA_NOCLDWAIT
    s7_define(sc, env, s7_make_symbol(sc, "SA_NOCLDWAIT"), s7_make_integer(sc, SA_NOCLDWAIT));
#endif
#ifdef SA_SIGINFO
    s7_define(sc, env, s7_make_symbol(sc, "SA_SIGINFO"), s7_make_integer(sc, SA_SIGINFO));
#endif
#ifdef SA_ONSTACK
    s7_define(sc, env, s7_make_symbol(sc, "SA_ONSTACK"), s7_make_integer(sc, SA_ONSTACK));
#endif
#ifdef SA_RESTART
    s7_define(sc, env, s7_make_symbol(sc, "SA_RESTART"), s7_make_integer(sc, SA_RESTART));
#endif
#ifdef SA_NODEFER
    s7_define(sc, env, s7_make_symbol(sc, "SA_NODEFER"), s7_make_integer(sc, SA_NODEFER));
#endif
#ifdef SA_RESETHAND
    s7_define(sc, env, s7_make_symbol(sc, "SA_RESETHAND"), s7_make_integer(sc, SA_RESETHAND));
#endif
#ifdef SA_NOMASK
    s7_define(sc, env, s7_make_symbol(sc, "SA_NOMASK"), s7_make_integer(sc, SA_NOMASK));
#endif
#ifdef SA_ONESHOT
    s7_define(sc, env, s7_make_symbol(sc, "SA_ONESHOT"), s7_make_integer(sc, SA_ONESHOT));
#endif
#ifdef SA_STACK
    s7_define(sc, env, s7_make_symbol(sc, "SA_STACK"), s7_make_integer(sc, SA_STACK));
#endif
#ifdef SIG_BLOCK
    s7_define(sc, env, s7_make_symbol(sc, "SIG_BLOCK"), s7_make_integer(sc, SIG_BLOCK));
#endif
#ifdef SIG_UNBLOCK
    s7_define(sc, env, s7_make_symbol(sc, "SIG_UNBLOCK"), s7_make_integer(sc, SIG_UNBLOCK));
#endif
#ifdef SIG_SETMASK
    s7_define(sc, env, s7_make_symbol(sc, "SIG_SETMASK"), s7_make_integer(sc, SIG_SETMASK));
#endif
#ifdef AI_PASSIVE
    s7_define(sc, env, s7_make_symbol(sc, "AI_PASSIVE"), s7_make_integer(sc, AI_PASSIVE));
#endif
#ifdef AI_CANONNAME
    s7_define(sc, env, s7_make_symbol(sc, "AI_CANONNAME"), s7_make_integer(sc, AI_CANONNAME));
#endif
#ifdef AI_NUMERICHOST
    s7_define(sc, env, s7_make_symbol(sc, "AI_NUMERICHOST"), s7_make_integer(sc, AI_NUMERICHOST));
#endif
#ifdef AI_V4MAPPED
    s7_define(sc, env, s7_make_symbol(sc, "AI_V4MAPPED"), s7_make_integer(sc, AI_V4MAPPED));
#endif
#ifdef AI_ALL
    s7_define(sc, env, s7_make_symbol(sc, "AI_ALL"), s7_make_integer(sc, AI_ALL));
#endif
#ifdef AI_ADDRCONFIG
    s7_define(sc, env, s7_make_symbol(sc, "AI_ADDRCONFIG"), s7_make_integer(sc, AI_ADDRCONFIG));
#endif
#ifdef AI_NUMERICSERV
    s7_define(sc, env, s7_make_symbol(sc, "AI_NUMERICSERV"), s7_make_integer(sc, AI_NUMERICSERV));
#endif
#ifdef EAI_BADFLAGS
    s7_define(sc, env, s7_make_symbol(sc, "EAI_BADFLAGS"), s7_make_integer(sc, EAI_BADFLAGS));
#endif
#ifdef EAI_NONAME
    s7_define(sc, env, s7_make_symbol(sc, "EAI_NONAME"), s7_make_integer(sc, EAI_NONAME));
#endif
#ifdef EAI_AGAIN
    s7_define(sc, env, s7_make_symbol(sc, "EAI_AGAIN"), s7_make_integer(sc, EAI_AGAIN));
#endif
#ifdef EAI_FAIL
    s7_define(sc, env, s7_make_symbol(sc, "EAI_FAIL"), s7_make_integer(sc, EAI_FAIL));
#endif
#ifdef EAI_FAMILY
    s7_define(sc, env, s7_make_symbol(sc, "EAI_FAMILY"), s7_make_integer(sc, EAI_FAMILY));
#endif
#ifdef EAI_SOCKTYPE
    s7_define(sc, env, s7_make_symbol(sc, "EAI_SOCKTYPE"), s7_make_integer(sc, EAI_SOCKTYPE));
#endif
#ifdef EAI_SERVICE
    s7_define(sc, env, s7_make_symbol(sc, "EAI_SERVICE"), s7_make_integer(sc, EAI_SERVICE));
#endif
#ifdef EAI_MEMORY
    s7_define(sc, env, s7_make_symbol(sc, "EAI_MEMORY"), s7_make_integer(sc, EAI_MEMORY));
#endif
#ifdef EAI_SYSTEM
    s7_define(sc, env, s7_make_symbol(sc, "EAI_SYSTEM"), s7_make_integer(sc, EAI_SYSTEM));
#endif
#ifdef EAI_OVERFLOW
    s7_define(sc, env, s7_make_symbol(sc, "EAI_OVERFLOW"), s7_make_integer(sc, EAI_OVERFLOW));
#endif
#ifdef NI_NUMERICHOST
    s7_define(sc, env, s7_make_symbol(sc, "NI_NUMERICHOST"), s7_make_integer(sc, NI_NUMERICHOST));
#endif
#ifdef NI_NUMERICSERV
    s7_define(sc, env, s7_make_symbol(sc, "NI_NUMERICSERV"), s7_make_integer(sc, NI_NUMERICSERV));
#endif
#ifdef NI_NOFQDN
    s7_define(sc, env, s7_make_symbol(sc, "NI_NOFQDN"), s7_make_integer(sc, NI_NOFQDN));
#endif
#ifdef NI_NAMEREQD
    s7_define(sc, env, s7_make_symbol(sc, "NI_NAMEREQD"), s7_make_integer(sc, NI_NAMEREQD));
#endif
#ifdef NI_DGRAM
    s7_define(sc, env, s7_make_symbol(sc, "NI_DGRAM"), s7_make_integer(sc, NI_DGRAM));
#endif
#ifdef SOCK_STREAM
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_STREAM"), s7_make_integer(sc, SOCK_STREAM));
#endif
#ifdef SOCK_DGRAM
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_DGRAM"), s7_make_integer(sc, SOCK_DGRAM));
#endif
#ifdef SOCK_RAW
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_RAW"), s7_make_integer(sc, SOCK_RAW));
#endif
#ifdef SOCK_RDM
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_RDM"), s7_make_integer(sc, SOCK_RDM));
#endif
#ifdef SOCK_SEQPACKET
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_SEQPACKET"), s7_make_integer(sc, SOCK_SEQPACKET));
#endif
#ifdef SOCK_DCCP
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_DCCP"), s7_make_integer(sc, SOCK_DCCP));
#endif
#ifdef SOCK_PACKET
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_PACKET"), s7_make_integer(sc, SOCK_PACKET));
#endif
#ifdef SOCK_CLOEXEC
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_CLOEXEC"), s7_make_integer(sc, SOCK_CLOEXEC));
#endif
#ifdef SOCK_NONBLOCK
    s7_define(sc, env, s7_make_symbol(sc, "SOCK_NONBLOCK"), s7_make_integer(sc, SOCK_NONBLOCK));
#endif
#ifdef PF_UNSPEC
    s7_define(sc, env, s7_make_symbol(sc, "PF_UNSPEC"), s7_make_integer(sc, PF_UNSPEC));
#endif
#ifdef PF_LOCAL
    s7_define(sc, env, s7_make_symbol(sc, "PF_LOCAL"), s7_make_integer(sc, PF_LOCAL));
#endif
#ifdef PF_UNIX
    s7_define(sc, env, s7_make_symbol(sc, "PF_UNIX"), s7_make_integer(sc, PF_UNIX));
#endif
#ifdef PF_FILE
    s7_define(sc, env, s7_make_symbol(sc, "PF_FILE"), s7_make_integer(sc, PF_FILE));
#endif
#ifdef PF_INET
    s7_define(sc, env, s7_make_symbol(sc, "PF_INET"), s7_make_integer(sc, PF_INET));
#endif
#ifdef PF_AX25
    s7_define(sc, env, s7_make_symbol(sc, "PF_AX25"), s7_make_integer(sc, PF_AX25));
#endif
#ifdef PF_IPX
    s7_define(sc, env, s7_make_symbol(sc, "PF_IPX"), s7_make_integer(sc, PF_IPX));
#endif
#ifdef PF_APPLETALK
    s7_define(sc, env, s7_make_symbol(sc, "PF_APPLETALK"), s7_make_integer(sc, PF_APPLETALK));
#endif
#ifdef PF_NETROM
    s7_define(sc, env, s7_make_symbol(sc, "PF_NETROM"), s7_make_integer(sc, PF_NETROM));
#endif
#ifdef PF_BRIDGE
    s7_define(sc, env, s7_make_symbol(sc, "PF_BRIDGE"), s7_make_integer(sc, PF_BRIDGE));
#endif
#ifdef PF_ATMPVC
    s7_define(sc, env, s7_make_symbol(sc, "PF_ATMPVC"), s7_make_integer(sc, PF_ATMPVC));
#endif
#ifdef PF_X25
    s7_define(sc, env, s7_make_symbol(sc, "PF_X25"), s7_make_integer(sc, PF_X25));
#endif
#ifdef PF_INET6
    s7_define(sc, env, s7_make_symbol(sc, "PF_INET6"), s7_make_integer(sc, PF_INET6));
#endif
#ifdef PF_ROSE
    s7_define(sc, env, s7_make_symbol(sc, "PF_ROSE"), s7_make_integer(sc, PF_ROSE));
#endif
#ifdef PF_DECnet
    s7_define(sc, env, s7_make_symbol(sc, "PF_DECnet"), s7_make_integer(sc, PF_DECnet));
#endif
#ifdef PF_NETBEUI
    s7_define(sc, env, s7_make_symbol(sc, "PF_NETBEUI"), s7_make_integer(sc, PF_NETBEUI));
#endif
#ifdef PF_SECURITY
    s7_define(sc, env, s7_make_symbol(sc, "PF_SECURITY"), s7_make_integer(sc, PF_SECURITY));
#endif
#ifdef PF_KEY
    s7_define(sc, env, s7_make_symbol(sc, "PF_KEY"), s7_make_integer(sc, PF_KEY));
#endif
#ifdef PF_NETLINK
    s7_define(sc, env, s7_make_symbol(sc, "PF_NETLINK"), s7_make_integer(sc, PF_NETLINK));
#endif
#ifdef PF_ROUTE
    s7_define(sc, env, s7_make_symbol(sc, "PF_ROUTE"), s7_make_integer(sc, PF_ROUTE));
#endif
#ifdef PF_PACKET
    s7_define(sc, env, s7_make_symbol(sc, "PF_PACKET"), s7_make_integer(sc, PF_PACKET));
#endif
#ifdef PF_ASH
    s7_define(sc, env, s7_make_symbol(sc, "PF_ASH"), s7_make_integer(sc, PF_ASH));
#endif
#ifdef PF_ECONET
    s7_define(sc, env, s7_make_symbol(sc, "PF_ECONET"), s7_make_integer(sc, PF_ECONET));
#endif
#ifdef PF_ATMSVC
    s7_define(sc, env, s7_make_symbol(sc, "PF_ATMSVC"), s7_make_integer(sc, PF_ATMSVC));
#endif
#ifdef PF_RDS
    s7_define(sc, env, s7_make_symbol(sc, "PF_RDS"), s7_make_integer(sc, PF_RDS));
#endif
#ifdef PF_SNA
    s7_define(sc, env, s7_make_symbol(sc, "PF_SNA"), s7_make_integer(sc, PF_SNA));
#endif
#ifdef PF_IRDA
    s7_define(sc, env, s7_make_symbol(sc, "PF_IRDA"), s7_make_integer(sc, PF_IRDA));
#endif
#ifdef PF_PPPOX
    s7_define(sc, env, s7_make_symbol(sc, "PF_PPPOX"), s7_make_integer(sc, PF_PPPOX));
#endif
#ifdef PF_WANPIPE
    s7_define(sc, env, s7_make_symbol(sc, "PF_WANPIPE"), s7_make_integer(sc, PF_WANPIPE));
#endif
#ifdef PF_LLC
    s7_define(sc, env, s7_make_symbol(sc, "PF_LLC"), s7_make_integer(sc, PF_LLC));
#endif
#ifdef PF_CAN
    s7_define(sc, env, s7_make_symbol(sc, "PF_CAN"), s7_make_integer(sc, PF_CAN));
#endif
#ifdef PF_TIPC
    s7_define(sc, env, s7_make_symbol(sc, "PF_TIPC"), s7_make_integer(sc, PF_TIPC));
#endif
#ifdef PF_BLUETOOTH
    s7_define(sc, env, s7_make_symbol(sc, "PF_BLUETOOTH"), s7_make_integer(sc, PF_BLUETOOTH));
#endif
#ifdef PF_IUCV
    s7_define(sc, env, s7_make_symbol(sc, "PF_IUCV"), s7_make_integer(sc, PF_IUCV));
#endif
#ifdef PF_RXRPC
    s7_define(sc, env, s7_make_symbol(sc, "PF_RXRPC"), s7_make_integer(sc, PF_RXRPC));
#endif
#ifdef PF_ISDN
    s7_define(sc, env, s7_make_symbol(sc, "PF_ISDN"), s7_make_integer(sc, PF_ISDN));
#endif
#ifdef PF_PHONET
    s7_define(sc, env, s7_make_symbol(sc, "PF_PHONET"), s7_make_integer(sc, PF_PHONET));
#endif
#ifdef PF_IEEE802154
    s7_define(sc, env, s7_make_symbol(sc, "PF_IEEE802154"), s7_make_integer(sc, PF_IEEE802154));
#endif
#ifdef PF_MAX
    s7_define(sc, env, s7_make_symbol(sc, "PF_MAX"), s7_make_integer(sc, PF_MAX));
#endif
#ifdef AF_UNSPEC
    s7_define(sc, env, s7_make_symbol(sc, "AF_UNSPEC"), s7_make_integer(sc, AF_UNSPEC));
#endif
#ifdef AF_LOCAL
    s7_define(sc, env, s7_make_symbol(sc, "AF_LOCAL"), s7_make_integer(sc, AF_LOCAL));
#endif
#ifdef AF_UNIX
    s7_define(sc, env, s7_make_symbol(sc, "AF_UNIX"), s7_make_integer(sc, AF_UNIX));
#endif
#ifdef AF_FILE
    s7_define(sc, env, s7_make_symbol(sc, "AF_FILE"), s7_make_integer(sc, AF_FILE));
#endif
#ifdef AF_INET
    s7_define(sc, env, s7_make_symbol(sc, "AF_INET"), s7_make_integer(sc, AF_INET));
#endif
#ifdef AF_AX25
    s7_define(sc, env, s7_make_symbol(sc, "AF_AX25"), s7_make_integer(sc, AF_AX25));
#endif
#ifdef AF_IPX
    s7_define(sc, env, s7_make_symbol(sc, "AF_IPX"), s7_make_integer(sc, AF_IPX));
#endif
#ifdef AF_APPLETALK
    s7_define(sc, env, s7_make_symbol(sc, "AF_APPLETALK"), s7_make_integer(sc, AF_APPLETALK));
#endif
#ifdef AF_NETROM
    s7_define(sc, env, s7_make_symbol(sc, "AF_NETROM"), s7_make_integer(sc, AF_NETROM));
#endif
#ifdef AF_BRIDGE
    s7_define(sc, env, s7_make_symbol(sc, "AF_BRIDGE"), s7_make_integer(sc, AF_BRIDGE));
#endif
#ifdef AF_ATMPVC
    s7_define(sc, env, s7_make_symbol(sc, "AF_ATMPVC"), s7_make_integer(sc, AF_ATMPVC));
#endif
#ifdef AF_X25
    s7_define(sc, env, s7_make_symbol(sc, "AF_X25"), s7_make_integer(sc, AF_X25));
#endif
#ifdef AF_INET6
    s7_define(sc, env, s7_make_symbol(sc, "AF_INET6"), s7_make_integer(sc, AF_INET6));
#endif
#ifdef AF_ROSE
    s7_define(sc, env, s7_make_symbol(sc, "AF_ROSE"), s7_make_integer(sc, AF_ROSE));
#endif
#ifdef AF_DECnet
    s7_define(sc, env, s7_make_symbol(sc, "AF_DECnet"), s7_make_integer(sc, AF_DECnet));
#endif
#ifdef AF_NETBEUI
    s7_define(sc, env, s7_make_symbol(sc, "AF_NETBEUI"), s7_make_integer(sc, AF_NETBEUI));
#endif
#ifdef AF_SECURITY
    s7_define(sc, env, s7_make_symbol(sc, "AF_SECURITY"), s7_make_integer(sc, AF_SECURITY));
#endif
#ifdef AF_KEY
    s7_define(sc, env, s7_make_symbol(sc, "AF_KEY"), s7_make_integer(sc, AF_KEY));
#endif
#ifdef AF_NETLINK
    s7_define(sc, env, s7_make_symbol(sc, "AF_NETLINK"), s7_make_integer(sc, AF_NETLINK));
#endif
#ifdef AF_ROUTE
    s7_define(sc, env, s7_make_symbol(sc, "AF_ROUTE"), s7_make_integer(sc, AF_ROUTE));
#endif
#ifdef AF_PACKET
    s7_define(sc, env, s7_make_symbol(sc, "AF_PACKET"), s7_make_integer(sc, AF_PACKET));
#endif
#ifdef AF_ASH
    s7_define(sc, env, s7_make_symbol(sc, "AF_ASH"), s7_make_integer(sc, AF_ASH));
#endif
#ifdef AF_ECONET
    s7_define(sc, env, s7_make_symbol(sc, "AF_ECONET"), s7_make_integer(sc, AF_ECONET));
#endif
#ifdef AF_ATMSVC
    s7_define(sc, env, s7_make_symbol(sc, "AF_ATMSVC"), s7_make_integer(sc, AF_ATMSVC));
#endif
#ifdef AF_RDS
    s7_define(sc, env, s7_make_symbol(sc, "AF_RDS"), s7_make_integer(sc, AF_RDS));
#endif
#ifdef AF_SNA
    s7_define(sc, env, s7_make_symbol(sc, "AF_SNA"), s7_make_integer(sc, AF_SNA));
#endif
#ifdef AF_IRDA
    s7_define(sc, env, s7_make_symbol(sc, "AF_IRDA"), s7_make_integer(sc, AF_IRDA));
#endif
#ifdef AF_PPPOX
    s7_define(sc, env, s7_make_symbol(sc, "AF_PPPOX"), s7_make_integer(sc, AF_PPPOX));
#endif
#ifdef AF_WANPIPE
    s7_define(sc, env, s7_make_symbol(sc, "AF_WANPIPE"), s7_make_integer(sc, AF_WANPIPE));
#endif
#ifdef AF_LLC
    s7_define(sc, env, s7_make_symbol(sc, "AF_LLC"), s7_make_integer(sc, AF_LLC));
#endif
#ifdef AF_CAN
    s7_define(sc, env, s7_make_symbol(sc, "AF_CAN"), s7_make_integer(sc, AF_CAN));
#endif
#ifdef AF_TIPC
    s7_define(sc, env, s7_make_symbol(sc, "AF_TIPC"), s7_make_integer(sc, AF_TIPC));
#endif
#ifdef AF_BLUETOOTH
    s7_define(sc, env, s7_make_symbol(sc, "AF_BLUETOOTH"), s7_make_integer(sc, AF_BLUETOOTH));
#endif
#ifdef AF_IUCV
    s7_define(sc, env, s7_make_symbol(sc, "AF_IUCV"), s7_make_integer(sc, AF_IUCV));
#endif
#ifdef AF_RXRPC
    s7_define(sc, env, s7_make_symbol(sc, "AF_RXRPC"), s7_make_integer(sc, AF_RXRPC));
#endif
#ifdef AF_ISDN
    s7_define(sc, env, s7_make_symbol(sc, "AF_ISDN"), s7_make_integer(sc, AF_ISDN));
#endif
#ifdef AF_PHONET
    s7_define(sc, env, s7_make_symbol(sc, "AF_PHONET"), s7_make_integer(sc, AF_PHONET));
#endif
#ifdef AF_IEEE802154
    s7_define(sc, env, s7_make_symbol(sc, "AF_IEEE802154"), s7_make_integer(sc, AF_IEEE802154));
#endif
#ifdef AF_MAX
    s7_define(sc, env, s7_make_symbol(sc, "AF_MAX"), s7_make_integer(sc, AF_MAX));
#endif
#ifdef MSG_OOB
    s7_define(sc, env, s7_make_symbol(sc, "MSG_OOB"), s7_make_integer(sc, MSG_OOB));
#endif
#ifdef MSG_PEEK
    s7_define(sc, env, s7_make_symbol(sc, "MSG_PEEK"), s7_make_integer(sc, MSG_PEEK));
#endif
#ifdef MSG_DONTROUTE
    s7_define(sc, env, s7_make_symbol(sc, "MSG_DONTROUTE"), s7_make_integer(sc, MSG_DONTROUTE));
#endif
#ifdef MSG_CTRUNC
    s7_define(sc, env, s7_make_symbol(sc, "MSG_CTRUNC"), s7_make_integer(sc, MSG_CTRUNC));
#endif
#ifdef MSG_PROXY
    s7_define(sc, env, s7_make_symbol(sc, "MSG_PROXY"), s7_make_integer(sc, MSG_PROXY));
#endif
#ifdef MSG_TRUNC
    s7_define(sc, env, s7_make_symbol(sc, "MSG_TRUNC"), s7_make_integer(sc, MSG_TRUNC));
#endif
#ifdef MSG_DONTWAIT
    s7_define(sc, env, s7_make_symbol(sc, "MSG_DONTWAIT"), s7_make_integer(sc, MSG_DONTWAIT));
#endif
#ifdef MSG_EOR
    s7_define(sc, env, s7_make_symbol(sc, "MSG_EOR"), s7_make_integer(sc, MSG_EOR));
#endif
#ifdef MSG_WAITFORONE
    s7_define(sc, env, s7_make_symbol(sc, "MSG_WAITFORONE"), s7_make_integer(sc, MSG_WAITFORONE));
#endif
#ifdef MSG_WAITALL
    s7_define(sc, env, s7_make_symbol(sc, "MSG_WAITALL"), s7_make_integer(sc, MSG_WAITALL));
#endif
#ifdef MSG_FIN
    s7_define(sc, env, s7_make_symbol(sc, "MSG_FIN"), s7_make_integer(sc, MSG_FIN));
#endif
#ifdef MSG_SYN
    s7_define(sc, env, s7_make_symbol(sc, "MSG_SYN"), s7_make_integer(sc, MSG_SYN));
#endif
#ifdef MSG_CONFIRM
    s7_define(sc, env, s7_make_symbol(sc, "MSG_CONFIRM"), s7_make_integer(sc, MSG_CONFIRM));
#endif
#ifdef MSG_RST
    s7_define(sc, env, s7_make_symbol(sc, "MSG_RST"), s7_make_integer(sc, MSG_RST));
#endif
#ifdef MSG_ERRQUEUE
    s7_define(sc, env, s7_make_symbol(sc, "MSG_ERRQUEUE"), s7_make_integer(sc, MSG_ERRQUEUE));
#endif
#ifdef MSG_NOSIGNAL
    s7_define(sc, env, s7_make_symbol(sc, "MSG_NOSIGNAL"), s7_make_integer(sc, MSG_NOSIGNAL));
#endif
#ifdef MSG_MORE
    s7_define(sc, env, s7_make_symbol(sc, "MSG_MORE"), s7_make_integer(sc, MSG_MORE));
#endif
#ifdef MSG_CMSG_CLOEXEC
    s7_define(sc, env, s7_make_symbol(sc, "MSG_CMSG_CLOEXEC"), s7_make_integer(sc, MSG_CMSG_CLOEXEC));
#endif
#ifdef IPPROTO_IP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_IP"), s7_make_integer(sc, IPPROTO_IP));
#endif
#ifdef IPPROTO_HOPOPTS
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_HOPOPTS"), s7_make_integer(sc, IPPROTO_HOPOPTS));
#endif
#ifdef IPPROTO_ICMP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_ICMP"), s7_make_integer(sc, IPPROTO_ICMP));
#endif
#ifdef IPPROTO_IGMP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_IGMP"), s7_make_integer(sc, IPPROTO_IGMP));
#endif
#ifdef IPPROTO_IPIP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_IPIP"), s7_make_integer(sc, IPPROTO_IPIP));
#endif
#ifdef IPPROTO_TCP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_TCP"), s7_make_integer(sc, IPPROTO_TCP));
#endif
#ifdef IPPROTO_EGP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_EGP"), s7_make_integer(sc, IPPROTO_EGP));
#endif
#ifdef IPPROTO_PUP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_PUP"), s7_make_integer(sc, IPPROTO_PUP));
#endif
#ifdef IPPROTO_UDP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_UDP"), s7_make_integer(sc, IPPROTO_UDP));
#endif
#ifdef IPPROTO_IDP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_IDP"), s7_make_integer(sc, IPPROTO_IDP));
#endif
#ifdef IPPROTO_TP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_TP"), s7_make_integer(sc, IPPROTO_TP));
#endif
#ifdef IPPROTO_DCCP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_DCCP"), s7_make_integer(sc, IPPROTO_DCCP));
#endif
#ifdef IPPROTO_IPV6
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_IPV6"), s7_make_integer(sc, IPPROTO_IPV6));
#endif
#ifdef IPPROTO_ROUTING
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_ROUTING"), s7_make_integer(sc, IPPROTO_ROUTING));
#endif
#ifdef IPPROTO_FRAGMENT
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_FRAGMENT"), s7_make_integer(sc, IPPROTO_FRAGMENT));
#endif
#ifdef IPPROTO_RSVP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_RSVP"), s7_make_integer(sc, IPPROTO_RSVP));
#endif
#ifdef IPPROTO_GRE
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_GRE"), s7_make_integer(sc, IPPROTO_GRE));
#endif
#ifdef IPPROTO_ESP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_ESP"), s7_make_integer(sc, IPPROTO_ESP));
#endif
#ifdef IPPROTO_AH
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_AH"), s7_make_integer(sc, IPPROTO_AH));
#endif
#ifdef IPPROTO_ICMPV6
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_ICMPV6"), s7_make_integer(sc, IPPROTO_ICMPV6));
#endif
#ifdef IPPROTO_NONE
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_NONE"), s7_make_integer(sc, IPPROTO_NONE));
#endif
#ifdef IPPROTO_DSTOPTS
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_DSTOPTS"), s7_make_integer(sc, IPPROTO_DSTOPTS));
#endif
#ifdef IPPROTO_MTP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_MTP"), s7_make_integer(sc, IPPROTO_MTP));
#endif
#ifdef IPPROTO_ENCAP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_ENCAP"), s7_make_integer(sc, IPPROTO_ENCAP));
#endif
#ifdef IPPROTO_PIM
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_PIM"), s7_make_integer(sc, IPPROTO_PIM));
#endif
#ifdef IPPROTO_COMP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_COMP"), s7_make_integer(sc, IPPROTO_COMP));
#endif
#ifdef IPPROTO_SCTP
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_SCTP"), s7_make_integer(sc, IPPROTO_SCTP));
#endif
#ifdef IPPROTO_UDPLITE
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_UDPLITE"), s7_make_integer(sc, IPPROTO_UDPLITE));
#endif
#ifdef IPPROTO_RAW
    s7_define(sc, env, s7_make_symbol(sc, "IPPROTO_RAW"), s7_make_integer(sc, IPPROTO_RAW));
#endif
#ifdef SOL_RAW
    s7_define(sc, env, s7_make_symbol(sc, "SOL_RAW"), s7_make_integer(sc, SOL_RAW));
#endif
#ifdef SOL_DECNET
    s7_define(sc, env, s7_make_symbol(sc, "SOL_DECNET"), s7_make_integer(sc, SOL_DECNET));
#endif
#ifdef SOL_X25
    s7_define(sc, env, s7_make_symbol(sc, "SOL_X25"), s7_make_integer(sc, SOL_X25));
#endif
#ifdef SOL_PACKET
    s7_define(sc, env, s7_make_symbol(sc, "SOL_PACKET"), s7_make_integer(sc, SOL_PACKET));
#endif
#ifdef SOL_ATM
    s7_define(sc, env, s7_make_symbol(sc, "SOL_ATM"), s7_make_integer(sc, SOL_ATM));
#endif
#ifdef SOL_AAL
    s7_define(sc, env, s7_make_symbol(sc, "SOL_AAL"), s7_make_integer(sc, SOL_AAL));
#endif
#ifdef SOL_IRDA
    s7_define(sc, env, s7_make_symbol(sc, "SOL_IRDA"), s7_make_integer(sc, SOL_IRDA));
#endif
#ifdef SHUT_RD
    s7_define(sc, env, s7_make_symbol(sc, "SHUT_RD"), s7_make_integer(sc, SHUT_RD));
#endif
#ifdef SHUT_WR
    s7_define(sc, env, s7_make_symbol(sc, "SHUT_WR"), s7_make_integer(sc, SHUT_WR));
#endif
#ifdef SHUT_RDWR
    s7_define(sc, env, s7_make_symbol(sc, "SHUT_RDWR"), s7_make_integer(sc, SHUT_RDWR));
#endif
#ifdef REG_NOTBOL
    s7_define(sc, env, s7_make_symbol(sc, "REG_NOTBOL"), s7_make_integer(sc, REG_NOTBOL));
#endif
#ifdef REG_NOTEOL
    s7_define(sc, env, s7_make_symbol(sc, "REG_NOTEOL"), s7_make_integer(sc, REG_NOTEOL));
#endif
#ifdef REG_NOMATCH
    s7_define(sc, env, s7_make_symbol(sc, "REG_NOMATCH"), s7_make_integer(sc, REG_NOMATCH));
#endif
#ifdef REG_ESPACE
    s7_define(sc, env, s7_make_symbol(sc, "REG_ESPACE"), s7_make_integer(sc, REG_ESPACE));
#endif
#ifdef REG_BADBR
    s7_define(sc, env, s7_make_symbol(sc, "REG_BADBR"), s7_make_integer(sc, REG_BADBR));
#endif
#ifdef REG_BADPAT
    s7_define(sc, env, s7_make_symbol(sc, "REG_BADPAT"), s7_make_integer(sc, REG_BADPAT));
#endif
#ifdef REG_BADRPT
    s7_define(sc, env, s7_make_symbol(sc, "REG_BADRPT"), s7_make_integer(sc, REG_BADRPT));
#endif
#ifdef REG_ECOLLATE
    s7_define(sc, env, s7_make_symbol(sc, "REG_ECOLLATE"), s7_make_integer(sc, REG_ECOLLATE));
#endif
#ifdef REG_ECTYPE
    s7_define(sc, env, s7_make_symbol(sc, "REG_ECTYPE"), s7_make_integer(sc, REG_ECTYPE));
#endif
#ifdef REG_EESCAPE
    s7_define(sc, env, s7_make_symbol(sc, "REG_EESCAPE"), s7_make_integer(sc, REG_EESCAPE));
#endif
#ifdef REG_ESUBREG
    s7_define(sc, env, s7_make_symbol(sc, "REG_ESUBREG"), s7_make_integer(sc, REG_ESUBREG));
#endif
#ifdef REG_EBRACK
    s7_define(sc, env, s7_make_symbol(sc, "REG_EBRACK"), s7_make_integer(sc, REG_EBRACK));
#endif
#ifdef REG_EPAREN
    s7_define(sc, env, s7_make_symbol(sc, "REG_EPAREN"), s7_make_integer(sc, REG_EPAREN));
#endif
#ifdef REG_EBRACE
    s7_define(sc, env, s7_make_symbol(sc, "REG_EBRACE"), s7_make_integer(sc, REG_EBRACE));
#endif
#ifdef REG_ERANGE
    s7_define(sc, env, s7_make_symbol(sc, "REG_ERANGE"), s7_make_integer(sc, REG_ERANGE));
#endif
#ifdef REG_EXTENDED
    s7_define(sc, env, s7_make_symbol(sc, "REG_EXTENDED"), s7_make_integer(sc, REG_EXTENDED));
#endif
#ifdef REG_ICASE
    s7_define(sc, env, s7_make_symbol(sc, "REG_ICASE"), s7_make_integer(sc, REG_ICASE));
#endif
#ifdef REG_NOSUB
    s7_define(sc, env, s7_make_symbol(sc, "REG_NOSUB"), s7_make_integer(sc, REG_NOSUB));
#endif
#ifdef REG_NEWLINE
    s7_define(sc, env, s7_make_symbol(sc, "REG_NEWLINE"), s7_make_integer(sc, REG_NEWLINE));
#endif
#ifdef FLT_MAX
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MAX"), s7_make_real(sc, FLT_MAX));
#endif
#ifdef DBL_MAX
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MAX"), s7_make_real(sc, DBL_MAX));
#endif
#ifdef LDBL_MAX
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MAX"), s7_make_real(sc, LDBL_MAX));
#endif
#ifdef FLT_EPSILON
    s7_define(sc, env, s7_make_symbol(sc, "FLT_EPSILON"), s7_make_real(sc, FLT_EPSILON));
#endif
#ifdef DBL_EPSILON
    s7_define(sc, env, s7_make_symbol(sc, "DBL_EPSILON"), s7_make_real(sc, DBL_EPSILON));
#endif
#ifdef LDBL_EPSILON
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_EPSILON"), s7_make_real(sc, LDBL_EPSILON));
#endif
#ifdef FLT_MIN
    s7_define(sc, env, s7_make_symbol(sc, "FLT_MIN"), s7_make_real(sc, FLT_MIN));
#endif
#ifdef DBL_MIN
    s7_define(sc, env, s7_make_symbol(sc, "DBL_MIN"), s7_make_real(sc, DBL_MIN));
#endif
#ifdef LDBL_MIN
    s7_define(sc, env, s7_make_symbol(sc, "LDBL_MIN"), s7_make_real(sc, LDBL_MIN));
#endif
#ifdef P_tmpdir
    s7_define(sc, env, s7_make_symbol(sc, "P_tmpdir"), s7_make_string(sc, P_tmpdir));
#endif
#ifdef _PATH_HEQUIV
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_HEQUIV"), s7_make_string(sc, _PATH_HEQUIV));
#endif
#ifdef _PATH_HOSTS
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_HOSTS"), s7_make_string(sc, _PATH_HOSTS));
#endif
#ifdef _PATH_NETWORKS
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_NETWORKS"), s7_make_string(sc, _PATH_NETWORKS));
#endif
#ifdef _PATH_NSSWITCH_CONF
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_NSSWITCH_CONF"), s7_make_string(sc, _PATH_NSSWITCH_CONF));
#endif
#ifdef _PATH_PROTOCOLS
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_PROTOCOLS"), s7_make_string(sc, _PATH_PROTOCOLS));
#endif
#ifdef _PATH_SERVICES
    s7_define(sc, env, s7_make_symbol(sc, "_PATH_SERVICES"), s7_make_string(sc, _PATH_SERVICES));
#endif
    return env;
}
