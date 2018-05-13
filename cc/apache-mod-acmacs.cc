// -*- C++ -*-

#include <iostream>
#include <string>

#include "apache2/httpd.h"
#include "apache2/http_core.h"
#include "apache2/http_protocol.h"
#include "apache2/http_request.h"
#include "apache2/http_log.h"
#include "apache2/util_script.h"

#include "acmacs-base/read-file.hh"

// #include "apr_hooks.h"

extern "C" {
//    module acmacs_module;
//AP_DECLARE_MODULE(acmacs);
}

#define AP_LOG_DEBUG(rec, fmt, ...) ap_log_rerror(APLOG_MARK, APLOG_DEBUG,  0, rec, fmt, ##__VA_ARGS__)
#define AP_LOG_INFO(rec, fmt, ...)  ap_log_rerror(APLOG_MARK, APLOG_INFO,   0, rec, "[" HR_AUTH "] " fmt, ##__VA_ARGS__)
#define AP_LOG_WARN(rec, fmt, ...)  ap_log_rerror(APLOG_MARK, APLOG_WARNING,0, rec, "[" HR_AUTH "] " fmt, ##__VA_ARGS__)
#define AP_LOG_ERR(rec, fmt, ...)   ap_log_rerror(APLOG_MARK, APLOG_ERR,    0, rec, "[" HR_AUTH "] " fmt, ##__VA_ARGS__)

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);

// extern "C" module acmacs_module;

extern "C"  {
module AP_MODULE_DECLARE_DATA acmacs_module = {
    STANDARD20_MODULE_STUFF, nullptr, nullptr, nullptr, nullptr, nullptr, register_hooks
};
}

static void register_hooks(apr_pool_t * /*pool*/) {
    ap_hook_handler(acmacs_handler, nullptr, nullptr, APR_HOOK_LAST);
}

static int acmacs_handler_processed = 0;

static int acmacs_handler(request_rec *r) {
    // std::cerr << "acmacs_handler handler " << r->handler << '\n';
    if (!r->handler || r->handler != std::string("acmacs"))
        return DECLINED;

    apr_table_t *GET;
    ap_args_to_table(r, &GET);
    const auto acv = apr_table_get(GET, "acv");
    if (!acv)
        return DECLINED;

    ++acmacs_handler_processed;
    // AP_LOG_DEBUG(r, "acv: %s processed: %d", acv, acmacs_handler_processed);
    const std::string data = acmacs::file::read(r->filename);

    ap_set_content_type(r, "application/json");
    ap_rprintf(r, "{N: \"Hello, world! filename:[%s] args:[%s] acv:[%s]\", processed: %d}\n\n", r->filename, r->args, acv ? acv : "null", acmacs_handler_processed);
      // ap_rputs(data.c_str(), r);
    return OK;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
