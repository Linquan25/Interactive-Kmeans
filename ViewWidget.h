#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  //Q_OBJECT

public:
  ViewWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  QVector<GLfloat> createPolygon(float x, float y, float z, float radius, int sides);

  float angleForTime(qint64 msTime, float secondsPerRotation) const;
protected:
  void initializeGL() override;
  void paintGL() override;
public slots:
  void updateTurntable();
  void generatePoints(int dimension, int sampleNumber);
  void generatePointsFromFile(QString dir);
  void kmeans_initial(int k, int mode);
  void kmeans_step();
  void kmeans_setpBack();
  void kmeans_runthrough();
  float euclideanDistance(int centroid_index, int point_index);
  void mapColor(int point_index, int colormap_index);
  void updateCentroids(QVector<float> coor, int count, int centroid_index);
  void setMovieOn(bool checked);
  void setPointsOn(bool checked);
  void setAxisOn(bool checked);
  void setCentroidsOn(bool checked);
  void setXRotation(int angle);
  void setYRotation(int angle);
  void setZRotation(int angle);
  void setZooming(int zoomLevel);
  void clearPoints();
  void calculatePointsNDVisual();
  void calculateCentroidsNDVisual();
  float energyCalculation();
  void setPointSize(float size);
  void setCentroidSize(float size);
  void setPanningX(float d);
  void setPanningY(float d);
private:
  QVector<float> colormapGenerator(int size);
  QElapsedTimer m_elapsedTimer;
  QElapsedTimer m_fpsTimer;
  int m_frameCount = 0;
  float m_fps;
  int m_K = 0;
  float m_turntableAngle = 0.0f;
  int m_dimension = 3;
  int m_pointNumber = 0;
  int m_iteration = 0;
  float m_energy = 0.0;
  bool m_dirty = true;
  QVector<float> m_points;
  QVector<float> m_colors;
  QVector<float> m_centroidsColor;
  QVector<float> m_colorMaps;
  QVector<float> m_centroids;
  QVector<float> m_centroids_history;
  QVector<float> m_centroids_history_history;
  QVector<int> m_class;
  QVector<float> m_pointsNDVisual;
  QVector<float> m_centroidsNDVisual;
  QOpenGLShaderProgram m_pointProgram;

  bool m_pointsOn = true;
  bool m_centroidsOn = true;
  bool m_axisOn = true;
  bool m_movieOn = false;
  float m_pointSize = 1.0f;
  float m_centroidSize = 1.0f;
  float m_xRotation = 0.0f;
  float m_yRotation = 0.0f;
  float m_zRotation = 0.0f;
  float m_zooming = 0.0f;
  float x_panning = 0.0f;
  float y_panning = 0.0f;
};

#endif // VIEWWIDGET_H
