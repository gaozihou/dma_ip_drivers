/*
 * Copyright(c) 2019 Xilinx, Inc. All rights reserved.
 */

#ifndef QDMA_MBOX_PROTOCOL_H_
#define QDMA_MBOX_PROTOCOL_H_

/**
 * DOC: QDMA message box handling interface definitions
 *
 * Header file *qdma_mbox_protocol.h* defines data structures and function
 * signatures exported for QDMA Mbox message handling.
 */

#include "qdma_platform_env.h"
#include "qdma_access.h"

/** mailbox register max */
#define MBOX_MSG_REG_MAX		32

#define QDMA_MBOX_VF_ONLINE                  (19)
#define QDMA_MBOX_VF_OFFLINE                   (20)

#define mbox_invalidate_msg(m)	{ (m)->hdr.op = MBOX_OP_NOOP; }

/**
 * struct mbox_descq_conf - collective bit-fields of all contexts
 */
struct mbox_descq_conf {

	/** @ring_bs_addr: ring base address */
	uint64_t ring_bs_addr;
	/** @cmpt_ring_bs_addr: completion ring base address */
	uint64_t cmpt_ring_bs_addr;
	/** @forced_en: enable fetch credit */
	uint32_t forced_en:1;
	/** @en_bypass: bypass enable */
	uint32_t en_bypass:1;
	/** @irq_arm: arm irq */
	uint32_t irq_arm:1;
	/** @wbi_intvl_en: writeback interval enable */
	uint32_t wbi_intvl_en:1;
	/** @wbi_chk: writeback pending check */
	uint32_t wbi_chk:1;
	/** @at: address translation */
	uint32_t at:1;
	/** @wbk_en: writeback enable */
	uint32_t wbk_en:1;
	/** @irq_en: irq enable */
	uint32_t irq_en:1;
	/** @pfch_en: prefetch enable */
	uint32_t pfch_en:1;
	/** @en_bypass_prefetch: prefetch bypass enable */
	uint32_t en_bypass_prefetch:1;
	/** @dis_overflow_check: disable overflow check */
	uint32_t dis_overflow_check:1;
	/** @cmpt_int_en: completion interrupt enable */
	uint32_t cmpt_int_en:1;
	/** @cmpt_at: completion address translation */
	uint32_t cmpt_at:1;
	/** @cmpt_color: completion ring initial color bit */
	uint32_t cmpt_color:1;
	/** @cmpt_full_upd: completion full update */
	uint32_t cmpt_full_upd:1;
	/** @cmpl_stat_en: completion status enable */
	uint32_t cmpl_stat_en:1;
	/** @desc_sz: descriptor size */
	uint32_t desc_sz:2;
	/** @cmpt_desc_sz: completion ring descriptor size */
	uint32_t cmpt_desc_sz:2;
	/** @triggermode: trigger mode */
	uint32_t triggermode:3;
	/** @rsvd: reserved */
	uint32_t rsvd:9;
	/** @func_id: function ID */
	uint32_t func_id:16;
	/** @cnt_thres: counter threshold */
	uint32_t cnt_thres:8;
	/** @timer_thres: timer threshold */
	uint32_t timer_thres:8;
	/** @intr_id: interrupt id */
	uint16_t intr_id:11;
	/** @intr_aggr: interrupt aggregation */
	uint16_t intr_aggr:1;
	/** @filler: filler bits */
	uint16_t filler:4;
	/** @ringsz: ring size */
	uint16_t ringsz;
	/** @bufsz: c2h buffer size */
	uint16_t bufsz;
	/** @cmpt_ringsz: completion ring size */
	uint16_t cmpt_ringsz;
};

/**
 * @enum - mbox_cmpt_ctxt_type
 * @brief  specifies whether cmpt is enabled with MM/ST
 */
enum mbox_cmpt_ctxt_type {
	/** @QDMA_MBOX_CMPT_CTXT_ONLY: only cmpt context programming required */
	QDMA_MBOX_CMPT_CTXT_ONLY,
	/** @QDMA_MBOX_CMPT_WITH_MM: completion context with MM */
	QDMA_MBOX_CMPT_WITH_MM,
	/** @QDMA_MBOX_CMPT_WITH_ST: complete context with ST */
	QDMA_MBOX_CMPT_WITH_ST,
	/** @QDMA_MBOX_CMPT_CTXT_NONE: No completion context */
	QDMA_MBOX_CMPT_CTXT_NONE
};

/**
 * @struct - mbox_msg_intr_ctxt
 * @brief	interrupt context mailbox message
 */
struct mbox_msg_intr_ctxt {
	/** @num_rings: number of intr context rings be assigned
	 * for virtual function
	 */
	uint8_t num_rings;	/* 1 ~ 8 */
	/** @ring_index_list: ring index associated for each vector */
	uint32_t ring_index_list[QDMA_NUM_DATA_VEC_FOR_INTR_CXT];
	/** @w: interrupt context data for all rings*/
	struct qdma_indirect_intr_ctxt ictxt[QDMA_NUM_DATA_VEC_FOR_INTR_CXT];
};

/*****************************************************************************/
/**
 * qdma_mbox_hw_init(): Initialize the mobx HW
 *
 * @dev_hndl:  device handle
 * @is_vf:  is VF mbox
 *
 * Return:	None
 *****************************************************************************/
void qdma_mbox_hw_init(void *dev_hndl, uint8_t is_vf);

/*****************************************************************************/
/**
 * qdma_mbox_pf_rcv_msg_handler(): handles the raw message received in pf
 *
 * @pci_bus_num:  pci bus number
 * @dev_hndl:  device handle
 * @func_id:   own function id
 * @rcv_msg:   received raw message
 * @resp_msg:  raw response message
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_pf_rcv_msg_handler(void *dev_hndl, uint8_t pci_bus_num,
				 uint16_t func_id, uint32_t *rcv_msg,
				 uint32_t *resp_msg);

/*****************************************************************************/
/**
 * qmda_mbox_compose_vf_online(): compose VF online message
 *
 * @func_id:   destination function id
 * @qmax: number of queues being requested
 * @qbase: q base at which queues are allocated
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qmda_mbox_compose_vf_online(uint16_t func_id,
				uint16_t qmax, int *qbase, uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_offline(): compose VF offline message
 *
 * @func_id:   destination function id
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_offline(uint16_t func_id,
				 uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qreq(): compose message to request queues
 *
 * @func_id:   destination function id
 * @qmax: number of queues being requested
 * @qbase: q base at which queues are allocated
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_qreq(uint16_t func_id,
			      uint16_t qmax, int qbase, uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_notify_qadd(): compose message to notify queue add
 *
 * @func_id:   destination function id
 * @qid_hw: number of queues being requested
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_notify_qadd(uint16_t func_id,
				     uint16_t qid_hw, uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_notify_qdel(): compose message to notify queue delete
 *
 * @func_id:   destination function id
 * @qid_hw: number of queues being requested
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_notify_qdel(uint16_t func_id,
				     uint16_t qid_hw, uint32_t *raw_data);
/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_fmap_prog(): handles the raw message received
 *
 * @func_id:   destination function id
 * @qmax: number of queues being requested
 * @qbase: q base at which queues are allocated
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_fmap_prog(uint16_t func_id,
				   uint16_t qmax, int qbase,
				   uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qctxt_write(): compose queue configuration data for
 * compose and program
 *
 * @func_id:   destination function id
 * @qid_hw:   HW queue for which the context has to be read
 * @st:   is st mode
 * @c2h:   is c2h direction
 * @cmpt_ctxt_type:   completion context type
 * @descq_conf:   pointer to queue config data structure
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_qctxt_write(uint16_t func_id,
			uint16_t qid_hw, uint8_t st, uint8_t c2h,
			enum mbox_cmpt_ctxt_type cmpt_ctxt_type,
			struct mbox_descq_conf *descq_conf,
			uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qctxt_read(): compose message to read context data of a
 * queue
 *
 * @func_id:   destination function id
 * @qid_hw:   HW queue for which the context has to be read
 * @st:   is st mode
 * @c2h:   is c2h direction
 * @cmpt_ctxt_type:   completion context type
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_qctxt_read(uint16_t func_id,
			uint16_t qid_hw, uint8_t st, uint8_t c2h,
			enum mbox_cmpt_ctxt_type cmpt_ctxt_type,
			uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qctxt_invalidate(): compose queue context invalidate
 * message
 *
 * @func_id:   destination function id
 * @qid_hw:   HW queue for which the context has to be invalidated
 * @st:   is st mode
 * @c2h:   is c2h direction
 * @cmpt_ctxt_type:   completion context type
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_qctxt_invalidate(uint16_t func_id,
			uint16_t qid_hw, uint8_t st, uint8_t c2h,
			enum mbox_cmpt_ctxt_type cmpt_ctxt_type,
			uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qctxt_clear(): compose queue context clear message
 *
 * @func_id:   destination function id
 * @qid_hw:   HW queue for which the context has to be cleared
 * @st:   is st mode
 * @c2h:   is c2h direction
 * @cmpt_ctxt_type:   completion context type
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_qctxt_clear(uint16_t func_id,
			uint16_t qid_hw, uint8_t st, uint8_t c2h,
			enum mbox_cmpt_ctxt_type cmpt_ctxt_type,
			uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_csr_read(): compose message to read csr info
 *
 * @func_id:   destination function id
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_csr_read(uint16_t func_id,
			       uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_intr_ctxt_write(): compose interrupt ring context
 * programming message
 *
 * @func_id:   destination function id
 * @intr_ctxt:   pointer to interrupt context data structure
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_intr_ctxt_write(uint16_t func_id,
					 struct mbox_msg_intr_ctxt *intr_ctxt,
					 uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_intr_ctxt_read(): handles the raw message received
 *
 * @func_id:   destination function id
 * @intr_ctxt:   pointer to interrupt context data structure
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_intr_ctxt_read(uint16_t func_id,
					struct mbox_msg_intr_ctxt *intr_ctxt,
					uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_intr_ctxt_clear(): compose interrupt ring context
 * clear message
 *
 * @func_id:   destination function id
 * @intr_ctxt:   pointer to interrupt context data structure
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_intr_ctxt_clear(uint16_t func_id,
					 struct mbox_msg_intr_ctxt *intr_ctxt,
					 uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_compose_vf_qctxt_invalidate(): compose interrupt ring context
 * invalidate message
 *
 * @func_id:   destination function id
 * @intr_ctxt:   pointer to interrupt context data structure
 * @raw_data: output raw message to be sent
 *
 * Return:	0  : success and < 0: failure
 *****************************************************************************/
int qdma_mbox_compose_vf_intr_ctxt_invalidate(uint16_t func_id,
				      struct mbox_msg_intr_ctxt *intr_ctxt,
				      uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_is_msg_response(): check if the received msg opcode is response
 *                              sent message opcode
 *
 * @send_data: mbox message sent
 * @rcv_data: mbox message recieved
 *
 * Return:	1  : match and  0: does not match
 *****************************************************************************/
uint8_t qdma_mbox_is_msg_response(uint32_t *send_data, uint32_t *rcv_data);

/*****************************************************************************/
/**
 * qdma_mbox_vf_response_status(): return the response received for the sent msg
 *
 * @rcv_data: mbox message recieved
 *
 * Return:	response status received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_response_status(uint32_t *rcv_data);

/*****************************************************************************/
/**
 * qdma_mbox_vf_func_id_get(): return the vf function id
 *
 * @rcv_data: mbox message recieved
 *
 * Return:	vf function id
 *****************************************************************************/
uint8_t qdma_mbox_vf_func_id_get(uint32_t *rcv_data);

/*****************************************************************************/
/**
 * qdma_mbox_vf_parent_func_id_get(): return the vf parent function id
 *
 * @rcv_data: mbox message recieved
 *
 * Return:	vf function id
 *****************************************************************************/
uint8_t qdma_mbox_vf_parent_func_id_get(uint32_t *rcv_data);

/*****************************************************************************/
/**
 * qdma_mbox_vf_dev_info_get(): get dev info from received message
 *
 * @rcv_data: mbox message recieved
 * @dev_cap: device capability information
 *
 * Return:	response status with dev info received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_dev_info_get(uint32_t *rcv_data,
				struct qdma_dev_attributes *dev_cap);

/*****************************************************************************/
/**
 * qdma_mbox_vf_qinfo_get(): get qinfo from received message
 *
 * @rcv_data: mbox message recieved
 * @qmax: number of queues
 * @qbase: q base at which queues are allocated
 *
 * Return:	response status received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_qinfo_get(uint32_t *rcv_data, int *qbase, uint16_t *qmax);

/*****************************************************************************/
/**
 * qdma_mbox_vf_csr_get(): get csr info from received message
 *
 * @rcv_data: mbox message recieved
 * @csr: pointer to the csr info
 *
 * Return:	response status received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_csr_get(uint32_t *rcv_data, struct qdma_csr_info *csr);

/*****************************************************************************/
/**
 * qdma_mbox_vf_context_get(): get queue context info from received message
 *
 * @rcv_data: mbox message recieved
 * @ctxt: pointer to the queue context info
 *
 * Return:	response status received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_context_get(uint32_t *rcv_data,
			     struct qdma_descq_context *ctxt);

/*****************************************************************************/
/**
 * qdma_mbox_vf_context_get(): get intr context info from received message
 *
 * @rcv_data: mbox message recieved
 * @ctxt: pointer to the intr context info
 *
 * Return:	response status received to the sent message
 *****************************************************************************/
int qdma_mbox_vf_intr_context_get(uint32_t *rcv_data,
				  struct mbox_msg_intr_ctxt *ictxt);


/*****************************************************************************/
/**
 * qdma_mbox_pf_hw_clear_ack() - clear the HW ack
 *
 * @dev_hndl:   device handle
 *
 * Return:	0   - success and < 0 - failure
 *****************************************************************************/
void qdma_mbox_pf_hw_clear_ack(void *dev_hndl);

/*****************************************************************************/
/**
 * qdma_mbox_send() - function to send raw data via qdma mailbox
 *
 * @dev_hndl:   device handle
 * @is_vf:	     Whether PF or VF
 * @raw_data:   pointer to message being sent
 *
 * The function sends the raw_data to the outgoing mailbox memory and if PF,
 * then assert the acknowledge status register bit.
 *
 * Return:	0   - success and < 0 - failure
 *****************************************************************************/
int qdma_mbox_send(void *dev_hndl, uint8_t is_vf, uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_rcv() - function to receive raw data via qdma mailbox
 *
 * @dev_hndl: device handle
 * @is_vf: Whether PF or VF
 * @raw_data:  pointer to the message being received
 *
 * The function receives the raw_data from the incoming mailbox memory and
 * then acknowledge the sender by setting msg_rcv field in the command
 * register.
 *
 * Return:	0   - success and < 0 - failure
 *****************************************************************************/
int qdma_mbox_rcv(void *dev_hndl, uint8_t is_vf, uint32_t *raw_data);

/*****************************************************************************/
/**
 * qdma_mbox_enable_interrupts() - Enable the QDMA mailbox interrupt
 *
 * @dev_hndl: pointer to xlnx_dma_dev
 * @is_vf: Whether PF or VF
 *
 * @return	none
 *****************************************************************************/
void qdma_mbox_enable_interrupts(void *dev_hndl, uint8_t is_vf);

/*****************************************************************************/
/**
 * qdma_mbox_disable_interrupts() - Disable the QDMA mailbox interrupt
 *
 * @dev_hndl: pointer to xlnx_dma_dev
 * @is_vf: Whether PF or VF
 *
 * @return	none
 *****************************************************************************/
void qdma_mbox_disable_interrupts(void *dev_hndl, uint8_t is_vf);

#endif /* QDMA_MBOX_PROTOCOL_H_ */
