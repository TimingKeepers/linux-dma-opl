/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Scatter-Gather functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef DMA_SG_H
#define DMA_SG_H

#include <linux/types.h>
#include <linux/list.h>

#include "dma_block.h"

/**
 * 
 * DMA SG structure. This structure is used to generate
 * the scatter-gather table for a DMA block.
 * 
 */
struct dma_sg {
	/* Generic DMA block */
	struct dma_block * block;
	
	/* 
	 * Offset for the DMA block
	 * 
	 * It allows to define two dma_sg structure
	 * for the same block but with different
	 * offsets.
	 * 
	 */
	size_t offset;
	
	/* For the linked list */
	struct list_head node;
};

/**
 * 
 * dma_sg_offset_create - Create a DMA SG for a DMA block.
 * 
 * @block: DMA block pointer.
 * @offset: offset for the DMA block.
 * @gfp: Specific flags to request memory.
 * 
 * Return: A DMA SG.
 * 
 */
struct dma_sg * dma_sg_offset_create(struct dma_block * block, \
	size_t offset, gfp_t gfp);

/**
 * 
 * dma_sg_create - Create a DMA SG with zero offset for a DMA block.
 * 
 * @block: DMA block pointer.
 * @gfp: Specific flags to request memory.
 * 
 * Return: A DMA SG.
 * 
 */	
struct dma_sg * dma_sg_create(struct dma_block * block, \
	gfp_t gfp);

/**
 * 
 * dma_sg_get_pages - Get the number of pages needed for the DMA SG.
 * 
 * @sg: DMA SG pointer.
 * 
 * Return: The number of pages needed.
 * 
 */
int dma_sg_get_pages(struct dma_sg * sg);
	
/**
 * 
 * dma_sg_free - Destroy a DMA SG.
 * 
 * @sg: DMA SG pointer.
 * 
 */	
void dma_sg_free(struct dma_sg * sg);

#endif /* DMA_SG_H */
