#ifndef U585_APP_H
#define U585_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u585_demo.h"

void U585_App_Init(void);
void U585_App_Loop(void);
const U585_Demo *U585_App_CurrentDemo(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_APP_H */
