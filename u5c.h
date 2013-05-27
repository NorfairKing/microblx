/* 
 * micro-5c: a 5C compliant distributed function blocks framework in
 * pure C
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>

#include <uthash.h>

#include "u5c_types.h"


/* module init, cleanup */
#define fblock_init(initfn) \
int __initialize_module(void) { return initfn(); }

#define fblock_cleanup(exitfn) \
void __cleanup_module(void) { exitfn(); }


/* normally the user would have to box/unbox his value himself. This
 * would generate a typed, automatic boxing version for
 * convenience. */
#define gen_write(function_name, typename) \
void function_name(u5c_port_t* port, typename *outval) \
{ \
 u5c_data_t val; \
 val.data = outval; \
 val.type = port->data_type_id; \
 __port_write(port, &val);	\
} \

/* generate a typed read function: arguments to the function are the
 * port and a pointer to the result value. 
 */
#define gen_read(function_name, typename) \
uint32_t function_name(u5c_port_t* port, typename *inval) \
{ \
 u5c_data_t val; 		\
 val.data = inval;	  	\
 return __port_read(port, &val);	\
} \

/*
 * Debug stuff
 */

#ifdef DEBUG
# define DBG(fmt, args...) ( fprintf(stderr, "%s: ", __FUNCTION__), \
			     fprintf(stderr, fmt, ##args),	    \
			     fprintf(stderr, "\n") )
#else
# define DBG(fmt, args...)  do {} while (0)
#endif

#define ERR(fmt, args...) ( fprintf(stderr, "%s: ", __FUNCTION__),	\
			    fprintf(stderr, fmt, ##args),		\
			    fprintf(stderr, "\n") )

/* same as ERR but errnum */
#define ERR2(err_num, fmt, args...) ( fprintf(stderr, "%s: ", __FUNCTION__), \
				      fprintf(stderr, fmt, ##args), \
				      fprintf(stderr, ": %s", strerror(err_num)), \
				      fprintf(stderr, "\n") )

