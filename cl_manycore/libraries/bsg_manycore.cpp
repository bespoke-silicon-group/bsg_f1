#define DEBUG
#ifndef COSIM
#include <bsg_manycore.h>
#include <bsg_manycore_printing.h>
#include <fpga_pci.h>
#include <fpga_mgmt.h>
#else
#include "bsg_manycore.h"
#include "bsg_manycore_printing.h"
#include "fpga_pci_sv.h"
#include <utils/sh_dpi_tasks.h>
#endif
#include <cstdlib>
#include <cstring>
#include <cinttypes>
/* these are conveniance macros that are only good for one line prints */
#define manycore_pr_dbg(mc, fmt, ...)			\
	bsg_pr_dbg("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_err(mc, fmt, ...)			\
	bsg_pr_err("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_warn(mc, fmt, ...)				\
	bsg_pr_warn("%s: " fmt, mc->name, ##__VA_ARGS__)

#define manycore_pr_info(mc, fmt, ...)				\
	bsg_pr_info("%s: " fmt, mc->name, ##__VA_ARGS__)


typedef struct hb_mc_manycore_private {
	pci_bar_handle_t handle;	
} hb_mc_manycore_private_t;


static int  hb_mc_manycore_mmio_read_mmio(hb_mc_manycore_t *mc, uintptr_t offset,
					  void *vo, size_t sz);

static int  hb_mc_manycore_mmio_read_pci(hb_mc_manycore_t *mc, uintptr_t offset,
					 void *vp, size_t sz);
static int hb_mc_manycore_mmio_read(hb_mc_manycore_t *mc, uintptr_t offset,
				    void *vp, size_t sz);

static int  hb_mc_manycore_init_mmio(hb_mc_manycore_t *mc, hb_mc_manycore_id_t id);
static void hb_mc_manycore_cleanup_mmio(hb_mc_manycore_t *mc);
static int  hb_mc_manycore_init_private_data(hb_mc_manycore_t *mc);
static void hb_mc_manycore_cleanup_private_data(hb_mc_manycore_t *mc);

///////////////////
// Init/Exit API //
///////////////////
static int hb_mc_manycore_init_mmio(hb_mc_manycore_t *mc, hb_mc_manycore_id_t id)
{
	hb_mc_manycore_private_t *pdata = (hb_mc_manycore_private_t*)mc->private_data;
	int pf_id = FPGA_APP_PF, write_combine = 0, bar_id = APP_PF_BAR0;
	int r = HB_MC_FAIL, err;

	// all IDs except 0 are unused at the moment
	if (id != 0) {
		manycore_pr_err(mc, "failed to init MMIO: invalid ID\n");
		return r;
	}

	if ((err = fpga_pci_attach(id, pf_id, bar_id, write_combine, &pdata->handle)) != 0) {
		manycore_pr_err(mc, "failed to init MMIO: %s\n", FPGA_ERR2STR(err));
		return r;		
	}

#if !defined(COSIM) // on F1
	// it is not clear to me where 0x4000 comes from...
	// map in the base address register to our address space
	if ((err = fpga_pci_get_address(pdata->handle, 0, 0x4000, (void**)&mc->mmio)) != 0) {
		manycore_pr_err(mc, "failed to init MMIO: %s\n", FPGA_ERR2STR(err));
		return r;
	}
#else
	mc->mmio = (uintptr_t)nullptr;
#endif
	mc->id = id;
	r = HB_MC_SUCCESS;
	manycore_pr_dbg(mc, "%s: mc->mmio = 0x%" PRIxPTR "\n", __func__, mc->mmio);
	goto done;
	
cleanup:
	fpga_pci_detach(pdata->handle);
	pdata->handle = PCI_BAR_HANDLE_INIT;
done:
	return r;	
}

static void hb_mc_manycore_cleanup_mmio(hb_mc_manycore_t *mc)
{
	hb_mc_manycore_private_t *pdata = (hb_mc_manycore_private_t*)mc->private_data;
	int err;
	
	if ((err = fpga_pci_detach(pdata->handle)) != 0)
		manycore_pr_err(mc, "failed to cleanup MMIO: %s\n", FPGA_ERR2STR(err));

	pdata->handle = PCI_BAR_HANDLE_INIT;
	mc->mmio = (uintptr_t)nullptr;
	mc->id = 0;
	return;
}

static int hb_mc_manycore_init_private_data(hb_mc_manycore_t *mc)
{
	int r = HB_MC_FAIL, err;
	hb_mc_manycore_private_t *pdata;

	mc->private_data = nullptr;
	
	if (!(pdata = (hb_mc_manycore_private_t*)calloc(sizeof(*pdata), 1))) {
		manycore_pr_err(mc, "%s failed: %m\n", __func__);
		return HB_MC_FAIL;
	}

	pdata->handle = PCI_BAR_HANDLE_INIT;
	mc->private_data = pdata;
	
	return HB_MC_SUCCESS;
}

static void hb_mc_manycore_cleanup_private_data(hb_mc_manycore_t *mc)
{
	free(mc->private_data);
}
/**
 * Initialize a manycore instance
 * @param[in] mc    A manycore to initialize
 * @param[in] name  A name to give this manycore instance (used for debugging)
 * @param[in] id    ID which selects the physical hardware from which this manycore is configured
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int  hb_mc_manycore_init(hb_mc_manycore_t *mc, const char *name, hb_mc_manycore_id_t id)
{
	int r = HB_MC_FAIL, err;

	// copy name
	mc->name = strdup(name);
	if (!mc->name) {
		bsg_pr_err("failed to initialize %s: %m\n", name);
		return r;
	}


	// initialize private data
	if ((err = hb_mc_manycore_init_private_data(mc) != HB_MC_SUCCESS))
		goto cleanup;

	// initialize manycore for MMIO
	if ((err = hb_mc_manycore_init_mmio(mc, id) != HB_MC_SUCCESS))
		goto cleanup;
	
	r = HB_MC_SUCCESS;
	goto done;

cleanup:
	hb_mc_manycore_cleanup_private_data(mc);
	free((void*)mc->name);	
	
done:
	return r;
}

/**
 * Cleanup an initialized manycore instance
 * @param[in] mc   A manycore instance that has been initialized with hb_mc_manycore_init()
 */
void hb_mc_manycore_exit(hb_mc_manycore_t *mc);

/************/
/* MMIO API */
/************/
/**
 * Reads data for MMIO by actualling doing loads
 */
static int  hb_mc_manycore_mmio_read_mmio(hb_mc_manycore_t *mc, uintptr_t offset,
					  void *vp, size_t sz)
{
	unsigned char *addr = (unsigned char *)mc->mmio;
        uint32_t tmp;
        
	if (addr == nullptr) {
		manycore_pr_err(mc, "%s: failed: MMIO not initialized", __func__);
		return HB_MC_FAIL;
	}

	// check that the address is aligned to a four byte boundar
	if (offset % 4) {
		manycore_pr_err(mc, "%s: failed: 0x%" PRIxPTR " is not aligned to 4 byte boundary\n",
				__func__, offset);
		return HB_MC_FAIL;
	}
	
	addr = &addr[offset];

        tmp = *(volatile uint32_t *)addr;
        
	switch (sz) {
	case 4:
		*(uint32_t*)vp = tmp;
		break;
	case 2:
		*(uint16_t*)vp = tmp;
		break;
	case 1:
		*(uint8_t*)vp  = tmp;
		break;
	default:
		manycore_pr_err(mc, "%s: failed: invalid load size (%zu)\n", __func__, sz);
		return HB_MC_FAIL;
	}

	return 0;
}
/**
 * Reads data for MMIO instead by using PCI ops (used in COSIM)
 */
static int  hb_mc_manycore_mmio_read_pci(hb_mc_manycore_t *mc, uintptr_t offset,
					 void *vp, size_t sz)
{
	hb_mc_manycore_private_t *pdata = (hb_mc_manycore_private_t*)mc->private_data;
	uint32_t val;
	int err;
	
	if ((err = fpga_pci_peek(pdata->handle, offset, &val)) != 0) {
		manycore_pr_err(mc, "%s: failed: %s\n", __func__, FPGA_ERR2STR(err));
		return HB_MC_FAIL;
	}

	switch (sz) {
	case 4:
		*(uint32_t*)vp = val;
		break;
	case 2:
		*(uint16_t*)vp = val;
		break;
	case 1:
		*(uint8_t *)vp = val;
		break;
	default:
		manycore_pr_err(mc, "%s: failed: invalid load size (%zu)\n", __func__, sz);
		return HB_MC_FAIL;
	}
}

static int hb_mc_manycore_mmio_read(hb_mc_manycore_t *mc, uintptr_t offset,
				    void *vp, size_t sz)
{        
#if !defined(COSIM)
	return hb_mc_manycore_mmio_read_mmio(mc, offset, vp, sz);
#else
	return hb_mc_manycore_mmio_read_pci(mc,  offset, vp, sz);
#endif
}

/**
 * Writes data for MMIO by direct stores
 */
static int hb_mc_manycore_mmio_write_mmio(hb_mc_manycore_t *mc, uintptr_t offset,
                                          void *vp, size_t sz)
{
        unsigned char *addr = (unsigned char *)mc->mmio;
        uint32_t tmp;
        
	if (addr == nullptr) {
		manycore_pr_err(mc, "%s: failed: MMIO not initialized", __func__);
		return HB_MC_FAIL;
	}

	// check that the address is aligned to a four byte boundary
	if (offset % 4) {
		manycore_pr_err(mc, "%s: failed: 0x%" PRIxPTR " is not aligned to 4 byte boundary\n",
				__func__, offset);
		return HB_MC_FAIL;
	}
	
	addr = &addr[offset];

	switch (sz) {
	case 4:
                tmp = *(uint32_t *)vp;
		break;
	case 2:
                tmp = *(uint16_t*)vp;
		break;
	case 1:
                tmp = *(uint8_t*)vp;
		break;
	default:
		manycore_pr_err(mc, "%s: failed: invalid load size (%zu)\n", __func__, sz);
		return HB_MC_FAIL;
	}

        *(volatile uint32_t *)addr = tmp;
        
	return 0;
}

/**
 * Writes data for MMIO instead by  PCI ops (used in COSIM)
 */
static int hb_mc_manycore_mmio_write_pci(hb_mc_manycore_t *mc, uintptr_t offset,
                                         void *vp, size_t sz)
{
        hb_mc_manycore_private_t *pdata = (hb_mc_manycore_private_t*)mc->private_data;
	uint32_t val;
	int err;

        switch (sz) {
	case 4:
                val = *(uint32_t*)vp;
		break;
	case 2:
                val = *(uint16_t*)vp;
		break;
	case 1:
                val = *(uint8_t*)vp;
		break;
	default:
		manycore_pr_err(mc, "%s: failed: invalid store size (%zu)\n", __func__, sz);
		return HB_MC_FAIL;
	}
        
	if ((err = fpga_pci_poke(pdata->handle, offset, val)) != 0) {
		manycore_pr_err(mc, "%s: failed: %s\n", __func__, FPGA_ERR2STR(err));
		return HB_MC_FAIL;
	}	
}
static int hb_mc_manycore_mmio_write(hb_mc_manycore_t *mc, uintptr_t offset,
                                     void *vp, size_t sz)
{
#if !defined(COSIM)
        return hb_mc_manycore_mmio_write_mmio(mc, offset, vp, sz);
#else
        return hb_mc_manycore_mmio_write_pci(mc, offset, vp, sz);
#endif
}
/**
 * Read one byte from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[out] vp     A byte to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_read8(hb_mc_manycore_t *mc, uintptr_t offset, uint8_t *vp)
{
	return hb_mc_manycore_mmio_read(mc, offset, (void*)vp, 1);
}

/**
 * Read a 16-bit half-word from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[out] vp     A half-word to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_read16(hb_mc_manycore_t *mc, uintptr_t offset, uint16_t *vp)
{
	return hb_mc_manycore_mmio_read(mc, offset, (void*)vp, 2);
}

/**
 * Read a 32-bit word from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[out] vp     A word to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_read32(hb_mc_manycore_t *mc, uintptr_t offset, uint32_t *vp)
{
	return hb_mc_manycore_mmio_read(mc, offset, (void*)vp, 4);
}

/**
 * Write one byte to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[in]  v      A byte value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_write8(hb_mc_manycore_t *mc, uintptr_t offset, uint8_t v)
{
        return hb_mc_manycore_mmio_write(mc, offset, (void*)vp, 1);
}

/**
 * Write a 16-bit half-word to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[in]  v      A half-word value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_write16(hb_mc_manycore_t *mc, uintptr_t offset, uint16_t v)
{
        return hb_mc_manycore_mmio_write(mc, offset, (void*)vp, 2);
}

/**
 * Write a 32-bit word to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  offset An  offset into the manycore's MMIO address space
 * @param[in]  v      A word value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */

int hb_mc_manycore_mmio_write32(hb_mc_manycore_t *mc, uintptr_t offset, uint32_t v)
{
        return hb_mc_manycore_mmio_write(mc, offset, (void*)vp, 4);
}

////////////////
// Packet API //
////////////////

/**
 * Transmit a packet to manycore hardware
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] packet A packet to transmit to manycore hardware
 * @param[in] type   Is this packet a request or response packet?
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_packet_tx(hb_mc_manycore_t *mc,
				    hb_mc_packet_t *packet,
				    hb_mc_fifo_tx_t type);

/**
 * Receive a packet from manycore hardware
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in] packet A packet into which data should be read
 * @param[in] type   Is this packet a request or response packet?
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_packet_rx(hb_mc_manycore_t *mc,
				    hb_mc_packet_t *packet,
				    hb_mc_fifo_rx_t type);
////////////////
// Memory API //
////////////////

/**
 * Read one byte from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t
 * @param[out] vp     A byte to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_read8(hb_mc_manycore_t *mc, npa_t *npa, uint8_t *vp);

/**
 * Read a 16-bit half-word from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t aligned to a two byte boundary
 * @param[out] vp     A half-word to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_read16(hb_mc_manycore_t *mc, npa_t *npa, uint16_t *vp);

/**
 * Read a 32-bit word from manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t aligned to a four byte boundary
 * @param[out] vp     A word to be set to the data read
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_read32(hb_mc_manycore_t *mc, npa_t *npa, uint32_t *vp);

/**
 * Write one byte to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t
 * @param[in]  v      A byte value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_write8(hb_mc_manycore_t *mc, npa_t *npa, uint8_t v);

/**
 * Write a 16-bit half-word to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t aligned to a two byte boundary
 * @param[in]  v      A half-word value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_write16(hb_mc_manycore_t *mc, npa_t *npa, uint16_t v);

/**
 * Write a 32-bit word to manycore hardware at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t aligned to a four byte boundary
 * @param[in]  v      A word value to be written out
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_write32(hb_mc_manycore_t *mc, npa_t *npa, uint32_t v);

/**
 * Write memory out to manycore hardware starting at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t
 * @param[in]  data   A buffer to be written out manycore hardware
 * @param[in]  sz     The number of bytes to write to manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_write_mem(hb_mc_manycore_t *mc, npa_t *npa,
				    const void *data, size_t sz);

/**
 * Read memory from manycore hardware starting at a given NPA
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t
 * @param[in]  data   A buffer into which data will be read
 * @param[in]  sz     The number of bytes to read from manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_read_mem(hb_mc_manycore_t *mc, npa_t *npa,
				   void *data, size_t sz);
/////////////////////////////
// Address Translation API //
/////////////////////////////
/**
 * Translates a Network Physical Address to an Endpoint Virtual Address
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  npa    A valid npa_t to translate
 * @param[out] eva    An eva to be set by translating #npa
 * @param[in]  coordinate A coordinate for which #eva will be formatted
 * @param[in]  eva_id An EVA address space ID (unused: should always be 0)
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_npa_to_eva(hb_mc_manycore_t *mc, npa_t *npa, eva_t *eva,
				     hb_mc_coordinate_t coordinate,
				     eva_id_t eva_id);

/**
 * Translate an Endpoint Virtual Address to a Network Physical Address
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  eva    An eva to translate
 * @param[out] npa    An npa to be set by translating #eva
 * @param[in]  coordinate A coordinate for which #eva is be formatted
 * @param[in]  eva_id An EVA address space ID (unused: should always be 0)
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_eva_to_npa(hb_mc_manycore_t *mc, eva_t eva,  npa_t *npa,
				     hb_mc_coordinate_t coordinate,
				     eva_id_t eva_id);

