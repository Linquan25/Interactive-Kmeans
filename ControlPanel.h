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

private slots:
  void on_randomSamplingB_clicked();

  void on_fileLoadingB_clicked();

private:
  void setSlider(QSlider * slider);
  Ui::ControlPanel *ui;
};

#endif // CONTROLPANEL_H
