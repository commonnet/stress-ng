/*
 * Copyright (C) 2013-2018 Canonical, Ltd.
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
 *
 * This code is a complete clean re-write of the stress tool by
 * Colin Ian King <colin.king@canonical.com> and attempts to be
 * backwardly compatible with the stress tool by Amos Waterland
 * <apw@rossby.metr.ou.edu> but has more stress tests and more
 * functionality.
 *
 */
#include "stress-ng.h"

#if defined(STRESS_THERMAL_ZONES)
/*
 *  tz_init()
 *	gather all thermal zones
 */
int tz_init(tz_info_t **tz_info_list)
{
	DIR *dir;
	struct dirent *entry;
	size_t i = 0;

	dir = opendir("/sys/class/thermal");
	if (!dir)
		return 0;

	while ((entry = readdir(dir)) != NULL) {
		char path[PATH_MAX];
		FILE *fp;
		tz_info_t *tz_info;

		/* Ignore non TZ interfaces */
		if (strncmp(entry->d_name, "thermal_zone", 12))
			continue;

		/* Ensure we don't overstep the max limit of TZs */
		if (i >= STRESS_THERMAL_ZONES_MAX)
			break;

		if ((tz_info = calloc(1, sizeof(*tz_info))) == NULL) {
			pr_err("Cannot allocate thermal information.\n");
			(void)closedir(dir);
			return -1;
		}
		(void)snprintf(path, sizeof(path),
			"/sys/class/thermal/%s/type",
			entry->d_name);

		tz_info->path = strdup(entry->d_name);
		if (!tz_info->path) {
			free(tz_info);
			(void)closedir(dir);
			return -1;
		}
		tz_info->type = NULL;
		if ((fp = fopen(path, "r")) != NULL) {
			char type[128];

			if (fgets(type, sizeof(type), fp) != NULL) {
				type[strcspn(type, "\n")] = '\0';
				tz_info->type  = strdup(type);
			}
			(void)fclose(fp);
		}
		if (!tz_info->type) {
			free(tz_info->path);
			free(tz_info);
			(void)closedir(dir);
			return -1;
		}
		tz_info->index = i++;
		tz_info->next = *tz_info_list;
		*tz_info_list = tz_info;
	}

	(void)closedir(dir);
	return 0;
}

/*
 *  tz_free()
 *	free thermal zones
 */
void tz_free(tz_info_t **tz_info_list)
{
	tz_info_t *tz_info = *tz_info_list;

	while (tz_info) {
		tz_info_t *next = tz_info->next;

		free(tz_info->path);
		free(tz_info->type);
		free(tz_info);
		tz_info = next;
	}
}

/*
 *  tz_get_temperatures()
 *	collect valid thermal_zones details
 */
int tz_get_temperatures(tz_info_t **tz_info_list, stress_tz_t *tz)
{
	tz_info_t *tz_info;

	for (tz_info = *tz_info_list; tz_info; tz_info = tz_info->next) {
		char path[PATH_MAX];

		(void)snprintf(
			path, sizeof(path), "/sys/class/thermal/%s/temp", tz_info->path);

		FILE *fp = fopen(path, "r");
		if (fp == NULL) {
			continue;
		}
		tz_stat_t stat;
		if (fscanf(fp, "%" SCNu64, &stat.temperature) == 1) {
			tz_stat_buf_push(&tz->tz_stat[tz_info->index], stat);
		}
		(void)fclose(fp);
	}
	return 0;
}

int tz_stat_buf_init(tz_stat_buf_t* buf)
{
    buf->head = 0;
    buf->len = 0;
    return 0;
}

void tz_stat_buf_push(tz_stat_buf_t* buf, tz_stat_t datum)
{
    buf->arr[buf->head] = datum;
    buf->head = (buf->head + 1) % DATA_POINTS_PER_TZ;
    buf->len =
        buf->len == DATA_POINTS_PER_TZ ? DATA_POINTS_PER_TZ : (buf->len + 1);
}

uint64_t tz_stat_buf_avg_temp(tz_stat_buf_t const * buf)
{
    if (buf->len == 0) {
        return 0;
    }
    uint64_t total = 0;
    for (size_t i = 0; i != buf->len; ++i) {
        total += buf->arr[i].temperature;
    }
    return total / buf->len;
}

uint64_t tz_stat_buf_max_temp(tz_stat_buf_t const * buf)
{
    uint64_t max = 0;
    for (size_t i = 0; i != buf->len; ++i) {
        uint64_t const temp = buf->arr[i].temperature;
        if (temp > max) {
            max = temp;
        }
    }
    return max;
}

#endif
