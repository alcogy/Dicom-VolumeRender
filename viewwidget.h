#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <vector>

class ViewWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    ViewWidget(QWidget *parent, std::vector<uint16_t> image);
    void render();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<uint16_t> image_;

};

#endif // VIEWWIDGET_H
