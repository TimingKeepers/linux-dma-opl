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

struct pdesc * pdesc_create(struct dma_block * block, \
		struct dma_op * dma_op, u16 id, gfp_t gfp)
{
	struct pdesc * desc = NULL;

	desc = kzalloc(sizeof(*desc),gfp);
	if(desc != NULL) {
		desc->block = block;
		desc->dma_op = dma_op;
		desc->id = id;
	}

	return desc;
}

int pdesc_xfer_start(struct pdesc * desc)
{
	return dma_op_start(desc->dma_op);
}

static void pdesc_tstamp_set(struct pdesc * desc, \
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
	if(desc != NULL)
		kfree(desc);
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

void pdesc_pool_free(struct pdesc_pool * pool)
{
	if(pool != NULL)
		kfree(pool);
}
