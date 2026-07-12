#ifndef U585_I2C2_H
#define U585_I2C2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern I2C_HandleTypeDef hi2c2_ns;

HAL_StatusTypeDef U585_I2C2_Init(void);
uint8_t U585_I2C2_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_I2C2_H */
