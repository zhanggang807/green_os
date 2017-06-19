#include <write_vga.h>

void toHex(char c , char* buf) ; 
void printdTotalMem(struct MEMMAN* man) ; 
void testMem(struct MEMMAN* man) ;

static char* vram = (char*)0xa0000 ; 
static int xsize  = 320 ;
static int ysize  = 200 ; 

/*
static char* vram = (char*)0xe0000000 ; 
static int xsize  = 640 ;
static int ysize  = 480 ; 
*/
extern unsigned int smap_size ; 
extern char keytable[1] ;
 
static int printd_x = 0 , printd_y = 0 ;

struct MEMMAN* memman = (struct MEMMAN*)0x100000;
void cmain(void){
	
	init_palette();
	init_pit();
	struct FIFO fifo ;
	int fifobuf[128] ; 
	fifo_init(&fifo , 128 , fifobuf) ; 

	init_keyboard(&fifo , 256);
	struct MOUSE_DEC mdec ; 		
	enable_mouse(&fifo , 512 , &mdec );
	
	struct TIMERCTL *timerctl = getTimerCTL(); 
	struct TIMER *timer , *timer2 , *timer3; 
	timer = timer_alloc(); 
	timer_init(timer , &fifo , 10);
	timer_settime(timer ,1000); 	
	
	timer2 = timer_alloc(); 
	timer_init(timer2 , &fifo , 3);
	timer_settime(timer2 ,300);
 	
	timer3 = timer_alloc(); 
	timer_init(timer3 , &fifo , 1);
	timer_settime(timer3 ,50); 	
	
	sti(); 
	int mx = (xsize -16) / 2 ;
	int my = (ysize - 16) / 2 ;	
	struct AddressRangeDes* memAddr = (struct AddressRangeDes*)get_smap_buf();
	//0x100000 0x1FEE0000
 	memman_init(memman);	
	memman_free(memman , 0x00108000 , 0x1FE00000);
	
	struct SHTCTL* shtctl ;
	struct SHEET* sht_back ;
	struct SHEET* sht_mouse ;
	struct SHEET* sht_win;
	static char buf_mouse[16*16] ;
	char *buf_back , *buf_win;
	shtctl = shtctl_init(memman , vram , xsize , ysize) ;
	
	buf_back = (char*)memman_alloc_4k(memman , xsize * ysize * 2) ;
	buf_win  = (char*)memman_alloc_4k(memman , 160 * 68) ;

	sht_back 	= sheet_alloc(shtctl) ;
	sht_mouse 	= sheet_alloc(shtctl) ; 	 	
	sht_win		= sheet_alloc(shtctl) ;

	sheet_setbuf(sht_back ,	 buf_back , xsize  ,ysize , -1 );
	sheet_setbuf(sht_mouse , buf_mouse , 16 ,16 , 99 );
	sheet_setbuf(sht_win , 	 buf_win , 	160 ,68 , -1 );

	init_screen(buf_back , xsize , ysize) ; 
	init_mouse( buf_mouse, 99) ;	
	make_window8(buf_win , 160 ,68  , "window");

	//showString(buf_win ,160  , 24 ,28 ,COL8_000000 , "Welcome to") ; 
	//showString(buf_win ,160  , 24 ,44 ,COL8_000000 , "M-OS!") ; 

	sheet_slide(sht_back , 0 , 0 ) ; 
	sheet_slide(sht_mouse , mx , my - 28) ; 		
	sheet_slide(sht_win , mx - 70  , my - 30) ; 		

	sheet_updown(sht_back , 0 ) ;
	sheet_updown(sht_win , 1) ;
	sheet_updown(sht_mouse , 2) ;
	
	//printdTotalMem(memman) ;
	char buf[64] ; 
	int count = 0 ; 	
	for(;;) {
		count++ ; 
		//showString_sht(sht_win , 40 , 28 , COL8_000000,COL8_C6C6C6 , buf , 8); 
		cli();
		if(fifo_status(&fifo)) {
			int i  = fifo_get(&fifo); 
			sti() ;
			if( 256 <= i && i <= 511) {
				//keyboard data
			 //1E A 30 B	
			/*
			if(i == 0x1C) {
				//showMemInfo(memAddr + count++) ;
				//count %= smap_size;
			showString(buf_back , xsize, 0 , ysize -16 , COL8_000000, buf);
			sheet_refresh(sht_back, 0 , 0 , xsize , ysize );
			init_screen(buf_back , xsize , ysize) ; 
			}
			*/
				if(keytable[i - 256] != 0 ){
				static int x = 0 ; 
				static int y = 0 ;
				x += 16 ; 
				char cbuf[2] = {keytable[i - 256] , 0};
				showString(buf_back , xsize , x , y  ,COL8_000000 , cbuf) ;
				sheet_refresh(sht_back , x  , y  ,16 + x , 16 + y  ) ;
				}
			} else if (512 <= i && i <= 767) {
				//mouse data
			if(mouse_decode(&mdec , i - 512)) {
				mx += mdec.x ;
				my += mdec.y ;
				if(mx < 0 ) 
					mx = 0 ;
				if(my < 0 ) 
					my = 0 ;
				/*
				if(mx > xsize - 1) 
					mx = xsize - 1 ;
				if(my > ysize - 1 ) 
					my = ysize - 1 ;
	*/
				if(mx > xsize - 16 )
					mx = xsize -16 ;
				if(my > ysize - 16 ) 
					my = ysize - 16 ;
				sheet_slide(sht_mouse , mx , my) ;
			}
				
			} else if (i == 10 ) {
				showString_sht(sht_back , 0 , 64 , COL8_FFFFFF ,COL8_008484 , "10[sec]" , 7) ; 
				sprintf(buf , "%x" , count) ; 
				showString_sht(sht_win , 40 ,28 , COL8_000000 ,COL8_C6C6C6 , buf , 8) ; 
			} else if (i == 3 ) {
				showString_sht(sht_back , 0 , 84 , COL8_FFFFFF ,COL8_008484 , "3[sec]" , 6) ; 
				count = 0 ; //开始测定
			} else if (i == 1) {
				timer_init(timer3 , &fifo , 0 ) ;
				boxfill(buf_back , xsize , COL8_FFFFFF , 8 , 96 ,15, 111) ; 
				timer_settime(timer3 , 50) ;
				sheet_refresh(sht_back , 8 , 96, 15 ,111);
			} else if (i == 0 ) {
				timer_init(timer3, &fifo , 1) ; 
				boxfill(buf_back , xsize ,COL8_008484 , 8 , 96 , 15 ,111) ; 
				timer_settime(timer3 , 50) ;
				sheet_refresh(sht_back , 8 , 96, 15 ,111);
			}
		}else {
			sti() ; 
		} 
	}
}


void printdTotalMem(struct MEMMAN* man) {

	int M_UNIT = 1024 * 1024 ;
	int mem_total = memman_total(memman) ; 
	printd("TotalMem:"); 
	printi(mem_total/M_UNIT) ;
	printd("M   ");
	printi(mem_total/1024);	
	printd("K\n");
}

void testMem(struct MEMMAN* man) {
 	printdTotalMem(man) ; 
	unsigned int addr = memman_alloc(memman , 1024*1024*200) ; 
	printd("After ");
 	printdTotalMem(man) ; 
		
	unsigned int addr1 = memman_alloc_4k(memman , 1) ; 
	printd("After ");
	printdTotalMem(man) ; 
	unsigned int addr2 = memman_alloc_4k(memman , 1024*4 + 200) ; 
	printd("After ");
	printdTotalMem(man) ; 
        
	memman_free_4k(memman, addr1 , 1) ; 
	printd("Ffter ");
	printdTotalMem(man) ; 
	memman_free_4k(memman, addr2 , 1024*4 + 200) ; 
	printd("Ffter ");
	printdTotalMem(man) ; 
	memman_free(memman, addr , 1024*1024*200 ) ; 
	printd("Ffter ");
	printdTotalMem(man) ; 
	
}	

void toHex(char c , char* buf) {
     char* _t = "0123456789ABCDEF" ;
     *(buf + 0) = _t[(c >> 4)&0x0F];
     *(buf + 1) = _t[c&0x0F];
}

void showMemInfo(struct AddressRangeDes* addr ){
//	printd_x = 0 ;
//	printd_y = 0 ;

	char uf[9] = {0} ; 
	printd("baseAddrLow:");
	int2hex(addr->baseAddrLow, uf) ; 
	printd(uf);
	printd("\nbaseAddrHigh:");
	int2hex(addr->baseAddrHigh, uf) ; 
	printd(uf);
	printd("\nlengthLow:");
	int2hex(addr->lengthLow, uf) ; 
	printd(uf);
	printd("\nlengthHigh:");
	int2hex(addr->lengthHigh, uf) ; 
	printd(uf);
	printd("\ntype:");
	int2hex(addr->type, uf) ; 
	printd(uf);
}


void printd(char* s){
	
	for( ; *s !='\0' ; s++ ) {
		if(*s == '\n') {
			printd_x = 0 ;
			printd_y += 16 ;
			continue;
		} else {
			putfont(vram ,xsize,  printd_x ,printd_y ,COL8_000000 ,  *s);   	
		}
		printd_x += 8 ; 
		if(printd_x == xsize ) {
			printd_x = 0 ;
			printd_y += 16 ;
		}		
	} 
}
void printi(int i ) {
	char uf[9] = {0} ; 
	int2hex(i, uf) ; 
	printd(uf) ; 
}
void printx(char c){
	char buf[3] = {0} ; 
	toHex(c , buf) ;
	printd(buf) ; 	 	
}

void set_palette(int start , int end , unsigned char* rgb) {
	int i , eflags ;
	eflags = load_eflags();
	cli();
	outb_p(0x03c8 , start); //set palette number
	for(i = start ; i <= end ; i++ ) {
		outb_p(0x03c9,rgb[0]/4);
		outb_p(0x03c9,rgb[1]/4);
		outb_p(0x03c9,rgb[2]/4);
		rgb = rgb + 3 ;
	}
	store_eflags(eflags);
}

void init_palette(void){
	static  unsigned char table_rgb[16 *3] = {
         0x00,  0x00,  0x00,
         0xff,  0x00,  0x00,
         0x00,  0xff,  0x00,
         0xff,  0xff,  0x00,
         0x00,  0x00,  0xff,
         0xff,  0x00,  0xff,
         0x00,  0xff,  0xff,
         0xff,  0xff,  0xff,
         0xc6,  0xc6,  0xc6,
         0x84,  0x00,  0x00,
         0x00,  0x84,  0x00,
         0x84,  0x84,  0x00,
         0x00,  0x00,  0x84,
         0x84,  0x00,  0x84,
         0x00,  0x84,  0x84,
         0x84,  0x84,  0x84
     };
	set_palette(0 ,15 , table_rgb);

}


/*
void putblock(int px , int py , char *buf);
void putblock(int px , int py , char *buf) {
	for(int y = 0 ; y < 16  ; y++ ){
		for(int x = 0 ; x < 16 ; x++ ) {
			vram[( y + py )*xsize + x + px ] = buf[y * 16 + x] ; 
		}
	}	

}
*/
void int2hex(unsigned int i , char* buf) {
     char* _t = "0123456789ABCDEF" ;
     unsigned int mask = 0x0F ;
     int pos  = 8 ; 
     do {
         buf[--pos] = _t[ i & mask] ;
         i >>= 4 ; 
    }while(pos) ;
}

void showString_sht(struct SHEET *sht ,int x , int y , int c , int b , char* s , int l ) {
		boxfill(sht->buf , sht->bxsize , b , x , y  , x + l*8 - 1 , y + 15) ;  
		showString(sht->buf , sht->bxsize , x , y , COL8_000000 , s) ; 
		sheet_refresh(sht , x , y , x + l*8  , y + 16) ;
		return ;  
}
