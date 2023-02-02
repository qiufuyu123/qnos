#include"utils/qconfig.h"
#include"mem/malloc.h"
#include"string.h"
#include"console.h"
//#include "iniparser.h"

enum States
{
	START,
	NEW_SECTION,	
	IN_SECTION,
	END_OF_FILE
};

typedef struct read_ini
{
	char *filename;
	char *data;
    uint32_t len;
    uint32_t cur_seek;
	char *tmp;
	int tmp_cap;
	int current_line;
	int state;
}read_ini_t;

/* Trims whitespace from the passed in string between begin_ind and
 * end_ind characters in the string. It returns the begin index and
 * the end index of the first and last valid (non-whitespace)
 * characters in the string. */
static void
trim(char *str, int *begin_ind, int *end_ind)
{
	int b = *begin_ind;
	int e = *end_ind;
	int i;

	/* find first index of non-whitespace character */
	for(i=b; i<e; i++)
	{
		char c = str[i];
		if(c != ' ' && c != '\t' && c != '\n' && c != '\r')
			break;
	}
	*begin_ind = i;
			
	/* find last index of non-whitespace character */
	for(i=e-1; i>=b; i--)
	{
		char c = str[i];
		if(c != ' ' && c != '\t' && c != '\n' && c != '\r')
			break;
	}
	*end_ind = i;
}
char getc(struct read_ini*ini)
{
    if(ini->cur_seek>=ini->len)return 0;
    char r= ini->data[ini->cur_seek];
    ini->cur_seek++;
    return r;
}
static int
read_line(struct read_ini *read_ini)
{
	int i = 0;
	int b;
	int new = 0;

	for(;;)
	{
		char c;

		/* resize buffer if necessary */
		if(i >= read_ini->tmp_cap)
		{
			read_ini->tmp_cap *= 2;
			read_ini->tmp = krealloc(read_ini->tmp, sizeof(*read_ini->tmp) * read_ini->tmp_cap);
		}

		/* read next byte from file */
		b = getc(read_ini);

		/* if end of file then end */
		if(b == 0)
		{
			read_ini->state = END_OF_FILE;
			break;
		}

		/* test for end of line */
		c = b;
		if(c == '\r' || c == '\n')
        {
            read_ini->current_line++;
			break;
        }

		/* place char in buffer */
		read_ini->tmp[i++] = c;		
		new = 1;
	}
	
	/* place end-of-string marker */
	if(new)
		read_ini->tmp[i] = '\0';

	return new;
}

static struct section *
parse_section(struct read_ini *read_ini)
{
	int new;
	struct section *section = kmalloc(sizeof(*section));
	section->num_configs = 0;
	section->configs = NULL;

	for(;;)
	{
		new = 0;

		switch(read_ini->state)
		{
		case START:
		case IN_SECTION:
			new = read_line(read_ini);
			break;
		case NEW_SECTION:
			new = 1;
			break;
		case END_OF_FILE:
			return section;
		default:
			printf("invalid state %d in parse_section\n", read_ini->state);
			break;
		}

		if(new)
		{
			int i, x, y, b=0, e=strlen(read_ini->tmp);
			struct config *cfg;
			trim(read_ini->tmp, &b, &e);

			/* check for comments */
			if(read_ini->tmp[b] == '#' || read_ini->tmp[b] == ';')
				continue;

			/* check for new section */
			if(read_ini->tmp[b] == '[')
			{
				if(read_ini->state == IN_SECTION)
				{
					read_ini->state = NEW_SECTION;
					break;
				}
				else
				{
					/* fill section name */
					read_ini->tmp[e] = '\0';
					section->name = strdup(&read_ini->tmp[b+1]);
					
					read_ini->state = IN_SECTION;
					continue;
				}
			}

			cfg = kmalloc(sizeof(*cfg));
			
			/* read key */

			for(i=b; i<=e; i++)		
			{
				char c = read_ini->tmp[i];
				if(c == ':' || c == '=')
					break;
			}
			x = b; y = i;
			trim(read_ini->tmp, &x, &y);
			read_ini->tmp[y+1] = '\0';
			cfg->key = strdup(&read_ini->tmp[x]);

			/* read value */
			
			x = i+1;
			y = e+1;
			trim(read_ini->tmp, &x, &y);
			read_ini->tmp[y+1] = '\0';
			cfg->value = strdup(&read_ini->tmp[x]);
			
			/* add the config to the section */
			
			if(section->configs)
			{
				section->num_configs++;
				section->configs = krealloc(section->configs, sizeof(*section->configs) * section->num_configs);
			}
			else
			{
				section->configs = kmalloc(sizeof(*section->configs));
				section->num_configs = 1;
			}
			section->configs[section->num_configs-1] = cfg;
		}
	}

	return section;
}
bool check_qnos_head(read_ini_t *ini)
{
	read_line(ini);
	//printf("tmp:%s;",ini->tmp);
	if(!strncmp(ini->tmp,"[QNOS INI]",10))return 1;
	return 0;
}
static struct ini*
parse_ini(struct read_ini *read_ini)
{
	struct ini *ini = kmalloc(sizeof(*ini));
	char finished = 0;

	ini->num_sections = 0;
	ini->sections = NULL;

	while(!finished)
	{
		switch(read_ini->state)
		{
		case START:

		case NEW_SECTION:
		{
			/* read a section */
			struct section * section = parse_section(read_ini);

			if(!section)
				break;

			/* add it to the sections structure */
			if(ini->sections)
			{
				ini->num_sections++;
				ini->sections = krealloc(ini->sections, sizeof(*ini->sections) * ini->num_sections);
			}
			else
			{
				ini->sections = kmalloc(sizeof(*ini->sections));			
				ini->num_sections = 1;
			}
			ini->sections[ini->num_sections-1] = section;
			break;
		}
		case END_OF_FILE:
			finished = 1;
			break;
		default:
			printf("error parsing file at line %d\n", read_ini->current_line);
			finished = 1;
			break;
		}
	}

	return ini;
}

struct ini*
read_ini(struct read_ini **read_inip,
		 char *filename,char *data,uint32_t len)
{
	struct ini *ini;    
    struct read_ini *read_ini = *read_inip;
    
    if(!read_ini)
    {
        read_ini = kmalloc(sizeof(*read_ini));
        *read_inip = read_ini;
    }
    else
    {
        kfree(read_ini->tmp);
    }    
        
	read_ini->filename = filename;
	read_ini->current_line = 0;
	read_ini->state = START;
	read_ini->tmp = kmalloc(sizeof(*read_ini->tmp) * 4);
	read_ini->tmp_cap = 4;
	read_ini->data=data;
    read_ini->len=len;
    read_ini->cur_seek=0;
	if(check_qnos_head(read_ini))
	{
		//read_ini->cur_seek=0;
		//read_ini->current_line=0;
	}else
	{
		printf("BAD INI HEAD!");
		return ini;
	}
	ini = parse_ini(read_ini);

	//fclose(read_ini->fin);

	return ini;
}

/* This does a linear search through the keys in the parsed ini file
 * so should happen in O(n) time. This could be made better through
 * using hashtables but I don't think this will generally be the
 * bottleneck (how large can ini files get?) */
char *
ini_get_value(struct ini* ini,
              char *section,
              char *key)
{
    int s, c;
	for(s=0; s<ini->num_sections; s++)
	{
        if( strcmp(section, ini->sections[s]->name) == 0 )
            for(c=0; c<ini->sections[s]->num_configs; c++)
                if( strcmp(key, ini->sections[s]->configs[c]->key) == 0)
                    return ini->sections[s]->configs[c]->value;        
	}

    return NULL;
}
int split_2(char *src,char **sl,char **sr)
{
	char *old=src;
	while (*src)
	{
		if(*src=='.'||*src=='/')
		{
			*src='\0';
			*sl=old;
			*sr=src+1;
			return 1;
		}
		src++;
	}
	return 0;
}
qconfig_value_t* qconfig_get_value(struct ini* ini,char *path)
{

	char *section;
	char *key_name;
	if(!split_2(path,&section,&key_name))return NULL;
	//sprintf("split ok:%s %s;",section,key_name);
	char *str=ini_get_value(ini,section,key_name);
	if(!str)return NULL;
	//printf("pure v:%s;",str);
	//qconfig_value_t *cfg=kmalloc(sizeof(qconfig_value_t));
	//if(!cfg)return NULL;
	char *buff=str;
	char *target=NULL;
	uint8_t type=QCONFIG_POINTER;
	if(*buff=='\"')
	{
		buff++;
		//printf("s<");
		while ((*buff!='\"')&&(*buff))
		{
			buff++;
		}
		//printf(">s %c;",*buff);
		if(*buff=='\"')
		{
			target=kmalloc((uint32_t)buff-(uint32_t)str);
			if(!target)
			{
				//kfree(cfg);
				return NULL;
			}
			memcpy(target,str+1,buff-str-1);
			//printf("t:%s;",target);
			type==QCONFIG_STR;
		}else
			return NULL;
	}else if (*buff>='0'&&*buff<='9')
	{
		if(!str2int(buff,&target))
		{
			return NULL;
		}
		type=QCONFIG_NUMBER;
	}else
	{
		qconfig_value_t *pointer=qconfig_get_value(ini,buff);
		return pointer;
	}
	qconfig_value_t *cfg=kmalloc(sizeof(qconfig_value_t));
	if(!cfg)return NULL;
	cfg->number=target;
	cfg->type=type;
	return cfg;
	
}
/* pretty print the structure */
void
ini_pp(struct ini* ini)
{
	int s, c;
	printf("num sections: %d\n", ini->num_sections);
	for(s=0; s<ini->num_sections; s++)
	{
		printf("section: \"%s\" ", ini->sections[s]->name);
		printf("(num configs: %d)\n", ini->sections[s]->num_configs);
		for(c=0; c<ini->sections[s]->num_configs; c++)
			printf("  key: \"%s\", value: \"%s\"\n",
				   ini->sections[s]->configs[c]->key,
				   ini->sections[s]->configs[c]->value);
	}
}

void
destroy_ini(struct ini* ini)
{
	int s, c;
	for(s=0; s<ini->num_sections; s++)
	{
		for(c=0; c<ini->sections[s]->num_configs; c++)
		{
			kfree(ini->sections[s]->configs[c]->key);
			kfree(ini->sections[s]->configs[c]->value);
		}
		kfree(ini->sections[s]->name);
		kfree(ini->sections[s]->configs);
	}

	kfree(ini->sections);
	kfree(ini);
}

void
cleanup_readini(struct read_ini* read_ini)
{
    kfree(read_ini->tmp);
    kfree(read_ini);
}