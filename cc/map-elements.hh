#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/text-style.hh"
#include "acmacs-base/line.hh"
#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }
namespace acmacs::draw { class DrawElements; }
class ChartDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Element;

    class Elements
    {
     public:
        enum Order { BeforePoints, AfterPoints };

        Elements();

        void reset();

        void list_keywords() const;

        void remove(std::string_view keyword);
        bool exists(std::string_view keyword) const;

        template <typename ElementType> ElementType& find(std::string_view keyword)
        {
            if (auto* found = find_base(keyword); found)
                return dynamic_cast<ElementType&>(*found);
            else
                throw std::runtime_error{fmt::format("no map element for \"{}\"", keyword)};
        }

        template <typename ElementType, typename ... Arg> ElementType& add(Arg&& ... arg)
        {
            auto ptr = std::make_unique<ElementType>(std::forward<Arg>(arg) ...);
            auto& elt = *ptr;
            mElements.push_back(std::move(ptr));
            return elt;
        }

        template <typename ElementType, typename ... Arg> ElementType& find_or_add(std::string_view keyword, Arg&& ... arg)
        {
            if (auto* found = find_base(keyword); found)
                return dynamic_cast<ElementType&>(*found);
            else
                return add<ElementType>(std::forward<Arg>(arg) ...);
        }

        void draw(acmacs::surface::Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const;

     private:
        std::vector<std::shared_ptr<Element>> mElements;

        void add_basic_elements_v1();

        Element* find_base(std::string_view keyword);

    }; // class Elements

// ----------------------------------------------------------------------

    class Element
    {
     public:
        Element(std::string_view keyword, Elements::Order aOrder) : mKeyword(keyword), mOrder(aOrder) {}
        Element(const Element&) = default;
        virtual ~Element() = default;

        std::string keyword() const { return mKeyword; }
        Elements::Order order() const { return mOrder; }
        virtual void draw(acmacs::surface::Surface& /* aSurface*/, const ChartDraw& /*aChartDraw*/) const {} // obsolete
        virtual void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const = 0; // { std::cerr << "WARNING: map_elements::Element::draw " << typeid(this).name() << '\n'; }

     protected:
        virtual acmacs::PointCoordinates subsurface_origin(acmacs::surface::Surface& aSurface, const acmacs::PointCoordinates& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const;

        void keyword(std::string keyword) { mKeyword = keyword; }

     private:
        std::string mKeyword;
        Elements::Order mOrder;
    };

} // namespace map_elements


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
