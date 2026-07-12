#include "u585_log.h"

#include <stddef.h>

#include "secure_nsc.h"

/* Tag identifying the non-secure world on the shared secure-side UART. */
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
  SECURE_UART1_WriteString(text);
}

void U585_Log_WriteLine(const char *text)
{
  /* Tag the line so the secure-side UART output can be attributed to the
   * non-secure world. An empty/blank line is emitted without the tag to
   * keep visual separators clean. */
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
