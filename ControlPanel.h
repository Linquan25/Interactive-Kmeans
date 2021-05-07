#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QDialog>
#include <QPushButton>
#include <QSlider>

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QDialog
{
  Q_OBJECT

public:
  explicit ControlPanel(QWidget *parent = nullptr);
  ~ControlPanel();

public slots:
  void initialCentroids_clicked();
  void step_clicked();
  void pointsChecked();
  void axisChecked();
  void movieChecked();
  void centroidsChecked();
  void xRotationChanged();
  void yRotationChanged();
  void zRotationChanged();
  void zoomingChanged();

signals:
  void initialCentroids(int K, int mode);
  void step();
  void stepBack();
  void runThrough();
  void pointsShow(bool checked);
  void axisShow(bool checked);
  void centroidsShow(bool checked);
  void movieOn(bool checked);
  void xAngle(int angle);
  void yAngle(int angle);
  void zAngle(int angle);
  void zooming(int distance);
  void randomSampling(int dimension, int sampleNumber);
  void loadingFileDir(QString dir);
  void pointSize(float size);
  void centroidSize(float size);
  void panningX(float d);
  void panningY(float d);

private slots:
  void on_randomSamplingB_clicked();

  void on_fileLoadingB_clicked();

  void on_stepbackB_clicked();

  void on_runB_clicked();

  void on_pointSizeSlider_valueChanged(int value);

  void on_centroidSizeSlider_valueChanged(int value);

  void on_horizontalSlider_valueChanged(int value);

  void on_verticalSlider_valueChanged(int value);

private:
  void setSlider(QSlider * slider);
  Ui::ControlPanel *ui;
};

#endif // CONTROLPANEL_H
