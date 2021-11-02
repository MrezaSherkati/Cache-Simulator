/*
 * cache.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;
static unsigned tag_mask;
static int tag_mask_offset;
static cache c1;
static cache c2;
static cache_line *array1;

static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/


void set_cache_param(param, value)
  int param;
  int value;
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
 }

}
/************************************************************/

/************************************************************/
void init_cache()
{
c1.size=cache_usize;
c1.associativity=cache_assoc;
c1.n_sets=cache_usize/cache_block_size;
c1.n_sets=c1.n_sets/c1.associativity;
c1.set_contents=c1.associativity;
c1.contents=c1.n_sets;
c1.index_mask_offset=log2(cache_block_size);
int temp1=log2(c1.n_sets);
c1.index_mask=0xffffffff;

c1.index_mask=c1.index_mask<<temp1;

c1.index_mask=~c1.index_mask;

c1.index_mask=c1.index_mask<<c1.index_mask_offset;

tag_mask_offset=temp1+c1.index_mask_offset;
tag_mask=0xffffffff;
tag_mask=tag_mask<<tag_mask_offset;


int memnum=c1.n_sets*c1.associativity;
array1=(cache_line *)malloc(sizeof(cache_line)*memnum);
int j;
for(j=0;j<memnum;j++){
	array1[j].used=0;
        array1[j].tag=0;
	array1[j].dirty=0;
	array1[j].lru=0;
}

    

}
/************************************************************/

/************************************************************/
void perform_access(addr, access_type)
  unsigned addr, access_type;
{unsigned index;
index = (addr&c1.index_mask) >> c1.index_mask_offset;
int indexi=index;
int indexii;
cache_line l1,l2,l3;
int array2[c1.associativity];
int i;
int j;

	if(access_type==0){
		cache_stat_data.accesses++;
		unsigned addrtag;
		addrtag=(addr&tag_mask) >> tag_mask_offset;
		int i;
		for(i=0;i<c1.associativity;i++){
			array2[i]=(i*c1.n_sets)+indexi;		
		}
		for(i=0;i<c1.associativity;i++){
			if(array1[i*c1.n_sets+indexi].used==0){
				indexii=i*c1.n_sets+indexi;					
				goto t1;		
			}		
		}
		goto t2;
			t1: 
			array1[indexii].tag=addrtag;
			array1[indexii].dirty=0;
			array1[indexii].used=1;
			array1[indexii].lru>>1;
			array1[indexii].lru=array1[indexii].lru|0x80000000;
			for(i=0;i<c1.associativity;i++){
				if(array2[i]!=indexii)
					array1[array2[i]].lru>>1;			
			}		
			cache_stat_data.misses++;
			goto end0;		
			
		t2: 
			l1.tag=addrtag;			
			for(i=0;i<c1.associativity;i++){
				if(l1.tag==array1[array2[i]].tag){
					cache_stat_data.demand_fetches=cache_stat_data.demand_fetches+4;
					array1[array2[i]].lru>>1;
					array1[array2[i]].lru=array1[array2[i]].lru|0x80000000;
					for(j=0;j<c1.associativity;j++){
					if(array2[j]!=array2[i])
					array1[array2[j]].lru>>1;			
					}goto end0;				
				}			
			}			
			
				cache_stat_data.replacements++;
				int min=array1[array2[0]].lru;
				int tempa=array2[0];
				for(i=0;i<c1.associativity;i++){
					
					if(min>array1[array2[i]].lru){
						min=array1[array2[i]].lru;
						tempa=array2[i];					
					}				
				}
				array1[tempa].tag=l1.tag;
				if(array1[tempa].dirty==1){
					cache_stat_data.copies_back++;
					array1[tempa].dirty=0;				
				}
				array1[tempa].lru>>1;
				array1[tempa].lru=array1[tempa].lru|0x80000000;
				for(j=0;j<c1.associativity;j++){
					if(array2[j]!=tempa)
					array1[array2[j]].lru>>1;			
				}
					
			
	
	}end0:
	if(access_type==1){
		cache_stat_data.accesses++;
		unsigned addrtag;
		addrtag=(addr&tag_mask) >> tag_mask_offset;
		for(i=0;i<c1.associativity;i++){
			array2[i]=i*c1.n_sets+indexi;		
		}
		for(i=0;i<c1.associativity;i++){
			if(array1[i*c1.n_sets+indexi].used==0){
				indexii=i*c1.n_sets+indexi;					
				goto t3;		
			}		
		}goto t4;
		
		t3:	array1[indexi].tag=addrtag;
			array1[indexi].dirty=1;
			array1[indexi].used=1;
			array1[indexii].lru>>1;
			array1[indexii].lru=array1[indexii].lru|0x80000000;
			for(i=0;i<c1.associativity;i++){
				if(array2[i]!=indexii)
					array1[array2[i]].lru>>1;			
			}		

			cache_stat_data.misses++;
			goto end1;
		
		t4:	l1.tag=addrtag;
			for(i=0;i<c1.associativity;i++){
				if(l1.tag==array1[array2[i]].tag){
					array1[indexi].dirty=1;
					array1[array2[i]].lru>>1;
					array1[array2[i]].lru=array1[array2[i]].lru|0x80000000;
					for(j=0;j<c1.associativity;j++){
					if(array2[j]!=array2[i])
					array1[array2[j]].lru>>1;			
					}goto end1;				
				}			
			}
			
				cache_stat_data.replacements++;
				int min=array1[array2[0]].lru;
				int tempa=array2[0];
				for(i=0;i<c1.associativity;i++){
					if(min>array1[array2[i]].lru){
						min=array1[array2[i]].lru;
						tempa=array2[i];					
					}				
				}
				array1[tempa].tag=l1.tag;
				if(array1[tempa].dirty==1)
					cache_stat_data.copies_back++;
				else
					array1[tempa].dirty=1;
		
		

	}end1:
	if(access_type==2){
			cache_stat_inst.accesses++;
		
		unsigned addrtag;
		addrtag=(addr&tag_mask) >> tag_mask_offset;
		int i;
		for(i=0;i<c1.associativity;i++){
			array2[i]=i*c1.n_sets+indexi;		
		}
		for(i=0;i<c1.associativity;i++){
			if(array1[i*c1.n_sets+indexi].used==0){
				indexii=i*c1.n_sets+indexi;					
				goto t5;		
			}		
		}
		goto t6;
			t5: 
			array1[indexii].tag=addrtag;
			array1[indexii].dirty=0;
			array1[indexii].used=1;
			array1[indexii].lru>>1;
			array1[indexii].lru=array1[indexii].lru|0x80000000;
			for(i=0;i<c1.associativity;i++){
				if(array2[i]!=indexii)
					array1[array2[i]].lru>>1;			
			}		
			cache_stat_inst.misses++;
			goto end2;		
			
		t6: 
			l1.tag=addrtag;			
			for(i=0;i<c1.associativity;i++){
				if(l1.tag==array1[array2[i]].tag){
					cache_stat_inst.demand_fetches=cache_stat_inst.demand_fetches+4;
					array1[array2[i]].lru>>1;
					array1[array2[i]].lru=array1[array2[i]].lru|0x80000000;
					for(j=0;j<c1.associativity;j++){
					if(array2[j]!=array2[i])
					array1[array2[j]].lru>>1;			
					}goto end2;				
				}			
			}			
			
				cache_stat_inst.replacements++;
				int min=array1[array2[0]].lru;
				int tempa=array2[0];
				for(i=0;i<c1.associativity;i++){
					if(min>array1[array2[i]].lru){
						min=array1[array2[i]].lru;
						tempa=array2[i];					
					}				
				}
				array1[tempa].tag=l1.tag;
				if(array1[tempa].dirty==1){
					cache_stat_inst.copies_back++;
					array1[tempa].dirty=0;				
				}
				array1[tempa].lru>>1;
				array1[tempa].lru=array1[tempa].lru|0x80000000;
				for(j=0;j<c1.associativity;j++){
					if(array2[j]!=tempa)
					array1[array2[j]].lru>>1;			
				}
				
			

} end2: printf("");
}
/************************************************************/

/************************************************************/
void flush()
{
int i=0;
 for(i=0;i<c1.n_sets;i++){
	if(array1[i].dirty==1)
		cache_stat_data.copies_back++;
}

}
/************************************************************/

/************************************************************/

/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("*** CACHE SETTINGS ***\n");
  if (cache_split) { 
    printf("  Split I- D-cache\n");
    printf("  I-cache size: \t%d\n", cache_isize);
    printf("  D-cache size: \t%d\n", cache_dsize);
  } else {
    printf("  Unified I- D-cache\n");
    printf("  Size: \t%d\n", cache_usize);
  } 
  printf("  Associativity: \t%d\n", cache_assoc);
  printf("  Block size: \t%d\n", cache_block_size);
  printf("  Write policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
//printf("Helooo");
  printf("  Allocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("\n*** CACHE STATISTICS ***\n");

  printf(" INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  if (!cache_stat_inst.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf(" DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  if (!cache_stat_data.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf(" TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/
