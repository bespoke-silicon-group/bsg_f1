#ifndef BSG_MANYCORE_EVA_H
#define BSG_MANYCORE_EVA_H

#ifndef COSIM
#include <bsg_manycore_features.h>
#include <bsg_manycore_coordinate.h>
#include <bsg_manycore.h>
#include <bsg_manycore_npa.h>
#include <bsg_manycore_epa.h>
#else
#include "bsg_manycore_features.h"
#include "bsg_manycore_coordinate.h"
#include "bsg_manycore.h"
#include "bsg_manycore_npa.h"
#include "bsg_manycore_epa.h"
#endif

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hb_mc_eva_t;

typedef struct __hb_mc_eva_map_t{
	const char *eva_map_name;
	const void *priv;
/**
 * Translate an Endpoint Virtual Address in a source tile's address space
 * to a Network Physical Address
 * @param[in]  cfg    An initialized manycore configuration struct
 * @param[in]  priv   Private data used for this EVA Map
 * @param[in]  src    Coordinate of the tile issuing this #eva
 * @param[in]  eva    An eva to translate
 * @param[out] npa    An npa to be set by translating #eva
 * @param[out] sz     The size in bytes of the NPA segment for the #eva
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
	int (*eva_to_npa)(const hb_mc_config_t *cfg, 
			const void *priv,
			const hb_mc_coordinate_t *src, 
			const hb_mc_eva_t *eva, 
			hb_mc_npa_t *npa, size_t *sz);

/**
 * Returns the number of contiguous bytes following an EVA, regardless of
 * the continuity of the underlying NPA.
 * @param[in]  cfg    An initialized manycore configuration struct
 * @param[in]  eva    An eva 
 * @param[out] sz     Number of contiguous bytes remaining in the #eva segment
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
	int (*eva_size)(const hb_mc_config_t *cfg, 
			const hb_mc_eva_t *eva, 
			size_t *sz);

/**
 * Translate a Network Physical Address to an Endpoint Virtual Address in a
 * target tile's address space
 * @param[in]  cfg    An initialized manycore configuration struct
 * @param[in]  priv   Private data used for this EVA Map
 * @param[in]  tgt    Coordinates of the target tile
 * @param[in]  len    Number of tiles in the target tile's group
 * @param[in]  npa    An npa to translate
 * @param[out] eva    An eva to set by translating #npa
 * @param[out] sz     The size in bytes of the EVA segment for the #npa
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
	int (*npa_to_eva)(const hb_mc_config_t *cfg, 
			const void *priv,
			const hb_mc_coordinate_t *tgt, 
			const hb_mc_npa_t *npa, 
			hb_mc_eva_t *eva, size_t *sz);
} hb_mc_eva_map_t;

extern const hb_mc_coordinate_t default_origin;
extern hb_mc_eva_map_t default_map;

/**
 * Get the name of an eva map.
 * @param[in] map  An EVA map. Behaviour is undefined if #map is NULL.
 * @return The name of the EVA map as a string.
 */
static inline const char *hb_mc_eva_map_get_name(const hb_mc_eva_map_t *map)
{
	return map->eva_map_name;
}

/**
 * Translate a Network Physical Address to an Endpoint Virtual Address in a
 * target tile's address space
 * @param[in]  cfg    An initialized manycore configuration struct
 * @param[in]  map    An eva map for computing the eva to npa translation
 * @param[in]  tgt    Coordinates of the target tile
 * @param[in]  npa    An npa to translate
 * @param[out] eva    An eva to set by translating #npa
 * @param[out] sz     The size in bytes of the EVA segment for the #npa
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
__attribute__((warn_unused_result))
int hb_mc_npa_to_eva(const hb_mc_config_t *cfg, 
		const hb_mc_eva_map_t *map, 
		const hb_mc_coordinate_t *tgt,
		const hb_mc_npa_t *npa, 
		hb_mc_eva_t *eva, size_t *sz);

/**
 * Translate an Endpoint Virtual Address in a source tile's address space
 * to a Network Physical Address
 * @param[in]  cfg    An initialized manycore configuration struct
 * @param[in]  map    An eva map for computing the eva to npa translation
 * @param[in]  src    Coordinate of the tile issuing this #eva
 * @param[in]  eva    An eva to translate
 * @param[out] npa    An npa to be set by translating #eva and #map
 * @param[out] sz     The size in bytes of the NPA segment for the #eva
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
__attribute__((warn_unused_result))
int hb_mc_eva_to_npa(const hb_mc_config_t *cfg, 
		const hb_mc_eva_map_t *map, 
		const hb_mc_coordinate_t *src, 
		const hb_mc_eva_t *eva, 
		hb_mc_npa_t *npa, size_t *sz);

/**
 * Write memory out to manycore hardware starting at a given EVA
 * @param[in]  mc     An initialized manycore struct
 * @param[in]  map    An eva map for computing the eva to npa translation
 * @param[in]  tgt    Coordinate of the tile issuing this #eva
 * @param[in]  eva    A valid hb_mc_eva_t
 * @param[in]  data   A buffer to be written out manycore hardware
 * @param[in]  sz     The number of bytes to write to manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
__attribute__((warn_unused_result))
int hb_mc_manycore_eva_write(hb_mc_manycore_t *mc,
			const hb_mc_eva_map_t *map,
			const hb_mc_coordinate_t *tgt, 
			const hb_mc_eva_t *eva,
			const void *data, size_t sz);

/**
 * Read memory from manycore hardware starting at a given EVA
 * @param[in]  mc     An initialized manycore struct
 * @param[in]  map    An eva map for computing the eva to npa translation
 * @param[in]  tgt    Coordinate of the tile issuing this #eva
 * @param[in]  eva    A valid hb_mc_eva_t
 * @param[out] data   A buffer into which data will be read
 * @param[in]  sz     The number of bytes to read from the manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
__attribute__((warn_unused_result))
int hb_mc_manycore_eva_read(hb_mc_manycore_t *mc,
			const hb_mc_eva_map_t *map,
			const hb_mc_coordinate_t *tgt, 
			const hb_mc_eva_t *eva,
			void *data, size_t sz);

/**
 * Set a EVA memory region to a value
 * @param[in]  mc     An initialized manycore struct
 * @param[in]  map    An eva map for computing the eva to npa translation
 * @param[in]  tgt    Coordinate of the tile issuing this #eva
 * @param[in]  eva    A valid hb_mc_eva_t
 * @param[in]  val    The value to write to the region
 * @param[in]  sz     The number of bytes to write to manycore hardware
 * @return HB_MC_FAIL if an error occured. HB_MC_SUCCESS otherwise.
 */
int hb_mc_manycore_eva_memset(hb_mc_manycore_t *mc,
			const hb_mc_eva_map_t *map,
			const hb_mc_coordinate_t *tgt, 
			const hb_mc_eva_t *eva,
			uint8_t val, size_t sz);

/**
 * Returns the EVA associated with a hb_mc_eva_t 
 * @param[in]  eva    A valid eva_t
 * @return A 32-bit EVA Address
 */
static inline uint32_t hb_mc_eva_addr(const hb_mc_eva_t *eva)
{
	return *eva;
}
#ifdef __cplusplus
}
#endif

#endif
