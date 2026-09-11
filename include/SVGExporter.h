#pragma once

#include "Scene.h"
#include "geometry/Circle.h"
#include "geometry/Polygon.h"
#include "geometry/Rectangle.h"
#include "geometry/ShapeVisitor.h"

#include <string>
#include <unordered_set>

namespace minicad {

/**
 * @brief Style configuration options for SVG export.
 */
struct SVGStyleOptions {
    std::string defaultStroke{"#2B6CB0"};      // Slate blue
    std::string defaultFill{"#EBF8FF"};        // Light blue tint
    double strokeWidth{2.0};
    double fillOpacity{0.6};

    // Collision highlighting style
    std::string highlightStroke{"#C53030"};    // Deep red
    std::string highlightFill{"#FED7D7"};      // Light red tint

    // Bounding box display
    bool drawBoundingBoxes{true};
    std::string aabbStroke{"#A0AEC0"};         // Cool gray
    double aabbStrokeWidth{1.0};
    std::string aabbDashArray{"4,4"};

    // Viewport padding
    double padding{20.0};
    int canvasWidth{800};
    int canvasHeight{600};
};

/**
 * @brief Serializes a MiniCAD Scene into Scalable Vector Graphics (SVG) format.
 *
 * Design Decisions & Architectural Rationale:
 * 1. Visitor Pattern Integration: Implements `ConstShapeVisitor` to decouple
 *    SVG XML generation from the geometric domain models.
 * 2. Visual Verification: Enables visual inspection of CAD modeling operations,
 *    affine transformations, and collision intersections beyond console printouts.
 * 3. Automatic Viewport Scaling: Automatically computes the scene bounding box
 *    with configurable margin padding, generating a self-contained SVG `viewBox`.
 */
class SVGExporter : public ConstShapeVisitor {
public:
    explicit SVGExporter(SVGStyleOptions options = {});

    void visit(const Circle& circle) override;
    void visit(const Rectangle& rectangle) override;
    void visit(const Polygon& polygon) override;

    /**
     * @brief Exports the entire scene to an SVG XML string.
     * @param scene The CAD scene to export.
     * @param highlightIndices Optional set of shape indices to render in alert style (e.g. colliding shapes).
     */
    [[nodiscard]] std::string exportToString(
        const Scene& scene,
        const std::unordered_set<size_t>& highlightIndices = {}
    );

    /**
     * @brief Exports the scene directly to a file on disk.
     */
    void exportToFile(
        const Scene& scene,
        const std::string& filepath,
        const std::unordered_set<size_t>& highlightIndices = {}
    );

    void setOptions(const SVGStyleOptions& options) noexcept { options_ = options; }
    [[nodiscard]] const SVGStyleOptions& getOptions() const noexcept { return options_; }

private:
    SVGStyleOptions options_;
    std::string currentElementsXml_;
    bool currentShapeHighlighted_{false};
};

} // namespace minicad
