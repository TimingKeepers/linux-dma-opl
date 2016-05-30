/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Operation functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef DMA_OP_H
#define DMA_OP_H

#include "dma_xfer.h"

/**
 * 
 * DMA Operation structure. It contains several DMA transactions/transfers.
 * 
 */
struct dma_op {
	/* Xfer list */
	struct list_head list_dma_xfer;
};

/**
 * 
 * dma_op_create - Create a new DMA Operation.
 * 
 * @gfp: Specific flags to request memory.
 * 
 * Return: A DMA Operation.
 * 
 */
struct dma_op * dma_op_create(gfp_t gfp);

/**
 * 
 * dma_op_add_xfer - Add a new DMA Xfer to the DMA Operation.
 * 
 * @op : DMA Operation pointer.
 * @xfer: DMA Xfer pointer.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_op_add_xfer(struct dma_op * op, \
	struct dma_xfer * xfer);

/**
 *
 * dma_op_del_xfer - Remove new DMA Xfer from the DMA Operation.
 *
 * @op : DMA Operation pointer.
 * @xfer: DMA Xfer pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int dma_op_del_xfer(struct dma_op * op, \
	struct dma_xfer * xfer);

/**
 *
 * dma_op_add_xfer - Remove all DMA Xfers from the DMA Operation.
 *
 * @op : DMA Operation pointer.
 * @xfer: DMA Xfer pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int dma_op_clear_xfer(struct dma_op * op);
	
/**
 * 
 * dma_op_start - Start all the transfers of the DMA Operation.
 * 
 * Return: 0 if success and an error code otherwise.
 * 
 */
int dma_op_start(struct dma_op * op);

/**
 * dma_op_all_xfers_completed - Check if all Xfers have been
 * completed.
 *
 * Return: 1 if all xfers have completed and 0 otherwise.
 *
 */
int dma_op_all_xfers_completed(struct dma_op * op);

/**
 * dma_op_xfer_error - Check if any error occurs in
 * some xfer.
 *
 * Return: 1 if any xfer has not completed properly and 0 otherwise.
 *
 */
int dma_op_xfer_error(struct dma_op * op);
	
/**
 * 
 * dma_op_free - Destroy a DMA Operation.
 * 
 * @xfer: DMA Operation pointer.
 * 
 */	
void dma_op_free(struct dma_op * op);

#endif /* DMA_OP_H */
