/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
**************************************************************************/

#ifndef GENERICPROJECTNODE_H
#define GENERICPROJECTNODE_H

#include <coreplugin/ifile.h>
#include <projectexplorer/projectnodes.h>
#include <QStringList>
#include <QHash>

namespace GenericProjectManager {
namespace Internal {

class GenericProject;

class GenericProjectNode : public ProjectExplorer::ProjectNode
{
public:
    GenericProjectNode(GenericProject *project, Core::IFile *projectFile);
    virtual ~GenericProjectNode();

    Core::IFile *projectFile() const;
    QString projectFilePath() const;

    virtual bool hasTargets() const;

    virtual QList<ProjectExplorer::ProjectNode::ProjectAction> supportedActions() const;

    virtual bool addSubProjects(const QStringList &proFilePaths);
    virtual bool removeSubProjects(const QStringList &proFilePaths);

    virtual bool addFiles(const ProjectExplorer::FileType fileType,
                          const QStringList &filePaths,
                          QStringList *notAdded = 0);

    virtual bool removeFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths,
                             QStringList *notRemoved = 0);

    virtual bool renameFile(const ProjectExplorer::FileType fileType,
                             const QString &filePath,
                             const QString &newFilePath);


    void refresh();

private:
    FolderNode *findOrCreateFolderByName(const QString &filePath);
    FolderNode *findOrCreateFolderByName(const QStringList &components, int end);

private:
    GenericProject *_project;
    Core::IFile *_projectFile;
    QHash<QString, FolderNode *> _folderByName;
};

} // namespace Internal
} // namespace GenericProjectManager

#endif // GENERICPROJECTNODE_H
