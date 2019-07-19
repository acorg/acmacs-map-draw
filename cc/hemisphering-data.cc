#include "acmacs-base/in-json-parser.hh"
#include "acmacs-map-draw/hemisphering-data.hh"

// ----------------------------------------------------------------------

namespace local
{
    class hemi_point_entry_t : public in_json::stack_entry
    {
      public:
        hemi_point_entry_t(acmacs::hemi::v1::hemi_point_t& target) : target_{target} {}
        const char* injson_name() override { return "hemi_point_entry"; }

        void injson_put_integer(std::string_view data) override
        {
            if (key_ == "point_no")
                target_.point_no = data;
            else
                injson_put_real(data);
            reset_key();
        }

        void injson_put_real(std::string_view data) override
        {
            if (key_ == "distance")
                target_.distance = data;
            else if (key_ == "contribution_diff")
                target_.contribution_diff = data;
            else if (key_ == "pos")
                target_.pos.emplace_back(data);
            else
                in_json::stack_entry::injson_put_real(data);
            if (key_ != "pos")
                reset_key();
        }

        void injson_put_string(std::string_view data) override
        {
            if (key_ == "name")
                target_.name = data;
            else
                in_json::stack_entry::injson_put_string(data);
            reset_key();
        }

        void injson_put_array() override
        {
            if (key_ != "pos")
                in_json::stack_entry::injson_put_array();
        }

        void injson_pop_array() override
        {
            reset_key();
        }

      private:
        acmacs::hemi::v1::hemi_point_t& target_;
    };

    class source_data_t : public in_json::stack_entry
    {
      public:
        source_data_t(acmacs::hemi::v1::hemi_data_t& hemi_data) : hemi_data_{hemi_data} {}

        const char* injson_name() override { return "source_data"; }

        std::unique_ptr<in_json::stack_entry> injson_put_object() override
        {
            if (key_ == "hemisphering")
                return std::make_unique<hemi_point_entry_t>(hemi_data_.hemi_points.emplace_back());
            else if (key_ == "trapped")
                return std::make_unique<hemi_point_entry_t>(hemi_data_.trapped_points.emplace_back());
            else
                throw in_json::parse_error("unsupported object for key: ", key_);
        }

        void injson_put_string(std::string_view data) override
        {
            if (key_ == "  version" || key_ == " version") {
                if (data != "grid-test-v1")
                    throw in_json::parse_error("unsupported version: ", data);
            }
            else if (key_ == "_" || key_ == "chart")
                ;
            else
                throw in_json::parse_error("unsupported field: ", data);
            reset_key();
        }

        void injson_put_array() override
        {
            // fmt::print(stderr, "DEBUG: injson_put_array {}\n", key_);
        }

        void injson_pop_array() override { reset_key(); }

      private:
        acmacs::hemi::v1::hemi_data_t& hemi_data_;
    };


    using sink = in_json::object_sink<acmacs::hemi::v1::hemi_data_t, source_data_t>;

}

// ----------------------------------------------------------------------

acmacs::hemi::v1::hemi_data_t acmacs::hemi::v1::parse(std::string&& source)
{
    hemi_data_t data{std::move(source)};
    local::sink sink{data};
    in_json::parse(sink, std::begin(data.source_), std::end(data.source_));
    return data;

} // acmacs::hemi::v1::parse

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
