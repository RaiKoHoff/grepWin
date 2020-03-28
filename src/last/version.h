#pragma once

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Info : "

//#pragma message(__LOC__"Run the NAnt script to get proper version info")

#define FILEVER         1, 9, 2, 610
#define PRODUCTVER      1, 9, 2, 610
#define STRFILEVER      "1.9.2.609\0"
#define STRPRODUCTVER   "1.9.2.609\0"

#define GREPWIN_VERMAJOR     1
#define GREPWIN_VERMINOR     9
#define GREPWIN_VERMICRO     2
#define GREPWIN_VERBUILD     610
#define GREPWIN_VERDATE      "2020-03-28"
