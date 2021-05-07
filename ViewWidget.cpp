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

  turntableTimer->start(1000.0/60.0);
  m_elapsedTimer.start();
  m_fpsTimer.start();
}

static const char *vertexShaderCode_points =
    "attribute highp vec4 vertex;\n"
    "attribute mediump vec4 color;\n"
    "varying mediump vec4 vColor;\n"
    "uniform highp mat4 matrix;\n"
    "uniform mediump float pSize;\n"
    "void main(void)\n"
    "{\n"
    "   gl_PointSize = pSize;\n"
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
 glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
 //glPointSize(8.0);
 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
   vertexShaderCode_points);

 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
   fragmentShaderCode_points);

 m_pointProgram.link();
}

void ViewWidget::paintGL()
{
  glEnable(GL_DEPTH_TEST);
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
   pmvMatrix.lookAt({x_panning,y_panning,5+m_zooming},{x_panning,y_panning,0},{0,1,0});
   if(m_movieOn){
     pmvMatrix.rotate(angleForTime(m_elapsedTimer.elapsed(),15), {0.0f, 1.0f, 0.0f});
   }
   pmvMatrix.rotate(m_xRotation, {1.0f, 0.0f, 0.0f});
   pmvMatrix.rotate(m_yRotation, {0.0f, 1.0f, 0.0f});
   pmvMatrix.rotate(m_zRotation, {0.0f, 0.0f, 1.0f});


   m_pointProgram.bind();
   m_pointProgram.enableAttributeArray("vertex");
   m_pointProgram.enableAttributeArray("color");
   //Draw Datapoints
   m_pointProgram.setUniformValue("pSize", m_pointSize);
   m_pointProgram.setUniformValue("matrix", pmvMatrix);
   if(m_dimension > 3){
     m_pointProgram.setAttributeArray("vertex", m_pointsNDVisual.constData(),3);
   }else{
     m_pointProgram.setAttributeArray("vertex", m_points.constData(),m_dimension);
   }
   m_pointProgram.setAttributeArray("color", m_colors.constData(),3);
   if(m_pointsOn) glDrawArrays(GL_POINTS, 0, m_pointNumber);
   //Draw Centroids
   m_pointProgram.setUniformValue("pSize", m_centroidSize);
   if(m_dimension>3){
     m_pointProgram.setAttributeArray("vertex", m_centroidsNDVisual.constData(),3);
   }else{
     m_pointProgram.setAttributeArray("vertex", m_centroids.constData(),m_dimension);
   }
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

   //Paint useful/interesting information to the screen
   QPainter painter(this);
   painter.setPen(QColor(255,255,255,255));
   painter.drawText(QRect(5, 5, width(), 15), QString::number(m_fps,'G',4)+QString("FPS"));
   painter.drawText(QRect(5, 20, width(), 15), QString("K: ")+QString::number(m_K,'G',4));
   painter.drawText(QRect(5, 35, width(), 15), QString("Iteration: ")+QString::number(m_iteration,'G',4));
   painter.drawText(QRect(5, 50, width(), 15), QString("Energy: ")+QString::number(m_energy,'G',4));
   painter.drawText(QRect(5, 65, width(), 15), QString("Samples: ")+QString::number(m_pointNumber,'G',4));
   m_frameCount++;
   if(m_fpsTimer.elapsed() > 500){
     m_fps = float(m_frameCount)/m_fpsTimer.restart()*1000.0f;
     m_frameCount = 0;
   }
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
  if(dimension<2 || sampleNumber<2){
    QMessageBox::warning(this,"title","Invalid Input");
    return;
  }
  clearPoints();
  m_dimension = dimension;
  m_pointNumber = sampleNumber;
  // Create random 3D points
  // Get seed from clock
  long seed = std::chrono::system_clock::now().time_since_epoch().count();

  // Seed engine and set random distribution to [-1, 1]
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-3.0, 3.0);

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
  if(m_dimension>3) calculatePointsNDVisual();
}

void ViewWidget::generatePointsFromFile(QString dir)
{
  QFile file(dir);

  if(!file.open(QFile::ReadOnly | QFile::Text)){
    QMessageBox::warning(this,"title","File openning failed!");
    return;
  }
  clearPoints();
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
  if(m_dimension>3) calculatePointsNDVisual();
}

void ViewWidget::kmeans_step()
{
  // Seed engine and set random distribution to [-1, 1]
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0, 1.0);
  m_centroids_history_history = m_centroids_history;
  m_centroids_history = m_centroids;
  //clustering part
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
  m_energy = energyCalculation();
  m_iteration += 1;
  if(m_dimension>3) calculateCentroidsNDVisual();
}

void ViewWidget::kmeans_setpBack()
{
  if(m_centroids_history.size()==0){
    QMessageBox::warning(this,"title","You already stepped back!");
    return;
  }
  if(m_iteration<2||m_centroids_history_history.size()==0){
    QMessageBox::warning(this,"title","Too early to setp back!");
    return;
  }
  m_centroids = m_centroids_history_history;
  m_centroids_history.clear();
  kmeans_step();
  m_iteration -= 2;
}

void ViewWidget::kmeans_runthrough()
{
  bool dirty = true;
  while(dirty && m_iteration<1000){
    float energy_old = m_energy;
    kmeans_step();
    if(energy_old==m_energy) dirty = false;
  }
  if(!dirty){
    QMessageBox::warning(this,"title","Clustered!");
  }else{
    QMessageBox::warning(this,"title","Reach to End!");
  }
}

void ViewWidget::kmeans_initial(int k, int mode)
{
  if (k<2||k>m_pointNumber){
    QMessageBox::warning(this,"title","Invalid K number");
    return;
  }
  m_K = k;
  //Initialization
  // Seed engine and set random distribution to [-1, 1]
  long seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-20.0, 20.0);
  //clear m parameters in case multiple initialization
  m_centroids.clear();
  m_centroids_history.clear();
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
      int x = (distribution(engine)+20) / 40 * m_pointNumber;
      mapColor(x, i);
      x = x * m_dimension;
      for (int j=0; j<m_dimension; j++) {
        m_centroids.append(m_points.at(x+j));
      }
    }
  }else{    //K-Means++
    //Randomly select 1 sample first
    int x = (distribution(engine)+20) / 40 * m_pointNumber;
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
  m_iteration = 0;
  if(m_dimension>3) calculateCentroidsNDVisual();
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
  m_zooming = (float)zoomLevel/180;
}
//Clear history points
void ViewWidget::clearPoints()
{
  m_points.clear();
  m_centroids.clear();
  m_centroids_history.clear();
  m_centroids_history_history.clear();
  m_iteration = 0;
}

void ViewWidget::calculatePointsNDVisual()
{
  m_pointsNDVisual.clear();
  for (int i=0; i<m_pointNumber; i++) {
    for(int j=0; j<3; j++){
      m_pointsNDVisual.append(m_points[i * m_dimension + j]);
    }
  }
}

void ViewWidget::calculateCentroidsNDVisual()
{
  m_centroidsNDVisual.clear();
  for (int i=0; i<m_K; i++){
    for (int j=0; j<3; j++){
      m_centroidsNDVisual.append(m_centroids[i * m_dimension + j]);
    }
  }
}

float ViewWidget::energyCalculation()
{
  float energy = 0;
  for(int i=0; i<m_pointNumber; i++){
    energy += euclideanDistance(m_class[i], i);
  }
  return energy;
}

void ViewWidget::setPointSize(float size)
{
  m_pointSize = size;
}

void ViewWidget::setCentroidSize(float size)
{
  m_centroidSize = size;
}

void ViewWidget::setPanningX(float d)
{
  x_panning = d;
}

void ViewWidget::setPanningY(float d)
{
  y_panning = d;
}





