#include <graphx.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <ti/vars.h>    
#include <keypadc.h>
#include <sys/util.h>
#include <sys/timers.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>      
#include <stdio.h>     
#include <fileioc.h>
#include "gfx/gfx.h"
#include <ctime>
#include <tice.h>
#include <algorithm>
#include <usbdrvce.h>
//mincraft
//uint8_t vis_map[64][16][64];
int chunkx=0;
int chunky=0;
int chunkz=0;



#define WX 32
#define WY 32
#define WZ 32

#define XJ (WY*WZ)
#define YJ WZ

#define XWS 10
#define YWS 5

#define vis_map(x, y, z) world[((x) << XWS) + ((y) << YWS) + (z)]

#define VD 15
//view distance. i recommend between 8 and 20. is measured in taxicab (manhattan) distance from player.

#define res 8
//res must be divisible by 320 can be 1,2,4,5,8,10,16,etc. ALSO, IN THE ASSEMBLY, YOU HAVE TO DO STUFF TOO!
// i recommend 8 for visuals, 10 for speed. maybe 5 for really good visuals but I'd decrease VD.
//here's a good table for approximate values.
/*
VD 15 (medium view distance):

	res 10 = 7.5 fps
	res 8  = 5 fps
	res 5  = 1.8 fps
	res 4  = 1.2 fps
	res 2  = 1/3 fps
	res 1  = 1/12 fps
	
VD 8 (low view distance):

	res 10 = 10 fps
	res 8  = 6.4 fps
	res 5  = 2.7 fps
	res 4  = 1.7 fps
	res 2  = 1/2 fps
	res 1  = 1/9 fps
*/
#define AVERAGEFPS 5
//#define BG_COLOR 191
uint8_t BG_COLOR=125;


#define INTERACTD 4
#define TREENUM 10
#define IRONNUM 600
#define DIAMONDNUM 40
#define GOLDNUM 90
#define TUNNELNUM 5
#define TUNNELWOBBLEF 10
#define TUNNELWOBBLEU 5
#define SANDNUM 10

#define GRASS 1
#define STONE 2
#define WOOD 3
#define LEAF 4
//barrier is 5
#define PLANKS 6
#define IRON 7
#define GOLD 8
#define DIAMOND 9
#define SAND 10
#define WOOL 11
#define block_num 28
#define reg_block_num 12
#define item_num 18
#define RECIPIENUM 14

#define invenblocknum (TEXTURENUM+reg_block_num)
#define WOODPIC (invenblocknum+1)
#define STONEPIC (invenblocknum+2)
#define GOLDPIC (invenblocknum+3)
#define IRONPIC (invenblocknum+4)
#define DIAMONDPIC (invenblocknum+5)
#define NETHERITEPIC (invenblocknum+6)
#define STICK (invenblocknum+7)
#define WOODSWORD (invenblocknum+8)
#define STONESWORD (invenblocknum+9)
#define GOLDSWORD (invenblocknum+10)
#define IRONSWORD (invenblocknum+11)
#define DIAMONDSWORD (invenblocknum+12)
#define IRONINGOT (invenblocknum+13)
#define GOLDINGOT (invenblocknum+14)
#define RAWMUTTON (invenblocknum+15)
#define COOKEDMUTTON (invenblocknum+16)
#define ROTTENFLESH (invenblocknum+17)

#define CRAFTTABLE (reg_block_num+1)
#define FURNACE (reg_block_num+2)
#define PLANKSTXT (reg_block_num+3)
#define BEDTOP (reg_block_num+4)
#define GLASS (reg_block_num+5)
#define DOORTOPOPEN (reg_block_num+6)
#define DOORBOTTOMOPEN (reg_block_num+7)
#define DOORTOPCLOSED (reg_block_num+8)
#define DOORBOTTOMCLOSED (reg_block_num+9)

#define TEXTURENUM 9
#define SKELETOPX (reg_block_num+TEXTURENUM+1)
#define SKELETOPZ (reg_block_num+TEXTURENUM+2)
#define SKELEBOTTOM (reg_block_num+TEXTURENUM+3)
#define SHEEPFACEX (reg_block_num+TEXTURENUM+4)
#define SHEEPFACEZ (reg_block_num+TEXTURENUM+5)
#define SHEEPBACKX (reg_block_num+TEXTURENUM+6)
#define SHEEPBACKZ (reg_block_num+TEXTURENUM+7)
#define ZOMBIETOPX (reg_block_num+TEXTURENUM+8)
#define ZOMBIETOPZ (reg_block_num+TEXTURENUM+9)
#define ZOMBIELEGS (reg_block_num+TEXTURENUM+10)

#define MOBNUM (10)
#define MOBSTART SKELETOPX

#define SHEEPNUM 10
#define ZOMBIENUM 7
//static uint8_t noninventory_blocks={DOORBOTTOMCLOSED,DOORBOTTOMOPEN,DOORTOPCLOSED,SKELEBOTTOM,SKELETOPX,SKELETOPZ,SHEEPFACEX,SHEEPFACEZ,SHEEPBACKX,SHEEPBACKZ}
//Error with reg block num  +1 to ^ ^ ^
static uint8_t world[WX*WY*WZ];
//static uint8_t* world =(uint8_t*)0xD09466;
static unsigned short* world_changes[32];//array of short*. each array of short* is block moved.
static unsigned short* prev_world_changes[32];//prev
int focal_length=200;//??? guess with this one ;)
// smaller = bigger fov but bigger distortion. BEWARE OF THE FISHEYE!!!




struct point{
	int x=0;
	int y=0;
	int z=0;
};
struct point2d
{
	int x=0;
	int y=0;
};
int wseed=0;
#define shift_by 9
struct flint//like float plus int
{       
	/** beautiful fixed precision class! I wrote first half, got really bored, and AI wrote the second half of this class!
	shift_by has to be 9 because anything lower and, ummm, random lines and things start appearing on the screen (impresicion in raycast screen)
	Believe me, if I could make it 8, i would.**/
	flint() {}                 // default
	flint(const flint& other) = default;
    flint(int other) { data = other << shift_by; } // from int
    flint(float other) { data = (int)(other * (1 << shift_by)); } // from float
	int data=0;// prob 24 bits. 4 digit in decimal, 5 in int 100000.10010 +10000 -> 10000010010
    inline flint& operator+=(int other) {
        data+=other<<shift_by;
		return *this;
    }
	inline flint& operator+=(flint other) {
        data+=other.data;
		return *this;
    }
	
	inline flint& operator-=(int other) {
        data-=other<<shift_by;
		return *this;
    }
	
	inline flint& operator-=(flint other) {
        data-=other.data;
		return *this;
    }
	
	inline flint& operator*=(int other) {
        data*=other;
		return *this;
    }
	inline flint& operator*=(flint other) {
        //long temp= (long)data*other.data;
		//data=(int)(temp>>shift_by);
		data=data*other.data>>shift_by;
		return *this;
    }
	inline flint& operator/=(int other) {
        data/=other;
		return *this;
    }
	inline flint& operator/=(flint other) {
        long temp= (long)data<<shift_by;
		data=(int)(temp/other.data);
		return *this;
    }
	inline flint& operator=(int other) {
        data=other<<shift_by;
		return *this;
    }
	inline flint& operator=(flint other) {
        data=other.data;
		return *this;	
    }
	inline operator int() const
	{
		return data>>shift_by;
	}
	inline operator float() const
	{
		return (float)(data)/(1<<shift_by);//WARNING THIS IS EVEN SLOWER!
	}
	
	inline bool operator>(flint other) const { return data > other.data; }
	inline bool operator<(flint other) const { return data < other.data; }
	inline bool operator==(flint other) const { return data == other.data; }
	inline bool operator>=(flint other) const { return data >= other.data; }
	inline bool operator<=(flint other) const { return data <= other.data; }
	inline bool operator!=(flint other) const { return data != other.data; }
	
	// Shift the int UP, don't shift the flint DOWN
	inline bool operator>(int other) const { return data > (other << shift_by); }
	inline bool operator<(int other) const { return data < (other << shift_by); }

	inline bool operator==(int other) const { return (data>>shift_by) == other; }
	inline bool operator>=(int other) const { return (data>>shift_by) >= other; }
	inline bool operator<=(int other) const { return (data>>shift_by) <= other; }
	inline bool operator!=(int other) const { return (data>>shift_by) != other; }
	inline flint& float2flint(const float other)
	{
		data = (int)(other * (1 << shift_by) + (other >= 0 ? 0.5f : -0.5f));//WARNING THIS IS SLOW!
		return *this;
	}
	inline flint flint_abs() const {flint temp; temp.data=abs(data);return temp;}
	inline flint& self_abs() {data=abs(data);return *this;}
	inline int decimal_part() const {return (data<<(24-shift_by))>>(24-shift_by);}
	inline flint operator+(flint other) const { flint result = *this; return result += other; }
    inline flint operator-(flint other) const { flint result = *this; return result -= other; }
    inline flint operator*(flint other) const { flint result = *this; return result *= other; }
    inline flint operator/(flint other) const { flint result = *this; return result /= other; }
	
	inline flint operator-() const {flint o; o.data=-data;return o; }
    // Overloads for raw integers
    inline flint operator+(int other) const { flint result = *this; return result += other; }
    inline flint operator-(int other) const { flint result = *this; return result -= other; }
    inline flint operator*(int other) const { flint result = *this; return result *= other; }
    inline flint operator/(int other) const { flint result = *this; return result /= other; }
	
};
flint focal_inv;
flint half=.5f;
/**Lots of caching! xdir and y dir are ray directions for each pixel! (by ray and column, NOT always equal distance from orgin)**/
static flint cached_x_dir[320];//direction to ray trace from. picture direction from camera line to screen. o-._|
static flint cached_y_dir[240];// same thing but for y

/** inv versions are 1/direction in the above arrays**/
static flint cached_x_dir_inv[320];
static flint cached_y_dir_inv[240];

/**faster sine/cosine for speed purposes in raycast screen. There are 64 x and y rotations for the camera 
(although i decided it was too slow for the x so now it increments by 2 [so really 32] ) so it is perfect length! (still 64 in y, though)**/
static flint cached_sin_rot[64];
static flint cached_cos_rot[64];

/** for saving purposes, see update_changes for more detail**/
static int prev_world_changes_sizes[32];
static int world_changes_sizes[32];


int cached_x_signs[320];

inline int sign(int n) {
    return 1 | (n >> 23);
}
flint dist_from_one(flint x)
{
	return (x-(int)(x+half*sign(x.data))).flint_abs();
}
inline void rot_point_2d(flint& x1,flint&y1,flint x2, flint y2,int angle)//angle between 0 and 32.
{
	flint nx;nx=x1-x2;
	flint ny;ny=y1-y2;
	x1=x2+(cached_cos_rot[angle]*nx-cached_sin_rot[angle]*ny);
	y1=y2+(cached_sin_rot[angle]*nx+cached_cos_rot[angle]*ny);
}
/*
inline void thick_line(int x0,int y0, int x1, int y1, int width)
{
	for(int i=0; i<width; ++i)
	{
		gfx_Line(x0,y0+i,x1,y1+i);
	}
}
inline void draw_pickaxe(int x, int y, uint8_t color)//draws a pickaxe
{
	//items are 30 by 30
	gfx_SetColor(64);
	thick_line(x,y+24,x+29,y,5);//hilt
	gfx_SetColor(color);
	gfx_FillRectangle(x+8,y,x+29,y+5);//first part of... head? (or whatever its called)
	gfx_FillRectangle(x+24,y,x+29,y+21);
}
inline void draw_sword(int x, int y, uint8_t color)//draws a sword
{
	gfx_SetColor(65);
	thick_line(x,y+13,x+13,y,5);//hilt
	thick_line(x,y+14,x+13,y+25,5);//hilt (but cross part)
	gfx_SetColor(color);
	thick_line(x+13,y+25,x+29,y+29,5);
}
*/
inline void rot_point_3d(flint&x1,flint&y1,flint&z1,flint cosanglexz, flint sinanglexz, flint cosangleyz,flint sinangleyz)
{
	/** more relics from a past slower raycast screen function**/
	//flint foo;foo=z1;
	//rot_point_2d(x1,z1,x2,z2,anglexz);
	//rot_point_2d(y1,foo,y2,z2,angleyz);
	flint ny;ny=y1;
	flint nz;nz=z1;
	y1=(cosangleyz*ny-sinangleyz*nz);	
	nz=(sinangleyz*ny+cosangleyz*nz);//yz
	
	flint nx;nx=x1;
	x1=(cosanglexz*nx-sinanglexz*nz);
	z1=(sinanglexz*nx+cosanglexz*nz);//xz
}

flint point01;
flint one=1;
flint small=.001f;
const flint step_divide=1;

static flint inv_table[1024];

/*
volatile int foo;// I forget what these do. Maybe temporary variables?
volatile int foo1;
volatile int foo2;
*/
int o;
flint plx=1;
flint ply=16;
flint plz=1;
int rotxz=0;
int rotyz=0;
inline int fast_abs(int x)/**suuuper speeedddyyyy abs**/
{
	
	int mask = x >> 23;// fills with sign bit
	return (x ^ mask) - mask;// does 2's compliment if negative.
	// if mask is 0 x^0 is x, if mask is all 1's x^0=!x. and x-all ones is x--1 or x+1.
}

//const int end_world=(int)world+WX*WY*WZ;
inline int fast_copysign_pos(int val, int src) {
	/**is a fast copy sign but only for positive numbers... I only use it for positive numbers though so it works.**/
    int mask=src>>23;//basically turns it into all 1s if pos, all 0s if neg
	return (val^mask)-mask;// inverts it if negitive (the xor) and then does --1 (if the flag was -1 it would be inverted). (+1) so 2's compliment
	
}
inline uint8_t * cross_hair_pt(uint8_t*& blockbefore)// pl is player, p is point rayracing.
{
	/** basically raycast point function below but gets the block (and previous block) where the crosshair is.
	block where the crosshair is pointing is used for using and breaking, block before is used to for placing blocks**/
	int ix=plx;
	int iy=ply;
	int iz=plz;
    // Get directions from cache
    flint dx = cached_x_dir[160];
    flint dy = cached_y_dir[120];
    flint dz = 1; // Looking forward
	flint cosxz=cached_cos_rot[rotxz];
	flint sinxz=cached_sin_rot[rotxz];
	
	flint cosyz=cached_cos_rot[rotyz];
	flint sinyz=cached_sin_rot[rotyz];

	rot_point_3d(dx,dy,dz,cosxz,sinxz,cosyz,sinyz);
	//rot_point_3d(idx,idy,one,0,0,0,anglexz,angleyz);
	flint idx = inv_table[fast_abs(dx.data)];//dx.data is << by 13. inv_table is >>7. 13-7=6
	
    flint idy = inv_table[fast_abs(dy.data)];
	
	flint idz = inv_table[fast_abs(dz.data)];
	
	//idy=-idy;
	
	flint deltax=idx;// dx is (how much dist)for (1 unit), idx is (1 unit) for (how much length)
	flint deltay=idy;//same with idy,idz
	flint deltaz=idz;
	
	int signdx=fast_copysign_pos(XJ,dx.data);
	int signdy=fast_copysign_pos(YJ,dy.data);
	int signdz=fast_copysign_pos(1,dz.data);
	flint tmaxx=deltax;// dist to next line *deltax. start on int coord, so dist to next line is 1
	flint tmaxy=deltay;
	flint tmaxz=deltaz;

	uint8_t *curr_block = world+((ix) << XWS) + ((iy) << YWS) + (iz);
	for (int i=0; i<INTERACTD; ++i)
	{
		blockbefore=curr_block;//kinda slow, i know. don't really use this fucntion much tho
		if (tmaxx<tmaxy){
			if (tmaxx<tmaxz){//tmaxx is smallest
				curr_block+=signdx;
				tmaxx+=deltax;
			}
			else
			{//tmaxx ___ tamxy, so tmaxz
				curr_block+=signdz;
				tmaxz+=deltaz;
			}
		}
		else{ 
			if (tmaxy<tmaxz){//tmaxx is smallest
				curr_block+=signdy;
				tmaxy+=deltay;
			}
			else{
				curr_block+=signdz;
				tmaxz+=deltaz;
			}
		}
		//asm("ld a,b\n "
		//"ld (_ix), a");

		if ((uint24_t)(curr_block - world) < WX*WY*WZ)//saying if <0, wrap to big val and if to big is not <
		{
			if (*curr_block!= 0){
				//dbg_printf("delte func wrks!,%d\n",*curr_block);
				return curr_block;
				
			}
		}
	}
	return NULL;// if no block
}
inline int ray_trace_point(const int pixelx,const int pixely, uint8_t* w, flint cosanglexz, flint sinanglexz, flint cosangleyz, flint sinangleyz)// pl is player, p is point rayracing.
{
	/** old function i used to draw stuff before i manually inlined it into raycast screen (so it would be faster), but still am keeping it for some reason... maybe i will need it later**/
    // Get directions from cache
    flint dx = cached_x_dir[pixelx];
    flint dy = cached_y_dir[pixely];
    flint dz = 1; // Looking forward
	
	rot_point_3d(dx,dy,dz,cosanglexz,sinanglexz,cosangleyz,sinangleyz);
	//rot_point_3d(idx,idy,one,0,0,0,anglexz,angleyz);
	flint deltax= inv_table[fast_abs(dx.data)];	
    flint deltay= inv_table[fast_abs(dy.data)];// had idx,idy,idz before
	flint deltaz = inv_table[fast_abs(dz.data)];
	
	flint tmaxx=deltax;// dist to next line *deltax. start on int coord, so dist to next line is 1
	flint tmaxy=deltay;
	flint tmaxz=deltaz;
	
	//flint deltax=idx;// dx is (how much dist)for (1 unit), idx is (1 unit) for (how much length)
	//flint deltay=idy;//same with idy,idz
	//flint deltaz=idz;
	unsigned int signdx=fast_copysign_pos(XJ,dx.data);
	unsigned int signdy=fast_copysign_pos(YJ,dy.data);
	unsigned int signdz=fast_copysign_pos(1,dz.data);


	uint8_t* curr_block = w;
	int dlocal=0;
	#pragma clang loop unroll(full)
	for (int i=0; i<VD; ++i)
	{
		if (tmaxx<tmaxy){
			if (tmaxx<tmaxz){//tmaxx is smallest
				curr_block+=signdx;
				tmaxx+=deltax;
				dlocal=0;//0<<8;
			}
			else
			{//tmaxx ___ tamxy, so tmaxz
				curr_block+=signdz;
				tmaxz+=deltaz;
				dlocal=2<<8;
			}
		}
		else{ 
			if (tmaxy<tmaxz){//tmaxx is smallest
				curr_block+=signdy;
				tmaxy+=deltay;
				dlocal=256;//1<<8;
			}
			else{
				curr_block+=signdz;
				tmaxz+=deltaz;
				dlocal=512;//2<<8;
			}
		}
		//asm("ld a,b\n "
		//"ld (_ix), a");

		//if ((uint24_t)(curr_block - base_world) < WX*WY*WZ)//saying if <0, wrap to big val and if to big is not <
			if (*curr_block){// if it is not 0
				return (int)(*curr_block)|(dlocal);
			}
	}
    return 0;
}
/** colors for solid blocks**/
//                                           NONE,grass,stone,wood,leaf,border,planks,iron,gold,diamond,sand,wool
static const uint8_t block_colors_x[reg_block_num]={0,130,   140,  65,   6   ,BG_COLOR,237, 162,229,   30,205,214};//darker
static const uint8_t block_colors_z[reg_block_num]={0,131,   139,  65,   7   ,BG_COLOR,238, 194,230,   31,238,254};//lighter
//top                                        NONE,grass,stone,wood,leaf,border,planks,iron
static const uint8_t block_colors_y[reg_block_num]={0, 5,   172,  238,   103 ,BG_COLOR,239, 195,231,   159,239,255};//lightest																							       
static const uint8_t hardness[block_num]=      {0,  1,    2,    1,    0,   255,    1,  3,    4,     4,  1, 1, /*textures*/0,1, 1,2,1,1,1,1,1,/*mobs*/255,255,255,255,255,255,255};
static uint8_t *colors;
//97
/** beautiful textures! I designed them in ms paint then put them in this amazing format (for some reason
they get like mirrored really weirdly but i explain that later, but what is important now is that the reason these arrays
are unintelligible is because i am mirroring them so they get unmirrored when rendered.)**/
static const uint8_t craftingtabletop[64]=
{
	238,64,238,238,238,238,64,238,
	64 ,64 ,64, 64, 64,64, 64, 64,
	238,64,238,238,238,238,64,238,
	238,64,238,238,238,238,64,238,
	238,64,238,238,238,238,64,238,
	238,64,238,238,238,238,64,238,
	64 ,64 ,64, 64, 64,64, 64, 64,
	238,64,238,238,238,238,64,238
};
static const uint8_t craftingtableside[64]=
{
	238,238,238,238,238,238,238,238,
	65, 65 ,65 ,65 ,65 ,65 ,65 ,65 ,
	65, 65 ,65 ,65 ,65 ,65 ,65 ,65 ,
	238,238,238,238,238,238,238,238,
	238,238,238,238,238,238,238,238,
	65, 65 ,65 ,65 ,65 ,65 ,65 ,65 ,
	65, 65 ,65 ,65 ,65 ,65 ,65 ,65 ,
	238,238,238,238,238,238,238,238,
};
static const uint8_t furnacefront[64]=
{
	107,107,107,107,107,107,107,107,
	0  ,0  ,107,107,107,107,0  ,0  ,
	0  ,0  ,0  ,107,107,0  ,0  ,0  ,
	107,107,107,107,107,107,107,107,
	172,172,172,172,172,172,172,172,
	0  ,0  ,172,172,172,172,0  ,0  ,
	0  ,0  ,0  ,172,172,0  ,0  ,0  ,
	172,172,172,172,172,172,172,172
};
static const uint8_t cobblestone[64]=
{
	74 ,107,74 ,107,107,74 ,107,74 ,
	74 ,222,107,74 ,74 ,74 ,107,74 ,
	222,222,74 ,74 ,74 ,222,222,107,
	74 ,107,74 ,107,107,222,107,74 ,
	74 ,222,222,107,107,74 ,107,74 ,
	107,222,107,74 ,74 ,74 ,74 ,74 ,
	74 ,107,74 ,74 ,107,74 ,222,222,
	74 ,107,74 ,107,74 ,107,74 ,222,
};
static const uint8_t bedtop[64]
{
	221,221,24,24,24,24,221,221,
	255,255,221,24,24,221,255,255,
	255,255,255,221,221,255,255,255,
	255,255,255,221,221,255,255,255,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24
};
static const uint8_t bedsidetop[64]=
{
	26,26,26,26,97,97,26,26,
	26,26,26,26,97,97,26,26,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24,
	26,26,26,26,97,97,26,26,
	26,26,26,26,97,97,26,26,
	26,26,26,26,97,97,26,26,
	26,26,26,26,97,97,26,26,
};
/*
static const uint8_t bedsidebottom[64]=
{
	26,26,97,97,26,26,26,26,
	26,26,97,97,26,26,26,26,
	26,26,97,97,26,26,26,26,
	26,26,97,97,26,26,26,26,
	26,26,97,97,26,26,26,26,
	26,26,97,97,26,26,26,26,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24,
};*/
static const uint8_t bedsidefront[64]=
{
	26,26,97,97,97,97,26,26,
	26,26,97,97,97,97,26,26,
	26,26,97,97,97,97,26,26,
	26,26,97,97,97,97,26,26,
	26,26,97,97,97,97,26,26,
	26,26,97,97,97,97,26,26,
	24,24,24,24,24,24,24,24,
	24,24,24,24,24,24,24,24,
};
static const uint8_t glass[64]=
{
	26,26,26,255,255,26,26,26,
	26,255,26,255,255,26,26,26,
	26,26,26,255,255,26,26,26,
	255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,
	26,26,26,255,255,26,26,26,
	26,26,26,255,255,26,255,26,
	26,26,26,255,255,26,26,26,
};
static const uint8_t doorfronttop[64]=
{
	163,163,163,163,163,163,163,163,
	163,26,26,163,163,26,26,163,
	163,26,26,163,163,26,26,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,26,26,163,163,26,26,163,
	163,26,26,163,163,26,26,163,
	163,163,163,163,163,163,163,163
};
static const uint8_t brown[64]=
{
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
	163,163,163,163,163,163,163,163,
};
static const uint8_t clear[64]=
{
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
	26,26,26,26,26,26,26,26,
};
static const uint8_t doorarch[64]=
{
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	163,163,163,163,163,163,163,163,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
};
static const uint8_t doorbottomarch[64]=
{
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
	26,26,26,163,163,26,26,26,
};
static const uint8_t skeletopf[64]=
{


	0,0,0,214,214,0,0,0,
	106,214,214,214,214,214,214,106,
	214,0 , 0  ,214,214,0   ,0 ,214,
	214,214,214,214,214,214,214,214,
	
	97,214,255,107,107,214,107,26,
	26,97,214,255,107,26,26,107,
	107,214,97,97,107,107,214,26,
	214,26,107,107,107,214,107,214,
};
static const uint8_t skeletope[64]=
{
	

	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	
	214,214,26,107,107,214,107,26,
	26,107,214,107,107,26,26,107,
	107,214,26,107,107,107,214,26,
	214,26,107,107,107,214,107,214,
};
static const uint8_t skelebottom[64]=
{	
	26,107,97 ,97 ,26 ,107 ,107,26,
	26,97 ,107,255,26 ,107 ,107,26,
	97,107,255,26 ,26 ,107 ,107,26,
	107,107,107,107,107,107,107,107,
	
	26,107,107,26 ,26 ,107 ,107,26,
	26,107,107,26 ,26 ,107 ,107,26,
	26,107,107,26 ,26 ,107 ,107,26,
	26,107,107,26 ,26 ,107 ,107,26,
};
static const uint8_t grey[64]=
{
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
	214,214,214,214,214,214,214,214,
};
static const uint8_t sheepface[64]=
{	
	244,204,204,255,    255,204,204,244,
	204,255,0  ,255,    255,0  ,255,204,
	204,204,204,255,    255,204,204,204,	
	255,255,255,255,    255,255,255,255,
	
	26,107,107,26,      26,107,107,26,
	26,107,107,26,      26,107,107,26,
	255,255,255,255,   255,255,255,255,
	171,204,255,255,   255,255,204,171,
	
};
static const uint8_t sheepside[64]=
{
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	
	26,26,26,26,        26,26,26,26,
	26,26,26,26,        26,26,26,26,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,

};
static const uint8_t sheepback[64]=
{
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,
	
	26,107,107,26,      26,107,107,26,
	26,107,107,26,      26,107,107,26,
	255,255,255,255,    255,255,255,255,
	255,255,255,255,    255,255,255,255,

};
static const uint8_t zombieface[64]=
{36, 45, 45, 45, 45, 45, 45,  36
, 45,  0,  0, 45, 45,  0,  0, 45
, 45,  0,  0, 45, 45,  0,  0, 45
, 45, 45, 45, 45, 45, 45, 45, 45
,158,158, 45, 45, 45, 45,158,158
,158,158, 45, 45, 45, 45,158,158
,158,158,158,158,158,158,158,158
,158,158,158,158,158,158,158,158
};
static const uint8_t zombiehead[64]=
{45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
,158,158,158,158,158,158,158,158
,158,158,158,158,158,158,158,158
,158,158,158,158,158,158,158,158
,158,158,158,158,158,158,158,158};

static const uint8_t zombielegs[64]=
{58, 58, 58,255,255, 58, 58, 58
,158,158, 45, 45, 45, 45,158,158
,158,158, 45, 45, 45, 45,158,158
,158,158, 45, 45, 45, 45,158,158
,115,115,115,26 ,26 ,115,115,115
, 58, 58, 58,26 ,26 , 58, 58, 58
, 58, 58, 58,26 ,26 , 58, 58, 58
, 58, 58, 58,26 ,26 , 58, 58, 58
};
static const uint8_t zombieheadtop[64]=
{45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
, 45, 45, 45, 45, 45, 45, 45, 45
};
// this v v v  is an array storing what sides of texture blocks go with what textures ^ ^ ^
//syntax is x axis, top, z axis, bottom(maybe).                  crafting table                                             furnace                                             planks                                                               bed top                         glass                               doortopopen                               doorbottomeopen             doortopclosed                         doorbottomclosed          skeletontopx                                skeletontopz                    skeletonbottom                                  sheepfacex                    sheepfacez                      sheepbackx                      sheepbackz                                           zombietopx                                                zombietopz                                            zombielegs
static const uint8_t* texturedata[]={craftingtableside,craftingtabletop,craftingtableside,craftingtableside,   cobblestone,cobblestone,furnacefront,cobblestone,  craftingtableside,craftingtableside,craftingtableside,craftingtableside, bedsidetop,bedtop,bedsidefront,bedtop,   glass, glass,glass,glass,  doorarch,brown,doorfronttop,brown,    doorbottomarch,clear,brown,brown, doorfronttop,brown,doorarch,brown, brown,clear,doorbottomarch,brown,    skeletopf,grey,skeletope,clear,    skeletope,grey,skeletopf,clear,  skelebottom,skelebottom,skelebottom,skelebottom,   sheepface,grey,sheepside,grey,   sheepside,grey,sheepface,grey,     sheepback,grey,sheepside,grey,  sheepside,grey,sheepback,grey,     zombieface,zombieheadtop,zombiehead,zombieheadtop,     zombiehead,zombieheadtop,zombieface,zombieheadtop,      zombielegs,zombieheadtop,zombielegs,zombieheadtop};
static flint cache [640]={0};
bool is_in_crafting_table=false;
static int inventory[block_num+item_num]={0};
void update_changes(int id, uint8_t* block);//see compiler, this function does exist! (i make it below and didn't feel like ctrl+x ing all of this below it v v v 
inline void draw_item(int temp,int x,int y,int show_num);
bool keyup(int key)
{
	if(kb_IsDown(key)){
		while(kb_IsDown(key))
			kb_Scan();
		return true;
	}
	return false;
}
void draw_arrow_nfull(int x,int y,int w,int h,int howfull)
{
	
	gfx_SetColor(74);
	int w2=w>>1;
	int h2=h>>1;
	
	gfx_FillRectangle(x,y+(h2>>1),w2,h2);//1/2 of arrow is going to be rect	

	//now drawing triangle!
	
	//int th=h;//triangle line height = height
	
	//gfx_SetColor(1);
	gfx_SetColor(255);
	for(int x2=0;x2<w2;x2++)//y=mx+b. h=-x+y
	{
		if(howfull<x2+w2)
			gfx_SetColor(74);
		//dbg_printf("    x2: %d, x2+w2: %d\n",x2,x2+w2);
		gfx_VertLine(x+x2+w2,y+(x2>>1),h-x2);
	}
	gfx_SetColor(255);
	//dbg_printf("howfull: %d\n",howfull);
	if(howfull>w2)
		howfull=w2;
	gfx_FillRectangle(x,y+(h2>>1),howfull,h2);
}
/** yeah, i am not gonna explain these in detail v v v Basically just does stuff when the player uses that block**/
void use_craftingtable(uint8_t* block)
{
	is_in_crafting_table=true;// goes into crafting table mode
}
// these open and close doors v v v 
void use_doorto(uint8_t* block)// if clicked on top open door block
{
	 *block+=2;//closes top door
	 update_changes(*block,block);
	 block-=YJ;//down one in y
	 update_changes(*block,block);
	 *block+=2;
}
void use_doortc(uint8_t* block)//if clicked on top closed door block
{
	 *block-=2;//opens top door
	 update_changes(*block,block);
	 block-=YJ;//down one in y
	 update_changes(*block,block);
	 *block-=2;//opens
}
void use_doorbo(uint8_t* block)// if clicked on bottom open door block
{
	 *block+=2;//closes  bottom door
	 update_changes(*block,block);
	 block+=YJ;//up one in y
	 update_changes(*block,block);
	 *block+=2;//closes top
}
void use_doorbc(uint8_t* block)//if clicked on bottom closed door block
{
	 *block-=2;//opens top door
	 update_changes(*block,block);
	 block+=YJ;//up one in y
	 update_changes(*block,block);
	 *block-=2;//opens
}


static const uint8_t fuels[]={WOOD,PLANKS,CRAFTTABLE,WOODPIC,WOODSWORD,STICK};
//fuel times are out of 50, where 50 is one item smelted
static const int fueltimes[]={   75,   75,     75,      50,       50,25};
static const int smeltable[]={SAND,IRON,GOLD,RAWMUTTON};
static const int resulttable[]={GLASS,IRONINGOT,GOLDINGOT,COOKEDMUTTON};
const static gfx_sprite_t *flames[]={emptyfurnaceflames,furnaceflames1,furnaceflames2,furnaceflames3,furnaceflames4,furnaceflames5};
void furnace_stuff(uint8_t* block)//uint8_t* block)//need block because of syntax for array
{
	gfx_SwapDraw();
	flint howfull=0;//you know that arrow in the furnace screen? well this is howfull it is (out of 50).
	uint8_t fuelb=0;//fuel block
	uint8_t cookedb=0;//block that is being cooked
	uint8_t resultb=0;//block that is made as a result
	int cursor=1;
	//uint8_t cursorblock=0;
	uint8_t cursorcolor=230;
	int fuel=0;
	int frame=0;
	int fuelid=0;
	int smeltid=0;
	while(true){
		gfx_SwapDraw();
		gfx_Wait();
		gfx_FillScreen(107);
		gfx_SetDrawBuffer();
		kb_Scan();
		//drawing stuff!
		int x=1;//r. 23. 23/2 = 11.5
		int y=100;
		
		for(int i=1; i<reg_block_num+item_num+TEXTURENUM; ++i)//looping through inventory (inventory is an int[block_num+item_num])
		{
			int foo=inventory[i];
			if(cursor==i)
			{
				gfx_SetColor(cursorcolor);
				gfx_FillRectangle_NoClip(x-1,y-1,32,32);
			}
			if (foo>0&&i!=fuelb && i!=cookedb && i!=resultb)
				draw_item(i,x,y,1);//item/block,x,y,showtext
			else
				draw_item(0,x,y,1);
			x+=32;//r 23.
			if(x>320-30)
			{
				x=1;
				y+=32;
			}
		}
		//dbg_printf("howfull l: %d\n",howfull);
		draw_arrow_nfull(130,30,50,25,howfull);
		
		
		gfx_SetColor(140);		
		gfx_FillRectangle_NoClip(60,0,36,36);
		draw_item(cookedb,63,3,1);
		
		gfx_SetColor(140);
		gfx_FillRectangle_NoClip(60,60,36,36);
		draw_item(fuelb,63,63,1);
		
		gfx_SetColor(140);
		gfx_FillRectangle_NoClip(210,25,36,36);
		draw_item(resultb,213,28,1);
		int foo=(fuel+15)>>4;
		if(foo>6)
			foo=6;
		gfx_Sprite_NoClip(flames[foo],68,38);
		if(frame==0)
		{
			
			if(fuelb!=0&&cookedb!=0)
			{
				if(fuel>0){
					howfull+=1;
				}
				else if(inventory[fuelb]>0)// if fuel is out
				{
					
					fuel=fueltimes[fuelid];
					inventory[fuelb]--;
				}
			}
			if(howfull>49)//finished smelting block
			{
				inventory[resulttable[smeltid]]++;
				howfull=0;
				inventory[cookedb]--;
			}
			if(fuel>0)
			{
				fuel--;
			}
			else
			{
				fuelid=0;
				fuelb=0;
			}
		}
		if(inventory[cookedb]<=0)
			cookedb=0;
		frame++;
		frame%=5;
		//msleep(100);
		//key stuff!
		if(kb_IsDown(kb_KeyClear))
			return;
		if(kb_Data[3] & kb_GraphVar){//xton
			while(kb_Data[3] & kb_GraphVar)
				kb_Scan();// waits till u release it until ending, so doesen't spawn you in crafting menu
			return;
		}
		//length is 9
		if(keyup(kb_KeyRight)){
			cursor++;
		}
		if(keyup(kb_KeyLeft))
			cursor--;
		if(keyup(kb_KeyUp))
			cursor-=10;
		if(keyup(kb_KeyDown))
			cursor+=10;
		if(cursor>invenblocknum+item_num-1)
			cursor=invenblocknum+item_num-1;
		if(cursor<1)
			cursor=1;
		

		if(keyup(kb_Key2nd))
		{
			cookedb=0;
			smeltid=0;
			if(inventory[cursor]>0)
			{
				for (int i=0; i<4; ++i)//how many possible blocks can smelt
				{
					if(smeltable[i]==cursor){
						cookedb=cursor;
						smeltid=i;
						break;
					}
				}
			}
		}
		if(keyup(kb_KeyAlpha))
		{
			fuelb=0;
			fuelid=0;
			if(inventory[cursor]>0)
			{
				for (int i=0; i<6; ++i)
				{
					if(fuels[i]==cursor)
					{
						fuelb=cursor;
						//fuel=fueltimes[i];
						fuelid=i;
						break;
					}
				}
			}
		}		
	}
}
//ZOMBIETOPX
//ZOMBIETOPZ
//ZOMBIELEGS
int8_t playerhp=10;
int8_t playerhunger=20;
struct Zombie
{
	int x=0;
	int y=0;//counting at feet, not head 
	int z=0;
	int facing=0;//0 is x, 1 is z
	int playerinsight=0;
	int hp=20;
	int cooldown=0;
	int alive=false;
	inline void draw()
	{
		vis_map(x,y,z)=ZOMBIELEGS;
		vis_map(x,y+1,z)=ZOMBIETOPX+facing;//+0 makes it x, +1 makes it z.
	}
	inline void clear()
	{
		vis_map(x,y,z)=0;
		vis_map(x,y+1,z)=0;
	}
	inline void spawn()
	{
		x=randInt(1,WX-1);
		z=randInt(2,WZ-2);
		y=WY-1;
		while(vis_map(x,y-1,z)==0)
			y--;
		draw();
	}
	inline void move()
	{
		clear();
		bool flag=1;
		int prevx=x;
		int prevy=y;
		int prevz=z;
		int iplx=(int)plx;
		int iplz=(int)plz;
		int step=0;
		if(playerinsight)
		{
			dbg_printf("playerinsight!");
			if(fast_abs(x-iplx)>fast_abs(z-iplz))//movex
			{
				notelse:
				step=sign(iplx-x);
				if(vis_map(x+step,y+1,z)==0)//head block will be empty
				{
					x+=step;
					facing=0;
					if(vis_map(x,y,z))
					{
						if(vis_map(x,y+2,z)==0)//jump
							y++;
						else
						{
							x-=step;
							if(flag){
								flag=0;
								goto alsoelse;
								
							}
						}
					}
				}
				else
				{
					if(flag)
					{
						flag=0;
						goto alsoelse;						
					}
				}
			}
			else//movez
			{
				alsoelse:
				int step=sign(iplz-z);
				if(vis_map(x,y+1,z+step)==0)
				{
					
					z+=step;
					facing=1;
					if(vis_map(x,y,z))//fit went underground
					{
						dbg_printf("feet went underground!");
						if(vis_map(x,y+2,z)==0)//if can jump
						{
							y++;
							dbg_printf("jumped!");
						}
						else//if can't jump
						{
							z-=step;
							if(flag)
							{
								flag=0;
								goto notelse;
							}
						}
					}
				}
				else if(flag)
				{
					flag=0;
					goto notelse;					
				}
			}
		}
		else
		{
			if(randInt(0,3)==0)
			{
				facing=!facing;
				int dist=fast_abs(plx-x)+abs(plz-z);
				if(dist<12)
					playerinsight=1;
			}
		}
		if(cooldown)
			cooldown--;
		if(iplx==x&&iplz==z&&fast_abs((ply-1)-y)<2)//went into player. (ply-1) because player counts at head, not feet
		{			
			if(cooldown==0)
			{
				playerhp--;
				cooldown=AVERAGEFPS;
			}
			x=prevx;
			y=prevy;
			z=prevz;
		}
		if(vis_map(x,y-1,z)==0)
			y--;
		draw();
		
	}

};
struct Sheep {
    int facingx=0;//either going to be 0, 1 or -1
	int facingz=0;//0,1,-1
    int x=0;//coords of head block
	int y=0;
	int z=0;
	
	int hp=8;
	inline void fall_down()
	{
		while(vis_map(x,y-1,z)==0&&vis_map(x-facingx,y-1,z-facingz)==0) //this makes it appear on the bottomest block
			y--;
	}
	inline void fall()
	{
		if(vis_map(x,y-1,z)==0&&vis_map(x-facingx,y-1,z-facingz)==0)
			y--;
	}
	inline void spawn() 
	{
		 x=randInt(2,WX-2);
		 z=randInt(1,WZ-2);
		 y=WY-1;//going to go down until hit block
		 facingx=1;
		 fall_down();
		 draw();
		
	}
	inline void clear()/**deletes sheep**/
	{
		vis_map(x,y,z)=0;
		vis_map(x-facingx,y,z-facingz)=0;
	}
	inline void draw()
	{
		if(facingx!=0)//is facing in x axis
		{
			vis_map(x,y,z)=SHEEPFACEX;
			vis_map(x-facingx,y,z)=SHEEPBACKX;
		}
		else//is facing in z axis
		{
			vis_map(x,y,z)=SHEEPFACEZ;
			vis_map(x,y,z-facingz)=SHEEPBACKZ;
		}
	}

	inline void move()
	{
		clear();
		x+=facingx;
		if(vis_map(x,y,z)!=0)
		{
			y++;//try to jump
			if(vis_map(x,y,z)!=0||vis_map(x-facingx,y,z)!=0)
			{
				y--;//nope, can't jump
				x-=facingx;
			}
		}
		z+=facingz;
		if(vis_map(x,y,z)!=0)
		{
			y++;//try to jump
			if(vis_map(x,y,z)!=0||vis_map(x,y,z-facingz)!=0)
			{
				y--;//nope, can't jump
				z-=facingz;
			}
		}
		fall();
		draw();
		
	}
	
	inline void randturn()
	{
		clear();
		int foo= randInt(0,10);
		switch(foo)
		{
			case 0:
				if(vis_map(x-1,y,z)==0){
					facingx=1;
					facingz=0;
				}
				break;
				
			case 1:
				if(vis_map(x+1,y,z)==0){
					facingx=-1;
					facingz=0;
				}
				break;
			case 2:
				if(vis_map(x,y,z-1)==0){
					facingx=0;
					facingz=1;
				}
				break;
			case 3:
				if(vis_map(x,y,z+1)==0){
					facingx=0;
					facingz=-1;
				}
				break;
				
		}
		draw();
			
	}
	 
};
uint8_t ghotbar[8]={0};
void add2hotbar(int id)
{
	for (int i=0; i<8;++i)
	{
		if(ghotbar[i]==id)
		{
			return;//it is already in hotbar; do nothing			
		}
	}
	//ok, so now time to try to find place for it
	for (int i=0; i<8;++i)
	{
		if(ghotbar[i]==0)
		{
			ghotbar[i]=id;//empty space
			return;			
		}
	}
}
static Sheep sheep[SHEEPNUM];
int32_t* gstarttime;
uint32_t offsettime=(uint32_t)(3932160/2)*(uint32_t)2;//2 minutes in ticks.
int hotbarblock=0;
// for reverence v v v 
//const static gfx_sprite_t *item_imgs[]={woodpic,stonepic,goldpic,ironpic,diamondpic,/*netheritepic*/NULL,stick,woodsword,stonesword,goldsword,ironsword,diamondsword,ironingot,goldingot,rawmutton,    cookedmutton,    rottenflesh};
const static uint8_t attack_damage[]  =   {2,        3,       2,       4,        5,           6,              1,       4,      5,         4,      6,           7,           1,        1,       1,          1,1};
void hurtsheep(uint8_t* block)
{
	int i=0;//need later (is index of what sheep it is)
	for(; i<SHEEPNUM;++i)
	{
		auto s=sheep[i];
		
		if(&(vis_map(s.x,s.y,s.z))==block||&(vis_map(s.x-s.facingx,s.y,s.z-s.facingz))==block)//if is right sheep
			break;
	}
	dbg_printf("Sheep number #%d has been hit!\n",i);
	{
	//i is index of sheep
		int amount=1;
		int hblock=ghotbar[hotbarblock];
		if(hblock>invenblocknum-1)//is an item
			amount=attack_damage[hblock-invenblocknum-1];
		sheep[i].hp-=amount;
		if(sheep[i].hp<=0)
		{
			sheep[i].clear();
			sheep[i].spawn();
			sheep[i].hp=8;
			inventory[WOOL]+=2;
			inventory[RAWMUTTON]+=1;
			add2hotbar(WOOL);
			add2hotbar(RAWMUTTON);
		}
	}
}
int respawnx=1;
int respawny=WY-1;
int respawnz=1;

Zombie zombies[ZOMBIENUM];
void use_bed(uint8_t* block)
{
	uint32_t temps=clock()-*gstarttime;//global start time
	temps+=offsettime;
	uint32_t elapsed_seconds=(unsigned int)(temps>>15);
	elapsed_seconds=(elapsed_seconds * 17477) >> 19;//now in minutes
	int wtime=((int)elapsed_seconds)%20;//world time. world resets every 20 minutes
	dbg_printf("Tried to use bed: witme %d\n",wtime);
	if(wtime>9)//night
	{
		//time to sleep
		gfx_SetDrawBuffer();
		gfx_FillScreen(0);
		gfx_PrintStringXY("Sleeping...",116,110);
		gfx_SwapDraw();
		msleep(3000);
		//we will skip backwards instead of forwards to delay overflow in the time variable
		uint32_t timetoskip=22-wtime;//in world time
		timetoskip=timetoskip*30;//now in minutes. should be *60 but made cycle 2x speed
		timetoskip=timetoskip<<15;
		offsettime+=timetoskip;
		respawnx=plx;
		respawny=ply;
		respawnz=plz;
	}
}

void hurtzombie(uint8_t* block)
{
	int amount=1;
	int hblock=ghotbar[hotbarblock];
	if(hblock>invenblocknum-1)//is an item
		amount=attack_damage[hblock-invenblocknum-1];
	int i;
	for (i=0; i<ZOMBIENUM;++i)
	{
		auto z=zombies[i];
		
		if(&(vis_map(z.x,z.y,z.z))==block||block==&(vis_map(z.x,z.y+1,z.z)))
			break;
	}
	int prevhp=zombies[i].hp;
	zombies[i].hp-=amount;
	if(zombies[i].hp<1&&prevhp>0)
	{
		inventory[ROTTENFLESH]+=2;
		add2hotbar(ROTTENFLESH);
	}
	
}


static void (*block_is_usable[])(uint8_t*) ={use_craftingtable,  furnace_stuff, NULL, use_bed,NULL,   use_doorto,use_doorbo,use_doortc,use_doorbc,     NULL,NULL,NULL,hurtsheep,hurtsheep,hurtsheep,hurtsheep,hurtzombie,hurtzombie,hurtzombie};


void draw_trans_rect_fake(int x,int y, int w, int h,int color)
{
	/** draws a transparent rectangle using dithering (or however you spell that). used for
	the greyed/grayed out portion of the crafting table and nothing else (so far!)**/
	uint8_t* buff=(uint8_t*)gfx_vbuffer;
	for (int i=x;i<w+x;++i)
	{
		int temp=i&1;
		for (int j=y+temp;j<y+h;j+=temp+1)
		{
			buff[j*320+i]=color;
		}
	}

}
void draw_trans_rect_fake_less_dark(int x,int y, int w, int h,int color)
{
	/** draws a transparent rectangle using dithering (or however you spell that). used for
	the greyed/grayed out portion of the crafting table and nothing else (so far!)**/
	uint8_t* buff=(uint8_t*)gfx_vbuffer;
	for (int i=x;i<w+x;i+=2)
	{
		for(int j=y;j<y+w;j+=2)
		{
			buff[j*320+i]=color;
		}
	}

}
#define RUN_DDA_LOOP(stepx,stepy,stepz)\
	_Pragma("clang loop unroll(full)") \
	for (uint8_t i=0; i<VD;++i)\
	{\
		\
		if (tmaxx<tmaxy){\
			if (tmaxx<tmaxz){/*tmaxx is smallest*/\
				curr_block+=stepx;\
				tmaxx.data+=deltax.data;\
				d=0;\
			}\
			else\
			{/*tmaxx ___ tamxy, so tmaxz*/\
				curr_block+=stepz;\
				tmaxz.data+=deltaz.data;\
				d=2;\
			}\
		}\
		else{\
			if (tmaxy<tmaxz){/*tmaxy is smallest*/\
				curr_block+=stepy;\
				tmaxy.data+=deltay.data;\
				d=1;\
			}\
			else{\
				curr_block+=stepz;\
				tmaxz.data+=deltaz.data;\
				d=2;\
			}\
		}\
		\
		/*if((uint24_t)(curr_block - w) < WX*WY*WZ)//saying if <0, wrap to big val and if to big is not <*/\
			if (*curr_block){/* if it is not 0*/\
				goto blocky_stuff;\
			}\
	}
//#define mlt32(a, b) ((int) ( ( (int32_t)(a.data) * (int32_t)(b.data) ) >> shift_by) )
#define mlt32(a,b) (a*b)
extern "C" void rect_8x8(int x, uint8_t y, uint8_t color, uint8_t* screenaddr);
inline void raycast_screen()
{
	/** this draws everything. This is kinda complicated, because it needs to be fast. It uses raycasting, but instead of being 2.5d it is 
	fully 3d (you can look up and down in addition to looking left and right). I cache a lot of stuff to make it faster. Basically, what happens, 
	is i get the ray direction from the x,y of the pixel, rotate it by	the players rotation (rotxz and rotyz) then use the dda algorithm to tell 
	when hit a block. DDA algorithm in a nutshell is instead of taking a bunch of really small steps we take a little bit of smart steps. 
	(we basiclally jump across on the grid lines of the block to tell when we hit one instead of moving a little bit at a time and seeing if we hit one 
	(what step raycasters do)) Look it up if you want more detail. there are 2 types of blocks. Solid blocks (like	grass, stone, or leaves) and textured blocks 
	(like glass or a crafting table). Textured blocks are a lot slower. Expecially if they are clear.like a door frame. That will tank your frame rate ;). 
	I try to avoid too many of those blocks. Anyways, Textured blocks are just blocks whose sides have 8x8 pictures on them instead of a solid color.
	Hard to represent a crafting table with one color. If it is a colored block, i can just get the color of the side. If it is a textured block, however, i have to use 
	some semi-broken-but-still-usable logic i saw on a wikipedia page (let me be clear: broken in my code, not theirs) to find the color of the exact block. If the color is 26, 
	which i treat as clear, it just skips the block goes back and keeps looking for another block. That is why doors look funky from the inside -- you can't see a block from the inside.
	
	Anyways, this part works. I promise the rotation logic is there, despite it being spread across the universe (again speed) I will admit, although AI did not write
	most of this, i did ask it to look through and see if it could make it faster. And it did help, despite many halluciations. For some reason, it
	could not seem to understand my rotation logic ;). It also kept suggesting things that made it slower so I had to double check everything. Like I said,
	AI DID NOT WRITE ANY FUNDAMENTAL LOGIC, I DID THAT!!! ME!!! although it did help with that branching in the dda loop and proably increased the overall speed by 20%
	Another thing: I didn't copy/paste really any ai code (mostly because it wasn't going to work, and with an exception for like one or two uncomplicated lines) so really i wrote almost all of
	this code. If you count AI suggestions (like using less variables), those probably made it 1.5x as fast, even though it was me who wrote the code. And not a stare at it's code and "write" it -- I actually wrote the logic.**/
	int tempx=(int)plx-(chunkx<<4);
	int tempy=(int)ply;
	int tempz=(int)plz-(chunkx<<4);
	uint8_t* w= world+(tempx << XWS) + (tempy << YWS) + tempz;
	
	//flint *cache =  (flint*)((uint8_t*)gfx_vbuffer+ (200 * 320));//screen where hotbar is (drawbuffer)
	
	flint cosxz=cached_cos_rot[rotxz];
	flint sinxz=cached_sin_rot[rotxz];
	
	flint cosyz=cached_cos_rot[rotyz];
	flint sinyz=cached_sin_rot[rotyz];
	
	flint* cacheaddr=&cache[0];
	flint * cacheaddr320=cacheaddr+320;
	//dbg_printf("\n\n\n");
	for(int i=0; i<320;i+=res)
	{
		flint nx=cached_x_dir[i];
		(*cacheaddr).data=(cosxz.data*nx.data)>>shift_by;
		(*(cacheaddr320)).data=(sinxz.data*nx.data)>>shift_by;
		//dbg_printf("cos: %d, sin: %d, nx: %d\n",cosxz.data,sinxz.data,nx.data);
		cacheaddr+=res;
		cacheaddr320+=res;
	}
	//dbg_printf("\n\n\n");
	

	flint* inv_tablel=&inv_table[0];
	//int y=200;
	const uint8_t* lcolors=colors;
	auto ltexturedata=texturedata;//the typing was a const uint8_t*[]. idk how to do that.
	for (unsigned int y=0; y<184;y+=res){
		flint tdy = cached_y_dir[y];	
		flint ny;ny=tdy;
		tdy=(cosyz*ny-sinyz);//dz is 1 so don't mult yet
		flint tdz=(sinyz*ny+cosyz);//yz
		flint deltay= inv_tablel[fast_abs(tdy.data)];
		
		flint sinxztdz;sinxztdz.data=(sinxz.data*tdz.data)>>shift_by;
		flint cosxztdz;cosxztdz.data=(cosxz.data*tdz.data)>>shift_by;
		flint *cx = &cache[0];
		flint *cz = &cache[320];
		//#pragma clang loop unroll_count(5)
		for(unsigned int x=0; x<320;x+=res){

			flint dx;dx.data=((*cx).data-sinxztdz.data);
			flint dz;dz.data=((*cz).data+cosxztdz.data);//xz
			flint deltax= inv_tablel[fast_abs(dx.data)];	
			// had idx,idy,idz before
			flint deltaz = inv_tablel[fast_abs(dz.data)];
			
			flint tmaxx=deltax;// dist to next line *deltax. start on int coord, so dist to next line is 1
			flint tmaxy=deltay;
			flint tmaxz=deltaz;
			
			uint8_t quadrant = (dx.data>0) | ((tdy.data>0)<<1) | ((dz.data > 0)<<2);
			
			uint8_t* curr_block = w;
			uint8_t d;
			cx+=res;
			cz+=res;
			start_loop:
			switch(quadrant) 
			{
				case 0: RUN_DDA_LOOP(-XJ, -YJ, -1); break; // All negative
				case 1: RUN_DDA_LOOP( XJ, -YJ, -1); break; // X positive
				case 2: RUN_DDA_LOOP(-XJ,  YJ, -1); break; // Y positive
				case 3: RUN_DDA_LOOP(XJ,  YJ, -1); break; // X, Y positive
				
				case 4: RUN_DDA_LOOP( -XJ, -YJ,  1); break; // Z positive
				case 5: RUN_DDA_LOOP(  XJ, -YJ,  1); break; // X, Z positive
				case 6: RUN_DDA_LOOP( -XJ,  YJ,  1); break; // Y, Z positive
				case 7: RUN_DDA_LOOP(  XJ,  YJ,  1); break; // All positive
			}

			continue;
			blocky_stuff:
			int id=*curr_block;
			uint8_t tcolor;
			if (id<reg_block_num+1)//normal blocks, faster
			{
				//gfx_SetColor();	
				tcolor=lcolors[(id<<2)+d];
			}
			else //textured blocks. slower. THIS CODE BRINGS DOWN PERFORMANCE WHEN IT ISN"T USED!!!
			{
				//interesction = rayorg +raydir*(smallest tmax)
				//uint8_t tcolor;
				
				
				
				switch ((uint8_t)d) {
					case 0://x
					{
						tmaxx-=deltax;
						//texture z. gets deciaml. turns from between 0 and 256 to between 0 and 8.
						//tmaxx is smallest
						
						flint tzf=mlt32(dz,tmaxx)+((int)plz);
						flint tyf=mlt32(tdy,tmaxx)+((int)ply);
						
						int tz= (fast_abs(tzf.data)&511)>>6;
						int ty=(fast_abs(tyf.data)&511)>>6;
						tcolor=ltexturedata[(id-(reg_block_num+1))<<2][(tz)+((ty)<<3)];
						if (tcolor==26)//transparent
						{
							tmaxx+=deltax;					
							goto start_loop;
						}
						break;
					}

					case 1://y
					{
						tmaxy-=deltay;
						int top=(tdy.data>0);
						top=(top<<1);// turns num<0 or num>0 into 0 or 2.
						top=0;
						flint txf=mlt32(dx,tmaxy)+((int)plx);
						flint tzf=mlt32(dz,tmaxy)+((int)plz);
						
						int tx=(fast_abs(txf.data)&511)>>6;
						int tz=(fast_abs(tzf.data)&511)>>6;
						tcolor=ltexturedata[((id-(reg_block_num+1))<<2)+top+1][(tx)+((tz)<<3)];
						if (tcolor==26)//transparent
						{
							tmaxy+=deltay;					
							goto start_loop;
						}
						break;
					}
					case 2://if d==2. z.
					{
						tmaxz-=deltaz;
						flint txf=mlt32(dx,tmaxz)+((int)plx);
						flint tyf=mlt32(tdy,tmaxz)+((int)ply);
						
						int tx=(fast_abs(txf.data)&511)>>6;
						int ty=(fast_abs(tyf.data)&511)>>6;
						
						tcolor=ltexturedata[((id-(reg_block_num+1))<<2)+2][(tx)+((ty)<<3)];
						if (tcolor==26)//transparent
						{
							tmaxz+=deltaz;					
							goto start_loop;
						}
						break;
					}
				}

											
			}
			//gfx_SetColor(tcolor);
			//gfx_FillRectangle_NoClip(x, y, res, res);
			rect_8x8(x,y,tcolor,(uint8_t*)gfx_vbuffer);
		}
			
	}
}
void draw_tree(int x, int y, int z)
{
	vis_map(x,y,z)=WOOD;
	vis_map(x,y+1,z)=WOOD;//trunk
	for(int xo=-2;xo<=2;xo++){// tree leaves are 5 by 5
		for(int zo=-2;zo<=2;zo++){//drawing bottom leaves
			vis_map(x+xo,y+2,z+zo)=LEAF;
		}
	}
	
	for(int xo=-1;xo<=1;xo++){
		for(int zo=-1;zo<=1;zo++){//drawing top leaves
			vis_map(x+xo,y+3,z+zo)=LEAF;
		}
	}
	vis_map(x,y+2,z)=WOOD;//more trunk
}
#define hmap(x,z) hmap[((x) * (WZ + 4)) + (z)]
void generate_world(){
	/** generates the world (from a seed you set before calling. first, generates a heightmap (hmap) full of random values.
	Then, 5x5 gaussian blurs it to look more natural. I ain't doing no perlin noise with this calculator! 
	
	Then, it puts the heightmap on to the world array (puts a grass block at the world index [x][hmap[x][z]][z].
	
	Puts grass at each height on the hmap in world. Then, puts 3 grass blocks under each grass block, then fills with stone till bottom.
	
	Then, makes some trees (picks random block that isn't too high and then plops a tree on it)
	
	Then, puts random ore/minerals randomly in random locations based on y level. Like diamonds close to bottom and iron higher up.
	
	Then makes some tunnels in the ground. (or caves, whatever you prefer). Basically, picks random coords then picks random 
	z and y angles then trots off in that direction destroying blocks, with each step adding a new random value to the angle (within bounds for y
	to avoid loop de loops) so the tunnels look more wobbley (through the macros TUNNELWOBBLEF and TUNNELWOBBLEU, which control the wobbleyness).
	
	Then, surrounds the world in barrier blocks (the same color as bg color so can't really tell).
	Overflow/underflow means i don't actually have to fill in everything (because my array is flattened)**/
	//hmap is grass terrain height. grass from y level 8 to 14.
	int kernel[5][5] = {//gausian stuff
    {1,  4,  7,  4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1,  4,  7,  4, 1}
	};
	int kernel_sum = 273;
	//uint8_t* hmap= (uint8_t*)malloc((WX+4)*(WZ+4));
	uint16_t *hmap = (uint16_t*)0xD40000;//this is screen's ram. some noise will appear, but not too much

	for (int x=0;x<WX+4;x++){
		for (int z=0;z<WZ+4;z++){// filling array with random values.
			hmap(x,z)=randInt(20,26);
		}
	}
	//now time to blur.
	for (int x=0;x<WX;x++){
		for (int z=0;z<WZ;z++){
			int total=0;
			for (int xo=0;xo<5;xo++){
				for (int zo=0;zo<5;zo++){// would say -2 but have to +2 anyways so 0
					total+=kernel[xo][zo]*hmap(x+xo,z+zo);
				}
			}
			//newhmap[x][z]=total/kernel_sum;
			vis_map(x,total/kernel_sum,z)=1;
		}
	}
	//now time to fill in under grass
	for (int x=0;x<WX;x++){
		for (int z=0;z<WZ;z++){
			int yflag=0;
			for (int y=WY-1;y>=0;y--)
			{
				if (vis_map(x,y,z)==1&&yflag==0)
					yflag=1;
				else if(yflag>0){
					vis_map(x,y,z)=1;//grass
					yflag++;
				}
				if(yflag>4)
					vis_map(x,y,z)=2;//stone
			}
		}
	}
	//now time to add random parts of sand!
	for (int i=0; i<SANDNUM;++i)
	{
		int xs=randInt(3,WX-4);
		int zs=randInt(3,WZ-4);
		int temp=xs+randInt(1,3);
		for (int x=xs-randInt(1,3);x<temp;++x)
		{
			int temp2=zs+randInt(1,3);
			for (int z=zs-randInt(1,3);z<temp2;++z)
			{
				for(int y=WY;y>=0;y--)
				{
					if(vis_map(x,y,z)==GRASS)
					{
						vis_map(x,y,z)=SAND;
						break;
					}
				}
			}
		}
	}
	//now time to add trees!
	for (int i=0;i<TREENUM;i++){
		int x=randInt(3,WX-3);// don't want tree too close to edge of map...
		int z=randInt(3,WZ-3);
		int y;
		for (y=WY-1;y>=0;y--){
			if (vis_map(x,y,z)==1)
				break;
		}
		y++;//brings from grass to space above grass
		if (y<WY-4){//if enough space to draw tree
			//draw tree!
			draw_tree(x,y,z);
			
		}
	}
	// now time to add some minerals!
	for (int i=0; i<IRONNUM;++i)
	{
		int x=randInt(0,WX-1);
		int y=randInt(4,12);
		int z=randInt(0,WZ-1);
		vis_map(x,y,z)=IRON;
	}
	for (int i=0; i<GOLDNUM;++i)
	{
		int x=randInt(0,WX-1);
		int y=randInt(2,10);
		int z=randInt(0,WZ-1);
		vis_map(x,y,z)=GOLD;
	}
	for (int i=0; i<DIAMONDNUM;++i)
	{
		int x=randInt(0,WX-1);
		int y=randInt(0,5);
		int z=randInt(0,WZ-1);
		vis_map(x,y,z)=DIAMOND;
	}
	//now time for.... tunnels!

	for (int i=0; i<TUNNELNUM;++i)
	{
		next:
		if(i>TUNNELNUM)
			break;
		flint tx=(int)randInt(0,WX-1);
		flint ty=(int)randInt(4,12);
		flint tz=(int)randInt(0,WZ-1);
		int ay=(int)randInt(0,63);//direction forward
		int az=(int)randInt(-5,5);//direction up
		az=(64+az)%64;
		//angle->vector2d is (x*cos(angle),y*sin(angle))
		
		//3d (where have angles y and z) is x=cos(y)*cos(z),   y=sin(z),    z=sin(y)*cos(z)
		// y angle is xz, z angle is xy
		
		flint p5=.5f;
		int ix=tx+p5;
		int iy=ty+p5;
		int iz=tz+p5;
		int length=0;
		//dbg_printf("ntunnel,%d   %d,%d,%d\n",i,ix,iy,iz);
		while(length<40)//vis_map(ix,iy,iz)!=0||
		{
			length++;
			//getting vectors...
			
			flint vx = cached_cos_rot[ay] * cached_cos_rot[az];
			flint vy = cached_sin_rot[az];
			flint vz = cached_sin_rot[ay] * cached_cos_rot[az];
			/*
			flint vx=cached_cos_rot[ay];
			flint vy=0;
			flint vz=cached_sin_rot[az];
			*/
			//adding vectors...
			tx+=vx;
			ty+=vy;
			tz+=vz;
			
			//now rounding coords...
			ix=(int)(tx+p5);
			iy=(int)(ty+p5);
			iz=(int)(tz+p5);
			
			//vis_map(ix,iy,iz)=255;
			for (int px=-1; px<2; ++px)
			{
				for(int py=-1; py<2;++py){
					for(int pz=-1;pz<2;++pz)
					{
						unsigned int ox=ix+px;//+randInt(-1,1);
						unsigned int oy=iy+py;//+randInt(-1,1);
						unsigned int oz=iz+pz;//+randInt(-1,1);
						if(ox>=WX||oy>=WY||oz>=WZ)
						{
							i++;
							goto next;
						}
						vis_map(ox,oy,oz)=255;
					}
				}
			}
			
			//now time to wobble the tunnel!
			ay+=randInt(-TUNNELWOBBLEF,TUNNELWOBBLEF);
			az+=randInt(-TUNNELWOBBLEU,TUNNELWOBBLEU);//how wiggly/wobbley the tunnels are
			if(ay<0)
				ay=64-az;
			if(ay>63)
				ay=63;
			az=az%64;
			if(az<0)
				az=0;
			if(az>5&&az<30)
				az=5;
			if(az>30&&az<59)
				az=59;
			
		}
	}
	for (int b=0; b<WX*WY*WZ; ++b)
	{
		if(world[b]==255)
			world[b]=0;
	}
	//now time to add the border!
	//border is 5
	
	int x=0;
	for (int y=0; y<WY;y++){
		memset(world+((x) << XWS) + ((y) << YWS),5,WZ);
	}
	
	x=WX-1;
	for (int y=0; y<WY;y++){
		memset(world+((x) << XWS) + ((y) << YWS),5,WZ);
	}
	int z=0;
	for (int x=0;x<WX;x++){
		for (int y=0;y<WY;y++)
			vis_map(x,y,z)=5;
	}/*
	z=WZ-1;
	for (int x=0;x<WX;x++){
		for (int y=0;y<WY;y++)
			vis_map(x,y,z)=5;
	}
	*/
	//good ol under/overflow^^^ 
	int y=0;
	for (int x=0; x<WX;x++)
		memset(world+((x) << XWS) + ((y) << YWS),5,WZ);//roof
}
/*
#define hmap(x,z) hmap[((x) * (16 + 4)) + (z)]
#define cx 16
#define cy 16
#define cz 16
#define chunk_map(x,y,z) chunk[((x)<<8)+((y)<<4)+(z)]
//chunk dimensions ^ ^ ^
void generate_chunk(uint8_t* chunk,int thischunkx,int thischunkz){*/
	/** generates the chunk (from a seed you set before calling. first, generates a heightmap (hmap) full of random values.
	Then, 5x5 gaussian blurs it to look more natural. I ain't doing no perlin noise with this calculator! 
	
	Then, it puts the heightmap on to the chunk array (puts a grass block at the chunk index [x][hmap[x][z]][z].
	
	Puts grass at each height on the hmap in chunk. Then, puts 3 grass blocks under each grass block, then fills with stone till bottom.
	
	Then, makes some trees (picks random block that isn't too high and then plops a tree on it)
	
	Then, puts random ore/minerals randomly in random locations based on y level. Like diamonds close to bottom and iron higher up.
	
	Then makes some tunnels in the ground. (or caves, whatever you prefer). Basically, picks random coords then picks random 
	z and y angles then trots off in that direction destroying blocks, with each step adding a new random value to the angle (within bounds for y
	to avoid loop de loops) so the tunnels look more wobbley (through the macros TUNNELWOBBLEF and TUNNELWOBBLEU, which control the wobbleyness).
	
	Then, surrounds the chunk in barrier blocks (the same color as bg color so can't really tell).
	Overflow/underflow means i don't actually have to fill in everything (because my array is flattened)**/
	/*
	//generating chunk seed...
	wseed+=thischunkx+(thischunkz<<16);//world can't be more than 65535x65535 blocks with this setup,
	//but already the limit is in the integer max value for saving (caps at 512x128x512 max blocks for int24)
	srand(wseed);//sets the seed
	//hmap is grass terrain height. grass from y level 8 to 14.
	int kernel[5][5] = {//gausian stuff
    {1,  4,  7,  4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1,  4,  7,  4, 1}
	};
	int kernel_sum = 273;
	//uint8_t* hmap= (uint8_t*)malloc((cx+4)*(cz+4));
	uint16_t *hmap = (uint16_t*)gfx_vbuffer;//this is screen's ram. some noise will appear, but not too much

	for (int x=0;x<cx+4;x++){
		for (int z=0;z<cz+4;z++){// filling array with random values.
			hmap(x,z)=randInt(8,14);
		}
	}
	//now time to blur.
	for (int x=0;x<cx;x++){
		for (int z=0;z<cz;z++){
			int total=0;
			for (int xo=0;xo<5;xo++){
				for (int zo=0;zo<5;zo++){// would say -2 but have to +2 anyways so 0
					total+=kernel[xo][zo]*hmap(x+xo,z+zo);
				}
			}
			//newhmap[x][z]=total/kernel_sum;
			chunk_map(x,total/kernel_sum,z)=1;
		}
	}
	//now time to fill in under grass
	for (int x=0;x<cx;x++){
		for (int z=0;z<cz;z++){
			int yflag=0;
			for (int y=cy-1;y>=0;y--)
			{
				if (chunk_map(x,y,z)==1&&yflag==0)
					yflag=1;
				if(yflag>0){
					chunk_map(x,y,z)=1;//grass
					yflag++;
				}
				if(yflag>3)
					chunk_map(x,y,z)=2;//stone
			}
		}
	}
	//now time to add random parts of sand!
	for (int i=0; i<SANDNUM;++i)
	{
		int xs=randInt(3,cx-4);
		int zs=randInt(3,cz-4);
		int temp=xs+randInt(1,3);
		for (int x=xs-randInt(1,3);x<temp;++x)
		{
			int temp2=zs+randInt(1,3);
			for (int z=zs-randInt(1,3);z<temp2;++z)
			{
				for(int y=15;y>=0;y--)
				{
					if(chunk_map(x,y,z)==GRASS)
					{
						chunk_map(x,y,z)=SAND;
						break;
					}
				}
			}
		}
	}
	//now time to add trees!
	for (int i=0;i<TREENUM;i++){
		int x=randInt(3,cx-3);// don't want tree too close to edge of map...
		int z=randInt(3,cz-3);
		int y;
		for (y=cy-1;y>=0;y--){
			if (chunk_map(x,y,z)==1)
				break;
		}
		y++;//brings from grass to space above grass
		if (y<12){//if enough space to draw tree
			//draw tree!
			chunk_map(x,y,z)=WOOD;
			chunk_map(x,y+1,z)=WOOD;//trunk
			for(int xo=-2;xo<=2;xo++){// tree leaves are 5 by 5
				for(int zo=-2;zo<=2;zo++){//drawing bottom leaves
					chunk_map(x+xo,y+2,z+zo)=LEAF;
				}
			}
			
			for(int xo=-1;xo<=1;xo++){
				for(int zo=-1;zo<=1;zo++){//drawing top leaves
					chunk_map(x+xo,y+3,z+zo)=LEAF;
				}
			}
			chunk_map(x,y+2,z)=WOOD;//more trunk
			
		}
	}
	// now time to add some minerals!
	for (int i=0; i<IRONNUM;++i)
	{
		int x=randInt(0,cx-1);
		int y=randInt(2,6);
		int z=randInt(0,cz-1);
		chunk_map(x,y,z)=IRON;
	}
	for (int i=0; i<GOLDNUM;++i)
	{
		int x=randInt(0,cx-1);
		int y=randInt(1,5);
		int z=randInt(0,cz-1);
		chunk_map(x,y,z)=GOLD;
	}
	for (int i=0; i<DIAMONDNUM;++i)
	{
		int x=randInt(0,cx-1);
		int y=randInt(0,3);
		int z=randInt(0,cz-1);
		chunk_map(x,y,z)=DIAMOND;
	}
	//now time for.... tunnels!

	for (int i=0; i<TUNNELNUM;++i)
	{
		next:
		if(i>TUNNELNUM)
			break;
		flint tx=(int)randInt(0,cx-1);
		flint ty=(int)randInt(2,6);
		flint tz=(int)randInt(0,cz-1);
		int ay=(int)randInt(0,63);//direction forward
		int az=(int)randInt(-5,5);//direction up
		az=(64+az)%64;
		//angle->vector2d is (x*cos(angle),y*sin(angle))
		
		//3d (where have angles y and z) is x=cos(y)*cos(z),   y=sin(z),    z=sin(y)*cos(z)
		// y angle is xz, z angle is xy
		
		flint p5=.5f;
		int ix=tx+p5;
		int iy=ty+p5;
		int iz=tz+p5;
		int length=0;
		//dbg_printf("ntunnel,%d   %d,%d,%d\n",i,ix,iy,iz);
		while(length<40)//chunk_map(ix,iy,iz)!=0||
		{
			length++;
			//getting vectors...
			
			flint vx = cached_cos_rot[ay] * cached_cos_rot[az];
			flint vy = cached_sin_rot[az];
			flint vz = cached_sin_rot[ay] * cached_cos_rot[az];

			//adding vectors...
			tx+=vx;
			ty+=vy;
			tz+=vz;
			
			//now rounding coords...
			ix=(int)(tx+p5);
			iy=(int)(ty+p5);
			iz=(int)(tz+p5);
			
			//chunk_map(ix,iy,iz)=255;
			for (int px=-1; px<2; ++px)
			{
				for(int py=-1; py<2;++py){
					for(int pz=-1;pz<2;++pz)
					{
						unsigned int ox=ix+px;//+randInt(-1,1);
						unsigned int oy=iy+py;//+randInt(-1,1);
						unsigned int oz=iz+pz;//+randInt(-1,1);
						if(ox>=cx||oy>=cy||oz>=cz)
						{
							i++;
							goto next;
						}
						chunk_map(ox,oy,oz)=255;
					}
				}
			}
			
			//now time to wobble the tunnel!
			ay+=randInt(-TUNNELWOBBLEF,TUNNELWOBBLEF);
			az+=randInt(-TUNNELWOBBLEU,TUNNELWOBBLEU);//how wiggly/wobbley the tunnels are
			if(ay<0)
				ay=64-az;
			if(ay>63)
				ay=63;
			az=az%64;
			if(az<0)
				az=0;
			if(az>5&&az<30)
				az=5;
			if(az>30&&az<59)
				az=59;
			
		}
	}
	for (int b=0; b<cx*cy*cz; ++b)
	{
		if(chunk[b]==255)
			chunk[b]=0;
	}
	//now time to add the border!
	//border is 5
	
	
	gfx_FillScreen(BG_COLOR);
}
*/
void outline_vis_map()
{
	int x=0;
	for (int y=0; y<WY;y++){
		memset(&vis_map(x,y,0),5,WZ);
	}
	
	x=WX-1;
	for (int y=0; y<WY;y++){
		memset(&vis_map(x,y,0),5,WZ);
	}
	int z=0;
	for (int x=0;x<WX;x++){
		for (int y=0;y<WY;y++)
			vis_map(x,y,z)=5;
	}
	
	z=WZ-1;
	for (int x=0;x<WX;x++){
		for (int y=0;y<WY;y++)
			vis_map(x,y,z)=5;
	}
	
	int y=0;
	for (int x=0; x<WX;x++)
		memset(&vis_map(x,y,0),5,WZ);//roof
}
void decrease_item(int id)
{
	if(inventory[id]>0)
	{
		inventory[id]--;
		if(inventory[id]==0)
		{
			for (int i=0;i<8;++i)
			{
				if(ghotbar[i]==id)
				{
					ghotbar[i]=0;
					break;
				}
			}
		}
	}
	
}
void use_rawmutton()
{
	playerhunger+=2;
	if(playerhunger>20)
		playerhunger=20;
	decrease_item(RAWMUTTON);
	
}
void use_cookedmutton()
{
	playerhunger+=6;
	if(playerhunger>20)
		playerhunger=20;
	decrease_item(COOKEDMUTTON);
}
void use_rottenflesh()
{
	playerhunger+=3;
	if(playerhunger>20)
		playerhunger=20;
	decrease_item(ROTTENFLESH);
}
const static gfx_sprite_t *item_imgs[]={woodpic,stonepic,goldpic,ironpic,diamondpic,/*netheritepic*/NULL,stick,woodsword,stonesword,goldsword,ironsword,diamondsword,ironingot,goldingot,rawmutton,    cookedmutton,    rottenflesh};
static void (*use_item[])(       ){      NULL,    NULL,  NULL,   NULL,     NULL,            NULL,        NULL, NULL,       NULL,      NULL,   NULL,       NULL,       NULL,       NULL, use_rawmutton,use_cookedmutton,use_rottenflesh};
//                                  crafting table   furnace    planks   bed top glass   doortop   doorbottom
const static uint8_t recog_text_side[]={1,            2,         0,        1,    0,       2        /*na*/};
const static uint8_t unmirror[]={3,2,1,0,7,6,5,4};//yeah... i have a mirroring problem with textures.
// this is based off of an array that goes {0,1,2,3,4,5,6,7,8} and shows what the mirroring does (used in unmirroring for draw_item
//because i mirrored all my textures so they don't look mirrored in the game.

// what side of the texture is most recognizable for the item label
inline void draw_item(int temp,int x,int y,int show_num)
{
	//draws the item (like what you see in the hotbar or inventory or such)
	if(temp<reg_block_num+1)//if block
	{
		gfx_SetColor(block_colors_y[temp]);
		gfx_FillRectangle_NoClip(x,y,30,30);

	}
	else if((unsigned int)(temp-(reg_block_num-1))<invenblocknum-reg_block_num)//if(temp>reg_block_num&&temp<block_num+1)//textureblock
	
	{
		int ptemp=temp;
		temp=temp-(reg_block_num+1);
		int side=recog_text_side[temp];//side
		int id=(temp<<2)+side;
		//dbg_printf("id : %d, temp: %d, side:%d\n",id,temp,side);
		for(int x2=0; x2<8;++x2)
		{
			for(int y2=0;y2<8;++y2)
			{
				int realx=unmirror[x2];
				int realy=unmirror[y2];
				int color=texturedata[id][x2+(y2<<3)];
				if (color!=26){
					gfx_SetColor(color);
					gfx_FillRectangle((realx<<2)+x-1,(realy<<2)+y-1,4,4);
				}
			}
		}
		temp=ptemp;
		
	}
	else//if item
	{
		//dbg_printf("item! %d",temp);
		gfx_Sprite_NoClip(item_imgs[temp-invenblocknum-1], x, y);
	}
	if (show_num&&inventory[temp]>0)
	{
	//gfx_SetColor(255); 
	gfx_SetTextXY(x+20, y+20);
	gfx_PrintInt(inventory[temp],0);
	}
}
//all the recipies you can make in a crafting table
int recipies[RECIPIENUM][3][3]={
	    { //planks
        {WOOD,0,0},
        {0,0,0},
        {0,0,0}
    },
    { //stick
        {PLANKS,0,0},
        {PLANKS,0,0},
        {0,0,0}
    },
	{ //wooden picaxe
        {PLANKS,PLANKS,PLANKS},
        {0,		STICK,0},
        {0,		STICK,0}
    },
	{ //stone pic
        {STONE,STONE,STONE},
        {0,		STICK,0},
        {0,		STICK,0}
    },
	{ //iron pic
        {IRONINGOT,IRONINGOT,IRONINGOT},
        {0,	STICK,0},
        {0,	STICK,0}
    },
	{ //diamond pic
        {DIAMOND,DIAMOND,DIAMOND},
        {0,	STICK,0},
        {0,	STICK,0}
    },
	{ //wood sword
        {0,PLANKS,0},
        {0,	PLANKS,0},
        {0,	STICK,0}
    },
	{ //stone sword
        {0,STONE,0},
        {0,	STONE,0},
        {0,	STICK,0}
    },
	{ //iron sword
        {0,IRONINGOT,0},
        {0,	IRONINGOT,0},
        {0,	STICK,0}
    },
	{ //diamond sword
        {0,DIAMOND,0},
        {0,	DIAMOND,0},
        {0,	STICK,0}
    },
	{ //crafting table
        {PLANKS,PLANKS,0},
        {PLANKS,PLANKS,0},
        {0,	0,0}
    },
	{ //door
        {PLANKS,PLANKS,0},
        {PLANKS,PLANKS,0},
        {PLANKS,PLANKS,0}
    },
	{//furnace
		{STONE,STONE,STONE},
        {STONE,  0  ,STONE},
        {STONE,STONE,STONE}
	},
	{ //bed (for now until i make sheep)
        {WOOL,WOOL,WOOL},
        {PLANKS,PLANKS,PLANKS},
        {0     ,0,     0}
    },
};// index of the recipie to id
int recipie_to_id_array[RECIPIENUM]=   {PLANKS,STICK,WOODPIC,STONEPIC,IRONPIC,DIAMONDPIC,WOODSWORD,STONESWORD,IRONSWORD,DIAMONDSWORD, CRAFTTABLE, DOORTOPOPEN,FURNACE,BEDTOP};
int num_items_from_recipie[RECIPIENUM]={4,       4      ,1,      1,      1,       1,          1,        1,        1,       1,               1,          3,       1,      1};
// this ^ ^ ^ is how much of the item you get when you craft it.
inline int recipie_to_id(int craftingtable[3][3][2], int &num)
{
	/**you give it a recipie and it sees if it can craft it. used in crafting screen**/
	for(int i=0; i<RECIPIENUM;++i)
	{
		int flag=1;
		for(int x=0; x<3;++x)
		{
			for(int y=0;y<3;++y)
			{
				if(craftingtable[x][y][0]!=recipies[i][y][x]){// long story short all were sideways
					flag=0;//if crafting table item is not in the recipie in right spot, not craftable
				}
			}
		}
		if(flag){//if craftable
			
			int id =recipie_to_id_array[i];
			num=num_items_from_recipie[i];
			return id;
		}
	}
	return -1;
}
char* int_to_chr(int x, int min)
{
	/**int ->string (or really char*). probs should have named int_to_string but too late now!**/
	 char* buffer = new char[min];
	for (int i=min-1; i>=0; --i){// so reverse
		buffer[i]=x%10;
		x/=10;
	}
	return buffer;
}
int menu(const char** options,int size)
{
	/** makes a menu. you give it number of options and options it returns what the user chose**/
	int choice=0;
	while(1){
		kb_Scan();
		gfx_SwapDraw();
		gfx_FillScreen(255);
		gfx_SetDrawBuffer();
		for(int i=0;i<size;i++){
			if (choice==i)
				gfx_SetTextBGColor(24);
			else
				gfx_SetTextBGColor(255);
			gfx_SetTextXY(1,i*20);
			gfx_PrintString(options[i]);
		}
		if(kb_IsDown(kb_KeyUp)){
			while(kb_IsDown(kb_KeyUp)){
				kb_Scan();
			}
			choice--;
		}
		if(kb_IsDown(kb_KeyDown)){
			while(kb_IsDown(kb_KeyDown)){
				kb_Scan();
			}
			choice++;
		}
		if(choice<0)
			choice=0;
		if(choice>3)
			choice=3;
		//dbg_printf("choice: %d\n",choice);
		if(kb_IsDown(kb_KeyEnter)||kb_IsDown(kb_KeyClear))
		{
			if(choice==0||kb_IsDown(kb_KeyClear))//exit
			{
				gfx_End();
				//free(world);
				return 0;
			}
			gfx_SetTextBGColor(255);
			return choice;
		}
		
	}
	
}
bool superstop=false;
void update_changes(int id, uint8_t* block)
{
	/**So, what does this do? this is basically used for saving. The world structure is basically a seed and then a list
	of all the changes the player has made. This, updates that list of changes. There are two lists it updates: world_changes
	and prev_world_changes. world_changes is all the changes the player has made (like placing a block) while prev_world_changes
	is what the block was before the player touched it. I need that so if a player breaks a grass block but then immediatly places it again,
	I can remove the change rather than adding a new one. prev_world_changes is not saved to the file and is calculated at the beginning of
	the program by comparing world_changes to the world generated by the seed. prev_world_changes_sizes and world_changes_sizes
	(I know, bad names) are the sizes of the arrays in the array of world_changes and prev_world_changes. I will explain 
	that really quickly. world_changes and prev_world_changes are a static array full of malloced arrays. the syntax is
	world_changes[id][whateverchangenumber]. Each malloced array corresponds to a different block id. That way, you can
	mine 1000 stone blocks and only mine like 1 diamond and not have your memory explode. If the memory gets to fragmented, this
	function will try to fit the change in as seen below and then will tell the user it is exiting and the exit and save the changes
	(either gonna be everything except the last block changed or just evrything although probably everything).**/
	if(block==NULL)return;
	//id is block to put, *block is block there.
	
	//3 options: pus block where it used to be, puts block where not suposed to be, puts block where prev block 
	//not supposed to be was
	
	unsigned short b=block-world;
	
	for(int i=0; i<prev_world_changes_sizes[id];++i)
	{
		if (prev_world_changes[id][i]==b)//block was one player already placed and alignes with seed
		{			
			for(int j=0; j<world_changes_sizes[*block];++j)
			{
				
				if (world_changes[*block][j]==b)//if is block player prev placed there
				{
					world_changes[*block][j]=65535;
					for(int k=j+1;k<world_changes_sizes[*block]; k++)
					{
						world_changes[*block][k-1]=world_changes[*block][k];
					}
					break;
				}
			}
			prev_world_changes[*block][i]=65535;//deletes
			return;//work is done
		}
	}
	//now if not aligned with seed
	
	//clearing if prevoius block placed was there
	for(int i=0; i<world_changes_sizes[*block];++i)
	{
		if(world_changes[*block][i]==b)//if was block prev placed there.
		{
			for(int k=i+1;k<world_changes_sizes[*block]; k++)
			{
				world_changes[*block][k-1]=world_changes[*block][k];
			}
			break;
		}
	}
	//now assigning block to world changes
	bool flag=true;
	assign_changes:
	for(int i=0; i<world_changes_sizes[id];++i)
	{
		if(world_changes[id][i]==65535)//open space!
		{
			world_changes[id][i]=b;
			flag=false;
			break;
		}
	}
	if(flag)//not enough space
	{
		dbg_printf("not enough space! resizing worldchanges\n");
		int size=20;
		unsigned short* temp=NULL;
		while(temp==NULL&&size>0){
			temp = (unsigned short*)realloc(world_changes[id], (world_changes_sizes[id]+size) * sizeof(unsigned short));
			size--;
		}
		if (size<15||temp==NULL)
		{
			gfx_SetTextXY(20,100);
			gfx_SetTextFGColor(224);
			gfx_SetTextBGColor(255);
			gfx_PrintString("YOU ARE LOW ON RAM!!! Quitting now to resolve changes.\nThis happens due to ram fragmentation, so if you restart you should be fine...");
			gfx_SetTextFGColor(0);
			gfx_SwapDraw();
			msleep(3000);
			gfx_SwapDraw();
			superstop=true;
			return;
		}
		if(temp!=NULL)
		{
			world_changes[id]=temp;
			memset(world_changes[id]+world_changes_sizes[id],65535,size*sizeof(unsigned short));
			world_changes_sizes[id]+=size;
			
			goto assign_changes;
		}
	}
	//prev_world_changes. gets seed of block previously there.
	assign_prevworldchanges:
	for(int i=0; i<prev_world_changes_sizes[*block]; ++i)
	{
		if(prev_world_changes[*block][i]==65535)
		{
			prev_world_changes[*block][i]=b;
			return;//assign prev_world_changes
		}
	}
	//only here if prev_world_changes ran out of space.
	dbg_printf("prevworlchanges resizing\n");
	unsigned short* temp=NULL;
	int size=20;
	while(temp==NULL&&size>0){
			temp = (unsigned short*)realloc(prev_world_changes[*block], (prev_world_changes_sizes[*block]+size) * sizeof(unsigned short));
			size--;
		}
		if (size<15||temp==NULL)
		{
			gfx_SetTextXY(20,100);
			gfx_SetTextFGColor(224);
			gfx_SetTextBGColor(255);
			gfx_PrintString("YOU ARE LOW ON RAM!!! Quitting now to resolve changes.\nThis happens due to ram fragmentation, so if you restart you should be fine...");
			gfx_SetTextFGColor(0);
			gfx_SwapDraw();
			msleep(3000);
			gfx_SwapDraw();
			superstop=true;
			return;
		}
		if(temp!=NULL)
		{
			prev_world_changes[*block]=temp;
			memset(prev_world_changes[*block]+prev_world_changes_sizes[*block],65535,size*sizeof(unsigned short));
			prev_world_changes_sizes[*block]+=size;
			goto assign_prevworldchanges;
		}
}
/*
void chunk2world(int chx,int chz,int wherex,int wherez)
{
	wherex<<=4;
	wherez<<=4;
	static uint8_t tempchunk[16*16*16]={0};
	generate_chunk(tempchunk,chx,chz);
	//dbg_printf("xo: %d, zo: %d\n",xo,zo);
	for(int x=0; x<16;++x)
	{
		for(int y=0; y<16;++y)
		{
			for(int z=0; z<16;++z)
			{
				vis_map(x+wherex,y,z+wherez)=tempchunk[(x<<8)+(y<<4)+z];
			}
		}
	}
}
void flipworldx()
{
	for(int x=1; x<WX/2-1;++x)
	{
		for(int y=0; y<WY-1;++y)
		{
			for(int z=1; z<WZ-1;++z)
			{
				uint8_t temp=vis_map(x,y,z);
				vis_map(x,y,z)=vis_map(x+16,y,z);
				vis_map(x+16,y,z)=temp;
			}
		}
	}
}
void flipworldz()
{
	for(int x=1; x<WX-1;++x)
	{
		for(int y=0; y<WY-1;++y)
		{
			for(int z=1; z<WZ/2;++z)
			{
				uint8_t temp=vis_map(x,y,z);
				vis_map(x,y,z)=vis_map(x,y,z+16);
				vis_map(x,y,z+16)=temp;
			}
		}
	}
}
*/
//                                              day                          night
static const uint8_t bg_colors[]={106,170,125,95,127,159,127,95,125,170, 106,73,41,8,0,0,0,8,41,73};
ti_var_t foo67;
#define set_archive(slotname,archived)\
 		foo67 = ti_Open(slotname, "r");\
		ti_SetArchiveStatus(archived, foo67); \
        ti_Close(foo67);
		

int main(void){
	//world=(uint8_t *)malloc(WX*WY*WZ * sizeof(uint8_t));	
	//init world_changes and prev_world_changes
	
	for(int i=0; i<32; ++i)
	{
		int s=0;
		
		if (i==GRASS||i==STONE)
		{
			s=100;
		}
		else if (i==0)
		{
			s=200;
		}
		else
		{
			s=20;
		}
		world_changes[i]=(unsigned short*)malloc(s*2);
		prev_world_changes[i]=(unsigned short*)malloc(s*2);
		prev_world_changes_sizes[i]=s;
		world_changes_sizes[i]=s;
		
		if(world_changes[i]==NULL||prev_world_changes[i]==NULL)
			dbg_printf("uhh... not enough ram?!\n");
		for(int j=0; j<s;j++)
		{
			world_changes[i][j]=65535;//0 is taken, but max short val isn't!
			prev_world_changes[i][j]=65535;
		}
	}
	
	//world= (uint8_t *)malloc(WX*WY*WZ * sizeof(uint8_t));
	beginning:
	dbg_printf("began!\n");
	ti_var_t worldslot;
	colors = (uint8_t *)malloc(block_num*4* sizeof(uint8_t));
	ti_var_t nslot = ti_Open("MNCFTD", "r");// main appvar that has list of worlds and stuff
	if (nslot==0){//not exist
					//version,number of worlds
		uint8_t header[3] = {0, 0};
		set_archive("MNCFTD",false);
		ti_var_t slot = ti_Open("MNCFTD", "w");
		ti_Resize(255, slot);
		ti_Write(header, sizeof(uint8_t), 2, slot);
		ti_Close(slot);
		set_archive("MNCFTD",true);
		//what had before
	}
	ti_Close(nslot);
	ti_var_t slot = ti_Open("MNCFTD", "r");
	gfx_Begin();
	ti_var_t worldslotr;
	int seed=0;
	const char *options[]={
		"Exit",
		"Create New",
		"Open World",
		"Delete World",
	};
	
	static uint8_t header[255];ti_Read(header, sizeof(uint8_t),255,slot);
	ti_Close(slot);
	uint8_t nheader[22]={};//16 bytes for name, 6 for appvar
	
	int choice=menu(options,4);
	char wname[7]={0};
	unsigned short sizes2[32];
	/**Courtesy of Claude: (they wrote that giant huge comment. I know AI is evil but documentation is a pain!)**/
	/* ============================================================
	 * SAVE FILE FORMATS
	 * ============================================================
	 * MNCFTD (world list):
	 *   [0]      1B   version (always 0)
	 *   [1]      1B   N = number of worlds
	 *   [2..]    22B * N   one entry per world:
	 *     [0-15]  16B  world name (not guaranteed null-padded)
	 *     [16-18] 3B   literal "MCO"
	 *     [19-21] 3B   zero-padded world number, e.g. "167"
	 *                  -> bytes [16-21] = save slot name, e.g. "MCO167"
	 *
	 * Per-world save (e.g. "MCO167"):
	 *   [0-1]    2B          seed (unsigned short)
	 *   [2-65]   64B         sizes[32] (unsigned short each) =
	 *                        # of valid (non-65535) entries per block ID
	 *                        in world_changes[], NOT array capacity
	 *   [66..]   sum(sizes[i])*2 B
	 *                        for i=0..31: sizes[i] entries of unsigned short,
	 *                        each a flat index into world[] (b = block - world)
	 *   [next]   sizeof(int) num2read2[0] = block_num+item_num (inventory count)
	 *   [next]   sizeof(int) * (block_num+item_num)
	 *                        inventory[] array
	 *   [next]   sizeof(int) * 3
	 *                        plx.data, ply.data, plz.data (raw fixed-point, NOT plain ints)
	 *   [next]   sizeof(uint32_t)
	 *                        elapsed play time -> offsettime on next load
	 *
	 * NOTE: sizeof(int) is 3 bytes (24-bit) on this toolchain, not 4.
	 * NOTE: header is 66 bytes total (2 seed + 64 sizes) -- world_changes
	 *       data must be seeked/written starting at offset 66, not 34.
	 * ============================================================ */
	 /** CLIFF NOTES (wrote by me):
	 header:
	 
		 version			 [1 byte]
		 number of worlds    [1 byte]
		 (a bunch of worlds) [22 bytes each]
		 
	world:
		seed	 									   [2 bytes]
		sizes of changes to seeded world 			   [64 bytes]
	    size of block_num+item_num when world was made [3 bytes]
		inventory                                      [3*(block_num+item_num) bytes]
		hotbar                                         [8 bytes]
		player's position in fixed point format        [3 bytes]
		game time (for daylight cycle)                 [1 bytes]
	 **/
	if (choice==1){//creating new
		char buffer [17];
		gfx_End();
		while (kb_IsDown(kb_KeyEnter)){
			kb_Scan();
		}
		os_GetStringInput("Enter world name (max 16 chars): ",buffer,sizeof(buffer));
		for (int i=0; i<16;++i)
			nheader[i]=buffer[i];
		nheader[16]='M';
		nheader[17]='C';// appvars are in format MC (o for overworld, n for nether) world num. like MCO167.
		nheader[18]='O';
		//char* temp=int_to_chr(header[1],3);//appvar name
		char temp[4];
		std::snprintf(temp, sizeof(temp), "%03d", header[1]); 
		nheader[19]=temp[0];nheader[20]=temp[1];nheader[21]=temp[2];// adding number to header. now MCO167 or something
		header[1]=header[1]+1;
		dbg_printf("Temp: %s",temp);
		static uint8_t newheader[200]={0};// header 1 is how many worlds, 22 bytes per world, 2 starting bytes
		int c=(header[1]-1)*22+2;//-1 bcs inc up there
		for (int i=0; i<c; i++)
			newheader[i]=header[i];//adds old header stuff
		for (int i=0;i<22;i++)
			newheader[i+c]=nheader[i];
		set_archive("MNCFTD",false);
		ti_var_t slotw = ti_Open("MNCFTD", "w");
		ti_Write(newheader,sizeof(uint8_t),200,slotw);
		ti_Close(slotw);//saves the new header
		set_archive("MNCFTD",true);
		char temp2[7]={0};
		temp2[0]='M';temp2[1]='C';temp2[2]='O';temp2[3]=temp[0];temp2[4]=temp[1];temp2[5]=temp[2];temp2[6]=0;
		worldslot=ti_Open(temp2, "w");
		ti_Resize(64, worldslot);
		char buffer2[17]={0};
		while (kb_IsDown(kb_KeyEnter)){
			kb_Scan();
		}
		os_GetStringInput("Enter seed: ", buffer2,sizeof(buffer2));
		seed=atoi(buffer2);
	
	unsigned short seedshort=seed;
	
	dbg_printf("seed start: %d, %s\n",seed,temp2);
	//world syntax is 1st two bytes is seed
	ti_Write(&seedshort,sizeof(unsigned short),1,worldslot);//writes seed
	wseed=seed;
	}
	else if(choice==2)//loading world
	{
		char **opt = (char**)malloc(header[1]* sizeof(char*));
		for (int i = 0; i < header[1]; i++) 
			opt[i] = new char[17];
		int c=0;
		for(int i=2; i<header[1]*22+2;i+=22)//skipping start bits, reading world
		{
			memcpy(opt[c],&header[i],16);
			c++;
		}
		msleep(500);
		int wchoice=menu((const char**)opt,header[1]);
		dbg_printf("wchocie: %d",wchoice);
		
		
		memcpy(wname,header+wchoice*22+18,6);
		for (int i = 0; i < header[1]; i++) {
			delete[] opt[i];
		 }
		free(opt);
		worldslotr=ti_Open(wname,"r");
		short x;
		ti_Read(&x, sizeof(short),1,worldslotr);
		seed=0;
		seed=x;
		dbg_printf("seed %d, %s\n",seed,wname);
		//order is 2seed,32size,changes
		wseed=seed;
		ti_Read(sizes2, sizeof(short),32,worldslotr);
		int i=0;
		while(i<32)//should used for but already put while
		{
			if(sizes2[i]>world_changes_sizes[i])
			{
				unsigned short * temp=(unsigned short *)realloc(world_changes[i],sizes2[i]);
				if(temp==NULL){
					dbg_printf("not enough ram to load world");
					return 67;
				}
				world_changes[i]=temp;
				memset(world_changes[i]+world_changes_sizes[i],65535,sizes2[i]*sizeof(unsigned short));
			}
			ti_Read(world_changes[i], 2,sizes2[i],worldslotr);
			++i;
		}
		int num2read[1];
		
		ti_Read(num2read,sizeof(int),1,worldslotr);//gets number of blocks when world was made (backwards compatibility)
		ti_Read(inventory,sizeof(int),num2read[0],worldslotr);
		ti_Read(ghotbar,sizeof(uint8_t),8,worldslotr);
		//reading player position
		ti_Read(&plx.data,sizeof(int),1,worldslotr);
		ti_Read(&ply.data,sizeof(int),1,worldslotr);
		ti_Read(&plz.data,sizeof(int),1,worldslotr);
		ti_Read(&offsettime,sizeof(uint32_t),1,worldslotr);
		
		ti_Read(&respawnx,sizeof(int),1,worldslotr);
		ti_Read(&respawny,sizeof(int),1,worldslotr);
		ti_Read(&respawnz,sizeof(int),1,worldslotr);
		
		ti_Close(worldslotr);
		set_archive(wname,false);
		worldslot=ti_Open(wname,"r+");
		ti_Seek(2, SEEK_SET, worldslot);
	}
	else if(choice==3)//delete world
	{
		if (header[1]>0){
			//gfx_Begin();
			char **opt = (char**)malloc(header[1]* sizeof(char*));
			for (int i = 0; i < header[1]; i++) 
				opt[i] = new char[17];
			int c=0;
			for(int i=2; i<header[1]*22+2;i+=22)//skipping start bits, reading world
			{
				memcpy(opt[c],&header[i],16);
				c++;
			}
			msleep(500);
			int wchoice=menu((const char**)opt,header[1]);
			gfx_Begin();
			dbg_printf("wchocie: %d",wchoice);
			
			
			memcpy(wname,header+wchoice*22+18,6);
			for (int i = 0; i < header[1]; i++) {
				delete[] opt[i];
			 }
			free(opt);
			while (kb_IsDown(kb_KeyEnter))
				kb_Scan();
			msleep(300);
			const char* tempstring[] = {
			"NEVERMIND, PLEASE DON'T DELETE MY WORLD!",
			"Proceed with world deletion?"        
			};
			int rusure=menu(tempstring,2);
			dbg_printf("rusure: %d\n",rusure);
			if (rusure==1)
			{
				dbg_printf("DELETING\n");
				int actualindex=wchoice*22+2;
				char appvarname[7];
				for(int i=0;i<7;++i)
				{
					appvarname[i]=header[actualindex+16+i];//see big huge freaking comment above
				}
				appvarname[6]=0;//so is null terminated
				ti_Delete(appvarname);//ok, so deleted appvar. Now need to remove it from header.
				int length=header[1]*22+2;
				length-=actualindex;
				length-=22;
				dbg_printf("header[1]: %d, length: %d, actualindex: %d, appvarname: %s", header[1], length, actualindex, appvarname);
				memcpy(&header[actualindex],&header[actualindex+22], length);
				header[1]-=1;
				set_archive("MNCFTD",false);
				ti_var_t slotw = ti_Open("MNCFTD", "w");
				ti_Write(header,sizeof(uint8_t),200,slotw);
				ti_Close(slotw);
				set_archive("MNCFTD",true);
			}
		}
		gfx_End();
		dbg_printf("going to beginning...");
		goto beginning;
		
	}
	
	// stores block ids and will appear on screen
	uint8_t* hotbar =&ghotbar[0];
	/*
	hotbar[0]=DIAMONDPIC;
	hotbar[1]=STICK;
	inventory[DIAMONDPIC]=1;
	inventory[STICK]=100;
	for (int i=0;i<block_num+item_num;i++)
		inventory[i]=0;
	*/
	{// just to minimize caching for viewing purposes
	/*
	if (world==NULL){
		gfx_End();
		ti_Close(worldslot);
		ti_Close(worldslotr);
	return 0;}
	*/
	
	char buffer[10];
	gfx_End();
	if(seed==0){
	os_GetStringInput("Enter seed: ", buffer,sizeof(buffer));
	gfx_Begin();
	seed=atoi(buffer);
	}
	//uint8_t** worldinfo;
	srand(seed);
	
	// doing caching for speeeeed
	for (int i=0; i<block_num;i++)
	{
		colors[i<<2]=block_colors_x[i];
		colors[(i<<2)+1]=block_colors_y[i];
		colors[(i<<2)+2]=block_colors_z[i];
	}
	flint fl256=256;
	
	
	for (int i = 0; i < 1024; i++) {
		if (i == 0) inv_table[i] = 300000; 
		else inv_table[i] = fl256/i;// i/(i/512) = 512/i
	}

	point01.float2flint(.01f);//exactly what it looks like
	focal_inv.float2flint(1/(float)focal_length);

	for (int i=0;i<64;i++)
	{
		float angle = i * (6.283185f / 64.0f);
		cached_sin_rot[i].float2flint((float)std::sin(angle));
		cached_cos_rot[i].float2flint((float)std::cos(angle));

	}
	flint max=0;
	for (int i=-160;i<160;i++){//slopes. (x1-x2)/(y1-y2)
		//points= (i,focal),(0,0)      (i-0)/(focal_length-0) xz
		cached_x_dir[i+160].float2flint(i/(float)focal_length);
		cached_x_dir[i+160]*=step_divide;

		if (i==0)
			cached_x_dir[i+160].data=0x1;//smallest flint value
		if (cached_x_dir[i+160].flint_abs()>max)
			max=cached_x_dir[i+160];
		if (i!=0)
			cached_x_dir_inv[i+160].float2flint(1/(float)cached_x_dir[i+160]);// for the z
		else
			cached_x_dir_inv[i+160]=3000000;
	}
	
	max=0;
	for (int i=-120;i<120;i++){//slopes. (x1-x2)/(y1-y2)
		//points= (i,focal),(0,0)      (i-0)/(focal_length-0)
		cached_y_dir[i+120].float2flint(-i/(float)focal_length);
		cached_y_dir[i+120]*=step_divide;
		if (i==0)
			cached_y_dir[i+120].data=0x1;//smallest flint value
		if (cached_y_dir[i+120].flint_abs()>max)
			max=cached_y_dir[i+120];
		if (i!=0)
			cached_y_dir_inv[i+120].float2flint(1/(float)cached_y_dir[i+120]);
		else
			cached_y_dir_inv[i+120]=3000000;//big number! basically oo (infinity) right...? 
			//not using max value to prevent overflow in calculations.
	}
	}
	gfx_Begin();
	gfx_SetDefaultPalette(gfx_8bpp);
	bool alive =true;
	flint dirz=1; flint dirx=0;
	int flag=0;
	flint grav=0; //player's GRAVity
	flint grav_amount=.3f;
	gfx_SetDrawBuffer();
	flint fln1pt5=-1;// flint negative 1 point 5
	//block on hotbar selected
	void *free_ram_ptr;
    size_t free_ram_size = os_MemChk(&free_ram_ptr);

    // This will appear in the CEmu Console (Docks > Console)
    dbg_printf("Free RAM: %zu bytes\n", free_ram_size);
	for (int i=0; i<WX*WY*WZ;i++){// makes everything black
		world[i]=0;
	}
	/*
	for (int xo=0; xo<2;xo++)
	{
		for(int zo=0; zo<2;zo++)//this... doesn't work
		{
			chunk2world(xo,zo,xo,zo);
		}
	}
	outline_vis_map();
	*/
	generate_world();
	//dbg_printf("generating world...\n");
	//generate_chunk(world);
	//dbg_printf("generated world!\n");
	//gohere working on textures. some crazy mirror stuff, but that isn't too bad. only shows part of texture sometimes.
	//BUG IN DOOR! SHEEP
	//can't fix ^ ^ ^ fisheye distorition by noneuclidean distance (taxicab). TODO: rasterization baby!!!! but first chunking :(
	//also, need more textured blocks.
	
	for(int i=0;i<32;++i)
	{
		//dbg_printf("world changes sizes of %d is %d\n",i,world_changes_sizes[i]);
		for(int j=0;j<world_changes_sizes[i];++j)
		{

			if(world_changes[i][j]!=65535)
			{
				dbg_printf("i,j: (%d,%d), w: %d\n",i,j,world_changes[i][j]);
				prev_world_changes[i][j]=world[world_changes[i][j]];
				world[world_changes[i][j]]=i;//i=id
			}
		}
	}
	
	gfx_SetTextBGColor(247);
	vis_map(1,3,1)=reg_block_num+6;
	vis_map(1,2,1)=reg_block_num+7;
	dbg_printf("starting loop\n\n");
	int mining_timer=0;
	unsigned int frame=AVERAGEFPS*5-1;//make sure starts off doing frame stuff
	int32_t start_time=clock();
	gstarttime=&start_time;
	
	//flint oneover60=1/60.0f;
	bool prevkeydel=0;
	//vis_map(2,13,2)=reg_block_num+10;
	//vis_map(2,12,2)=reg_block_num+14;
	//vis_map(2,12,3)=reg_block_num+16;
	//mob stuff!
	
	rtc_Enable(0); 
    srand(rtc_Time()); 
	
	for(int i=0; i<SHEEPNUM;++i)
		sheep[i].spawn();
	//end mob stuff
	gfx_SetTextFGColor(0);
	superstop=false;
	int wtime=0;
	alive=true;
	while (1)
	{		
		playerhp=10;
		playerhunger=20;
		if (!alive)
		{
			plx=respawnx;
			ply=respawny;
			plz=respawnz;
		}
		alive=true;
		while (alive){
			
			frame+=1;
			
			if((frame&1)==0)
			{
				for(int i=0; i<ZOMBIENUM;++i)
				{
					if(zombies[i].alive)
					{
						zombies[i].move();
						if(wtime<10)
						{
							if (zombies[i].hp>0)//if day and zombie hp>0
							{
								zombies[i].hp-=3;
							}
						}
					}
					else
					{
						if(wtime>9)
						{
							zombies[i].alive=true;
							zombies[i].spawn();		
							zombies[i].hp=20;
						}
					}
					
					if(zombies[i].hp<1)
					{
						zombies[i].clear();
						zombies[i].alive=false;
						zombies[i].hp=20;
					}
				}
			}
			
			if((frame&7)==0)//every 1-2 ish seconds
			{
				if(playerhunger>17&&playerhp<10)
				{
					playerhp+=1;
					playerhunger-=randInt(1,2);
				}
			}
			if((frame&15)==0)//every 3 ish seconds
			{
				uint32_t temps=clock()-start_time;
				temps+=offsettime;
				uint32_t elapsed_seconds=(unsigned int)(temps>>15);
				//dbg_printf("elapsed_seconds: %d, %lu,%ld\n",(int)elapsed_seconds,clock(),temps);
				elapsed_seconds=(elapsed_seconds * 17477) >> 19;//same thing as dividing by ~60. done to get minutes. 
				//speed up daylight cycle due to short attenetion spans. should be >>20, but i did >>19 to make 2x as fast
				// 273/(2**14) is around .016662497, close enough to .016666666666 (1/60)
				//will hicup at 6.61 hours of gameplay and overflow after a little over 68 hours.
				//by hicup i mean skip a cycle. it will think 66 is 67 i believe. But who plays on their calc for >hour???
				wtime=((int)elapsed_seconds)%20;//what time of day in world. 0-9 is day, 10-19 is night
				dbg_printf("s %lu g %lu w %d\n",start_time,*gstarttime,wtime);
				//dbg_printf("wtime: %d\n",wtime);
				BG_COLOR=bg_colors[wtime];//bg_color was macro, but i decided i wanted daylight cycle so is now variable
				colors[(5<<2)+0]=BG_COLOR;
				colors[(5<<2)+1]=BG_COLOR;//changing color of barrier block
				colors[(5<<2)+2]=BG_COLOR;
			}
			//mob stuff!
			
			{
				for(int i=0; i<SHEEPNUM;++i)
				{
					if(randInt(0,AVERAGEFPS)==0){
						sheep[i].move();
						sheep[i].randturn();
					}
				}
			}
			
			gfx_SetDrawBuffer();
			gfx_FillScreen(BG_COLOR); 
			/*
			for(int i=pl.x; i<pl.x+6&&i<16&&i>-1;i++)
			{
				for (int j=pl.z;j<pl.z+6&&j>-1&&j<16;j++)
				{
					for (int k=0; k<6;k++)
						vis_map[i-pl.x][k][j-pl.z]=map[i][k][j];
				}
					
			}*/
			kb_Scan(); // Update the key status
			flag=1;
			if(superstop)
			{
				goto end;
			}
			if (kb_IsDown(kb_KeyRight)) {
				rotxz-=2;
				if (rotxz<0)
					rotxz=63;
				
				
			}
			else if (kb_IsDown(kb_KeyLeft)) {
					  
				rotxz+=2;
				if(rotxz>63)
					rotxz-=64;
			}
			else if (kb_IsDown(kb_KeyDown)){
				if(rotyz<15||rotyz>48) {
					  
				rotyz+=1;
				if(rotyz>63)
					rotyz-=64;
				}
			}
			else if (kb_IsDown(kb_KeyUp)){
				if (rotyz<16||rotyz>49) {
				
				if (rotyz==0)
					rotyz=64;				
				rotyz-=1;
				}
			}
			else
				flag=0;
			if (flag)
			{
				dirx=0;dirz=1;
				rot_point_2d(dirx,dirz,0,0,rotxz);
				
			}
			//dbg_printf("X: %d, Y: %d, Z: %d\n",(int)plx,(int)ply,(int)plz);
			if (kb_IsDown(kb_Key2nd)) {
					if(randInt(0,100)==0)
					{
						playerhunger--;
					}
				int nx=(int)(dirx+plx);
				int nz=(int)(dirz+plz);
				
				int iplx=(int)plx;
				int iply=(int)ply;
				int iplz=(int)plz;//                                               door
				if ( (vis_map(nx,iply,iplz)==0&&vis_map(nx,iply-1,iplz)==0) || vis_map(nx,iply,iplz)==DOORTOPOPEN)
					plx+=dirx;
				if ( (vis_map(iplx,iply,nz)==0&&vis_map(iplx,iply-1,nz)==0) ||vis_map(iplx,iply,nz)==DOORTOPCLOSED)
					plz+=dirz;
				/*
				if(plz>15)
				{
					flipworldz();
					plz=1;
					chunkz++;
					chunk2world(chunkx,chunkz,0,1);//no worky?
					outline_vis_map();
				}*/
			}
			else if (kb_IsDown(kb_KeyAlpha)) {
					if(randInt(0,100)==0)
					{
						playerhunger--;
					}
				int nx=(int)(-dirx+plx);
				int nz=(int)(-dirz+plz);
				if ((vis_map(nx,(int)ply,(int)plz)==0&&vis_map(nx,(int)ply-1,(int)plz)==0) || vis_map(nx,(int)ply,(int)plz)==DOORTOPOPEN)
					plx-=dirx;
				if ((vis_map((int)plx,(int)ply,nz)==0&&vis_map((int)plx,(int)ply-1,nz)==0) ||vis_map((int)plx,(int)ply,nz)==DOORTOPCLOSED)
					plz-=dirz;
			}

			 
			 if ((kb_Data[3] & kb_GraphVar)||is_in_crafting_table) {//xton key. inventory/crafting. haven't implemented it yet tho.
				while (kb_Data[3] & kb_GraphVar)
				{
					kb_Scan();//do nothing, wait until they release
				}
				int crafting_table[3][3][2]={};// all should be 0. syntax is x,y,(num,id)
				
				bool prevup=0;
				bool prevdown=0;
				bool prevright=0;
				bool prevleft=0;
				bool prevadd=0;
				bool prevminus=0;
				bool prevalpha=0;
				bool prev2nd=0;
				
				int cursor=0;
				uint8_t cursorcolor=231;//231 is yellow
				int cursorblock=0;
				int from_where=0;//where item is being moved from
				int cursornum=0;// for crafting table item splitting
				int where=0;//0 is inventory, 1 is hotbar, 2 is crafting table
				while (!(kb_Data[3] & kb_GraphVar||kb_IsDown(kb_KeyClear)))// if press it again, break
				{
					
					gfx_SwapDraw();
					gfx_FillScreen(75);
					gfx_SetDrawBuffer();

					kb_Scan();

					int c=0;
					if (where==1){
						
						gfx_SetColor(cursorcolor);
						gfx_FillRectangle(cursor*40,200,40,40);
						gfx_FillRectangle(cursor*40,200,40,40);
						if(cursor>7)
						{
							cursor=0;
							where++;
						}
						if(cursor<0){
							where--;
							cursor=item_num+reg_block_num+TEXTURENUM-2;
						}
					}
					for (int i=5; i<320;i+=40)// draws hotbar
					{						
						int temp=hotbar[c];
						draw_item(temp,i,205,1);
						/*
						if(temp<block_num)//if block
						{
							gfx_SetColor(block_colors_y[temp]);
							gfx_FillRectangle_NoClip(i,205,30,30);
							gfx_SetColor(0);
							gfx_SetTextXY(i+20, 220);
							gfx_PrintInt(inventory[temp],0);
						}
						else//if item
						{
							gfx_Sprite(item_imgs[temp-block_num], i, 205);
						}*/
						++c;
						
					}
					int x=2;
					int y=3;

					for (int i=1;i<reg_block_num+item_num+TEXTURENUM;++i){//drawing inventory. 0 is air.
						int flag=1;
						for(int j=0;j<8;++j){
							if (i==hotbar[j])// if item in hotbar don't draw it
								flag=0;
						}
						for(int x=0;x<3;x++)
						{
							for(int y=0;y<3;y++)
							{
								if(i==crafting_table[x][y][0])
									flag=0;
							}
						}
						gfx_SetColor(0);
						if(where==0)
						{
							if(i==cursor+1){

							gfx_SetColor(cursorcolor);
							gfx_FillRectangle(x-3,y-3,36,36);
							gfx_SetColor(0);
							}

						
						if(cursor>=reg_block_num+item_num+TEXTURENUM-1)
							{
								where++;
								cursor=0;
							}
						if(cursor<0)
						{
							where=2;
							cursor=8;
							if(!is_in_crafting_table)
								cursor=4;
						}
						}
						if (inventory[i]==0)
							flag=0;// don't draw if nothing there
						
						gfx_FillRectangle(x,y,30,30);
						if (flag){
							draw_item(i,x,y,1);
						}
						if(i<DOORBOTTOMOPEN||i>DOORBOTTOMCLOSED)
							x+=33;
						if (x+30>200){
							x=2;
							y+=33;
						}
					}
				// now drawing crafting portion
				gfx_SetColor(175);
				gfx_FillRectangle_NoClip(200,0,120,120);//lime green part
				if (!is_in_crafting_table)
				{
					gfx_SetColor(140);
					gfx_FillRectangle_NoClip(275,0,45,75);
					gfx_FillRectangle_NoClip(200,75,120,45);
				}
				gfx_SetColor(64);
				
				gfx_FillRectangle_NoClip(200,35,120,10);// horizantal ones			
				gfx_FillRectangle_NoClip(200,75,120,10);
				
				gfx_FillRectangle_NoClip(235,0,10,120);//vertical ones.
				gfx_FillRectangle_NoClip(275,0,10,120);
				int size=3;
				if (!is_in_crafting_table)
				{
					size=2;
					draw_trans_rect_fake(275,0,45,75,74);
					draw_trans_rect_fake(200,75,120,45,74);
					if(where==2)//crafting table
					{
						if(cursor%3>1)//to far right
							cursor++;//move it down one
						if(cursor>5){//to far down
							where=0;
							cursor=0;
						}							
					}
				}
				if(where==2){
					if(cursor<0)
					{
						where--;
						cursor=7;
					}
					if(cursor>=9)
					{
						where=0;
						cursor=0;
					}
				}
				for(int x=0; x<size;++x){
					for(int y=0;y<size;++y)
					{
						if (crafting_table[x][y][1]<=0)
							crafting_table[x][y][0]=0;
						int temp=crafting_table[x][y][0];

						if(where==2){
							if(cursor==x+y*3)
							{
								gfx_SetColor(cursorcolor);
								gfx_FillRectangle(205+x*40,5+y*40,30,30);
							}
							
						}
						if(temp)//!=0
						{
							int draw_x=205+x*40;
							int draw_y=5+y*40;
							draw_item(temp,draw_x,draw_y,0);
							gfx_SetColor(255);
							gfx_SetTextXY(draw_x+20,draw_y+20);
							gfx_PrintInt(crafting_table[x][y][1],0);
							if(cursor==x+y*3)
							{
								if(where==2&&crafting_table[x][y])//!=0
								{
									draw_trans_rect_fake_less_dark(205+x*40,5+y*40,30,30,cursorcolor);
								}
							}
						}
						
					}
				}
				//dbg_printf("cursor pos: %d, where: %d\n",cursor,where);
				if (kb_IsDown(kb_KeyEnter))
				{
					int num;
					int id=recipie_to_id(crafting_table,num);
					if (id!=-1)
					{
						for(int x=0; x<3;x++)
						{
							for(int y=0; y<3;y++)
							{
								if (crafting_table[x][y][0]!=0){
									inventory[crafting_table[x][y][0]]--;
									crafting_table[x][y][0]=0;
								}
							}
						}
						inventory[id]+=num;
						crafting_table[1][1][0]=id;
						crafting_table[1][1][1]=num;
					}
				}
				
				if (kb_IsDown(kb_KeyDown))
				{
					if(prevdown){
						if (where==0)
						{
							int pcursor=cursor;
							cursor+=6;
							if(pcursor<DOORBOTTOMCLOSED+1&&cursor>DOORTOPOPEN)
								cursor+=3;
						}
						if(where==2)
							cursor+=3;
						//msleep(200);
					}
					prevdown=0;
				}					
				else
					prevdown=1;
				

				if (kb_IsDown(kb_KeyUp))
				{
					if(prevup){
						if (where==0)
						{
							int pcursor=cursor;
							cursor-=6;
							if(pcursor>DOORBOTTOMCLOSED&&cursor<DOORTOPOPEN+1)
								cursor-=3;
						}
						if(where==2)
							cursor-=3;
						//msleep(200);
					}
					prevup=0;
				}					
				else 
					prevup=1;
				
				if (kb_IsDown(kb_KeyAlpha))//split if on crafting table
				{
					if(where==2&&prevalpha){
						if(cursorcolor==135)//already is in alpha
						{
						goto key2ndforce;
						}
						cursorblock=crafting_table[cursor%3][cursor/3][0];
						cursornum=crafting_table[cursor%3][cursor/3][1]/2;
						crafting_table[cursor%3][cursor/3][1]-=cursornum;//splits in half
						cursorcolor=135;
						from_where=2;
					}
					prevalpha=0;
				}
				else
					prevalpha=1;
				if (kb_IsDown(kb_KeyRight))// increase hotbarblock
				{
					if(prevright){
					cursor++;
					//msleep(200);
					}
					prevright=0;
					if(where==0)
					{
						if(cursor>DOORTOPOPEN&&cursor<DOORBOTTOMCLOSED+1)
							cursor=DOORBOTTOMCLOSED+1;
					}
				}
				else
					prevright=1;
				
				if (kb_IsDown(kb_KeyLeft))
				{

					if(prevleft){
					cursor--;
					//msleep(200);
					}
					prevleft=0;
					if(where==0)
					{
						if(cursor<DOORBOTTOMCLOSED+1&&cursor>DOORTOPOPEN)
							cursor=DOORTOPOPEN;
					}
				}					
				else
					prevleft=1;
				
				if(kb_IsDown(kb_KeyAdd))
				{
					if(prevadd){
					where++;
					where%=3;
					//msleep(200);
					cursor=0;
					}
					prevadd=0;
				}
				else
					prevadd=1;
				
				if(kb_IsDown(kb_KeySub))
				{
					if(prevminus){
						cursor=0;
						where--;
						if(where<0)
							where=2;
						//msleep(200);
					}
					prevminus=0;
				}
				else
					prevminus=1;
				
				if (kb_IsDown(kb_Key2nd))
				{
					if(prev2nd){
					key2ndforce:
					//msleep(300);
					switch(where){
						case 0:
							//dbg_printf("num: %d, cursor: %d, cursorblock: %d\n",inventory[cursor],cursor,cursorblock);
							if (inventory[cursor+1]>0&&cursorblock==0){
								cursorblock=cursor+1;// 0 is air
								cursornum=inventory[cursorblock];//sets item holding to one it is on
							}
							else if (cursorblock)//!=0
							{
								//inventory is not ordered so don't matter where put item
								if(from_where==2)// if on crafting table
								{
									for (int x=0; x<3;++x)
									{
										for(int y=0; y<3;++y)
										{
											if(crafting_table[x][y][0]==cursorblock)// if item is same as cursorblock
												crafting_table[x][y][0]=0;//clears item and *moves* it to inventory
										}
									}
								}
								cursorblock=0;
							}
							//dbg_printf("new cursorblock: %d\n",cursorblock);
							break;
							
						case 1:
							if (cursorblock==0){
								if(hotbar[cursor]){// !=0
									cursorblock=hotbar[cursor];
									hotbar[cursor]=0;
									cursornum=inventory[cursorblock];
								}
							}						
							else//cursorblock!=0
							{
								if(from_where==2)// if on crafting table
								{
									for (int x=0; x<3;++x){
										for(int y=0; y<3;++y){
											if(crafting_table[x][y][0]==cursorblock)// if item is same as cursorblock
												crafting_table[x][y][0]=0;//clears item so no duplicates are in hotbar
										}
									}
								}
								hotbar[cursor]=cursorblock;
								cursorblock=0;
								cursornum=0;
							}
							break;
						case 2://crafting table
							if (cursorblock)//!=0. if stuff in cursor
							{
								if(crafting_table[cursor%3][cursor/3][0]==cursorblock){//if put same item on stuff
									crafting_table[cursor%3][cursor/3][1]+=cursornum;
									cursorblock=0;
									cursornum=0;
								}
								else if (crafting_table[cursor%3][cursor/3][0])//if stuff in crafting table but not same stuff
								{
									int temp=crafting_table[cursor%3][cursor/3][0];
									crafting_table[cursor%3][cursor/3][0]=cursorblock;//swap block type
									cursorblock=temp;
									
									temp=crafting_table[cursor%3][cursor/3][1];//swap number of blocks
									crafting_table[cursor%3][cursor/3][1]=cursornum;
									cursornum=temp;//sorry no fancy xor swap :( this part can be slow though
								}
								else{
									crafting_table[cursor%3][cursor/3][0]=cursorblock;// if nothing there just put stuff in cursor there
									crafting_table[cursor%3][cursor/3][1]=cursornum;
									cursorblock=0;
									cursornum=0;
								}
								
							}
							else{
								cursorblock=crafting_table[cursor%3][cursor/3][0];
								cursornum=crafting_table[cursor%3][cursor/3][1];//inventory[cursorblock];
								crafting_table[cursor%3][cursor/3][0]=0;
							}break;
					}
					if (cursorblock){//!=0 if something in cursorblock
						cursorcolor=127;//cyan. light up cursor
						from_where=where;//where they got block from
					}
					else
					{
						cursorcolor=231;
					}
				}
				prev2nd=0;
				}
				else
					prev2nd=1;
				
			}
			gfx_FillScreen(BG_COLOR);
			is_in_crafting_table=false;
			}
			
			if (kb_IsDown(kb_KeyDel)) {
				
				uint8_t* foo;
				uint8_t* block =cross_hair_pt(foo);
				
				if(block!=NULL)//going to do hit logic for combat
				{
					if(*block>MOBSTART-1)//is a mob
					{
						int indx=*block-(reg_block_num+1);
						if(block_is_usable[indx]!=NULL)// is usable. Should be though.
						{
							dbg_printf("using!");
							block_is_usable[indx](block);
							goto endplace2;
						}
					}
				}
				int bv;
				int temp=1;
				int temp2=hotbar[hotbarblock];//wood,stone,gold,iron,diamond,netherite (optional)
				if (temp2>invenblocknum && temp2<invenblocknum+6){//if picaxe
					if (temp2<invenblocknum+4)//if wood, stone or gold
						temp=temp2-invenblocknum+1;//wood is 1, so +1 to make 2
					else// if iron, diamond, or netherite(optional)
						temp=temp2-invenblocknum+1;//iron is same as gold
				}
				//dbg_printf("player hard: %d, block hard: %d, temp2: %d block: %p\n",temp,hardness[*block],temp2,block);
				if (block!=NULL&&*block!=5){// if hit block and block is not barrier
					mining_timer--;
					bv=*block;//block value
					if(prevkeydel)
						mining_timer=(hardness[bv]*AVERAGEFPS)/temp;//sorry about using division!
					prevkeydel=0;
						if(mining_timer<=0){
							prevkeydel=1;
							int iv=bv;//inventory value
							if(temp>=hardness[bv]){//only add to inventory if have strong enough picaxe
								dbg_printf("temp: %d, hardness[bv]: %d, bv: %d\n",temp,hardness[bv],bv);
								iv=bv;//inventory value = block value
								if(bv>DOORTOPOPEN&&bv<DOORBOTTOMCLOSED+1)//door, not door top open (regular)
								{
									iv=DOORTOPOPEN;
								}
								inventory[iv]++;
								add2hotbar(iv);
							}
							//dbg_printf("update changes...\n");
							update_changes(0,block);
							*block=0;
							//dbg_printf("assign block...\n");
							if(iv!=bv)//is door
							{
								if(bv==DOORTOPCLOSED||bv==DOORTOPOPEN)
								{
									block-=YJ;//moves down one in y								
								}
								else//bottom door part
									block+=YJ;
								update_changes(0,block);
								*block=0;
							}
							
							}
							
						}
			}
				
			else{
				 mining_timer=0;
				 prevkeydel=1;
			}
			endplace2:
			if (kb_IsDown(kb_KeyStat))//place or use or use item
			{
				int id=hotbar[hotbarblock];
				if (id>invenblocknum-1)//item
				{
					if(use_item[id-invenblocknum-1]!=NULL)//if is usable (like cooked mutton)
					{
						use_item[id-invenblocknum-1]();
						keyup(kb_KeyStat);
						goto endplace;
					}
						
				}
				uint8_t* block;
				uint8_t* temp=cross_hair_pt(block);
				
				if(temp==NULL)
					goto endplace;
				
				if(*temp>reg_block_num&&*temp<MOBSTART)//textured block (often are usable, like crafting table or door)
				{
					int indx=*temp-(reg_block_num+1);
					if(block_is_usable[indx]!=NULL)// is usable
					{
						dbg_printf("using!");
						block_is_usable[indx](temp);
						goto endplace;
					}
				}
				if (id<block_num){// if not item

					
					if (id&&*block==0){//making sure placing in empty block, should always be empty but who knows???
						if(id==DOORTOPOPEN)
						{
							uint8_t* bottomblock=block-YJ;//down one in y
							if(*bottomblock)//is not zero (aka can't place door bcs block is there or trying to place door at base)
							{
								dbg_printf("base/no space detected!\n");
								block+=YJ;//up one in y, seeing if was placing at base
								bottomblock+=YJ;
								if(*block)//wasn't placing at base								
									goto endplace;
								dbg_printf("base detected!\n");
							}
							//if here can place door.
							//will only place bottom because down there will place top.
							update_changes(DOORBOTTOMOPEN,bottomblock);
							*bottomblock=DOORBOTTOMOPEN;
						}
						update_changes(id,block);
						*block=id;
						
						inventory[id]--;
						if(inventory[id]<=0)
							hotbar[hotbarblock]=0;
					}
					
				}
				
			}
			else if (kb_IsDown(kb_KeyAdd))// increase hotbarblock
			{
				hotbarblock++;
				hotbarblock&=7;
			}
			else if (kb_IsDown(kb_KeySub))
			{
				hotbarblock--;
				if(hotbarblock<0)
					hotbarblock=7;
			}
			endplace:
			if (kb_IsDown(kb_KeyClear)) {
				goto end;
			}
			
			int temp2=vis_map((int)plx,(int)ply-1,(int)plz);
			if (temp2)// if feet stuck in ground
			{
				if(!(temp2==DOORBOTTOMCLOSED||temp2==DOORBOTTOMOPEN))//demorganed it!
					ply+=1;
			}
			if (vis_map((int)plx,(int)ply-2,(int)plz)==0||grav<0){
				grav+=grav_amount;
				ply-=grav;
				temp2=vis_map((int)plx,(int)ply,(int)plz);
				if (temp2)//if head stuck in wall
				{
					if(!(temp2==DOORTOPCLOSED||temp2==DOORTOPOPEN))//not in door
						ply-=1;
				}
			}
			else{
				if (kb_IsDown(kb_KeyMode))
				{
					grav=fln1pt5;
					if(randInt(0,40)==0)
					{
						playerhunger--;
					}
				}
				else
					grav=0;
			}
			//dbg_printf("frame!,%d,%d",rotxz,rotyz);
			
			//std::clock_t t1 = std::clock();
			
			raycast_screen();
			//std::clock_t t2 = std::clock();
			//dbg_printf("time: %d\n",(t2-t1));
			gfx_SetColor(75);
			//hotbar
			gfx_FillRectangle_NoClip(0,184,320,56);
			gfx_SetColor(231);
			gfx_FillRectangle_NoClip(hotbarblock*40,200,40,40);
			int c=0;
			int temp;
			for (int i=5; i<320;i+=40)// draws hotbar
			{				
				//dbg_printf("hotbar: %d,i: %d",hotbar[c],i);
				temp=hotbar[c];
				if(temp<reg_block_num+1)//if block
				{
					gfx_SetColor(block_colors_y[temp]);
					gfx_FillRectangle_NoClip(i,205,30,30);
				}
				else if ((unsigned int)(temp-(reg_block_num-1))<invenblocknum-reg_block_num)//textureblock 
				{
					int ptemp=temp;
					temp=temp-(reg_block_num+1);
					int side=recog_text_side[temp];//side
					int id=(temp<<2)+side;
					//dbg_printf("id : %d, temp: %d, side:%d\n",id,temp,side);
					for(int x2=0; x2<8;++x2)
					{
						for(int y2=0;y2<8;++y2)
						{
							int realx=unmirror[x2];
							int realy=unmirror[y2];
							int color=texturedata[id][x2+(y2<<3)];
							if (color!=26){
								gfx_SetColor(color);
								gfx_FillRectangle((realx<<2)+i,(realy<<2)+205,4,4);
							}
						}
					}
					temp=ptemp;
					
				}
				else
				{
					gfx_Sprite_NoClip(item_imgs[temp-invenblocknum-1], i, 205);
				}
				if(inventory[hotbar[c]]>0){
				//gfx_SetColor(255);
				gfx_SetTextXY(i+20, 220);
				gfx_PrintInt(inventory[temp],0);
				}
				++c;
			}
			gfx_SetColor(0);
			gfx_FillRectangle_NoClip(157,119,6,2);//crosshair
			gfx_FillRectangle_NoClip(159,117,2,6);
			for(int x=0; x<playerhp*15;x+=15)
			{
				gfx_Sprite_NoClip(heart,x,185);
			}
			int a=(10-(playerhunger>>1))*15;
			//dbg_printf("playerhunger: %d", playerhunger);
			for(int x=169+a; x<305;x+=15)
			{
				gfx_Sprite_NoClip(hunger,x,185);
			}
			gfx_SwapDraw();
			if(playerhp<1)
			{
				alive=false;
			}
		}
	}
	end:
	if (!worldslot) {
    gfx_End();
    return 0;
}
	dbg_printf("freeing world\n");
	//free(world);
	unsigned short sizes[32]={0};
	int sum=0;
	dbg_printf("calculating size\n");
	for(int i=0; i<32; ++i)
	{
		int len=0;
		bool flag=true;
		for(int j=0; j<world_changes_sizes[i]; ++j)
		{
			if(world_changes[i][j]==65535)
			{
				len=j;
				sum+=len;
				flag=false;
				break;
			}
		}
		if(flag)
		{
			len=world_changes_sizes[i];//if there are no empty spaces in the array
			sum+=len;
		}
		sizes[i]=len;
	}
	dbg_printf("writing\n");
	ti_Resize(
	66
	+ sum*2
	+ sizeof(int)
	+ sizeof(int)*(block_num+item_num)
	+ 8
	+ sizeof(int)*3
	+ sizeof(uint32_t)
	+ sizeof(int)*3,
	worldslot
	);
	ti_Seek(0, SEEK_SET, worldslot);
	unsigned short tempseedshort=seed;
	ti_Write(&tempseedshort,2,1,worldslot);
	ti_Write(sizes,2,32,worldslot);	
	ti_Seek(66, SEEK_SET, worldslot);
	for(int i=0; i<32; ++i)
	{
		ti_Write(world_changes[i],2,sizes[i],worldslot);
	}
	/*
	dbg_printf("freeing world_changes\n");
	for(int i=0; i<32; ++i)
	{
		free(world_changes[i]);
		free(prev_world_changes[i]);
	}
	*/
	//writing inventory
	//int inventory[block_num+item_num]; for reference
	int num2read2[1];
	num2read2[0]=block_num+item_num;
	ti_Write(num2read2,sizeof(int),1,worldslot);
	ti_Write(inventory,sizeof(int),block_num+item_num,worldslot);
	
	ti_Write(hotbar, sizeof(uint8_t), 8, worldslot);//saving hotbar...
	
	int tempplarray[]={plx.data,ply.data,plz.data};	
	ti_Write(tempplarray,sizeof(int),3,worldslot);//saving player's position
	
	uint32_t temptime=clock()-start_time;//time elapsed
	temptime+=offsettime;
	uint32_t value=32768;
	value=value*60*20;//to avoid overflow
	temptime=temptime%value;//make repeat every 20 minutes (daylight cycle)
	ti_Write(&temptime,sizeof(uint32_t),1,worldslot);//saving time
	ti_Write(&respawnx,sizeof(int),1,worldslot);
	ti_Write(&respawny,sizeof(int),1,worldslot);
	ti_Write(&respawnz,sizeof(int),1,worldslot);
	dbg_printf("closing\n");
	ti_SetArchiveStatus(true, worldslot);
	ti_Close(worldslot);
	gfx_End();
	usb_Cleanup();
    return 0;
}