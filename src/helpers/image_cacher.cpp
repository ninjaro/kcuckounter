#include "helpers/image_cacher.hpp"

#include <QPainter>
#include <QtGlobal>
#include <cmath>

image_cacher::image_cacher(const QString& source_path)
    : source_path(source_path)
    , target_size()
    , cached_pixmap()
    , renderer()
    , base_scale(1.75)
    , min_short_px(63) {
    if (!source_path.isEmpty()) {
        renderer.load(source_path);
    }
}

void image_cacher::set_source(const QString& new_source_path) {
    if (source_path == new_source_path) {
        return;
    }
    source_path = new_source_path;
    renderer.load(source_path);
    rasterize();
}

void image_cacher::set_target_size(const QSize& new_target_size) {
    if (target_size == new_target_size) {
        return;
    }
    target_size = new_target_size;
    rasterize();
}

void image_cacher::set_base_scale(qreal new_base_scale) {
    const qreal clamped = std::max<qreal>(0.1, new_base_scale);
    if (qFuzzyCompare(base_scale, clamped)) {
        return;
    }
    base_scale = clamped;
    rasterize();
}

void image_cacher::set_min_short_px(int new_min_short_px) {
    const int clamped = std::max(0, new_min_short_px);
    if (min_short_px == clamped) {
        return;
    }
    min_short_px = clamped;
    rasterize();
}

QSize image_cacher::display_size() const { return target_size; }

const QPixmap& image_cacher::pixmap() const { return cached_pixmap; }

bool image_cacher::is_ready() const { return !cached_pixmap.isNull(); }

bool image_cacher::has_source() const {
    return !source_path.isEmpty() && renderer.isValid();
}

QSize image_cacher::raster_cache_size(const QSize& desired_size) const {
    if (desired_size.isEmpty()) {
        return QSize();
    }
    const qreal min_side
        = std::min(desired_size.width(), desired_size.height());
    qreal scale = base_scale;
    if (min_side > 0.0 && min_short_px > 0) {
        scale = std::max(scale, static_cast<qreal>(min_short_px) / min_side);
    }
    const int width = std::max(
        1, static_cast<int>(std::ceil(desired_size.width() * scale))
    );
    const int height = std::max(
        1, static_cast<int>(std::ceil(desired_size.height() * scale))
    );
    return QSize(width, height);
}

void image_cacher::rasterize() {
    if (!renderer.isValid() || target_size.isEmpty()) {
        cached_pixmap = QPixmap();
        return;
    }

    QPixmap new_pixmap(raster_cache_size(target_size));
    new_pixmap.fill(Qt::transparent);

    QPainter painter(&new_pixmap);
    renderer.render(
        &painter, QRectF(QPointF(0.0, 0.0), QSizeF(new_pixmap.size()))
    );
    cached_pixmap = new_pixmap;
}
