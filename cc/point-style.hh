#pragma once

#include "seqdb-3/sequence.hh"
#include "acmacs-base/point-style.hh"

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    struct passage_color_t
    {
        std::optional<acmacs::color::Modifier> egg;
        std::optional<acmacs::color::Modifier> reassortant;
        std::optional<acmacs::color::Modifier> cell;
        std::optional<acmacs::seqdb::pos1_t> pos;         // coloring by aa at pos
        std::vector<acmacs::color::Modifier> color_order; // coloring by aa at pos

        void init_passage_colors()
        {
            egg = acmacs::color::Modifier{Color{0xFF4040}};
            reassortant = acmacs::color::Modifier{Color{0xFFB040}};
            cell = acmacs::color::Modifier{Color{0x4040FF}};
        }

        void apply(const acmacs::color::Modifier& modifier)
        {
            if (egg)
                egg->add(modifier);
            if (reassortant)
                reassortant->add(modifier);
            if (cell)
                cell->add(modifier);
        }
    };

    struct point_style_t
    {
        acmacs::PointStyleModified style;
        std::optional<passage_color_t> passage_fill;
        std::optional<passage_color_t> passage_outline;
    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
