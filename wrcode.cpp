/**
* @brief
*		Find errors and decrease probability of getting errors of the same kind in the future
*		This piece of code won't compile and it doesn't describe an entire algorithm: just part of some page storage
*
* @author
*		AnnaM
*/

/*Commiter: russian is the language of code comments there*/
/*Commiter: I've used keyword CHANGE to show initial varian of code parts*/
/*CHANGE:*/
/*Commiter: I've used keyword OFFER to offer logical additions, but vecouse of the lack of information, they might be wrong*/
/*OFFER:*/
/*Commiter: I've used keyword ERROR to draw attention to errors, which can't be fixed by commiter, because the lack of information, they might be wrong*/
/*ERROR:*/
#include <Windows.h>
#include <stdio.h>

enum PAGE_COLOR
{
	PG_COLOR_GREEN = 1, /* page may be released without high overhead */
	PG_COLOR_YELLOW, /* nice to have */
	PG_COLOR_RED	/* page is actively used */
};


/**
 * UINT Key of a page in hash-table (prepared from color and address)
 */
/*CHANGE:

Использование явных чисел для констант, которые могут понадобится более чем в 1 случае, черевато ошибками
(так как изменив эти числа в одном случае, можно забыть про остальные)*/
#define PageKey_cColor_size 8
#define PageKey_cAddr_size 24

/*CHANGE:
union PageKey
{
	struct
	{
        CHAR	cColor: 8;
		UINT	cAddr: 24;
	};

	UINT	uKey;
};*/

/*Как следствие замены явных чисел, на константы*/
union PageKey
{
	struct
	{
        	CHAR	cColor: PageKey_cColor_size;
		UINT	cAddr: PageKey_cAddr_size; 
		/*OFFER: использование intptr_t (или __intptr_t) вместо UINT для работы с значениями указателей в C++ более уместно*/
	};

	UINT	uKey;
};


/* Prepare from 2 chars the key of the same configuration as in PageKey */
/*CHANGE:
#define CALC_PAGE_KEY( Addr, Color )	(  (Color) + (Addr) << 8 )  */

/*так как в PAGE_INIT в строке 
(Desc).uKey = CALC_PAGE_KEY( Addr, Color );  (старый вариант) 
используется присваивание целому юниону значение, свойственное его полю, во избежаение ошибок следует явно указать поле,
т.е мы подрозумеваем, что он есть -> нужно указать тип юниона. Использование функции как следствие*/
union PageKey CALC_PAGE_KEY( UINT Addr, CHAR Color ){
	union PageKey temp;
	temp.uKey = (  (Color) + (Addr) << PageKey_cColor_size );
	return temp;
}


/**
 * Descriptor of a single guest physical page
 */
struct PageDesc
{
	/*ERROR: Pg->uAddr - не был определён*/
	PageKey			uKey;	
	/* list support */
	PageDesc		*next; 
	PageDesc		*prev;
};

/*CHANGE:
#define PAGE_INIT( Desc, Addr, Color )              
    {                                               
        (Desc).uKey = CALC_PAGE_KEY( Addr, Color ); 
        (Desc).next = (Desc).prev = NULL;           
    }*/

/* так как предпологается, что у Desc есть поля uKey, next, prev, то во избежание ошибок следует это явно указать
как следствие - использование функции */ 
void PAGE_INIT(struct PageDesc * Desc, void* Addr, CHAR Color )             
{                    
	/*так как в исходной задаче мы используем UINT как значение указателя, необходимо привести указатель к UINT*/
        Desc->uKey = CALC_PAGE_KEY( static_cast<UINT>(reinterpret_cast<intptr_t>(Addr)), Color ); 
        Desc->next = Desc->prev = NULL;           
}

/*CHANGE:

так как мы используем статический массив, чтобы не выйти за его пределы стоит где-то хранить его размер
(по задачи, видимо, классы не преветствуются)*/
#define PageStrg_size 3

/* storage for pages of all colors *//*singlton*/
static PageDesc* PageStrg[ PageStrg_size ];
/* CHANGE:
void PageStrgInit()
{
	memset( PageStrg, 0, sizeof(&PageStrg) );
}*/

/*в исходном коде используется указатель на указатель на массив, не подходит под симантику функции 
инициализация статического массива начальными элементами*/
/*void PageStrgInit()
{
	memset( PageStrg, 0, PageStrg_size * sizeof(PageStrg*) );
}
должно заполнить нулями элементы массива
*/
/*более подходящее под инициализацию, - заполнить инициализированными элементами*/
void PageStrgInit()
{
	for(int i = 0; i < PageStrg_size; ++i){
		PAGE_INIT(PageStrg[i], NULL, 0) ;
	}
}
/*CHANGE:
PageDesc* PageFind( void* ptr, char color )
{
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next );
        if( Pg->uKey == CALC_PAGE_KEY(ptr,color) )
           return Pg;                                                                                                                                     
    return NULL;
}*/

PageDesc* PageFind( void* ptr, char color )
{
	/*CHANGE:*/
	if(color > PageStrg_size)
		return NULL;
	/*Во избежание выхода из массива*/
	for( PageDesc* Pg = PageStrg[color]; Pg; Pg = Pg->next );
        if( Pg->uKey == CALC_PAGE_KEY(ptr, color) )
           return Pg;                                                                                                                                     
	return NULL;
}
/* CHANGE:
PageDesc* PageReclaim( UINT cnt )
{
	UINT color = 0;
	PageDesc* Pg;
	while( cnt )
	{
		Pg = Pg->next;
		PageRemove( PageStrg[ color ] );
		cnt--;
		if( Pg == NULL )
		{
			color++;
			Pg = PageStrg[ color ];
		}
	}
}
*/
void PageReclaim( UINT cnt )/*CHANGE: PageDesc* PageReclaim( UINT cnt )*/ /*функция ничего не возвращает по своей структуре*/
{
	UINT color = 0;
	/*CHANGE:*/
	PageDesc* Pg = PageStrg[ color ];/*инициализация изначального значения*/
	while( cnt )
	{
		PageRemove( PageStrg[ color ] ); 
		/*Если PageRemove удаляет все элементы списка, иначе :
		PageDesc* nextPg;
		for(PageDesc* tempPg = PageStrg[ color ]; tempPg; ){
			nextPg = tempPg->next; 
			PageRemove(tempPg);
			tempPg = nextPg;
		}
		*/
		cnt--;
		if( Pg == NULL )
		{
			color++;
			/*CHANGE:*/
			if(color > PageStrg_size)
				return NULL;
			/*Во избежания выхлда из массива*/
			Pg = PageStrg[ color ];
		}
	}
}
            
PageDesc* PageInit( void* ptr, UINT color )
{
    PageDesc* pg = new PageDesc;
    if(pg)
        PAGE_INIT(*pg, ptr, color);
    else
        printf("Allocation has failed\n");
    return pg;
}

/**
 * Print all mapped pages
 */

/*CHANGE:
void PageDump()
{
	UINT color = 0;
	#define PG_COLOR_NAME(clr) #clr
	char* PgColorName[] = 
	{
		PG_COLOR_NAME(PG_COLOR_RED),
		PG_COLOR_NAME(PG_COLOR_YELLOW),
		PG_COLOR_NAME(PG_COLOR_GREEN)
	};

	while( color <= PG_COLOR_RED )
	{
		printf("PgStrg[(%s) %u] ********** \n", color, PgColorName[color] );
		for( PageDesc* Pg = PageStrg[++color]; Pg != NULL; Pg = Pg->next ) 
		{
			if( Pg->uAddr = NULL )
				continue;

			printf("Pg :Key = 0x%x, addr %p\n", Pg->uKey, Pg->uAddr );
		}
	}
	#undef PG_COLOR_NAME
}*/
void PageDump()
{
	UINT color = 0;
	/**CHANGE: #define PG_COLOR_NAME(clr) #clr*//*Нигде не используется, поэтому убрано*/
	char* PgColorName[] = 
	{
		"PG_COLOR_RED",
		"PG_COLOR_YELLOW",
		"PG_COLOR_GREEN"
	};

	while( color <= PG_COLOR_RED )
	{
		/*Во избежание выхода из массива*/
		if(color > PageStrg_size)
			break;
		/**CHANGE: printf("PgStrg[(%s) %u] ********** \n", color, PgColorName[color] );*/
		/*PgColorName[color] - строка, color - число, перепутан порядок*/
		printf("PgStrg[(%s) %u] ********** \n", PgColorName[color], color);
		for( PageDesc* Pg = PageStrg[color]; Pg != NULL; Pg = Pg->next )/*PageStrg[++color] пропуск исследования при color == 0*/
		{
			if( Pg->uAddr == NULL )/*CHANGE: if( Pg->uAddr = NULL )*//*выполнимаост присваивания как условие не подходит по логике программы*/
				continue;
			/*CHANGE: 
			printf("Pg :Key = 0x%x, addr %p\n", Pg->uKey, Pg->uAddr ); Pg->uKey - union, для итерпритации его значения, стоит использовать его поле*/
			printf("Pg :Key = 0x%x, addr %p\n", Pg->uKey.uKey, Pg->uAddr );
		}
		color++;
	}
}
