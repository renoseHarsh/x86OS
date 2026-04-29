extern void (*_yield)();

#define yield() _yield()
