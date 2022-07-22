#include "prizm/print_color.h"

void red() { printf("\033[0;31m"); }
void blue() { printf("\033[0;34m"); }
void yellow() { printf("\033[0;33m"); }
void green() { printf("\033[0;32m"); }
void cyan() { printf("\033[0;36m"); }
void purple() { printf("\033[0;35m"); }
void magenta() { printf("\033[0;35m"); }
void black() { printf("\033[0;30m"); }
void white() { printf("\033[0;37m"); }
void grey() { printf("\033[0;90m"); }
void lred() { printf("\033[0;91m"); }
void lgreen() { printf("\033[0;92m"); }
void lyellow() { printf("\033[0;93m"); }
void lblue() { printf("\033[0;94m"); }
void lpurple() { printf("\033[0;95m"); }
void lmagenta() { printf("\033[0;95m"); }
void lcyan() { printf("\033[0;96m"); }
void lwhite() { printf("\033[0;97m"); }

void redbg() { printf("\033[0;41m"); }
void bluebg() { printf("\033[0;44m"); }
void yellowbg() { printf("\033[0;43m"); }
void greenbg() { printf("\033[0;42m"); }
void cyanbg() { printf("\033[0;46m"); }
void purplebg() { printf("\033[0;45m"); }
void magentabg() { printf("\033[0;45m"); }
void blackbg() { printf("\033[0;40m"); }
void whitebg() { printf("\033[0;47m"); }
void greybg() { printf("\033[0;100m"); }
void lredbg() { printf("\033[0;101m"); }
void lgreenbg() { printf("\033[0;102m"); }
void lyellowbg() { printf("\033[0;103m"); }
void lbluebg() { printf("\033[0;104m"); }
void lpurplebg() { printf("\033[0;105m"); }
void lmagentabg() { printf("\033[0;105m"); }
void lcyanbg() { printf("\033[0;106m"); }
void lwhitebg() { printf("\033[0;107m"); }

void bold() { printf("\033[1m"); }
void underscore() { printf("\033[4m"); }
void blink() { printf("\033[5m"); }
void reverse() { printf("\033[7m"); }
void conceal() { printf("\033[8m"); }
void clearcolor() { printf("\033[0m"); }

void br() { printf("\n"); }