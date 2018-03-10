/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QUILOADER_P_H
#define QUILOADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QByteArray>
#include <QtCore/QMetaType>

QT_FORWARD_DECLARE_CLASS(QDataStream)

// This file is here for use by the form preview in Linguist. If you change anything
// here or in the code which uses it, remember to adapt Linguist accordingly.

#define PROP_GENERIC_PREFIX "_q_notr_"
#define PROP_TOOLITEMTEXT "_q_toolItemText_notr"
#define PROP_TOOLITEMTOOLTIP "_q_toolItemToolTip_notr"
#define PROP_TABPAGETEXT "_q_tabPageText_notr"
#define PROP_TABPAGETOOLTIP "_q_tabPageToolTip_notr"
#define PROP_TABPAGEWHATSTHIS "_q_tabPageWhatsThis_notr"

QT_BEGIN_NAMESPACE

class QUiTranslatableStringValue
{
public:
    QByteArray value() const { return m_value; }
    void setValue(const QByteArray &value) { m_value = value; }
    QByteArray comment() const { return m_comment; }
    void setComment(const QByteArray &comment) { m_comment = comment; }

private:
    friend QDataStream &operator>>(QDataStream &in, QUiTranslatableStringValue &s);

    QByteArray m_value;
    QByteArray m_comment;
};

struct QUiItemRolePair {
    int realRole;
    int shadowRole;
};

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

extern const QUiItemRolePair qUiItemRoles[];

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QUiTranslatableStringValue)


#endif // QUILOADER_P_H
