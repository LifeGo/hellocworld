#ifndef __STOCK_H__
#define __STOCK_H__

#define MAX_STOCKS 						(4000)
#define MAX_TRADE_DAYS 					(40*365)

typedef struct _tag_section_block
{
	char name[32];
	int  size;
	int  chk0;
	int  chk1;
} section_block;

typedef struct _tag_stock_list_
{
	int used;
	int code[MAX_STOCKS];
	int name[MAX_STOCKS][16];
} stock_list;

typedef struct _tag_trade_date
{
	int used;
	int date[MAX_TRADE_DAYS];
} trade_date;

typedef struct _tag_cci_data
{
	float cci14;
	float cci21;
	float cci55;
	float cci89;
} cci_data;

typedef struct _tag_mama_data
{
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
} mama_data;

typedef struct _tag_macd_data
{
	//MACD@,DIFF@,DEA@,
	float macd;
	float diff;
	float dea;
} macd_data;

typedef struct _tag_boll_data
{
	//BOLLUPR@,BOLLMID@,BOLLDWN@,
	float upr;
	float mid;
	float dwn;
} boll_data;

typedef struct _tag_price_data
{
	float open;
	float close;
	float high;
	float low;
} price_data;

typedef struct _tag_day_item
{
	int date;
	price_data price;

	//MA5@,MA10@,MA13@,MA21@,MA34@,MA55@,MA89@,MA144@,MA233@,
	mama_data  mama;

	//MACD@,DIFF@,DEA@,
	macd_data  macd;

	//BOLLUPR@,BOLLMID@,BOLLDWN@,
	boll_data  boll;

	//CCI14@,CCI21@,CCI55@
	cci_data   cci;
} day_data;

typedef struct _tag_idate
{
	int date;
	int time;
} idate;

typedef struct _tag_m15_item
{
	int date;
	int time;
	float open;
	float close;
	float high;
	float low;
	float volume;
} m15_data;;

typedef struct _tag_stock_data
{
	void *m_this;
	char code[16];
	char name[16];
	int stockid;
	int records;
	int memsize;
	day_data data[];
} stock_data;

typedef struct _tag_stock_map
{
	int used;
	stock_data *stocks[MAX_STOCKS];
} stock_map;

typedef struct _tag_index_data
{
	int used;
	char *buffer;
	char *name[MAX_STOCKS];
} index_file;

typedef struct _tag_stock_root
{
	index_file *indexfile;
	trade_date *tradedate;
	stock_map  *stockmap;
} mem_root;

typedef struct _tag_backtest_argv
{
	int beg;
	int end;
	int max_stock;
	float money;
	float taxrate;
} backtest;

#endif /* __STOCK_H__ */
