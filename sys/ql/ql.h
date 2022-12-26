#ifndef _H_QL
#define _H_QL
#include"inttypes.h"
#define SYM_MAX 10

enum
{
	q_nul,
	q_ifloop,
	q_ifjudge,
	q_ifing,
	q_ifforelse,
	ql_number,
	ql_string,
	ql_func,
	ql_builtin,
	ql_ptr,
	ql_list
};
typedef struct ql_value
{
	union
	{
		uint32_t addr;
		struct ql_value* obj;
	};
	char type;
	char ref : 1;
	uint32_t gc;
}ql_res;

typedef struct
{
	char name[SYM_MAX + 1];
	ql_res obj;
}ql_sym;
typedef struct
{
	char* mem;
	int memlen;
	ql_sym* sym_list;
	int sym_list_len;
	char* func_list;
	int func_list_len;
	int func_used;
	ql_res last_res;
	ql_res* last_ref;
	uint32_t gc_mark;
	char if_state;
	char loop_state;
}ql_ctx;

void ql_gc_static(ql_ctx* ctx);
void ql_init(ql_ctx* ctx, char* mem,int memlen);
void ql_add_builtin(ql_ctx* ctx, char* func_name, int(*callback)(ql_ctx* ctx, char** codes));
int ql_eval(ql_ctx* ctx, char** ocodes,int depth);
#endif