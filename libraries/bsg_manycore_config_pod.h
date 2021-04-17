#ifndef BSG_MANYCORE_CONFIG_POD_H
#define BSG_MANYCORE_CONFIG_POD_H

#include <bsg_manycore_config.h>
#include <bsg_manycore_coordinate.h>

#ifdef __cplusplus
extern "C" {
#endif

        /**
         * Returns the number of pods in each dimension.
         */
        static inline hb_mc_coordinate_t hb_mc_config_pods(const hb_mc_config_t *cfg)
        {
                return cfg->pods;
        }

        /*******************************************/
        /* Pod geometry, addressing, and iteration */
        /*******************************************/

        /**
         * Return the number of bits used for pods in each dimension
         */
        static inline hb_mc_coordinate_t
        hb_mc_config_pod_coord_width(const hb_mc_config_t *cfg)
        {
                return hb_mc_coordinate(3,4);
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_tile_coord_width(const hb_mc_config_t *cfg)
        {
                hb_mc_coordinate_t pod = hb_mc_config_pod_coord_width(cfg);
                hb_mc_coordinate_t noc = cfg->noc_coord_width;
                return hb_mc_coordinate(noc.x - pod.x, noc.y - pod.y);
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_tile_coord_mask(const hb_mc_config_t *cfg)
        {
                hb_mc_coordinate_t w = hb_mc_config_tile_coord_width(cfg);
                return hb_mc_coordinate((1 << w.x)-1, (1 << w.y)-1);
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_pod_coord_mask(const hb_mc_config_t *cfg)
        {
                hb_mc_coordinate_t tm = hb_mc_config_tile_coord_mask(cfg);
                return hb_mc_coordinate(~tm.x, ~tm.y);
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_coord_pod(const hb_mc_config_t *cfg, hb_mc_coordinate_t coord)
        {
                return hb_mc_coordinate(0,0);
        }

        /**
         * Iterates over pods
         */
#define hb_mc_config_foreach_pod(coord, cfg)                            \
        foreach_coordinate(coord, hb_mc_coordinate(0,0), (cfg)->pods)

        /**
         * Returns the network coordinate of the origin core for a pod ID
         */
        static inline hb_mc_coordinate_t
        hb_mc_config_pod_vcore_origin(const hb_mc_config_t *cfg, hb_mc_coordinate_t pod)
        {
                hb_mc_coordinate_t tile_w = hb_mc_config_tile_coord_width(cfg);
                hb_mc_coordinate_t og = hb_mc_coordinate(1 << tile_w.x, 1 << tile_w.y);
                return hb_mc_coordinate( og.x + (1<<tile_w.x) * (pod.x),
                                         og.y + (1<<tile_w.y) * (pod.y*2) );
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_vcore_to_pod(const hb_mc_config_t *cfg, hb_mc_coordinate_t vcore)
        {
                hb_mc_coordinate_t tile_w = hb_mc_config_tile_coord_width(cfg);
                return hb_mc_coordinate(vcore.x >> tile_w.x, vcore.y >> tile_w.y);
        }

        /**
         * Iterates over a pod's vanilla cores
         */
#define hb_mc_config_pod_foreach_vcore(coord, pod_id, cfg)              \
        foreach_coordinate(coord,                                       \
                           hb_mc_config_pod_vcore_origin(cfg, pod_id), \
                           (cfg)->pod_shape)

        /**
         * Start iteration over a pod's dram banks
         */
        static inline hb_mc_coordinate_t
        hb_mc_config_pod_dram_start(const hb_mc_config_t *cfg, hb_mc_coordinate_t pod_id)
        {
                // subtract a row from the origin vcore of the pod
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod_id);
                return hb_mc_coordinate( og.x, og.y - 1 );
        }

        /**
         * Stop iteration over a pod's dram banks
         */
        static inline int
        hb_mc_config_pod_dram_stop(const hb_mc_config_t *cfg,
                                   hb_mc_coordinate_t pod_id,
                                   hb_mc_coordinate_t pos)
        {
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod_id);
                return  pos.x >= (og.x + cfg->pod_shape.x) ||
                        pos.y >= (og.y + cfg->pod_shape.y + 1);
        }

        /**
         * Continue iteration over a pod's dram banks
         */
        static inline hb_mc_coordinate_t
        hb_mc_config_pod_dram_next(const hb_mc_config_t *cfg,
                                       hb_mc_coordinate_t pod_id,
                                       hb_mc_coordinate_t pos)
        {
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod_id);
                hb_mc_idx_t north_y;
                north_y = og.y - 1;

                ++pos.x;
                if (pos.x >= og.x + cfg->pod_shape.x &&
                    pos.y == north_y) {
                        pos.x = og.x;
                        pos.y = og.y + cfg->pod_shape.y;
                }
                return pos;
        }

        /**
         * Iterates over a pod's DRAM banks
         */
#define hb_mc_config_pod_foreach_dram(coord, pod_id, cfg)               \
        for (coord = hb_mc_config_pod_dram_start(cfg, pod_id);          \
             !hb_mc_config_pod_dram_stop(cfg, pod_id, coord);           \
             coord = hb_mc_config_pod_dram_next(cfg, pod_id, coord))

        /****************************************/
        /* Network pod and tile from coordinate */
        /****************************************/
        static inline hb_mc_coordinate_t
        hb_mc_config_npod(const hb_mc_config_t *cfg,
                          hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t pm = hb_mc_config_pod_coord_mask(cfg);
                hb_mc_coordinate_t s  = hb_mc_config_tile_coord_width(cfg);
                return hb_mc_coordinate((pm.x & co.x) >> s.x,
                                        (pm.y & co.y) >> s.y);
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_tile_in_pod(const hb_mc_config_t *cfg,
                                 hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t tm = hb_mc_config_tile_coord_mask(cfg);
                return hb_mc_coordinate(tm.x & co.x, tm.y & co.y);
        }

        /********************************/
        /* Query what the coordinate is */
        /********************************/
        static inline int
        hb_mc_config_is_dram_north(const hb_mc_config_t *cfg,
                                   hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t tile = hb_mc_config_tile_in_pod(cfg, co);
                hb_mc_coordinate_t npod = hb_mc_config_npod(cfg, co);
                hb_mc_coordinate_t tm = hb_mc_config_tile_coord_mask(cfg);
                // dram npods are at even coordinates
                return (tile.y == tm.y) && (npod.y % 2 == 0);
        }

        static inline int
        hb_mc_config_is_dram_south(const hb_mc_config_t *cfg,
                                   hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t tile = hb_mc_config_tile_in_pod(cfg, co);
                hb_mc_coordinate_t npod = hb_mc_config_npod(cfg, co);
                // dram npods are at even coordinates
                // the northern most row is reserved for host
                return (tile.y == 0) && (npod.y % 2 == 0) && (co.y != cfg->host_interface.y);
        }

        static inline int
        hb_mc_config_is_dram(const hb_mc_config_t *cfg,
                             hb_mc_coordinate_t co)
        {
                return hb_mc_config_is_dram_north(cfg, co) || hb_mc_config_is_dram_south(cfg, co);
        }

        static inline int
        hb_mc_config_is_vanilla_core(const hb_mc_config_t *cfg,
                                     hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t npod = hb_mc_config_npod(cfg, co);
                // odd npod rows are vanilla cores
                return npod.y % 2 == 1;
        }

        static inline int
        hb_mc_config_is_host(const hb_mc_config_t *cfg,
                             hb_mc_coordinate_t co)
        {
                return  co.x == cfg->host_interface.x &&
                        co.y == cfg->host_interface.y;
        }

        /*************************************************/
        /* Calculate the "logical" pod of the coordinate */
        /*************************************************/
        static inline
        hb_mc_coordinate_t
        hb_mc_config_pod(const hb_mc_config_t *cfg,
                         hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t npod = hb_mc_config_npod(cfg, co);
                if (hb_mc_config_is_vanilla_core(cfg, co)) {
                        return hb_mc_coordinate(npod.x-1, npod.y >> 1);
                } else if (hb_mc_config_is_dram_south(cfg, co)) {
                        return hb_mc_coordinate(npod.x-1, (npod.y >> 1)-1);
                } else if (hb_mc_config_is_dram_north(cfg, co)) {
                        return hb_mc_coordinate(npod.x-1, (npod.y >> 1));
                }

                return hb_mc_coordinate(0,0);
        }

        static inline hb_mc_idx_t
        hb_mc_config_pod_dram_id(const hb_mc_config_t *cfg,
                                 hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t tile = hb_mc_config_tile_in_pod(cfg, co);
                if (hb_mc_config_is_dram_north(cfg, co)) {
                        return tile.x;
                // Default to south DRAM pod. If hashing function becomes
                //   more complex, check for south DRAM specifically.
                //} else if (hb_mc_config_is_dram_south(cfg, co)) {
                } else {
                        return  tile.x
                                + cfg->pod_shape.x;
                }
        }

        static inline hb_mc_idx_t
        hb_mc_config_dram_id(const hb_mc_config_t *cfg,
                             hb_mc_coordinate_t co)
        {
                hb_mc_coordinate_t pod  = hb_mc_config_pod(cfg, co);
                hb_mc_idx_t pod_id = hb_mc_coordinate_to_index(pod, cfg->pods);
                hb_mc_idx_t pod_ndrams = cfg->pod_shape.x * 2;
                return pod_id * pod_ndrams + hb_mc_config_pod_dram_id(cfg, co);
        }

        static inline hb_mc_idx_t
        hb_mc_config_pod_dram_x(const hb_mc_config_t *cfg,
                              hb_mc_coordinate_t pod,
                              hb_mc_idx_t pod_dram_id)
        {
                hb_mc_idx_t x = pod_dram_id % cfg->pod_shape.x;
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod);
                return  og.x + x;
        }

        static inline hb_mc_idx_t
        hb_mc_config_pod_dram_y(const hb_mc_config_t *cfg,
                              hb_mc_coordinate_t pod,
                              hb_mc_idx_t pod_dram_id)
        {
                int north = (pod_dram_id / cfg->pod_shape.x);
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod);
                return og.y + (north ? cfg->pod_shape.y : -1);;
        }

        static inline hb_mc_idx_t
        hb_mc_config_pod_dram_north_y(const hb_mc_config_t *cfg,
                              hb_mc_coordinate_t pod)
        {
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod);
                return og.y - 1;
        }

        static inline hb_mc_idx_t
        hb_mc_config_pod_dram_south_y(const hb_mc_config_t *cfg,
                                      hb_mc_coordinate_t pod)
        {
                hb_mc_coordinate_t og = hb_mc_config_pod_vcore_origin(cfg, pod);
                return og.y + cfg->pod_shape.y;
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_pod_dram(const hb_mc_config_t *cfg,
                              hb_mc_coordinate_t pod,
                              hb_mc_idx_t pod_dram_id)
        {
                return hb_mc_coordinate(hb_mc_config_pod_dram_x(cfg, pod, pod_dram_id),
                                        hb_mc_config_pod_dram_y(cfg, pod, pod_dram_id));
        }

        static inline hb_mc_coordinate_t
        hb_mc_config_dram(const hb_mc_config_t *cfg,
                          hb_mc_idx_t dram_id)
        {
                hb_mc_idx_t pod_ndrams = cfg->pod_shape.x * 2;
                hb_mc_idx_t pod_id = dram_id / pod_ndrams;
                hb_mc_coordinate_t pod = hb_mc_index_to_coordinate(pod_id, cfg->pods);
                hb_mc_idx_t pod_dram_id = dram_id % pod_ndrams;
                return hb_mc_config_pod_dram(cfg, pod, pod_dram_id);
        }

#ifdef __cplusplus
}
#endif
#endif
