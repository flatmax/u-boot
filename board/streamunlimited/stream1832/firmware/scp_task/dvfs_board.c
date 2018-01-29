
/*
 * board/amlogic/txl_skt_v1/firmware/scp_task/dvfs_board.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * NOTE: Even though we do not use the scpi interface to set the voltages,
 * we still keep this table just so we can pass it to get_dvfs_info_board(),
 * otherwise the kernel will hang on boot. However the voltages are all set
 * to 0 because the voltage scaling is done by the cpufreq-dt driver.
 */
struct scpi_opp_entry cpu_dvfs_tbl[] = {
	DVFS( 100000000, 0),
	DVFS( 250000000, 0),
	DVFS( 500000000, 0),
	DVFS( 667000000, 0),
	DVFS(1000000000, 0),
	DVFS(1200000000, 0),
	DVFS(1296000000, 0),
	DVFS(1416000000, 0),
};

void set_dvfs(unsigned int domain, unsigned int index)
{
}

void get_dvfs_info_board(unsigned int domain,
		unsigned char *info_out, unsigned int *size_out)
{
	unsigned int cnt;
	cnt = ARRAY_SIZE(cpu_dvfs_tbl);

	buf_opp.latency = 200;
	buf_opp.count = cnt;

	memset(&buf_opp.opp[0], 0,
	       MAX_DVFS_OPPS * sizeof(struct scpi_opp_entry));

	memcpy(&buf_opp.opp[0], cpu_dvfs_tbl ,
		cnt * sizeof(struct scpi_opp_entry));

	memcpy(info_out, &buf_opp, sizeof(struct scpi_opp));
	*size_out = sizeof(struct scpi_opp);
	return;
}
