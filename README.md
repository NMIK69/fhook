## fhook
This is a C library for Linux that allows you to hook functions. Currently, it
supports only x86\_64 architectures.

### TODOs
- Add compatibility with x86\_32.

### Dependencies
- None

## Usage
The library provides 5 functions and 1 macro:

```c

#define FHOOK_GET_TFP(var, h)

void *fhook_create(void *target, void *hook);
void fhook_free(void *h);

void fhook_unhook(void *h);
void fhook_rehook(void *h);

void *fhook_get_tfp(void *h);
```

The function ```fhook_create``` takes a pointer to the target function and a pointer
to the hook function. It then creates the hook. On success, it returns a valid
void pointer handle associated with the hooked function. On failure, it returns
```NULL```.

To remove the hook and also free the resources, you can call the ```fhook_free``` function.
If you only want to unhook a function, use ```fhook_unhook```. To rehook a previously
unhooked function, use ```fhook_rehook```. Both functions expect a void pointer handle
that is returned by fhook_create.

The ```FHOOK_GET_TFP``` macro can be used to obtain a function pointer that
allows you to call the target function while the hook is in place, without
invoking the hook. The function pointer will be assigned to ```var```. The
```h``` parameter should be a handle returned by fhook_create. Under the hood,
the macro calls the ```fhook_get_tfp``` function and handles the conversion (from void
pointer to function pointer) to avoid compiler warnings.

> [!Warning] 
> Using the function pointer returned by ```FHOOK_GET_TFP``` or
> ```fhook_get_tfp``` only works if the target function begins with the
> following instructions:
>
> ```asm
> endbr64 
> push   rbp
> mov    rbp,rsp
> sub    rsp,0x...
> ```

## Example
An example demonstrating the usage of the library can be found in the example
folder. To run the example, navigate to the folder. Build it with make, and then
run it by executing ./example.
