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
  void generatePoints();
  void kmeans_initial(int mode);
  void kmeans_step();
  float euclideanDistance(int centroid_index, int point_index);
  void mapColor(int point_index, int colormap_index);
  void updateCentroids(QVector<float> coor, int count, int centroid_index);
private:
  int m_K = 3;
  float m_turntableAngle = 0.0f;
  int m_dimension = 2;
  int m_pointNumber = 300;
  bool m_dirty = true;
  QVector<float> m_points;
  QVector<float> m_colors;
  QVector<float> m_colorMaps = {255.0, 0.0, 0.0, 0.0, 255.0, 0.0, 0.0, 0.0, 255.0};
  QVector<float> m_centroids;
  QVector<int> m_class;
  QOpenGLShaderProgram m_pointProgram;
};

#endif // VIEWWIDGET_H
