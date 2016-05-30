/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Block functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include <linux/slab.h>

#include "dma_block.h"

struct dma_block * dma_block_create(unsigned int priv_size,\
	gfp_t gfp)
{
	struct dma_block * block = NULL;
	
	block = kzalloc(sizeof(*block),gfp);
	if(block != NULL) {
		block->priv = kzalloc(priv_size,gfp);
		if(block->priv == NULL) {
			kfree(block);
			block = NULL;
		}
	}
	
	return block;
}

void dma_block_op_bind(struct dma_block * block, \
	struct dma_block_op * op)
{
	block->op = op;
}

void * dma_block_get_buffer(struct dma_block * block)
{
	return block->op->get_buffer(block);
}

size_t dma_block_get_size(struct dma_block * block)
{
	return block->op->get_size(block);
}

void dma_block_free(struct dma_block * block) {
	if(block != NULL) {
		if(block->priv != NULL)
			kfree(block->priv);
			
		kfree(block);
	}
}

static void * simple_dma_block_get_buffer(struct dma_block * block)
{
	struct simple_dma_block * block_priv = block->priv;
	
	return block_priv->buffer;
}

static size_t simple_dma_block_get_size(struct dma_block * block)
{
	struct simple_dma_block * block_priv = block->priv;
	
	return block_priv->size;
}

static struct dma_block_op simple_dma_block_ops = {
	.get_buffer = simple_dma_block_get_buffer,
	.get_size = simple_dma_block_get_size
};

struct dma_block * simple_dma_block_create(void * buffer,\
	size_t size, gfp_t gfp)
{
	struct dma_block * block = NULL;
	struct simple_dma_block * block_priv;
	
	block = dma_block_create(sizeof(*block_priv),gfp);
	if(block != NULL) {
		block_priv = block->priv;
			
		block_priv->buffer = buffer;
		block_priv->size = size;
		block_priv->alloc_buffer = 0;
			
		dma_block_op_bind(block,&simple_dma_block_ops);
	}
	
	return block;
}

struct dma_block * simple_dma_block_alloc_buffer(size_t size,\
		gfp_t gfp)
{
	void * buf;
	struct dma_block * block = NULL;
	struct simple_dma_block * block_priv;

	buf = kzalloc(size,gfp);
	if(buf != NULL) {
		block = simple_dma_block_create(buf,size,gfp);
		if(block != NULL) {
			block_priv = block->priv;
			block_priv->alloc_buffer = 1;
		}
	}

	return block;
}

void simple_dma_block_free(struct dma_block * block)
{
	struct simple_dma_block * block_priv = block->priv;
	void * buf = block_priv->buffer;
	int alloc_buffer = block_priv->alloc_buffer;

	dma_block_free(block);

	if(alloc_buffer)
		kfree(buf);

}
