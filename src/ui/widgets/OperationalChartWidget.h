#pragma once

#include <QVector>
#include <QWidget>

class OperationalChartWidget : public QWidget {
public:
    explicit OperationalChartWidget(QWidget *parent = nullptr);

    void setValues(const QVector<double> &values, const QString &caption = QString());
    void setPlaceholder(const QString &message);
    bool hasSeries() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> values_;
    QString caption_;
    QString placeholderMessage_ = "Aguardando dados operacionais.";
    bool hasSeries_ = false;
};
