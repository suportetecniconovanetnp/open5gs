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

#include "test-common.h"
#include "test-eir-fd-path.h"

static void eir_enabled_attach_whitelist_flow(abts_case *tc, void *data)
{
    int rv;
    ogs_socknode_t *s1ap = NULL;
    ogs_pkbuf_t *emmbuf = NULL;
    ogs_pkbuf_t *esmbuf = NULL;
    ogs_pkbuf_t *sendbuf = NULL;
    ogs_pkbuf_t *recvbuf = NULL;
    ogs_nas_5gs_mobile_identity_suci_t mobile_identity_suci;
    test_ue_t *test_ue = NULL;
    test_sess_t *sess = NULL;
    bson_t *doc = NULL;

    memset(&mobile_identity_suci, 0, sizeof(mobile_identity_suci));
    mobile_identity_suci.h.supi_format = OGS_NAS_5GS_SUPI_FORMAT_IMSI;
    mobile_identity_suci.h.type = OGS_NAS_5GS_MOBILE_IDENTITY_SUCI;
    mobile_identity_suci.routing_indicator1 = 0;
    mobile_identity_suci.routing_indicator2 = 0xf;
    mobile_identity_suci.routing_indicator3 = 0xf;
    mobile_identity_suci.routing_indicator4 = 0xf;
    mobile_identity_suci.protection_scheme_id = OGS_PROTECTION_SCHEME_NULL;
    mobile_identity_suci.home_network_pki_value = 0;

    test_ue = test_ue_add_by_suci(&mobile_identity_suci, "3746000007");
    ABTS_PTR_NOTNULL(tc, test_ue);

    test_ue->e_cgi.cell_id = 0x1079baf0;
    test_ue->nas.ksi = OGS_NAS_KSI_NO_KEY_IS_AVAILABLE;
    test_ue->nas.value = OGS_NAS_ATTACH_TYPE_COMBINED_EPS_IMSI_ATTACH;
    test_ue->k_string = "465b5ce8b199b49faa5f0a2ee238a6bc";
    test_ue->opc_string = "e8ed289deba952e4283b54e88e6183ca";

    sess = test_sess_add_by_apn(test_ue, "internet", OGS_GTP2_RAT_TYPE_EUTRAN);
    ABTS_PTR_NOTNULL(tc, sess);

    s1ap = tests1ap_client(AF_INET);
    ABTS_PTR_NOTNULL(tc, s1ap);

    sendbuf = test_s1ap_build_s1_setup_request(
            S1AP_ENB_ID_PR_macroENB_ID, 0x54f64);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(NULL, recvbuf);

    doc = test_db_new_simple(test_ue);
    ABTS_PTR_NOTNULL(tc, doc);
    ABTS_INT_EQUAL(tc, OGS_OK, test_db_insert_ue(test_ue, doc));

    memset(&sess->pdn_connectivity_param, 0, sizeof(sess->pdn_connectivity_param));
    sess->pdn_connectivity_param.eit = 1;
    sess->pdn_connectivity_param.request_type = OGS_NAS_EPS_REQUEST_TYPE_INITIAL;
    esmbuf = testesm_build_pdn_connectivity_request(
            sess, false, OGS_NAS_EPS_PDN_TYPE_IPV4V6);
    ABTS_PTR_NOTNULL(tc, esmbuf);

    memset(&test_ue->attach_request_param, 0, sizeof(test_ue->attach_request_param));
    test_ue->attach_request_param.ms_network_capability = 1;
    emmbuf = testemm_build_attach_request(test_ue, esmbuf, true, false);
    ABTS_PTR_NOTNULL(tc, emmbuf);

    sendbuf = test_s1ap_build_initial_ue_message(
            test_ue, emmbuf, S1AP_RRC_Establishment_Cause_mo_Signalling, false);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap); /* Authentication Request */
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    emmbuf = testemm_build_authentication_response(test_ue);
    ABTS_PTR_NOTNULL(tc, emmbuf);
    sendbuf = test_s1ap_build_uplink_nas_transport(test_ue, emmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap); /* Security Mode Command */
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    test_eir_set_equipment_status(OGS_DIAM_S13_EQUIPMENT_WHITELIST);

    test_ue->mobile_identity_imeisv_presence = true;
    emmbuf = testemm_build_security_mode_complete(test_ue);
    ABTS_PTR_NOTNULL(tc, emmbuf);
    sendbuf = test_s1ap_build_uplink_nas_transport(test_ue, emmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    ABTS_INT_EQUAL(tc, OGS_NAS_EPS_ESM_INFORMATION_REQUEST,
            test_ue->esm_message_type);

    ABTS_INT_EQUAL(tc, OGS_OK, test_db_remove_ue(test_ue));
    testenb_s1ap_close(s1ap);
    test_ue_remove(test_ue);
}

static void eir_enabled_attach_blacklist_reject_flow(abts_case *tc, void *data)
{
    int rv;
    ogs_socknode_t *s1ap = NULL;
    ogs_pkbuf_t *emmbuf = NULL;
    ogs_pkbuf_t *esmbuf = NULL;
    ogs_pkbuf_t *sendbuf = NULL;
    ogs_pkbuf_t *recvbuf = NULL;
    ogs_nas_5gs_mobile_identity_suci_t mobile_identity_suci;
    test_ue_t *test_ue = NULL;
    test_sess_t *sess = NULL;
    bson_t *doc = NULL;

    memset(&mobile_identity_suci, 0, sizeof(mobile_identity_suci));
    mobile_identity_suci.h.supi_format = OGS_NAS_5GS_SUPI_FORMAT_IMSI;
    mobile_identity_suci.h.type = OGS_NAS_5GS_MOBILE_IDENTITY_SUCI;
    mobile_identity_suci.routing_indicator1 = 0;
    mobile_identity_suci.routing_indicator2 = 0xf;
    mobile_identity_suci.routing_indicator3 = 0xf;
    mobile_identity_suci.routing_indicator4 = 0xf;
    mobile_identity_suci.protection_scheme_id = OGS_PROTECTION_SCHEME_NULL;
    mobile_identity_suci.home_network_pki_value = 0;

    test_ue = test_ue_add_by_suci(&mobile_identity_suci, "3746000006");
    ABTS_PTR_NOTNULL(tc, test_ue);

    test_ue->e_cgi.cell_id = 0x1079baf0;
    test_ue->nas.ksi = OGS_NAS_KSI_NO_KEY_IS_AVAILABLE;
    test_ue->nas.value = OGS_NAS_ATTACH_TYPE_COMBINED_EPS_IMSI_ATTACH;
    test_ue->k_string = "465b5ce8b199b49faa5f0a2ee238a6bc";
    test_ue->opc_string = "e8ed289deba952e4283b54e88e6183ca";

    sess = test_sess_add_by_apn(test_ue, "internet", OGS_GTP2_RAT_TYPE_EUTRAN);
    ABTS_PTR_NOTNULL(tc, sess);

    s1ap = tests1ap_client(AF_INET);
    ABTS_PTR_NOTNULL(tc, s1ap);

    sendbuf = test_s1ap_build_s1_setup_request(
            S1AP_ENB_ID_PR_macroENB_ID, 0x54f64);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(NULL, recvbuf);

    doc = test_db_new_simple(test_ue);
    ABTS_PTR_NOTNULL(tc, doc);
    ABTS_INT_EQUAL(tc, OGS_OK, test_db_insert_ue(test_ue, doc));

    memset(&sess->pdn_connectivity_param, 0, sizeof(sess->pdn_connectivity_param));
    sess->pdn_connectivity_param.eit = 1;
    sess->pdn_connectivity_param.request_type = OGS_NAS_EPS_REQUEST_TYPE_INITIAL;
    esmbuf = testesm_build_pdn_connectivity_request(
            sess, false, OGS_NAS_EPS_PDN_TYPE_IPV4V6);
    ABTS_PTR_NOTNULL(tc, esmbuf);

    memset(&test_ue->attach_request_param, 0, sizeof(test_ue->attach_request_param));
    test_ue->attach_request_param.ms_network_capability = 1;
    emmbuf = testemm_build_attach_request(test_ue, esmbuf, true, false);
    ABTS_PTR_NOTNULL(tc, emmbuf);

    sendbuf = test_s1ap_build_initial_ue_message(
            test_ue, emmbuf, S1AP_RRC_Establishment_Cause_mo_Signalling, false);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap); /* Authentication Request */
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    emmbuf = testemm_build_authentication_response(test_ue);
    ABTS_PTR_NOTNULL(tc, emmbuf);
    sendbuf = test_s1ap_build_uplink_nas_transport(test_ue, emmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap); /* Security Mode Command */
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    test_eir_set_equipment_status(OGS_DIAM_S13_EQUIPMENT_BLACKLIST);

    test_ue->mobile_identity_imeisv_presence = true;
    emmbuf = testemm_build_security_mode_complete(test_ue);
    ABTS_PTR_NOTNULL(tc, emmbuf);
    sendbuf = test_s1ap_build_uplink_nas_transport(test_ue, emmbuf);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    recvbuf = testenb_s1ap_read(s1ap);
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    ABTS_INT_EQUAL(tc, OGS_NAS_EPS_ATTACH_REJECT, test_ue->emm_message_type);

    recvbuf = testenb_s1ap_read(s1ap); /* UE Context Release Command */
    ABTS_PTR_NOTNULL(tc, recvbuf);
    tests1ap_recv(test_ue, recvbuf);

    sendbuf = test_s1ap_build_ue_context_release_complete(test_ue);
    ABTS_PTR_NOTNULL(tc, sendbuf);
    rv = testenb_s1ap_send(s1ap, sendbuf);
    ABTS_INT_EQUAL(tc, OGS_OK, rv);

    ABTS_INT_EQUAL(tc, OGS_OK, test_db_remove_ue(test_ue));
    testenb_s1ap_close(s1ap);
    test_ue_remove(test_ue);
}

abts_suite *test_eir_enabled(abts_suite *suite)
{
    suite = ADD_SUITE(suite);

    test_eir_set_equipment_status(OGS_DIAM_S13_EQUIPMENT_WHITELIST);
    abts_run_test(suite, eir_enabled_attach_whitelist_flow, NULL);

    test_eir_set_equipment_status(OGS_DIAM_S13_EQUIPMENT_BLACKLIST);
    abts_run_test(suite, eir_enabled_attach_blacklist_reject_flow, NULL);

    return suite;
}
