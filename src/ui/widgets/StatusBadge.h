#pragma once

#include <QWidget>

class QLabel;

class StatusBadge : public QWidget {
public:
    explicit StatusBadge(const QString &status = "OK", QWidget *parent = nullptr);

    void setStatus(const QString &status);
    QString status() const;
    QString category() const;

private:
    QLabel *label_ = nullptr;
    QString status_;
    QString category_;

    void applyStyle();
    static QString normalizedStatus(QString status);
    static QString tooltipForStatus(const QString &status);
};
