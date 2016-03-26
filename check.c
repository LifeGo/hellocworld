
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#define MAX_FILE_NUM					(8*1024)
#define MAX_LINE_LENGTH					(8*1024)

char line_buf[MAX_LINE_LENGTH];
char item_buf[MAX_LINE_LENGTH];
char *item[MAX_LINE_LENGTH];

char *file_buff[MAX_FILE_NUM];
char *file_name[MAX_FILE_NUM];
char *file_mem = NULL;
int  file_cnt  = 0;
int  buff_cnt  = 0;

typedef struct _tag_day_item
{
	int date;
	float open;
	float close;
	float high;
	float low;

	//MA5@,MA10@,MA13@,MA21@,MA34@,MA55@,MA89@,MA144@,MA233@,
	float ma5;
	float ma10;
	float ma13;
	float ma21;
	float ma34;
	float ma55;
	float ma89;
	float ma144;
	float ma233;

	//MACD@,DIFF@,DEA@,
	float macd;
	float diff;
	float dea;

	//BOLLUPR@,BOLLMID@,BOLLDWN@,
	float bollupr;
	float bollmid;
	float bolldwn;

	//CCI14@,CCI21@,CCI55@
	float cci14;
	float cci21;
	float cci55;
} day_item;

typedef struct _tag_stock_mem_map
{
	char stockid[12];
	int  total;
	day_item array[];
} stock_mem_map;

static sotck_mem_map *memroot = NULL;

int parse_line(char *ibuf, char *obuf, char **oidx)
{
	int idx = 0;
	int first = 0;
	char *cp= NULL;

	if ( (oidx == NULL) || (obuf == NULL) )
		return -1;

	strcpy(obuf, ibuf);
	cp = obuf;

	while (*cp != '\0')
	{
		//if( (*cp == ':') || (*cp == '#') || (*cp == '$') )
		if ( (*cp == '\t') || (*cp == ' ') || (*cp == ',') )
		{
			*cp++ = '\0';;
			first = 1;
			continue;
		}
		if (first == 1) 
		{
			first = 0;
			oidx[idx++] = cp;
		}
		cp++;
	}

	return idx;
}

int load_index(char *filename)
{
	int first = 0;
	int length = 0;
	int filelen = 0;
	int ret = 0;
	char *cpwork;
	char *cpend;
	FILE *fin;

	fin = fopen((char *)filename, "rb");
	if (fin)
	{
		fseek( fin, 0L, SEEK_END);
		filelen = ftell( fin );

		file_mem = (char *)malloc( (size_t)(filelen+128) );
		memset((void *)file_mem, 0x00, filelen+128);
		if (file_mem == NULL)
		{
			printf("memory not enough memory.\n");
			fclose(fin);
			return -1;
		}

		fseek(fin, 0L, SEEK_SET);
		ret = fread(file_mem, filelen, 1, fin);
		if (1 != ret) return -2;
		fclose(fin);

		cpwork = file_mem;
		cpend  = file_mem + filelen;
		*(cpend+0) = 0x0D;
		*(cpend+1) = 0x0A;

		while (cpwork < cpend)
		{
			if ( (*cpwork == ' ') || (*cpwork == '\t') || (*cpwork == 0x0D) || (*cpwork == 0x0A) )
			{
				*cpwork++ = '\0';
				first = 1;
				continue;
			}
			if (first == 1)
			{
				first = 0;
				file_name[file_cnt++] = cpwork;
			}
			cpwork++;
		}
	}

	return 0;
}

int cvs_to_mem(char *in, char *out)
{

}

int load_csv(char *filename)
{
	int ret = 0;
	int cnt = 0;
	int idx = 0;
	int length = 0;
	int filelen = 0;
	int line_idx = 0;
	int line_total = 0;
	char *cphead;
	char *cpwork;
	char *cpend;
	FILE *fin;

	fin = fopen((char *)filename, "rb");
	if (fin)
	{
		fseek( fin, 0L, SEEK_END);
		filelen = ftell( fin );

		cphead = (char *)malloc( (size_t)(filelen+128) );
		memset((void *)cphead, 0x00, filelen+128);
		if (cphead == NULL)
		{
			printf("memory not enough memory.\n");
			fclose(fin);
			return -1;
		}

		fseek(fin, 0L, SEEK_SET);
		ret = fread(cphead, filelen, 1, fin);
		if (1 != ret) return -2;
		fclose(fin);

		cpwork = cphead;
		cpend   = cphead + filelen;
		*(cpend+0) = 0x0D;
		*(cpend+1) = 0x0A;

		line_idx = 0;
		while (cpwork < cpend)
		{
			if ( (*cpwork != 0x0D) && (*cpwork != 0x0A) )
			{
				if (line_idx >= MAX_LINE_LENGTH-2)
				{
					line_buf[line_idx++] = '\0';
					line_buf[line_idx  ]= '\0';
					printf("[%4d]:<%s>\n", line_idx, line_buf);
					line_idx = 0;
				}
				line_buf[line_idx++] = *cpwork++;
			}
			else
			{
				line_buf[line_idx++] = '\0';
				line_buf[line_idx  ] = '\0';
#if 0
				/printf("[%4d]:[%s]\n", line_idx, line_buf);
				//printf("%s\n", line_buf);
#else
				cnt = parse_line(line_buf, item_buf, &item[0]);
				for (idx=0; idx<cnt; idx++)
				{
				//	printf("<%d>:%s\n", idx, item[idx]);
				}
#endif
				line_idx = 0;
				line_total++;
				while ((*cpwork == 0x0D) || (*cpwork == 0x0A) ) cpwork++;
			}
		}

		//free(cphead);
		file_buff[buff_cnt++] = cphead;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int idx = 0;
	char *cpbuf = NULL;

	if (argc == 2)
		ret = load_index(argv[1]);

	for (idx=0; idx<file_cnt; idx++)
	{
		printf("file_name:[%s]\r", file_name[idx]);
		ret = load_csv(file_name[idx]);
	}

	if (file_mem != NULL)
		free(file_mem);

	for (idx=0; idx<file_cnt; idx++)
	{
		cpbuf =file_buff[idx];
		if (cpbuf != NULL) free(cpbuf);
	}
	printf("\n over \n");

	return ret;
}

