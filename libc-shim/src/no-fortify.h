// We don't expect overloads of the libc, which _FORTIFY_SOURCE provides us
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif