#include "QNodeSelect.h"
#include <QFileDialog>
#include "QPropertyModel.h"
#include "NodePathChooseDialog.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/scene/node.h>

namespace QT_UI
{
	QNodeSelect::QNodeSelect(class QPropertyModel* model, QString propertyName, const char* selectNodeId, const char* files, QWidget* parent)
		: QWidget( parent)
        , m_selectNodeId(selectNodeId ? selectNodeId : "")
        , m_files(files)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
	{
		// 布局控件
		m_horizonLayout = new QHBoxLayout( this);
		m_horizonLayout->setSpacing( 0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName( QString::fromUtf8("horizontalLayout"));

		// LineEdit
		m_lineEdit = new QLineEdit( this);
		m_lineEdit->setObjectName( QString::fromUtf8("lineEdit"));
		m_lineEdit->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_lineEdit);

		// ToolButton
		m_toolButton = new QToolButton( this);
		m_toolButton->setObjectName( QString::fromUtf8("toolButton"));
		m_toolButton->setText( "...");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_toolButton);

		setFocusProxy( m_toolButton);

		// 消息
		QObject::connect( m_toolButton, SIGNAL(clicked()), this, SLOT(OnSelectPath()));
		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));

		adjustHeightSize();
	}

	// 选择路径
	void QNodeSelect::OnSelectPath()
	{
		Echo::String qFileName = Studio::NodePathChooseDialog::getSelectingNode(nullptr);
		if (!qFileName.empty())
		{
			Echo::i32 nodeId = Echo::StringUtil::ParseI32(m_selectNodeId, -1);
			if (nodeId != -1)
			{
				Echo::Node* selectedNode = ECHO_DOWN_CAST<Echo::Node*>(Echo::Object::getById(nodeId));
				if (selectedNode)
				{
					Echo::Node* node = selectedNode->getNode(qFileName.c_str());
					if (node)
					{
						Echo::String relativePath = node->getNodePathRelativeTo(selectedNode);

						m_lineEdit->setText(relativePath.c_str());
						onEditFinished();
					}
				}
			}
		}
	}

	// correct size
	void QNodeSelect::adjustHeightSize()
	{
		//m_lineEdit->setMinimumHeight(m_lineEdit->geometry().height()*1.6);
		//m_toolButton->setMinimumHeight(m_toolButton->geometry().height() * 1.6);
	}

	// redefine paintEvent
	void QNodeSelect::paintEvent(QPaintEvent* event)
	{
		QWidget::paintEvent(event);
	}

	// edit finished
	void QNodeSelect::onEditFinished()
	{
		Echo::String value = m_lineEdit->text().toStdString().c_str();
		m_propertyModel->setValue(m_propertyName, value.c_str());
	}
}
