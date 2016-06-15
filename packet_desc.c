/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * Packet descriptor functions (implementation).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#include "packet_desc.h"

/* Functions for packet_desc structure */

struct pdesc * pdesc_create(struct dma_chan *dma_chan, \
	struct dma_block * block, enum pdesc_type pdesc_t, \
	u16 id, struct device *dev, gfp_t gfp)
{
	struct pdesc * desc = NULL;
	struct dma_xfer *xfer = NULL;
	struct dma_sg *sg = NULL;
	struct dma_slave_config dma_config;

	desc = kzalloc(sizeof(*desc),gfp);
	if(desc != NULL) {
		desc->block = block;
		desc->id = id;
		desc->pdesc_t = pdesc_t;
		
		sg = dma_sg_create(block,gfp);
		
		dma_config.direction \
			= ((pdesc_t == PDESC_TX) ? DMA_MEM_TO_DEV : DMA_DEV_TO_MEM);
		dma_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		dma_config.src_maxburst = 1;
	
		xfer = dma_xfer_create(dma_chan,&dma_config,dev,gfp);
		dma_xfer_add_sg(xfer,sg);
		desc->dma_op = dma_op_create(gfp);
		dma_op_add_xfer(desc->dma_op,xfer);
	}

	return desc;
}

struct pdesc * pdesc_tx_create(struct dma_chan *dma_chan, \
	struct dma_block * block, u16 id, struct device *dev, \
	gfp_t gfp)
{
	return pdesc_create(dma_chan,block,PDESC_TX,id,dev,gfp);
}
	
struct pdesc * pdesc_rx_create(struct dma_chan *dma_chan, \
	struct dma_block * block, u16 id, struct device *dev, \
	gfp_t gfp)
{
	return pdesc_create(dma_chan,block,PDESC_RX,id,dev,gfp);
}

int pdesc_xfer_prep(struct pdesc *desc, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags, \
	void * context, \
	gfp_t gfp)
{
	struct dma_xfer *xfer = \
		list_first_entry(&desc->dma_op->list_dma_xfer,\
		struct dma_xfer,node);
	int r = 0;
	
	/** Map the DMA SG of the DMA transfer **/
	r = dma_xfer_map_sg(xfer, \
		(desc->pdesc_t == PDESC_TX) ? DMA_TO_DEVICE : DMA_FROM_DEVICE,\
		gfp);
	if(r != 0)
		return r;
	
	/** Prepare the DMA SG transfer **/
	r = dma_xfer_prep_start_sg(xfer, \
		(desc->pdesc_t == PDESC_TX) ? DMA_MEM_TO_DEV : DMA_DEV_TO_MEM,\
		dma_cb_f, dma_cb_param, flags, context);
		
	return r;
}

int pdesc_xfer_start(struct pdesc * desc)
{
	return dma_op_start(desc->dma_op);
}

void pdesc_tstamp_set(struct pdesc * desc, \
	struct timespec ts)
{
	struct skb_shared_hwtstamps *hwts;

	hwts = skb_hwtstamps(desc->skb);
	hwts->hwtstamp = timespec_to_ktime(ts);
}

void pdesc_copy_from(struct pdesc * desc, \
	struct sk_buff * skb, struct timespec * ts)
{
	size_t size = desc->block->op->get_size(desc->block);
	unsigned char * pbuf = desc->block->op->get_buffer(desc->block);
	unsigned char * pskb = skb_put(skb,size);

	desc->skb = skb;
	memcpy(pskb,pbuf,size);

	if(ts != NULL)
		pdesc_tstamp_set(desc,*ts);
}

void pdesc_copy_to(struct pdesc * desc, \
	struct sk_buff * skb, struct timespec * ts)
{
	unsigned char * pbuf = desc->block->op->get_buffer(desc->block);
	unsigned char * pskb = skb->data;

	desc->skb = skb;
	memcpy(pbuf,pskb,skb->len);

	if(ts != NULL)
		pdesc_tstamp_set(desc,*ts);
}

void pdesc_free(struct pdesc * desc)
{
	if(desc != NULL) {
		struct dma_xfer *xfer = \
			list_first_entry(&desc->dma_op->list_dma_xfer,\
			struct dma_xfer,node);
		struct dma_sg *sg = \
			list_first_entry(&xfer->list_dma_sg,\
			struct dma_sg,node);
			
		dma_op_clear_xfer(desc->dma_op);	
		dma_op_free(desc->dma_op);
		dma_xfer_clear_sg(xfer);
		dma_xfer_free(xfer);
		dma_sg_free(sg);
		
		kfree(desc);
	}
}

/* Functions for packet_desc_pool structure */

struct pdesc_pool * pdesc_pool_create(gfp_t gfp)
{
	struct pdesc_pool * pool = NULL;

	pool = kzalloc(sizeof(*pool),gfp);
	if(pool != NULL)
		INIT_LIST_HEAD(&pool->list_pdesc);

	return pool;
}

int pdesc_pool_add(struct pdesc_pool * pool, \
		struct pdesc * desc)
{
	int r = 0;

	if(pool != NULL && desc != NULL)
		list_add_tail(&desc->node,&pool->list_pdesc);
	else
		r = -1;

	return r;
}


int pdesc_pool_del(struct pdesc_pool * pool, \
		struct pdesc * desc)
{
	int r = 0;

	if(pool != NULL && desc != NULL)
		list_del(&desc->node);
	else
		r = -1;

	return r;
}

int pdesc_pool_clear(struct pdesc_pool * pool)
{
	int r = 0;
	struct list_head * p;
	struct list_head * aux;
	struct pdesc * pdesc;

	list_for_each_safe(p,aux,&pool->list_pdesc) {
		pdesc = list_entry(p, struct pdesc, node);

		r = pdesc_pool_del(pool,pdesc);
		if(r < 0)
			break;
	}

	return r;
}

struct pdesc * pdesc_pool_find(struct pdesc_pool * pool, \
	u16 frame_id)
{
	struct list_head *p;
	struct pdesc *pd;
	int found = 0;
	
	list_for_each(p,&pool->list_pdesc) {
		pd = list_entry(p, struct pdesc, node);
		
		if(pd->id == frame_id) {
			found = 1;
			break;
		}
	}
	
	if(!found)
		pd = NULL;
		
	return pd;
}

void pdesc_pool_free(struct pdesc_pool * pool)
{
	if(pool != NULL)
		kfree(pool);
}
