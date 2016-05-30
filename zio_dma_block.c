/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * ZIO DMA Block functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include "zio_dma_block.h"

static void * zio_dma_block_get_buffer(struct dma_block * block)
{
	struct zio_dma_block * block_priv = block->priv;
	
	return block_priv->block->data;
}

static size_t zio_dma_block_get_size(struct dma_block * block)
{
	struct zio_dma_block * block_priv = block->priv;
	
	return block_priv->block->datalen;
}

static struct dma_block_op zio_dma_block_ops = {
	.get_buffer = zio_dma_block_get_buffer,
	.get_size = zio_dma_block_get_size
};

struct dma_block * zio_dma_block_create(struct zio_block * block,\
	gfp_t gfp)
{
	struct dma_block * blk;
	struct zio_dma_block * block_priv;
	
	blk = dma_block_create(sizeof(*block_priv),gfp);
	if(blk != NULL) {
		block_priv = blk->priv;
			
		block_priv->block = block;
		
		dma_block_op_bind(blk,&zio_dma_block_ops);
	}
	
	return blk;
}

void zio_dma_block_free(struct dma_block * block)
{
	dma_block_free(block);
}
