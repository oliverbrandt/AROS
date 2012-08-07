
#ifndef PROTO_AMISSL_H
#include <proto/amissl.h>
#endif /* PROTO_AMISSL_H */

#ifndef HEADER_COMP_H
#define HEADER_COMP_H

#include <openssl/crypto.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct comp_method_st
	{
	int type;		/* NID for compression library */
	const char *name;	/* A text string to identify the library */
	int (*init)();
	void (*finish)();
	int (*compress)();
	int (*expand)();
	long (*ctrl)();
	long (*callback_ctrl)();
	} COMP_METHOD;

typedef struct comp_ctx_st
	{
	COMP_METHOD *meth;
	unsigned long compress_in;
	unsigned long compress_out;
	unsigned long expand_in;
	unsigned long expand_out;

	CRYPTO_EX_DATA	ex_data;
	} COMP_CTX;


COMP_CTX *COMP_CTX_new(COMP_METHOD *meth);
void COMP_CTX_free(COMP_CTX *ctx);
int COMP_compress_block(COMP_CTX *ctx, unsigned char *out, int olen,
	unsigned char *in, int ilen);
int COMP_expand_block(COMP_CTX *ctx, unsigned char *out, int olen,
	unsigned char *in, int ilen);
COMP_METHOD *COMP_rle(void );
COMP_METHOD *COMP_zlib(void );

/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */
void ERR_load_COMP_strings(void);

/* Error codes for the COMP functions. */

/* Function codes. */

/* Reason codes. */

#ifdef  __cplusplus
}
#endif
#endif
