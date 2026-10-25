/* wq.c: Chamesle Source's :wq. ex has written the file, tcc compiles it against c.h, and the program takes vi's
 * place as PID 1: hello.c becomes /hello, and runs. if it doesn't compile, the errors are on the screen and ex
 * carries on, so you fix them and :wq again */
static void wq_message(void *unused, const char *msg)
{
    (void)unused;
    fputs(msg, stderr);
    fputc('\n', stderr);
}

/* ex calls it with the name it wrote. it only comes back if something went wrong */
int wqrun(char *file)
{
    static char out[256];
    char *argv[2];
    TCCState *s;
    size_t n = c_strlen(file);
    if (n >= sizeof out) return -1;
    c_strcpy(out, file);
    if (n > 2 && !c_strcmp(out + n - 2, ".c")) out[n - 2] = 0;
    else c_strcpy(out + n, ".out");
    s = tcc_new();
    if (!s) return -1;
    tcc_set_error_func(s, NULL, wq_message);
    tcc_set_options(s, "-nostdlib -nostdinc -static");
    tcc_add_include_path(s, "/usr/include");
    tcc_set_output_type(s, TCC_OUTPUT_EXE);
    if (tcc_add_file(s, file) < 0 || tcc_output_file(s, out) < 0) {
        tcc_delete(s);
        return -1;
    }
    tcc_delete(s);
    argv[0] = out;
    argv[1] = NULL;
    execve(out, argv, environ);
    fprintf(stderr, "%s: %s\n", out, strerror(errno));
    return -1;
}
