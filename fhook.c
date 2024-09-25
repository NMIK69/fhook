#define _GNU_SOURCE

#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "fhook.h"


#define DETOUR_NBYTES 12
#define GATEWAY_SIZE (DETOUR_NBYTES * 2)

/* TODO:
	- Track all fhook_create calls to be able to restore memory access
	  rights in fhook_free without removing it from other hooks on the same
	  page.
*/

struct fhook_ctx
{
	void *src;
	void *dst;

	void *orig_func;
	uint8_t orig[DETOUR_NBYTES];
	uint8_t hook[DETOUR_NBYTES];
	uint8_t gateway[GATEWAY_SIZE];
};

static int gateway_create(struct fhook_ctx *ctx);

static int hook_init(struct fhook_ctx *ctx);
static void hook_insert(struct fhook_ctx *ctx);
static void hook_remove(struct fhook_ctx *ctx);

static int set_mem_access(void *vaddr, int prot);


void *fhook_create(void *target, void *hook)
{
	/* function pointers must be representable as void pointers. */
	assert(sizeof(void *(*)(void)) <= sizeof(void *));


	struct fhook_ctx *ctx = malloc(sizeof(*ctx));
	if(ctx == NULL) {
		perror("Failed to allocate fhook context\n");
		errno = 0;
		return NULL;
	}
	ctx->src = target;
	ctx->dst = hook;

	/* get the original bytes before overwriting them. */
	memcpy(ctx->orig, ctx->src, DETOUR_NBYTES);
	
	int ret = hook_init(ctx);
	if(ret != 0) {
		free(ctx);
		return NULL;
	}

	/* overwrite orig bytes with detour to hook. */
	hook_insert(ctx);
	
	/* create a gateway. */
	ret = gateway_create(ctx);
	if(ret != 0) {
		free(ctx);
		return NULL;
	}

	*(void **)(&(ctx->orig_func)) = ctx->gateway;

	return (void *)ctx;
}


void fhook_free(void *h)
{
	struct fhook_ctx *ctx = (struct fhook_ctx *)h;	
	if(ctx == NULL)
		return;

	hook_remove(ctx);
	
	free(ctx);
}

void fhook_unhook(void *h)
{
	struct fhook_ctx *ctx = (struct fhook_ctx *)h;
	hook_remove(ctx);
}

void fhook_rehook(void *h)
{
	struct fhook_ctx *ctx = (struct fhook_ctx *)h;
	hook_insert(ctx);
}

void *fhook_get_tfp(void *h)
{
	struct fhook_ctx *ctx = (struct fhook_ctx *)h;	
	return ctx->orig_func;	
}

static int hook_init(struct fhook_ctx *ctx)
{
	/* fill the detour array */	
	size_t i = 0;

	/* movabs rax */
	ctx->hook[i++] = 0x48;
	ctx->hook[i++] = 0xB8;

	/* addr (into rax) */
	memcpy(&(ctx->hook[i]), &(ctx->dst), sizeof(ctx->dst));
	i += sizeof(ctx->dst);

	/* jmp rax */
	ctx->hook[i++] = 0xFF;
	ctx->hook[i++] = 0xE0;
	
	/* make memory page of where the target function is writeable */
	int ret = set_mem_access(ctx->src, PROT_READ | PROT_WRITE | PROT_EXEC);
	if(ret != 0) 
		return -1;

	hook_insert(ctx);

	return 0;
}

static void hook_insert(struct fhook_ctx *ctx)
{
	/* replace first bytes with our detour to the hook. */
	memcpy(ctx->src, ctx->hook, DETOUR_NBYTES);	
}

static void hook_remove(struct fhook_ctx *ctx)
{
	/* restore the replaced bytes in the original function. */
	memcpy(ctx->src, ctx->orig, DETOUR_NBYTES);	
}

static int gateway_create(struct fhook_ctx *ctx)
{
	size_t i = 0;
	while(i < DETOUR_NBYTES) {
		ctx->gateway[i] = ctx->orig[i];
		i += 1;
	}

	/* movabs rax */
	ctx->gateway[i++] = 0x48;
	ctx->gateway[i++] = 0xB8;
	
	/* addr (into rax) */
	void *gate_addr = (void *)((uintptr_t)ctx->src + DETOUR_NBYTES);
	memcpy(&(ctx->gateway[i]), &gate_addr, sizeof(gate_addr));	
	i += sizeof(gate_addr);
	
	/* jmp rax */
	ctx->gateway[i++] = 0xFF;
	ctx->gateway[i] = 0xE0;

	/* make gateway executable */
	int ret = set_mem_access(ctx->gateway, PROT_READ | PROT_WRITE | PROT_EXEC);
	if(ret != 0)
		return -1;

	return 0;
}


static int set_mem_access(void *vaddr, int prot)
{
	/* align virtual address to the immediate previous page boundary
	 * source: https://stackoverflow.com/q/22970621 */

	long page_size = sysconf(_SC_PAGESIZE);
	void *prv_page = (void *)(((uintptr_t)vaddr) & ~(page_size - 1));

	int ret = mprotect(prv_page, page_size, prot);
	if(ret < 0) {
		perror("mprotect failed");
		errno = 0;
		return -1;
	}

	return 0;
}
