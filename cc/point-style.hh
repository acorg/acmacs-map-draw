#pragma once

#include "acmacs-base/string-compare.hh"
#include "acmacs-base/point-style.hh"
#include "seqdb-3/sequence.hh"

// ----------------------------------------------------------------------

namespace rjson::v3
{
    class value;
};

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

    using modifier_or_passage_t = std::variant<acmacs::color::Modifier, passage_color_t, const rjson::v3::value*>; // value* is to support extensions

    struct point_style_t
    {
        acmacs::PointStyleModified style;
        std::optional<passage_color_t> passage_fill;
        std::optional<passage_color_t> passage_outline;

        void fill(const modifier_or_passage_t& mod)
            {
                std::visit(
                    [this]<typename Modifier>(const Modifier& modifier) {
                        if constexpr (std::is_same_v<Modifier, acmacs::color::Modifier>)
                            style.fill(modifier);
                        else if constexpr (std::is_same_v<Modifier, passage_color_t>)
                            passage_fill = modifier;
                        // const rjson::v3::value* variant not handled (extension)
                    },
                    mod);
            }

        void outline(const modifier_or_passage_t& mod)
            {
                std::visit(
                    [this]<typename Modifier>(const Modifier& modifier) {
                        if constexpr (std::is_same_v<Modifier, acmacs::color::Modifier>)
                            style.outline(modifier);
                        else if constexpr (std::is_same_v<Modifier, passage_color_t>)
                            passage_outline = modifier;
                        // const rjson::v3::value* variant not handled (extension)
                    },
                    mod);
            }
    };

    inline modifier_or_passage_t make_modifier_or_passage(std::string_view source)
    {
        if (source.empty())
            return nullptr;
        constexpr std::string_view passage_key{"passage"};
        if (acmacs::string::startswith(source, passage_key)) {
            passage_color_t colors;
            colors.init_passage_colors();
            if (source.size() > passage_key.size())
                colors.apply(acmacs::color::Modifier{source.substr(passage_key.size())});
            return colors;
        }
        else
            return acmacs::color::Modifier{source};
    }

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
