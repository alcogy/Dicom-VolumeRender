#include "volumewidget.h"

const char* vertexSource = R"#(
    #version 430
    layout (location = 0) in vec3 vertex;
    smooth out vec3 vertexPosition;
    uniform mat4 mvp;

    void main()
    {
        gl_Position = mvp * vec4(vertex, 1.0);
        vertexPosition = vertex + 0.5;
    }
)#";


const char* fragmentSource = R"#(
    #version 430
    layout(location = 0) out vec4 fragColor;
    smooth in vec3 vertexPosition;
    layout (binding = 0) uniform sampler3D volume;
    uniform vec3 cameraPosision;
    uniform vec3 stepSize;
    uniform float iso;
    const int MAX_SAMPLES = 1000;
    const float DELTA = 0.01;
    const float isoValue = iso / 65536;

    vec3 bisection(vec3 left, vec3 right, float iso)
    {
        for(int i = 0; i < 4; i++)
        {
            vec3 midpoint = (right + left) * 0.5;
            float cM = texture(volume, midpoint).r;
            if (cM < iso) left = midpoint;
            else right = midpoint;
        }
        return vec3(right + left) * 0.5;
    }

    vec3 gradient(vec3 uvw)
    {
        vec3 s1, s2;

        s1.x = texture(volume, uvw - vec3(DELTA, 0.0, 0.0)).r;
        s1.y = texture(volume, uvw - vec3(0.0, DELTA, 0.0)).r;
        s1.z = texture(volume, uvw - vec3(0.0, 0.0, DELTA)).r;

        s2.x = texture(volume, uvw + vec3(DELTA, 0.0, 0.0)).r;
        s2.y = texture(volume, uvw + vec3(0.0, DELTA, 0.0)).r;
        s2.z = texture(volume, uvw + vec3(0.0, 0.0, DELTA)).r;

        return normalize((s1 - s2) / 2.0);
    }

    void main()
    {
        vec3 dataPosision = vertexPosition;
        vec3 direction = normalize((vertexPosition - vec3(0.5)) - cameraPosision);
        vec3 dirStep = direction * stepSize;

        for (int i = 0; i < MAX_SAMPLES; i++)
        {
            dataPosision = dataPosision + dirStep;
            if (dot(sign(dataPosision - vec3(0.0)), sign(vec3(1.0) - dataPosision)) < 3.0) break;

            float sample1 = texture(volume, dataPosision).r;
            float sample2 = texture(volume, dataPosision + dirStep).r;

            if ((sample1 - isoValue) < 0 && (sample2 - isoValue) >= 0.0)
            {
                vec3 left = dataPosision;
                vec3 right = dataPosision + dirStep;
                vec3 tc = bisection(left, right, isoValue);
                vec3 N = gradient(tc);
                vec3 Lambert = vec3(max(dot(-direction, N), 0.0));
                fragColor = vec4(Lambert, 1.0);
                break;
            }
        }
    }
)#";


const GLfloat vertices[24] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
};

const GLushort indices[36]={
    0, 5, 4,
    5, 0, 1,
    3, 7, 6,
    3, 6, 2,
    7, 4, 6,
    6, 4, 5,
    2, 1, 3,
    3, 1, 0,
    3, 0, 7,
    7, 0, 4,
    6, 5, 2,
    2, 5, 1
};

VolumeWidget::VolumeWidget(QWidget *parent): QOpenGLWidget(parent)
{
    setFixedSize(800, 800);
    setAutoFillBackground(false);
    this->setFocusPolicy(Qt::StrongFocus);
}

void VolumeWidget::setResolution(int row, int column)
{
    imageHeight_ = row;
    imageWidth_ = column;
}

void VolumeWidget::addSliceImage(std::vector<uint16_t> image)
{
    slice_.push_back(image);
}

void VolumeWidget::animate()
{
    update();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
}

void VolumeWidget::initializeGL()
{
    initializeOpenGLFunctions();

    program_ = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);

    mvp_ = glGetUniformLocation(program_, "mvp");
    cameraPosision_ = glGetUniformLocation(program_, "cameraPosision");
    iso_ = glGetUniformLocation(program_, "iso");
    stepSize_ = glGetUniformLocation(program_, "stepSize");

    glGenVertexArrays(1, vao_);
    glGenBuffers(2, vbo_);

    glBindVertexArray(vao_[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    generateVolume();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

}

void VolumeWidget::resizeGL(int w, int h)
{}

void VolumeWidget::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QVector3D camera = camera_.positionXYZ();
    QVector3D dist(0.0f, 0.0f, 0.0f);
    QVector3D up(0.0f, 1.0f, 0.0f);

    QMatrix4x4 view;
    view.lookAt(camera, dist, up);

    QMatrix4x4 projection;
    projection.perspective(60.0f, 1.0f, 0.1f, 1000.0f);

    QMatrix4x4 mvp = projection * view;

    glEnable(GL_BLEND);

    glBindVertexArray(vao_[0]);

    glUseProgram(program_);

    glUniformMatrix4fv(mvp_, 1, GL_FALSE, mvp.data());
    glUniform3f(cameraPosision_, camera.x(), camera.y(), camera.z());
    glUniform3f(stepSize_, 1.0f / imageWidth_, 1.0f / imageHeight_, 1.0f / slice_.size());
    glUniform1f(iso_, isoValue_);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

}

void VolumeWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_PageUp:
        {
            int value = onShift_ ? 10 : 1;
            float newValue = isoValue_ + value;
            isoValue_ = newValue >= 65536 ? 65536 : newValue;
        }
        break;

        case Qt::Key_PageDown:
        {
            int value = onShift_ ? 10 : 1;
            float newValue = isoValue_ - value;
            isoValue_ = newValue <= 0 ? 0 : newValue;
        }
        break;

        case Qt::Key_Left:
        {
            camera_.setTheta(onShift_ ? 2.0f : 0.5f);
        }
        break;

        case Qt::Key_Right:
        {
            camera_.setTheta(onShift_ ? -2.0f : -0.5f);
        }
        break;

        case Qt::Key_Up:
        {
            camera_.setPhi(onShift_ ? -2.0f : -0.5f);
        }
        break;

        case Qt::Key_Down:
        {
            camera_.setPhi(onShift_ ? 2.0f : 0.5f);
        }
        break;

        case Qt::Key_Shift:
        {
            onShift_ = true;
        }
        break;
    }
}

void VolumeWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Shift:
        {
            onShift_ = false;
        }
        break;
    }
}

void VolumeWidget::wheelEvent(QWheelEvent *event)
{

    if (event->pixelDelta().y() < 0) camera_.setRadius(1.2f);
    else camera_.setRadius(0.9f);
}

void VolumeWidget::generateVolume()
{
    int size = slice_.size();
    std::vector<GLushort> texValue(imageWidth_ * imageHeight_ * imageHeight_);

    unsigned long count = 0;
    for (int depth = 0; depth < size; depth++)
    {
        for (int row = 0; row < imageHeight_; row++)
        {
            for (int column = 0; column < imageWidth_; column++)
            {
                texValue[count++] = (GLushort) slice_[depth][(row * imageHeight_) + column];
            }
        }
    }

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_3D, texture_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, imageWidth_, imageHeight_, imageHeight_, 0, GL_RED, GL_UNSIGNED_SHORT, texValue.data());
    glGenerateMipmap(GL_TEXTURE_3D);

}

Camera::Camera()
{
    position_.radius = 1.0f;
    position_.phi = 179.99f;
    position_.theta = 0.0f;
}

QVector3D Camera::positionXYZ()
{
    float radTheta = qDegreesToRadians(position_.theta);
    float radPhi = qDegreesToRadians(position_.phi);

    float x = position_.radius * sin(radPhi) * sin(radTheta);
    float y = position_.radius * cos(radPhi);
    float z = position_.radius * sin(radPhi) * cos(radTheta);

    return QVector3D(x, y, z);
}
