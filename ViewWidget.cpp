#include "ViewWidget.h"

#include <chrono>
#include <random>
#include <QOpenGLShaderProgram>
#include <QtMath>
#include <QTimer>

ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  turntableTimer->start(30);

  // Create random 3D points
  int samples = 100000;

  // Get seed from clock
  long seed = std::chrono::system_clock::now().time_since_epoch().count();

  // Seed engine and set random distribution to [-1, 1]
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0, 1.0);

  // Create points inside a sphere
  int count = 0;
  while(count < samples)
  {
    // Uniformly sample cube
    float x = distribution(engine);
    float y = distribution(engine);
    float z = distribution(engine);

    // Reject all points outside the sphere
    if(std::sqrt(x*x + y*y + z*z) <= 1.0)
    {
      m_points.append({x, y, z});

      // Re-map positions to [0, 1] and use as color
      float r = (x + 1.0f)/2.0;
      float g = (y + 1.0f)/2.0;
      float b = (z + 1.0f)/2.0;

      m_colors.append({r, g, b});

      count++;
    }
  }
}

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

void ViewWidget::initializeGL()
{
 initializeOpenGLFunctions();

 glClearColor(0.2,0.2,0.2,1.0);

 glEnable(GL_DEPTH_TEST);
 glEnable(GL_POINT_SMOOTH);
 //glPointSize(5.0);

 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
   "attribute highp vec4 vertex;\n"
   "attribute mediump vec4 color;\n"
   "varying mediump vec4 vColor;\n"
   "uniform highp mat4 matrix;\n"
   "void main(void)\n"
   "{\n"
   "   gl_Position = matrix * vertex;\n"
   "   vColor = color;\n"
   "}");

 m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
   "varying mediump vec4 vColor;\n"
   "void main(void)\n"
   "{\n"
   "   gl_FragColor = vColor;\n"
   "}");

 m_pointProgram.link();
}

void ViewWidget::paintGL()
{
  QOpenGLShaderProgram program;
  program.addShaderFromSourceCode(QOpenGLShader::Vertex,
       "attribute highp vec4 vertex;\n"
       "uniform highp mat4 matrix;\n"
       "void main(void)\n"
       "{\n"
       "   gl_Position = matrix * vertex;\n"
       "}");
   program.addShaderFromSourceCode(QOpenGLShader::Fragment,
       "uniform mediump vec4 color;\n"
       "void main(void)\n"
       "{\n"
       "   gl_FragColor = color;\n"
       "}");
   program.link();
   program.bind();

   int vertexLocation = program.attributeLocation("vertex");
   int matrixLocation = program.uniformLocation("matrix");
   int colorLocation = program.uniformLocation("color");

   /*static GLfloat const triangleVertices[] = {
       60.0f,  10.0f,  0.0f,
       110.0f, 110.0f, 0.0f,
       10.0f,  110.0f, 0.0f
   };*/



   QMatrix4x4 pmvMatrix;
   //pmvMatrix.ortho(rect());
   //pmvMatrix.ortho(10, 110, 110, 10, -1, 1);float(width())/height()
   pmvMatrix.perspective(60,float(width())/height(), 1.0f, 1500.0f);
   pmvMatrix.lookAt({0,0,5},{0,0,0},{0,1,0});
   pmvMatrix.rotate(m_turntableAngle, {0.0f, 1.0f, 0.0f});


   m_pointProgram.bind();
   m_pointProgram.enableAttributeArray("vertex");
   m_pointProgram.enableAttributeArray("color");

   m_pointProgram.setUniformValue("matrix", pmvMatrix);
   m_pointProgram.setAttributeArray("vertex", m_points.constData(),3);
   m_pointProgram.setAttributeArray("color", m_colors.constData(),3);

   glDrawArrays(GL_POINTS, 0, m_points.count()/3);

   m_pointProgram.disableAttributeArray("vertex");
   m_pointProgram.disableAttributeArray("color");

//   auto pentagon = createPolygon(60,60, 0,50,5);
//   //program.setAttributeArray(vertexLocation, triangleVertices, 3);
//   program.setAttributeArray(vertexLocation, pentagon.constData(), 3);
//   program.setUniformValue(colorLocation, QColor(255, 255, 0, 255));
//   //glDrawArrays(GL_POLYGON, 0, pentagon.length()/3);

//   auto actagon = createPolygon(60,60,-0.5,60,8);
//   program.setAttributeArray(vertexLocation, actagon.constData(), 3);
//   program.setUniformValue(colorLocation, QColor(0, 255, 0, 255));
//   //glDrawArrays(GL_POLYGON, 0, actagon.length()/3);
//   //glDrawArrays(GL_TRIANGLES, 0, 3);

   program.bind();
   program.setUniformValue(matrixLocation, pmvMatrix);
   program.enableAttributeArray(vertexLocation);
   GLfloat axisWidth = 4.0;
   GLfloat axisLength = 500.0;
   glLineWidth(axisWidth);
   GLfloat const xAxis[] = {0.0f, 0.0f, 0.0f,
                           axisLength, 0.0f,0.0f};
   program.setAttributeArray(vertexLocation, xAxis, 3);
   program.setUniformValue("color",QColor(0,255,0,255));
   glDrawArrays(GL_LINES, 0, 2);
   GLfloat const yAxis[] = {0.0f, 0.0f, 0.0f,
                           0.0f, axisLength, 0.0f};
   program.setAttributeArray(vertexLocation, yAxis, 3);
   program.setUniformValue("color",QColor(255,0,255,255));
   glDrawArrays(GL_LINES, 0, 2);
   GLfloat const zAxis[] = {0.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, axisLength};
   program.setAttributeArray(vertexLocation, zAxis, 3);
   program.setUniformValue("color",QColor(255,0,0,255));
   glDrawArrays(GL_LINES, 0, 2);



   program.disableAttributeArray(vertexLocation);
}

void ViewWidget::updateTurntable()
{
  m_turntableAngle += 1.0f;

  update();
}






