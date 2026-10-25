/* the first thing the kernel runs (PID 1). for now: a banner, then it echoes what you type.
 * this is where the editor with tcc will live. it must never return: when PID 1 ends, the kernel panics */
#include "c.h"

int main(int argc, char **argv)
{
    char line[256];
    (void)argc; (void)argv;
    puts("Chamesle Source Linux");
    puts("there's nothing here yet. you'll write all of it.");
    for (;;) {
        puts_fd(1, "> ");
        ssize_t n = read(0, line, sizeof line - 1);
        if (n <= 0) continue;
        line[n] = 0;
        puts_fd(1, "you typed: ");
        puts_fd(1, line);
    }
}
