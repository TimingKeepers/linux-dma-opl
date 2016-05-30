/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Scatter-Gather functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/page.h>

#include "dma_sg.h"

struct dma_sg * dma_sg_offset_create(struct dma_block * block, \
	size_t offset, gfp_t gfp)
{
	struct dma_sg * sg = NULL;
	
	sg = kzalloc(sizeof(*sg),gfp);
	if(sg != NULL) {
		sg->block = block;
		sg->offset = offset;
	}
	
	return sg;
}

struct dma_sg * dma_sg_create(struct dma_block * block, \
	gfp_t gfp)
{
	return dma_sg_offset_create(block,0,gfp);
}

/* This function is inspired by zio_calculate_nents (dma.c) of
 * the ZIO project (http://www.ohwr.org/projects/zio).
 */
int dma_sg_get_pages(struct dma_sg * sg)
{
	void * bufp;
	int bytesleft;
	int mapbytes;
	int nents = 0;
	struct dma_block * blk;
	
	if(sg == NULL)
		return -1;
		
	blk = sg->block;
	
	bufp = dma_block_get_buffer(blk)+sg->offset;
	bytesleft = dma_block_get_size(blk)-sg->offset;
	
	while(bytesleft) {
		nents++;
		
		if(bytesleft < (PAGE_SIZE - offset_in_page(bufp)))
			mapbytes = bytesleft;
		else
			mapbytes = PAGE_SIZE - offset_in_page(bufp);
			
		bufp += mapbytes;
		bytesleft -= mapbytes;
	}
	
	return nents;
}

void dma_sg_free(struct dma_sg * sg)
{
	if(sg != NULL) {
		/* FIXME: Maybe, Free the block as well? */
		kfree(sg);
	}
}
