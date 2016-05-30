/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Block functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef DMA_BLOCK_H
#define DMA_BLOCK_H

#include <linux/types.h>

struct dma_block;

/**
 * 
 * DMA Block operations. 
 * 
 * get_buffer - Return a pointer to the data buffer.
 * 		@block: Block pointer.
 * 
 * 		Return: Buffer pointer.
 * 
 * get_size: Get the size of the data block.
 * 		@block: Block pointer.
 * 
 * 		Return: Block size.
 * 
 */
struct dma_block_op {
	void * (*get_buffer)(struct dma_block * block);
	size_t (*get_size)(struct dma_block * block);
};

/**
 * 
 * DMA block structure. It is a generic structure and
 * it is only use to provide the standard methods.
 * 
 */
struct dma_block {
	/* Private block data */
	void * priv;
	
	/* Generic block operations */
	struct dma_block_op * op;
};

/**
 * 
 * dma_block_create - Create a DMA block and request some memory
 * for the private field.
 * 
 * @priv_size: Private field size (in bytes).
 * @gfp: Specific flags to request memory.
 * 
 * Return: A DMA block.
 * 
 */
struct dma_block * dma_block_create(unsigned int priv_size,\
	gfp_t gfp);
	

/**
 * 
 * dma_block_op_bind - Assign handlers for the DMA block methods.
 * 
 * @block: DMA Block.
 * @op: Block function handlers.
 * 
 */	
void dma_block_op_bind(struct dma_block * block, \
	struct dma_block_op * op);

/**
 * dma_block_get_buffer - Get the buffer related with the
 * DMA block.
 *
 * @block: DMA Block.
 *
 * Return: A Buffer.
 */
void * dma_block_get_buffer(struct dma_block * block);

/**
 * dma_block_get_size - Get the size of the
 * DMA block.
 *
 * @block: DMA Block.
 *
 * Return: The size of the DMA Block.
 */
size_t dma_block_get_size(struct dma_block * block);
	
/**
 * 
 * dma_block_free - Destroy a DMA block.
 * 
 * @block: DMA Block.
 * 
 */	
void dma_block_free(struct dma_block * block);

/**
 * 
 * Simple DMA block structure. It is the simpliest way to
 * define a block for a DMA transfer.
 * 
 */
struct simple_dma_block {
	/* Data buffer */
	void * buffer; 
	
	/* Data buffer size */
	size_t size; 

	/* Reserve memory for buffer? */
	int alloc_buffer;
};

/**
 * 
 * simple_dma_block_create - Create a new simple DMA block.
 * 
 * @buffer: Data buffer.
 * @size: Block size.
 * @gfp: Specific flags to request memory.
 * 
 * Return: A initialized DMA block.
 * 
 */
struct dma_block * simple_dma_block_create(void * buffer,\
	size_t size, gfp_t gfp);

/**
 *
 * simple_dma_block_alloc_buffer - Create a new simple DMA block and
 * allocate memory for the buffer.
 *
 * @size: Block size.
 * @gfp: Specific flags to request memory.
 *
 * Return: A initialized DMA block.
 *
 */
struct dma_block * simple_dma_block_alloc_buffer(size_t size,\
		gfp_t gfp);

/**
 * 
 * simple_dma_block_free - Destroy a simple DMA block.
 * 
 * @block: Block pointer.
 * 
 */	
void simple_dma_block_free(struct dma_block * block);

#endif /* DMA_BLOCK_H */
