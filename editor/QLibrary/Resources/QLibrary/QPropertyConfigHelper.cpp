#include "QPropertyConfigHelper.h"
#include <engine/core/Util/StringUtil.h>
#include "QProperty.hpp"

namespace QT_UI
{
	QPropertyConfigHelper::QPropertyConfigHelper()
		: m_curDepth(0)
		, m_property(nullptr)
	{
		m_idxs.assign(0);
	}

	QPropertyConfigHelper::~QPropertyConfigHelper() 
	{
		EchoSafeDelete(m_property, QProperty);
	}

	void QPropertyConfigHelper::setHeader(const char* header0, const char* header1, const char* header2, const char* header3)
	{
		Echo::String header = Echo::StringUtil::Format("<cfg><headers><header text=\"%s\" /><header text=\"%s\" />", header0, header1);

		if (header2)
			header += Echo::StringUtil::Format("<header text = \"%s\" />", header2);

		if (header3)
			header += Echo::StringUtil::Format("<header text = \"%s\" />", header3);

		header += "</headers>";
		m_result = header;
	}

	void QPropertyConfigHelper::beginMenu(const char* text)
	{
		Echo::ui32& curIdx = m_idxs[m_curDepth];
		Echo::String format = Echo::StringUtil::Format("<item text=\"%s\" row=\"%d\" col=\"0\" bold=\"1\">", text, curIdx);
		m_result += format;

		curIdx++;
		m_curDepth++;
	}

	void QPropertyConfigHelper::endMenu()
	{
		Echo::ui32& curIdx = m_idxs[m_curDepth];
		curIdx = 0;

		m_result += "</item>";
		m_curDepth--;

		EchoAssert(m_curDepth >= 0);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, float value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, const Echo::ui32 value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, const Echo::i32 value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, bool value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, const Echo::Vector3& value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, const Echo::Color& value, WidgetType widget, const char* widgetParams)
	{
		addItem(propertyName, Echo::StringUtil::ToString(value), widget, widgetParams);
	}

	void QPropertyConfigHelper::addItem(const char* propertyName, const Echo::String& value, WidgetType widget, const char* widgetParams)
	{
		Echo::ui32& curIdx = m_idxs[m_curDepth];

		// 根据"_" 取最长的字符串
		Echo::StringArray displayArr = Echo::StringUtil::Split(propertyName, ".");
		Echo::String displayText = displayArr.back();

		// 组织控件字符串
		Echo::String widgetStr = FormatUI(widget, widgetParams);
		Echo::String format = Echo::StringUtil::Format("<item text=\"%s\" row=\"%d\" col=\"0\" /><item text=\"\" row=\"%d\" col=\"1\" property=\"%s\" widget=\"%s\" />", displayText.c_str(), curIdx, curIdx, propertyName, widgetStr.c_str());
		m_result += format;

		curIdx++;

		m_values[QString::fromLocal8Bit(propertyName)] = value.c_str();
	}

	void QPropertyConfigHelper::addItem_c2(const char* propertyName, const Echo::String& value, WidgetType widget, const char* widgetParams)
	{
		Echo::ui32& curIdx = m_idxs[m_curDepth];

		// 组织控件字符串
		Echo::String widgetStr = FormatUI( widget, widgetParams);

		Echo::String propertyNameEx = Echo::StringUtil::Format("%s_co_%d", propertyName, 2);
		Echo::String format = Echo::StringUtil::Format("<item text=\"\" row=\"%d\" col=\"%d\" property=\"%s\" widget=\"%s\" />", curIdx-1, 2, propertyNameEx.c_str(), widgetStr.c_str());
		m_result += format;

		m_values[QString::fromLocal8Bit(propertyNameEx.c_str())] = value.c_str();
	}

	void QPropertyConfigHelper::addItem_c3(const char* propertyName, const Echo::String& value, WidgetType widget, const char* widgetParams)
	{
		Echo::ui32& curIdx = m_idxs[m_curDepth];

		// 组织控件字符串
		Echo::String widgetStr = FormatUI(widget, widgetParams);

		Echo::String propertyNameEx = Echo::StringUtil::Format("%s_co_%d", propertyName, 3);
		Echo::String format = Echo::StringUtil::Format("<item text=\"\" row=\"%d\" col=\"%d\" property=\"%s\" widget=\"%s\" />", curIdx - 1, 3, propertyNameEx.c_str(), widgetStr.c_str());
		m_result += format;

		m_values[QString::fromLocal8Bit(propertyNameEx.c_str())] = value.c_str();
	}

	Echo::String QPropertyConfigHelper::FormatUI(WidgetType widget, const char* widgetParams)
	{
		// 组织控件字符串
		Echo::String widgetStr;
		switch (widget)
		{
		case WT_Int:			widgetStr = "Int";			 break;
		case WT_Real:			widgetStr = "Real";			 break;
		case WT_Vector2:		widgetStr = "Vector2";		 break;
		case WT_Vector3:		widgetStr = "Vector3";		 break;
		case WT_CheckBox:		widgetStr = "CheckBox";		 break;
		case WT_ColorSelect:	widgetStr = "ColorSelect";	 break;
		case WT_ComboBox:		widgetStr = Echo::String("ComboBox,") + widgetParams;		break;
		case WT_AssetsSelect:	widgetStr = Echo::String("AssetsSelect,") + widgetParams;	break;
		case WT_Res:			widgetStr = Echo::String("ResEdit,") + widgetParams;		break;
		default:				widgetStr = "";				 break;
		}

		return widgetStr;
	}

	Echo::String QPropertyConfigHelper::getResult() 
	{
		return m_result + "</cfg>";
	}

	void QPropertyConfigHelper::applyTo(const Echo::String& id, QTreeView* treeView, const QObject* receiver, const char* memeber, bool clear)
	{
		// 断开消息链接
		if (m_property && m_property->m_model)
		{
			QObject::disconnect(m_signalConn);
		}

		// 刷新显示
		if (m_lastResult != getResult() || ( m_property && treeView->model() != m_property->m_model) || m_id!=id)
		{
			// 删除之
			EchoSafeDelete(m_property, QProperty);

			static Echo::ui32 idx = 0; idx++;

			// property create.
			m_property = (EchoNew(QT_UI::QProperty(getResult().c_str(), nullptr, true)));
			m_property->m_model->setID(Echo::StringUtil::Format("property_model_%d", idx).c_str());
			treeView->setModel(m_property->m_model);
			treeView->setItemDelegate(m_property->m_delegate);
			treeView->setAlternatingRowColors(true);
			treeView->expandAll();
			treeView->resizeColumnToContents(0);

			m_lastResult = getResult();

			m_id = id;
		}

		// 应用所有初始值
		for ( auto& iter : m_values)
		{
			m_property->m_model->setValue(iter.first, iter.second);
		}

		// 重置索引
		m_curDepth = 0;
		m_idxs.assign(0);
		if (clear)
		{
			m_result.clear();
		}

		// 消息链接
		if (memeber)
			m_signalConn = QObject::connect(m_property->m_model, SIGNAL(Signal_ValueChanged(const QString&, QVariant,int, int)), receiver, memeber);
	}
}