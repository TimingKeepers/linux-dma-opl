/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Operation functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include <linux/slab.h>

#include "dma_op.h"

struct dma_op * dma_op_create(gfp_t gfp)
{
	struct dma_op * op = NULL;
	
	op = kzalloc(sizeof(*op),gfp);
	if(op != NULL)
		INIT_LIST_HEAD(&op->list_dma_xfer);
	
	return op;
}

int dma_op_add_xfer(struct dma_op * op, \
	struct dma_xfer * xfer)
{
	int r = 0;
	
	if(op != NULL && xfer != NULL)
		list_add_tail(&xfer->node,&op->list_dma_xfer);
	else
		r = -1;
		
	return r;
}

int dma_op_del_xfer(struct dma_op * op, \
	struct dma_xfer * xfer)
{
	int r = 0;

	if(op != NULL && xfer != NULL)
		list_del(&xfer->node);
	else
		r = -1;

	return r;
}

int dma_op_clear_xfer(struct dma_op * op)
{
	int r = 0;
	struct list_head * p;
	struct list_head * aux;
	struct dma_xfer * xfer;

	list_for_each_safe(p,aux,&op->list_dma_xfer) {
		xfer = list_entry(p,struct dma_xfer,node);

		r = dma_op_del_xfer(op,xfer);
		if(r < 0)
			break;
	}

	return r;
}

int dma_op_start(struct dma_op * op)
{
	struct list_head *p;
	struct dma_xfer *xfer;
	int r = 0;
	
	list_for_each(p,&op->list_dma_xfer) {
		xfer = list_entry(p,struct dma_xfer,node);
		
		r = dma_xfer_start(xfer);
		if(r != 0)
			break;
	}
	
	return r;
}

int dma_op_all_xfers_completed(struct dma_op * op)
{
	struct list_head *p;
	struct dma_xfer *xfer;
	int r = 1;

	list_for_each(p,&op->list_dma_xfer) {
		xfer = list_entry(p,struct dma_xfer,node);

		if(dma_xfer_status(xfer) != DMA_COMPLETE) {
			r = 0;
			break;
		}
	}

	return r;
}

int dma_op_xfer_error(struct dma_op * op)
{
	struct list_head *p;
	struct dma_xfer *xfer;
	int r = 0;

	list_for_each(p,&op->list_dma_xfer) {
		xfer = list_entry(p,struct dma_xfer,node);

		if(dma_xfer_status(xfer) == DMA_ERROR) {
			r = 1;
			break;
		}
	}

	return r;
}

void dma_op_free(struct dma_op * op) 
{
	if(op != NULL)
		kfree(op);
}
