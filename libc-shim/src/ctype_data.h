#pragma once

// This file can not have any includes

namespace shim {

    extern const char *_ctype_;
    extern const short *_tolower_tab_;
    extern const short *_toupper_tab_;

    int isalnum(int c);
    int isalpha(int c);
    int isblank(int c);
    int iscntrl(int c);
    int isdigit(int c);
    int isgraph(int c);
    int islower(int c);
    int isprint(int c);
    int ispunct(int c);
    int isspace(int c);
    int isupper(int c);
    int isxdigit(int c);

}
