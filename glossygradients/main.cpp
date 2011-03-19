/****************************************************************************
**
** Copyright (C) 2008-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Graphics Dojo project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QtGui>

QLinearGradient getGradient(const QColor &col, const QRectF &rect)
{
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());

    qreal hue = col.hueF();
    qreal value = col.valueF();
    qreal saturation = col.saturationF();

    QColor c = col;
    c.setHsvF(hue, 0.42 * saturation, 0.98 * value);
    g.setColorAt(0, c);
    c.setHsvF(hue, 0.58 * saturation, 0.95 * value);
    g.setColorAt(0.25, c);
    c.setHsvF(hue, 0.70 * saturation, 0.93 * value);
    g.setColorAt(0.5, c);

    c.setHsvF(hue, 0.95 * saturation, 0.9 * value);
    g.setColorAt(0.501, c);
    c.setHsvF(hue * 0.95, 0.95 * saturation, 0.95 * value);
    g.setColorAt(0.75, c);
    c.setHsvF(hue * 0.90, 0.95 * saturation, 1 * value);
    g.setColorAt(1.0, c);

    return g;
}

QLinearGradient darken(const QLinearGradient &gradient)
{
    QGradientStops stops = gradient.stops();
    for (int i = 0; i < stops.size(); ++i) {
        QColor color = stops.at(i).second;
        stops[i].second = color.darker(160);
    }

    QLinearGradient g = gradient;
    g.setStops(stops);
    return g;
}

void drawPath(QPainter *p, const QPainterPath &path,
              const QColor &col, const QString &name,
              bool dark = false)
{
    const QRectF pathRect = path.boundingRect();

    const QLinearGradient baseGradient = getGradient(col, pathRect);
    const QLinearGradient darkGradient = darken(baseGradient);

    p->save();

    p->setOpacity(0.25);

    // glow
    if (dark)
        p->strokePath(path, QPen(darkGradient, 6));
    else
        p->strokePath(path, QPen(baseGradient, 6));

    p->setOpacity(1.0);

    // fill
    if (dark)
        p->fillPath(path, darkGradient);
    else
        p->fillPath(path, baseGradient);

    QLinearGradient g(pathRect.topLeft(), pathRect.topRight());
    g.setCoordinateMode(QGradient::ObjectBoundingMode);

    p->setOpacity(0.2);
    p->fillPath(path, g);

    p->setOpacity(0.5);

    // highlight
    if (dark)
        p->strokePath(path, QPen(col.lighter(160).darker(160), 2));
    else
        p->strokePath(path, QPen(col.lighter(160), 2));

    p->setOpacity(1.0);

    // text
    QFont font;
    font.setBold(true);
    p->setFont(font);
    p->setPen(Qt::white);

    const QRectF textBoundingRect = p->boundingRect(QRectF(), 0, name);
    p->translate(pathRect.center());
    p->translate(-textBoundingRect.center());
    p->drawText(textBoundingRect, name);
    p->restore();
}

class ColorButton : public QPushButton
{
public:
    ColorButton(const QColor &col, const QString &text, QWidget *parent = 0);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    QSize sizeHint() const;

    QColor color() const { return m_color; }
    void setColor(const QColor &color);

private:
    void invalidateCache();

    QColor m_color;
    qreal m_buttonRadius;

    QPixmap m_cached[2];
};

ColorButton::ColorButton(const QColor &col, const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_color(col)
    , m_buttonRadius(15)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainterPath path;
    path.addRoundedRect(3, 3, width()-6, height()-6, m_buttonRadius, m_buttonRadius);

    bool down = isDown();

    if (m_cached[down].isNull()) {
        QPixmap pix(width(), height());
        pix.fill(Qt::transparent);

        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing, true);
        drawPath(&p, path, m_color, text(), down);
        p.end();

        m_cached[down] = pix;
    }

    QPainter p(this);
    p.drawPixmap(0, 0, m_cached[down]);
}

void ColorButton::resizeEvent(QResizeEvent *)
{
    invalidateCache();
}

QSize ColorButton::sizeHint() const
{
    return QSize(120, 100);
}

void ColorButton::setColor(const QColor &color)
{
    m_color = color;
    invalidateCache();
    update();
}

void ColorButton::invalidateCache()
{
    m_cached[0] = QPixmap();
    m_cached[1] = QPixmap();
}

class View : public QGraphicsView
{
    Q_OBJECT
public:
    View(QWidget *parent = 0);

public slots:
    void animate();
    void update(qreal time);
    void chooseNewColor();

private:
    QWidget *m_widget;
    QGraphicsScene *m_scene;
    QGraphicsProxyWidget *m_item;
    QVector<ColorButton *> m_buttons;

    QTimeLine *m_rotation;
    int m_animationStage;
};

void View::chooseNewColor()
{
    if (!sender())
        return;

    ColorButton *button = static_cast<ColorButton *>(sender());

    QColor color = QColorDialog::getColor(button->color(), this);
    if (color.isValid())
        button->setColor(color);
}

QColor colors[] = { Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow,
                    QColor(255, 183, 230), QColor(160, 200, 120), QColor(150, 85, 79) };
const char *names[] = { "Red", "Green", "Blue", "Cyan", "Magenta", "Yellow",
                        "Custom 1", "Custom 2", "Custom 3" };

View::View(QWidget *parent)
    : QGraphicsView(parent)
    , m_widget(new QWidget)
    , m_animationStage(0)
{
    QPalette palette;
    palette.setColor(QPalette::Window, Qt::transparent);

    viewport()->setAutoFillBackground(false);
    m_widget->setPalette(palette);

    QGridLayout *layout = new QGridLayout(m_widget);

    for (int i = 0; i < 9; ++i) {
        const int x = i % 3;
        const int y = i / 3;

        m_buttons << new ColorButton(colors[i], QLatin1String(names[i]));
        layout->addWidget(m_buttons.last(), y, x);

        if (i >= 6)
            connect(m_buttons.last(), SIGNAL(clicked()), this, SLOT(chooseNewColor()));
        else
            connect(m_buttons.last(), SIGNAL(clicked()), this, SLOT(animate()));

        if ((x + y) & 1)
            m_buttons.last()->hide();
    }

    m_scene = new QGraphicsScene;
    setScene(m_scene);

    setFrameShape(QFrame::NoFrame);

    m_item = m_scene->addWidget(m_widget);
    m_item->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-10, -40, 10, 40));
    m_rotation = new QTimeLine(200);

    setRenderHints(QPainter::SmoothPixmapTransform);

    connect(m_rotation, SIGNAL(valueChanged(qreal)), this, SLOT(update(qreal)));
    connect(m_rotation, SIGNAL(finished()), this, SLOT(animate()));

    m_rotation->setUpdateInterval(20);
}

void View::animate()
{
    if (m_rotation->state() == QTimeLine::Running) {
        QTimer::singleShot(20, this, SLOT(animate()));
        return;
    }

    int stage = m_animationStage % 3;
    if (stage == 1) {
        for (int i = 0; i < 9; ++i) {
            const int x = i % 3;
            const int y = i / 3;
            m_buttons[i]->setVisible(!((x + y) & 1) ^ ((m_animationStage % 6) == 1));
        }
    }

    ++m_animationStage;
    if (stage != 2) {
        m_rotation->setCurveShape(QTimeLine::LinearCurve);
        m_rotation->start();
    }
}

void View::update(qreal time)
{
    int stage = (m_animationStage % 3) - 1;

    QTransform transform;
    QPointF delta = m_scene->sceneRect().center();
    transform.translate(delta.x(), delta.y());
    transform.rotate(90 * (stage + time), Qt::YAxis);
    transform.scale(1 - 2 * stage, 1);
    transform.translate(-delta.x(), -delta.y());

    m_item->setTransform(transform);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget widget(0, Qt::FramelessWindowHint);
    widget.setAttribute(Qt::WA_NoSystemBackground);
    new QGridLayout(&widget);
    widget.layout()->addWidget(new View(&widget));
    widget.show();

    return app.exec();
}

#include "main.moc"
