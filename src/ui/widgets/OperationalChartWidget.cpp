#include "OperationalChartWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPointF>
#include <QSizePolicy>

#include <algorithm>

OperationalChartWidget::OperationalChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("OperationalChartWidget");
    setMinimumHeight(260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setToolTip("Gráfico temporal operacional dos últimos 30 dias.");
}

void OperationalChartWidget::setValues(const QVector<double> &values, const QString &caption)
{
    values_ = values;
    caption_ = caption;
    hasSeries_ = values_.size() >= 2;
    update();
}

void OperationalChartWidget::setPlaceholder(const QString &message)
{
    values_.clear();
    caption_.clear();
    placeholderMessage_ = message;
    hasSeries_ = false;
    update();
}

bool OperationalChartWidget::hasSeries() const
{
    return hasSeries_;
}

void OperationalChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer = rect().adjusted(1, 1, -1, -1);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#151c24"));
    painter.drawRoundedRect(outer, 12, 12);

    painter.setPen(QPen(QColor("#2d3743"), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(outer, 12, 12);

    const QRectF plot = outer.adjusted(28, 24, -28, -34);
    painter.setPen(QPen(QColor("#25313d"), 1));
    for (int i = 1; i < 4; ++i) {
        const qreal y = plot.top() + plot.height() * i / 4.0;
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
    }
    for (int i = 1; i < 6; ++i) {
        const qreal x = plot.left() + plot.width() * i / 6.0;
        painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
    }

    if (!hasSeries_) {
        painter.setPen(QColor("#c1cad4"));
        QFont messageFont = painter.font();
        messageFont.setPointSize(10);
        messageFont.setBold(true);
        painter.setFont(messageFont);
        painter.drawText(plot, Qt::AlignCenter, placeholderMessage_);

        painter.setPen(QPen(QColor("#f28c28"), 2));
        QPainterPath placeholderPath;
        placeholderPath.moveTo(plot.left(), plot.center().y() + 18);
        placeholderPath.cubicTo(
            plot.left() + plot.width() * 0.28, plot.center().y() - 20,
            plot.left() + plot.width() * 0.58, plot.center().y() + 24,
            plot.right(), plot.center().y() - 10);
        painter.drawPath(placeholderPath);
        return;
    }

    const auto minmax = std::minmax_element(values_.cbegin(), values_.cend());
    double minValue = *minmax.first;
    double maxValue = *minmax.second;
    if (maxValue - minValue < 0.001) {
        maxValue += 1.0;
        minValue -= 1.0;
    }

    QVector<QPointF> points;
    points.reserve(values_.size());
    for (int i = 0; i < values_.size(); ++i) {
        const qreal x = plot.left() + plot.width() * i / static_cast<qreal>(values_.size() - 1);
        const qreal normalized = (values_.at(i) - minValue) / (maxValue - minValue);
        const qreal y = plot.bottom() - plot.height() * normalized;
        points.append(QPointF(x, y));
    }

    QPainterPath area;
    area.moveTo(points.first().x(), plot.bottom());
    area.lineTo(points.first());

    QPainterPath line;
    line.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i) {
        const QPointF previous = points.at(i - 1);
        const QPointF current = points.at(i);
        const qreal midX = (previous.x() + current.x()) / 2.0;
        line.cubicTo(QPointF(midX, previous.y()), QPointF(midX, current.y()), current);
        area.cubicTo(QPointF(midX, previous.y()), QPointF(midX, current.y()), current);
    }
    area.lineTo(points.last().x(), plot.bottom());
    area.closeSubpath();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(242, 140, 40, 38));
    painter.drawPath(area);

    painter.setPen(QPen(QColor("#f28c28"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(line);

    painter.setPen(QPen(QColor("#0f1720"), 2));
    painter.setBrush(QColor("#f28c28"));
    const int step = std::max(1, static_cast<int>(points.size()) / 8);
    for (int i = 0; i < points.size(); i += step) {
        painter.drawEllipse(points.at(i), 3.5, 3.5);
    }
    painter.drawEllipse(points.last(), 4.0, 4.0);

    if (!caption_.isEmpty()) {
        painter.setPen(QColor("#c1cad4"));
        QFont captionFont = painter.font();
        captionFont.setPointSize(9);
        captionFont.setBold(false);
        painter.setFont(captionFont);
        painter.drawText(outer.adjusted(28, 0, -28, -10), Qt::AlignLeft | Qt::AlignBottom, caption_);
    }
}
