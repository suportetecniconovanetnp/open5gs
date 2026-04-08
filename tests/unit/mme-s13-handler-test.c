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

#include "ogs-core.h"
#include "ogs-diameter-s13.h"
#include "ogs-nas-eps.h"
#include "core/abts.h"

/* Exposed by mme-s13-handler.h for tests */
extern uint8_t validate_s13_message(ogs_diam_s13_message_t *s13_message);
extern uint8_t validate_eca(
        ogs_diam_s13_eca_message_t eca_message, ogs_nas_eir_t eir_config);

static void test_validate_eca_whitelist_allowed(abts_case *tc, void *data)
{
    ogs_nas_eir_t eir = { 0 };
    ogs_diam_s13_eca_message_t eca = { 0 };

    eir.allow_whitelist = true;
    eca.equipment_status_code = OGS_DIAM_S13_EQUIPMENT_WHITELIST;

    ABTS_INT_EQUAL(tc, OGS_NAS_EMM_CAUSE_REQUEST_ACCEPTED,
            validate_eca(eca, eir));
}

static void test_validate_eca_blacklist_rejected(abts_case *tc, void *data)
{
    ogs_nas_eir_t eir = { 0 };
    ogs_diam_s13_eca_message_t eca = { 0 };

    eir.allow_whitelist = true;
    eir.allow_greylist = true;
    eir.allow_blacklist = false;
    eca.equipment_status_code = OGS_DIAM_S13_EQUIPMENT_BLACKLIST;

    ABTS_INT_EQUAL(tc, OGS_NAS_EMM_CAUSE_ILLEGAL_ME,
            validate_eca(eca, eir));
}

static void test_validate_eca_greylist_allowed(abts_case *tc, void *data)
{
    ogs_nas_eir_t eir = { 0 };
    ogs_diam_s13_eca_message_t eca = { 0 };

    eir.allow_greylist = true;
    eca.equipment_status_code = OGS_DIAM_S13_EQUIPMENT_GREYLIST;

    ABTS_INT_EQUAL(tc, OGS_NAS_EMM_CAUSE_REQUEST_ACCEPTED,
            validate_eca(eca, eir));
}

static void test_validate_s13_message_success(abts_case *tc, void *data)
{
    ogs_diam_s13_message_t msg = { 0 };

    msg.result_code = ER_DIAMETER_SUCCESS;

    ABTS_INT_EQUAL(tc, OGS_OK, validate_s13_message(&msg));
}

static void test_validate_s13_message_diameter_error(abts_case *tc, void *data)
{
    ogs_diam_s13_message_t msg = { 0 };
    uint32_t err = ER_DIAMETER_UNABLE_TO_COMPLY;

    msg.result_code = ER_DIAMETER_UNABLE_TO_COMPLY;
    msg.err = &err;
    msg.exp_err = NULL;

    ABTS_INT_EQUAL(tc, OGS_NAS_EMM_CAUSE_NETWORK_FAILURE,
            validate_s13_message(&msg));
}

static void test_validate_s13_message_exp_error(abts_case *tc, void *data)
{
    ogs_diam_s13_message_t msg = { 0 };
    uint32_t exp_err = OGS_DIAM_S13_ERROR_UNKNOWN_EPS_SUBSCRIPTION;

    msg.result_code = exp_err;
    msg.err = NULL;
    msg.exp_err = &exp_err;

    ABTS_INT_EQUAL(tc, OGS_NAS_EMM_CAUSE_NO_SUITABLE_CELLS_IN_TRACKING_AREA,
            validate_s13_message(&msg));
}

abts_suite *test_mme_s13_handler(abts_suite *suite)
{
    suite = ADD_SUITE(suite);

    abts_run_test(suite, test_validate_eca_whitelist_allowed, NULL);
    abts_run_test(suite, test_validate_eca_blacklist_rejected, NULL);
    abts_run_test(suite, test_validate_eca_greylist_allowed, NULL);
    abts_run_test(suite, test_validate_s13_message_success, NULL);
    abts_run_test(suite, test_validate_s13_message_diameter_error, NULL);
    abts_run_test(suite, test_validate_s13_message_exp_error, NULL);

    return suite;
}
