#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include <QMessageBox>
#include "Studio.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "ResChooseDialog.h"
#include "PathChooseDialog.h"
#include "ResPanel.h"
#include "MainWindow.h"
#include "RenderWindow.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/modules/gltf/gltf_res.h>

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
        
        m_nodeTreeWidget->setAttribute(Qt::WA_MacShowFocusRect,0);
        m_propertyTreeView->setAttribute(Qt::WA_MacShowFocusRect,0);

		QObject::connect(m_newNodeButton,  SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionAddNode,  SIGNAL(triggered()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionImportGltfScene, SIGNAL(triggered()), this, SLOT(importGltfScene()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onClickedNodeItem(QTreeWidgetItem*, int)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onDoubleClickedNodeItem(QTreeWidgetItem*, int)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onSelectNode()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onChangedNodeName(QTreeWidgetItem*)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemPositionChanged(QTreeWidgetItem*)), this, SLOT(onItemPositionChanged(QTreeWidgetItem*)));

		// make the invisible item can't be drop
		m_nodeTreeWidget->invisibleRootItem()->setFlags( Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);

		QObject::connect(m_actionChangeType, SIGNAL(triggered()), this, SLOT(onChangeType()));
		QObject::connect(m_actionDuplicateNode, SIGNAL(triggered()), this, SLOT(onDuplicateNode()));
		QObject::connect(m_actionDeleteNode, SIGNAL(triggered()), this, SLOT(onDeleteNodes()));
		QObject::connect(m_actionRenameNode, SIGNAL(triggered()), this, SLOT(onRenameNode()));
		QObject::connect(m_actionAddChildScene, SIGNAL(triggered()), this, SLOT(onInstanceChildScene()));
		QObject::connect(m_actionSaveBranchasScene, SIGNAL(triggered()), this, SLOT(onSaveBranchAsScene()));
		QObject::connect(m_actionDiscardInstancing, SIGNAL(triggered()), this, SLOT(onDiscardInstancing()));

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
			m_nodeTreeWidget->header()->resizeSection(1, 32);
			m_nodeTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
			m_nodeTreeWidget->header()->resizeSection(0, m_nodeTreeWidget->width() - 32);
			m_width = m_nodeTreeWidget->width();

			//m_propertyTreeView->header()->resizeSection(0, std::max<int>(140, m_propertyTreeView->header()->sectionSize(0)));
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
				addNode(node);
			}
		}
	}

	// clear
	void NodeTreePanel::clear()
	{
		m_nodeTreeWidget->clear();
		onEditObject(nullptr);
	}

	// refresh node tree display
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
		nodeItem->setIcon(0, QIcon( getNodeIcon(node).c_str()));
		nodeItem->setData(0, Qt::UserRole, QVariant(node->getId()));
		nodeItem->setFlags( nodeItem->flags() | Qt::ItemIsEditable);
		parent->insertChild(nodeIdx, nodeItem);

		// child scene
		if (!node->getPath().empty())
			nodeItem->setIcon(1, QIcon(":/icon/node/link_child_scene.png"));

		// change foreground color based on node state
		updateNodeTreeWidgetItemDisplay(treeWidget, nodeItem);

		// show property
		treeWidget->setCurrentItem(nodeItem);

		if (recursive)
		{
			for (Echo::ui32 i = 0; i < node->getChildNum(); i++)
			{
				Echo::Node* childNode = node->getChild(i);
				if(!childNode->isLink())
					addNode(treeWidget, childNode, nodeItem, recursive);
			}
		}
	}

	Echo::String NodeTreePanel::getNodeIcon(Echo::Node* node)
	{
		Echo::String iconPath = node->getEditor() ? node->getEditor()->getEditorIcon() : "";

		Echo::String rootPath = AStudio::instance()->getRootPath();
		Echo::String nodeClassName = node->getClassName();

		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeClassName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		Echo::String qIconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		return iconPath.empty() ? qIconPath.c_str() : (rootPath + iconPath).c_str();
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

	// update item display
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
				// change color
				curItem->setForeground(0, QBrush(node->isEnable() ? QColor(220, 220, 220) : QColor(120, 120, 120)));
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

	// node tree widget show menu
	void NodeTreePanel::showMenu(const QPoint& point)
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->itemAt(point);
		if (item)
		{
			Echo::Node* node = getNode(item);
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

	// remove item
	void NodeTreePanel::removeItem(QTreeWidgetItem* item)
	{
		Echo::Node* node = getNode(item);
		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem)
			parentItem->removeChild(item);
		else
			m_nodeTreeWidget->invisibleRootItem()->removeChild(item);

		node->remove();
		node->queueFree();
	}

	// on trigger delete nodes
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

			// set as nullptr
			if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
			{
				EchoEngine::instance()->setCurrentEditNode(nullptr);
			}

			// update property panel display
			onSelectNode();
		}
	}

	// on trigger rename node
	void NodeTreePanel::onRenameNode()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			m_nodeTreeWidget->editItem( item);
		}
	}

	// on duplicate node
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
				QTreeWidgetItem* parentItem = item->parent() ? item->parent() : m_nodeTreeWidget->invisibleRootItem();
				removeItem(item);
				addNode(m_nodeTreeWidget, newNode, parentItem, true);
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
				if (Echo::IO::instance()->covertFullPathToResPath(savePath, resPath))
				{
					EchoEngine::instance()->saveBranchAsScene(resPath.c_str(), node);
					node->setPath(resPath);
					for (Echo::ui32 idx = 0; idx < node->getChildNum(); idx++)
					{
						node->getChild(idx)->setLink(true);
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

	// on discard instancing
	void NodeTreePanel::onDiscardInstancing()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			Echo::Node* node = getNode(item);
			node->setPath("");
			for (Echo::ui32 idx =0; idx < node->getChildNum(); idx++)
			{
				node->getChild(idx)->setLink(false);
			}

			refreshNodeDisplay(item);
		}
	}

	// refresh node display
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

	// node tree drag drop operator
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

	// when modifyd item name
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

	// import gltf scene
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

	// show object property when select a new object
	void NodeTreePanel::showSelectedObjectProperty()
	{
		m_propertyHelper.clear();
		m_propertyHelper.setHeader("Property", "Value");

		Echo::Object* object = getCurrentEditObject();
		if (object)
		{
			showObjectPropertyRecursive(object, object->getClassName());
			m_propertyHelper.applyTo(object->getName(), m_propertyTreeView, this, SLOT(refreshPropertyToObject(const QString&, QVariant)), false);
		}
		else
		{
			m_propertyHelper.applyTo("empty", m_propertyTreeView, this, SLOT(refreshPropertyToObject(const QString&, QVariant)), false);
		}	
	}

	// show property recursive
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
			m_propertyHelper.beginMenu(className.c_str());
			for (const Echo::PropertyInfo* prop : propertys)
			{
				Echo::Variant var;
				Echo::Class::getPropertyValue(classPtr, prop->m_name, var);

				showPropertyByVariant(classPtr, prop->m_name, var, prop);
			}
			m_propertyHelper.endMenu();
		}
	}

	// show property
	void NodeTreePanel::showPropertyByVariant(Echo::Object* object, const Echo::String& name, const Echo::Variant& var, const Echo::PropertyInfo* propInfo)
	{
		switch (var.getType())
		{
		case Echo::Variant::Type::Bool:			m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_CheckBox); break;
		case Echo::Variant::Type::Int:			m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_Int); break;
		case Echo::Variant::Type::Real:			m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_Real); break;
		case Echo::Variant::Type::String:		m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_String); break;
		case Echo::Variant::Type::Vector2:		m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_Vector2); break;
		case Echo::Variant::Type::Vector3:		m_propertyHelper.addItem(name.c_str(), var.toVector3(), QT_UI::WT_Vector3); break;
		case Echo::Variant::Type::Color:		m_propertyHelper.addItem(name.c_str(), var.toColor(), QT_UI::WT_ColorSelect); break;
		case Echo::Variant::Type::ResourcePath:	m_propertyHelper.addItem(name.c_str(), var.toResPath().getPath(), QT_UI::WT_AssetsSelect, var.toResPath().getSupportExts().c_str());break;
		case Echo::Variant::Type::StringOption: m_propertyHelper.addItem(name.c_str(), var.toStringOption().getValue(), QT_UI::WT_ComboBox, var.toStringOption().getOptionsStr().c_str()); break;
		case Echo::Variant::Type::NodePath:		m_propertyHelper.addItem(name.c_str(), var.toNodePath().getPath(), QT_UI::WT_NodeSelect, Echo::StringUtil::ToString(object->getId()).c_str()); break;
		case Echo::Variant::Type::Object:		m_propertyHelper.addItem(name.c_str(), var.toObj() ? var.toObj()->getId() : -1, QT_UI::WT_Res, propInfo->m_hintStr.c_str()); break;
		default:								m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_None); break;
		}
	}

	void NodeTreePanel::refreshPropertyToObject(const QString& property, QVariant value)
	{
		Echo::String propertyName = property.toStdString().c_str();
		Echo::String valStr = value.toString().toStdString().c_str();
		Echo::Object* object = getCurrentEditObject();
		Echo::Variant::Type type = Echo::Class::getPropertyType(object, propertyName);

		Echo::Variant propertyValue;
		if(propertyValue.fromString(type, valStr))
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

	void NodeTreePanel::onClickedNodeItem(QTreeWidgetItem* item, int column)
	{
		if (column == 1)
		{
			Echo::Node* node = getNode(item);
			if (node)
			{
				const Echo::String& path = node->getPath();
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

	// on select node
	void NodeTreePanel::onSelectNode()
	{
		// editor extension : unselect object
		if (m_currentEditObject && m_currentEditObject->getEditor())
		{
			m_currentEditObject->getEditor()->onEditorUnSelectThisNode();
		}

		m_currentEditObject = getCurrentSelectNode();
		showSelectedObjectProperty();

		// editor extension : select object
		if (m_currentEditObject && m_currentEditObject->getEditor())
		{
			m_currentEditObject->getEditor()->onEditorSelectThisNode();
		}

		// change subeditor 2d or 3d
		Echo::Render* renderNode = dynamic_cast<Echo::Render*>(m_currentEditObject);
		if (renderNode)
		{
			MainWindow::instance()->setSubEdit( renderNode->getRenderType().getIdx()==0 ? "2D" : "3D");
		}
	}

	// edit res
	void NodeTreePanel::onSelectRes(const Echo::String& resPath)
	{
		Echo::String extWithDot = Echo::PathUtil::GetFileExt(resPath, true);
		m_currentEditObject = Echo::Res::getResFunByExtension(extWithDot) ? Echo::Res::get(resPath) : nullptr;

		showSelectedObjectProperty();
	}

	// edit res
	void NodeTreePanel::onEditObject(Echo::Object* res)
	{
		m_currentEditObject = res;

		showSelectedObjectProperty();
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
