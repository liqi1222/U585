#ifndef U585_BOARD_H
#define U585_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Prefer CubeMX-generated pin defines from main.h so the .ioc stays the
 * single source of truth; fall back to the measured board mapping
 * (B-U585I-IOT02A: red LED=PH6, green LED=PH7, user button=PC13). */
#ifdef LED_RED_GPIO_Port
#define U585_LED_RED_PORT LED_RED_GPIO_Port
#define U585_LED_RED_PIN LED_RED_Pin
#else
#define U585_LED_RED_PORT GPIOH
#define U585_LED_RED_PIN GPIO_PIN_6
#endif

#ifdef LED_GREEN_GPIO_Port
#define U585_LED_GREEN_PORT LED_GREEN_GPIO_Port
#define U585_LED_GREEN_PIN LED_GREEN_Pin
#else
#define U585_LED_GREEN_PORT GPIOH
#define U585_LED_GREEN_PIN GPIO_PIN_7
#endif

#ifdef USER_Button_GPIO_Port
#define U585_USER_BUTTON_PORT USER_Button_GPIO_Port
#define U585_USER_BUTTON_PIN USER_Button_Pin
#else
#define U585_USER_BUTTON_PORT GPIOC
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
