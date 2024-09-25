#ifndef FHOOK_H
#define FHOOK_H

#include <stdint.h>


/* To avoid compiler warning. Manual page dlopen(3) example section shows how to convert
 * from void poninter (or object pointer) to function pointer). */
#define FHOOK_GET_TFP(var, h)\
	*(void **)(&var) = fhook_get_tfp(h);

void *fhook_create(void *target, void *hook);
void fhook_free(void *h);

void fhook_unhook(void *h);
void fhook_rehook(void *h);

void *fhook_get_tfp(void *h);

#endif
