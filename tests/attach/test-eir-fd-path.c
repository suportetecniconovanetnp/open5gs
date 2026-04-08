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

static ogs_diam_config_t diam_config;

static struct disp_hdl *hdl_s13_fb = NULL;
static struct disp_hdl *hdl_s13_ecr = NULL;

static ogs_thread_mutex_t equipment_status_mutex;
static uint32_t equipment_status_code = OGS_DIAM_S13_EQUIPMENT_WHITELIST;

static void test_eir_diam_config(void)
{
    memset(&diam_config, 0, sizeof(ogs_diam_config_t));

    diam_config.cnf_diamid = "eir.localdomain";
    diam_config.cnf_diamrlm = "localdomain";
    diam_config.cnf_port = DIAMETER_PORT;
    diam_config.cnf_port_tls = DIAMETER_SECURE_PORT;
    diam_config.cnf_flags.no_sctp = 1;
    diam_config.cnf_flags.no_fwd = 1;
    diam_config.cnf_addr = "127.0.0.16";

    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dbg_msg_dumps.fdx";
    diam_config.ext[diam_config.num_of_ext].conf = "0x8888";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_rfc5777.fdx";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_mip6i.fdx";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_nasreq.fdx";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_nas_mipv6.fdx";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_dcca.fdx";
    diam_config.num_of_ext++;
    diam_config.ext[diam_config.num_of_ext].module =
        FD_EXT_DIR OGS_DIR_SEPARATOR_S "dict_dcca_3gpp" \
        OGS_DIR_SEPARATOR_S "dict_dcca_3gpp.fdx";
    diam_config.num_of_ext++;

    diam_config.conn[diam_config.num_of_conn].identity = "mme.localdomain";
    diam_config.conn[diam_config.num_of_conn].addr = "127.0.0.2";
    diam_config.num_of_conn++;
}

void test_eir_set_equipment_status(uint32_t status_code)
{
    ogs_thread_mutex_lock(&equipment_status_mutex);
    equipment_status_code = status_code;
    ogs_thread_mutex_unlock(&equipment_status_mutex);
}

static int test_s13_fb_cb(struct msg **msg, struct avp *avp,
        struct session *sess, void *opaque, enum disp_action *act)
{
    ogs_warn("Unexpected S13 message received");
    return ENOTSUP;
}

static int test_s13_ecr_cb(struct msg **msg, struct avp *avp,
        struct session *sess, void *opaque, enum disp_action *act)
{
    int ret;
    uint32_t status = OGS_DIAM_S13_EQUIPMENT_WHITELIST;
    struct msg *ans;
    union avp_value val;

    ogs_assert(msg);

    ret = fd_msg_new_answer_from_req(fd_g_config->cnf_dict, msg, 0);
    ogs_assert(ret == 0);
    ans = *msg;

    ret = fd_msg_rescode_set(ans, (char *)"DIAMETER_SUCCESS", NULL, NULL, 1);
    ogs_assert(ret == 0);

    ret = ogs_diam_message_vendor_specific_appid_set(
            ans, OGS_DIAM_S13_APPLICATION_ID);
    ogs_assert(ret == 0);

    ret = fd_msg_avp_new(ogs_diam_auth_session_state, 0, &avp);
    ogs_assert(ret == 0);
    val.i32 = OGS_DIAM_AUTH_SESSION_NO_STATE_MAINTAINED;
    ret = fd_msg_avp_setvalue(avp, &val);
    ogs_assert(ret == 0);
    ret = fd_msg_avp_add(ans, MSG_BRW_LAST_CHILD, avp);
    ogs_assert(ret == 0);

    ogs_thread_mutex_lock(&equipment_status_mutex);
    status = equipment_status_code;
    ogs_thread_mutex_unlock(&equipment_status_mutex);

    ret = fd_msg_avp_new(ogs_diam_s13_equipment_status, 0, &avp);
    ogs_assert(ret == 0);
    val.i32 = status;
    ret = fd_msg_avp_setvalue(avp, &val);
    ogs_assert(ret == 0);
    ret = fd_msg_avp_add(ans, MSG_BRW_LAST_CHILD, avp);
    ogs_assert(ret == 0);

    ret = fd_msg_send(msg, NULL, NULL);
    ogs_assert(ret == 0);

    ogs_assert(pthread_mutex_lock(&ogs_diam_stats_self()->stats_lock) == 0);
    ogs_diam_stats_self()->stats.nb_echoed++;
    ogs_assert(pthread_mutex_unlock(&ogs_diam_stats_self()->stats_lock) == 0);

    return 0;
}

static int test_eir_s13_init(void)
{
    int ret;
    struct disp_when data;

    ret = ogs_diam_s13_init();
    ogs_assert(ret == 0);

    memset(&data, 0, sizeof(data));
    data.app = ogs_diam_s13_application;

    ret = fd_disp_register(test_s13_fb_cb, DISP_HOW_APPID, &data, NULL,
                &hdl_s13_fb);
    ogs_assert(ret == 0);

    data.command = ogs_diam_s13_cmd_ecr;
    ret = fd_disp_register(test_s13_ecr_cb, DISP_HOW_CC, &data, NULL,
                &hdl_s13_ecr);
    ogs_assert(ret == 0);

    ret = fd_disp_app_support(ogs_diam_s13_application, ogs_diam_vendor, 1, 0);
    ogs_assert(ret == 0);

    return OGS_OK;
}

static void test_eir_s13_final(void)
{
    if (hdl_s13_fb)
        (void) fd_disp_unregister(&hdl_s13_fb, NULL);
    if (hdl_s13_ecr)
        (void) fd_disp_unregister(&hdl_s13_ecr, NULL);
}

int test_eir_fd_init(void)
{
    int ret;

    test_eir_diam_config();

    ogs_thread_mutex_init(&equipment_status_mutex);
    test_eir_set_equipment_status(OGS_DIAM_S13_EQUIPMENT_WHITELIST);

    ret = ogs_diam_init(FD_MODE_CLIENT, NULL, &diam_config);
    ogs_assert(ret == 0);

    ret = test_eir_s13_init();
    ogs_assert(ret == OGS_OK);

    ret = ogs_diam_start();
    ogs_assert(ret == 0);

    return OGS_OK;
}

void test_eir_fd_final(void)
{
    test_eir_s13_final();
    ogs_diam_final();
    ogs_thread_mutex_destroy(&equipment_status_mutex);
}
