#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow), m_controlPanel(new ControlPanel(this))
{
  ui->setupUi(this);
  m_controlPanel->show();
  connect(ui->actionStep_Kmeans, &QAction::triggered, ui->openGLWidget, &ViewWidget::kmeans_step);
  //Control Panel
  connect(ui->actionShow_Control_Panel, &QAction::triggered, m_controlPanel, &ControlPanel::show);
  connect(m_controlPanel, &ControlPanel::initialCentroids, ui->openGLWidget, &ViewWidget::kmeans_initial);
  connect(m_controlPanel, &ControlPanel::step, ui->openGLWidget, &ViewWidget::kmeans_step);
  connect(m_controlPanel, &ControlPanel::stepBack, ui->openGLWidget, &ViewWidget::kmeans_setpBack);
  connect(m_controlPanel, &ControlPanel::runThrough, ui->openGLWidget, &ViewWidget::kmeans_runthrough);
  connect(m_controlPanel, &ControlPanel::pointsShow, ui->openGLWidget, &ViewWidget::setPointsOn);
  connect(m_controlPanel, &ControlPanel::axisShow, ui->openGLWidget, &ViewWidget::setAxisOn);
  connect(m_controlPanel, &ControlPanel::centroidsShow, ui->openGLWidget, &ViewWidget::setCentroidsOn);
  connect(m_controlPanel, &ControlPanel::movieOn, ui->openGLWidget, &ViewWidget::setMovieOn);
  //Control Panel axis rotation
  connect(m_controlPanel, &ControlPanel::xAngle, ui->openGLWidget, &ViewWidget::setXRotation);
  connect(m_controlPanel, &ControlPanel::yAngle, ui->openGLWidget, &ViewWidget::setYRotation);
  connect(m_controlPanel, &ControlPanel::zAngle, ui->openGLWidget, &ViewWidget::setZRotation);
  //Control Panel zooming & panning
  connect(m_controlPanel, &ControlPanel::zooming, ui->openGLWidget, &ViewWidget::setZooming);
  connect(m_controlPanel, &ControlPanel::panningX, ui->openGLWidget, &ViewWidget::setPanningX);
  connect(m_controlPanel, &ControlPanel::panningY, ui->openGLWidget, &ViewWidget::setPanningY);
  //Sampling datapoints
  connect(m_controlPanel, &ControlPanel::randomSampling, ui->openGLWidget, &ViewWidget::generatePoints);
  //Sampling from file
  connect(m_controlPanel, &ControlPanel::loadingFileDir, ui->openGLWidget, &ViewWidget::generatePointsFromFile);
  //Changing point/centroid size
  connect(m_controlPanel, &ControlPanel::pointSize, ui->openGLWidget, &ViewWidget::setPointSize);
  connect(m_controlPanel, &ControlPanel::centroidSize, ui->openGLWidget, &ViewWidget::setCentroidSize);
}

MainWindow::~MainWindow()
{
  delete ui;
}
