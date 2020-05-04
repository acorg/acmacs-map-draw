#include "acmacs-map-draw/select-filter.hh"

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::rectangle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Rectangle& aRectangle, Rotation rotation)
{
    acmacs::Transformation transformation;
    transformation.rotate(- rotation); // rectangle rotated -> layout rotated in opposite direction
    auto layout = aLayout.transform(transformation);
    const auto not_in_rectangle = [&layout,aIndexBase,&aRectangle](auto index) -> bool {
        const auto& p = layout->at(index + aIndexBase);
        return p.number_of_dimensions() == acmacs::number_of_dimensions_t{2} ? !aRectangle.within(p) : true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_rectangle), indexes.end());

} // acmacs::map_draw::select::filter::rectangle_in

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
