/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * Packet descriptor functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef PACKET_DESC_H
#define PACKET_DESC_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>

#include "dma_op.h"

/**
 *
 * Packet descriptor structure. It stores all the
 * information related with a packet transaction.
 *
 */
struct pdesc {

	/* Data */
	struct dma_block * block;
	struct sk_buff * skb;

	/* DMA Stuff */
	struct dma_op * dma_op;

	/* Packet ID */
	u16 id;

	/* Linked list of packet descriptors */
	struct list_head node;
};

/**
 *
 * pdesc_create - Create a new Packet descriptor.
 *
 * @block: Data block pointer.
 * @dma_op: DMA Operation structure pointer.
 * id: Packet ID.
 * @gfp: Specific flags to request memory.
 *
 * Return: A Packet descriptor.
 *
 */
struct pdesc * pdesc_create(struct dma_block * block, \
		struct dma_op * dma_op, u16 id, gfp_t gfp);

/**
 *
 * pdesc_xfer_start - Start the transfer of the packet.
 *
 * @desc: A Packet descriptor pointer.
 *
 * Return: 0 if success and an error code otherwise.
 *
 */
int pdesc_xfer_start(struct pdesc * desc);


/**
 *
 * pdesc_copy_from - Copy the data from the packet
 * descriptor to the sk_buff (networking layer structure).
 *
 * @desc: A Packet descriptor pointer.
 * @skb: A networking layer structure pointer.
 * @ts: Packet timestamping if any.
 *
 */
void pdesc_copy_from(struct pdesc * desc, \
	struct sk_buff * skb, struct timespec * ts);

/**
 *
 * pdesc_copy_from - Copy the data from the sk_buff
 * (networking layer structure) to the packet descriptor.
 *
 * @desc: A Packet descriptor pointer.
 * @skb: A networking layer structure pointer.
 * @ts: Packet timestamping if any.
 *
 */
void pdesc_copy_to(struct pdesc * desc, \
	struct sk_buff * skb, struct timespec * ts);

/**
 *
 * pdesc_free - Destroy a Packet descriptor.
 *
 * @desc: A Packet descriptor pointer.
 *
 */
void pdesc_free(struct pdesc * desc);

/**
 *
 * Packet descriptor pool structure. It contains several
 * packet descriptors in a linked list.
 *
 */
struct pdesc_pool {
	struct list_head list_pdesc;
};

/**
 *
 * pdesc_pool_create - Create a new Packet descriptor pool.
 *
 * @gfp: Specific flags to request memory.
 *
 * Return: A Packet descriptor pool.
 *
 */
struct pdesc_pool * pdesc_pool_create(gfp_t gfp);

/**
 *
 * pdesc_pool_add - Add a new Packet descriptor to the pool.
 *
 * @pool : Packet descriptor pool pointer.
 * @desc: Packet descriptor pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int pdesc_pool_add(struct pdesc_pool * pool, \
		struct pdesc * desc);

/**
 *
 * pdesc_pool_del - Remove a Packet descriptor from the pool.
 *
 * @pool : Packet descriptor pool pointer.
 * @desc: Packet descriptor pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int pdesc_pool_del(struct pdesc_pool * pool, \
		struct pdesc * desc);

/**
 *
 * pdesc_pool_clear - Remove all Packet descriptors from the pool.
 *
 * @pool : Packet descriptor pool pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int pdesc_pool_clear(struct pdesc_pool * pool);

/**
 *
 * pdesc_pool_free - Destroy a Packet descriptor pool.
 *
 * @pool: A Packet descriptor pool pointer.
 *
 */
void pdesc_pool_free(struct pdesc_pool * pool);

#endif /* PACKET_DESC_H_ */
