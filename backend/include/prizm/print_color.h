/* 
Frank Bender
Prizm software testing
*/

#ifndef PRINTCOLOR_H_ 
#define PRINTCOLOR_H_

#include <stdio.h>

void red();
void blue();
void yellow();
void green();
void cyan();
void purple();
void magenta();
void black();
void white();
void grey();
void lred();
void lgreen();
void lyellow();
void lblue();
void lpurple();
void lmagenta();
void lcyan();
void lwhite();

void redbg();
void bluebg();
void yellowbg();
void greenbg();
void cyanbg();
void purplebg();
void magentabg();
void blackbg();
void whitebg();
void greybg();
void lredbg();
void lgreenbg();
void lyellowbg();
void lbluebg();
void lpurplebg();
void lmagentabg();
void lcyanbg();
void lwhitebg();

void bold();
void underscore();
void blink();
void reverse();
void conceal();
void clearcolor();

void br();

#define RED(...) red(); printf(__VA_ARGS__); clearcolor();
#define BLU(...) blue(); printf(__VA_ARGS__); clearcolor();
#define YEL(...) yellow(); printf(__VA_ARGS__); clearcolor();
#define MAG(...) magenta(); printf(__VA_ARGS__); clearcolor();
#define GRE(...) green(); printf(__VA_ARGS__); clearcolor();
#define CYA(...) cyan(); printf(__VA_ARGS__); clearcolor();
#define WHI(...) white(); printf(__VA_ARGS__); clearcolor();
#define GRY(...) grey(); printf(__VA_ARGS__); clearcolor();
#define BLA(...) black(); printf(__VA_ARGS__); clearcolor();

#define BRED(...) red(); bold(); printf(__VA_ARGS__); clearcolor();
#define BBLU(...) blue(); bold(); printf(__VA_ARGS__); clearcolor();
#define BYEL(...) yellow(); bold(); printf(__VA_ARGS__); clearcolor();
#define BMAG(...) magenta(); bold(); printf(__VA_ARGS__); clearcolor();
#define BGRE(...) green(); bold(); printf(__VA_ARGS__); clearcolor();
#define BCYA(...) cyan(); bold(); printf(__VA_ARGS__); clearcolor();
#define BWHI(...) white(); bold(); printf(__VA_ARGS__); clearcolor();
#define BGRY(...) grey(); bold(); printf(__VA_ARGS__); clearcolor();
#define BBLA(...) black(); bold(); printf(__VA_ARGS__); clearcolor();

#endif
