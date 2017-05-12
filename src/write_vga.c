#define  COL8_000000  0
#define  COL8_FF0000  1
#define  COL8_00FF00  2
#define  COL8_FFFF00  3
#define  COL8_0000FF  4
#define  COL8_FF00FF  5
#define  COL8_00FFFF  6
#define  COL8_FFFFFF  7
#define  COL8_C6C6C6  8
#define  COL8_840000  9
#define  COL8_008400  10
#define  COL8_848400  11
#define  COL8_000084  12
#define  COL8_840084  13
#define  COL8_008484  14
#define  COL8_848484  15

typedef unsigned char uchar ;

void io_hlt();
void init_palette(void);
void io_cli(void);
void io_out8(int port , int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void boxfill8( unsigned char c, int x0, int y0,int x1, int y1);
void putfont( unsigned char c, int x, int y , char* fp) ; 
void drawFont( unsigned char c , int x , int y  , char f);
void showString(uchar c , int , int y , char* s) ;

extern char systemFont[4096] ;
char* vram = (char*)0xa0000 ; 
int xsize  = 320 ;
int ysize  = 200 ; 

void cmian(void){
	
	init_palette();

	boxfill8( COL8_008484, 0, 0, xsize-1, ysize-29);
	boxfill8( COL8_C6C6C6, 0, ysize - 28 , xsize-1, ysize-28);
	boxfill8( COL8_FFFFFF, 0, ysize - 27 , xsize-1, ysize-27);
	boxfill8( COL8_C6C6C6, 0, ysize - 26 , xsize-1, ysize-1);

	boxfill8( COL8_FFFFFF, 3, ysize - 24 , 59, ysize-24);
	boxfill8( COL8_FFFFFF, 2, ysize - 24 , 2, ysize-4);
	boxfill8( COL8_848484, 4, ysize - 4 , 59, ysize-4);
	boxfill8( COL8_848484, 59, ysize - 23 , 59, ysize-5);
	boxfill8( COL8_000000, 2, ysize - 3 , 59, ysize-3);
	boxfill8( COL8_000000, 60, ysize - 24 , 60, ysize-3);
	
	boxfill8( COL8_848484, xsize -47, ysize - 24 , xsize-4, ysize-24);
	boxfill8( COL8_848484, xsize -47, ysize - 23 , xsize-47, ysize-3);
	boxfill8( COL8_FFFFFF, xsize -47, ysize - 3 , xsize-4, ysize-3);
	boxfill8( COL8_FFFFFF, xsize -3, ysize - 24 , xsize-3, ysize-3);

	showString(COL8_FFFFFF , 16 , 16 , "hello green os !");
	showString(COL8_FFFFFF , 8 , 0 , "welcome !");
	
	for(;;) {
		io_hlt();
	}
}

void set_palette(int start , int end , unsigned char* rgb) {
	int i , eflags ;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8 , start); //set palette number
	for(i = start ; i <= end ; i++ ) {
		io_out8(0x03c9,rgb[0]/4);
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);
		rgb = rgb + 3 ;
	}
	io_store_eflags(eflags);
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

void boxfill8( unsigned char c, int x0, int y0, int x1 , int y1){
	int x, y ;
	for(y = y0 ; y <= y1 ; y++ ){
		for(x = x0 ; x <= x1 ; x++){
			vram[y*xsize + x] = c ; 
		}
	}

}

void putfont(unsigned char color ,  int x, int y , char* font){
	//8 * 16
	int i = 0 ; 
	char* p ;	
	for( ; i < 16  ; i ++ ) {
		char c = font[i] ;
		p = vram + (y + i ) *xsize  + x ;
		if(c & 0x80 ) p[0] = color ;
		if(c & 0x40 ) p[1] = color ;
		if(c & 0x20 ) p[2] = color ;
		if(c & 0x10 ) p[3] = color ;
		if(c & 0x08 ) p[4] = color ;
		if(c & 0x04 ) p[5] = color ;
		if(c & 0x02 ) p[6] = color ;
		if(c & 0x01 ) p[7] = color ;
	}
	
}

void drawFont( unsigned char c , int x , int y  , char f){
	putfont( c , x , y , systemFont + f * 16 );   	
}

void showString(uchar color , int x ,int y , char* pf ){
	for(; *pf != '\0' ; pf++ ) {
		drawFont(color , x , y ,*pf);
		x = x + 8 ; 
	}
}


