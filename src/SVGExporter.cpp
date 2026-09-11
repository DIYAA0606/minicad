#include "SVGExporter.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace minicad {

SVGExporter::SVGExporter(SVGStyleOptions options)
    : options_(std::move(options)) {}

void SVGExporter::visit(const Circle& circle) {
    const std::string& stroke = currentShapeHighlighted_ ? options_.highlightStroke : options_.defaultStroke;
    const std::string& fill = currentShapeHighlighted_ ? options_.highlightFill : options_.defaultFill;

    std::ostringstream oss;
    oss << "  <circle cx=\"" << circle.getCenter().x
        << "\" cy=\"" << circle.getCenter().y
        << "\" r=\"" << circle.getRadius()
        << "\" stroke=\"" << stroke
        << "\" stroke-width=\"" << options_.strokeWidth
        << "\" fill=\"" << fill
        << "\" fill-opacity=\"" << options_.fillOpacity
        << "\" />\n";

    currentElementsXml_ += oss.str();
}

void SVGExporter::visit(const Rectangle& rectangle) {
    const std::string& stroke = currentShapeHighlighted_ ? options_.highlightStroke : options_.defaultStroke;
    const std::string& fill = currentShapeHighlighted_ ? options_.highlightFill : options_.defaultFill;

    // Use computed corner vertices to guarantee exact rendering of rotated rectangles
    const auto vertices = rectangle.getVertices();

    std::ostringstream oss;
    oss << "  <polygon points=\"";
    for (size_t i = 0; i < vertices.size(); ++i) {
        oss << vertices[i].x << "," << vertices[i].y;
        if (i + 1 < vertices.size()) oss << " ";
    }
    oss << "\" stroke=\"" << stroke
        << "\" stroke-width=\"" << options_.strokeWidth
        << "\" fill=\"" << fill
        << "\" fill-opacity=\"" << options_.fillOpacity
        << "\" />\n";

    currentElementsXml_ += oss.str();
}

void SVGExporter::visit(const Polygon& polygon) {
    const std::string& stroke = currentShapeHighlighted_ ? options_.highlightStroke : options_.defaultStroke;
    const std::string& fill = currentShapeHighlighted_ ? options_.highlightFill : options_.defaultFill;

    const auto& vertices = polygon.getVertices();

    std::ostringstream oss;
    oss << "  <polygon points=\"";
    for (size_t i = 0; i < vertices.size(); ++i) {
        oss << vertices[i].x << "," << vertices[i].y;
        if (i + 1 < vertices.size()) oss << " ";
    }
    oss << "\" stroke=\"" << stroke
        << "\" stroke-width=\"" << options_.strokeWidth
        << "\" fill=\"" << fill
        << "\" fill-opacity=\"" << options_.fillOpacity
        << "\" />\n";

    currentElementsXml_ += oss.str();
}

std::string SVGExporter::exportToString(
    const Scene& scene,
    const std::unordered_set<size_t>& highlightIndices
) {
    currentElementsXml_.clear();

    const size_t count = scene.getShapeCount();
    AABB sceneBox = scene.getBoundingBox();

    // Determine viewport dimensions with margins
    double minX = 0.0;
    double minY = 0.0;
    double width = static_cast<double>(options_.canvasWidth);
    double height = static_cast<double>(options_.canvasHeight);

    if (sceneBox.isValid() && count > 0) {
        minX = sceneBox.minPt.x - options_.padding;
        minY = sceneBox.minPt.y - options_.padding;
        width = sceneBox.width() + (2.0 * options_.padding);
        height = sceneBox.height() + (2.0 * options_.padding);

        // Avoid degenerate 0-width or 0-height viewBox
        if (width < 1.0) width = 100.0;
        if (height < 1.0) height = 100.0;
    }

    // Optional: Draw Axis-Aligned Bounding Boxes (broadphase visualization)
    std::string aabbXml;
    if (options_.drawBoundingBoxes) {
        for (size_t i = 0; i < count; ++i) {
            const Shape* shape = scene.getShape(i);
            const AABB box = shape->getBoundingBox();
            std::ostringstream oss;
            oss << "  <!-- AABB for Shape #" << i << " (" << shape->getTypeName() << ") -->\n"
                << "  <rect x=\"" << box.minPt.x
                << "\" y=\"" << box.minPt.y
                << "\" width=\"" << box.width()
                << "\" height=\"" << box.height()
                << "\" fill=\"none\""
                << " stroke=\"" << options_.aabbStroke << "\""
                << " stroke-width=\"" << options_.aabbStrokeWidth << "\""
                << " stroke-dasharray=\"" << options_.aabbDashArray << "\""
                << " />\n";
            aabbXml += oss.str();
        }
    }

    // Render each shape via double dispatch Visitor
    for (size_t i = 0; i < count; ++i) {
        currentShapeHighlighted_ = (highlightIndices.find(i) != highlightIndices.end());
        scene.getShape(i)->accept(*this);
    }

    // Assemble final SVG document
    std::ostringstream svg;
    svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
        << " width=\"" << options_.canvasWidth << "\""
        << " height=\"" << options_.canvasHeight << "\""
        << " viewBox=\"" << minX << " " << minY << " " << width << " " << height << "\">\n"
        << "  <!-- Background Canvas -->\n"
        << "  <rect x=\"" << minX << "\" y=\"" << minY << "\" width=\"" << width << "\" height=\"" << height << "\" fill=\"#FFFFFF\" />\n"
        << "  <!-- Broadphase Bounding Boxes (Dashed) -->\n"
        << aabbXml
        << "  <!-- Geometry Elements -->\n"
        << currentElementsXml_
        << "</svg>\n";

    return svg.str();
}

void SVGExporter::exportToFile(
    const Scene& scene,
    const std::string& filepath,
    const std::unordered_set<size_t>& highlightIndices
) {
    const std::string content = exportToString(scene, highlightIndices);
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("SVGExporter::exportToFile: failed to open file for writing: " + filepath);
    }
    file << content;
    if (!file.good()) {
        throw std::runtime_error("SVGExporter::exportToFile: error occurred while writing to file: " + filepath);
    }
}

} // namespace minicad
