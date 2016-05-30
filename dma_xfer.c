/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Transfer functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>

#include <asm/page.h>

#include "dma_xfer.h"

struct dma_xfer * dma_xfer_create(struct dma_chan * dma_chan, \
	struct dma_slave_config * dma_config, struct device * hwdev, \
	gfp_t gfp)
{
	struct dma_xfer * xfer = NULL;
	
	xfer = kzalloc(sizeof(*xfer),gfp);
	if(xfer != NULL) {
		xfer->dma_chan = dma_chan;
		xfer->dma_config = *dma_config;
		xfer->hwdev = hwdev;
		
		INIT_LIST_HEAD(&xfer->list_dma_sg);
	}
	
	return xfer;
}

int dma_xfer_add_sg(struct dma_xfer * xfer, struct dma_sg * sg)
{
	int r = 0;
	
	if(xfer != NULL && sg != NULL)
		list_add_tail(&sg->node,&xfer->list_dma_sg);
	else
		r = -1;
		
	return r;
}

int dma_xfer_del_sg(struct dma_xfer * xfer, struct dma_sg * sg)
{
	int r = 0;

	if(xfer != NULL && sg != NULL)
		list_del(&sg->node);
	else
		r = -1;

	return r;
}

int dma_xfer_clear_sg(struct dma_xfer * xfer)
{
	int r = 0;
	struct list_head * p;
	struct list_head * aux;
	struct dma_sg * dsg;

	list_for_each_safe(p,aux,&xfer->list_dma_sg) {
		dsg = list_entry(p,struct dma_sg,node);

		r = dma_xfer_del_sg(xfer,dsg);
		if(r < 0)
			break;
	}

	return r;
}

static int _dma_xfer_create_sg_table(struct dma_xfer * xfer, gfp_t gfp)
{
	struct list_head * p;
	struct dma_sg * dsg;
	int npages = 0;
	int r;
	
	list_for_each(p,&xfer->list_dma_sg) {
		dsg = list_entry(p,struct dma_sg,node);
		
		r = dma_sg_get_pages(dsg);
		if(r > 0) {
			npages += r;
		}
		else {
			dev_err(xfer->hwdev,"Couldn't get pages for DMA SG \n");
			npages = -1;
			break;
		}
	}
	
	if(npages > 0) {
		r = sg_alloc_table(&xfer->sgt,npages,gfp);
		if(r) {
			dev_err(xfer->hwdev,"Couldn't allocate SG Table \n");
			npages = -2;
		}
	}
	
	return npages;
}

/* This function is inspired by zio_dma_setup_scatter (dma.c) of
 * the ZIO project (http://www.ohwr.org/projects/zio).
 */
static int _dma_xfer_setup_sg_table(struct dma_xfer * xfer)
{
	struct scatterlist * sg;
	struct list_head * p;
	struct dma_sg * dsg;
	int bytesleft = 0;
	void * bufp = NULL;
	int mapbytes;
	int i;
	struct dma_block * blk;
	
	if(xfer == NULL)
		return -1;
		
	sg = xfer->sgt.sgl;
	i = 0;
		
	list_for_each(p,&xfer->list_dma_sg) {
		dsg = list_entry(p,struct dma_sg,node);
		
		blk = dsg->block;
		
		bufp = dma_block_get_buffer(blk)+dsg->offset;
		bytesleft = dma_block_get_size(blk)-dsg->offset;
		
		while(bytesleft && i < (xfer->sgt.nents)) {
			if(bytesleft < (PAGE_SIZE-offset_in_page(bufp)))
				mapbytes = bytesleft;
			else
				mapbytes = PAGE_SIZE-offset_in_page(bufp);
				
			if(is_vmalloc_addr(bufp))
				sg_set_page(sg,vmalloc_to_page(bufp), mapbytes, \
					offset_in_page(bufp));
			else
				sg_set_buf(sg,bufp,mapbytes);
				
			bufp += mapbytes;
			bytesleft -= mapbytes;
			
			sg = sg_next(sg);
			i++;
		}
	}
	
	return 0;
}

static int _dma_xfer_map_sg(struct dma_xfer * xfer)
{
	int nents;
	
	nents = dma_map_sg(xfer->hwdev, xfer->sgt.sgl,\
				xfer->sgt.nents, xfer->dma_map_dir);
	
	return (nents == xfer->sgt.nents) ? 0 : -1;
}

static void _dma_xfer_unmap_sg(struct dma_xfer * xfer)
{
	dma_unmap_sg(xfer->hwdev, xfer->sgt.sgl,\
				xfer->sgt.nents, xfer->dma_map_dir);
}

static int _dma_xfer_init_sg_table(struct dma_xfer * xfer, \
	gfp_t gfp)
{
	int r = 0;
	
	r = _dma_xfer_create_sg_table(xfer,gfp);
	if(r > 0) {
		r = _dma_xfer_setup_sg_table(xfer);
	}
		
	return r;
}

int dma_xfer_map_sg(struct dma_xfer * xfer, \
	enum dma_data_direction dma_map_dir, \
	gfp_t gfp)
{
	int r = 0;
	
	xfer->dma_map_dir = dma_map_dir;
	
	r = _dma_xfer_init_sg_table(xfer,gfp);
	if(r == 0) {
		r = _dma_xfer_map_sg(xfer);
	}
	
	return r;
}

int dma_xfer_tx_map_sg(struct dma_xfer * xfer, \
	gfp_t gfp)
{
	return dma_xfer_map_sg(xfer,DMA_TO_DEVICE,\
		gfp);
}

int dma_xfer_rx_map_sg(struct dma_xfer * xfer, \
	gfp_t gfp)
{
	return dma_xfer_map_sg(xfer,DMA_FROM_DEVICE,\
		gfp);
}

int dma_xfer_prep_start_sg(struct dma_xfer * xfer, \
	enum dma_transfer_direction dma_dir, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags,
	void * context)
{
	int r = 0;
	
	xfer->dma_dir = dma_dir;
	
	dmaengine_slave_config(xfer->dma_chan, &(xfer->dma_config));

	xfer->dma_desc = xfer->dma_chan->device->device_prep_slave_sg(\
			xfer->dma_chan, xfer->sgt.sgl, xfer->sgt.nents, \
			dma_dir,flags,context);

	if(xfer->dma_desc == NULL) {
		r = -1;
	} else {
		xfer->dma_desc->callback = dma_cb_f;
		xfer->dma_desc->callback_param = dma_cb_param;
	}

	return r;
	
}

int dma_xfer_prep_tx_start_sg(struct dma_xfer * xfer, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags,
	void * context)
{
	return dma_xfer_prep_start_sg(xfer,DMA_MEM_TO_DEV,\
		dma_cb_f,dma_cb_param,flags,context);
}

int dma_xfer_prep_rx_start_sg(struct dma_xfer * xfer, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags,
	void * context)
{
	return dma_xfer_prep_start_sg(xfer,DMA_DEV_TO_MEM,\
		dma_cb_f,dma_cb_param,flags,context);
}

void dma_xfer_cyclic_setup(struct dma_xfer * xfer, \
		struct dma_cyclic_info * dcyc_info)
{
	xfer->dcyc_info = *dcyc_info;
}

int dma_xfer_prep_start_cyclic(struct dma_xfer *xfer, \
		enum dma_transfer_direction dma_dir, \
		void (*dma_cb_f)(void * param), \
		void * dma_cb_param, \
		unsigned long flags)
{
	int r = 0;

	xfer->dma_dir = dma_dir;

	dmaengine_slave_config(xfer->dma_chan, &(xfer->dma_config));

	xfer->dma_desc = xfer->dma_chan->device->device_prep_dma_cyclic(\
			xfer->dma_chan, xfer->dcyc_info.dma_addr, \
			xfer->dcyc_info.len, \
			xfer->dcyc_info.period_len,dma_dir,\
			flags);

	if(xfer->dma_desc == NULL) {
		r = -1;
	} else {
		xfer->dma_desc->callback = dma_cb_f;
		xfer->dma_desc->callback_param = dma_cb_param;
	}

	return r;
}

void dma_xfer_memcpy_setup(struct dma_xfer * xfer, \
		struct dma_memcpy_info * dmemcpy_info)
{
	xfer->dmemcpy_info = *dmemcpy_info;
}

int dma_xfer_prep_start_memcpy(struct dma_xfer *xfer, \
		void (*dma_cb_f)(void * param), \
		void * dma_cb_param, \
		unsigned long flags)
{
	int r = 0;

	xfer->dma_dir = DMA_MEM_TO_MEM;

	dmaengine_slave_config(xfer->dma_chan, &(xfer->dma_config));

	xfer->dma_desc = xfer->dma_chan->device->device_prep_dma_memcpy(\
			xfer->dma_chan, xfer->dmemcpy_info.dst, \
			xfer->dmemcpy_info.src, \
			xfer->dmemcpy_info.len,\
			flags);

	if(xfer->dma_desc == NULL) {
		r = -1;
	} else {
		xfer->dma_desc->callback = dma_cb_f;
		xfer->dma_desc->callback_param = dma_cb_param;
	}

	return r;
}

int dma_xfer_start(struct dma_xfer * xfer)
{
	int r = 0;
	
	xfer->dma_cookie = dmaengine_submit(xfer->dma_desc);
	dma_async_issue_pending(xfer->dma_chan);
	
	return r;
}

enum dma_status dma_xfer_status(struct dma_xfer * xfer)
{
	return dma_async_is_tx_complete(xfer->dma_chan,\
			xfer->dma_cookie, NULL, NULL);
}

void dma_xfer_free(struct dma_xfer * xfer)
{
	if(xfer != NULL) {
		_dma_xfer_unmap_sg(xfer);
		sg_free_table(&xfer->sgt);
		kfree(xfer);
	}
}
