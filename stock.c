#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "stock.h"

#define MAX_LINE_LENGTH					(8*1024)

static mem_root groot;

int load_index(char *filename)
{
	int ret = 0;
	int len = 0;
	int size = 0;
	int first = 0;
	char *cpwork;
	char *cpend;
	FILE *fin;

	char *file_buf = NULL;
	char *name_buf = NULL;
	char **name = groot.indexfile->name;
	int  name_cnt  = 0;

	fin = fopen((char *)filename, "rb");
	if (fin)
	{
		fseek( fin, 0L, SEEK_END);
		len = ftell( fin );
		size = len + 128;

		file_buf = (char *)malloc( (size_t)(size) );
		if (!file_buf)
		{
			printf("memory not enough memory.\n");
			fclose(fin);
			return -1;
		}
		memset((void *)file_buf, 0x00, size);

		fseek(fin, 0L, SEEK_SET);
		ret = fread(file_buf, len, 1, fin);
		if (1 != ret) return -2;
		fclose(fin);

		cpwork = file_buf;
		cpend  = file_buf + len;
		*(cpend+0) = 0x0D;
		*(cpend+1) = 0x0A;

		first = 1;
		while (cpwork < cpend)
		{
			if ( (*cpwork != ' ') && (*cpwork != '\t') && (*cpwork != 0x0D) && (*cpwork != 0x0A) )
			{
				if (first == 1)
				{
					first = 0;
					*name++ = cpwork;
					name_cnt++;
				}
			}
			else
			{
				*cpwork = '\0';
				first = 1;
			}
			cpwork++;
		}
	}

	if (name_cnt > 0)
	{
		groot.indexfile->used = name_cnt;
		groot.indexfile->buffer = file_buf;
	}

	return 0;
}

//"2005-01-04 09:45:00" to { 20050104,800945 }
//"1997-07-04" to { 19970704, 0 }
//"2005-01-04" to 20050104
//"09:45:00"   to 800945
#define DEFAULT_DATE            (19770812)
#define DEFAULT_TIME            (800945)
int str_to_date(char *istr, idate *odate)
{
	int idx = 0;
	int type = 0;
	char buf[16];
	char *cp = istr;

	if ( (cp == NULL) || (odate == NULL) )
		return 0;

	odate->date = DEFAULT_DATE;
	odate->time = DEFAULT_TIME;

	while ( (*cp == '\t') || (*cp == ' ') )
		cp++;

	while (*cp != '\0')
	{
		if ( (*cp >= '0') && (*cp <= '9') && (*cp != ' ') && (*cp != '-') && (*cp != ':') )
			buf[idx++] = *cp;
		cp++;

		if ( (8 == idx) && (0 == type) )
		{
			buf[idx] = '\0';
			odate->date = atoi(buf);

			idx = 0;
			buf[idx++] = '8';
			buf[idx++] = '0';
			type++;
		}
		else if ( (8 == idx) && (1 == type) )
		{
			buf[idx] = '\0';
			odate->time = atoi(buf) / 100;
			idx = 0;
			type++;
		}
	}

	return type;
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

int parse_line(char *ibuf, char *ocode, char *oname, day_data *oday, m15_data *om15)
{
	int cnt = 0;
	char *cp = ibuf;
	char *cpb= ibuf;
	char *item[128];

	if ( (ibuf == NULL) || (ocode == NULL) || (oname == NULL)  || (oday == NULL) || (om15 == NULL) )
		return -1;

	while (*cp != '\0')
	{
		while ( (*cp == '\t') || (*cp == ' ') )
		{
			*cp++ = '\0';;
		}

		cpb = cp;
		while (*cp != ',')
		{
			if( (*cp == '@') || (*cp == '#') || (*cp == '$') )
				return 0;
			cp++;
		}
		*cp = '\0';;

		if (cp != cpb)
		{
			item[cnt++] = cpb;
		}
		cp++;
	}

	if (cnt >= 26) // .day data
	{
		idate mdate;
		int type = str_to_date(item[2], &mdate);

		strcpy(ocode, item[0]);
		strcpy(oname, item[1]);

		oday->date = mdate.date;
		oday->price.open   = (float)atof(item[3 ]);
		oday->price.close  = (float)atof(item[4 ]);
		oday->price.high   = (float)atof(item[5 ]);
		oday->price.low    = (float)atof(item[6 ]);
		oday->mama.ma5     = (float)atof(item[7 ]);
		oday->mama.ma10    = (float)atof(item[8 ]);
		oday->mama.ma13    = (float)atof(item[9 ]);
		oday->mama.ma21    = (float)atof(item[10]);
		oday->mama.ma34    = (float)atof(item[11]);
		oday->mama.ma55    = (float)atof(item[12]);
		oday->mama.ma89    = (float)atof(item[13]);
		oday->mama.ma144   = (float)atof(item[14]);
		oday->mama.ma233   = (float)atof(item[15]);
		oday->macd.macd    = (float)atof(item[16]);
		oday->macd.diff    = (float)atof(item[17]);
		oday->macd.dea     = (float)atof(item[18]);
		oday->boll.upr     = (float)atof(item[19]);
		oday->boll.mid     = (float)atof(item[20]);
		oday->boll.dwn     = (float)atof(item[21]);
		oday->cci.cci14    = (float)atof(item[22]);
		oday->cci.cci21    = (float)atof(item[23]);
		oday->cci.cci55    = (float)atof(item[24]);
		oday->cci.cci89    = (float)atof(item[25]);
	}
	else if (cnt >= 8) // .m15 data
	{
		idate mdate;
		str_to_date(item[2], &mdate);

		strcpy(ocode, item[0]);
		strcpy(oname, item[1]);

		om15->date	 = mdate.date;
		om15->time   = mdate.time; 
		om15->open   = (float)atof(item[3]);
		om15->close  = (float)atof(item[4]);
		om15->high   = (float)atof(item[5]);
		om15->low    = (float)atof(item[6]);
		om15->volume = (float)atof(item[7]);
	}

	return cnt;
}

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
	int  m15_cnt = 0;
	day_data daydata;
	m15_data m15data;
	m15_data *m15work;
	m15_data *m15head;
	stock_data *stock;
	stock_data *new_mem;

	char line_buf[MAX_LINE_LENGTH];

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
		m15head = (m15_data *) (cpend + 64);
		m15work = m15head;
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
					line_buf[linelen++] = '\0';
					line_buf[linelen  ] = '\0';
					printf("[%4d]:<%s>\n", linelen, line_buf);
					linelen = 0;
				}
				line_buf[linelen++] = *cpwork++;
			}
			else
			{
				line_buf[linelen++] = '\0';
				line_buf[linelen  ] = '\0';
				cnt = parse_line(line_buf, scode, sname, &daydata, &m15data);
				if (cnt >= 26)
				{
					if (stock->code[0] == '\0') strcpy(stock->code, scode);
					if (stock->name[0] == '\0') strcpy(stock->name, sname);
					memcpy(&stock->data[day_cnt], &daydata, sizeof(day_data));
					day_cnt++;
				}
				else if (cnt >= 8)
				{
					memcpy(m15work, &m15data, sizeof(m15_data));
					m15work++;
					m15_cnt++;
				}

				linelen = 0;
				while ((*cpwork == 0x0D) || (*cpwork == 0x0A) ) cpwork++;
			}
		}

		if (day_cnt > 0)
		{
			memsize = (~0x0F) & (15 + sizeof(stock_data) + day_cnt*sizeof(day_data));
			new_mem = (stock_data*)malloc( (size_t)(memsize) );
			if (new_mem) 
			{
				stock->stockid = str_to_stockid(stock->code);
				stock->records = day_cnt;
				stock->memsize = memsize;
				memcpy(new_mem, stock, memsize);

				groot.stockmap->stocks[groot.stockmap->used++] = new_mem;
			}
		}
		if (m15_cnt > 0)
		{
			section_block section;
			FILE *fpout = NULL;

			sprintf(line_buf, "./mem/%s.m15", scode);
			printf("mem: %s\n", line_buf);
			fpout = fopen(line_buf, "w");

			/* head */
			memset(&section, 0x00, sizeof(section_block));
			sprintf(section.name, ".section=<%s>", ".m15");
			section.size = m15_cnt;
			fwrite(&section, sizeof(section_block), 1, fpout);

			/* m15_data[] */
			fwrite(m15head, sizeof(m15_data)*m15_cnt, 1, fpout);
			fclose(fpout);
		}

		free(cpmem);
	}

	return 0;
}

int init_mem()
{
	void *vp = NULL;
	memset(&groot, 0x00, sizeof(mem_root));

	vp = malloc(sizeof(index_file));
	if (vp) {
		memset(vp, 0x00, sizeof(index_file));
		groot.indexfile = vp;
	}

	vp = malloc(sizeof(trade_date));
	if (vp) {
		memset(vp, 0x00, sizeof(trade_date));
		groot.tradedate = vp;
	}

	vp = malloc(sizeof(stock_map));
	if (vp) {
		memset(vp, 0x00, sizeof(stock_map));
		groot.stockmap = vp;
	}

	return 0;
}

int free_mem()
{
	int idx = 0;

	for (idx=0; idx<groot.stockmap->used; idx++)
		free(groot.stockmap->stocks[idx]);

	if (groot.indexfile->buffer)
		free (groot.indexfile->buffer);

	free(groot.tradedate);
	free(groot.indexfile);
	free(groot.stockmap );

	memset(&groot, 0x00, sizeof(mem_root));

	return 0;
}

int int_sort(int *array, int len)
{
	int idx = 0;
	int beg = 0;
	int end = len-1;

	while ( beg < end )
	{
		int minid = beg;
		int maxid = end;
		int value = 0;

		value = array[beg];
		if (value > array[end])
		{
			array[beg] = array[end];
			array[end] = value;
		}

		for (idx=beg; idx<=end; idx++)
		{
			value = array[idx];
			if ( value > array[maxid]) maxid = idx;
			if ( value < array[minid]) minid = idx;
		}

		value = array[maxid];
		array[maxid] = array[end];
		array[end] = value;

		value = array[minid];
		array[minid] = array[beg];
		array[beg] = value;

		beg++; end--;
	}

	return 0;
}

int get_tradeday_list()
{
	int chk = 0;
	int idx = 0;
	int idy = 0;
	int idz = 0;
	int *day1 = NULL;
	int *day2 = NULL;
	trade_date *tdate = groot.tradedate;

	printf("tdate->used = %d\n", tdate->used);
	for (idx=0; idx<groot.stockmap->used; idx++)
	{
		stock_data *stock = (stock_data *)groot.stockmap->stocks[idx];

		for (idy=0; idy<stock->records; idy++)
		{
			int find  = 0;
			int today = stock->data[idy].date;
			for (idz=0; idz<tdate->used; idz++)
			{
				if (today == tdate->date[idz])
				{
					find = 1;
					break;
				}
			}
			if (0 == find)
			{
				tdate->date[tdate->used++] = today;
			}
		}
	}

	int_sort(tdate->date, tdate->used);
	printf("tdate->used = %d\n", tdate->used);

	day1 = tdate->date;
	day2 = day1 + 1;
	for (idx=0; idx<tdate->used-1; idx++)
	{
		if ( *day1 >= *day2 )
		{
			printf("ERROR: idx=%d <day1=%d,day2=%d>\n", idx, *day1, *day2);
			chk++;
		}
		day1++;
		day2++;
	}

	if (chk)
	{
		day1 = tdate->date;
		for (idx=0; idx<tdate->used; idx++)
		{
			printf("[%d]:%d\n", idx, *day1);
		}
	}

	day1 = tdate->date;
	printf("FirstDay: %d\n", *day1);
	day1 += tdate->used - 1;
	printf("LastDay : %d\n", *day1);

	return 1;
}

int mem_dump(char *file)
{
	int idx = 0;
	FILE *fpout;
	section_block section;

	fpout = fopen((char *)file, "wb");
	if (fpout)
	{
		/* head */
		memset(&section, 0x00, sizeof(section_block));
		sprintf(section.name, ".section=<%s>", "head");
		section.size = 0;
		fwrite(&section, sizeof(section_block), 1, fpout);

		/* trade_day */
		memset(&section, 0x00, sizeof(section_block));
		sprintf(section.name, ".section=<%s>", "tradedate");
		section.size = sizeof(trade_date);
		fwrite(&section, sizeof(section_block), 1, fpout);
		fwrite(groot.tradedate, section.size, 1, fpout);

		/* stock_cnt */
		memset(&section, 0x00, sizeof(section_block));
		sprintf(section.name, ".section=<%s>", "stockmap");
		section.size = groot.stockmap->used;
		fwrite(&section, sizeof(section_block), 1, fpout);

		/* stock_data */
		for (idx=0; idx<groot.stockmap->used; idx++)
		{
			stock_data *stock = (stock_data *)groot.stockmap->stocks[idx];

			memset(&section, 0x00, sizeof(section_block));
			sprintf(section.name, ".stock<%s>", stock->code);
			section.size = stock->memsize;
			fwrite(&section, sizeof(section_block), 1, fpout);
			fwrite(stock, section.size, 1, fpout);
		}

		fclose(fpout);
		return 0;
	}

	return -1;
}

int mem_load(char *file)
{
	int ret = 0;
	int size = 0;
	int count = 0;
	FILE *fpin;
	section_block section;
	stock_data *sd = NULL;

	fpin = fopen((char *)file, "rb");
	if (fpin)
	{
		/* head */
		memset(&section, 0x00, sizeof(section_block));
		fread(&section, sizeof(section_block), 1, fpin);
		if ( (strstr(section.name, "head") == NULL) || (section.size != 0) )
		{
			ret = -1;
			goto _error;
		}

		/* trade_date */
		memset(&section, 0x00, sizeof(section_block));
		fread(&section, sizeof(section_block), 1, fpin);
		if ( (strstr(section.name, "tradedate") == NULL)
				|| (section.size != sizeof(trade_date)) )
		{
			ret = -2;
			goto _error;
		}
		fread(groot.tradedate, section.size, 1, fpin);

		/* stock_cnt */
		memset(&section, 0x00, sizeof(section_block));
		fread(&section, sizeof(section_block), 1, fpin);
		if (strstr(section.name, "stockmap") == NULL)
		{
			ret = -3;
			goto _error;
		}
		count = section.size;
		printf("load %d stocks data to memory.\n", count);

		while ( !feof(fpin) )
		{
			memset(&section, 0x00, sizeof(section_block));
			fread(&section, sizeof(section_block), 1, fpin);
			if (strstr(section.name, ".stock") == NULL)
			{
				ret = -4;
				goto _error;
			}

			/* stock_data */
			sd = malloc( (size_t)(section.size) );
			if (!sd)
			{
				ret = -5;
				goto _error;
			}

			fread(sd, section.size, 1, fpin);
#if 0 // for Debug
			printf("code:<%s> stockid=%d records=%d\n",
					sd->code, sd->stockid, sd->records);
#endif
			groot.stockmap->stocks[groot.stockmap->used++] = sd;
			count--;
		}
		if (count) ret = -5;
	}
_error:
	fclose(fpin);
	return ret;
}

int back_test_go(backtest *bt)
{
	int idx   = 0;
	int idy   = 0;
	int beg   = 20100101;
	int end   = 20101231;
	int days  = 0;
	int *today;

	if (bt == NULL) return -1;

	beg = bt->beg;
	end = bt->end;

	today = groot.tradedate->date;
	while (*today < beg) today++;

	while ( (*today) && (*today <= end) )
	{
		for (idx=0; idx<groot.stockmap->used; idx++)
		{
			stock_data *stock = (stock_data *)groot.stockmap->stocks[idx];
			for (idy=0; idy<stock->records; idy++)
			{
				int m_date = stock->data[idy].date;
				if (m_date  == *today)
				{
					printf("[%d]: %d open:%6.02f, close:%6.02f, high:%6.02f, low:%6.02f\n",
							stock->stockid,
							stock->data[idy].date,
							stock->data[idy].price.open,
							stock->data[idy].price.close,
							stock->data[idy].price.high,
							stock->data[idy].price.low);
					break;
				}
			}
		}
		today++;
	}

	return 1;
}

int check_go()
{
	int idx   = 0;
	int idy   = 0;
	int date_min = 0;
	int date_max = 0;
	float close_min = 1000.0f;
	float close_max = 0;

	for (idx=0; idx<groot.stockmap->used; idx++)
	{
		close_min = 1000.0f;
		close_max = 0.0f;

		stock_data *stock = (stock_data *)groot.stockmap->stocks[idx];
		for (idy=0; idy<stock->records; idy++)
		{

			if (close_min > stock->data[idy].price.close)
			{
				close_min = stock->data[idy].price.close;
				date_min  = stock->data[idy].date;
			}
			if (close_max < stock->data[idy].price.close)
			{
				close_max = stock->data[idy].price.close;
				date_max  = stock->data[idy].date;
			}
		}

		close_min = stock->data[0].price.close;
		date_min  = stock->data[0].date;
		if (close_min < 0.01) close_min = 0.01;
		if (close_max < 0.01) close_max = 0.01;
		printf("%s, %6.02f, %d, %6.02f, %d, %6.02f\n",
				stock->code,
				close_max, date_max,
				close_min, date_min,
				close_max / close_min);
	}

	return 1;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int idx = 0;
	int fdump = 0;
	int fload = 0;
	int fhelp = 0;
	int ftest = 0;
	char key = 0;
	char *findex = NULL;
	index_file *idxfile = NULL;
	backtest btargv;

	for (idx=1; idx<argc; idx++)
	{
		printf("[%d]:<%s>\n", idx, argv[idx]);
		if (!strcmp(argv[idx], "--dump")) fdump = 1;
		else if (!strcmp(argv[idx], "--load")) fload = 1;
		else if (!strcmp(argv[idx], "--test")) ftest = 1;
		else if (!strcmp(argv[idx], "--file"))
		{
			idx++;
			findex = argv[idx];
			printf("index: %s\n", findex);
		}
		else
		{
			printf("usage: %s --file index.txt --dump --load\n", argv[0]);
			return 0;
		}
	}

#if 0
	if (fdump) printf("fdump\n");
	if (fload) printf("fload\n");
	if (findex) printf("%s\n", findex);
#endif

	//Test mode
	if (ftest)
	{
		printf("Start backtest ...\n");
		init_mem();
		mem_load("./stock.mem");

		printf("Press any key to start backtest ...");
		key = getchar();

		btargv.beg = 20100101;
		btargv.end = 20110101;
		btargv.max_stock = 20;
		btargv.money = 100 * 10000.00;
		btargv.taxrate = 0.001f;

		//back_test_go(&btargv);
		printf("\n\n");
		check_go();
		printf("\n\n");

		printf("Press any key to release memory.");
		key = getchar();

		free_mem();

		return 0;
	}

	init_mem();

	if (findex)
	{
		ret = load_index(findex);

		idxfile = groot.indexfile;
		for (idx=0; idx<idxfile->used; idx++)
		{
			printf("load %s\n", idxfile->name[idx]);
			ret = load_csv(idxfile->name[idx]);
		}

		get_tradeday_list();
	}

	if (fdump) 
		mem_dump("./stock.mem");

	free_mem();
	if (fload)
	{
		init_mem();
		mem_load("./stock.mem");

		printf("Press any key to release memory.");
		key = getchar();
		free_mem();
	}

	printf("Press 'x' to exit ... ");
	while (key != 'x')
	{
		key = getchar();
	}

	printf("\n over \n");

	return ret;
}
