/*
 * Copyright (C) 2016 University of Granada
 * 		Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * DMA Transfer functions (header).
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
*/

#ifndef DMA_XFER_H
#define DMA_XFER_H

#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/dmaengine.h>
#include <linux/dma-direction.h>
#include <linux/device.h>
#include <linux/list.h>

#include "dma_sg.h"

/**
 *
 * DMA Cyclic mode info.
 *
 */
struct dma_cyclic_info {
	dma_addr_t dma_addr;
	size_t len;
	size_t period_len;
};

/**
 *
 * DMA memcpy mode info.
 *
 */
struct dma_memcpy_info {
	dma_addr_t dst;
	dma_addr_t src;
	size_t len;
};

/**
 * 
 * DMA Transfer (xfer) structure. It represents a DMA transaction.
 * 
 */
struct dma_xfer {
	/* SG structures */
	struct sg_table sgt;
	struct list_head list_dma_sg;
	enum dma_data_direction dma_map_dir;
	
	/* memcpy and cyclic stuff */
	struct dma_cyclic_info dcyc_info;
	struct dma_memcpy_info dmemcpy_info;

	/* DMAengine stuff */
	struct dma_chan * dma_chan;
	struct dma_slave_config dma_config;
	enum dma_transfer_direction dma_dir;
	struct dma_async_tx_descriptor * dma_desc;
	dma_cookie_t dma_cookie;
	
	/* Reference to internal Linux dev */
	struct device * hwdev;
	
	/* dma_xfer linked list */
	struct list_head node;
};

/**
 * 
 * dma_sg_create - Create a new DMA Xfer.
 * 
 * @dma_chan: DMA channel.
 * @dma_config: DMA configuration settings.
 * @hwdev: reference to internal Linux device.
 * @gfp: Specific flags to request memory.
 * 
 * Return: A DMA Xfer.
 * 
 */
struct dma_xfer * dma_xfer_create(struct dma_chan * dma_chan, \
	struct dma_slave_config * dma_config, struct device * hwdev, \
	gfp_t gfp);
	
/**
 * 
 * dma_xfer_add_sg - Add a new DMA SG to the DMA Xfer.
 * 
 * @xfer: DMA Xfer pointer.
 * @sg: DMA SG pointer.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_xfer_add_sg(struct dma_xfer * xfer, struct dma_sg * sg);

/**
 *
 * dma_xfer_del_sg - Remove a DMA SG from the DMA Xfer.
 *
 * @xfer: DMA Xfer pointer.
 * @sg: DMA SG pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int dma_xfer_del_sg(struct dma_xfer * xfer, struct dma_sg * sg);

/**
 *
 * dma_xfer_clear_sg - Remove all DMA SGs from the DMA Xfer.
 *
 * @xfer: DMA Xfer pointer.
 *
 * Return: 0 if sucess and an error code otherwise.
 *
 */
int dma_xfer_clear_sg(struct dma_xfer * xfer);

/**
 * 
 * dma_xfer_map_sg - Map all the DMA SG structures related with
 * the DMA Xfer. This function also creates and initializes the
 * Scatter-Gather Table for the DMA transaction.
 * 
 * @xfer: DMA Xfer pointer.
 * @dma_map_dir: DMA direction (@see <linux/dma-direction.h>)
 * 		DMA_BIDIRECTIONAL
 *      DMA_TO_DEVICE
 *      DMA_FROM_DEVICE
 *      DMA_NONE
 * 
 * @gfp: Specific flags to request memory.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_xfer_map_sg(struct dma_xfer * xfer, \
	enum dma_data_direction dma_map_dir, \
	gfp_t gfp);

/**
 * 
 * dma_xfer_tx_map_sg - Map all the DMA SG structures related with
 * the TX DMA Xfer. This function also creates and initializes the
 * Scatter-Gather Table for the DMA transaction.
 * 
 * @xfer: DMA Xfer pointer.
 * @gfp: Specific flags to request memory.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */	
int dma_xfer_tx_map_sg(struct dma_xfer * xfer, \
	gfp_t gfp);

/**
 * 
 * dma_xfer_rx_map_sg - Map all the DMA SG structures related with
 * the RX DMA Xfer. This function also creates and initializes the
 * Scatter-Gather Table for the DMA transaction.
 * 
 * @xfer: DMA Xfer pointer.
 * @gfp: Specific flags to request memory.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_xfer_rx_map_sg(struct dma_xfer * xfer, \
	gfp_t gfp);

/**
 * 
 * dma_xfer_prep_start_sg - Prepare everything before the start
 * of a DMA SG transfer.
 * 
 * @xfer: DMA Xfer pointer.
 * @dma_dir: DMA direction (@see <linux/dmaengine.h>).
 *		DMA_MEM_TO_MEM
 *		DMA_MEM_TO_DEV
 *		DMA_DEV_TO_MEM
 *		DMA_DEV_TO_DEV
 *		DMA_TRANS_NONE
 * 
 * @dma_cb_f: DMA Callback function.
 * @dma_cb_param: DMA Callback parameter.
 * @flags: DMA controller flags.
 * @context: Extra arguments for some DMA drivers.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_xfer_prep_start_sg(struct dma_xfer * xfer, \
	enum dma_transfer_direction dma_dir, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags, \
	void * context);

/**
 * 
 * dma_xfer_prep_tx_start_sg - Prepare everything before the start
 * of a TX DMA SG transfer.
 * 
 * @xfer: DMA Xfer pointer.
 * @dma_cb_f: DMA Callback function.
 * @dma_cb_param: DMA Callback parameter.
 * @flags: DMA controller flags.
 * @context: Extra arguments for some DMA drivers.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */	
int dma_xfer_prep_tx_start_sg(struct dma_xfer * xfer, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags, \
	void * context);

/**
 * 
 * dma_xfer_prep_rx_start_sg - Prepare everything before the start
 * of a RX DMA SG transfer.
 * 
 * @xfer: DMA Xfer pointer.
 * @dma_cb_f: DMA Callback function.
 * @dma_cb_param: DMA Callback parameter.
 * @flags: DMA controller flags.
 * @context: Extra arguments for some DMA drivers.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */	
int dma_xfer_prep_rx_start_sg(struct dma_xfer * xfer, \
	void (*dma_cb_f)(void * param), \
	void * dma_cb_param, \
	unsigned long flags, \
	void * context);

/**
 *
 * dma_xfer_cyclic_setup - Setup the info related with the
 * DMA cyclic mode.
 *
 * @xfer: DMA Xfer pointer.
 * @dcyc_info: DMA cyclic mode info.
 *
 */
void dma_xfer_cyclic_setup(struct dma_xfer * xfer, \
		struct dma_cyclic_info * dcyc_info);

/**
 *
 * dma_xfer_prep_start_cyclic - Prepare everything before the
 * start of a DMA Cyclic transfer.
 *
 * @xfer: DMA Xfer pointer.
 * @dma_dir: DMA direction (@see <linux/dmaengine.h>).
 *		DMA_MEM_TO_MEM
 *		DMA_MEM_TO_DEV
 *		DMA_DEV_TO_MEM
 *		DMA_DEV_TO_DEV
 *		DMA_TRANS_NONE
 *
 * @dma_cb_f: DMA Callback function.
 * @dma_cb_param: DMA Callback parameter.
 * @flags: DMA controller flags.
 *
 */
int dma_xfer_prep_start_cyclic(struct dma_xfer *xfer, \
		enum dma_transfer_direction dma_dir, \
		void (*dma_cb_f)(void * param), \
		void * dma_cb_param, \
		unsigned long flags);

/**
 *
 * dma_xfer_memcpy_setup - Setup the info related with the
 * DMA memcpy mode.
 *
 * @xfer: DMA Xfer pointer.
 * @dmemcpy_info: DMA memcpy mode info.
 *
 */
void dma_xfer_memcpy_setup(struct dma_xfer * xfer, \
		struct dma_memcpy_info * dmemcpy_info);

/**
 *
 * dma_xfer_prep_start_memcpy - Prepare everything before the
 * start of a DMA memcpy transfer.
 *
 * @xfer: DMA Xfer pointer.
 * @dma_cb_f: DMA Callback function.
 * @dma_cb_param: DMA Callback parameter.
 * @flags: DMA controller flags.
 *
 */
int dma_xfer_prep_start_memcpy(struct dma_xfer *xfer, \
		void (*dma_cb_f)(void * param), \
		void * dma_cb_param, \
		unsigned long flags);

/**
 * 
 * dma_xfer_start - Start a DMA transfer.
 * 
 * @xfer: DMA Xfer pointer.
 * 
 * Return: 0 if sucess and an error code otherwise.
 * 
 */
int dma_xfer_start(struct dma_xfer * xfer);

/**
 *
 * dma_xfer_status - Get the DMA Xfer status.
 *
 * @xfer: DMA Xfer pointer.
 *
 * Return: DMA Xfer status.
 *
 */
enum dma_status dma_xfer_status(struct dma_xfer * xfer);

/**
 * 
 * dma_xfer_free - Destroy a DMA Xfer.
 * 
 * @xfer: DMA Xfer pointer.
 * 
 */	
void dma_xfer_free(struct dma_xfer * xfer);

#endif /* DMA_XFER_H */
