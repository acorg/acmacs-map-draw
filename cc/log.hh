#pragma once

#include "seqdb-3/log.hh"

// ----------------------------------------------------------------------

namespace acmacs::log
{
    // enum {
    // };

    inline void register_enabler_acmacs_map_draw()
    {
        register_enabler_seqdb3();
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
