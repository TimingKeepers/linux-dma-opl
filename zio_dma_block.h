/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * ZIO DMA Block functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef ZIO_DMA_BLOCK_H
#define ZIO_DMA_BLOCK_H

#include <linux/types.h>
#include <linux/bug.h>

#include <linux/zio.h>

#include "dma_block.h"

/**
 * 
 * ZIO block structure.
 * 
 */
struct zio_dma_block {
	/* ZIO block */
	struct zio_block * block;
};

/**
 * 
 * zio_dma_block_create - Create a new ZIO DMA block.
 * 
 * @block: ZIO block.
 * @gfp: Specific flags to request memory.
 * 
 * Return: A initialized DMA block.
 * 
 */
struct dma_block * zio_dma_block_create(struct zio_block * block,\
	gfp_t gfp);
	

/**
 * 
 * zio_dma_block_free - Destroy a ZIO DMA block.
 * 
 * @block: Block pointer.
 * 
 */	
void zio_dma_block_free(struct dma_block * block);

#endif /* ZIO_DMA_BLOCK_H */
