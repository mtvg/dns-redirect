/*
 * Copyright (c) 2018 MTVG
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"
#include "user_interface.h"

static void ev_handler(struct mg_connection *c, int ev, void *ev_data,
                       void *user_data) {
  struct mg_dns_message *msg = (struct mg_dns_message *) ev_data;
  struct mbuf reply_buf;
  int i;

  if (ev != MG_DNS_MESSAGE) return;

  mbuf_init(&reply_buf, 512);
  struct mg_dns_reply reply = mg_dns_create_reply(&reply_buf, msg);
  for (i = 0; i < msg->num_questions; i++) {
    char rname[256];
    struct mg_dns_resource_record *rr = &msg->questions[i];
    mg_dns_uncompress_name(msg, &rr->name, rname, sizeof(rname) - 1);
    fprintf(stdout, "Q type %d name %s\n", rr->rtype, rname);
    if (rr->rtype == MG_DNS_A_RECORD) {
      uint32_t ip = inet_addr(mgos_sys_config_get_dns_redirect());
      mg_dns_reply_record(&reply, rr, NULL, rr->rtype, 10, &ip, 4);
    }
  }
  mg_dns_send_reply(c, &reply);
  mbuf_free(&reply_buf);
  (void) user_data;
}

bool mgos_dns_redirect_init(void) {
  if (strlen(mgos_sys_config_get_dns_redirect()) > 0) {
    struct mg_connection *c = mg_bind(mgos_get_mgr(), "udp://:53", ev_handler, 0);
    mg_set_protocol_dns(c);
  }
    
  uint8 on = 1;
  wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &on);
  return true;
}