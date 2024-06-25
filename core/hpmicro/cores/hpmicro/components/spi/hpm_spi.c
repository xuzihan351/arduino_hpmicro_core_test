/*
 * Copyright (c) 2021 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hpm_spi.h"
#include "hpm_clock_drv.h"
#include <stdlib.h>

#if USE_DMA_MGR
#include "hpm_dma_mgr.h"
#endif
typedef struct {
    SPI_Type *spi_ptr;
    clock_name_t spi_clock_name;
#if USE_DMA_MGR
    uint8_t tx_dmamux_src;
    uint8_t rx_dmamux_src;
    dma_resource_t txdma_resource;
    dma_resource_t rxdma_resource;
    spi_dma_complete_cb tx_dma_complete;
    spi_dma_complete_cb rx_dma_complete;
#endif
} hpm_spi_cfg_t;

static hpm_spi_cfg_t spi_dma_cfg_table[] = {
#if defined(HPM_SPI0)
    {
        .spi_ptr = HPM_SPI0,
        .spi_clock_name = clock_spi0,
#if USE_DMA_MGR
        .tx_dmamux_src = HPM_DMA_SRC_SPI0_TX,
        .rx_dmamux_src = HPM_DMA_SRC_SPI0_RX,
        .rx_dma_complete = NULL,
        .tx_dma_complete = NULL
#endif
    },
#endif

#if defined(HPM_SPI1)
    {
        .spi_ptr = HPM_SPI1,
        .spi_clock_name = clock_spi1,
#if USE_DMA_MGR
        .tx_dmamux_src = HPM_DMA_SRC_SPI1_TX,
        .rx_dmamux_src = HPM_DMA_SRC_SPI1_RX,
        .rx_dma_complete = NULL,
        .tx_dma_complete = NULL,
#endif
    },
#endif

#if defined(HPM_SPI2)
    {
        .spi_ptr = HPM_SPI2,
        .spi_clock_name = clock_spi2,
#if USE_DMA_MGR
        .tx_dmamux_src = HPM_DMA_SRC_SPI2_TX,
        .rx_dmamux_src = HPM_DMA_SRC_SPI2_RX,
        .rx_dma_complete = NULL,
        .tx_dma_complete = NULL,
#endif
    },
#endif

#if defined(HPM_SPI3)
    {
        .spi_ptr = HPM_SPI3,
        .spi_clock_name = clock_spi3,
#if USE_DMA_MGR
        .tx_dmamux_src = HPM_DMA_SRC_SPI3_TX,
        .rx_dmamux_src = HPM_DMA_SRC_SPI3_RX,
        .rx_dma_complete = NULL,
        .tx_dma_complete = NULL,
#endif
    },
#endif
};


static hpm_stat_t hpm_spi_tx_trigger_dma(DMA_Type *dma_ptr, uint8_t ch_num, SPI_Type *spi_ptr, uint32_t src, uint8_t data_width, uint32_t size)
{
    dma_handshake_config_t config;

    dma_default_handshake_config(dma_ptr, &config);
    config.ch_index = ch_num;
    config.dst = (uint32_t)&spi_ptr->DATA;
    config.dst_fixed = true;
    config.src = src;
    config.src_fixed = false;
    config.data_width = data_width;
    config.size_in_byte = size;

    return dma_setup_handshake(dma_ptr, &config, true);
}

static hpm_stat_t hpm_spi_rx_trigger_dma(DMA_Type *dma_ptr, uint8_t ch_num, SPI_Type *spi_ptr, uint32_t dst, uint8_t data_width, uint32_t size)
{
    dma_handshake_config_t config;

    dma_default_handshake_config(dma_ptr, &config);
    config.ch_index = ch_num;
    config.dst = dst;
    config.dst_fixed = false;
    config.src = (uint32_t)&spi_ptr->DATA;
    config.src_fixed = true;
    config.data_width = data_width;
    config.size_in_byte = size;

    return dma_setup_handshake(dma_ptr, &config, true);
}


void hpm_spi_prepare_dma_tx_descriptors(spi_context_t *context, spi_control_config_t *config, uint32_t trans_count,
                                    uint32_t *spi_transctrl, dma_linked_descriptor_t *tx_dma_descriptors)
{
    SPI_Type *ptr = context->ptr;
    uint32_t dma_transfer_size[trans_count];
    uint32_t tx_count = context->tx_count;
    uint32_t per_trans_size = context->per_trans_max;
    uint32_t dma_ch = context->dma_context.tx_dma_ch;
    uint8_t *tx_buff = context->tx_buff;
    dma_channel_config_t dma_ch_config;

    static uint8_t dummy_cmd = 0xff;

    uint32_t temp32;
    uint32_t tx_buff_index = 0;

    dma_default_channel_config(context->dma_context.dma_ptr, &dma_ch_config);
    for (uint32_t i = 0; i < trans_count; i++) {
        if (tx_count > per_trans_size) {
            temp32 = per_trans_size;
            tx_count -= per_trans_size;
        } else {
            temp32 = tx_count;
        }

        *(spi_transctrl + i) = SPI_TRANSCTRL_TRANSMODE_SET(config->common_config.trans_mode == spi_trans_write_read_together ?
                                                           spi_trans_write_read_together : spi_trans_write_only)
                                | SPI_TRANSCTRL_DUALQUAD_SET(config->common_config.data_phase_fmt)
                                | SPI_TRANSCTRL_WRTRANCNT_SET(temp32 - 1)
                                | SPI_TRANSCTRL_RDTRANCNT_SET(temp32 - 1);

        if (i == 0) {
            /* Set the count of data transferred by dma to be one more than that of spi */
            /* when dma transfer finished, there are data in SPI fifo, dma should not execute the dma descriptor which changes SPI CTRL register */
            temp32 = temp32 + 1;
        }
        if (i == trans_count - 1) {
            temp32 = temp32 - 1;
        }
        dma_transfer_size[i] = temp32;

        /* SPI CTRL */
        dma_ch_config.size_in_byte = 4;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(spi_transctrl + i));
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->TRANSCTRL);
        dma_ch_config.src_width = DMA_TRANSFER_WIDTH_WORD;
        dma_ch_config.dst_width = DMA_TRANSFER_WIDTH_WORD;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(tx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 1));
        dma_config_linked_descriptor(context->dma_context.dma_ptr, tx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS, dma_ch, &dma_ch_config);

        /* SPI CMD */
        dma_ch_config.size_in_byte = 1;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&dummy_cmd);
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->CMD);
        dma_ch_config.src_width = DMA_TRANSFER_WIDTH_BYTE;
        dma_ch_config.dst_width = DMA_TRANSFER_WIDTH_BYTE;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(tx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 2));
        dma_config_linked_descriptor(context->dma_context.dma_ptr, tx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 1, dma_ch, &dma_ch_config);

        /* SPI DATA */
        dma_ch_config.size_in_byte = dma_transfer_size[i] << context->dma_context.data_width;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(tx_buff + tx_buff_index));
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->DATA);
        dma_ch_config.src_width = context->dma_context.data_width;
        dma_ch_config.dst_width = context->dma_context.data_width;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        if (i == trans_count - 1) {
            dma_ch_config.linked_ptr = 0;
        } else {
            dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(tx_dma_descriptors + (i + 1) * SPI_DMA_DESC_COUNT_PER_TRANS));
        }
        dma_config_linked_descriptor(context->dma_context.dma_ptr, tx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 2, dma_ch, &dma_ch_config);

        tx_buff_index += temp32 * context->data_len_in_byte;
    }
}

void hpm_prepare_dma_rx_descriptors(spi_context_t *context, spi_control_config_t *config, uint32_t trans_count,
                                    uint32_t *spi_transctrl, dma_linked_descriptor_t *rx_dma_descriptors)
{
    SPI_Type *ptr = context->ptr;
    uint32_t dma_transfer_size[trans_count];
    uint32_t rx_count = context->rx_count;
    uint32_t per_trans_size = context->per_trans_max;
    uint32_t dma_ch = context->dma_context.rx_dma_ch;
    uint8_t *rx_buff = context->rx_buff;
    dma_channel_config_t dma_ch_config;

    static uint8_t dummy_cmd = 0xff;

    uint32_t temp32;
    uint32_t rx_buff_index = 0;

    dma_default_channel_config(context->dma_context.dma_ptr, &dma_ch_config);
    for (uint32_t i = 0; i < trans_count; i++) {
        if (rx_count > per_trans_size) {
            temp32 = per_trans_size;
            rx_count -= per_trans_size;
        } else {
            temp32 = rx_count;
        }

        *(spi_transctrl + i) = SPI_TRANSCTRL_TRANSMODE_SET(spi_trans_read_only) |
                                SPI_TRANSCTRL_DUALQUAD_SET(config->common_config.data_phase_fmt) |
                                SPI_TRANSCTRL_WRTRANCNT_SET(temp32 - 1) |
                                SPI_TRANSCTRL_RDTRANCNT_SET(temp32 - 1);
        dma_transfer_size[i] = temp32;

        /* SPI CTRL */
        dma_ch_config.size_in_byte = 4;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(spi_transctrl + i));
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->TRANSCTRL);
        dma_ch_config.src_width = DMA_TRANSFER_WIDTH_WORD;
        dma_ch_config.dst_width = DMA_TRANSFER_WIDTH_WORD;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(rx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 1));
        dma_config_linked_descriptor(context->dma_context.dma_ptr, rx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS, dma_ch, &dma_ch_config);

        /* SPI CMD */
        dma_ch_config.size_in_byte = 1;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&dummy_cmd);
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->CMD);
        dma_ch_config.src_width = DMA_TRANSFER_WIDTH_BYTE;
        dma_ch_config.dst_width = DMA_TRANSFER_WIDTH_BYTE;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(rx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 2));
        dma_config_linked_descriptor(context->dma_context.dma_ptr, rx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 1, dma_ch, &dma_ch_config);

        /* SPI DATA */
        dma_ch_config.size_in_byte = dma_transfer_size[i] << context->dma_context.data_width;
        dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&ptr->DATA);
        dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(rx_buff + rx_buff_index));
        dma_ch_config.src_width = context->dma_context.data_width;
        dma_ch_config.dst_width = context->dma_context.data_width;
        dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
        dma_ch_config.src_mode = DMA_HANDSHAKE_MODE_HANDSHAKE;
        dma_ch_config.dst_mode = DMA_HANDSHAKE_MODE_NORMAL;
        dma_ch_config.src_addr_ctrl = DMA_ADDRESS_CONTROL_FIXED;
        dma_ch_config.dst_addr_ctrl = DMA_ADDRESS_CONTROL_INCREMENT;
        if (i == trans_count - 1) {
            dma_ch_config.linked_ptr = 0;
        } else {
            dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(rx_dma_descriptors + (i + 1) * SPI_DMA_DESC_COUNT_PER_TRANS));
        }
        dma_config_linked_descriptor(context->dma_context.dma_ptr, rx_dma_descriptors + i * SPI_DMA_DESC_COUNT_PER_TRANS + 2, dma_ch, &dma_ch_config);

        rx_buff_index += temp32 * context->data_len_in_byte;
    }
}

static uint32_t hpm_spi_get_trans_count(spi_context_t *context, spi_control_config_t *config)
{
    uint32_t total_trans_count, per_trans_count, trans_count;

    per_trans_count = context->per_trans_max;
    if (config->common_config.trans_mode == spi_trans_write_only || config->common_config.trans_mode == spi_trans_dummy_write) {
        total_trans_count = context->tx_count;
    } else if (config->common_config.trans_mode == spi_trans_read_only || config->common_config.trans_mode == spi_trans_dummy_read) {
        total_trans_count = context->rx_count;
    } else {
        /* write read together */
        assert(context->tx_count == context->rx_count);
        total_trans_count = context->tx_count;
    }
    trans_count = (total_trans_count + per_trans_count - 1) / per_trans_count;

    return trans_count;
}

/**
 * spi with dma chain workflow
 *
 * 1. call spi_setup_dma_transfer to config SPI for first transmission
 * 2. execute data transmission phase in dma chain descriptor
 * 3. execute setting SPI CTRL register phase in dma chain descriptor
 * 4. execute writing SPI CMD register phase in dma chain descriptor
 * 5. Repeat steps 2-4 until finish the transmission
 */
static hpm_stat_t spi_setup_trans_with_dma_chain(spi_context_t *context, spi_control_config_t *config)
{
    hpm_stat_t stat = status_success;
    SPI_Type *spi_ptr = context->ptr;
    DMA_Type *dma_ptr = context->dma_context.dma_ptr;
    DMAMUX_Type *dmamux_ptr = context->dma_context.dmamux_ptr;
    dma_linked_descriptor_t *dma_linked_descriptor = context->dma_linked_descriptor;
    uint32_t *spi_transctrl = context->spi_transctrl;
    uint32_t dma_channel = 0;
    uint32_t trans_count;
    dma_channel_config_t dma_ch_config = {0};

    /* use a dummy dma transfer to start SPI trans dma chain */
    static uint32_t dummy_data1 = 0xff, dummy_data2 = 0xff;

    trans_count = hpm_spi_get_trans_count(context, config);

    /* active spi cs pin */
    context->write_cs(context->cs_pin, SPI_CS_ACTIVE);

    /* config SPI for first dma transmission */
    stat = spi_setup_dma_transfer(spi_ptr,
                                config,
                                &context->cmd,
                                &context->addr,
                                MIN(context->tx_count, context->per_trans_max),
                                MIN(context->rx_count, context->per_trans_max));
    if (stat != status_success) {
        return stat;
    }

    if (config->common_config.trans_mode == spi_trans_write_only || config->common_config.trans_mode == spi_trans_dummy_write) {
        /* write only */
        hpm_spi_prepare_dma_tx_descriptors(context, config, trans_count, spi_transctrl, dma_linked_descriptor);
        dma_channel = context->dma_context.tx_dma_ch;
        dmamux_config(dmamux_ptr, context->dma_context.tx_dmamux_ch, context->dma_context.tx_req, true);
    } else if (config->common_config.trans_mode == spi_trans_read_only || config->common_config.trans_mode == spi_trans_dummy_read) {
        /* read only */
        hpm_prepare_dma_rx_descriptors(context, config, trans_count, spi_transctrl, dma_linked_descriptor);
        dma_channel = context->dma_context.rx_dma_ch;
        dmamux_config(dmamux_ptr, context->dma_context.rx_dmamux_ch, context->dma_context.rx_req, true);
    } else if (config->common_config.trans_mode == spi_trans_write_read_together) {
        /* write and read together */
        hpm_spi_prepare_dma_tx_descriptors(context, config, trans_count, spi_transctrl, dma_linked_descriptor);
        dma_channel = context->dma_context.tx_dma_ch;
        dmamux_config(dmamux_ptr, context->dma_context.tx_dmamux_ch, context->dma_context.tx_req, true);
        dmamux_config(dmamux_ptr, context->dma_context.rx_dmamux_ch, context->dma_context.rx_req, true);
        /* spi tx use chained dma descriptor, spi rx use unchained dma */
        stat = hpm_spi_rx_trigger_dma(dma_ptr,
                            context->dma_context.rx_dma_ch,
                            spi_ptr,
                            core_local_mem_to_sys_address(context->running_core, (uint32_t)context->rx_buff),
                            context->dma_context.data_width,
                            context->rx_size);
        if (stat != status_success) {
            return stat;
        }
    } else {
        return status_invalid_argument;
    }

    dma_default_channel_config(context->dma_context.dma_ptr, &dma_ch_config);
    dma_ch_config.src_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&dummy_data1);
    dma_ch_config.dst_addr = core_local_mem_to_sys_address(context->running_core, (uint32_t)&dummy_data2);
    dma_ch_config.src_burst_size = DMA_NUM_TRANSFER_PER_BURST_1T;
    dma_ch_config.src_width = DMA_TRANSFER_WIDTH_WORD;
    dma_ch_config.dst_width = DMA_TRANSFER_WIDTH_WORD;
    dma_ch_config.size_in_byte = 4;
    /* start data transmission phase in dma chain */
    dma_ch_config.linked_ptr = core_local_mem_to_sys_address(context->running_core, (uint32_t)(dma_linked_descriptor + SPI_DMA_DESC_COUNT_PER_TRANS - 1));

    stat = dma_setup_channel(dma_ptr, dma_channel, &dma_ch_config, true);
    if (stat != status_success) {
        return stat;
    }

    return stat;
}

static hpm_stat_t spi_setup_trans_with_dma(spi_context_t *context, spi_control_config_t *config)
{
    hpm_stat_t stat = status_success;
    SPI_Type *spi_ptr = context->ptr;
    DMA_Type *dma_ptr = context->dma_context.dma_ptr;
    DMAMUX_Type *dmamux_ptr = context->dma_context.dmamux_ptr;
    uint32_t trans_mode = config->common_config.trans_mode;

    if (context->write_cs != NULL) {
        context->write_cs(context->cs_pin, SPI_CS_ACTIVE);
    }
    stat = spi_setup_dma_transfer(spi_ptr, config,
                                &context->cmd, &context->addr,
                                context->tx_count, context->rx_count);
    if (stat != status_success) {
        return stat;
    }

    if (trans_mode != spi_trans_write_only && trans_mode != spi_trans_dummy_write && trans_mode != spi_trans_no_data) {
        dmamux_config(dmamux_ptr, context->dma_context.rx_dmamux_ch, context->dma_context.rx_req, true);
        stat = hpm_spi_rx_trigger_dma(dma_ptr,
                                context->dma_context.rx_dma_ch,
                                spi_ptr,
                                core_local_mem_to_sys_address(context->running_core, (uint32_t)context->rx_buff),
                                context->dma_context.data_width,
                                context->rx_size);
        if (stat != status_success) {
            return stat;
        }
    }
    if (trans_mode != spi_trans_read_only && trans_mode != spi_trans_dummy_read && trans_mode != spi_trans_no_data) {
        dmamux_config(dmamux_ptr, context->dma_context.tx_dmamux_ch, context->dma_context.tx_req, true);
        stat = hpm_spi_tx_trigger_dma(dma_ptr,
                                context->dma_context.tx_dma_ch,
                                spi_ptr,
                                core_local_mem_to_sys_address(context->running_core, (uint32_t)context->tx_buff),
                                context->dma_context.data_width,
                                context->tx_size);
        if (stat != status_success) {
            return stat;
        }
    }

    return stat;
}

hpm_stat_t hpm_spi_setup_dma_transfer(spi_context_t *context, spi_control_config_t *config)
{
    assert(context != NULL || config != NULL);
    /* use dma */
    assert(&context->dma_context != NULL);
    /* spi per trans data size not zero */
    assert(context->per_trans_max);

    hpm_stat_t stat = status_success;

    if (l1c_dc_is_enabled()) {
        /* cache writeback for tx buff */
        if (context->tx_buff != NULL && context->tx_size != 0) {
            uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t)context->tx_buff);
            uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t)context->tx_buff + context->tx_size);
            uint32_t aligned_size = aligned_end - aligned_start;
            l1c_dc_writeback(aligned_start, aligned_size);
        }
        /* cache invalidate for receive buff */
        if (context->rx_buff != NULL && context->rx_size != 0) {
            uint32_t aligned_start = HPM_L1C_CACHELINE_ALIGN_DOWN((uint32_t)context->rx_buff);
            uint32_t aligned_end = HPM_L1C_CACHELINE_ALIGN_UP((uint32_t)context->rx_buff + context->rx_size);
            uint32_t aligned_size = aligned_end - aligned_start;
            l1c_dc_invalidate(aligned_start, aligned_size);
        }
    }

    if ((context->rx_count > context->per_trans_max) || (context->tx_count > context->per_trans_max)) {
        /* multiple SPI transmissions with chained DMA */
        assert(config->common_config.trans_mode == spi_trans_read_only || config->common_config.trans_mode == spi_trans_dummy_read
                || config->common_config.trans_mode == spi_trans_write_only || config->common_config.trans_mode == spi_trans_dummy_write
                || config->common_config.trans_mode == spi_trans_write_read_together);
        /* master mode */
        assert((context->ptr->TRANSFMT & SPI_TRANSFMT_SLVMODE_MASK) != SPI_TRANSFMT_SLVMODE_MASK);
        /* GPIO should be used to replace SPI CS pin for SPI chained DMA transmissions */
        assert(context->write_cs != NULL);

        stat = spi_setup_trans_with_dma_chain(context, config);
    } else {
        /* one SPI transmissions with chained DMA */
        stat = spi_setup_trans_with_dma(context, config);
    }

    return stat;
}

/* Using GPIO as SPI CS pin */
/* When SPI trans completed, GPIO cs pin should be released manually */
hpm_stat_t hpm_spi_release_gpio_cs(spi_context_t *context)
{
    hpm_stat_t stat;
    SPI_Type *ptr = context->ptr;
    assert(context->write_cs != NULL);

    stat = spi_wait_for_idle_status(ptr);
    if (stat != status_success) {
        return stat;
    }

    context->write_cs(context->cs_pin, !SPI_CS_ACTIVE);
    return status_success;
}


static hpm_stat_t wait_spi_slave_active(SPI_Type *ptr, bool active_status, uint32_t timeout)
{
    uint32_t ticks_per_us = (hpm_core_clock + 1000000 - 1U) / 1000000;
    uint64_t expected_ticks = hpm_csr_get_core_cycle() + (uint64_t)ticks_per_us * 1000UL * timeout;
    do {
        if (hpm_csr_get_core_cycle() > expected_ticks) {
            return status_timeout;
        }
    } while (spi_is_active(ptr) == !active_status);
    return status_success;
}

static hpm_spi_cfg_t *hpm_spi_get_cfg_obj(SPI_Type *ptr)
{
    hpm_spi_cfg_t *obj;
    uint8_t i = 0;
    for (i = 0; i < (sizeof(spi_dma_cfg_table) / sizeof(hpm_spi_cfg_t)); i++) {
        obj = &spi_dma_cfg_table[i];
        if (obj->spi_ptr == ptr) {
            return obj;
        }
    }
    return NULL;
}

static void hpm_spi_transfer_init(SPI_Type *ptr, spi_trans_mode_t mode, uint32_t size)
{
    uint32_t slv_mode = SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT);
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if (slv_mode == spi_master_mode) {
        spi_set_transfer_mode(ptr, mode);
    } else {
        /* for slave mode, only support trans_write_read_together mode in only_data_mode */
        spi_set_transfer_mode(ptr, spi_trans_write_read_together);
    }
    if ((mode == spi_trans_write_read_together) || (slv_mode == spi_slave_mode)) {
        spi_set_write_data_count(ptr, size >> (data_len_in_bytes - 1));
        spi_set_read_data_count(ptr, size >> (data_len_in_bytes - 1));
    } else if (mode == spi_trans_write_only) {
        spi_set_write_data_count(ptr, size >> (data_len_in_bytes - 1));
    } else {
        spi_set_read_data_count(ptr, size >> (data_len_in_bytes - 1));
    }
    /* start new transmission, reset spi for slave*/
    if (slv_mode == spi_slave_mode) {
        spi_reset(ptr);
    }
    spi_transmit_fifo_reset(ptr);
    spi_receive_fifo_reset(ptr);
}

void hpm_spi_get_default_init_config(spi_initialize_config_t *config)
{
    config->mode = spi_master_mode;
    config->io_mode = spi_single_io_mode;
    config->clk_polarity = spi_sclk_low_idle;
    config->clk_phase = spi_sclk_sampling_odd_clk_edges;
    config->data_len = 8;
    config->data_merge = false;
    config->direction = msb_first;
}

hpm_stat_t hpm_spi_initialize(SPI_Type *ptr, spi_initialize_config_t *config)
{
    if (config->data_len == 0) {
        return status_invalid_argument;
    }
    /* frist init TRANSFMT reg*/
    ptr->TRANSFMT = SPI_TRANSFMT_DATALEN_SET(config->data_len - 1) |
                    SPI_TRANSFMT_DATAMERGE_SET(config->data_merge) |
                    SPI_TRANSFMT_LSB_SET(config->direction) |
                    SPI_TRANSFMT_SLVMODE_SET(config->mode) |
                    SPI_TRANSFMT_CPOL_SET(config->clk_polarity) |
                    SPI_TRANSFMT_CPHA_SET(config->clk_phase);
    /* second init TRANSCTRL reg
     * default phase: disable command/address/token phase,
     * default Transfer mode: write and read at the same time
     */
    ptr->TRANSCTRL = SPI_TRANSCTRL_SLVDATAONLY_SET(1) | /* it's only data anyway for slave*/
                     SPI_TRANSCTRL_DUALQUAD_SET(config->io_mode);

    /* third: init TIMING reg */
    ptr->TIMING = SPI_TIMING_CS2SCLK_SET(spi_cs2sclk_half_sclk_1) |
                  SPI_TIMING_CSHT_SET(spi_csht_half_sclk_1) |
                  SPI_TIMING_SCLK_DIV_SET(0x10);
    return status_success;
}

hpm_stat_t hpm_spi_set_sclk_frequency(SPI_Type *ptr, uint32_t freq)
{
    int freq_list[clock_source_general_source_end] = {0};
    int _freq = freq;
    uint8_t i = 0;
    int min_diff_freq;
    int current_diff_freq;
    int best_freq;
    hpm_stat_t stat;
    uint32_t div;
    clock_source_t clock_source;
    clk_src_t clk_src;
    static spi_timing_config_t timing_config = {0};
    uint32_t pll_clk = 0;
    hpm_spi_cfg_t *obj = hpm_spi_get_cfg_obj(ptr);
    if (obj == NULL) {
        return status_invalid_argument;
    }
    spi_master_get_default_timing_config(&timing_config);
    timing_config.master_config.clk_src_freq_in_hz = clock_get_frequency(obj->spi_clock_name);
    timing_config.master_config.sclk_freq_in_hz = freq;
    timing_config.master_config.cs2sclk = spi_cs2sclk_half_sclk_1;
    timing_config.master_config.csht = spi_csht_half_sclk_1;
    stat = spi_master_timing_init(ptr, &timing_config);
    if (stat != status_success) {
        spi_master_set_sclk_div(ptr, 0xFF);
        for (clock_source = (clock_source_t)0; clock_source < clock_source_general_source_end; clock_source++) {
            pll_clk = get_frequency_for_source(clock_source);
            div = pll_clk / freq;
            /* The division factor ranges from 1 to 256 as any integer */
            if ((div > 0) && (div <= 0x100)) {
                freq_list[clock_source] = pll_clk / div;
            }
        }
        /* Find the best sclk frequency */
        min_diff_freq = abs(freq_list[0] - _freq);
        best_freq = freq_list[0];
        for (i = 1; i < clock_source_general_source_end; i++) {
            current_diff_freq = abs(freq_list[i] - _freq);
            if (current_diff_freq < min_diff_freq) {
                min_diff_freq = current_diff_freq;
                best_freq = freq_list[i];
            }
        }
        /* Find the best spi clock frequency */
        for (i = 0; i < clock_source_general_source_end; i++) {
            if (best_freq == freq_list[i]) {
                pll_clk = get_frequency_for_source((clock_source_t)i);
                clk_src = MAKE_CLK_SRC(CLK_SRC_GROUP_COMMON, i);
                div = pll_clk / best_freq;
                clock_set_source_divider(obj->spi_clock_name, clk_src, div);
                break;
            }
        }
        stat = status_success;
    }
    return stat;
}

hpm_stat_t hpm_spi_transmit_receive_blocking(SPI_Type *ptr, uint8_t *wbuff, uint8_t *rbuff, uint32_t size, uint32_t timeout)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((wbuff == NULL) || (rbuff == NULL) || (size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes))) ||
        (spi_master_get_data_phase_format(ptr) != spi_single_io_mode)) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_write_read_together, size);
    /* for master mode, This CMD register must be written with a dummy value
     *  to start a SPI transfer even when the command phase is not enabled
     */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    } else {
        /* waiting....in slave mode, becomes 1 after the SPI CS signal is asserted */
        stat = wait_spi_slave_active(ptr, true, timeout);
    }
    if (stat == status_success) {
        stat = spi_write_read_data(ptr, data_len_in_bytes, wbuff, size, rbuff, size);
    }
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        /* waiting....in master mode, becomes 0 after the transfer is finished */
        stat = wait_spi_slave_active(ptr, false, timeout);
    }
    return stat;
}

hpm_stat_t hpm_spi_receive_blocking(SPI_Type *ptr, uint8_t *buff, uint32_t size, uint32_t timeout)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((buff == NULL) || (size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_read_only, size);
    /* for master mode, This CMD register must be written with a dummy value
     * to start a SPI transfer even when the command phase is not enabled
     */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    } else {
        /* waiting....in slave mode, becomes 1 after the SPI CS signal is asserted */
        stat = wait_spi_slave_active(ptr, true, timeout);
    }
    /* no need to write TXFIFO because no effect SPI bus for slave mode */
    if (stat == status_success) {
        stat = spi_read_data(ptr, data_len_in_bytes, buff, size);
    }
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        /* waiting....in master mode, becomes 0 after the transfer is finished */
        stat = wait_spi_slave_active(ptr, false, timeout);
    }
    return stat;
}

hpm_stat_t hpm_spi_transmit_blocking(SPI_Type *ptr, uint8_t *buff, uint32_t size, uint32_t timeout)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((buff == NULL) || (size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_write_only, size);
    /* for master mode, This CMD register must be written with a dummy value
     * to start a SPI transfer even when the command phase is not enabled
     */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    } else {
        /* waiting....in slave mode, becomes 1 after the SPI CS signal is asserted */
        stat = wait_spi_slave_active(ptr, true, timeout);
    }
    /* no need to read RXFIFO because no effect SPI bus for slave mode */
    if (stat == status_success) {
        stat = spi_write_data(ptr, data_len_in_bytes, buff, size);
    }
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        /* waiting....in master mode, becomes 0 after the transfer is finished */
        stat = wait_spi_slave_active(ptr, false, timeout);
    }
    return stat;
}

hpm_stat_t hpm_spi_transmit_receive_setup_dma(SPI_Type *ptr, uint32_t size)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_write_read_together, size);
    spi_enable_tx_dma(ptr);
    spi_enable_rx_dma(ptr);
    /* for master mode, This CMD register must be written with a dummy value
     * to start a SPI transfer even when the command phase is not enabled
     */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    }
    return stat;
}

hpm_stat_t hpm_spi_receive_setup_dma(SPI_Type *ptr, uint32_t size)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_read_only, size);
    spi_disable_tx_dma(ptr);
    spi_enable_rx_dma(ptr);
    /* for master mode, This CMD register must be written with a dummy value
        to start a SPI transfer even when the command phase is not enabled
    */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    }
    return stat;
}

hpm_stat_t hpm_spi_transmit_setup_dma(SPI_Type *ptr, uint32_t size)
{
    hpm_stat_t stat = status_success;
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((size == 0) ||
        ((SPI_SOC_TRANSFER_COUNT_MAX == 512) && (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    hpm_spi_transfer_init(ptr, spi_trans_write_only, size);
    spi_enable_tx_dma(ptr);
    spi_disable_rx_dma(ptr);
    /* for master mode, This CMD register must be written with a dummy value
     * to start a SPI transfer even when the command phase is not enabled
     */
    if (SPI_TRANSFMT_SLVMODE_GET(ptr->TRANSFMT) == spi_master_mode) {
        ptr->CMD = 0xFF;
    }
    return stat;
}

#if USE_DMA_MGR
void dma_channel_tc_callback(DMA_Type *ptr, uint32_t channel, void *user_data)
{
    hpm_spi_cfg_t *obj = (hpm_spi_cfg_t *)user_data;
    if ((obj->rxdma_resource.channel == channel) &&
        (obj->rxdma_resource.base == ptr) &&
        (obj->rx_dma_complete != NULL)) {
            obj->rx_dma_complete(channel);
    }
    if ((obj->txdma_resource.channel == channel) &&
        (obj->txdma_resource.base == ptr) &&
        (obj->tx_dma_complete != NULL)) {
            obj->tx_dma_complete(channel);
    }
}

hpm_stat_t hpm_spi_dma_install_callback(SPI_Type *ptr, spi_dma_complete_cb tx_complete, spi_dma_complete_cb rx_complete)
{
    dma_mgr_chn_conf_t chg_config;
    dma_resource_t *resource = NULL;
    hpm_spi_cfg_t *obj = hpm_spi_get_cfg_obj(ptr);
    if (obj == NULL) {
        return status_invalid_argument;
    }
    dma_mgr_get_default_chn_config(&chg_config);
    chg_config.src_width = DMA_MGR_TRANSFER_WIDTH_BYTE;
    chg_config.dst_width = DMA_MGR_TRANSFER_WIDTH_BYTE;
    /* spi rx dma config */
    resource = &obj->rxdma_resource;
    if (dma_mgr_request_resource(resource) == status_success) {
        chg_config.src_mode = DMA_MGR_HANDSHAKE_MODE_HANDSHAKE;
        chg_config.src_addr_ctrl = DMA_MGR_ADDRESS_CONTROL_FIXED;
        chg_config.src_addr = (uint32_t)&ptr->DATA;
        chg_config.dst_mode = DMA_MGR_HANDSHAKE_MODE_NORMAL;
        chg_config.dst_addr_ctrl = DMA_MGR_ADDRESS_CONTROL_INCREMENT;
        chg_config.en_dmamux = true;
        chg_config.dmamux_src = obj->rx_dmamux_src;
        dma_mgr_setup_channel(resource, &chg_config);
        dma_mgr_install_chn_tc_callback(resource, dma_channel_tc_callback, (void *)obj);
        dma_mgr_enable_chn_irq(resource, DMA_MGR_INTERRUPT_MASK_TC);
        dma_mgr_enable_dma_irq_with_priority(resource, 1);
        obj->rx_dma_complete = rx_complete;
    }
     /* spi tx dma config */
    resource = &obj->txdma_resource;
    if (dma_mgr_request_resource(resource) == status_success) {
        chg_config.src_mode = DMA_MGR_HANDSHAKE_MODE_NORMAL;
        chg_config.src_addr_ctrl = DMA_MGR_ADDRESS_CONTROL_INCREMENT;
        chg_config.dst_mode = DMA_MGR_HANDSHAKE_MODE_HANDSHAKE;
        chg_config.dst_addr_ctrl = DMA_MGR_ADDRESS_CONTROL_FIXED;
        chg_config.dst_addr = (uint32_t)&ptr->DATA;
        chg_config.en_dmamux = true;
        chg_config.dmamux_src = obj->tx_dmamux_src;
        dma_mgr_setup_channel(resource, &chg_config);
        dma_mgr_install_chn_tc_callback(resource, dma_channel_tc_callback, (void *)obj);
        dma_mgr_enable_chn_irq(resource, DMA_MGR_INTERRUPT_MASK_TC);
        dma_mgr_enable_dma_irq_with_priority(resource, 1);
        obj->tx_dma_complete = tx_complete;
    }
    return status_success;
}

hpm_stat_t hpm_spi_transmit_receive_nonblocking(SPI_Type *ptr, uint8_t *wbuff, uint8_t *rbuff, uint32_t size)
{
    hpm_stat_t stat = status_success;
    hpm_spi_cfg_t *obj = hpm_spi_get_cfg_obj(ptr);
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((obj == NULL) ||
        (spi_master_get_data_phase_format(ptr) != spi_single_io_mode) ||
        (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes))) {
        return status_invalid_argument;
    }
    spi_disable_rx_dma(ptr);
    spi_disable_tx_dma(ptr);
    spi_transmit_fifo_reset(ptr);
    spi_receive_fifo_reset(ptr);
    dma_resource_t *resource = &obj->rxdma_resource;
    uint32_t buf_addr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)rbuff);
    dma_mgr_set_chn_dst_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_src_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_dst_addr(resource, buf_addr);
    dma_mgr_set_chn_transize(resource, size >> (data_len_in_bytes - 1));
    dma_mgr_enable_channel(resource);
    resource = &obj->txdma_resource;
    buf_addr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)wbuff);
    dma_mgr_set_chn_dst_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_src_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_src_addr(resource, buf_addr);
    dma_mgr_set_chn_transize(resource, size >> (data_len_in_bytes - 1));
    dma_mgr_enable_channel(resource);
    stat = hpm_spi_transmit_receive_setup_dma(ptr, size);
    return stat;
}

hpm_stat_t hpm_spi_receive_nonblocking(SPI_Type *ptr, uint8_t *buff, uint32_t size)
{
    hpm_stat_t stat = status_success;
    hpm_spi_cfg_t *obj = hpm_spi_get_cfg_obj(ptr);
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((obj == NULL) ||
        ((size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes)))) {
        return status_invalid_argument;
    }
    spi_disable_rx_dma(ptr);
    spi_disable_tx_dma(ptr);
    spi_transmit_fifo_reset(ptr);
    spi_receive_fifo_reset(ptr);
    dma_resource_t *resource = &obj->rxdma_resource;
    uint32_t buf_addr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)buff);
    dma_mgr_set_chn_dst_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_src_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_dst_addr(resource, buf_addr);
    dma_mgr_set_chn_transize(resource, size >> (data_len_in_bytes - 1));
    dma_mgr_enable_channel(resource);
    stat = hpm_spi_receive_setup_dma(ptr, size);
    return stat;
}

hpm_stat_t hpm_spi_transmit_nonblocking(SPI_Type *ptr, uint8_t *buff, uint32_t size)
{
    hpm_stat_t stat = status_success;
    hpm_spi_cfg_t *obj = hpm_spi_get_cfg_obj(ptr);
    uint8_t data_len_in_bytes = spi_get_data_length_in_bytes(ptr);
    if ((obj == NULL) ||
        (size > (SPI_SOC_TRANSFER_COUNT_MAX * data_len_in_bytes))) {
        return status_invalid_argument;
    }
    dma_resource_t *resource = &obj->txdma_resource;
    uint32_t buf_addr = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)buff);
    spi_disable_rx_dma(ptr);
    spi_disable_tx_dma(ptr);
    spi_transmit_fifo_reset(ptr);
    spi_receive_fifo_reset(ptr);
    dma_mgr_set_chn_src_addr(resource, buf_addr);
    dma_mgr_set_chn_dst_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_src_width(resource, data_len_in_bytes - 1);
    dma_mgr_set_chn_transize(resource, size >> (data_len_in_bytes - 1));
    dma_mgr_enable_channel(resource);
    stat = hpm_spi_transmit_setup_dma(ptr, size);
    return stat;
}
#endif
