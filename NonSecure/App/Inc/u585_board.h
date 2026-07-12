#ifndef U585_BOARD_H
#define U585_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#ifndef U585_LED_RED_PORT
#define U585_LED_RED_PORT GPIOH
#endif

#ifndef U585_LED_RED_PIN
#define U585_LED_RED_PIN GPIO_PIN_6
#endif

#ifndef U585_LED_GREEN_PORT
#define U585_LED_GREEN_PORT GPIOH
#endif

#ifndef U585_LED_GREEN_PIN
#define U585_LED_GREEN_PIN GPIO_PIN_7
#endif

#ifndef U585_USER_BUTTON_PORT
#define U585_USER_BUTTON_PORT GPIOC
#endif

#ifndef U585_USER_BUTTON_PIN
#define U585_USER_BUTTON_PIN GPIO_PIN_13
#endif

#ifndef U585_BUTTON_PRESSED_STATE
#define U585_BUTTON_PRESSED_STATE GPIO_PIN_SET
#endif

void U585_Board_InitBasicGpio(void);
void U585_Board_SetRedLed(GPIO_PinState state);
void U585_Board_SetGreenLed(GPIO_PinState state);
void U585_Board_ToggleRedLed(void);
void U585_Board_ToggleGreenLed(void);
GPIO_PinState U585_Board_ReadUserButton(void);
uint8_t U585_Board_IsUserButtonPressed(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_BOARD_H */
