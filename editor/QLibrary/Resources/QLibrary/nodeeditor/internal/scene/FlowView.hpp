#pragma once

#include <QtWidgets/QGraphicsView>
#include "../base/Export.hpp"

namespace QtNodes
{
    class FlowScene;
    class NODE_EDITOR_PUBLIC FlowView : public QGraphicsView
    {
        Q_OBJECT
        
    public:
        FlowView(QWidget *parent = Q_NULLPTR);
        FlowView(FlowScene *scene, QWidget *parent = Q_NULLPTR);

        FlowView(const FlowView&) = delete;
        FlowView operator=(const FlowView&) = delete;

        // delete|clear action
        QAction* clearSelectionAction() const;
        QAction* deleteSelectionAction() const;

        // flow scene
        void setScene(FlowScene *scene);
        FlowScene * scene();

    public Q_SLOTS:
        // scale
        void scaleUp();
        void scaleDown();

        // delete selected nodes
        void deleteSelectedNodes();

    protected:

        void contextMenuEvent(QContextMenuEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;

        void keyReleaseEvent(QKeyEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void drawBackground(QPainter* painter, const QRectF& r) override;

        void showEvent(QShowEvent *event) override;

    private:
        QAction*    _clearSelectionAction;
        QAction*    _deleteSelectionAction;
        QPointF     _clickPos;
        FlowScene*  _scene;
    };
}
