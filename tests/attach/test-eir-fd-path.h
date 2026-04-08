/*
 * Copyright (C) 2026 by OpenAI
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TEST_EIR_FD_PATH_H
#define TEST_EIR_FD_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "diameter/s13/ogs-diameter-s13.h"

int test_eir_fd_init(void);
void test_eir_fd_final(void);
void test_eir_set_equipment_status(uint32_t status_code);

#ifdef __cplusplus
}
#endif

#endif /* TEST_EIR_FD_PATH_H */
