#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "../fhook.h"

#define UNUSED_VAR(var) (void)(var)


static int hook_func(int a, int b);
static int target_func(int a, int b);
static int (*orig_target_func)(int a, int b);

void *h_target_func;


int main(void)
{
	printf("[*] No hook in place\n");
	int res = target_func(1, 2);
	printf("res: %d\n", res);

	void *hook_func_ptr = (void *)((uintptr_t)hook_func);
	void *target_func_ptr = (void *)((uintptr_t)target_func);

	h_target_func = fhook_create(target_func_ptr, hook_func_ptr);
	if(h_target_func == NULL) {
		fprintf(stderr, "[!] Failed to create hook.\n");
		return 0;
	}
	
	UNUSED_VAR(orig_target_func);
	/* only nessesary if approach 1 is used. */
	//FHOOK_GET_TFP(orig_target_func, h_target_func);

	printf("[*] Hook placed\n");
	res = target_func(1, 2);
	printf("res: %d\n", res);

	fhook_free(h_target_func);	

	printf("[*] Hook removed\n");
	res = target_func(1, 2);
	printf("res: %d\n", res);


	return 0;
}

static int target_func(int a, int b)
{
	if(a > b) {
		printf("%d is bigger then %d\n", a, b);
		return a;
	}
	else {
		printf("%d is smaler then %d\n", a, b);
		return b;
	}

	return 0;
}

static int hook_func(int a, int b)
{
	UNUSED_VAR(a);
	UNUSED_VAR(b);

	printf("[-] Inside Hook\n");

	/* approach 1: */
	/* call original function with original parameters. */
	//int orig_res = orig_target_func(a, b);	

	/* manipulate result. */
	//int hook_res = orig_res == a ? b : a; 

	/* approach 2: */
	fhook_unhook(h_target_func);

	/* call hooked function with different parameters. */
	int orig_res = target_func(69, 420);
	/* manipulate result. */
	int hook_res =  orig_res + 42;

	fhook_rehook(h_target_func);


	return hook_res;
}

