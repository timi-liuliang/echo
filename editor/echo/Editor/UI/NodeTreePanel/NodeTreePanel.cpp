#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include <QMessageBox>
#include "Studio.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "ResChooseDialog.h"
#include "PathChooseDialog.h"
#include "ReferenceChooseDialog.h"
#include "ResPanel.h"
#include "MainWindow.h"
#include "RenderWindow.h"
#include "OperationManager.h"
#include "SlotChooseDialog.h"
#include "ChannelExpressionDialog.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/editor/property_editor.h>
#include <engine/modules/gltf/gltf_res.h>

#define NodeScriptIndex		1
#define NodeLinkIndex		2
#define NodeDisableIndex	3
#define NodeIconWidth		24

namespace Studio
{
	static NodeTreePanel* g_inst = nullptr;

	NodeTreePanel::NodeTreePanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_width(0)
	{
		EchoAssert(!g_inst);
		g_inst = this;

		setupUi( this);
        
		m_nodeTreeWidget->setMouseTracking(true);
        m_nodeTreeWidget->setAttribute(Qt::WA_MacShowFocusRect,0);
        m_propertyTreeView->setAttribute(Qt::WA_MacShowFocusRect,0);
		m_propertyTreeView->header()->setDefaultAlignment(Qt::AlignCenter);

		QObject::connect(m_newNodeButton,  SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionAddNode,  SIGNAL(triggered()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionImportGltfScene, SIGNAL(triggered()), this, SLOT(importGltfScene()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onClickedNodeItem(QTreeWidgetItem*, int)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onDoubleClickedNodeItem(QTreeWidgetItem*, int)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onSelectNode()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onChangedNodeName(QTreeWidgetItem*)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemPositionChanged(QTreeWidgetItem*)), this, SLOT(onItemPositionChanged(QTreeWidgetItem*)));
        
        // property right
        QObject::connect(m_propertyTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showPropertyMenu(const QPoint&)));
		QObject::connect(m_actionPropertyReference, SIGNAL(triggered()), this, SLOT(onReferenceProperty()));
		QObject::connect(m_actionDeletePropertyReference, SIGNAL(triggered()), this, SLOT(onDeletePropertyReference()));
		QObject::connect(m_actionPropertyResetToDefault, SIGNAL(triggered()), this, SLOT(onPropertyResetToDefault()));
        
        // signal widget
        QObject::connect(m_signalTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showSignalTreeWidgetMenu(const QPoint&)));
		QObject::connect(m_actionDisconnectAll, SIGNAL(triggered()), this, SLOT(onSignalDisconnectAll()));
		QObject::connect(m_actionDisconnect, SIGNAL(triggered()), this, SLOT(onSignalDisconnect()));
		QObject::connect(m_actionGoToMethod, SIGNAL(triggered()), this, SLOT(onSignalGotoMethod()));

		// make the invisible item can't be drop
		m_nodeTreeWidget->invisibleRootItem()->setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);

		QObject::connect(m_actionChangeType, SIGNAL(triggered()), this, SLOT(onChangeType()));
		QObject::connect(m_actionDuplicateNode, SIGNAL(triggered()), this, SLOT(onDuplicateNode()));
		QObject::connect(m_actionDeleteNode, SIGNAL(triggered()), this, SLOT(onDeleteNodes()));
		QObject::connect(m_actionRenameNode, SIGNAL(triggered()), this, SLOT(onRenameNode()));
		QObject::connect(m_actionAddChildScene, SIGNAL(triggered()), this, SLOT(onInstanceChildScene()));
		QObject::connect(m_actionSaveBranchasScene, SIGNAL(triggered()), this, SLOT(onSaveBranchAsScene()));
		QObject::connect(m_actionDiscardInstancing, SIGNAL(triggered()), this, SLOT(onDiscardInstancing()));
        QObject::connect(m_actionConnectSlot, SIGNAL(triggered()), this, SLOT(onConnectOjectSlot()));

		// timer
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
		m_timer->start(50);
	}

	NodeTreePanel::~NodeTreePanel()
	{

	}

	NodeTreePanel* NodeTreePanel::instance()
	{
		return g_inst;
	}

	void NodeTreePanel::update()
	{
		if (m_nextEditObject)
		{
			onEditObject(m_nextEditObject);
			m_nextEditObject = nullptr;
		}

		if (m_width != m_nodeTreeWidget->width())
		{
			m_nodeTreeWidget->header()->resizeSection(1, NodeIconWidth);
			m_nodeTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
			m_nodeTreeWidget->header()->resizeSection(2, NodeIconWidth);
			m_nodeTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
			m_nodeTreeWidget->header()->resizeSection(3, NodeIconWidth);
			m_nodeTreeWidget->header()->setSectionResizeMode(2, QHeaderView::Fixed);
			m_nodeTreeWidget->header()->resizeSection(0, max<int>( 16, m_nodeTreeWidget->width() - (NodeIconWidth + 1) * 3));
			m_width = m_nodeTreeWidget->width();
		}

		// update property value to property model
		static Echo::i32 frameCount = 0; frameCount++;
		if (frameCount > 2 && m_currentEditObject)
		{		
			updateObjectPropertyValueToModel(m_currentEditObject, m_currentEditObject->getClassName());

			frameCount = 0;
		}
	}

	void NodeTreePanel::updateObjectPropertyValueToModel(Echo::Object* classPtr, const Echo::String& className)
	{
		// show parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if (parentClassName != "Object")
				updateObjectPropertyValueToModel(classPtr, parentClassName);
		}

		// show self property
		Echo::PropertyInfos propertys;
		Echo::Class::getPropertys(className, classPtr, propertys);
		if (propertys.size())
		{
			for (const Echo::PropertyInfo* prop : propertys)
			{
				Echo::Variant var;
				Echo::Class::getPropertyValue(classPtr, prop->m_name, var);

				QT_UI::QProperty* qproperty = m_propertyHelper.getQProperty();
				if (qproperty && qproperty->m_model)
				{
					qproperty->m_model->setValue(prop->m_name.c_str(), toModelValue(classPtr, prop->m_name, var).c_str());
				}
			}
		}
	}

	void NodeTreePanel::showNewNodeDialog()
	{
		Echo::String selectNodeType = NewNodeDialog::getSelectedNodeType();
		if (!selectNodeType.empty())
		{
			Echo::Node* node = Echo::Class::create<Echo::Node*>(selectNodeType.c_str());
			if (node)
			{
				node->setName(node->getClassName());
				if (node->getEditor())
					node->getEditor()->postEditorCreateObject();

				addNode(node);
			}
		}
	}

	void NodeTreePanel::clear()
	{
		m_nodeTreeWidget->clear();
		onEditObject(nullptr);
	}

	void NodeTreePanel::refreshNodeTreeDisplay()
	{
		refreshNodeTreeDisplay(m_nodeTreeWidget);
	}

	void NodeTreePanel::refreshNodeTreeDisplay(QTreeWidget* treeWidget)
	{
		treeWidget->clear();

		// begin with "Node"
		addNode( treeWidget, EchoEngine::instance()->getCurrentEditNode(), treeWidget->invisibleRootItem(), true);

		treeWidget->expandAll();
	}

	void NodeTreePanel::addNode(QTreeWidget* treeWidget, Echo::Node* node, QTreeWidgetItem* parent, bool recursive)
	{
		Echo::ui32   nodeIdx = node->getParent()->getParent() ? node->getParent()->getChildIdx(node) : 0;
		Echo::String nodeName = node->getName();

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem();
		nodeItem->setText(0, nodeName.c_str());
		nodeItem->setIcon(0, getNodeIcon(node));
		nodeItem->setData(0, Qt::UserRole, QVariant(node->getId()));
		nodeItem->setFlags( nodeItem->flags() | Qt::ItemIsEditable);
		parent->insertChild(nodeIdx, nodeItem);

		// change foreground color based on node state
		updateNodeTreeWidgetItemDisplay(treeWidget, nodeItem);

		// show property
		treeWidget->setCurrentItem(nodeItem);

		if (recursive)
		{
			for (Echo::ui32 i = 0; i < node->getChildNum(); i++)
			{
				Echo::Node* childNode = node->getChildByIndex(i);
				if(!childNode->isLink())
					addNode(treeWidget, childNode, nodeItem, recursive);
			}
		}
	}

	QIcon NodeTreePanel::getNodeIcon(Echo::Object* node)
	{
		Echo::ImagePtr thumbnail = Echo::ObjectEditor::getThumbnail(node);
		if (thumbnail)
		{
			QImage image(thumbnail->getData(), thumbnail->getWidth(), thumbnail->getHeight(), QImage::Format_RGBA8888);
			return QIcon(QPixmap::fromImage(image));
		}
		else
		{
			Echo::String nodeClassName = node->getClassName();

			// get icon path by node name
			Echo::String lowerCaseNodeName = nodeClassName;
			Echo::StringUtil::LowerCase(lowerCaseNodeName);
			Echo::String qIconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

			return QIcon(qIconPath.c_str());
		}
	}

	// get node in the item
	Echo::Node* NodeTreePanel::getNode(QTreeWidgetItem* item)
	{
		if (item)
		{
			Echo::i32 nodeId = item->data(0, Qt::UserRole).toInt();
			return (Echo::Node*)Echo::Node::getById(nodeId);
		}
		else
		{
			return nullptr;
		}
	}

	Echo::Node* NodeTreePanel::getCurrentSelectNode()
	{
		if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
			return nullptr;

		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem() ? m_nodeTreeWidget->currentItem() : m_nodeTreeWidget->invisibleRootItem()->child(0);
		return  item  ? getNode(item) : nullptr;
	}

	void NodeTreePanel::updateNodeTreeWidgetItemDisplay(QTreeWidget* treeWidget, QTreeWidgetItem* item)
	{
		if (treeWidget->invisibleRootItem()->childCount() == 0)
			return;

		QTreeWidgetItem* curItem = item ? item : (treeWidget->currentItem() ? treeWidget->currentItem() : treeWidget->invisibleRootItem()->child(0));
		if (curItem)
		{
			Echo::Node* node = getNode(curItem);
			if (node)
			{
				curItem->setForeground(0, QBrush(node->isEnable() ? QColor(220, 220, 220) : QColor(120, 120, 120)));

				if (!node->getScript().isEmpty())
				{
					curItem->setIcon(NodeScriptIndex, QIcon(":/icon/Icon/node_lua_script.png"));
					curItem->setStatusTip(NodeScriptIndex, node->getScript().getPath().c_str());
				}
				else
				{
					curItem->setIcon(NodeScriptIndex, QIcon());
					curItem->setStatusTip(NodeScriptIndex, "");
				}

				if (!node->getPath().empty())
					curItem->setIcon(NodeLinkIndex, QIcon(":/icon/node/link_child_scene.png"));
				else
					curItem->setIcon(NodeLinkIndex, QIcon());

				if (node->isEnable())
					curItem->setIcon(NodeDisableIndex, QIcon(":/icon/Icon/eye_open.png"));
				else
					curItem->setIcon(NodeDisableIndex, QIcon(":/icon/Icon/eye_close.png"));
			}
		}
	}

	Echo::Object* NodeTreePanel::getCurrentEditObject()
	{
		return m_currentEditObject;
	}

	void NodeTreePanel::addNode(Echo::Node* node)
	{
		if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
		{
			EchoEngine::instance()->setCurrentEditNode(node);
			refreshNodeTreeDisplay();
		}
		else
		{
			QTreeWidgetItem* parentItem = m_nodeTreeWidget->currentItem() ? m_nodeTreeWidget->currentItem() : m_nodeTreeWidget->invisibleRootItem()->child(0);
			Echo::Node* parentNode = getNode(parentItem);
			if (parentNode)
			{
				node->setParent(parentNode);

				addNode(m_nodeTreeWidget, node, parentItem, true);

				parentItem->setExpanded(true);
			}
		}

		// show property
		onSelectNode();
	}

    void NodeTreePanel::showPropertyMenu(const QPoint& point)
    {
		Echo::Object* currentEditObject = getCurrentEditObject();

        QModelIndex index = m_propertyTreeView->indexAt(point);
        if(index.isValid())
        {
            m_propertyTarget = m_propertyHelper.getPropertyName(index);
            if(!m_propertyTarget.empty())
            {
                EchoSafeDelete(m_propertyMenu, QMenu);
                m_propertyMenu = EchoNew(QMenu);
                
                m_propertyMenu->addAction(m_actionPropertyReference);

				if(currentEditObject && currentEditObject->isChannelExist(m_propertyTarget))
					m_propertyMenu->addAction(m_actionDeletePropertyReference);

				m_propertyMenu->addSeparator();
				m_propertyMenu->addAction(m_actionPropertyResetToDefault);
                
                m_propertyMenu->exec(QCursor::pos());
            }
        }
    }

	void NodeTreePanel::onReferenceProperty()
	{
		Echo::Object* object = getCurrentEditObject();
		if (object)
		{
			Echo::Channel* channel = object->getChannel(m_propertyTarget);
			Echo::String   expression = channel ? channel->getExpression() : "";
			if (Studio::ChannelExpressionDialog::getExpression(this, expression, object->getId()))
			{
				object->unregisterChannel(m_propertyTarget);
				if(!expression.empty())
					object->registerChannel(m_propertyTarget, expression);

				showSelectedObjectProperty();
			}
		}
	}

	void NodeTreePanel::onDeletePropertyReference()
	{
		Echo::Object* currentEditObject = getCurrentEditObject();
		if (currentEditObject)
		{
			currentEditObject->unregisterChannel(m_propertyTarget);

			// refresh property display
			showSelectedObjectProperty();
		}
	}

	void NodeTreePanel::onPropertyResetToDefault()
	{
		if (m_currentEditObject)
		{
			Echo::Variant value;
			if (Echo::Class::getPropertyValueDefault(m_currentEditObject, m_propertyTarget, value))
			{
				Echo::Class::setPropertyValue(m_currentEditObject, m_propertyTarget, value);

				// refresh property display
				showSelectedObjectProperty();
			}
		}
	}
    
	void NodeTreePanel::showMenu(const QPoint& point)
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->itemAt(point);
		if (item)
		{
			Echo::Node* node = getNode(item);
            bool isRootNode = EchoEngine::instance()->getCurrentEditNode() == node ? true : false;
			if (node->getPath().empty())
			{
				m_nodeTreeWidget->setCurrentItem(item);

				EchoSafeDelete(m_nodeTreeMenu, QMenu);
				m_nodeTreeMenu = EchoNew(QMenu);
				m_nodeTreeMenu->addAction(m_actionAddNode);
				m_nodeTreeMenu->addAction(m_actionAddChildScene);
				m_nodeTreeMenu->addAction(m_actionImportGltfScene);
				m_nodeTreeMenu->addSeparator();
				m_nodeTreeMenu->addAction(m_actionRenameNode);
				m_nodeTreeMenu->addAction(m_actionChangeType);
				if(node->getParent())
					m_nodeTreeMenu->addAction(m_actionDuplicateNode);
				m_nodeTreeMenu->addSeparator();
				m_nodeTreeMenu->addAction(m_actionSaveBranchasScene);
				m_nodeTreeMenu->addSeparator();
                if(!isRootNode)
                    m_nodeTreeMenu->addAction(m_actionDeleteNode);
				m_nodeTreeMenu->exec(QCursor::pos());
			}
			else
			{
				m_nodeTreeWidget->setCurrentItem(item);

				EchoSafeDelete(m_nodeTreeMenu, QMenu);
				m_nodeTreeMenu = EchoNew(QMenu);
				m_nodeTreeMenu->addAction(m_actionAddNode);
				m_nodeTreeMenu->addAction(m_actionAddChildScene);
				m_nodeTreeMenu->addAction(m_actionImportGltfScene);
				m_nodeTreeMenu->addSeparator();
				m_nodeTreeMenu->addAction(m_actionRenameNode);
				m_nodeTreeMenu->addAction(m_actionChangeType);
				if(node->getParent())
					m_nodeTreeMenu->addAction(m_actionDuplicateNode);
				m_nodeTreeMenu->addSeparator();
				m_nodeTreeMenu->addAction(m_actionSaveBranchasScene);
				m_nodeTreeMenu->addAction(m_actionDiscardInstancing);
				m_nodeTreeMenu->addSeparator();
                if(!isRootNode)
                    m_nodeTreeMenu->addAction(m_actionDeleteNode);
				m_nodeTreeMenu->exec(QCursor::pos());
			}
		}
		else
		{
			EchoSafeDelete(m_nodeTreeMenu, QMenu);
			m_nodeTreeMenu = EchoNew(QMenu);
			m_nodeTreeMenu->addAction(m_actionAddNode);
			m_nodeTreeMenu->addAction(m_actionAddChildScene);
			m_nodeTreeMenu->addAction(m_actionImportGltfScene);
			m_nodeTreeMenu->exec(QCursor::pos());
		}
	}
    
	void NodeTreePanel::removeItem(QTreeWidgetItem* item)
	{
		Echo::Node* node = getNode(item);
		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem)
        {
			parentItem->removeChild(item);
            if(node==m_currentEditObject)
                onUnselectCurrentEditObject();
            
            node->queueFree();
        }
		else
        {
			m_nodeTreeWidget->invisibleRootItem()->removeChild(item);
            if(node==m_currentEditObject)
                onUnselectCurrentEditObject();
            
            EchoEngine::instance()->setCurrentEditNode(nullptr);
        }
	}

	void NodeTreePanel::onDeleteNodes()
	{
		if (QMessageBox::Yes == QMessageBox(QMessageBox::Warning, "Warning", "Do you really want to delete selected nodes ?", QMessageBox::Yes | QMessageBox::No).exec())
		{
			QList<QTreeWidgetItem*> items = m_nodeTreeWidget->selectedItems();
			while (items.size() > 0)
			{
				QTreeWidgetItem* item = items[0];

				// remove item from ui
				removeItem(item);

				items = m_nodeTreeWidget->selectedItems();
			}

			// update property panel display
			onSelectNode();
		}
	}

	void NodeTreePanel::onRenameNode()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			m_nodeTreeWidget->editItem( item);
		}
	}

	void NodeTreePanel::onDuplicateNode()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			Echo::Node* node = getNode(item);
			if (node)
			{
				Echo::Node* duplicateNode = node->duplicate(true);
				duplicateNode->setParent(node->getParent());

				addNode( m_nodeTreeWidget, duplicateNode, item->parent(), true);
			}
		}
	}

	void NodeTreePanel::onChangeType()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			Echo::String nodeType = NewNodeDialog::getSelectedNodeType();
			if (!nodeType.empty())
			{
				Echo::Node* curNode = getNode(item);
				Echo::Node* newNode = Echo::Class::create<Echo::Node*>(nodeType.c_str());
				Echo::Node::NodeArray childrenNodes = curNode->getChildren();
				for (Echo::Node* child : childrenNodes)
				{
					if (!child->isLink())
					{
						child->setParent(newNode);
					}
				}

				// set new node parent
				if (curNode->getParent())
				{
					Echo::ui32 curNodeIdx = curNode->getParent()->getChildIdx(curNode);
					curNode->getParent()->insertChild(curNodeIdx, newNode);
				}
				else
				{
					newNode->setParent( nullptr);
				}

				// set new node name
				newNode->setName(curNode->getName());

				// remember parent item before delete this item
				if (item->parent())
				{
					QTreeWidgetItem* parentItem = item->parent();
					removeItem(item);
					addNode(m_nodeTreeWidget, newNode, parentItem, true);
				}
				else
				{
					removeItem(item);
					addNode(newNode);
				}
			}
		}
	}

	void NodeTreePanel::onSaveBranchAsScene()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			Echo::Node* node = getNode(item);
			Echo::String savePath = PathChooseDialog::getExistingPathName(this, ".scene", "Save").toStdString().c_str();
			if (node && !savePath.empty() && !Echo::PathUtil::IsDir(savePath))
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(savePath, resPath))
				{
					EchoEngine::instance()->saveBranchAsScene(resPath.c_str(), node);
					node->setPath(resPath);
					for (Echo::ui32 idx = 0; idx < node->getChildNum(); idx++)
					{
						node->getChildByIndex(idx)->setLink(true);
					}
				}

				// refresh respanel display
				ResPanel::instance()->reslectCurrentDir();
			}

			refreshNodeDisplay(item);
		}
	}

	void NodeTreePanel::onInstanceChildScene()
	{
		Echo::String nodeTreeFile = ResChooseDialog::getSelectingFile(this, ".scene");
		if (!nodeTreeFile.empty())
		{
			Echo::Node* node = Echo::Node::loadLink(nodeTreeFile, true);
			if (node)
			{
				addNode(node);
			}
		}
	}

	void NodeTreePanel::onDiscardInstancing()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			Echo::Node* node = getNode(item);
			node->setPath("");
			for (Echo::ui32 idx =0; idx < node->getChildNum(); idx++)
			{
				node->getChildByIndex(idx)->setLink(false);
			}

			refreshNodeDisplay(item);
		}
	}

	void NodeTreePanel::refreshNodeDisplay(QTreeWidgetItem* item)
	{
		if (item)
		{
			Echo::Node* node = getNode(item);

			// remove item from ui
			QTreeWidgetItem* parentItem = item->parent() ? item->parent() : m_nodeTreeWidget->invisibleRootItem();
			if (parentItem)
			{
				addNode(m_nodeTreeWidget, node, parentItem, true);

				// update property panel display
				onSelectNode();

				// remove this
				parentItem->removeChild(item);
			}
		}
	}

	void NodeTreePanel::onItemPositionChanged(QTreeWidgetItem* item)
	{
		QTreeWidgetItem* itemParent = item->parent();
		Echo::Node* parent = getNode(itemParent);
		if (parent)
		{
			for (int idx = 0; idx < itemParent->childCount(); idx++)
			{
				Echo::Node* childNode = getNode(itemParent->child(idx));
				if (childNode)
				{
					parent->insertChild(idx, childNode);
				}
			}
		}
	}

	void NodeTreePanel::onChangedNodeName(QTreeWidgetItem* item)
	{
		if (item)
		{
			Echo::Node* node = getNode(item);
			if (!node)
				return;

			Echo::String newName = item->text(0).toStdString().c_str();
			if (newName.empty())
				item->setText(0, node->getName().c_str());

			Echo::Node* parent = node->getParent();
			if (parent)
			{
				if (parent->isChildExist(newName))
				{
					item->setText(0, node->getName().c_str());
				}
				else
				{
					node->setName(newName);
				}
			}
			else
			{
				node->setName(newName);
			}
		}
	}

	void NodeTreePanel::importGltfScene()
	{
		Echo::String gltfFile = ResChooseDialog::getSelectingFile(this, ".gltf");
		if (!gltfFile.empty())
		{
			Echo::GltfResPtr asset = (Echo::GltfRes*)Echo::Res::get( gltfFile);
			Echo::Node* node = asset->build();
			if (node)
			{
				addNode(node);
			}
		}
	}

	void NodeTreePanel::showSelectedObjectProperty()
	{
		m_propertyHelper.clear();
		m_propertyHelper.setHeader("Property", "Value");

		Echo::Object* object = getCurrentEditObject();
		if (object)
		{
			showObjectPropertyRecursive(object, object->getClassName());
			m_propertyHelper.applyTo(Echo::StringUtil::ToString(object->getId()).c_str(), m_propertyTreeView, this, SLOT(refreshPropertyToObject(const QString&, QVariant)), false);
		}
		else
		{
			m_propertyHelper.applyTo("empty", m_propertyTreeView, this, SLOT(refreshPropertyToObject(const QString&, QVariant)), false);
		}	
	}
    
    void NodeTreePanel::showSelectedObjectSignal()
    {
        m_signalTreeWidget->clear();
        
        Echo::Object* object = getCurrentEditObject();
        if(object)
        {
            showObjectSignalRecursive( object, object->getClassName());
        }
        
        m_signalTreeWidget->expandAll();
        
        // update signal tab visible
        updateSignaltabVisible();
    }
    
    // because Qt has no easy method hide|show tab, we have to remove and add it frquently.
    // https://stackoverflow.com/questions/18394706/show-hide-sub-tab-on-qtabwidget
    void NodeTreePanel::updateSignaltabVisible()
    {
        // hide tab when there are no signals
        int signalCount = m_signalTreeWidget->invisibleRootItem()->childCount();
        if(signalCount == 0)
        {
            // hide tab
            if(m_tabWidget->count()==2)
                m_tabWidget->removeTab(1);
        }
        else
        {
            // show tab
            if(m_tabWidget->count()==1)
                m_tabWidget->addTab(m_tabSignal, QString("Signal"));
        }
    }
    
    void NodeTreePanel::showObjectSignalRecursive(Echo::Object* classPtr, const Echo::String& className)
    {
        // show parent property first
        Echo::String parentClassName;
        if (Echo::Class::getParentClass(parentClassName, className))
        {
            // don't display property of object
            if(parentClassName!="Object")
                showObjectSignalRecursive(classPtr, parentClassName);
        }
        
        // add all signals
        Echo::ClassInfo* classInfo = Echo::Class::getClassInfo(className);
        if(classInfo)
        {
            // class name
            if(!classInfo->m_signals.empty())
            {
                QTreeWidgetItem* classItem = new QTreeWidgetItem();
                classItem->setText(0, className.c_str());
                //classItem->setBold(true);
                
                // signals
                for(auto it : classInfo->m_signals)
                {
					Echo::Signal* signal = Echo::Class::getSignal(classPtr, it.first);
					if (signal)
					{
						QTreeWidgetItem* signalItem = new QTreeWidgetItem();
						signalItem->setText(0, it.first.c_str());
						signalItem->setIcon(0, QIcon(":/icon/Icon/signal/signal.png"));
						signalItem->setData(0, Qt::UserRole, QString("signal"));
						//nodeItem->setFlags( nodeItem->flags() | Qt::ItemIsEditable);
						classItem->addChild(signalItem);

						// show all connects
						auto connects = signal->getConnects();
						if (connects)
						{
							for (Echo::Connect* conn : *connects)
							{
								Echo::ConnectLuaMethod* luaConn = ECHO_DOWN_CAST<Echo::ConnectLuaMethod*>(conn);
								if (luaConn)
								{
									QTreeWidgetItem* connItem = new QTreeWidgetItem();
									connItem->setText(0, Echo::StringUtil::Format("%s:%s()", luaConn->m_targetPath.c_str(), luaConn->m_functionName.c_str()).c_str());
									connItem->setIcon(0, QIcon(":/icon/Icon/signal/connect.png"));
									connItem->setData(0, Qt::UserRole, QString("connect"));
									signalItem->addChild(connItem);
								}
							}
						}
					}
                }
                
                m_signalTreeWidget->invisibleRootItem()->addChild(classItem);
            }
        }
    }
    
    void NodeTreePanel::showSignalTreeWidgetMenu(const QPoint& point)
    {
        QTreeWidgetItem* item = m_signalTreeWidget->itemAt(point);
        if (item)
        {
            Echo::String itemType = item->data(0, Qt::UserRole).toString().toStdString().c_str();
            if (itemType == "signal")
            {
                m_signalName = item->text(0).toStdString().c_str();
                
                EchoSafeDelete(m_signalTreeMenu, QMenu);
                m_signalTreeMenu = EchoNew(QMenu);
                m_signalTreeMenu->addAction(m_actionConnectSlot);
				m_signalTreeMenu->addAction(m_actionDisconnectAll);
                m_signalTreeMenu->exec(QCursor::pos());
            }
			else if (itemType == "connect")
			{
				m_signalName = item->parent()->text(0).toStdString().c_str();
				m_connectStr = item->text(0).toStdString().c_str();

				EchoSafeDelete(m_signalTreeMenu, QMenu);
				m_signalTreeMenu = EchoNew(QMenu);
				m_signalTreeMenu->addAction(m_actionConnectEdit);
				m_signalTreeMenu->addAction(m_actionGoToMethod);
				m_signalTreeMenu->addAction(m_actionDisconnect);
				m_signalTreeMenu->exec(QCursor::pos());
			}
        }
    }
    
    void NodeTreePanel::onConnectOjectSlot()
    {
        Echo::Node* currentNode = ECHO_DOWN_CAST<Echo::Node*>(m_currentEditObject);
        if(currentNode)
        {
            Echo::String nodePath;
            Echo::String functionName = m_signalName;
            if(SlotChooseDialog::getSlot(this, nodePath, functionName))
            {
                Echo:: Signal* signal = Echo::Class::getSignal( currentNode, m_signalName);
                if(signal)
                {
                    Echo::Node* slotNode = currentNode->getNode(nodePath.c_str());
                    Echo::String relativePath = slotNode->getNodePathRelativeTo(currentNode);
                    signal->connectLuaMethod(relativePath, functionName);

					showSelectedObjectSignal();
                }
            }
        }
    }

	void NodeTreePanel::onSignalDisconnectAll()
	{
		Echo::Node* currentNode = ECHO_DOWN_CAST<Echo::Node*>(m_currentEditObject);
		if (currentNode)
		{
			Echo::Signal* signal = Echo::Class::getSignal(currentNode, m_signalName);
			if (signal)
			{
				signal->disconnectAll();

				showSelectedObjectSignal();
			}
		}
	}

	void NodeTreePanel::onSignalDisconnect()
	{
		Echo::Node* currentNode = ECHO_DOWN_CAST<Echo::Node*>(m_currentEditObject);
		if (currentNode)
		{
			Echo::Signal* signal = Echo::Class::getSignal(currentNode, m_signalName);
			if (signal)
			{
				auto connects = signal->getConnects();
				if (connects)
				{
					for (Echo::Connect* conn : *connects)
					{
						Echo::ConnectLuaMethod* luaConn = ECHO_DOWN_CAST<Echo::ConnectLuaMethod*>(conn);
						if (luaConn)
						{
							Echo::String connStr = Echo::StringUtil::Format("%s:%s()", luaConn->m_targetPath.c_str(), luaConn->m_functionName.c_str()).c_str();
							if (connStr == m_connectStr)
								signal->disconnect(conn);
						}
					}
				}

				showSelectedObjectSignal();
			}
		}
	}

	void NodeTreePanel::onSignalGotoMethod()
	{
		Echo::Node* currentNode = ECHO_DOWN_CAST<Echo::Node*>(m_currentEditObject);
		if (currentNode)
		{
			Echo::Signal* signal = Echo::Class::getSignal(currentNode, m_signalName);
			if (signal)
			{
				auto connects = signal->getConnects();
				if (connects)
				{
					for (Echo::Connect* conn : *connects)
					{
						Echo::ConnectLuaMethod* luaConn = ECHO_DOWN_CAST<Echo::ConnectLuaMethod*>(conn);
						if (luaConn)
						{
							Echo::String connStr = Echo::StringUtil::Format("%s:%s()", luaConn->m_targetPath.c_str(), luaConn->m_functionName.c_str()).c_str();
							if (connStr == m_connectStr)
							{
								Echo::Node* target = dynamic_cast<Echo::Node*>(luaConn->getTarget());
								if (target)
								{
									const Echo::ResourcePath& script = target->getScript();
									if (!script.isEmpty())
									{
										MainWindow::instance()->openTextEditor(script.getPath());
									}
								}
							}
						}
					}
				}

				showSelectedObjectSignal();
			}
		}
	}

	void NodeTreePanel::showObjectPropertyRecursive(Echo::Object* classPtr, const Echo::String& className)
	{
		// show parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if(parentClassName!="Object")
				showObjectPropertyRecursive(classPtr, parentClassName);
		}

		// show self property
		Echo::PropertyInfos propertys;
		Echo::Class::getPropertys(className, classPtr, propertys);
		if (propertys.size())
		{
            // collect all categories
            std::set<Echo::String> categorySet;
            for (const Echo::PropertyInfo* prop : propertys)
                categorySet.insert(prop->getHint(Echo::PropertyHintType::Category));
            
            // show propertys by category
			m_propertyHelper.beginMenu(className.c_str());
            for(const Echo::String& category : categorySet)
            {
				if(!category.empty())
					m_propertyHelper.beginMenu(category.c_str());

                for (const Echo::PropertyInfo* prop : propertys)
                {
                    if(prop->getHint(Echo::PropertyHintType::Category)==category)
                    {
                        Echo::Variant var;
                        Echo::Class::getPropertyValue(classPtr, prop->m_name, var);

                        showPropertyByVariant(classPtr, className, prop->m_name, var, prop);
                    }
                }

				if (!category.empty())
					m_propertyHelper.endMenu();
            }
			m_propertyHelper.endMenu();
		}
	}

	Echo::String NodeTreePanel::toModelValue(Echo::Object* object, const Echo::String& name, const Echo::Variant& var)
	{
		if (!object->isChannelExist(name))
		{
			return var.toString();
		}
		else
		{
			Echo::String expression = object->getChannel(name)->getExpression();
			return Echo::StringUtil::Format("%s#%s#%d", expression.c_str(), var.toString().c_str(), int(var.getType()));
		}
	}

	void NodeTreePanel::showPropertyByVariant(Echo::Object* object, const Echo::String& className, const Echo::String& propertyName, const Echo::Variant& var, const Echo::PropertyInfo* propInfo)
	{
		Echo::String modelValue = toModelValue(object, propertyName, var);
		if (!object->isChannelExist(propertyName))
		{
			Echo::PropertyEditorFactory* customFactory = Echo::PropertyEditor::getFactory(className, propertyName);
			if (!customFactory)
			{
				Echo::String extraData = Echo::StringUtil::Format("%d:%s", object->getId(), propertyName.c_str());
				Echo::String resourceHint = propInfo->getHint(Echo::PropertyHintType::ObjectType);

				switch (var.getType())
				{
				case Echo::Variant::Type::Bool:			m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_CheckBox); break;
				case Echo::Variant::Type::Int:			m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Int); break;
				case Echo::Variant::Type::Real:			m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Real); break;
				case Echo::Variant::Type::Vector2:		m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Vector2); break;
				case Echo::Variant::Type::Vector3:		m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Vector3); break;
				case Echo::Variant::Type::Color:		m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_ColorSelect); break;
				case Echo::Variant::Type::String:
				case Echo::Variant::Type::Base64String:	m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_String, extraData.c_str()); break;
				case Echo::Variant::Type::ResourcePath:	
				{
					Echo::String resourceBehavir = propInfo->getHint(Echo::PropertyHintType::ResourceBehavior);
					if(resourceBehavir=="load")
						m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_AssetsSelect, var.toResPath().getSupportExts().c_str());
					else
						m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_AssetsSave, var.toResPath().getSupportExts().c_str());
					
					break;
				}
				case Echo::Variant::Type::StringOption: m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_ComboBox, var.toStringOption().getOptionsStr().c_str()); break;
				case Echo::Variant::Type::NodePath:		m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_NodeSelect, Echo::StringUtil::ToString(object->getId()).c_str()); break;
				case Echo::Variant::Type::Object:		m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Res, resourceHint.c_str()); break;
				default:								m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_None); break;
				}
			}
			else
			{
				Echo::String extraData = Echo::StringUtil::Format("%d:%s:%s", object->getId(), className.c_str(), propertyName.c_str());
				m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_Custom, extraData.c_str());
			}
		}
		else
		{
			m_propertyHelper.addItem(propertyName.c_str(), modelValue, QT_UI::WT_ChannelEditor, nullptr);
		}
	}

	void NodeTreePanel::refreshPropertyToObject(const QString& property, QVariant value)
	{
		Echo::String propertyName = property.toStdString().c_str();
		Echo::String valStr = value.toString().toStdString().c_str();
		Echo::Object* object = getCurrentEditObject();
		Echo::Variant::Type type = Echo::Class::getPropertyType(object, propertyName);
		if (!object->isChannelExist(propertyName))
		{
			Echo::Variant propertyValue;
			if (propertyValue.fromString(type, valStr))
			{
				Echo::Class::setPropertyValue(object, propertyName, propertyValue);

				// refresh property display
				showSelectedObjectProperty();

				// update select item display
				updateNodeTreeWidgetItemDisplay(m_nodeTreeWidget, nullptr);
			}
			else
			{
				EchoLogError("Can't set property [%s] value [%s]", propertyName.c_str(), valStr.c_str());
			}
		}
		else
		{
			Echo::StringArray infos = Echo::StringUtil::Split(valStr, "#");
			Echo::String expression = infos[0];

			object->registerChannel(propertyName, expression);

			// sync channel
			Echo::Channel::syncAll();

			// refresh property display
			showSelectedObjectProperty();

			// update select item display
			updateNodeTreeWidgetItemDisplay(m_nodeTreeWidget, nullptr);
		}
	}

	void NodeTreePanel::onClickedNodeItem(QTreeWidgetItem* item, int column)
	{
		if (column == NodeDisableIndex)
		{
			Echo::Node* node = getNode(item);
			if (node)
			{
				node->setEnable(!node->isEnable());
				if (node->isEnable())
					item->setIcon(NodeDisableIndex, QIcon(":/icon/Icon/eye_open.png"));
				else
					item->setIcon(NodeDisableIndex, QIcon(":/icon/Icon/eye_close.png"));

			}
		}
		if (column == NodeScriptIndex)
		{
			Echo::Node* node = getNode(item);
			if (node)
			{
				const Echo::ResourcePath& script = node->getScript();
				if (!script.isEmpty())
				{
					MainWindow::instance()->openTextEditor(script.getPath());
				}
			}
		}
		else if (column == NodeLinkIndex)
		{
			Echo::Node* node = getNode(item);
			if (node)
			{
				Echo::String path = node->getPath();
				if (!path.empty())
				{
					MainWindow::instance()->openNodeTree(path);
				}
			}
		}
		else
		{
			onSelectNode();
		}
	}

	void NodeTreePanel::onDoubleClickedNodeItem(QTreeWidgetItem* item, int column)
	{
		Echo::Node* node = getNode(item);
		if (node)
		{
			IRWInputController* controller = AStudio::instance()->getRenderWindow()->getInputController();
			if (controller)
			{
				controller->onFocusNode( node);
			}
		}
	}

    void NodeTreePanel::onUnselectCurrentEditObject()
    {
        // editor extension : unselected object
        if (m_currentEditObject && m_currentEditObject->getEditor())
        {
            m_currentEditObject->getEditor()->onEditorUnSelectThisNode();
        }
        
        m_currentEditObject = nullptr;
    }

	void NodeTreePanel::onSelectNode()
	{
        onUnselectCurrentEditObject();

		m_currentEditObject = getCurrentSelectNode();
		showSelectedObjectProperty();
        showSelectedObjectSignal();

		OperationManager::instance()->onSelectedObject(m_currentEditObject ? m_currentEditObject->getId() : 0, false);

		// editor extension : select object
		if (m_currentEditObject && m_currentEditObject->getEditor())
		{
			m_currentEditObject->getEditor()->onEditorSelectThisNode();
		}

		// change subeditor 2d or 3d
		Echo::Render* renderNode = dynamic_cast<Echo::Render*>(m_currentEditObject);
		if (renderNode)
		{
			MainWindow::instance()->setSubEdit( renderNode->getRenderType().getValue()!="3d" ? "2D" : "3D");
		}
	}

	void NodeTreePanel::onSelectRes(const Echo::String& resPath)
	{
		Echo::String extWithDot = Echo::PathUtil::GetFileExt(resPath, true);
		m_currentEditObject = Echo::Res::getResFunByExtension(extWithDot) ? Echo::Res::get(resPath) : nullptr;

		showSelectedObjectProperty();

		OperationManager::instance()->onSelectedObject(m_currentEditObject ? m_currentEditObject->getId() : 0, false);

		// editor extension : select object
		if (m_currentEditObject && m_currentEditObject->getEditor())
		{
			m_currentEditObject->getEditor()->onEditorSelectThisNode();
		}
	}

	void NodeTreePanel::onEditObject(Echo::Object* res)
	{
		m_currentEditObject = res;

		showSelectedObjectProperty();

		OperationManager::instance()->onSelectedObject(m_currentEditObject ? m_currentEditObject->getId() : 0, false);
	}

	void NodeTreePanel::saveCurrentEditRes()
	{
		Echo::Res* res = dynamic_cast<Echo::Res*>(m_currentEditObject);
		if ( res && !res->getPath().empty())
		{
			res->save();
		}
	}
}
