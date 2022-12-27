#include"ql.h"
#include"string.h"
#include"stdio.h"
//#define QL_DEBUG
int pow10(int n)
{
	int sum = 10;
	int m;
	if (n == 0) return 1;
	for (m = 1; m < n; m++) sum *= 10;
	return sum;
}
int toint(char* str, int len,char*err)
{
	int r = 0;
	*err = 0;
	for (int i = len-1; i >=0; i--)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			r += pow10(len - i - 1) * (str[i] - '0');
		}
		else if (str[i] == '-')
		{
			r = -r;
		}
		else 
		{
			*err = 1;
			return 0;
		}
	}
	return r;
}
char* trim(char* str, char ch)
{
	char* r = str;
	while (*str)
	{
		if (*str == ch)
		{
			r = str+1;
		}
		else break;
		str++;
	}
	return r;
}
char ql_isalpha(char ch)
{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}
char ql_isnum(char ch)
{
	return (ch >= '0' && ch <= '9');
}
char* forwardbrk(char* str)
{
	int i = 0;
	while (*str)
	{
		if (*str == '(')i++;
		else if (*str == ')')
		{
			if (i == 1)return str + 1;
			if (i > 0)i--;
			else return 0;
		}
		str++;
	}
	return 0;
}
int forward(char* str)
{
	int i = 0;
	while (*str)
	{
		if (!ql_isalpha(*str) && !ql_isnum(*str)&&*str!='-'&&*str!='_')
		{
			if (i != 0)return i;
			else return 1;
		}
		if (*str==' ')
		{
			return i;
		}
		else i++;
		str++;
	}
	return i;
}
#define ql_err printf
int ql_builtin_print(ql_ctx* ctx, char** ocode);
int ql_builtin_array(ql_ctx* ctx, char** ocodes);
int ql_eval(ql_ctx* ctx, char** ocodes, int depth);
void ql_init(ql_ctx* ctx, char* mem,int memlen)
{
	memset(mem, 0, memlen);
	ctx->mem = mem;
	ctx->memlen = memlen;
	ctx->sym_list = ctx->mem;
	ctx->sym_list_len = (memlen/3) / sizeof(ql_sym);
	ctx->func_list = ctx->mem + ctx->sym_list_len*sizeof(ql_sym);
	ctx->func_list_len = ctx->memlen - ctx->sym_list_len;
	ctx->func_used = 0;
	ctx->gc_mark = 0;
	ctx->if_state = 0;
	ctx->loop_state = 0;
	ql_add_builtin(ctx, "print", ql_builtin_print);
	ql_add_builtin(ctx, "array", ql_builtin_array);
}
#define ql_quit(o,n)*o=n;return 1;
#define ql_conftype(v1,v2)if(v1.type!=v2.type){ql_err("conflip type operation!\n");return -1;}
#define ql_chk(r) if(r<0){return -1;}
#define ql_chkmsg(r,msg) if(r<0){ql_err(msg);return -1;}
#define ql_chkzero(r,msg)ql_chkmsg((int)(r)-1,msg)
#define ql_reqnext(r,codes) if(r<1){ql_err("require words after '%c%c'\n", *(codes-1),*codes);return -1;}
#define ql_ctrim(codes,len)codes+=len;codes=trim(codes,' ');
#define ql_arg(ctx,codes,name)ql_chkmsg(ql_eval(ctx, codes,1),"Expect args!\n");ql_res name = ctx->last_res;
#define ql_mapsym(ctx,name,val)if(ql_addsym(ctx,name,val,0)<0){ql_err("cannot map sym!\n ");return -1;}

void ql_add_builtin(ql_ctx* ctx, char* func_name, int(*callback)(ql_ctx* ctx, char** codes))
{
	ql_res res;
	res.addr = callback;
	res.gc = 0;
	res.type = ql_builtin;
	ql_mapsym(ctx, func_name, res);
}
int ql_findsym(ql_ctx* ctx, char* name, ql_res* value)
{
	if (!ql_isalpha(name[0]))return -1;
	for (int i = 0; i <ctx->sym_list_len ; i++)
	{
		if (!strcmp(ctx->sym_list[i].name, name))
		{
			*value = ctx->sym_list[i].obj;
			return i;
		}
	}
	return -1;
}
int ql_setsym_quot(ql_ctx* ctx, char* name, ql_res value)
{
	ql_res v2;
	int idx = ql_findsym(ctx, name, &v2);
	if (idx >= 0)
	{
		if (ctx->sym_list[idx].obj.type == ql_ptr)
		{
			*ctx->sym_list[idx].obj.obj = value;
			return 1;
		}
		return -1;
	}
	return -1;
}
int ql_addsym(ql_ctx*ctx, char* name, ql_res value,uint32_t gc)
{
	if (!gc)
	{
		ql_res v2;
		int idx = ql_findsym(ctx, name, &v2);
		if (idx >= 0)
		{
			ctx->sym_list[idx].obj = value;
			return idx;
		}
	}
	for (int i = ctx->sym_list_len-1; i >=0 ; i--)
	{
		if (ctx->sym_list[i].name[0]=='\0')
		{
			strcpy(ctx->sym_list[i].name,name);
			ctx->sym_list[i].obj = value;
			return i;
		}
	}
	return -1;
}

char* ql_addfun(ql_ctx* ctx, char* codes,uint32_t *str)
{
	char*n= forwardbrk(codes);
	if (!n)return 0;
	n = forwardbrk(n);
	if (!n)return 0;
	*str = ctx->func_list;
	memcpy(ctx->func_list, codes, n - codes);
	ctx->func_list += n - codes+1;
	*ctx->func_list = '\0';
	//ctx->func_list++;
	return codes + (n - codes);
}

char* ql_readstr(char *str,char ch,int *len)
{
	char* old = str;
	char* r = old;
	while (*str!=ch)
	{
		if (*str == '\\')
		{
			str++;
		}
		*old = *str;
		old++;
		str++;
	}
	*len = old - r;
	return str+1;
}
void ql_gc_string(ql_ctx*ctx, ql_sym *o)
{
	int l = strlen(o->obj.addr);
	if (l)
	{
		
		memcpy(o->obj.addr, o->obj.addr + l + 1, ctx->func_list_len - (l + 1));
		memset(ctx->func_list, 0, l + 1);
		ctx->func_list -= (l + 1);
	}
	o->name[0] = '\0';
}
void ql_gc_static(ql_ctx* ctx)
{
	int num = 0;
	for (int i = 0; i < ctx->sym_list_len; i++)
	{
		ql_sym o = ctx->sym_list[i];
		if (o.name[0] == '\0')continue;
		if (o.obj.type == ql_string&&o.name[0]=='@')
		{
			//static string must appear with a binded variable
			o = ctx->sym_list[i - 1];
			if (o.name[0]!='\0'&&o.name[0]!='@')
			{
				if (o.obj.type == ql_string)
				{
					continue;
				}
			}
			num++;
			ql_gc_string(ctx, &ctx->sym_list[i]);
		}
		else if (o.name[0] != '\0' && !ql_isalpha(o.name[0])&&o.obj.ref==0)
		{
			ctx->sym_list[i].name[0] = '\0';
			num++;
		}
	}
#ifdef QL_DEBUG
	ql_err("[DEBUG]GC Total %d resources!\n", num);
#endif // _QL_DEBUG
}
void ql_gc(ql_ctx* ctx,uint32_t mark)
{
	for (int i = 0; i <ctx->sym_list_len ; i++)
	{
		if (!ql_isalpha(ctx->sym_list[i].name[0]))continue;
		if (ctx->sym_list[i].obj.gc == mark)ctx->sym_list[i].name[0] = '\0';
		else break;
	}
}
char* ql_fn_arg(ql_ctx* ctx, char* codes,char *given,uint32_t *next,int *cnt)
{
	uint32_t marks = codes;
	while (*codes)
	{
		if (*codes == ')')
		{
			*next = codes + 1;
			return given;
		}
		else if (*codes != '('&& *codes!=' ')
		{
			if (ql_eval(ctx, &given,0) < 0)return 0;
			codes= trim(codes, ' ');
			int l = forward(codes);
			char old = *(codes + l);
			*(codes + l) = '\0';
			char* vname = codes;
			ql_res r=ctx->last_res;
			r.gc = marks;
			ql_addsym(ctx, vname, r, 1);
			*(vname + l) = old;
			ql_ctrim(codes, l-1);
			(*cnt)++;
		}
		codes++;
	}
	return 0;
}

int ql_builtin_array(ql_ctx* ctx, char** ocodes)
{
	char* codes = *ocodes;
	int t = -1;
	int root_idx = -1;
	int num = 0;
	while (1)
	{
		codes = trim(codes,' ');
		if (!*codes)break;
		int r = ql_eval(ctx, &codes, 0);
		if (r == 2)
		{
			break;
		}
		else if (r < 0)return -1;
		else
		{
			ql_res res = ctx->last_res;
			if (t == -1)t = res.type;
			if (res.type != t)
			{
				ql_chkmsg(-1, "Types in an array should be the same!\n");
			}
			res.gc = ctx->gc_mark;
			int idx= ql_addsym(ctx, "#", res, 1);//start with an alpha means it can be automatically GC
			if (root_idx < 0)root_idx = idx;
			num++;
		}
	}
	ql_res res;
	res.gc = ctx->gc_mark;
	res.type = ql_list;
	short* info = &res.addr;
	info[0] = root_idx;
	info[1] = num;
	int idx=ql_addsym(ctx, "#", res, 0);
	ctx->last_res.addr = &ctx->sym_list[idx].obj;
	ctx->last_res.type = ql_ptr;
	ql_quit(ocodes, codes);
}
int ql_builtin_print(ql_ctx* ctx, char** ocode)
{
	char* codes = *ocode;
	ql_arg(ctx, &codes, v1);
	if (v1.type == ql_string)
	{
		ql_err("%s\n", v1.addr);
	}
	else if (v1.type == ql_number)
	{
		ql_err("%d\n", v1.addr);
	}
	else if (v1.type == ql_ptr)
	{
		ql_res v2 = *v1.obj;
		if (v2.type == ql_list)
		{
			printf("(");
			short* info = &v2.addr;
			int root_idx = info[0];
			int nums = info[1];
			for (int i = 0; i < nums; i++)
			{
				printf("%d ", ctx->sym_list[root_idx - i].obj.addr);
			}
			printf(")\n");
		}
	}
	else ql_err("Cannot print this type!\n");
	ql_quit(ocode, codes);
}
uint32_t ql_buildstring(ql_ctx* ctx, char* str, int len)
{

	uint32_t addr = ctx->func_list;
	memcpy(ctx->func_list, str, len);
	ctx->func_list += len + 1;
	*ctx->func_list = '\0';
	//ctx->func_list++;
	return addr;
}
char ql_isvarname(char* codes)
{
	int fst = 0;
	while (*codes)
	{
		if (*codes == ' ')break;
		if (fst == 0 && ql_isnum(*codes))return 0;
		if ((!ql_isalpha(*codes) && !ql_isnum(*codes)) && *codes != '_')return 0;
		codes++;
		fst=1;
	}
	return 1;
}
char* ql_varname(char** codes,char *old,int *l)
{
	int len = forward(*codes);
	if (len == 0)return 0;
	char* vname = *codes;
	*old = *(*codes + len);
	*(*codes + len) = '\0';
	*l = len;
	ql_ctrim(*codes, len);
	return vname;
}
#define ql_restorename(vname,old,len)*(vname+len)=old;
int ql_eval(ql_ctx* ctx, char** ocodes,int depth)
{
	char* codes = *ocodes;
	codes=trim(codes, ' ');
	int len=forward(codes);
	ql_reqnext(len, codes);
	if (*codes == '*')
	{
		codes++;
		if (*codes == ' ')codes--;
		else
		{
			ql_arg(ctx, &codes, v1);
			if (v1.type == ql_ptr)
			{
				ctx->last_res = *v1.obj;
				ql_quit(ocodes, codes);
			}
			else
			{
				ql_err("Expect a pointer after '*'\n");
				return -1;
			}
		}
	}
	else if (*codes == '&')
	{
		codes++;
		if (*codes == ' ')codes--;
		else
		{
			len = forward(codes);
			ql_chkmsg(len, "Expect variable after &!\n")
			char old;
			char* vname = ql_varname(&codes, &old,&len);
			ql_chkzero(vname, "Cannot get variable's name after '&'!")
			ql_res r;
			int idx=ql_findsym(ctx, vname, &r);
			if (idx < 0)
			{
				ql_err("Cannot find variable %s!\n", vname);
			}
			ql_restorename(vname, old, len);
			ctx->last_res.addr = &ctx->sym_list[idx].obj;
			ctx->last_res.type = ql_ptr;
			ql_quit(ocodes, codes);
		}
	}
	if (len==1&&(
		*codes == '+' || *codes == '-' || *codes == '*' || *codes == '/' || *codes == '<' || *codes == '>'||*codes=='&'||*codes=='|'
		))
	{
		char ops = 0;
		if (*codes == '+')ops = 0;
		else if (*codes == '-')ops = 1;
		else if (*codes == '*')ops = 2;
		else if (*codes == '/')ops = 3;
		else if (*codes == '<')
		{
			if (*(codes + 1) == '=')
			{
				ops = 8;
				codes++;
			}
			else ops = 4;
		}
		else if (*codes == '>')
		{
			if (*(codes + 1) == '=')
			{
				ops = 9;
				codes++;
			}
			else ops = 5;
		}
		else if (*codes == '&')ops = 6;
		else if (*codes == '|')ops = 7;
		else
		{
			ql_err("Unknow operation , it should be + - * or /!\n"); return-1;
		}
		ql_ctrim(codes, len);
		ql_arg(ctx, &codes, v1);
		ql_arg(ctx, &codes, v2);
		ql_conftype(v1, v2);
		if (v1.type == ql_number)
		{
			int vv = 0;
			if (ops == 0)vv = (int)v1.addr + (int)v2.addr;
			else if (ops == 1)vv = (int)v1.addr - (int)v2.addr;
			else if (ops == 2)vv = (int)v1.addr * (int)v2.addr;
			else if (ops == 3)vv = (int)v1.addr / (int)v2.addr;
			else if (ops == 4)vv = (v1.addr < v2.addr);
			else if(ops==5)vv = (v1.addr > v2.addr);
			else if (ops == 6)vv = (v1.addr & v2.addr);
			else if (ops == 7)vv = (v1.addr | v2.addr);
			else if (ops == 8)vv = (v1.addr <= v2.addr);
			else if (ops == 9)vv = (v1.addr >= v2.addr);
			ctx->last_res.addr = vv;
			ctx->last_res.type = ql_number;
			ql_quit(ocodes, codes);
		}
		else
		{
			ql_err("Cannot do '+' operation between type %d and %d!\n", v1.type, v2.type); return -1;
		}
	}
	else if (len == 1 && *codes == '!')
	{
		ql_ctrim(codes, len);
		ql_arg(ctx, &codes, v1);
		ctx->last_res.addr = (!v1.addr);
		ctx->last_res.type = ql_number;
		ql_quit(ocodes, codes);
	}
	else if (len == 1 && ((*codes == '\"') || (*codes == '\'')))
	{
		int len = 0;
		char* old = codes + 1;
		codes= ql_readstr(codes+1, *codes,&len);
		uint32_t addr= ql_buildstring(ctx, old, len);
		ql_res res;
		res.addr = addr;
		res.type = ql_string;
		res.gc = ctx->gc_mark;
		ql_addsym(ctx, "@", res, res.gc);
		ctx->last_res = res;
		ql_quit(ocodes, codes);
	}
	else if ((*codes == '('))
	{
		codes++;
		while (1)
		{
			if (ql_eval(ctx, &codes, 0) < 0)return -1;
			codes = trim(codes, ' ');
			if (*codes == ')')
			{
				codes++;
				//else
				codes = trim(codes, ' ');
				int len = forward(codes);
				if (!len)break;
				if (len == 3 && !strncmp(codes, "els", 3))
				{
					ql_ctrim(codes, len);
					codes = forwardbrk(codes);
					if (!codes)
					{
						ql_err("Unmathed brks!\n");
						return -1;
					}
					break;
				}
				if (depth == 1)
				{
					ql_quit(ocodes, codes);
				}
				codes = trim(codes, ' ');
				if (*codes != '(')
				{
					//codes++;
					break;
				}
				codes++;
			}
			else if(*codes!='(') break;
			
		}
		ql_quit(ocodes, codes);
	}
	else if (len == 3 && (!strncmp(codes, "brk", 3)))
	{
		ql_ctrim(codes, len);
		ctx->loop_state = 0;
		ql_quit(ocodes, codes);
	}
	else if (len == 5 && (!strncmp(codes, "while", 5)))
	{
		ql_ctrim(codes, len);
		codes=trim(codes, ' ');
		char* judge_expr = codes;
		codes = forwardbrk(codes);
		ctx->loop_state = q_ifloop;
		char* copy = codes;
		while (1)
		{
			copy = codes;
			if (ql_eval(ctx, &copy,1) < 0)return -1;
			if (ctx->loop_state != q_ifloop)break;
			char* copy2 = judge_expr;
			//ctx->loop_state = q_ifjudge;
			if (ql_eval(ctx, &copy2,1) < 0)return -1;
			//ctx->loop_state = q_ifloop;
			if (!ctx->last_res.addr)break;
			
		}
		ctx->loop_state = 0;
		codes= forwardbrk(codes);
		ql_quit(ocodes, codes);
	}
	else if (len == 2 && (!strncmp(codes, "at", 2)))
	{
		ql_ctrim(codes, len);
		ql_arg(ctx, &codes, v1);
		ql_arg(ctx, &codes, v2);
		if (v1.type == ql_ptr&&v2.type==ql_number)
		{
			ql_res v0 = *v1.obj;
			if (v0.type == ql_list)
			{
				short* info = &v0.addr;
				int root_idx = info[0];
				int nums = info[1];
				if (v2.addr < nums)
				{
					ctx->last_res = ctx->sym_list[root_idx - v2.addr].obj;
					ctx->last_ref = &ctx->sym_list[root_idx - v2.addr].obj;
					ql_quit(ocodes, codes);
				}
				else
				{
					ql_chkmsg(-1, "Index out of range!\n");
				}
			}
		}
		ql_chkmsg(-1, "Bad args for at ,it needs (array) (index)!\n");
	}
	else if (len == 2 && (!strncmp(codes, "fn", 2)))
	{
		ql_ctrim(codes, len);
		uint32_t addr = 0;
		codes= ql_addfun(ctx, codes,&addr);
		if (!codes) { ql_err("Unmatched '(' in function define!\n"); return -1; }
		ctx->last_res.addr = addr;
		ctx->last_res.type = ql_func;
		ql_quit(ocodes, codes);
	}
	else if (len == 2 && (!strncmp(codes, "is", 2)))
	{
		ql_ctrim(codes, len);
		ql_arg(ctx, &codes, v1);
		ql_arg(ctx, &codes, v2);
		ql_conftype(v1, v2);
		if (v1.type == ql_number||v1.type==ql_func)
		{
			ctx->last_res.addr = (v1.addr == v2.addr);
			ctx->last_res.type = ql_number;
			ql_quit(ocodes, codes);
		}
		else if (v1.type == ql_string)
		{
			ctx->last_res.addr = (strcmp(v1.addr,v2.addr)==0);
			ctx->last_res.type = ql_number;
			ql_quit(ocodes, codes);
		}
		else
		{
			ql_err("Unsupported comparison between two types!\n");
			return -1;
		}
	}
	else if (*codes == ')')
	{
		*ocodes = codes;
		return 2;
	}
	else if (len == 2 && (!strncmp(codes, "if", 2)))
	{
		ql_ctrim(codes, len);
		ql_arg(ctx, &codes, v1);
		if (v1.type != ql_number)
		{
			ql_err("If can only compare 2 numerical value!");
			return -1;
		}
		if (v1.addr)
		{
			if (ql_eval(ctx, &codes,1) < 0)return -1;
			ql_quit(ocodes, codes);
		}
		else
		{
			codes = forwardbrk(codes);
			codes = trim(codes,' ');
			int len = forward(codes);
			codes = trim(codes,' ');
			if (len == 3 && !strncmp(codes, "els", 3))
			{
				ql_ctrim(codes, len);	
			}
			if (ql_eval(ctx, &codes,1) < 0)return -1;
			ql_quit(ocodes, codes);
			
		}
	}
	else if ((len==2&&(!strncmp(codes,"lt",2)))
		||(len==1&&*codes=='='))
	{
		// let sym val
		ql_ctrim(codes, len);
		char quot = 0;
		if (*codes == '*')
		{
			codes++;
			quot = 1;
		}
		char* vname=0;
		ql_res* ref = 0;
		char old;
		if (ql_isvarname(codes))
		{
			vname = ql_varname(&codes, &old, &len);
			if (strlen(vname) > SYM_MAX)
			{
				ql_err("Too long var name!\n");
				return -1;
			}
			codes++;
			if (*codes == '\0')
			{
				ql_err("Not match args for let\n");
				return -1;
			}
		}
		else
		{
			ctx->last_ref = 0;
			if (ql_eval(ctx, &codes, 1) < 0)return -1;
			ref = ctx->last_ref;
		}
		//codes++;
		ql_arg(ctx, &codes, v2);
		v2.gc = ctx->gc_mark;

		if (v2.type == ql_ptr)
		{

			v2.obj->ref = 1;
			if (v2.obj->type == ql_list)
			{
				short* info = &v2.obj->addr;
				int root_idx = info[0];
				int nums = info[1];
				for (int i = 0; i<nums; i++)
				{
					ctx->sym_list[root_idx - i].obj.ref = 1;
				}
			}
		}
		if (vname)
		{
			if (!quot)
			{
				ql_mapsym(ctx, vname, v2);
			}
			else
			{
				v2.gc = 0;
				ql_chkmsg(ql_setsym_quot(ctx, vname, v2), "Cannot get value from a pointer variable!\n");
			}
			ql_restorename(vname, old, len);
		}
		else if(ref)
		{
			*ref = v2;
		}
		else
		{
			ql_chkmsg(-1, "Left must be a variable or a var name!\n");
		}
		ctx->last_res = v2;
		
		ql_quit(ocodes, codes);
	}
	else if (ql_isalpha(*codes))
	{
		ql_res v;
		if (len > SYM_MAX)
		{
			ql_err("Too long var!:%s\n", codes);
			return -1;
		}
		/*char old = *(codes + len);
		*(codes + len) = '\0';
		char* vname = codes;*/
		char old = 0;
		char *vname= ql_varname(&codes, &old, &len);
		if(!vname)
		{
			ql_err("Cannot read symbol name as %s\n",codes);
			return -1;
		}
		int idx = ql_findsym(ctx, vname, &v);
		ql_restorename(vname, old, len);
		if (idx>=0)
		{	
			if (v.type == ql_func)
			{
				uint32_t next;
				int cnt=0;
				codes= ql_fn_arg(ctx, v.addr,codes,&next,&cnt);
				if (!codes)
				{
					ql_err("Cannot solve args for function");
					return -1;
				}
				char* ncode = next;
				ctx->gc_mark = v.addr;
				int r = ql_eval(ctx, &ncode, 0);
				ql_gc(ctx, v.addr);  //must gc after call a function(before err back!)
				ctx->gc_mark = 0;
				if(r<0)return -1;
				ql_quit(ocodes, codes);
			}
			else if (v.type == ql_builtin)
			{
				int (*callback)(ql_ctx* ctx, char** codes) = v.addr;
				if (callback(ctx, &codes) < 0)return -1;
				ql_quit(ocodes, codes);
			}
			ctx->last_res = v;
			ctx->last_ref = &ctx->sym_list[idx].obj;
			ql_quit(ocodes, codes);
		}
		else
		{
			*(vname + len) = old;
			ql_err("Cannot find symbol:%s\n", codes);
			return -1;
		}
	}
	else
	{
		char e;
		int val = toint(codes, len, &e);
		if (!e)
		{
			codes += len;
			ctx->last_res.addr = val;
			ctx->last_res.type = ql_number;
			ql_quit(ocodes, codes);
		}
		else ql_err("Unknow expr!\n");
	}
}