#ifndef QYUVOPENGLWIDGET_H
#define QYUVOPENGLWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class QYUVOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit QYUVOpenGLWidget(QWidget *parent = nullptr);
    virtual ~QYUVOpenGLWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setFrameSize(const QSize& frameSize);
    void setFrameVideoSize(const QSize& frameSize);
    const QSize& frameSize();
    void updateTextures(const quint8* dataY, const quint8* dataU, const quint8* dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);
    void updateVideoTextures(const quint8* dataY, const quint8* dataU, const quint8* dataV, quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:    
    void initShader();
    void initTextures();
    void deInitTextures();
    void updateTexture(bool video, GLuint texture, quint32 textureType, const quint8* pixels, quint32 stride);

private:
    // 视频帧尺寸
    QSize m_frameSize = {-1, -1};
    QSize m_frameVideoSize = {-1, -1};
    bool m_needUpdate = false;
    bool m_textureInited = false;

    // 顶点缓冲对象(Vertex Buffer Objects, VBO)：默认即为VertexBuffer(GL_ARRAY_BUFFER)类型
    QOpenGLBuffer m_vbo;

    // 着色器程序：编译链接着色器
    QOpenGLShaderProgram m_shaderProgram;    

    // YUV纹理，用于生成纹理贴图
    GLuint m_texture[3] = {0};
    GLuint m_textureVideo[3] = {0};
};

#endif // QYUVOPENGLWIDGET_H
