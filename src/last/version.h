#pragma once

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Info : "

//#pragma message(__LOC__"Run the NAnt script to get proper version info")

#define FILEVER         2, 1, 2, 16
#define PRODUCTVER      2, 1, 2, 16
#define STRFILEVER      "2.1.2.16\0"
#define STRPRODUCTVER   "2.1.2.16\0"

#define GREPWIN_VERMAJOR     2
#define GREPWIN_VERMINOR     1
#define GREPWIN_VERMICRO     2
#define GREPWIN_VERBUILD     16
#define GREPWIN_VERDATE      "2020-07-02"
