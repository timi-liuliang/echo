#include "QTextureSelect.h"
#include <QColorDialog>
#include "ResChooseDialog.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>

namespace QT_UI
{
	QTextureSelect::QTextureSelect( QWidget* parent)
		: QPushButton( parent)
	{ 
		connect( this, SIGNAL(clicked()), this, SLOT(OnSelectTexture()));
	}

	void QTextureSelect::OnSelectTexture()
	{
        Echo::String qFileName = Studio::ResChooseDialog::getSelectingFile(nullptr, ".png|*.atla", "", "");
        if (!qFileName.empty())
        {
			setTexture(qFileName);
        }
	}

	void QTextureSelect::setTexture(const Echo::String& texturePath)
	{
		m_texturePath = texturePath; 

		emit Signal_TextureChagned();
	}

	bool QTextureSelect::ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val)
	{
        Echo::String path = val;
        if (!path.empty())
        {
            Echo::String fullPath = Echo::IO::instance()->convertResPathToFullPath(path);
            Echo::String ext = Echo::PathUtil::GetFileExt(path, true);
            if (Echo::StringUtil::Equal(ext, ".png", false))
            {
                QPixmap pixmap(fullPath.c_str());
                QRect tRect = QRect(rect.left() + 3, rect.top() + 3, rect.height() - 6, rect.height() - 6);
                painter->drawPixmap(tRect, pixmap);

                return true;
            }
        }

		return true;
	}

	QSize QTextureSelect::sizeHint() const
	{
		return size();
	}

	void QTextureSelect::paintEvent( QPaintEvent* event)
	{
		QPainter painter( this);

		ItemDelegatePaint( &painter, rect(), m_texturePath);
	}
}
