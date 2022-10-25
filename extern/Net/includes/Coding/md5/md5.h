#pragma once

#define MD5_SIZE	16
typedef unsigned int		md5_uint32;
#define MAX_MD5_UINT32		((md5_uint32)4294967295U)
#define MD5_BLOCK_SIZE		64

typedef struct
{
	md5_uint32	md_A;			/* accumulater 1 */
	md5_uint32	md_B;			/* accumulater 2 */
	md5_uint32	md_C;			/* accumulater 3 */
	md5_uint32	md_D;			/* accumulater 4 */

	md5_uint32	md_total[2];		/* totaling storage */
	md5_uint32	md_buf_len;		/* length of the storage buffer */
	char		md_buffer[MD5_BLOCK_SIZE * 2];	/* character storage buffer */
} md5_t;

extern void	md5_init(md5_t* md5_p);
extern void md5_process(md5_t*, const void*, const unsigned int);
extern void md5_finish(md5_t*, void*);
extern void md5_buffer(const char*, const unsigned int, void*);
extern void md5_sig_to_string(void*, char*, const int);
extern void md5_sig_from_string(void*, const char*);