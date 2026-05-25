#pragma once

#include <QMainWindow>

class QButtonGroup;
class QLabel;
class QStackedWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *pages_ = nullptr;
    QButtonGroup *navigationGroup_ = nullptr;
    QLabel *versionStatusLabel_ = nullptr;
    QLabel *systemStatusLabel_ = nullptr;
    QLabel *clockStatusLabel_ = nullptr;

    QWidget *createSidebar();
    void addNavigationButton(QWidget *sidebar, const QString &label, int pageIndex);
    void addAboutButton(QWidget *sidebar);
    void configureStatusBar();
    void centerOnScreen();
};
