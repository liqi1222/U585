#include "u585_fault.h"

#include "main.h"

static void u585_fault_uart_putc(char ch)
{
  while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0U)
  {
  }
  USART1->TDR = (uint16_t)ch;
}

static void u585_fault_uart_puts(const char *text)
{
  if (text == NULL)
  {
    return;
  }

  while (*text != '\0')
  {
    u585_fault_uart_putc(*text);
    text++;
  }
}

static void u585_fault_uart_put_hex32(uint32_t value)
{
  static const char hex[] = "0123456789ABCDEF";
  char buffer[11];
  uint32_t index = 0U;

  u585_fault_uart_puts("0x");
  if (value == 0U)
  {
    u585_fault_uart_putc('0');
    return;
  }

  while (value > 0U)
  {
    buffer[index++] = hex[value & 0xFU];
    value >>= 4U;
  }

  while (index > 0U)
  {
    index--;
    u585_fault_uart_putc(buffer[index]);
  }
}

void U585_Fault_EnableConfigurableFaults(void)
{
  SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk |
                 SCB_SHCSR_USGFAULTENA_Msk |
                 SCB_SHCSR_MEMFAULTENA_Msk);
}

void U585_Fault_PrintAndHalt(const char *label, uint32_t *stack_ptr)
{
  uint32_t exc_return;

  __asm volatile("mov %0, lr" : "=r"(exc_return));

  u585_fault_uart_puts("\r\n[NS] [U585][FAULT] ");
  u585_fault_uart_puts(label);
  u585_fault_uart_puts("\r\n");

  if (stack_ptr != NULL)
  {
    u585_fault_uart_puts("[NS] [U585][FAULT] R0="); u585_fault_uart_put_hex32(stack_ptr[0]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] R1="); u585_fault_uart_put_hex32(stack_ptr[1]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] R2="); u585_fault_uart_put_hex32(stack_ptr[2]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] R3="); u585_fault_uart_put_hex32(stack_ptr[3]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] R12="); u585_fault_uart_put_hex32(stack_ptr[4]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] LR="); u585_fault_uart_put_hex32(stack_ptr[5]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] PC="); u585_fault_uart_put_hex32(stack_ptr[6]); u585_fault_uart_puts("\r\n");
    u585_fault_uart_puts("[NS] [U585][FAULT] xPSR="); u585_fault_uart_put_hex32(stack_ptr[7]); u585_fault_uart_puts("\r\n");
  }

  u585_fault_uart_puts("[NS] [U585][FAULT] EXC_RETURN="); u585_fault_uart_put_hex32(exc_return); u585_fault_uart_puts("\r\n");
  u585_fault_uart_puts("[NS] [U585][FAULT] CFSR="); u585_fault_uart_put_hex32(SCB->CFSR); u585_fault_uart_puts("\r\n");
  u585_fault_uart_puts("[NS] [U585][FAULT] HFSR="); u585_fault_uart_put_hex32(SCB->HFSR); u585_fault_uart_puts("\r\n");
  u585_fault_uart_puts("[NS] [U585][FAULT] BFAR="); u585_fault_uart_put_hex32(SCB->BFAR); u585_fault_uart_puts("\r\n");
  u585_fault_uart_puts("[NS] [U585][FAULT] MMFAR="); u585_fault_uart_put_hex32(SCB->MMFAR); u585_fault_uart_puts("\r\n");
  u585_fault_uart_puts("[NS] [U585][FAULT] halted\r\n");

  while (1)
  {
  }
}
