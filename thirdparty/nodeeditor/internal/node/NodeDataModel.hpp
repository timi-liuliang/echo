#pragma once


#include <QtWidgets/QWidget>

#include "PortType.hpp"
#include "NodeData.hpp"
#include "../base/Serializable.hpp"
#include "NodeGeometry.hpp"
#include "../style/NodeStyle.hpp"
#include "NodePainterDelegate.hpp"
#include "../base/Export.hpp"
#include "../base/memory.hpp"

namespace QtNodes
{
    enum class NodeValidationState
    {
      Valid,
      Warning,
      Error
    };

    class FlowScene;
    class Connection;
    class StyleCollection;
    class NODE_EDITOR_PUBLIC NodeDataModel : public QObject, public Serializable
    {
      Q_OBJECT

    public:
		enum class ConnectionPolicy
		{
			One,
			Many,
		};

    public:
        NodeDataModel();
        virtual ~NodeDataModel() = default;

        /// Caption is used in GUI
        virtual QString caption() const = 0;

        /// It is possible to hide caption in GUI
        virtual bool captionVisible() const { return true; }

        /// Port caption is used in GUI to label individual ports
        virtual QString portCaption(PortType, PortIndex) const { return QString(); }

        /// It is possible to hide port caption in GUI
        virtual bool portCaptionVisible(PortType, PortIndex) const { return false; }

        /// Name makes this model unique
        virtual QString name() const = 0;

    public:
        QJsonObject save() const override;

    public:
        virtual unsigned int nPorts(PortType portType) const = 0;

        virtual NodeDataType dataType(PortType portType, PortIndex portIndex) const = 0;

    public:
        // connection policy
        virtual ConnectionPolicy portOutConnectionPolicy(PortIndex) const {   return ConnectionPolicy::Many; }

        // node style
        NodeStyle const& nodeStyle() const;
        void setNodeStyle(NodeStyle const& style);

        // scene
        FlowScene* scene() { return m_scene; }
        void setScene(FlowScene* scene) { m_scene = scene; }

    public:
        /// Triggers the algorithm
        virtual void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) = 0;

        virtual std::shared_ptr<NodeData> outData(PortIndex port) = 0;

        virtual QWidget* embeddedWidget() = 0;

        virtual bool resizable() const { return false; }

        virtual NodeValidationState validationState() const { return NodeValidationState::Valid; }

        virtual QString validationMessage() const { return QString(""); }

        // paint delegate
        virtual NodePainterDelegate* painterDelegate() const { return nullptr; }

    public Q_SLOTS:
        // input connection
        virtual void inputConnectionCreated(Connection const&) {}
        virtual void inputConnectionDeleted(Connection const&) {}

        // output connection
        virtual void outputConnectionCreated(Connection const&) {}
        virtual void outputConnectionDeleted(Connection const&) {}

    Q_SIGNALS:
        // data change signal
        void dataUpdated(PortIndex index);
        void dataInvalidated(PortIndex index);

        // computing signal
        void computingStarted();
        void computingFinished();

        // widget size changed
        void embeddedWidgetSizeUpdated();

        // need update display
        void portUpdated();
        void captionUpdated();

     protected:
        NodeStyle   m_nodeStyle;
        FlowScene*  m_scene = nullptr;
    };
}
