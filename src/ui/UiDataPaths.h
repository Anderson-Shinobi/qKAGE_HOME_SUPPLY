#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

inline QString qkageDataPath(const QString &fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();
    const QStringList roots = {
        QDir(currentDir).absoluteFilePath(".."),
        QDir(appDir).absoluteFilePath(".."),
        currentDir,
        QDir(appDir).absoluteFilePath("../.."),
        appDir
    };

    for (const QString &root : roots) {
        const QString candidate = QDir(root).absoluteFilePath("data/" + fileName);
        if (QFileInfo::exists(candidate)) {
            return QDir::cleanPath(candidate);
        }
    }

    return QDir(currentDir).absoluteFilePath("data/" + fileName);
}
