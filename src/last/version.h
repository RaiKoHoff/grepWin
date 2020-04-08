#pragma once

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Info : "

//#pragma message(__LOC__"Run the NAnt script to get proper version info")

#define FILEVER         2, 0, 0, 1
#define PRODUCTVER      2, 0, 0, 1
#define STRFILEVER      "2.0.0.1\0"
#define STRPRODUCTVER   "2.0.0.1\0"

#define GREPWIN_VERMAJOR     2
#define GREPWIN_VERMINOR     0
#define GREPWIN_VERMICRO     0
#define GREPWIN_VERBUILD     1
#define GREPWIN_VERDATE      "2020-04-08"
