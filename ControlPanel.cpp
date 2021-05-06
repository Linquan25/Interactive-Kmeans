#include "ControlPanel.h"
#include "ui_ControlPanel.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>

ControlPanel::ControlPanel(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ControlPanel)
{
  ui->setupUi(this);
  ui->axisCheckbox->setChecked(true);
  ui->pointsCheckbox->setChecked(true);
  ui->centroidsCheckbox->setChecked(true);
  connect(ui->initialB, &QPushButton::clicked, this, &ControlPanel::initialCentroids_clicked);
  connect(ui->stepB, &QPushButton::clicked, this, &ControlPanel::step_clicked);
  connect(ui->axisCheckbox, &QCheckBox::stateChanged, this, &ControlPanel::axisChecked);
  connect(ui->pointsCheckbox, &QCheckBox::stateChanged, this, &ControlPanel::pointsChecked);
  connect(ui->movieCheckbox, &QCheckBox::stateChanged, this, &ControlPanel::movieChecked);
  connect(ui->centroidsCheckbox, &QCheckBox::stateChanged, this, &ControlPanel::centroidsChecked);
  //Sliders settings and connection
  setSlider(ui->xSlider);
  setSlider(ui->ySlider);
  setSlider(ui->zSlider);
  setSlider(ui->zoomingSlider);
  connect(ui->xSlider, &QSlider::valueChanged, this, &ControlPanel::xRotationChanged);
  connect(ui->ySlider, &QSlider::valueChanged, this, &ControlPanel::yRotationChanged);
  connect(ui->zSlider, &QSlider::valueChanged, this, &ControlPanel::zRotationChanged);
  connect(ui->zoomingSlider, &QSlider::valueChanged, this, &ControlPanel::zoomingChanged);
}

ControlPanel::~ControlPanel()
{
  delete ui;
}

void ControlPanel::initialCentroids_clicked()
{
  emit initialCentroids(ui->kNumber->value(), ui->comboBox->currentIndex());
}

void ControlPanel::step_clicked()
{
  emit step();
}

void ControlPanel::axisChecked()
{
  emit axisShow(ui->axisCheckbox->isChecked());
}

void ControlPanel::movieChecked()
{
  emit movieOn(ui->movieCheckbox->isChecked());
}

void ControlPanel::centroidsChecked()
{
  emit centroidsShow(ui->centroidsCheckbox->isChecked());
}

void ControlPanel::xRotationChanged()
{
  emit xAngle(ui->xSlider->value());
}

void ControlPanel::yRotationChanged()
{
  emit yAngle(ui->ySlider->value());
}

void ControlPanel::zRotationChanged()
{
  emit zAngle(ui->zSlider->value());
}

void ControlPanel::zoomingChanged()
{
  emit zooming(ui->zoomingSlider->value());
}

void ControlPanel::setSlider(QSlider *slider)
{
  slider->setRange(0, 360 * 16);
  slider->setSingleStep(16);
  slider->setPageStep(15 * 16);
  slider->setTickInterval(15 * 16);
  slider->setTickPosition(QSlider::TicksRight);
}


void ControlPanel::pointsChecked()
{
  emit pointsShow(ui->pointsCheckbox->isChecked());
}


void ControlPanel::on_randomSamplingB_clicked()
{
  emit randomSampling(ui->dimension->value(),ui->sampleNumber->value());
}

void ControlPanel::on_fileLoadingB_clicked()
{
  QString file_name = QFileDialog::getOpenFileName(this,"Select a File", QDir::homePath());
  emit loadingFileDir(file_name);
}
