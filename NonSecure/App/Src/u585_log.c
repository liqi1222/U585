#include "u585_log.h"

#include <stddef.h>

#include "u585_usart1.h"
#include "secure_nsc.h"

#define U585_LOG_NS_TAG "[NS] "

static void u585_log_write_decimal(uint32_t value)
{
  char buffer[11];
  uint32_t index = 0U;

  if (value == 0U)
  {
    U585_Log_WriteString("0");
    return;
  }

  while ((value > 0U) && (index < sizeof(buffer)))
  {
    buffer[index] = (char)('0' + (value % 10U));
    value /= 10U;
    index++;
  }

  while (index > 0U)
  {
    char digit[2];
    index--;
    digit[0] = buffer[index];
    digit[1] = '\0';
    U585_Log_WriteString(digit);
  }
}

void U585_Log_WriteString(const char *text)
{
  const char *cursor = text;
  uint16_t length = 0U;

  if (cursor == NULL)
  {
    return;
  }

  while ((cursor[length] != '\0') && (length < 256U))
  {
    length++;
  }

  if (length == 0U)
  {
    return;
  }

  if (U585_USART1_IsReady() != 0U)
  {
    (void)HAL_UART_Transmit(&huart1_ns, (uint8_t *)text, length, 100U);
  }
  else
  {
    SECURE_UART1_WriteString(text);
  }
}

void U585_Log_WriteLine(const char *text)
{
  if ((text != NULL) && (text[0] != '\0'))
  {
    U585_Log_WriteString(U585_LOG_NS_TAG);
  }
  U585_Log_WriteString(text);
  U585_Log_WriteString("\r\n");
}

void U585_Log_WriteU32(const char *prefix, uint32_t value)
{
  U585_Log_WriteString(U585_LOG_NS_TAG);
  U585_Log_WriteString(prefix);
  u585_log_write_decimal(value);
  U585_Log_WriteString("\r\n");
}
