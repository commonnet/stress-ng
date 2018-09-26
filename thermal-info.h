/*
 * Copyright (C) 2018 Common Networks Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __STRESS_NG_THERMAL_INFO_BUFFER_H__
#define __STRESS_NG_THERMAL_INFO_BUFFER_H__

#include "stress-ng.h"

#if defined(__linux__)
#define STRESS_THERMAL_ZONES (1)
#define STRESS_THERMAL_ZONES_MAX (31) /* best if prime */
#endif

#ifdef STRESS_THERMAL_ZONES

#define DATA_POINTS_PER_TZ 64

/* per stressor thermal zone info */
typedef struct tz_info {
	char           *path; /* thermal zone path */
	char           *type; /* thermal zone type */
	size_t         index; /* thermal zone # index */
	struct tz_info *next; /* next thermal zone in list */
} tz_info_t;


// A set of statistics taken at a single point in time.
typedef struct {
	uint64_t temperature; /* temperature in Celsius * 1000 */
} tz_stat_t;


// A buffer storing the most recent statistics accumulated over the course of a
// test for a specific thermal zone.
typedef struct {
	tz_stat_t arr[DATA_POINTS_PER_TZ];
	size_t head;
	size_t len;
} tz_stat_buf_t;

int tz_stat_buf_init(tz_stat_buf_t* buf);
void tz_stat_buf_push(tz_stat_buf_t* buf, tz_stat_t datum);
uint64_t tz_stat_buf_avg_temp(tz_stat_buf_t const * buf);
uint64_t tz_stat_buf_max_temp(tz_stat_buf_t const * buf);


// An array to hold all of the accumulated statistics for all thermal zones.
typedef struct {
	tz_stat_buf_t tz_stat[STRESS_THERMAL_ZONES_MAX];
} stress_tz_t;

int tz_init(tz_info_t **tz_info_list);
void tz_free(tz_info_t **tz_info_list);
int tz_get_temperatures(tz_info_t **tz_info_list, stress_tz_t *tz);

#endif // STRESS_THERMAL_ZONES
#endif // __STRESS_NG_THERMAL_INFO_BUFFER_H__
