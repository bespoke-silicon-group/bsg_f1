#ifndef BSG_MANYCORE_TILE_H
#define BSG_MANYCORE_TILE_H

#include <bsg_manycore_features.h>
#include <bsg_manycore_epa.h>
#include <bsg_manycore_driver.h>
#include <bsg_manycore.h>
#include <bsg_manycore_mem.h>
#include <bsg_manycore_elf.h>
#include <bsg_manycore_printing.h>
#include <bsg_manycore_eva.h>
#include <bsg_manycore_loader.h>

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __hb_mc_csr_freeze_t{
        HB_MC_CSR_FREEZE = 1,
        HB_MC_CSR_UNFREEZE = 0
} hb_mc_csr_freeze_t;

/*************/
/* Tile EPAs */
/*************/

/* Offsets in bytes */
#define HB_MC_TILE_EPA_DMEM_BASE   0x00001000
#define HB_MC_TILE_EVA_DMEM_BASE   0x00001000
#define HB_MC_TILE_EPA_ICACHE_BASE 0x01000000
#define HB_MC_TILE_EPA_CSR_BASE                       0x20000
#define HB_MC_TILE_EPA_CSR_FREEZE_OFFSET              0x00
#define HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_X_OFFSET 0x04
#define HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_Y_OFFSET 0x08


#define HB_MC_TILE_DMEM_LOGSZ 12
#define HB_MC_TILE_DMEM_SIZE (1 << HB_MC_TILE_DMEM_LOGSZ)

#define EPA_TILE_CSR_FROM_BYTE_OFFSET(offset)				\
	EPA_FROM_BASE_AND_OFFSET(HB_MC_TILE_EPA_CSR_BASE, offset)

/* EPAs */
#define HB_MC_TILE_EPA_ICACHE					\
	EPA_FROM_BASE_AND_OFFSET(HB_MC_TILE_EPA_ICACHE_BASE, 0)

#define HB_MC_TILE_EPA_CSR_FREEZE					\
	EPA_TILE_CSR_FROM_BYTE_OFFSET(HB_MC_TILE_EPA_CSR_FREEZE_OFFSET)

#define HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_X				\
	EPA_TILE_CSR_FROM_BYTE_OFFSET(HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_X_OFFSET)

#define HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_Y				\
	EPA_TILE_CSR_FROM_BYTE_OFFSET(HB_MC_TILE_EPA_CSR_TILE_GROUP_ORIGIN_Y_OFFSET)


/**
 * Set a tile's x origin
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] tile   A tile to set the origin of.
 * @param[in] x      The X coordinate of the origin tile
 * @return HB_MC_SUCCESS if succesful. Otherwise an error code is returned.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_origin_x(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *tile, 
			const hb_mc_idx_t x);
/**
 * Set a tile's y origin
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] tile   A tile to set the origin of.
 * @param[in] y      The Y coordinate of the origin tile
 * @return HB_MC_SUCCESS if succesful. Otherwise an error code is returned.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_origin_y(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *tile, 
			const hb_mc_idx_t y);
/**
 * Set a tile's origin
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] tile   A tile to set the origin of.
 * @param[in] o      The origin tile
 * @return HB_MC_SUCCESS if succesful. Otherwise an error code is returned.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_origin(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *tile, 
			const hb_mc_coordinate_t *o);

/**
 * Freeze a tile.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] tile   A tile to freeze.
 * @return HB_MC_SUCCESS if succesful. Otherwise an error code is returned.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_freeze(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *tile);

/**
 * Unfreeze a tile.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc     A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] tile   A tile to unfreeze.
 * @return HB_MC_SUCCESS if succesful. Otherwise an error code is returned.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_unfreeze(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *tile);

/**
 * Get the size of a tiles local data memory.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  tile   The coordinate tile to query for its data memory size.
 * @return the size of the tiles data memory.
 */
static inline size_t hb_mc_tile_get_size_dmem(const hb_mc_manycore_t *mc,
					const hb_mc_coordinate_t *tile)
{
	return hb_mc_config_get_dmem_size(hb_mc_manycore_get_config(mc));
}

/**
 * Get the size of a tiles local data memory.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in]  mc     A manycore instance initialized with hb_mc_manycore_init()
 * @param[in]  tile   The coordinate tile to query for its instruction cache size.
 * @return the size of the tiles instruction cache.
 */
static inline size_t hb_mc_tile_get_size_icache(const hb_mc_manycore_t *mc,
						const hb_mc_coordinate_t *tile)
{
	return 4 * (1<<10); //4K -- this might be later read from the config
}





/*! 
 * Writes a uint8_t into a tile given its coordinates and epa 
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] coord      Cooridnates of the destination tile
 * @param[in] epa        Epa address in desitnation tile to which the write occurs
 * @param[in] v          Variable to be written into tile 
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_write8(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *coord, const hb_mc_epa_t *epa, uint8_t v); 




/*! 
 * Writes a uint16_t into a tile given its coordinates and epa 
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] coord      Cooridnates of the destination tile
 * @param[in] epa        Epa address in desitnation tile to which the write occurs
 * @param[in] vp         Variable to be written into tile 
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_write16(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *coord, const hb_mc_epa_t *epa, uint16_t v); 




/*! 
 * Writes a uint32_t into a tile given its coordinates and epa 
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] coord      Cooridnates of the destination tile
 * @param[in] epa        Epa address in desitnation tile to which the write occurs
 * @param[in] vp         Variable to be written into tile 
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_write32(hb_mc_manycore_t *mc, const hb_mc_coordinate_t *coord, const hb_mc_epa_t *epa, uint32_t v); 




/*!
 * Sets a Vanilla Core Endpoint's tile group's origin hardware registers CSR_TGO_X/Y.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] coord      Tile coordinates to set the origin of.
 * @param[in] origin     Origin coordinates.
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_origin_registers (hb_mc_manycore_t *mc, const hb_mc_coordinate_t *coord, const hb_mc_coordinate_t *origin); 




/*!
 * Sets a Vanilla Core Endpoint's tile group's origin symbols __bsg_grp_org_x/y.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] map        Eva to npa mapping. 
 * @param[in] bin        Binary elf file.
 * @param[in] bin_size   Size of binary file. 
 * @param[in] coord      Tile coordinates to set the origin of.
 * @param[in] origin     Origin coordinates.
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_origin_symbols(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, unsigned char *bin, size_t bin_size, const hb_mc_coordinate_t *coord, const hb_mc_coordinate_t *origin);




/*!
 * Sets a Vanilla Core Endpoint's tile group's coordinate symbols __bsg_x/y.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] map        Eva to npa mapping. 
 * @param[in] bin        Binary elf file. 
 * @param[in] bin_size   Size of binary file. 
 * @param[in] coord      Tile coordinates to set the coordinates of.
 * @param[in] coord_val  The coordinates to set the tile.
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
 */
__attribute__((warn_unused_result))
int hb_mc_tile_set_coord_symbols(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, unsigned char* bin, size_t bin_size, const hb_mc_coordinate_t *coord, const hb_mc_coordinate_t *coord_val);




/*! 
 * Sets a Vanilla Core Endpoint's tile's __bsg_id symbol.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] map        Eva to npa mapping. 
 * @param[in] bin        Binary elf file. 
 * @param[in] bin_size   Size of binary file. 
 * @param[in] coord      Tile coordinates to set the id of.
 * @param[in] coord_val  The coordinates to set the tile.
 * @param[in] dim        Tile group dimensions
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_set_id_symbol(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, unsigned char* bin, size_t bin_size, const hb_mc_coordinate_t *coord, const hb_mc_coordinate_t *coord_val, const hb_mc_dimension_t *dim);




/*! 
 * Sets a Vanilla Core Endpoint's tile's __bsg_tile_group_id_x/y symbol.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] map        Eva to npa mapping. 
 * @param[in] bin        Binary elf file. 
 * @param[in] bin_size   Size of binary file. 
 * @param[in] coord      Tile coordinates to set the tile group id of.
 * @param[in] tg_id      Tile group id
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_set_tile_group_id_symbols(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, unsigned char* bin, size_t bin_size, const hb_mc_coordinate_t *coord, const hb_mc_coordinate_t *tg_id);




/*! 
 * Sets a Vanilla Core Endpoint's tile's __bsg_grid_dim_x/y symbol.
 * Behavior is undefined if #mc is not initialized with hb_mc_manycore_init().
 * @param[in] mc         A manycore instance initialized with hb_mc_manycore_init().
 * @param[in] map        Eva to npa mapping. 
 * @param[in] bin        Binary elf file. 
 * @param[in] bin_size   Size of binary file. 
 * @param[in] coord      Tile coordinates to set the tile group id of.
 * @param[in] tg_id      Grid dimensions
 * @return HB_MC_SUCCESS on success and HB_MC_FAIL on failure.
* */
__attribute__((warn_unused_result))
int hb_mc_tile_set_grid_dim_symbols(hb_mc_manycore_t *mc, hb_mc_eva_map_t *map, unsigned char* bin, size_t bin_size, const hb_mc_coordinate_t *coord, const hb_mc_dimension_t *grid_dim);








/** Deprecated **/
#define hb_mc_tile_epa_get_byte_addr(base, offset) (base + offset)

#define hb_mc_tile_epa_get_word_addr(base, offset) \
	(hb_mc_tile_epa_get_byte_addr(base, offset) >> 2)

__attribute__((deprecated))
int hb_mc_tile_freeze_dep (uint8_t fd, uint8_t x, uint8_t y);

__attribute__((deprecated))
int hb_mc_tile_unfreeze_dep (uint8_t fd, uint8_t x, uint8_t y);

__attribute__((deprecated))
int hb_mc_tile_set_origin_registers_dep(uint8_t fd, hb_mc_coordinate_t coord, hb_mc_coordinate_t origin);

__attribute__((deprecated))
int hb_mc_tile_set_origin_symbols_dep(uint8_t fd, eva_id_t eva_id, char* bin_name, hb_mc_coordinate_t coord, hb_mc_coordinate_t origin);

__attribute__((deprecated))
int hb_mc_tile_set_coord_symbols_dep(uint8_t fd, eva_id_t eva_id, char* bin_name, hb_mc_coordinate_t coord, hb_mc_coordinate_t coord_val);

__attribute__((deprecated))
int hb_mc_tile_set_id_symbol_dep(uint8_t fd, eva_id_t eva_id, char* bin_name, hb_mc_coordinate_t coord, hb_mc_coordinate_t coord_val, hb_mc_dimension_t dim);

__attribute__((deprecated))
int hb_mc_tile_set_tile_group_id_symbols_dep(uint8_t fd, eva_id_t eva_id, char* bin_name, hb_mc_coordinate_t coord, hb_mc_coordinate_t tg_id);

__attribute__((deprecated))
int hb_mc_tile_set_grid_dim_symbols_dep(uint8_t fd, eva_id_t eva_id, char* bin_name, hb_mc_coordinate_t coord, hb_mc_dimension_t grid_dim);




#ifdef __cplusplus
}
#endif

#endif
