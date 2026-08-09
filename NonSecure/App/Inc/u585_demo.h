#ifndef U585_DEMO_H
#define U585_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  const char *id;
  const char *title;
  void (*init)(void);
  void (*loop)(void);
} U585_Demo;

extern const U585_Demo U585_Demo_Exp03LedButton;
extern const U585_Demo U585_Demo_Exp04DebugFault;
extern const U585_Demo U585_Demo_Exp05ClockTree;
extern const U585_Demo U585_Demo_Exp06;
extern const U585_Demo U585_Demo_Exp07;
extern const U585_Demo U585_Demo_Exp08;
extern const U585_Demo U585_Demo_Exp09;
extern const U585_Demo U585_Demo_Exp10;
extern const U585_Demo U585_Demo_Exp11;
extern const U585_Demo U585_Demo_Exp12;
extern const U585_Demo U585_Demo_Exp13;
extern const U585_Demo U585_Demo_Exp14;
extern const U585_Demo U585_Demo_Exp15;
extern const U585_Demo U585_Demo_Exp16;
extern const U585_Demo U585_Demo_Exp17;
extern const U585_Demo U585_Demo_Exp18;
extern const U585_Demo U585_Demo_Exp19;
extern const U585_Demo U585_Demo_Exp20;
extern const U585_Demo U585_Demo_Exp21;
extern const U585_Demo U585_Demo_Exp22;
extern const U585_Demo U585_Demo_Exp23;
extern const U585_Demo U585_Demo_Exp24;
extern const U585_Demo U585_Demo_Exp25;
extern const U585_Demo U585_Demo_Exp26;
extern const U585_Demo U585_Demo_Exp27;
extern const U585_Demo U585_Demo_Exp28;
extern const U585_Demo U585_Demo_Exp29;
extern const U585_Demo U585_Demo_Exp30;
extern const U585_Demo U585_Demo_Exp31;
extern const U585_Demo U585_Demo_Exp32;
extern const U585_Demo U585_Demo_Exp33;
extern const U585_Demo U585_Demo_Unsupported;

#ifdef __cplusplus
}
#endif

#endif /* U585_DEMO_H */
