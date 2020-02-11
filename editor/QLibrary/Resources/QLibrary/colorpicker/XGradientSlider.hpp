#ifndef XGRADIENTSLIDER_HPP
#define XGRADIENTSLIDER_HPP

#include <QSlider>

class XGradientSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QVector<QColor> colors READ colors WRITE setColors)
    Q_PROPERTY(QLinearGradient gradient READ gradient WRITE setGradient STORED false)
    Q_PROPERTY(QColor firstColor READ firstColor WRITE setFirstColor STORED false)
    Q_PROPERTY(QColor lastColor READ lastColor WRITE setLastColor STORED false)

private:
    QVector<QColor> col_list;
    QBrush back;

public:
    explicit XGradientSlider(QWidget *parent = 0);

    QBrush background() const { return back; }
    void setBackground(QBrush bg);

    QVector<QColor> colors() const { return col_list; }
    void setColors(QVector<QColor> bg);
    void setGradient(QLinearGradient bg);
    QLinearGradient gradient() const ;

    void setFirstColor(QColor c);
    void setLastColor(QColor c);
    QColor firstColor() const;
    QColor lastColor() const;
    
protected:
    void paintEvent(QPaintEvent *ev);
};

#endif // XGRADIENTSLIDER_HPP
