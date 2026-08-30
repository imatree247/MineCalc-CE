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
extern "C" {
    #include "gfx/gfx.h"
}
#define AUTO_GENERATE_GLOBALS
#include <ctime>
#include <tice.h>
#include <algorithm>
#include <usbdrvce.h>
#include <cstdlib>



//mincraft /minecalc TInCEraft
/*

M  I  N  E  C  R  A  F  T
  T    84+^  E
 */
//made by imatree
//(imatree247) 
//also possibly imaTIrCEe but maybe not. still deciding on the name. 
//also deciding on name for this. MINcraft? like minimal mine craft? 
//but minecalc is good to.


//uint8_t vis_map[64][16][64];



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

#define res 5
//res must be divisible by 320 can be 1,2,4,5,8,10,16,etc. ALSO, IN THE ASSEMBLY, YOU HAVE TO DO STUFF TOO!
// i recommend 8 for visuals, 10 for speed. maybe 5 for really good visuals but I'd decrease VD.
//here's a good table for approximate values.
/*
VD 15 (medium view distance):

	res 8  = 12 fps
	res 5  = 5 fps
*/
#define AVERAGEFPS 7
//#define BG_COLOR 191
uint8_t BG_COLOR=125;


#define INTERACTD 4
//#define TREENUM 10
#define IRONNUM 600
#define DIAMONDNUM 40
#define GOLDNUM 90
#define TUNNELNUM 5
#define TUNNELWOBBLEF 10
#define TUNNELWOBBLEU 5
//#define SANDNUM 10

#define GRASS 1
#define STONE 2
#define WOOD 3
#define LEAF 4
//barrier is 5
#define OBSIDIAN 6
#define IRON 7
#define GOLD 8
#define DIAMOND 9
#define SAND 10
#define WOOL 11
#define SNOWGRASS 12
#define SNOW 13
#define NETHERRACK 14

#define block_num 42
#define reg_block_num 15
#define item_num 18
#define RECIPIENUM 16

#define invenblocknum (TEXTURENUM+reg_block_num+FLOORNUM)

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


#define FLOORNUM 3
#define FLOORVOXELS reg_block_num                 // last id that is NOT a floor voxel (15)

#define SNOWCARPET (FLOORVOXELS+1)                // 16
#define REDSTONEDUSTUNLIT (FLOORVOXELS+2)          // 17
#define REDSTONEDUSTLIT (FLOORVOXELS+3)            // 18

#define FLOORVOXELSEND REDSTONEDUSTLIT             // last floor id (18)
static uint8_t floorcolorsconst[]={255,96,224};
uint8_t* floorcolors=floorcolorsconst-(FLOORVOXELS+1);

#define CRAFTTABLE (FLOORVOXELSEND+1)              // 19
#define FURNACE (FLOORVOXELSEND+2)
#define PLANKS (FLOORVOXELSEND+3)
#define BEDTOP (FLOORVOXELSEND+4)
#define GLASS (FLOORVOXELSEND+5)
#define DOORTOPOPEN (FLOORVOXELSEND+6)
#define DOORBOTTOMOPEN (FLOORVOXELSEND+7)
#define DOORTOPCLOSED (FLOORVOXELSEND+8)
#define DOORBOTTOMCLOSED (FLOORVOXELSEND+9)
#define MAGMABLOCK (FLOORVOXELSEND+10)
#define FIRE (FLOORVOXELSEND+11)
#define SANDSTONE (FLOORVOXELSEND+12)
#define CACTUS (FLOORVOXELSEND+13)                 // 31

#define TEXTURENUM 13









#define SKELETOPX (reg_block_num+FLOORNUM+TEXTURENUM+1)
#define SKELETOPZ (reg_block_num+FLOORNUM+TEXTURENUM+2)
#define SKELEBOTTOM (reg_block_num+FLOORNUM+TEXTURENUM+3)
#define SHEEPFACEX (reg_block_num+FLOORNUM+TEXTURENUM+4)
#define SHEEPFACEZ (reg_block_num+FLOORNUM+TEXTURENUM+5)
#define SHEEPBACKX (reg_block_num+FLOORNUM+TEXTURENUM+6)
#define SHEEPBACKZ (reg_block_num+FLOORNUM+TEXTURENUM+7)
#define ZOMBIETOPX (reg_block_num+FLOORNUM+TEXTURENUM+8)
#define ZOMBIETOPZ (reg_block_num+FLOORNUM+TEXTURENUM+9)
#define ZOMBIELEGS (reg_block_num+FLOORNUM+TEXTURENUM+10)

#define MOBNUM (10)
#define MOBSTART SKELETOPX

#define SHEEPNUM 5
#define ZOMBIENUM 5
//static uint8_t noninventory_blocks={DOORBOTTOMCLOSED,DOORBOTTOMOPEN,DOORTOPCLOSED,SKELEBOTTOM,SKELETOPX,SKELETOPZ,SHEEPFACEX,SHEEPFACEZ,SHEEPBACKX,SHEEPBACKZ}
//Error with reg block num  +1 to ^ ^ ^
static uint8_t* world;//[WX*WY*WZ];
static uint8_t blockspritebuffer[(reg_block_num+1)<<8];   // 16 ids * 256B (16x16) = 4096B
static uint8_t texturespritebuffer[TEXTURENUM<<6];     // 13 ids * 64B  (8x8)  = 832B
//static uint8_t* world =(uint8_t*)0xD0087C;
static unsigned short* world_changes[MOBSTART];//array of short*. each array of short* is block moved.
//static unsigned short* prev_world_changes[MOBSTART];//prev
int focal_length=200;//??? guess with this one ;)
// smaller = bigger fov but bigger distortion. BEWARE OF THE FISHEYE!!!
//also, if you change this, have to update inv_table as well
void make_block_fall(uint8_t* block);
struct block2update
{
	uint8_t* block;
	void (*bfunction)(uint8_t*);
};
struct id2func
{
	uint8_t block;
	void (*bfunction)(uint8_t*);
	
	id2func(uint8_t first_val, void (*second_val)(uint8_t*)) : block(first_val), bfunction(second_val) {}
};

static block2update bstack[1024];//good high number. 1024 is officially max number of blocks to update at one moment.

static id2func block_updates[1]={id2func(SAND,make_block_fall)};
//if updates size here update size in update_block future me!
int bstacktop=0;
void add_updates_fully(uint8_t* block);
void make_block_fall(uint8_t* block)
{	
	//makes sand/other stuff eventually fall
	if(*(block-YJ)==0)
	{
		uint8_t id =*block;
		*block=0;
		add_updates_fully(block);
		block-=YJ;
		*block=id;
		add_updates_fully(block);
	}
}

void call_update();
void add_update(uint8_t* block)
{
	if(block>=world&&block<world+32768)
	{
		if(bstacktop>1022)
		{
			call_update();   // safe now — no-op if we're already inside one
		}
		if(bstacktop<1024)   // hard cap, since a reentrant call_update() may have declined to trim
		{
			for(int i=0; i<1; ++i)
			{
				if(block_updates[i].block==*block)
				{
					dbg_printf("added update for %d, block_updates block %d\n",*block,block_updates[i].block);
					bstack[bstacktop].block=block;
					bstack[bstacktop].bfunction=block_updates[i].bfunction;
					bstacktop++;
				}
			}
		}
	}
}
void add_updates_fully(uint8_t* block)
{
	add_update(block);
	//now will update blocks around it. (6 others)
	add_update(block-XJ);
	add_update(block+XJ);
	
	add_update(block-YJ);
	add_update(block+YJ);
	
	add_update(block+1);//no zj because zj is 1
	add_update(block-1);
}
static bool updating_blocks = false;

void call_update()
{
	if(updating_blocks) return;   // already running further up the call stack — do nothing
	updating_blocks = true;

	int count=bstacktop;
	while(bstacktop>1&&count>1)
	{
		bstacktop--;
		count--;
		dbg_printf("calling update! %d\n",bstacktop);
		auto temp=bstack[bstacktop];
		temp.bfunction(temp.block);   // may call add_update -> call_update again,
		                               // but that reentrant call now just returns immediately
	}

	updating_blocks = false;
}

/**
Ok, how block updates work is minecraft java style. (idk how bedrock does it).
Every time you mine/break/interact with a block, it updates the blocks around it (taxicab, no funny diagonal buisness.
If the block is in block_updates, it adds the function
**/




int wseed=0;
bool superstop=false;
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
	inline void float2flint(const float other)
	{
		data = (int)(other * (1 << shift_by) + (other >= 0 ? 0.5f : -0.5f));//WARNING THIS IS SLOW!
		//return *this;
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
//static flint cached_x_dir_inv[320];
//static flint cached_y_dir_inv[240];

/**faster sine/cosine for speed purposes in raycast screen. There are 64 x and y rotations for the camera 
(although i decided it was too slow for the x so now it increments by 2 [so really 32] ) so it is perfect length! (still 64 in y, though)**/
static flint cached_sin_rot[64];
static flint cached_cos_rot[64];

/** for saving purposes, see update_changes_fully for more detail**/
//static int prev_world_changes_sizes[MOBSTART];
static int world_changes_sizes[MOBSTART];


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

static flint inv_tabler[1500];
static flint* inv_table;

/*
volatile int foo;// I forget what these do. Maybe temporary variables?
volatile int foo1;
volatile int foo2;
*/
int o;
flint plx=1;
flint ply=WY-4;
flint plz=1;
int rotxz=0;
int rotyz=0;
#define fast_abs(x) std::abs((int)x)
/*
inline int fast_abs(int x)//suuuper speeedddyyyy abs
{
	
	int mask = x >> 23;// fills with sign bit
	return (x ^ mask) - mask;// does 2's compliment if negative.
	// if mask is 0 x^0 is x, if mask is all 1's x^0=!x. and x-all ones is x--1 or x+1.

}
inline int fast_abs(int x)
{
	return (x < 0) ? -x : x;
}

inline __attribute__((always_inline)) int24_t fast_abs(int24_t input_val) {
    // 1. Force the compiler to place 'input_val' into the 24-bit DE register
    register int24_t de_reg __asm__("de") = input_val;
    int24_t result;

    __asm__ volatile (
        "ld hl, 0       \n\t"   // Clear HL
        "add hl, de     \n\t"   // Copy DE into HL
        "add hl, hl     \n\t"   // Shift HL left; pushes sign bit into Carry
		"ex hl, de \n\t"
        "jp nc, 1f      \n\t"   // If positive, skip to label 1 (HL already matches DE)
        "ex hl, de \n\t"
        // If negative, negate DE into HL
        "ld hl, 0       \n\t"   // Clear HL to 0
        "or a, a        \n\t"   // Clear the Carry flag
        "sbc hl, de     \n\t"   // HL = 0 - DE (Negation complete!)
        
        "1:             \n\t"   // Local label
        "ld %0, hl      \n\t"   // Move the final positive value from HL into 'result'
        : "=r" (result)         // %0: Output variable (compiler will assign a register)
        : "r" (de_reg)          // %1: Input variable (forced to be 'de')
        : "cc", "hl"      // Clobber list: Flags, HL, and AF are modified
    );

    return result;
}
*/


//const int end_world=(int)world+WX*WY*WZ;
inline int fast_copysign_pos(int val, int src) {
	/**is a fast copy sign but only for positive numbers... I only use it for positive numbers though so it works.**/
    int mask=src>>23;//basically turns it into all 1s if pos, all 0s if neg
	return (val^mask)-mask;// inverts it if negitive (the xor) and then does --1 (if the flag was -1 it would be inverted). (+1) so 2's compliment
	
}
uint8_t * cross_hair_pt(uint8_t*& blockbefore)// pl is player, p is point rayracing.
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
//                                              NONE,grass,stone,wood,leaf,border,obsidian,iron,gold,diamond,sand,wool,snowgrass,snow,netherack
static const uint8_t hardness[block_num]=
{
	/*id 0-14, reg blocks*/    0,1,2,1,0,255,5,3,4,4,1,1,1,1,2,
	/*id 15, unused*/          0,
	/*id 16-18, floor*/        0,0,0,   // SNOWCARPET, REDSTONEDUSTUNLIT, REDSTONEDUSTLIT
	/*id 19-31, textures*/     1,2,1,1,1,1,1,1,2,2,255,2,1,
	/*id 32-41, mobs*/         255,255,255,255,255,255,255,255,255,255
};
static uint8_t colors[(reg_block_num + 1) << 2] = {
    /* Block ID:                 0    1    2    3    4         5    6    7    8    9   10   11   12   13   14  15 */
    /*            Block Name:  none grss stne wood leaf border obsd iron gold dmnd sand wool sgrs snow nthr guard */

    /* Channel 0 (x)light */     0, 130, 140,  65,   6, BG_COLOR,   0, 162, 229,  30, 205, 214, 130, 215,  64,   0,
    /* Channel 1 (y)drker */     0,   5, 172, 238, 103, BG_COLOR,  41, 195, 231, 159, 239, 247, 255, 255, 128,   0,
    /* Channel 2(z)darksst */    0, 131, 139,  97,   7, BG_COLOR,   8, 194, 230,  31, 238, 254, 131, 247,  96,   0,
};
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
static const uint8_t planksnorm[64]=
{238,238,238,238,238, 64,238,238
,238,238, 64,238,238,238,238,238
, 64, 64, 64, 64, 64, 64, 64, 64
,238,238,238,238,238, 64,238,238
, 64, 64, 64, 64, 64, 64, 64, 64
,238,238, 64,238,238,238,238,238
, 64,238,238,238,238,238,238,238
, 64, 64, 64, 64, 64, 64, 64, 64};
static const uint8_t planksdarker[64]=
{172,172,172,172,172, 64,172,172
,172,172, 64,172,172,172,172,172
, 64, 64, 64, 64, 64, 64, 64, 64
,172,172,172,172,172, 64,172,172
, 64, 64, 64, 64, 64, 64, 64, 64
,172,172, 64,172,172,172,172,172
, 64,172,172,172,172,172,172,172
, 64, 64, 64, 64, 64, 64, 64, 64};
static const uint8_t plankslighter[64]=
{239,239,239,239,239, 64,239,239
,239,239, 64,239,239,239,239,239
, 64, 64, 64, 64, 64, 64, 64, 64
,239,239,239,239,239, 64,239,239
, 64, 64, 64, 64, 64, 64, 64, 64
,239,239, 64,239,239,239,239,239
, 64,239,239,239,239,239,239,239
, 64, 64, 64, 64, 64, 64, 64, 64};
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
static const uint8_t magmablocky[64]=
{128,128,128,227,128,128,227,227
,227,227,227,128,128,227,128,128
,128,227,128,128,227,128,128,128
,227,128,128,227,128,227,128,128
,128,128,227,128,128,227,128,128
,128,227,128,128,128,227,128,227
,227,128,227,227,128,128,227,128
,227,128,128,128,227,227,128,128};
static const uint8_t magmablockx[64]=
{96, 96, 96,195, 96, 96,195,195
,195,195,195, 96, 96,195, 96, 96
, 96,195, 96, 96,195, 96, 96, 96
,195, 96, 96,195,128,195, 96, 96
, 96, 96,195, 96, 96,195, 96, 96
, 96,195, 96, 96, 96,195, 96,195
,195, 96,195,195, 96, 96,195, 96
,195, 96, 96, 96,195,195, 96, 96};
static const uint8_t fire[64]=
{230,227,232,193,193,232,227,230
,227,232,232,193,193,232,227,230
,232,232,193,193,193,193,232,227
,232,193,193,193,193,193,193,232
,255,255,230,232,232,255,255,255
,255,230,227,232,232,230,255,255
,255,230,227,232,232,230,230,255
,230,227,227,232,232,227,230,255};
static const uint8_t firetop[64]=
{255,230,227,230,230,227,232,232
,232,193,193,193,193,193,193,227
,227,193,193,193,193,193,193,230
,227,193,193,193,193,193,193,232
,227,193,193,193,193,193,193,232
,230,193,193,193,193,193,193,227
,227,193,193,193,193,193,193,232
,232,227,232,230,230,232,227,255};

static const uint8_t cactusside[64]=
{ 4,  2,255,  2,  2,  4,  2,255
,  4,  2,  4,  2,  2,255,  2,  4
,255,  2,  4,255,  2,  4,  2,  4
,  4,  2,  4,  2,255,  4,  2,255
,255,  2,  4,255,255,  4,  2,  4
,  4,  2,  4,  2,  2,  4,255,  4
,  4,255,  4,  2,  2,  4,  2,  4
,  4,  2,  4,  2,  2,255,  2,  4};

static const uint8_t cactustop[64]=
{ 4,  4,  4,  4,  4,  4,  4,  4
,  4,  4,255,  2,  2,  2,  4,255
,  4,  2,  4,  4,  4,  4,  2,  4
,255,  2,255,  2,  2,255,  2,  4
,255,  2,  4,  2,  2,255,  2,  4
,  4,  2,  4,255,  4,  4,  2,  4
,  4,  4,255,  2,  2,  2,  4,  4
,255,  4,  4,  4,255,  4,255,  4};

static const uint8_t sandstoneside[64]=
{246,246,246,237,246,246,246,237
,237,237,237,246,237,237,246,246
,247,247,237,237,247,247,237,237
,247,247,247,247,247,247,247,247
,246,237,237,246,237,237,237,246
,246,246,246,237,246,237,246,237
,237,246,246,246,237,246,246,246
,246,246,237,237,246,246,237,246};
static const uint8_t sandstonesidedarker[64]=
{181,181,181,172,181,181,181,172
,172,172,172,181,172,172,181,181
,213,213,172,172,213,213,172,172
,213,213,213,213,213,213,213,213
,181,172,172,181,172,172,172,181
,181,181,181,172,181,172,181,172
,172,181,181,181,172,181,181,181
,181,181,172,172,181,181,172,181};
static const uint8_t sandstonetop[64]=
{247,246,246,247,247,247,247,247
,247,247,247,247,247,247,247,247
,246,247,247,247,246,247,247,247
,247,247,247,246,247,247,247,247
,247,247,247,247,246,247,247,247
,247,247,246,247,247,247,246,247
,247,247,247,247,247,247,247,247
,247,247,247,247,247,246,247,247};


// this v v v  is an array storing what sides of texture blocks go with what textures ^ ^ ^
static const uint8_t* texturedata[]={
	clear,clear,clear,clear,  clear,clear,clear,clear,  clear,clear,clear,clear, /*id 16-18: snowcarpet, redstonedustunlit, redstonedustlit - side/bottom transparent, top handled by floorcolors[]*/
//                  crafting table                                             furnace                                             planks                                                               bed top                         glass                               doortopopen                               doorbottomeopen             doortopclosed                         doorbottomclosed                   magmablock                                        fire                             sandstone                                                  cactus                                    skeletontopx                                skeletontopz                    skeletonbottom                                  sheepfacex                    sheepfacez                      sheepbackx                      sheepbackz                                           zombietopx                                                zombietopz                                            zombielegs                  
	planksdarker,craftingtabletop,plankslighter,planksnorm,   cobblestone,cobblestone,furnacefront,cobblestone,  planksdarker,plankslighter,planksnorm,plankslighter, bedsidetop,bedtop,bedsidefront,bedtop,   glass, glass,glass,glass,  doorarch,brown,doorfronttop,brown,    doorbottomarch,clear,brown,brown, doorfronttop,brown,doorarch,brown, brown,clear,doorbottomarch,brown, magmablockx,magmablocky,magmablockx,magmablocky,       fire,firetop,fire,firetop, sandstonesidedarker,sandstonetop,sandstoneside,sandstonetop,     cactusside,cactustop,cactusside,cactustop, skeletopf,grey,skeletope,clear,    skeletope,grey,skeletopf,clear,  skelebottom,skelebottom,skelebottom,skelebottom,   sheepface,grey,sheepside,grey,   sheepside,grey,sheepface,grey,     sheepback,grey,sheepside,grey,  sheepside,grey,sheepback,grey,     zombieface,zombieheadtop,zombiehead,zombieheadtop,     zombiehead,zombieheadtop,zombieface,zombieheadtop,      zombielegs,zombieheadtop,zombielegs,zombieheadtop};
static flint cache [640]={};
bool is_in_crafting_table=false;
static int inventory[block_num+item_num]={};
//void update_changes(int id, uint8_t* block);//see compiler, this function does exist! (i make it below and didn't feel like ctrl+x ing all of this below it v v v 
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
	
	gfx_FillRectangle_NoClip(x,y+(h2>>1),w2,h2);//1/2 of arrow is going to be rect	

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
	gfx_FillRectangle_NoClip(x,y+(h2>>1),howfull,h2);
}
int updatehotbar=2;
/** yeah, i am not gonna explain these in detail v v v Basically just does stuff when the player uses that block**/
void use_craftingtable(uint8_t* block)
{
	is_in_crafting_table=true;// goes into crafting table mode
}
// these open and close doors v v v 
void use_doorto(uint8_t* block)// if clicked on top open door block
{
	 //update_changes(*block+2,block);
	 *block+=2;//closes top door
	 block-=YJ;//down one in y
	 //update_changes(*block+2,block);
	 *block+=2;
}
void use_doortc(uint8_t* block)//if clicked on top closed door block
{
	 //update_changes(*block-2,block);
	 *block-=2;//opens top door
	 block-=YJ;//down one in y
	 //update_changes(*block-2,block);
	 *block-=2;//opens
}
void use_doorbo(uint8_t* block)// if clicked on bottom open door block
{
	 //update_changes(*block+2,block);
	 *block+=2;//closes bottom door
	 block+=YJ;//up one in y
	 //update_changes(*block+2,block);
	 *block+=2;//closes top
}
void use_doorbc(uint8_t* block)//if clicked on bottom closed door block
{
	 //update_changes(*block-2,block);
	 *block-=2;//opens top door
	 block+=YJ;//up one in y
	 //update_changes(*block-2,block);
	 *block-=2;//opens
}

static const uint8_t fuels[]={WOOD,PLANKS,CRAFTTABLE,WOODPIC,WOODSWORD,STICK};
//fuel times are out of 50, where 50 is one item smelted
static const int fueltimes[]={   75,   75,     75,      50,       50,25};
static const int smeltable[]={SAND,IRON,GOLD,RAWMUTTON};
static const int resulttable[]={GLASS,IRONINGOT,GOLDINGOT,COOKEDMUTTON};
gfx_sprite_t **flames;
void decrease_item(int id);
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
	int scroll=0;
	//40 items per screen
	while(true){
		gfx_SwapDraw();
		gfx_Wait();
		gfx_FillScreen(107);
		gfx_SetDrawBuffer();
		kb_Scan();
		//drawing stuff!
		int x=2;//r. 23. 23/2 = 11.5
		int y=98;
		int c=0;
		//inventory[DIAMONDPIC]=1;
		for(int i=scroll+1; i<invenblocknum+item_num; ++i)//looping through inventory (inventory is an int[block_num+item_num])
		{
			if(c>35)
				break;
			c++;
			int foo=inventory[i];
			if(cursor==i)
			{
				gfx_SetColor(cursorcolor);
				gfx_FillRectangle(x-2,y-2,36,36);
			}
			if (foo>0&&i!=fuelb && i!=cookedb && i!=resultb)
				draw_item(i,x,y,1);//item/block,x,y,showtext
			else
				draw_item(0,x,y,1);
			x+=35;//r 23.
			if(x>320-30)
			{
				x=2;
				y+=35;
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
		if(foo>5)
			foo=5;
		gfx_Sprite_NoClip(flames[foo],68,38);
		if(cursor-scroll<37)
			scroll-=9;
		if(cursor-scroll>36)
			scroll+=9;
		if(scroll<0)
			scroll=0;
		resultb=0;
		if(howfull>0)
			resultb = resulttable[smeltid];
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
					decrease_item(fuelb);
				}
			}
			if(howfull>49)//finished smelting block
			{
				inventory[resulttable[smeltid]]++;
				howfull=0;
				
				decrease_item(cookedb);
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
		{
			superstop=true;
			return;
		}
		if(kb_Data[3] & kb_GraphVar){//xton
			while(kb_Data[3] & kb_GraphVar)
				kb_Scan();// waits till u release it until ending, so doesen't spawn you in crafting menu
			updatehotbar=2;
			return;
		}
		//length is 9
		if(keyup(kb_KeyRight)){
			cursor++;
		}
		if(keyup(kb_KeyLeft))
			cursor--;
		if(keyup(kb_KeyUp))
		{
			cursor-=9;
		}
		if(keyup(kb_KeyDown))
		{
			cursor+=9;
		}
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
	void move()
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
						if(vis_map(x,y+2,z)==0&&y<WY-1)//jump
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
						if(vis_map(x,y+2,z)==0&&y<WY-1)//if can jump
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
				updatehotbar=2;
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
		 facingz=0;
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

	void move()
	{
		
		clear();
		x+=facingx;
		if(vis_map(x,y,z)!=0)
		{
			y++;//try to jump
			if(vis_map(x,y,z)!=0||vis_map(x-facingx,y,z)!=0||y>WY-1)
			{
				y--;//nope, can't jump
				x-=facingx;
			}
		}
		z+=facingz;
		if(vis_map(x,y,z)!=0)
		{
			y++;//try to jump
			if(vis_map(x,y,z)!=0||vis_map(x,y,z-facingz)!=0||y>WY-1)
			{
				y--;//nope, can't jump
				z-=facingz;
			}
		}
		fall();
		draw();
		
	}
	
	void randturn()
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
uint8_t ghotbar[8]={};
void add2hotbar(int id)
{
	updatehotbar=2;
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
const static uint8_t attack_damage[]  =   {2,        3,       2,       4,        5,           6,              1,       4,      5,         4,      6,           7,           1,        1,       1,          1,                  1};
void hurtsheep(uint8_t* block)
{
	int i=0;//need later (is index of what sheep it is)
	for(; i<SHEEPNUM;++i)
	{
		auto s=sheep[i];			
		if(&(vis_map(s.x,s.y,s.z))==block||&(vis_map(s.x-s.facingx,s.y,s.z-s.facingz))==block)//if is right sheep
			break;
	}
	if(i==SHEEPNUM)//not supposed to happen
	{
		*block=0;
		return;
	}
	{
	//i is index of sheep
		int amount=1;
		int hblock=ghotbar[hotbarblock];
		if(hblock>invenblocknum)//is an item
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
uint8_t chunkx=32;
uint8_t chunkz=32;
uint8_t respawnchunkx=32;
uint8_t respawnchunkz=32;

Zombie zombies[ZOMBIENUM];
void use_bed(uint8_t* block)
{
	respawnx=plx;
	respawny=ply;
	respawnz=plz;
	respawnchunkx=chunkx;
	respawnchunkz=chunkz;
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
	}
}

void hurtzombie(uint8_t* block)
{
	int amount=1;
	int hblock=ghotbar[hotbarblock];
	if(hblock>invenblocknum)//is an item
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


// indexed by (id - (reg_block_num+1)), covers ids SNOWCARPET(16)..ZOMBIELEGS(41)
static void (*block_is_usable[])(uint8_t*) ={
	NULL, NULL, NULL,                                                  // 16-18: snowcarpet,redstonedustunlit,redstonedustlit
	use_craftingtable, furnace_stuff, NULL, use_bed, NULL,             // 19-23: crafttable,furnace,planks,bedtop,glass
	use_doorto, use_doorbo, use_doortc, use_doorbc,                    // 24-27: doors
	NULL, NULL, NULL, NULL,                                            // 28-31: magmablock,fire,sandstone,cactus
	NULL, NULL, NULL,                                                  // 32-34: skeletopx,skeletopz,skelebottom (unimplemented)
	hurtsheep, hurtsheep, hurtsheep, hurtsheep,                        // 35-38: sheepfacex,sheepfacez,sheepbackx,sheepbackz
	hurtzombie, hurtzombie, hurtzombie                                 // 39-41: zombietopx,zombietopz,zombielegs
};


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
		for(int j=y;j<y+h;j+=2)
		{
			buff[j*320+i]=color;
		}
	}

}
#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)

#define RUN_DDA_LOOP_P(stepx,stepy,stepz,pastenum)\
	_Pragma("clang loop unroll(full)") \
	for (uint8_t i=0; i<VD;++i)\
	{\
		if (tmaxx<tmaxy){\
			if (tmaxx<tmaxz){\
				curr_block+=stepx;\
				tmaxx.data+=deltax.data;\
				if (*curr_block){\
				CONCAT(d,pastenum)=0;\
				goto CONCAT(blocky_stuff,pastenum);\
				}\
			}\
			else\
			{\
				curr_block+=stepz;\
				tmaxz.data+=deltaz.data;\
				if (*curr_block){\
				CONCAT(d,pastenum)=2*(reg_block_num+1);\
				goto CONCAT(blocky_stuff,pastenum);\
				}\
			}\
		}\
		else{\
			if (tmaxy<tmaxz){\
				curr_block+=stepy;\
				tmaxy.data+=deltay.data;\
				if (*curr_block){\
				CONCAT(d,pastenum)=(reg_block_num+1);\
				goto CONCAT(blocky_stuff,pastenum);\
				}\
			}\
			else{\
				curr_block+=stepz;\
				tmaxz.data+=deltaz.data;\
				if (*curr_block){\
				CONCAT(d,pastenum)=2*(reg_block_num+1);\
				goto CONCAT(blocky_stuff,pastenum);\
				}\
			}\
		}\
	}

#define TRACE_COLUMN(pastenum)\
	flint dx;dx.data=((*cx).data-sinxztdz.data);\
	flint dz;dz.data=((*cz).data+cosxztdz.data);\
	deltax= inv_tablel[dx.data];\
	deltaz = inv_tablel[dz.data];\
	\
	tmaxx=deltax;\
	tmaxy=deltay;\
	tmaxz=deltaz;\
	\
	uint8_t quadrant = (dx.data>0) | tdy_bit | ((dz.data > 0)<<2);\
	\
	uint8_t* curr_block = w;\
	static uint8_t CONCAT(d,pastenum);\
	cx+=res*2;\
	cz+=res*2;\
	CONCAT(start_loop,pastenum):\
	switch(quadrant)\
	{\
		case 0: RUN_DDA_LOOP_P(-XJ, -YJ, -1, pastenum); break;\
		case 1: RUN_DDA_LOOP_P( XJ, -YJ, -1, pastenum); break;\
		case 2: RUN_DDA_LOOP_P(-XJ,  YJ, -1, pastenum); break;\
		case 3: RUN_DDA_LOOP_P(XJ,  YJ, -1, pastenum); break;\
		case 4: RUN_DDA_LOOP_P( -XJ, -YJ,  1, pastenum); break;\
		case 5: RUN_DDA_LOOP_P(  XJ, -YJ,  1, pastenum); break;\
		case 6: RUN_DDA_LOOP_P( -XJ,  YJ,  1, pastenum); break;\
		case 7: RUN_DDA_LOOP_P(  XJ,  YJ,  1, pastenum); break;\
	}\
	*tempval+=res*2;\
	goto CONCAT(dda_done,pastenum);\
	CONCAT(blocky_stuff,pastenum):\
	{\
	uint8_t id=*curr_block;\
	uint8_t tcolor;\
	if (id<reg_block_num+1)\
	{\
		tcolor=lcolors[(id)+CONCAT(d,pastenum)];\
	}\
	else\
	{\
		switch ((uint8_t)CONCAT(d,pastenum)) {\
			case 0:\
			{\
				tmaxx-=deltax;\
				flint tzf=mlt32(dz,tmaxx)+((int)plz);\
				flint tyf=mlt32(tdy,tmaxx)+((int)ply);\
				int tz= (tzf.data&511)>>6;\
				int ty = (tyf.data >> 3) & 0x38;\
				tcolor=ltexturedata[id<<2][(tz)+ty];\
				if (tcolor==26)\
				{\
					tmaxx+=deltax;\
					goto CONCAT(start_loop,pastenum);\
				}\
				break;\
			}\
			case (reg_block_num+1):\
			{\
				tmaxy-=deltay;\
				flint txf=mlt32(dx,tmaxy)+((int)plx);\
				flint tzf=mlt32(dz,tmaxy)+((int)plz);\
				int tx=((txf.data)&511)>>6;\
				int tz = (tzf.data >> 3) & 0x38;\
				tcolor=ltexturedata[(id<<2)+1][(tx)+tz];\
				if (tcolor==26)\
				{\
					tmaxy+=deltay;\
					goto CONCAT(start_loop,pastenum);\
				}\
				break;\
			}\
			case 2*(reg_block_num+1):\
			{\
				tmaxz-=deltaz;\
				flint txf=mlt32(dx,tmaxz)+((int)plx);\
				flint tyf=mlt32(tdy,tmaxz)+((int)ply);\
				int tx=((txf.data)&511)>>6;\
				int ty = (tyf.data >> 3) & 0x38;\
				tcolor=ltexturedata[(id<<2)+2][(tx)+ty];\
				if (tcolor==26)\
				{\
					tmaxz+=deltaz;\
					goto CONCAT(start_loop,pastenum);\
				}\
				break;\
			}\
		}\
	}\
	*colortorect=tcolor;\
	}\
	/*#if res == 8\*/\
		optomized_raycast_rect_5x5();\
	/*#elif res==5\
		optomized_raycast_rect_5x5();\
	#endif\*/\
	CONCAT(dda_done,pastenum): ;/*if((uint24_t)(curr_block - w) < WX*WY*WZ)//saying if <0, wrap to big val and if to big is not <*/\
	
//#define mlt32(a, b) ((int) ( ( (int32_t)(a.data) * (int32_t)(b.data) ) >> shift_by) )
#define mlt32(a,b) (a*b)
//#define mlt32(a,b) ([&]{ flint r; r.data=(int)(((int32_t)(a).data*(int32_t)(b).data)>>shift_by); return r; }())
extern "C" void rect_8x8(int x, uint8_t y, uint8_t color, uint8_t* screenaddr);
extern "C" uint8_t* getscratch3();
extern "C" uint8_t** getrectaddr();
extern "C" void optomized_raycast_rect_8x8();
extern "C" void optomized_raycast_rect_5x5();

extern "C" void notfullfillscreen(uint8_t color,uint8_t* screenaddr);
extern "C" void rect8x8complete(uint8_t* screenaddr);

//__attribute__((section(".ramfunc"), noinline))
void raycast_screen()
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
	uint8_t** tempval=getrectaddr();
	*tempval=&gfx_vbuffer[0][0];
	uint8_t* colortorect=getscratch3();
	int tempx=(int)plx;
	int tempy=(int)ply;
	int tempz=(int)plz;
	uint8_t* w= world+(tempx << XWS) + (tempy << YWS) + tempz;
	
	//flint *cache =  (flint*)((uint8_t*)gfx_vbuffer+ (200 * 320));//screen where hotbar is (drawbuffer)
	
	flint cosxz=cached_cos_rot[rotxz];
	flint sinxz=cached_sin_rot[rotxz];
	
	flint cosyz=cached_cos_rot[rotyz];
	flint sinyz=cached_sin_rot[rotyz];
	
	flint* cacheaddr=&cache[0];
	flint * cacheaddr320=cacheaddr+320;
	//dbg_printf("\n\n\n");
	/* My version v v v
	for(int i=0; i<320;i+=res)
	{
		flint nx=cached_x_dir[i];
		(*cacheaddr).data=(cosxz.data*nx.data)>>shift_by;
		(*(cacheaddr320)).data=(sinxz.data*nx.data)>>shift_by;
		//dbg_printf("cos: %d, sin: %d, nx: %d\n",cosxz.data,sinxz.data,nx.data);
		cacheaddr+=res;
		cacheaddr320+=res;
	}
	*/
	// claude's version v v v  (asked it to optimize my version.
	flint dnx = cached_x_dir[res] - cached_x_dir[0];      // constant step in nx
	flint dcos; dcos.data = (cosxz.data * dnx.data) >> shift_by;  // 1 mult
	flint dsin; dsin.data = (sinxz.data * dnx.data) >> shift_by;  // 1 mult

	flint valcos; valcos.data = (cosxz.data * cached_x_dir[0].data) >> shift_by;
	flint valsin; valsin.data = (sinxz.data * cached_x_dir[0].data) >> shift_by;

	for(int i=0; i<320; i+=res)
	{
		(*cacheaddr).data = valcos.data;
		(*cacheaddr320).data = valsin.data;
		valcos.data += dcos.data;
		valsin.data += dsin.data;
		cacheaddr+=res;
		cacheaddr320+=res;
	}
	//dbg_printf("\n\n\n");
	

	flint* inv_tablel=&inv_table[0];
	//int y=200;
	const uint8_t* lcolors=colors;
	auto ltexturedata = (texturedata - ((reg_block_num+1) << 2));//the typing was a const uint8_t*[]. idk how to do that.
	//-reg_block_num+1 because otherwise i'd have to say -reg_block_num+1 for every texture thing to convert the id for the array
	//<<2 because do id <<2
	
	
	flint tmaxx;// dist to next line *deltax. start on int coord, so dist to next line is 1
	static flint tmaxy;//for some reason, when i declare some variables as static it gets much faster
	static flint tmaxz;
	static flint deltax;
	static flint deltay;//gohere
	flint deltaz;//but not all variables
	//basically, i made one x and one z not static because only two variables not being static holds up
	//the best performance, but i didn't want looking in one direction to be slower so i tried to split it
	//looking up and down are slower, but not as much, because there is often less to see (like a bottom of a hole)
	//also, nobody is going to walk around looking up at the sky. als, the fps drop is by about .3 fps for direction with statics.
	for (unsigned int y=0; y<184;y+=res){
		flint tdy = cached_y_dir[y];	
		flint ny;ny=tdy;
		tdy=(cosyz*ny-sinyz);//dz is 1 so don't mult yet
		flint tdz=(sinyz*ny+cosyz);//yz
		deltay= inv_tablel[tdy.data];
		
		flint sinxztdz;sinxztdz.data=(sinxz.data*tdz.data)>>shift_by;
		flint cosxztdz;cosxztdz.data=(cosxz.data*tdz.data)>>shift_by;
		flint *cx = &cache[0];
		flint *cz = &cache[320];
		//dbg_printf("y: %d, addr: %p\n",y, *tempval);
		//#pragma clang loop unroll_count(5)
		uint8_t tdy_bit=((tdy.data>0)<<1);
		auto temp=*tempval;
		for(unsigned int x=0; x<320;x+=res*2){

			TRACE_COLUMN(0);
		}
		auto temp2=*tempval;
		cx = &cache[0+res];
		cz = &cache[320+res];
		*tempval=temp;
		*tempval+=res;
		for(unsigned int x=res; x<320;x+=res*2){

			uint8_t color=*(*tempval-1);
			if(color==*(*tempval+res+1))
			{
				*colortorect=color;
				optomized_raycast_rect_5x5();
				cx+=res*2;
				cz+=res*2;
				//*tempval+=res*2;
			}
			else
			{
				TRACE_COLUMN(1);
			}
		}
		*tempval=temp2;
		*tempval+=(320*(res-1));
	}
	 //rect8x8complete(&gfx_vbuffer[0][0]);
}
void draw_tree(int x, int y, int z)
{
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
	vis_map(x,y,z)=WOOD;
	vis_map(x,y+1,z)=WOOD;//trunk
	vis_map(x,y+2,z)=WOOD;//more trunk
}
void outline_world_with_barrier()
{
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
		memset(world+((x) << XWS) + ((y) << YWS),5,WZ);//roof/floor
}
#define hmap(x,z) hmap[((x) * (WZ + 4)) + (z)]
void generate_terrain(uint8_t surfblock, uint8_t underblock,uint8_t bottom,uint8_t top,uint8_t range)
{
	//hmap is grass terrain height. grass from y level 8 to 14.
	uint8_t kernel[5][5] = {//gausian stuff
    {1,  4,  7,  4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1,  4,  7,  4, 1}
	};
	/**#define kernel_sum 273**/
	//uint8_t* hmap= (uint8_t*)malloc((WX+4)*(WZ+4));
	uint8_t *hmap = (uint8_t*)gfx_vbuffer;//this is screen's ram back buffer. nothing should appear.
	int tmr=top-range;
	for (uint8_t x=0;x<WX+4;x++){
		for (uint8_t z=0;z<WZ+4;z++){// filling array with random values.
			hmap(x,z)=randInt(tmr,top);
		}
	}
	//now time to blur.
	for (uint8_t x=0;x<WX;x++){
		for (uint8_t z=0;z<WZ;z++){
			int total=0;
			for (int xo=0;xo<5;xo++){
				for (int zo=0;zo<5;zo++){// would say -2 but have to +2 anyways so 0
					total+=kernel[xo][zo]*hmap(x+xo,z+zo);
				}
			}
			//newhmap[x][z]=total/kernel_sum;
			//273 is kernel sum. magic numbers!
			
			vis_map(x,(uint8_t) ((total* 241) >> 16),z)=surfblock;
		}
	}
	//now time to fill in under grass
	for (uint8_t x=0;x<WX;x++){
		for (uint8_t z=0;z<WZ;z++){
			int yflag=0;
			for (int8_t y=top;y>=bottom;y--)
			{
				if(yflag==0)
				{
					if (vis_map(x,y,z)==surfblock)
						yflag=1;
				}
				else
				{
					vis_map(x,y,z)=surfblock;
					yflag++;				
				}
				if(yflag>4)
					vis_map(x,y,z)=underblock;//stone
			}
		}
	}
}
uint8_t biome=0;
void generate_world_underground()
{
	
	// now time to add some minerals!
	for (int i=0; i<IRONNUM;++i)
	{
		uint8_t x=randInt(0,WX-1);
		uint8_t y=randInt(4,12);
		uint8_t z=randInt(0,WZ-1);
		vis_map(x,y,z)=IRON;
	}
	for (int i=0; i<GOLDNUM;++i)
	{
		uint8_t x=randInt(0,WX-1);
		uint8_t y=randInt(2,10);
		uint8_t z=randInt(0,WZ-1);
		vis_map(x,y,z)=GOLD;
	}
	for (int i=0; i<DIAMONDNUM;++i)
	{
		uint8_t x=randInt(0,WX-1);
		uint8_t y=randInt(0,5);
		uint8_t z=randInt(0,WZ-1);
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
		az=(64+az)&63;//mod 64
		//angle->vector2d is (x*cos(angle),y*sin(angle))
		
		//3d (where have angles y and z) is x=cos(y)*cos(z),   y=sin(z),    z=sin(y)*cos(z)
		// y angle is xz, z angle is xy
		
		flint p5=.5f;
		int ix=tx;//+p5;
		int iy=ty;//+p5;
		int iz=tz;//+p5;
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
						if(ox>WX-1||oy>WY-1||oz>WZ-1)
						{
							i++;
							goto next;
						}
						vis_map(ox,oy,oz)=0;//255;
					}
				}
			}
			
			//now time to wobble the tunnel!
			ay+=randInt(-TUNNELWOBBLEF,TUNNELWOBBLEF);
			az+=randInt(-TUNNELWOBBLEU,TUNNELWOBBLEU);//how wiggly/wobbley the tunnels are
			if(ay<0)
				ay=64-ay;
			if(ay>63)
				ay=63;
			az=az&63;
			if(az<0)
				az=0;
			if(az>5&&az<30)
				az=5;
			if(az>30&&az<59)
				az=59;
			
		}
	}
	/*
	for (int b=0; b<WX*WY*WZ; ++b)
	{
		if(world[b]==255)
			world[b]=0;
	}
	*/
}
void generate_trees(int numtrees)
{
	for (uint8_t i=0;i<numtrees;i++){
		uint8_t x=randInt(3,WX-3);// don't want tree too close to edge of map...
		uint8_t z=randInt(3,WZ-3);
		int8_t y;
		for (y=WY-1;y>=0;y--){
			if (vis_map(x,y,z)&&vis_map(x,y,z)!=LEAF&&vis_map(x,y,z)!=WOOD)
				break;
		}
		y++;//brings from grass to space above grass
		if (y<WY-4){//if enough space to draw tree
			//draw tree!
			draw_tree(x,y,z);
			
		}
	}
}
void generate_cacti(int numcacti)
{
	for(int i=0; i<numcacti;++i)
	{
		uint8_t x=randInt(1,WX-1);
		uint8_t z=randInt(1,WZ-1);
		int8_t y;
		for (y=WY-1;y>=0;y--){
			if (vis_map(x,y,z)==SAND)
				break;
		}
		vis_map(x,y+1,z)=CACTUS;
		vis_map(x,y+2,z)=CACTUS;
		vis_map(x,y+3,z)=CACTUS;
	}
}
void draw_house(int hx, int y, int hz)
{
	//draws house from bottom left corner
	
	//drawing floor and roof...
	for(int x=hx;x<hx+6;++x)
	{
		for(int z=hz;z<hz+6;++z)
		{
			vis_map(x,y,z)=PLANKS;
			vis_map(x,y+4,z)=PLANKS;
		}
	}
	//drawing x walls...
	for(int x=hx;x<hx+6;++x)
	{
			vis_map(x,y+1,hz)=PLANKS;
			vis_map(x,y+2,hz)=PLANKS;
			vis_map(x,y+3,hz)=PLANKS;
			
			vis_map(x,y+1,hz+5)=PLANKS;
			vis_map(x,y+2,hz+5)=PLANKS;
			vis_map(x,y+3,hz+5)=PLANKS;
	}
	//drawing z walls...
	for(int z=hz;z<hz+6;++z)
	{
			vis_map(hx,y+1,z)=PLANKS;
			vis_map(hx,y+2,z)=PLANKS;
			vis_map(hx,y+3,z)=PLANKS;
			
			vis_map(hx+5,y+1,z)=PLANKS;
			vis_map(hx+5,y+2,z)=PLANKS;
			vis_map(hx+5,y+3,z)=PLANKS;
	}
	vis_map(hx+2,y+1,hz)=DOORBOTTOMOPEN;
	vis_map(hx+2,y+2,hz)=DOORTOPOPEN;
}
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
	memset(world,0,WX*WY*WZ);
	biome=randInt(0,4);//0 is grassland, 1 is snowy grassland 2 is desert,3 is forest 4 is snowy forest
	//now time to add random parts of snow for snow biome
	
	
	

	
	switch(biome)
	{
		case 0://grassland
			generate_terrain(GRASS,STONE,0,26,6);
			generate_trees(3);
			break;
		case 1://snowy grassland
			generate_terrain(SNOWGRASS,STONE,0,26,6);
			generate_trees(3);
			break;
		case 2://desert
			generate_terrain(SAND,SANDSTONE,0,26,6);
			generate_cacti(15);
			break;
		case 3://forest
			generate_terrain(GRASS,STONE,0,26,6);
			generate_trees(30);
			break;
		case 4:
			generate_terrain(SNOWGRASS,STONE,0,26,6);
			generate_trees(30);
			break;
	}
	if(biome==1||biome==4)
	{
		for (int i=0; i<15;++i)
		{
			uint8_t xs=randInt(3,WX-4);
			uint8_t zs=randInt(3,WZ-4);
			uint8_t temp=xs+randInt(1,3);
			for (uint8_t x=xs-randInt(1,3);x<temp;++x)
			{
				uint8_t temp2=zs+randInt(1,3);
				for (uint8_t z=zs-randInt(1,3);z<temp2;++z)
				{
					for(int8_t y=WY;y>=0;y--)
					{
						if(vis_map(x,y,z)==SNOWGRASS)
						{
							vis_map(x,y,z)=SNOW;
							break;
						}
					}
				}
			}
		}
	}
	generate_world_underground();
	outline_world_with_barrier();
	
}
void spawn_fire()
{
	for(int x=0; x<WX;++x)
	{
		for(int z=0; z<WZ;++z)
		{
			for (int y=28;y>0; --y)
			{
				if(vis_map(x,y,z))
				{
					if(randInt(0,64)==0)
						vis_map(x,y+1,z)=FIRE;
					break;
				}
			}
		}
	}
}
void generate_nether()
{
	/*Chicken jockeeyyy, the Nethaa, flint and steel enderpearrrlll*/
	/** plan for nether: 2 hmaps. One for top nether, one for bottom. just to be a pain.
	floor should be 4 netherack, roof should be 4. that leaves 28 for world. 14 for each half. with guasian, 
	will have hmap vary between 10 and 14. Then will apply bitmap of whether there should be a hole. 
	**/
	for(int x=0; x<WX;++x)
	{
		for(int z=0; z<WZ;++z)
		{
			vis_map(x,0,z)=MAGMABLOCK;
			vis_map(x,1,z)=MAGMABLOCK;//no lava but there are magmablocks!
			
			vis_map(x,31,z)=NETHERRACK;
			vis_map(x,30,z)=NETHERRACK;//nether roof
			vis_map(x,29,z)=NETHERRACK;
		}
	}
	generate_terrain(NETHERRACK,NETHERRACK,6,21,10);//lower section
	spawn_fire();
	generate_terrain(NETHERRACK,NETHERRACK,20,25,2);
	spawn_fire();
	outline_world_with_barrier();
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
gfx_sprite_t ** item_imgs;//            ={woodpic,stonepic,goldpic,ironpic,diamondpic,/*netheritepic*/NULL,stick,woodsword,stonesword,goldsword,ironsword,diamondsword,ironingot,goldingot,rawmutton,    cookedmutton,    rottenflesh};
static void (*use_item[])(       ){      NULL,    NULL,  NULL,   NULL,     NULL,            NULL,        NULL, NULL,       NULL,      NULL,   NULL,       NULL,       NULL,       NULL, use_rawmutton,use_cookedmutton,use_rottenflesh};
//                                  crafting table   furnace    planks   bed top glass   doortop   doorbottom magmablock  fire cacti, sandstone
const static uint8_t recog_text_side[]={1,            2,         2,        1,    0,       2        ,0,0,0,         1,     0,    0,  0};
const static uint8_t unmirror[]={3,2,1,0,7,6,5,4};//yeah... i have a mirroring problem with textures.
// this is based off of an array that goes {0,1,2,3,4,5,6,7,8} and shows what the mirroring does (used in unmirroring for draw_item
//because i mirrored all my textures so they don't look mirrored in the game.

// what side of the texture is most recognizable for the item label
extern "C" void Sprite_8x8to32x32(uint8_t* sprite, uint8_t* screenstart);
extern "C" void Sprite_16x16to32x32(uint8_t* sprite, uint8_t* screenstart);
inline void draw_item(int temp,int x,int y,int show_num)
{
	if(temp<reg_block_num+1)//solid block, 16x16-cached icon
	{
		Sprite_16x16to32x32(&blockspritebuffer[temp<<8],&gfx_vbuffer[y][x]);
	}
	else if(temp<=FLOORVOXELSEND)//floor block, flat diamond icon
	{
		gfx_SetColor(floorcolors[temp]);
		gfx_FillTriangle(x+7,y+7,x+2,y+28,x+28,y+7);
		gfx_FillTriangle(x+2,y+28,x+28,y+28,x+28,y+7);
	}
	else if(temp<invenblocknum+1)//textured block, 8x8-cached icon
	{
		Sprite_8x8to32x32(&texturespritebuffer[(temp-CRAFTTABLE)<<6],&gfx_vbuffer[y][x]);
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


inline void draw_itemnobuffer(int temp,int x,int y)
{
	//draws the item (like what you see in the hotbar or inventory or such)

	if(temp<reg_block_num+1)//if block
	{
		/*
		gfx_SetColor(block_colors_y[temp]);
		gfx_FillRectangle_NoClip(x,y,30,30);
		*/
		gfx_SetColor(0);
		gfx_FillRectangle_NoClip(x,y,30,30);
		gfx_SetColor(colors[temp]);
		gfx_FillTriangle(x+2,y+6, x+15,y+13, x+15,y+28);
		gfx_FillTriangle(x+2,y+23, x+2,y+6, x+15,y+28);
		
		gfx_SetColor(colors[temp+(reg_block_num+1)]);
		gfx_FillTriangle(x+2,y+6, x+15,y+2, x+15,y+13);
		gfx_FillTriangle(x+28,y+6, x+15,y+2, x+15,y+13);
		
		gfx_SetColor(colors[temp+2*(reg_block_num+1)]);
		gfx_FillTriangle(x+15,y+13, x+15,y+28, x+28,y+23);
		gfx_FillTriangle(x+15,y+13, x+28,y+6, x+28,y+23);
		

	}
	
	else if(temp<=FLOORVOXELSEND)//floor block, flat diamond icon
	{
		gfx_SetColor(floorcolors[temp]);
		gfx_FillTriangle(x+7,y+7,x+2,y+28,x+28,y+7);
		gfx_FillTriangle(x+2,y+28,x+28,y+28,x+28,y+7);
	}
	else if(temp<invenblocknum+1)
	
	{
		//dbg_printf("drawing: %d",temp);
		int ptemp=temp;
		int texidx=temp-CRAFTTABLE;          // 0-based texture index for recog_text_side[]
		int side=recog_text_side[texidx];    //side
		int id=((temp-(reg_block_num+1))<<2)+side;   // texturedata[] is anchored at id16 (floor rows included)
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
					gfx_FillRectangle_NoClip((realx<<2)+x, (realy<<2)+y, 4, 4);
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
	{ //bed 
        {WOOL,WOOL,WOOL},
        {PLANKS,PLANKS,PLANKS},
        {0     ,0,     0}
    },
	{//gold pickaxe
        {GOLDINGOT,GOLDINGOT,GOLDINGOT},
        {0        ,STICK    ,0},
        {0        ,STICK,    0}
    },
	{
		//gold sword
        {0,GOLDINGOT,0},
        {0,GOLDINGOT,0},
        {0   ,STICK, 0}
    }
};// index of the recipie to id
int recipie_to_id_array[RECIPIENUM]=   {PLANKS,STICK,WOODPIC,STONEPIC,IRONPIC,DIAMONDPIC,WOODSWORD,STONESWORD,IRONSWORD,DIAMONDSWORD, CRAFTTABLE, DOORTOPOPEN,FURNACE,BEDTOP,GOLDPIC, GOLDSWORD};
int num_items_from_recipie[RECIPIENUM]={4,       4      ,1,      1,      1,       1,          1,        1,        1,       1,               1,          3,       1,      1,    1,       1,};
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
	int scroll=0;
	const int VISIBLE_ROWS=12;
	while(1){
		kb_Scan();
		gfx_SwapDraw();
		gfx_FillScreen(255);
		gfx_SetDrawBuffer();
		int end=scroll+VISIBLE_ROWS;
		if(end>size)
			end=size;
		for(int i=scroll;i<end;i++){
			if (choice==i)
				gfx_SetTextBGColor(24);
			else
				gfx_SetTextBGColor(255);
			gfx_SetTextXY(1,(i-scroll)*20);
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
		if(choice>=size)
			choice=size-1;
		if(choice<scroll)
			scroll=choice;
		if(choice>=scroll+VISIBLE_ROWS)
			scroll=choice-VISIBLE_ROWS+1;
		//dbg_printf("choice: %d\n",choice);
		if(kb_IsDown(kb_KeyEnter))
		{
			gfx_SetTextBGColor(255);
			return choice;
		}
		if(kb_IsDown(kb_KeyClear))//exit
		{
			gfx_End();
			//free(world);
			return -1;
		}
		
	}
	
}
/*
void update_changes(int id, uint8_t* block)
{*/
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
	(either gonna be everything except the last block changed or just evrything although probably everything). id is the thing to change block to.**/
	/*
	if(block==NULL)return;
	//id is block to put, *block is block there.
	
	//3 options: puts block where it used to be, puts block where not suposed to be, puts block where prev block 
	//not supposed to be was
	
	unsigned short b=block-world;
	
	for(int i=0; i<prev_world_changes_sizes[id];++i)
	{
		if (prev_world_changes[id][i]==b)//block was one player already placed and alignes with seed
		{			
			dbg_printf("seed align\n");
			for(int j=0; j<world_changes_sizes[*block];++j)
			{
				
				if (world_changes[*block][j]==b)//if is block player prev placed there
				{
					world_changes[*block][j]=65535;
					for(int k=j+1;k<world_changes_sizes[*block]; k++)
					{
						world_changes[*block][k-1]=world_changes[*block][k];
					}
					world_changes[*block][world_changes_sizes[*block]-1]=65535;
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
			dbg_printf("block prev placed there\n");
			for(int k=i+1;k<world_changes_sizes[*block]; k++)
			{
				world_changes[*block][k-1]=world_changes[*block][k];
			}
			world_changes[*block][world_changes_sizes[*block]-1]=65535;
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
			dbg_printf("new\n");
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
	for(int j=0; j<MOBSTART;++j)
	{
		for(int i=0; i<prev_world_changes_sizes[j]; ++i)
		{
			if(prev_world_changes[j][i]==b)//if it is already in prev worldchanges
				return;
		}
	}
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

void chunk2world(int chx,int chz,int wherex,int wherez)
{
	wherex<<=4;
	wherez<<=4;
	static uint8_t tempchunk[16*16*16]={};
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
{\
 		foo67 = ti_Open(slotname, "r");\
		if(!ti_SetArchiveStatus(archived, foo67)) \
		{superstop=true;dbg_printf("failed to set archive :(\n");}\
        ti_Close(foo67);\
}

char wname[9]={};
char origwname[9]={};
bool is_chunk=false;

char number2char(uint8_t num)
{
	if(num<10)//0-9 - digits
		return num+'0';
	if(num<36)//10-36 - A-Z
		return num+55;
	if(num<62)//36-62 - a-z
		return num+61;
	return 0;
}
uint8_t char2number(char c)
{
	if(c<='9')
		return c-'0';
	if(c<='Z')
		return c-'A'+10;
	if(c<='z')
		return c-'a'+36;
	return 0;
}
bool appvar_exists(const char* name) {
    ti_var_t slot = ti_Open(name, "r");
    
    if (slot != 0) {
        ti_Close(slot);
        return true;
    }
    
    return false;
}
void srandchunkseed(unsigned short seed)
{
	unsigned int loadseed=seed&0xFFF;
	loadseed|=chunkx<<12;
	loadseed|=chunkz<<18;
	srand(loadseed);
	dbg_printf("seed: %d,chunkx: %d, chunkz: %d,loadseed %d\n",seed,chunkx,chunkz,loadseed);
}
void clear_changes()
{
	for (int i=0; i<MOBSTART;++i)
	{
		for (int b=0; b<world_changes_sizes[i];++b)
			world_changes[i][b]=65535;
	}
	/*
	for (int i=0; i<MOBSTART;++i)
	{
		for (int b=0; b<prev_world_changes_sizes[i];++b)
			prev_world_changes[i][b]=65535;
	}
	*/
}
uint8_t* update_changes_fully()
{
    if((uint8_t*)gfx_vbuffer != (uint8_t*)0xD40000)
        gfx_SwapDraw();
    
    uint8_t* realworld = (uint8_t*)0xD40000 + 1300;//0xD40000 is screen start, and hmap in generate world usises that + 1294 bytes, so do +1300
    memcpy(realworld, world, WX * WY * WZ);
    
    dbg_printf("wseed: %d,chunkx: %d, chunkz: %d\n", wseed, chunkx, chunkz);
    
    srandchunkseed(wseed);
    generate_world();
    
    memset(world_changes_sizes, 0, sizeof(world_changes_sizes));
    
    size_t size = WX * WY * WZ;
    uint8_t *ptr = realworld;
    uint8_t *ptr2 = world;
    
    do {
		if(*ptr > MOBSTART-1)//if mob
			*ptr=0;//turn all mobs into air. don't use .clear() in case mob isn't spawned (like zombie in day)
        if(*ptr != *ptr2)
            world_changes_sizes[*ptr]++; 
        ptr++;
        ptr2++;
    } while (--size); 
    
    for(int i = 0; i < MOBSTART; ++i)
        world_changes_sizes[i]++;
    
    uint16_t* nextaddr = (uint16_t*)(0xD40000 + 1310 + WX * WY * WZ);
    for(int i = 0; i < MOBSTART; ++i)
    {
        world_changes[i] = nextaddr;
        nextaddr += world_changes_sizes[i];
        world_changes[i][world_changes_sizes[i] - 1] = 65535;
    }
    
    uint8_t* realw = realworld;
    uint8_t* genw = world;
    size = WX * WY * WZ;
    
    int next[block_num];
    memset(next, 0, sizeof(next));
    
    do {
        if(*realw != *genw)
        {
            uint8_t temp = *realw;
            if(temp < MOBSTART)
            {
                world_changes[temp][next[temp]] = (uint16_t)(realw - realworld);
                next[temp]++;
                dbg_printf("change of %d at %d\n", temp, (int)(realw - realworld));
            }
        }
        realw++;
        genw++;
    } while (--size);
	return (uint8_t*)nextaddr;
}
uint8_t tchunkx=32;
uint8_t tchunkz=32;
void archiveallchunks(char* name);
#include <fileioc.h>
#include <string.h>

/** generated by gemini flash v v v**/
void clear_file_contents(uint8_t slot) {
    uint16_t size;
    uint16_t bytes_written = 0;
    uint8_t zero_buffer[32]; // Small buffer to speed up writing
    
    // 1. Get the current total size of the file
    size = ti_GetSize(slot);
    if (size == 0) return; 

    // 2. Move file pointer to the very beginning
    ti_Rewind(slot);

    // Initialize our cleaning buffer to zeros
    memset(zero_buffer, 0, sizeof(zero_buffer));

    // 3. Overwrite the entire file size with zeros
    while (bytes_written < size) {
        uint16_t to_write = size - bytes_written;
        if (to_write > sizeof(zero_buffer)) {
            to_write = sizeof(zero_buffer);
        }
        
        ti_Write(zero_buffer, to_write, 1, slot);
        bytes_written += to_write;
    }

    // 4. Rewind again so your next operations start at byte 0
    ti_Rewind(slot);
}

void save_world(char* wname)
{
	//memset(world,255,WX*WY*WZ);
	/**IMPORTANT COMMENT FOR PEOPLE READING THIS CODE!!!**/
	/*the ti 84 plus ce has 153600 of vram. graphx uses that to make two buffers.
	basically what I do is start writing to the beginning in the back buffer, so it doesn't appear on the scrren
	but if I need more ram, it will spill over the first buffer and enter the visible screen. The worst case scenario
	of memory is under 153600 (3000 for hmap)+(32768 for world) + (58000 for all the changes) + (58000 again, but this is for the back up of file)
	is under 153600 (it is 150400 around).*/
	dbg_printf("saveworld wname: %s\n",wname);
	unsigned short tempseedshort;
	if(appvar_exists(wname))
	{
		set_archive(wname,false);
	}
	uint8_t* nextaddr=update_changes_fully();	
	
	dbg_printf("wname: %s\n",wname);
	ti_var_t worldslot=ti_Open(wname,"r");
	auto oldsize=ti_GetSize(worldslot);
	
	ti_Read(nextaddr, sizeof(uint8_t),oldsize, worldslot);
	ti_Close(worldslot);
	worldslot=ti_Open(wname,"w");
	unsigned short sizes[MOBSTART]={};
	int sum=0;
	for(int i=0; i<MOBSTART; ++i)
	{
		sizes[i]=world_changes_sizes[i];
		sum+=world_changes_sizes[i];
	}
	int i=sizeof(int)*(block_num+item_num);
	if (is_chunk)
		i=0;
	//safe
	bool rflag=1;
	dbg_printf("sum*2: %d\n", sum*2);
	resize:
	int totalsize=	  32
	+ 66
	+ sum*2
	+ sizeof(int)
	+ i
	+ 8
	+ sizeof(int)*3
	+ sizeof(uint32_t)
	+ sizeof(int)*3;
	
	bool test=ti_Resize(totalsize,worldslot);
	if(!test)
	{
		gfx_FillScreen(BG_COLOR);
		gfx_SetTextFGColor(224); // Assuming 224 is Red in your palette
		gfx_SetTextXY(10, 10);
		gfx_PrintString("NOT ENOUGH RAM!!! Archiving some stuff...");
		gfx_SwapDraw();
		msleep(1000);
		ti_Close(worldslot);
		superstop=true;
		if(rflag)
		{
			archiveallchunks(origwname);
			set_archive(origwname,true);
			set_archive(wname,false);
			if(world!=NULL)
				free(world);
			world=NULL;
			worldslot=ti_Open(wname,"w");
			rflag=0;
			goto resize;
		}
		//ok, now everything is screwed because can't save world.
		gfx_FillScreen(BG_COLOR);
		gfx_SetTextFGColor(224); // Assuming 224 is Red in your palette
		gfx_SetTextXY(10, 10);
		gfx_PrintString("Unable to save world");
		gfx_SwapDraw();
		worldslot=ti_Open(wname,"w");
		if(ti_Resize(oldsize,worldslot))
		{
			ti_Write(nextaddr, 1, oldsize, worldslot);//restore file
		}
		ti_Close(worldslot);
		msleep(1000);
		goto reallocingworld;
	}
	//clear_file_contents(worldslot);
	ti_Seek(0, SEEK_SET, worldslot);
	tempseedshort=wseed;
	ti_Write(&tempseedshort,2,1,worldslot);
	ti_Write(sizes,2,MOBSTART,worldslot);	
	//ti_Seek(66, SEEK_SET, worldslot);
	for(int i=0; i<MOBSTART; ++i)
	{
		ti_Write(world_changes[i],2,sizes[i],worldslot);
	}
	/*
	dbg_printf("freeing world_changes\n");
	for(int i=0; i<MOBSTART; ++i)
	{
		free(world_changes[i]);
		free(prev_world_changes[i]);
	}
	*/
	//writing inventory
	//int inventory[block_num+item_num]; for reference
	//safe
	if(!is_chunk)//if is in main chunk
	{
		int num2read2[1];
		num2read2[0]=block_num+item_num;
		ti_Write(num2read2,sizeof(int),1,worldslot);
		ti_Write(inventory,sizeof(int),block_num+item_num,worldslot);
		
		ti_Write(ghotbar, sizeof(uint8_t), 8, worldslot);//saving hotbar...
		
		int tempplarray[]={plx.data,ply.data,plz.data};	
		ti_Write(tempplarray,sizeof(int),3,worldslot);//saving player's position
		
		uint32_t temptime=clock()-*gstarttime;//time elapsed
		temptime+=offsettime;
		uint32_t value=32768;
		value=value*60*20;//to avoid overflow
		temptime=temptime%value;//make repeat every 20 minutes (daylight cycle)
		ti_Write(&temptime,sizeof(uint32_t),1,worldslot);//saving time
		ti_Write(&respawnx,sizeof(int),1,worldslot);
		ti_Write(&respawny,sizeof(int),1,worldslot);
		ti_Write(&respawnz,sizeof(int),1,worldslot);
		
		ti_Write(&tchunkx,sizeof(uint8_t),1,worldslot);
		ti_Write(&tchunkz,sizeof(uint8_t),1,worldslot);
		dbg_printf("tchunkx: %d, tchunkz:%d\n",tchunkx,tchunkz);
		ti_Write(&respawnchunkx,sizeof(uint8_t),1,worldslot);
		ti_Write(&respawnchunkz,sizeof(uint8_t),1,worldslot);
	}
	ti_Close(worldslot);
	
	dbg_printf("closed\n");

	gfx_FillScreen(BG_COLOR);
	gfx_SwapDraw();
	gfx_FillScreen(BG_COLOR);
	gfx_SwapDraw();
	
	reallocingworld:
	if(world==NULL)
	{
		world=(uint8_t*)malloc(32768);
		if(world==NULL)
		{
			archiveallchunks(origwname);
			set_archive(origwname,true);
			gfx_End();
			os_ClrHome();
			os_HomeUp();
			printf("sorry, unable to find enough memory to load the world. Side effects might include an outdated inventory.");
			os_GetKey();
			exit(0);
		}
	}
}
//void loadnewchunk();
void load_world(char* wname)
{
	
	unsigned short sizes2[MOBSTART];
	ti_var_t worldslotr=ti_Open(wname,"r");
	if(worldslotr==0)
		dbg_printf("not real file\n");
	short x;
	ti_Read(&x, sizeof(short),1,worldslotr);
	short seed=0;
	seed=x;
	

	memcpy(wname,origwname,6);
	wname[6]=number2char(chunkx);
	wname[7]=number2char(chunkz);
	
	if(chunkx==32&&chunkz==32)
	{
		dbg_printf("main chunk\n");
		memcpy(wname,origwname,9);
	}
	wseed=seed;
	srandchunkseed(seed);
	generate_world();
	
	dbg_printf("seed %d, %s\n",seed,wname);
	//order is 2seed,32size,changes
	
	ti_Read(sizes2, sizeof(short),MOBSTART,worldslotr);
	int i=0;
	while(i<MOBSTART)//should used for but already put while
	{

		//ti_Read(world_changes[i], 2,sizes2[i],worldslotr);
		for (int j=0; j<sizes2[i];++j)
		{
			unsigned short change;
			ti_Read(&change, 2,1,worldslotr);
			if(change<WX*WY*WZ)
				world[change]=i;
		}
		++i;
	}
	if(!is_chunk)
	{
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
		
		ti_Read(&chunkx,sizeof(uint8_t),1,worldslotr);
		ti_Read(&chunkz,sizeof(uint8_t),1,worldslotr);
		
		ti_Read(&respawnchunkx,sizeof(uint8_t),1,worldslotr);
		ti_Read(&respawnchunkz,sizeof(uint8_t),1,worldslotr);
		
		dbg_printf("chunkx: %d, chunkz:%d, respawncx %d, respawn cz %d\n",chunkx,chunkz,respawnchunkx,respawnchunkz);
	}
	
	ti_Close(worldslotr);
	is_chunk=true;
	if(chunkx==32&&chunkz==32)
	{
		is_chunk=false;
	}
	
}


void deleteallchunks(char* name)
{
	for(int x=0; x<62;++x)
	{
		for(int z=0; z<62;++z)
		{
			char temp[9]={};
			memcpy(temp,name,6);
			temp[6]=number2char(x);
			temp[7]=number2char(z);
			if(appvar_exists(temp))
				ti_Delete(temp);
		}
	}
}
void archiveallchunks(char* name)
{
	for(int x=0; x<62;++x)
	{
		for(int z=0; z<62;++z)
		{
			if(x!=32||z!=32)
			{
				char temp[9]={};
				memcpy(temp,name,6);
				temp[6]=number2char(x);
				temp[7]=number2char(z);
				if(appvar_exists(temp))
				{
					set_archive(temp,true);
				}
			}
		}
	}	
}

void loadnewchunk()
{

	//for(int i=0; i<SHEEPNUM;++i)
	//	sheep[i].clear();
	for(int i=0; i<ZOMBIENUM; ++i)
		zombies[i].alive=false;

	updatehotbar=2;
	is_chunk=true;
	
	if(chunkx==32&&chunkz==32)
	{
		memcpy(wname,origwname,9);
	}
	else//if isn't main chunk
	{
		memcpy(wname,origwname,6);
		wname[6]=number2char(chunkx);
		wname[7]=number2char(chunkz);
	}
	
	

	ply=WY-1;
	//clear_changes();
	dbg_printf("cleared changes\n");

	if(appvar_exists(wname))//if have loaded chunk
	{
		dbg_printf("loaded chunk\n");
		load_world(wname);
	}
	else//if is new chunk
	{
		srandchunkseed(wseed);
		generate_world(); 
	}
	if(biome==0)
	{
		for(int i=0; i<SHEEPNUM;++i)
			sheep[i].spawn();
	}
	if(chunkx==32&&chunkz==32)
	{
		is_chunk=false;
	}
}

inline bool walk_thru_block(int blockid)
{
	if(blockid==0)
		return true;
	if(blockid>MOBSTART)
		return true;
	if(blockid>reg_block_num&&blockid<FLOORVOXELSEND+1)//if is a floor voxel
		return true;
	return false;
}
void try_walk(flint dirx, flint dirz)
{
	if(randInt(0,100)==0)
	{
		playerhunger--;
		updatehotbar=2;
	}
	int nx=(int)(dirx+plx);
	int nz=(int)(dirz+plz);
	
	int iplx=(int)plx;
	int iply=(int)ply;
	int iplz=(int)plz;
	//                                               door
	if(plx+dirx>30&&chunkx<61)
	{
		save_world(wname);
		chunkx++;
		loadnewchunk();
		plx=1;
	}
	else if(plx+dirx<1&&chunkx>0)
	{
		save_world(wname);
		chunkx--;
		loadnewchunk();
		plx=30;
	}
	else if(plz+dirz>=32&&chunkz<61)//z barrier is only on 0 due to helpful properties of flattened arrays
	{
		save_world(wname);
		chunkz++;
		loadnewchunk();
		plz=1;
	}
	else if(plz+dirz<1&&chunkz>0)
	{
		save_world(wname);
		chunkz--;
		loadnewchunk();
		plz=31;
	}
	else
	{
		if ( (walk_thru_block(vis_map(nx,iply,iplz))&&walk_thru_block(vis_map(nx,iply-1,iplz))) || vis_map(nx,iply,iplz)==DOORTOPOPEN)
			plx+=dirx;
		iplx=(int)plx;//updates it
		if ( (walk_thru_block(vis_map(iplx,iply,nz))&&walk_thru_block(vis_map(iplx,iply-1,nz))) ||vis_map(iplx,iply,nz)==DOORTOPCLOSED)
			plz+=dirz;
	}
}

void gc_before(void)
{
	gfx_End();
}

void gc_after(void)
{
	gfx_Begin();
	gfx_SetDefaultPalette(gfx_8bpp);
	gfx_SetDrawBuffer();
	MNCFTIMG_init();
}

int main(void){
	auto *data_ptr = os_GetAppVarData("MNCFTIMG", NULL);

	if (data_ptr == NULL)//no asset file
	{
		os_ClrHome();
		os_HomeUp();
		printf("Uh oh! you (or whoever sent this game) forgot to send the assets file (it has all the item images and stuff) to the calculator! Please send the file titled MNCFTIMG.8xv");
		os_NewLine();
		os_NewLine();
		printf("Press any key to continue");
		os_GetKey();
		return 6767;
	}
		
	//init important stuff
	world=(uint8_t*)malloc(32768);
	if(world==NULL)
	{
		os_HomeUp();
		os_ClrHome();
		printf("not enough ram!!! free up some stuff");
		os_GetKey();
	}
	ti_SetGCBehavior(gc_before, gc_after);
	MNCFTIMG_init();
	gfx_sprite_t* tempimgs[]={woodpic,stonepic,goldpic,ironpic,diamondpic,/*netheritepic*/NULL,stick,woodsword,stonesword,goldsword,ironsword,diamondsword,ironingot,goldingot,rawmutton,    cookedmutton,    rottenflesh};
	gfx_sprite_t* tempimgs2[]={emptyfurnaceflames,furnaceflames1,furnaceflames2,furnaceflames3,furnaceflames4,furnaceflames5};
	
	item_imgs=tempimgs;
	flames=tempimgs2;
	for (int i=0;i<64;i++)
	{
		float angle = i * (6.283185f / 64.0f);
		cached_sin_rot[i].float2flint((float)std::sin(angle));
		cached_cos_rot[i].float2flint((float)std::cos(angle));

	}
	beginning:
	dbg_printf("began!\n");
	ti_var_t worldslot;
	ti_var_t nslot = ti_Open("MNCFTD", "r");// main appvar that has list of worlds and stuff
	if (nslot==0){//not exist
					//version,number of worlds
		uint8_t header[3] = {0, 0};
		set_archive("MNCFTD",false);
		ti_var_t slot = ti_Open("MNCFTD", "w");
		ti_Resize(1024, slot);
		ti_Write(header, sizeof(uint8_t), 2, slot);
		ti_Close(slot);
		set_archive("MNCFTD",true);
		//what had before
	}
	ti_Close(nslot);
	ti_var_t slot = ti_Open("MNCFTD", "r");
	gfx_Begin();
	//ti_var_t worldslotr;
	int seed=0;
	const char *options[]={
		"Exit",
		"Create New",
		"Open World",
		"Delete World",
	};
	
	uint8_t* header=(uint8_t*)world;
	ti_Read(header, sizeof(uint8_t),2048,slot);
	ti_Close(slot);
	uint8_t nheader[22]={};//16 bytes for name, 6 for appvar
	
	/*
	ti_var_t tempslot = ti_Open("TEMPVR67", "w");
	if(tempslot==0)
	{
		gfx_End();
		return 4;
	}
	
	auto didwork=ti_Resize(40000,tempslot);
	ti_Close(tempslot);
	ti_Delete("TEMPVR67");
	if(didwork==0)
	{
		gfx_End();
		os_ClrHome();
		printf("Sorry! not enough ram to run program :(");
		os_GetKey(); 
		return 3;
	}
	*/
	int choice=menu(options,4);
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
	 *   [2-59]   58B         sizes[MOBSTART] (unsigned short each) =
	 *                        # of valid (non-65535) entries per block ID
	 *                        in world_changes[], NOT array capacity
	 *   [60..]   sum(sizes[i])*2 B
	 *                        for i=0..MOBSTART-1: sizes[i] entries of unsigned short,
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
		 (a bunch of world headers) [22 bytes each]
		 
	world headers:
		
		world name				[16 bytes]
		appvar name for world	[6 bytes]
		 
	world:
		seed	 									   [2 bytes]
		sizes of changes to seeded world 			   [64 bytes]
	    size of block_num+item_num when world was made [3 bytes]
		inventory                                      [3*(block_num+item_num) bytes]
		hotbar                                         [8 bytes]
		player's position in fixed point format        [3 bytes]
		game time (for daylight cycle)                 [1 bytes]
	 **/
	if(choice==-1)
	{
		gfx_End();
		return 0;
	}
	if(choice==0)
	{
		gfx_End();
		return 0;
	}
	if (choice==1){//creating new
		if(header[1]>39)
		{
			os_HomeUp();
			os_ClrHome();
			printf("Sorry, this game has a cap of 39 worlds! Please delete a world if you truly want to make another one");
			os_GetKey();
			goto beginning;
		}
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
		//int count=header[1];//this is how many worlds there are. the catch: if one was deleted, could accidentaly overwrite last world.
		
		int count=0;
		for (;count<=header[1];++count)
		{	
			bool flag=true;
			for(int i=2; i<header[1]*22+2;i+=22)//skipping start bits, reading world
			{
				
				char tempc[4];
				//+19 because +16 for name, +3 for MCO
				memcpy(tempc,&header[i+19],3);
				tempc[3]=0;//null terminated
				if(atoi(tempc)==count)
				{
					flag=false;
					break;
				}
				
			}
			if(flag)
				break;
		}
		//ok, now count should be the highest number that won't break anything
		char temp[4];
		std::snprintf(temp, sizeof(temp), "%03d", count); 
		nheader[19]=temp[0];nheader[20]=temp[1];nheader[21]=temp[2];// adding number to header. now MCO167 or something
		header[1]=header[1]+1;
		dbg_printf("Temp: %s",temp);
		uint8_t* newheader=(uint8_t*) &world[16384];// header 1 is how many worlds, 22 bytes per world, 2 starting bytes
		int c=(header[1]-1)*22+2;//-1 bcs inc up there
		for (int i=0; i<c; i++)
			newheader[i]=header[i];//adds old header stuff
		for (int i=0;i<22;i++)
			newheader[i+c]=nheader[i];
		set_archive("MNCFTD",false);
		ti_var_t slotw = ti_Open("MNCFTD", "w");
		ti_Write(newheader,sizeof(uint8_t),2+22*40,slotw);
		ti_Close(slotw);//saves the new header
		set_archive("MNCFTD",true);
		char temp2[9]={};
		temp2[0]='M';temp2[1]='C';temp2[2]='O';temp2[3]=temp[0];temp2[4]=temp[1];temp2[5]=temp[2];temp2[6]=0;
		worldslot=ti_Open(temp2, "w");
		ti_Resize(64, worldslot);
		char buffer2[17]={};
		while (kb_IsDown(kb_KeyEnter)){
			kb_Scan();
		}
		os_GetStringInput("Enter seed: ", buffer2,sizeof(buffer2));
		seed=atoi(buffer2);
		srandchunkseed(seed);
		generate_world();
		wseed=seed;
		memcpy(origwname,temp2,9);
		memcpy(wname,temp2,9);
	
	unsigned short seedshort=seed;
	
	dbg_printf("seed start: %d, %s\n",seed,temp2);
	//world syntax is 1st two bytes is seed
	ti_Write(&seedshort,sizeof(unsigned short),1,worldslot);//writes seed
	wseed=seed;
	ti_Close(worldslot);
	}
	else if(choice==2)//loading world
	{
		char **opt = (char**)malloc(header[1]* sizeof(char*));
		for (int i = 0; i < header[1]; i++) 
			opt[i] = new char[17]();
		int c=0;
		for(int i=2; i<header[1]*22+2;i+=22)//skipping start bits, reading world
		{
			memcpy(opt[c],&header[i],16);
			c++;
		}
		msleep(500);
		if(header[1]==0)
			goto beginning;
		int wchoice=menu((const char**)opt,header[1]);
		if(wchoice==-1)
			return 0;
		dbg_printf("wchocie: %d",wchoice);
		
		
		memcpy(wname,header+wchoice*22+18,6);
		for (int i = 0; i < header[1]; i++) {
			delete[] opt[i];
		 }
		free(opt);
		memcpy(origwname,wname,9);
		dbg_printf("loding wrld\n");
		load_world(wname);
		dbg_printf("ply that was loaded: %d\n",(int)ply);
		flint tempply=ply;
		loadnewchunk();
		ply=tempply;
		
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
			if(wchoice==-1)
				return 0;
			gfx_Begin();
			dbg_printf("wchocie: %d",wchoice);
			
			
			memcpy(wname,header+wchoice*22+18,6);
			for (int i = 0; i < header[1]; i++) {
				delete[] opt[i];
			 }
			free(opt);
			while (kb_IsDown(kb_KeyEnter))//to catch stray keydown
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
				char appvarname[7]={};
				for(int i=0;i<7;++i)
				{
					appvarname[i]=header[actualindex+16+i];//see big huge freaking comment above
				}
				appvarname[6]=0;//so is null terminated
				ti_Delete(appvarname);//ok, so deleted appvar. Now need to remove it from header.
				deleteallchunks(appvarname);
				
				int length=header[1]*22+2;
				length-=actualindex;
				length-=22;
				dbg_printf("header[1]: %d, length: %d, actualindex: %d, appvarname: %s", header[1], length, actualindex, appvarname);
				memcpy(&header[actualindex],&header[actualindex+22], length);
				header[1]-=1;
				set_archive("MNCFTD",false);
				ti_var_t slotw = ti_Open("MNCFTD", "w");
				ti_Write(header,sizeof(uint8_t),header[1]*22+2,slotw);
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
	{
		gfx_Begin();
		gfx_SetDrawBuffer();
		gfx_FillScreen(0);
		for(int i=0; i<reg_block_num+1;++i)   // solid blocks: cache at 16x16
		{
			gfx_FillScreen(0);
			draw_itemnobuffer(i,0,0);
			int c=i<<8;
			for(int y=0; y<32; y+=2)
			{
				for(int x=0; x<32; x+=2)
				{
					blockspritebuffer[c]=gfx_vbuffer[y][x];
					++c;
				}
			}
		}
		for(int i=CRAFTTABLE; i<invenblocknum+1;++i)   // textured blocks: cache at 8x8
		{
			gfx_FillScreen(0);
			draw_itemnobuffer(i,0,0);
			int c=(i-CRAFTTABLE)<<6;
			for(int y=0; y<32; y+=4)
			{
				for(int x=0; x<32; x+=4)
				{
					texturespritebuffer[c]=gfx_vbuffer[y][x];
					++c;
				}
			}
		}
		gfx_End();
	}
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
	
	//char buffer[10];
	//gfx_End();
	
	// doing caching for speeeeed
	flint fl256=256;
	
	inv_table=inv_tabler+750;
	for (int i = -750; i < 750; i++) {
		if (i == 0) 
			inv_table[i] = 300000; 
		else 
		{
			inv_table[i] = fl256/i;// i/(i/512) = 512/i
			inv_table[i].data=fast_abs(inv_table[i].data);
		}
	}

	point01.float2flint(.01f);//exactly what it looks like
	focal_inv.float2flint(1/(float)focal_length);

	
	flint max=0;

	
	//good
	for (int i=-160;i<160;i++){//slopes. (x1-x2)/(y1-y2)
		//points= (i,focal),(0,0)      (i-0)/(focal_length-0) xz
		cached_x_dir[i+160].float2flint(i/(float)focal_length);
		cached_x_dir[i+160]*=step_divide;

		if (i==0)
			cached_x_dir[i+160].data=0x1;//smallest flint value
		if (cached_x_dir[i+160].flint_abs()>max)
			max=cached_x_dir[i+160];
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
	//void *free_ram_ptr;
    //size_t free_ram_size = os_MemChk(&free_ram_ptr);

    // This will appear in the CEmu Console (Docks > Console)

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
	//generate_world();
	//dbg_printf("generating world...\n");
	//generate_chunk(world);
	//dbg_printf("generated world!\n");
	//gohere working on textures. some crazy mirror stuff, but that isn't too bad. only shows part of texture sometimes.
	//BUG IN DOOR! SHEEP
	//can't fix ^ ^ ^ fisheye distorition by noneuclidean distance (taxicab). TODO: rasterization baby!!!! but first chunking :(
	//also, need more textured blocks.
	
	gfx_SetTextBGColor(247);
	//vis_map(1,3,1)=reg_block_num+6;
	//vis_map(1,2,1)=reg_block_num+7;
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
	if(biome==0)
	{
		for(int i=0; i<SHEEPNUM;++i)
			sheep[i].spawn();
	}
	//end mob stuff
	gfx_SetTextFGColor(0);
	superstop=false;
	int wtime=0;
	alive=true;
	gfx_SetTextTransparentColor(26);

	//goto end;
	while (1)
	{		
		playerhp=10;
		playerhunger=20;
		if (!alive)
		{
			plx=respawnx;
			ply=respawny;
			plz=respawnz;
			if(chunkx!=respawnchunkx||chunkz!=respawnchunkz)
			{
				save_world(wname);
				chunkx=respawnchunkx;
				chunkz=respawnchunkz;
				flint tempply=ply;
				loadnewchunk();
				ply=tempply;
			}

			//add respawn chunkx and z
		}
		alive=true;
		gfx_FillScreen(BG_COLOR);
		updatehotbar=2;
		//dbg_printf("magmablock: %d fire: %d\n",MAGMABLOCK,FIRE);
		int lspeed=0;//look speed multiplier
		while (alive){
			dbg_printf("gfx_vbuffer=%p\n", (void*)gfx_vbuffer);
			call_update();
			frame+=1;
			//dbg_printf("chunkx: %d, chunkz: %d\n",chunkx,chunkz);
			//dbg_printf("plx: %d, plz: %d\n",(int)plx,(int)plz);
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
			
			if((frame&15)==0)//every 3 ish seconds
			{
				if(playerhunger>17&&playerhp<10)
				{
					playerhp+=1;
					playerhunger-=randInt(1,2);
					updatehotbar=2;
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
				colors[5]=BG_COLOR;
				colors[5+reg_block_num+1]=BG_COLOR;//changing color of barrier block
				colors[5+2*(reg_block_num+1)]=BG_COLOR;
			}
			//mob stuff!
			if(biome==0)
			{
				
				for(int i=0; i<SHEEPNUM;++i)
				{
					if(randInt(0,AVERAGEFPS)==0){
						sheep[i].move();
						sheep[i].randturn();
					}
				}
				
			}
			
			//gfx_FillScreen(BG_COLOR); 
			gfx_Wait();
			notfullfillscreen(BG_COLOR,&gfx_vbuffer[0][0]);
			kb_Scan(); // Update the key status
			if(kb_IsDown(kb_KeyMath))
			{
				keyup(kb_KeyMath);//wait until release
				lspeed=!lspeed;
			}
			flag=1;
			if(superstop)
			{
				goto end;
			}
			if (kb_IsDown(kb_KeyRight)) {
				rotxz-=2<<lspeed;
				if (rotxz<0)
					rotxz=63;
				
				
			}
			else if (kb_IsDown(kb_KeyLeft)) {
					  
				rotxz+=2<<lspeed;
				if(rotxz>63)
					rotxz-=64;
			}
			//15 is bottom, 0 is middle, 49 is top
			else if (kb_IsDown(kb_KeyDown)){
				if(rotyz<15||rotyz>48) {
					  
				rotyz+=1<<lspeed;
				if(rotyz>63)
					rotyz-=64;
				}
			}
			else if (kb_IsDown(kb_KeyUp)){
				if (rotyz<16||rotyz>49) 
				{				
					if (rotyz<1)
						rotyz=64;				
					rotyz-=1<<lspeed;
				}
				if(rotyz==16)
					rotyz=15;
				if(rotyz<0)
					rotyz=0;
				if(rotyz==48)
					rotyz=49;
			}
			else
				flag=0;
			if (flag)
			{
				dirx=0;dirz=1;//for movement. movement vector if move.
				rot_point_2d(dirx,dirz,0,0,rotxz);
				if(dirx>1)
					dirx=1;
				if(dirx<-1)
					dirx=-1;
				if(dirz>1)
					dirz=1;
				if(dirz<-1)
					dirz=-1;
			}
			//dbg_printf("X: %d, Y: %d, Z: %d\n",(int)plx,(int)ply,(int)plz);
			if (kb_IsDown(kb_Key2nd)) 
				try_walk(dirx,dirz);
			else if (kb_IsDown(kb_KeyAlpha)) {
				try_walk(-dirx,-dirz);
			}

			 if(ply>WY-2)
				 ply=WY-2;
			 if(ply<0)
				 ply=0;
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
				int scroll=0;
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
					int y=2;
					if((uint8_t)scroll%12)
						y=6;
					if(where==0)
					{
						if(cursor-scroll<36)
							scroll-=6;
						if(cursor-scroll>35)
							scroll+=6;
						if(scroll<0)
							scroll=0;
					}
					else
						scroll=0;
					dbg_printf("Scroll: %d, Cursor: %d\n",scroll,cursor);
					int count=0;
					for (int i=scroll+1;i<invenblocknum+item_num;++i){//drawing inventory. 0 is air.
						
						if(count>35)
							break;
						count++;
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

						
						if(cursor>=invenblocknum+item_num-1)
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
						
						gfx_FillRectangle_NoClip(x,y,30,30);
						if (flag){
							draw_item(i,x,y,1);
						}
						/*
						if(i<DOORBOTTOMOPEN||i>DOORBOTTOMCLOSED)
							x+=33;
						*/
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
								gfx_FillRectangle_NoClip(205+x*40,5+y*40,30,30);
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
						crafting_table[1][1][1]=inventory[id];
						if(cursorblock==id)
						{
							cursornum=0;
							cursorblock=0;
						}
					}
				}
				
				if (kb_IsDown(kb_KeyDown))
				{
					if(prevdown){
						if (where==0)
						{
							//int pcursor=cursor;
							cursor+=6;
							/*
							if(pcursor<DOORBOTTOMCLOSED+1&&cursor>DOORTOPOPEN)
								cursor+=3;
							*/
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
							//int pcursor=cursor;
							cursor-=6;
							/*
							if(pcursor>DOORBOTTOMCLOSED&&cursor<DOORTOPOPEN+1)
								cursor-=3;
							*/
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
						/*
						if(cursor>DOORTOPOPEN&&cursor<DOORBOTTOMCLOSED+1)
							cursor=DOORBOTTOMCLOSED+1;
						*/
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
						/*
						if(cursor<DOORBOTTOMCLOSED+1&&cursor>DOORTOPOPEN)
							cursor=DOORTOPOPEN;
						*/
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
			updatehotbar=2;
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
								if(bv>DOORTOPOPEN-1&&bv<DOORBOTTOMCLOSED+1)//door, not door top open (regular)
								{
									iv=DOORTOPOPEN;
								}
								inventory[iv]++;
								add2hotbar(iv);
							}
							//dbg_printf("update changes...\n");
							//update_changes(0,block);
							*block=0;
							add_updates_fully(block);
							//dbg_printf("assign block...\n");
							if(iv!=bv)//is door
							{
								if(bv==DOORTOPCLOSED||bv==DOORTOPOPEN)
								{
									block-=YJ;//moves down one in y								
								}
								else//bottom door part
									block+=YJ;
								//update_changes(0,block);
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
				updatehotbar=2;
				if (id>invenblocknum)//item
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
				if (id<invenblocknum+1){// if not item

					
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
							//update_changes(DOORBOTTOMOPEN,bottomblock);
							*bottomblock=DOORBOTTOMOPEN;
						}
						//update_changes(id,block);
						*block=id;
						add_updates_fully(block);
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
				updatehotbar=2;
			}
			else if (kb_IsDown(kb_KeySub))
			{
				hotbarblock--;
				if(hotbarblock<0)
					hotbarblock=7;
				updatehotbar=2;
			}
			endplace:
			if (kb_IsDown(kb_KeyClear)) {
				goto end;
			}
			
			int temp2=vis_map((int)plx,(int)ply-1,(int)plz);
			if (!walk_thru_block(temp2))// if feet stuck in ground
			{
				if(!(temp2==DOORBOTTOMCLOSED||temp2==DOORBOTTOMOPEN))//demorganed it!
					ply+=1;
			}
			if (walk_thru_block(vis_map((int)plx,(int)ply-2,(int)plz))||grav<0){
				grav+=grav_amount;
				ply-=grav;
				temp2=vis_map((int)plx,(int)ply,(int)plz);
				if (!walk_thru_block(temp2))//if head stuck in wall
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
						updatehotbar=2;
					}
				}
				else
					grav=0;
			}
			//dbg_printf("frame!,%d,%d",rotxz,rotyz);
			
			std::clock_t t1 = std::clock();
			
			raycast_screen();
			std::clock_t t2 = std::clock();
			
			gfx_SetTextXY(0,100);
			gfx_PrintString("FPS: ");
			uint8_t fps=(int)((float)32768/(t2-t1)*10);
			gfx_PrintInt(fps/10,1);
			gfx_PrintString(".");
			gfx_PrintInt(fps%10,1);
			dbg_printf("time: %d\n",(t2-t1));
			
			gfx_SetColor(75);
			gfx_HorizLine_NoClip(0, 184, 320);//For some reason, there is a line here so I "patched" it ;)
			if(updatehotbar>0)
			{
				updatehotbar--;
				
				//hotbar
				gfx_FillRectangle_NoClip(0,184,320,56);
				gfx_SetColor(231);
				gfx_FillRectangle_NoClip(hotbarblock*40,200,40,40);

				int c=0;
				int temp;
				for (int i=5; i<320;i+=40)// draws hotbar
				{
					temp=hotbar[c];
					draw_item(temp,i,205,1);
					++c;
				}
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
			}
			gfx_SetColor(0);
			gfx_FillRectangle_NoClip(157,119,6,2);//crosshair
			gfx_FillRectangle_NoClip(159,117,2,6);
			
			//now drawing x,y,z coords and chunk x y coords
			gfx_SetTextBGColor(26);
			uint8_t c=0;
			if(wtime>12)
				c=255;
			gfx_SetTextFGColor(c);
			gfx_SetTextXY(0,0);
			gfx_PrintString("X: ");
			gfx_PrintInt((int)plx,1);
			gfx_PrintString(", Y: ");
			gfx_PrintInt((int)ply,1);
			gfx_PrintString(", Z: ");
			gfx_PrintInt((int)plz,1);

			gfx_SetTextXY(0,10);
			gfx_PrintString("chunkX: ");
			gfx_PrintInt((int)chunkx,1);
			gfx_SetTextXY(0,20);
			gfx_PrintString("chunkZ: ");
			gfx_PrintInt((int)chunkz,1);
			gfx_SetTextBGColor(255);
			gfx_SetTextFGColor(0);
			gfx_SwapDraw();
			
			if(playerhp<1)
			{
				alive=false;
			}
		}
	}
	end:
	
	bool bflag=0;
	for(int i=0; i<8;++i)
	{
		if(wname[i]!=origwname[i])
		{
			bflag=1;
			break;
		}
	}
	save_world(wname);
	dbg_printf("pbflag wname: %s, origwname: %s, bflag: %d\n",wname,origwname,bflag);
	if(bflag)//if wname!=origwname
	{
		dbg_printf("bflag!!!\n");
		tchunkx=chunkx;//tchunkx and z are temp chunkx for saving in the save_world function
		tchunkz=chunkz;
		chunkx=32;
		chunkz=32;
		flint tempply=ply;//loadnewchunk resets ply to 30 or something.
		loadnewchunk();
		ply=tempply;
		save_world(origwname);
		
		
	}
	set_archive(origwname,true);
	archiveallchunks(origwname);
	gfx_End();
	if (world!=NULL)
		free(world);
	//usb_Cleanup();
    return 0;
}
