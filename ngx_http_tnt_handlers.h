/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Copyright (C) 2015-2016 Tarantool AUTHORS:
 * please see AUTHORS file.
 */

#ifndef NGX_HTTP_TNT_CREATE_REQUEST_H_INCLUDED
#define NGX_HTTP_TNT_CREATE_REQUEST_H_INCLUDED

#include <ngx_core.h>
#include <ngx_http.h>
#include <tp_transcode.h>

typedef enum ngx_tnt_conf_states {
    NGX_TNT_CONF_ON = 0,
    NGX_TNT_CONF_OFF = 1
} ngx_tnt_conf_states_e;

typedef struct {
    ngx_http_upstream_conf_t upstream;
    ngx_int_t                index;

    size_t                   in_multiplier;
    size_t                   out_multiplier;

    ngx_str_t                method;
    size_t                   pass_http_request_buffer_size;
    ngx_uint_t               pass_http_request;
} ngx_http_tnt_loc_conf_t;

enum ctx_state {
    OK = 0,

    INPUT_JSON_PARSE_FAILED,
    INPUT_TO_LARGE,
    INPUT_EMPTY,

    READ_PAYLOAD,
    READ_BODY,
    SEND_REPLY
};

typedef struct ngx_http_tnt_ctx {

    struct {
        u_char mem[6];
        u_char *p, *e;
    } payload;

    enum ctx_state     state;
    ngx_buf_t          *in_err, *tp_cache;
    ssize_t            rest, payload_size;
    int                rest_batch_size, batch_size;
    ngx_int_t          greeting:1;

} ngx_http_tnt_ctx_t;

ngx_http_tnt_ctx_t * ngx_http_tnt_create_ctx(ngx_http_request_t *r);
void ngx_http_tnt_reset_ctx(ngx_http_tnt_ctx_t *ctx);

void ngx_http_tnt_set_handlers(ngx_http_request_t *r,
                               ngx_http_upstream_t *u,
                               ngx_http_tnt_loc_conf_t *tlcf);

/** create tarantool requests handlers [
 */
ngx_int_t ngx_http_tnt_body_json_handler(ngx_http_request_t *r);
ngx_int_t ngx_http_tnt_query_handler(ngx_http_request_t *r);
/* ] */

ngx_int_t ngx_http_tnt_reinit_request(ngx_http_request_t *r);
ngx_int_t ngx_http_tnt_process_header(ngx_http_request_t *r);
void ngx_http_tnt_abort_request(ngx_http_request_t *r);
void ngx_http_tnt_finalize_request(ngx_http_request_t *r, ngx_int_t rc);

ngx_buf_t* ngx_http_tnt_set_err(ngx_http_request_t *r,
                                int errcode,
                                const u_char *msg,
                                size_t msglen);

/**
 */
typedef struct ngx_http_tnt_error {
    const ngx_str_t msg;
    int code;
} ngx_http_tnt_error_t;

enum ngx_http_tnt_err_messages_idx {
    REQUEST_TOO_LARGE   = 0,
    UNKNOWN_PARSE_ERROR = 1,
    HTTP_REQUEST_TOO_LARGE = 2
};

const ngx_http_tnt_error_t *get_error_text(int type);

static inline size_t
ngx_http_tnt_overhead(void)
{
    return sizeof("[{"
        "'error': {"
            "'code':-XXXXX,"
            "'message':''"
        "},"
        "[['result':{},"
        "'id':4294967295]]"
    "}");
}

#endif