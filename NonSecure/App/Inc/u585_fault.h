#ifndef U585_FAULT_H
#define U585_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void U585_Fault_EnableConfigurableFaults(void);
void U585_Fault_PrintAndHalt(const char *label, uint32_t *stack_ptr);

#ifdef __cplusplus
}
#endif

#endif /* U585_FAULT_H */
