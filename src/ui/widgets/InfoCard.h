#pragma once

#include <QWidget>

class QLabel;

class InfoCard : public QWidget {
public:
    explicit InfoCard(
        const QString &title,
        const QString &value,
        const QString &status = QString(),
        QWidget *parent = nullptr);
    explicit InfoCard(
        const QString &title,
        const QString &value,
        const QString &status,
        const QString &iconText,
        QWidget *parent);

    void setValue(const QString &value);
    void setStatus(const QString &status);

private:
    QLabel *titleLabel_ = nullptr;
    QLabel *iconLabel_ = nullptr;
    QLabel *valueLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};
