#ifndef VOLUMEWIDGET_H
#define VOLUMEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core>
#include <QMatrix4x4>
#include <QTimer>
#include <QtMath>
#include <vector>
#include <QKeyEvent>
#include <QWheelEvent>

class Camera
{
    struct Position {
        float radius;
        float phi;
        float theta;
    };

public:
    Camera();
    QVector3D positionXYZ();
    Position positionPolar() { return position_; }

    void setRadius(float value) { position_.radius *= value; };
    void setPhi(float value)
    {
        position_.phi += value;
        if (position_.phi >= 180.0f) position_.phi = 179.99f;
        if (position_.phi <= 0.0f) position_.phi = 0.001f;
    };
    void setTheta(float value) { position_.theta += value; };

private:
    Position position_;

};

class VolumeWidget : public QOpenGLWidget, public QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    VolumeWidget(QWidget *parent);
    void setResolution(int row, int column);
    void addSliceImage(std::vector<uint16_t> image);
    void animate();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // Image
    std::vector<std::vector<uint16_t>> slice_;
    int imageWidth_;
    int imageHeight_;

    // OpenGL;
    GLuint program_;
    GLuint vao_[1];
    GLuint vbo_[2];
    GLuint texture_;
    GLuint mvp_;
    GLuint cameraPosision_;
    GLuint iso_;
    GLuint stepSize_;

    // Image & Viewer Params;
    float isoValue_ = 300.0f;
    GLfloat angle_ = 0.001f;
    GLfloat crane_ = 0.0f;
    GLfloat zoom_ = 2.0f;

    // Event
    bool onShift_ = false;

    void generateVolume();
    Camera camera_;

};

#endif // VOLUMEWIDGET_H
