#include "ViewWidget.h"
#include <chrono>
#include <random>
#include <QOpenGLShaderProgram>
#include <QtMath>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  turntableTimer->start(1000.0/30.0);
  m_elapsedTimer.start();

  //generatePoints();
  //kmeans_initial(2);
  //kmeans_step();
}

static const char *vertexShaderCode_points =
    "attribute highp vec4 vertex;\n"
    "attribute mediump vec4 color;\n"
    "varying mediump vec4 vColor;\n"
    "uniform highp mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "   gl_Position = matrix * vertex;\n"
    "   vColor = color;\n"
    "}";

static const char *fragmentShaderCode_points =
    "varying mediump vec4 vColor;\n"
    "void main(void)\n"
    "{\n"
    "   gl_FragColor = vColor;\n"
    "}";

static const char *vertexShaderCode_axis =
    "attribute highp vec4 vertex;\n"
    "uniform highp mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "   gl_Position = matrix * vertex;\n"
    "}";

static const char *fragmentShaderCode_axis =
    "uniform mediump vec4 color;\n"
    "void main(void)\n"
    "{\n"
    "   gl_FragColor = color;\n"
    "}";

QVector<GLfloat> ViewWidget::createPolygon(float x, float y, float z, float radius, int sides)
{
  QVector<GLfloat> result;
  float a = -M_PI_2;
  for(int i=0; i<sides;++i){
    result.push_back(x+radius * qCos(a));
    result.push_back(y+radius * qSin(a));
    result.push_back(z);
    a += 2.0 * M_PI/sides;
  }
  return result;
}

float ViewWidget::angleForTime(qint64 msTime, float secondsPerRotation) const
{
  float millisecondsPerRotation = secondsPerRotation * 1000.0;
  float t = msTime/millisecondsPerRotation;

  return (t - qFloor(t)) * 360.0;
}

void ViewWidget::initializeGL()
{
 initializeOpenGLFunctions();

 glClearColor(0.2,0.2,0.2,1.0);

 glEnable(GL_DEPTH_TEST);
 glEnable(GL_POINT_SMOOTH);
 //glPointSize(5.0);

 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
   vertexShaderCode_points);

 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
   fragmentShaderCode_points);

 m_pointProgram.link();
}

void ViewWidget::paintGL()
{
  QOpenGLShaderProgram program;
  program.addShaderFromSourceCode(QOpenGLShader::Vertex,
       vertexShaderCode_axis);
   program.addShaderFromSourceCode(QOpenGLShader::Fragment,
       fragmentShaderCode_axis);
   program.link();
   program.bind();

   int vertexLocation = program.attributeLocation("vertex");
   int matrixLocation = program.uniformLocation("matrix");

   QMatrix4x4 pmvMatrix;
   //pmvMatrix.ortho(rect());
   //pmvMatrix.ortho(10, 110, 110, 10, -1, 1);float(width())/height()
   pmvMatrix.perspective(60,float(width())/height(), 1.0f, 1500.0f);
   pmvMatrix.lookAt({0,0,5+m_zooming},{0,0,0},{0,1,0});
   if(m_movieOn){
     pmvMatrix.rotate(angleForTime(m_elapsedTimer.elapsed(),15), {0.0f, 1.0f, 0.0f});
   }
   pmvMatrix.rotate(m_xRotation, {1.0f, 0.0f, 0.0f});
   pmvMatrix.rotate(m_yRotation, {0.0f, 1.0f, 0.0f});
   pmvMatrix.rotate(m_zRotation, {0.0f, 0.0f, 1.0f});


   m_pointProgram.bind();
   m_pointProgram.enableAttributeArray("vertex");
   m_pointProgram.enableAttributeArray("color");

   m_pointProgram.setUniformValue("matrix", pmvMatrix);
   m_pointProgram.setAttributeArray("vertex", m_points.constData(),m_dimension);
   m_pointProgram.setAttributeArray("color", m_colors.constData(),3);

   if(m_pointsOn) glDrawArrays(GL_POINTS, 0, m_points.count()/m_dimension);

   m_pointProgram.setAttributeArray("vertex", m_centroids.constData(),m_dimension);
   m_pointProgram.setAttributeArray("color", m_colorMaps.constData(),3);

   if(m_centroidsOn) glDrawArrays(GL_POINTS, 0, m_K);

   m_pointProgram.disableAttributeArray("vertex");
   m_pointProgram.disableAttributeArray("color");

   program.bind();
   program.setUniformValue(matrixLocation, pmvMatrix);
   program.enableAttributeArray(vertexLocation);
   GLfloat axisWidth = 1.0;
   GLfloat axisLength = 500.0;
   glLineWidth(axisWidth);
   if(m_axisOn){
     GLfloat const xAxis[] = {0.0f, 0.0f, 0.0f,
                             axisLength, 0.0f,0.0f};
     program.setAttributeArray(vertexLocation, xAxis, 3);
     program.setUniformValue("color",QColor(171,171,171,255));
     glDrawArrays(GL_LINES, 0, 2);
     GLfloat const yAxis[] = {0.0f, 0.0f, 0.0f,
                             0.0f, axisLength, 0.0f};
     program.setAttributeArray(vertexLocation, yAxis, 3);
     //program.setUniformValue("color",QColor(255,0,255,255));
     glDrawArrays(GL_LINES, 0, 2);
     GLfloat const zAxis[] = {0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, axisLength};
     program.setAttributeArray(vertexLocation, zAxis, 3);
     //program.setUniformValue("color",QColor(255,0,0,255));
     glDrawArrays(GL_LINES, 0, 2);
   }

   program.disableAttributeArray(vertexLocation);
}

QVector<float> ViewWidget::colormapGenerator(int size)
{
  QVector<float> output;
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(0.0, 1.0);

  for(int i=0; i<size*3; i++){
    output.append(distribution(engine));
  }
  return output;
}

void ViewWidget::updateTurntable()
{
  //m_turntableAngle += 1.0f;

  update();
}

void ViewWidget::generatePoints(int dimension, int sampleNumber)
{
  m_dimension = dimension;
  m_pointNumber = sampleNumber;
  // Create random 3D points
  // Get seed from clock
  long seed = std::chrono::system_clock::now().time_since_epoch().count();

  // Seed engine and set random distribution to [-1, 1]
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0, 1.0);

  // Create points inside a sphere
  int count = 0;
  while(count < m_pointNumber)
  {
    // Uniformly sample cube
    for (int i=0; i<m_dimension; i++) {
      float x = distribution(engine);
      m_points.append({x});
    }
    count++;
  }
  m_colors = QVector<float>(m_pointNumber * 3, 1.0f);
}

void ViewWidget::generatePointsFromFile(QString dir)
{
  QFile file(dir);

  if(!file.open(QFile::ReadOnly | QFile::Text)){
    QMessageBox::warning(this,"title","File openning failed!");
  }
  QTextStream in(&file);
  QString text = in.readLine();
  m_pointNumber = text.toInt();
  text = in.readLine();
  m_dimension = text.toInt();
  QVector<float> temp;
  while(!in.atEnd()){
    text = in.readLine();
    for (int i = 0; i < m_dimension; i++) {
      temp.append(text.split(" ")[i].toFloat());
    }
  }
  m_points = temp;
  m_colors = QVector<float>(m_pointNumber * 3, 1.0f);
  file.close();
}

void ViewWidget::kmeans_step()
{
  qDebug()<<"Here is in Step function";
  // Seed engine and set random distribution to [-1, 1]
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0, 1.0);
  //clustering part
  int iteration = 0;
    iteration++;
    //distance calculation and clusering
    for (int i = 0 ; i < m_pointNumber; i++) {
      float min = 999999.9f;
      for (int j = 0; j < m_K; j++) {
        float distance = euclideanDistance(j, i);
        if(distance < min){
          min = distance;
          m_class[i]=j;
          mapColor(i,j);
        }else if(distance == min){
          //Equal distance 50% chance change class
          if (distribution(engine)>0){
            m_class[i]=j;
            mapColor(i,j);
          }
        }
      }
    }
    //Update new centroid
    for (int i = 0; i< m_K; i++) {
      QVector<float> coor = QVector<float>(m_dimension, 0.0f);
      int count = 0;
      for (int j = 0; j<m_pointNumber; j++) {
        if(m_class[j]==i){
          for (int k=0; k<m_dimension; k++) {
            coor[k] += m_points[ j * m_dimension + k];
          }
          count++;
        }
      }
      updateCentroids(coor, count, i);
    }
}

void ViewWidget::kmeans_initial(int k, int mode)
{
  m_K = k;
  //Initialization
  // Seed engine and set random distribution to [-1, 1]
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0, 1.0);
  //clear m parameters in case multiple initialization
  m_centroids.clear();
  m_class = QVector<int>(m_pointNumber,0);
  m_colors = QVector<float>(m_pointNumber * 3, 1.0f);
  m_colorMaps = colormapGenerator(m_K);
  //Random real
  if(mode == 0){
    qDebug()<<"random real value";
    for (int i=0; i<m_dimension * m_K; i++){
      float x = distribution(engine);
      m_centroids.append(x);
    }
  }else if(mode == 1){    //random sample
    qDebug()<<"random sample";
    for (int i=0; i<m_K; i++){
      int x = (distribution(engine)+1) / 2 * m_pointNumber;
      mapColor(x, i);
      x = x * m_dimension;
      for (int j=0; j<m_dimension; j++) {
        m_centroids.append(m_points.at(x+j));
      }
    }
  }else{    //K-Means++
    //Randomly select 1 sample first
    int x = (distribution(engine)+1) / 2 * m_pointNumber;
    mapColor(x, 0);
    x = x * m_dimension;
    for (int j=0; j<m_dimension; j++) {
      m_centroids.append(m_points.at(x+j));
    }
    // find the farthest sample
    for (int i = 0; i < m_K-1 ; ++i ) {
      float max = -1;
      int index = 0;
      for ( int j = 0; j < m_pointNumber; j++){
        float distance = 0;
        for (int k = 0; k < i+1; k++){
          distance += euclideanDistance(k, j);
          }
        if( distance > max ){
          max = distance;
          index = j;
        }
      }
      mapColor(index, i+1);
      //qDebug()<<"new centroids index"<<index;
      for (int j=0; j<m_dimension; j++) {
        m_centroids.append(m_points.at(index * m_dimension+j));
      }
    }
  }
}

float ViewWidget::euclideanDistance(int centroid_index, int point_index)
{
  float distance = 0.0f;
  for (int i=0; i<m_dimension; i++) {
    distance += qPow((m_centroids.at(centroid_index * m_dimension + i)-m_points.at(point_index * m_dimension + i)), 2);
  }
  return qSqrt(distance);
}

void ViewWidget::mapColor(int point_index, int colormap_index)
{
  for (int i=0; i<3; i++) {
    m_colors[point_index * 3 + i] = m_colorMaps[colormap_index * 3 + i];
  }
}

void ViewWidget::updateCentroids(QVector<float> coor, int count, int centroid_index)
{
  bool dirty = false;
  for (int i=0; i<m_dimension; i++) {
    if(coor[i]/count != m_centroids[centroid_index * m_dimension + i]){
      m_centroids[centroid_index * m_dimension + i] = coor[i]/count;
      dirty = true;
    }
  }
  dirty ? m_dirty = true : m_dirty = false;
}

void ViewWidget::setMovieOn(bool checked)
{
  m_movieOn = checked;
}

void ViewWidget::setPointsOn(bool checked)
{
  m_pointsOn = checked;
}

void ViewWidget::setAxisOn(bool checked)
{
  m_axisOn = checked;
}

void ViewWidget::setCentroidsOn(bool checked)
{
  m_centroidsOn = checked;
}

float rotationNorm(int angle){
  return (float)angle/16;
}

void ViewWidget::setXRotation(int angle)
{
  m_xRotation = rotationNorm(angle);
}

void ViewWidget::setYRotation(int angle)
{
  m_yRotation = rotationNorm(angle);
}

void ViewWidget::setZRotation(int angle)
{
  m_zRotation = rotationNorm(angle);
}

void ViewWidget::setZooming(int zoomLevel)
{
  m_zooming = (float)zoomLevel/360;
}





