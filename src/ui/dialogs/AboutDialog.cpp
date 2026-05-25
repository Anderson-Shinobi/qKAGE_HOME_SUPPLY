#include "AboutDialog.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Sobre qKAGE_HOME_SUPPLY");
    setWindowIcon(QIcon(":/logo/qkage_logo.svg"));
    setModal(true);
    setMinimumWidth(520);
    setObjectName("AboutDialog");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 26, 28, 24);
    root->setSpacing(16);

    QLabel *logo = new QLabel(this);
    logo->setObjectName("AboutLogo");
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(QPixmap(":/logo/qkage_logo.svg").scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel *projectName = new QLabel("qKAGE_HOME_SUPPLY", this);
    projectName->setObjectName("AboutProjectName");
    projectName->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Domestic Supply & Financial Logistics System", this);
    subtitle->setObjectName("AboutSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    QLabel *statement = new QLabel("Menos desperdício.\nMais autonomia.\nMais capital livre.", this);
    statement->setObjectName("AboutStatement");
    statement->setAlignment(Qt::AlignCenter);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("AboutSeparator");

    QLabel *details = new QLabel(
        "Version: v1.2.1-beta\n\n"
        "Built with:\n"
        "Qt6 • C++17 • CMake • Linux\n\n"
        "Architecture:\n"
        "CLI • Controllers • Services • Core\n\n"
        "Designed and developed by:\n"
        "Anderson Nogueira",
        this);
    details->setObjectName("AboutDetails");
    details->setAlignment(Qt::AlignCenter);

    QLabel *signature = new QLabel("\"Engenharia começa onde abstrações terminam.\"", this);
    signature->setObjectName("AboutSignature");
    signature->setAlignment(Qt::AlignCenter);

    QLabel *copyright = new QLabel(
        "© 2026 Anderson Nogueira\n"
        "qKAGE_HOME_SUPPLY Project\n"
        "All rights reserved.",
        this);
    copyright->setObjectName("AboutCopyright");
    copyright->setAlignment(Qt::AlignCenter);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addWidget(logo);
    root->addWidget(projectName);
    root->addWidget(subtitle);
    root->addWidget(statement);
    root->addWidget(line);
    root->addWidget(details);
    root->addWidget(signature);
    root->addWidget(copyright);
    root->addWidget(buttons);

    setStyleSheet(R"(
        QDialog#AboutDialog {
            background: #0d141c;
            color: #f2f5f7;
            font-family: "Inter", "Segoe UI", sans-serif;
        }
        QLabel#AboutProjectName {
            color: #f28c28;
            font-size: 26px;
            font-weight: 800;
            letter-spacing: 0;
        }
        QLabel#AboutSubtitle {
            color: #d7dee7;
            font-size: 15px;
            font-weight: 600;
        }
        QLabel#AboutStatement {
            color: #f2f5f7;
            font-size: 18px;
            font-weight: 700;
            line-height: 1.35;
        }
        QFrame#AboutSeparator {
            color: #f28c28;
            background: #f28c28;
            min-height: 1px;
            max-height: 1px;
        }
        QLabel#AboutDetails {
            color: #c1cad4;
            font-size: 13px;
            line-height: 1.4;
        }
        QLabel#AboutSignature {
            color: #ffbe72;
            font-size: 13px;
            font-weight: 700;
            padding-top: 4px;
        }
        QLabel#AboutCopyright {
            color: #98a4b2;
            font-size: 12px;
        }
        QPushButton {
            background: #182331;
            border: 1px solid #f28c28;
            border-radius: 8px;
            color: #ffffff;
            font-weight: 700;
            min-width: 96px;
            padding: 8px 14px;
        }
        QPushButton:hover {
            background: #231d18;
            border-color: #ffab5c;
        }
    )");
}
