#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  //Q_OBJECT

public:
  ViewWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  QVector<GLfloat> createPolygon(float x, float y, float z, float radius, int sides);
protected:
  void initializeGL() override;
  void paintGL() override;
private slots:
  void updateTurntable();
private:
  float m_turntableAngle = 0.0f;

  QVector<float> m_points;
  QVector<float> m_colors;

  QOpenGLShaderProgram m_pointProgram;
};

#endif // VIEWWIDGET_H
