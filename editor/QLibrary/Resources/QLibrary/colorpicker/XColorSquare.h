#ifndef XCOLORSQUARE_H
#define XCOLORSQUARE_H

#include <QFrame>
#include <QColor>
#include <qmath.h>

class XColorSquare : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true STORED false )
    Q_PROPERTY(qreal hue READ hue WRITE setHue DESIGNABLE false )
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation DESIGNABLE false )
    Q_PROPERTY(qreal value READ value WRITE setValue DESIGNABLE false )

private:
    qreal huem, sat, val;
    qreal colorX, colorY;
    int nSquareWidth;
    QImage colorSquare;
    char    colorChar;

    enum CheckedColor
    {
        H,S,V,R,G,B
    }checkedColor;

    enum MouseStatus
    {
        Nothing,
        DragSquare
    }mouseStatus;

public:
    explicit XColorSquare(QWidget *parent = 0);

    void setSquareWidth(int width);

    QColor color() const;
    qreal hue() const {return huem;}
    qreal saturation() const {return sat;}
    qreal value() const {return val;}

public slots:
    void setColor(QColor c);
    void setHue(qreal h);
    void setSaturation(qreal s);
    void setValue(qreal v);

    void setCheckedColor(char checked);

signals:
    void colorChanged(QColor);
    void colorSelected(QColor);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    void RenderRectangle();

    QLineF lineToPoint(QPoint p) const
    {
        return QLineF(geometry().width()/2, geometry().height()/2, p.x(), p.y());
    }
};

#endif // XCOLORSQUARE_H
