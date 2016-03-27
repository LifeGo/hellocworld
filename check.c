
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#define MAX_FILE_NUM					(8*1024)
#define MAX_LINE_LENGTH					(8*1024)

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
} day_data;

typedef struct _tag_stock_mem_map
{
	char code[16];
	char name[16];
	int stockid;
	int total;
	day_data array[];
} stock_data;

char item_buf[MAX_LINE_LENGTH];
char *item[MAX_LINE_LENGTH];

char *index_file[MAX_FILE_NUM];
char *index_mem = NULL;
int  index_cnt  = 0;

stock_data *stock_mem[MAX_FILE_NUM];
int  stock_cnt = 0;

//"1997-07-04" to "19970704"
#define DEFAULT_DATE            (19770218)
int str_to_date(char *istr)
{
    int idx = 0;
    int idate = DEFAULT_DATE;
    char date[12];
    char *cp = istr;

    if (cp == NULL)
        return DEFAULT_DATE;
    
    while (*cp != '\0')
    {
        if ( (*cp >= '0') && (*cp <= '9') && (*cp != '-') )
            date[idx++] = *cp;
        else if (*cp != '-')
            return DEFAULT_DATE;
        cp++;
    }

    date[idx] = '\0';

    return atoi(date);
}

int str_to_stockid(char *istr)
{
	int idx = 0;
	char date[12];
	char *cp = istr;

	if (cp == NULL)
		return 0;

	while (*cp != '\0')
	{
		if      (*cp == 'S') { date[idx++] = '8'; cp++; }
		else if (*cp == 'H') { date[idx++] = '6'; cp++; }
		else if (*cp == 'Z') { date[idx++] = '8'; cp++; }
		else if ( ((*cp >= '0') && (*cp <= '9')) )
			date[idx++] = *cp++;
		else
			return 0;
	}

	date[idx] = '\0';
	return atoi(date);
}

#if 1
int parse_line(char *ibuf, char *ocode, char *oname, day_data *oitem)
{
	int cnt = 0;
	int first = 0;
	char *cp= ibuf;

	if ( (ibuf == NULL) || (ocode == NULL) || (oname == NULL) || (oitem == NULL) )
		return -1;

	while (*cp != '\0')
	{
		if( (*cp == '@') || (*cp == '#') || (*cp == '$') )
			return 0;

		if ( (*cp == '\t') || (*cp == ' ') || (*cp == ',') )
		{
			*cp++ = '\0';;
			first = 1;
			continue;
		}
		if (first == 1) 
		{
			first = 0;
			item[cnt++] = cp;
		}
		cp++;
	}

	strcpy(ocode, item[0]);
	strcpy(oname, item[1]);

	oitem->date 	= str_to_date(item[2]);
	oitem->open 	= atof(item[3 ]);
	oitem->close 	= atof(item[4 ]);
	oitem->high 	= atof(item[5 ]);
	oitem->low 		= atof(item[6 ]);
	oitem->ma5 		= atof(item[7 ]);
	oitem->ma10 	= atof(item[8 ]);
	oitem->ma13 	= atof(item[9 ]);
	oitem->ma21 	= atof(item[10]);
	oitem->ma34 	= atof(item[11]);
	oitem->ma55 	= atof(item[12]);
	oitem->ma89 	= atof(item[13]);
	oitem->ma144 	= atof(item[14]);
	oitem->ma233 	= atof(item[15]);
	oitem->macd 	= atof(item[16]);
	oitem->diff 	= atof(item[17]);
	oitem->dea 		= atof(item[18]);
	oitem->bollupr 	= atof(item[19]);
	oitem->bollmid 	= atof(item[20]);
	oitem->bolldwn 	= atof(item[21]);
	oitem->cci14 	= atof(item[22]);
	oitem->cci21 	= atof(item[23]);
	oitem->cci55 	= atof(item[24]);

	return cnt;
}
#else
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
#endif

int load_index(char *filename)
{
	int ret = 0;
	int first = 0;
	int filelen = 0;
	char *cpwork;
	char *cpend;
	FILE *fin;

	fin = fopen((char *)filename, "rb");
	if (fin)
	{
		fseek( fin, 0L, SEEK_END);
		filelen = ftell( fin );

		index_mem = (char *)malloc( (size_t)(filelen+128) );
		if (index_mem == NULL)
		{
			printf("memory not enough memory.\n");
			fclose(fin);
			return -1;
		}
		memset((void *)index_mem, 0x00, filelen+128);

		fseek(fin, 0L, SEEK_SET);
		ret = fread(index_mem, filelen, 1, fin);
		if (1 != ret) return -2;
		fclose(fin);

		cpwork = index_mem;
		cpend  = index_mem + filelen;
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
				index_file[index_cnt++] = cpwork;
			}
			cpwork++;
		}
	}

	return 0;
}
#if 0
int cvs_to_mem(char *in, char *out)
{
	char *temp_mem = NULL;
	temp_mem = (char *)malloc( (size_t)(filelen+128) );
	if (temp_mem == NULL)
	{
		printf("memory not enough memory.\n");
		fclose(fin);
		return -1;
	}
	memset((void *)temp_mem, 0x00, filelen+128);
}
#endif

int load_csv(char *filename)
{
	int ret = 0;
	int cnt = 0;
	int idx = 0;
	int filelen = 0;
	int linelen = 0;
	int memsize = 0;
	char *cpmem;
	char *cpwork;
	char *cpend;
	FILE *fin;

	char sname[16];
	char scode[16];
	int  day_cnt = 0;
	stock_data *stock;
	stock_data *new_mem;
	day_data daydata;

	fin = fopen((char *)filename, "rb");
	if (fin)
	{
		fseek(fin, 0L, SEEK_END);
		filelen = ftell( fin );
		memsize = 2 * filelen + 128;

		cpmem = (char *)malloc( (size_t) memsize );
		if (cpmem == NULL)
		{
			printf("memory not enough memory.\n");
			fclose(fin);
			return -1;
		}
		memset((void *)cpmem, 0x00, memsize);
		cpwork = cpmem;
		cpend  = cpmem + filelen;
		stock  = (stock_data*) (cpend + 64);
		*(cpend+0) = 0x0D;
		*(cpend+1) = 0x0A;

		fseek(fin, 0L, SEEK_SET);
		ret = fread(cpmem, filelen, 1, fin);
		if (1 != ret) return -2;
		fclose(fin);

		linelen = 0;
		while (cpwork < cpend)
		{
			if ( (*cpwork != 0x0D) && (*cpwork != 0x0A) )
			{
				if (linelen >= MAX_LINE_LENGTH-2)
				{
					item_buf[linelen++] = '\0';
					item_buf[linelen  ] = '\0';
					printf("[%4d]:<%s>\n", linelen, item_buf);
					linelen = 0;
				}
				item_buf[linelen++] = *cpwork++;
			}
			else
			{
				item_buf[linelen++] = '\0';
				item_buf[linelen  ] = '\0';
				cnt = parse_line(item_buf, scode, sname, &daydata);
				if (cnt > 0)
                {
					if (stock->code[0] == '\0') strcpy(stock->code, scode);
					if (stock->name[0] == '\0') strcpy(stock->name, sname);
					memcpy(&stock->array[day_cnt], &daydata, sizeof(day_data));
					day_cnt++;
#if 0 //for Debug
                    printf("\n item.date    = %d\n" , daydata.date   );
                    printf(" item.open    = %.02f\n", daydata.open   );
                    printf(" item.close   = %.02f\n", daydata.close  );
                    printf(" item.high    = %.02f\n", daydata.high   );
                    printf(" item.low     = %.02f\n", daydata.low    );
                    printf(" item.ma5     = %.02f\n", daydata.ma5    );
                    printf(" item.ma10    = %.02f\n", daydata.ma10   );
                    printf(" item.ma13    = %.02f\n", daydata.ma13   );
                    printf(" item.ma21    = %.02f\n", daydata.ma21   );
                    printf(" item.ma34    = %.02f\n", daydata.ma34   );
                    printf(" item.ma55    = %.02f\n", daydata.ma55   );
                    printf(" item.ma89    = %.02f\n", daydata.ma89   );
                    printf(" item.ma144   = %.02f\n", daydata.ma144  );
                    printf(" item.ma233   = %.02f\n", daydata.ma233  );
                    printf(" item.macd    = %.02f\n", daydata.macd   );
                    printf(" item.diff    = %.02f\n", daydata.diff   );
                    printf(" item.dea     = %.02f\n", daydata.dea    );
                    printf(" item.bollupr = %.02f\n", daydata.bollupr);
                    printf(" item.bollmid = %.02f\n", daydata.bollmid);
                    printf(" item.bolldwn = %.02f\n", daydata.bolldwn);
                    printf(" item.cci14   = %.02f\n", daydata.cci14  );
                    printf(" item.cci21   = %.02f\n", daydata.cci21  );
                    printf(" item.cci55   = %.02f\n", daydata.cci55  );
#endif
                }

				linelen = 0;
				while ((*cpwork == 0x0D) || (*cpwork == 0x0A) ) cpwork++;
			}
		}

		if (day_cnt > 0)
		{
			stock->stockid = str_to_stockid(stock->code);
			stock->total   = day_cnt;
		}

		memsize = sizeof(stock_data) + day_cnt * sizeof(day_data);
		new_mem = (stock_data*)malloc( (size_t)(memsize) );
		memcpy(new_mem, stock, memsize);
		
		free(cpmem);
		stock_mem[stock_cnt++] = new_mem;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int idx = 0;
	int idy = 0;
	char *cpbuf = NULL;
	stock_data *stock;

	if (argc == 2)
		ret = load_index(argv[1]);

	for (idx=0; idx<index_cnt; idx++)
	{
		printf("index_file:[%s]\r", index_file[idx]);
		ret = load_csv(index_file[idx]);
	}

	if (index_mem != NULL)
		free(index_mem);

	for (idx=0; idx<stock_cnt; idx++)
	{
		stock = (stock_data *)stock_mem[idx];
		printf("code:<%s> name:<%s> stockid=%d total=%d\r\n",
				stock->code, stock->name, stock->stockid, stock->total);
	}

	for (idx=0; idx<stock_cnt; idx++)
	{
		stock= stock_mem[idx];
		if (cpbuf != NULL) free(stock);
	}
	printf("\n over \n");

	return ret;
}

