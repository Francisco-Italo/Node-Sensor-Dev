#ifndef E32_H
#define E32_H

enum module_mode
{
    NORMAL=0,
    WAKEUP,
    POWERDOWN,
    PROGRAM
};

void e32_set_mode(enum module_mode);

#endif // E32_H
