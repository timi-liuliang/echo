#ifndef XCOLORPREVIEW_HPP
#define XCOLORPREVIEW_HPP

#include <QWidget>

class XColorPreview : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true)
    Q_PROPERTY(Alpha_Mode alpha_mode READ alphaMode WRITE setAlphaMode DESIGNABLE true)
    Q_PROPERTY(QBrush background READ getBackground WRITE setBackground DESIGNABLE true)
    Q_ENUMS(Alpha_Mode)
public:
    enum Alpha_Mode
    {
        NoAlpha,
        SplitAlpha,
    };

private:
    QColor col;
    QBrush back;
    Alpha_Mode alpha_mode;
    QColor colorPrevious;

public:
    explicit XColorPreview(QWidget *parent = 0);

    void setPreviousColor(QColor colorPre);

    void setBackground(QBrush bk)
    {
        back = bk;
        update();
    }

    QBrush getBackground() const
    {
        return back;
    }

    Alpha_Mode alphaMode() const
    {
        return alpha_mode;
    }

    void setAlphaMode(Alpha_Mode am)
    {
        alpha_mode = am;
        update();
    }


    QColor color() const
    {
        return col;
    }

    QSize sizeHint () const;
    
public slots:
    void setColor(QColor c);

signals:
    void colorChanged(QColor);

protected:
    void paintEvent(QPaintEvent *);

};

#endif // XCOLORPREVIEW_HPP
