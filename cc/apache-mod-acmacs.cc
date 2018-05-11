// -*- C++ -*-

#include <iostream>
#include <string>

#include "apache2/httpd.h"
#include "apache2/http_core.h"
#include "apache2/http_protocol.h"
#include "apache2/http_request.h"

#include "acmacs-base/read-file.hh"

// #include "apr_hooks.h"

static void register_hooks(apr_pool_t *pool);
static int acmacs_handler(request_rec *r);

extern "C" module acmacs_module;

module AP_MODULE_DECLARE_DATA acmacs_module = {
    STANDARD20_MODULE_STUFF, nullptr, nullptr, nullptr, nullptr, nullptr, register_hooks
};

static void register_hooks(apr_pool_t * /*pool*/) {
    ap_hook_handler(acmacs_handler, nullptr, nullptr, APR_HOOK_LAST);
}

static int acmacs_handler(request_rec *r) {
    // std::cerr << "acmacs_handler handler " << r->handler << '\n';
    if (!r->handler || r->handler != std::string("acmacs"))
        return (DECLINED);

    const std::string data = acmacs::file::read(r->filename);

    ap_set_content_type(r, "application/json");
    ap_rprintf(r, "{N: \"Hello, world! filename:[%s] args:[%s]\"}\n\n", r->filename, r->args);
      // ap_rputs(data.c_str(), r);
    return OK;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
